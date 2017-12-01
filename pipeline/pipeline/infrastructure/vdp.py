"""
vdp is a pipeline framework module that contains classes to make writing task
Inputs easier.

- InputsContainer lets task implementations operate within the scope of a
  single measurement set, even if the pipeline run contains multiple data
  sets.

- VisDependentProperty is a reworking of pipeline properties to reduce the
  amount of boilerplate code required to implement an Inputs class.
    
Implementation details:

    See the documentation on the classes, particularly VisDependentProperty,
    for detailed information on how the framework operates. 
    
Examples:

    There are three common scenarios that use VisDependentProperty. The
    following examples show each scenario for an Inputs property belonging to
    an Inputs class that extends vdp.StandardInputs.
    
    1. To provide a default value that can be overridden on a per-MS basis. Use
       the optional 'default' argument to VisDependentProperty, eg: 
    
        myarg = VisDependentProperty(default='some value')
        
    2. For more sophisticated default values, e.g., a default value that is a
       function of other data or properties, use the @VisDependentProperty
       decorator. A class property decorated with @VisDependentProperty should
       return the default value for that property. The function will execute
       in the scope of a single measurement set, i.e., at the time it is
       called, vis is set to exactly one value. The function will be called to
       provide a default value for any measurement set that does not have a
       user override value.

        @VisDependentProperty
        def myarg():
            # do some processing then return the calculated value
            return 'I am a custom property for measurement set %s' % self.vis

    3. Convert or validate user input before accept it as an Inputs argument.
       Use the @VisDependentProperty.convert decorator, possibly alongside the
       getter decorator as above.
       
        @VisDependentProperty
        def myarg():
            # this will return 100 - but only if the user has not supplied
            # an override value!
            return 100

        @VisDependentProperty.convert
        def myarg(user_input):
            # convert then return the user input which is provided as an
            # argument to the convert decorator. The converted value will be
            # returned for all subsequent 'gets'.
            return int(user_input)
    
"""
from __future__ import absolute_import
import collections
import copy
import inspect
import numbers
import os
import pprint

from . import api
from . import argmapper
from . import launcher
from . import logging
from . import utils

__all__ = ['VisDependentProperty',
           'InputsContainer',
           'StandardInputs',
           'ModeInputs']

LOG = logging.get_logger(__name__)


class SingletonType(type):
    """
    SingletonType is a metaclass that ensures that only a single instance of
    a class exists. Creating additional instances returns the existing
    instance.
    """

    def __call__(cls, *args, **kwargs):
        try:
            return cls.__instance
        except AttributeError:
            cls.__instance = super(SingletonType, cls).__call__(*args, **kwargs)
            return cls.__instance


class NullMarker(object):
    """
    NullMarker is a class that represents the null "parameter not-set" case.
    It exists to distinguish between a user-provided null value, such as None
    or '', and an argument that is null because it has not been set.
    """
    __metaclass__ = SingletonType

    # user inputs considered equivalent to a NullMarker. Inputs contained in
    # this set will
    __NULL_INPUT = frozenset(['', None])

    def convert(self, val):
        """
        Process the argument, converting user input considered equivalent to
        null to a NullMarker object.
        """
        # can't check __NULL_INPUT for unhashable types. We know that
        # __NULL_INPUT does not contain them, so return the value
        if isinstance(val, NullMarker):
            return self
        elif isinstance(val, list):
            return val
        elif val in self.__NULL_INPUT:
            return self
        else:
            return val

    def __eq__(self, other):
        return isinstance(other, NullMarker)

    def __ne__(self, other):
        return not isinstance(other, NullMarker)


class NoDefaultMarker(object):
    """
    NoDefaultMarker is a class that represents the null "parameter not-set" case
    for default values. It exists to distinguish between a user-provided
    default value of None, and when a default has not been set.
    """
    __metaclass__ = SingletonType

    def __eq__(self, other):
        return isinstance(other, NoDefaultMarker)

    def __ne__(self, other):
        return not isinstance(other, NoDefaultMarker)


class PipelineInputsMeta(type):
    """
    Sets the name of a VisDependentProperty at class definition time.
    """
    def __new__(mcls, name, bases, attrs):
        cls = super(PipelineInputsMeta, mcls).__new__(mcls, name, bases, attrs)
        for attr, obj in attrs.iteritems():
            if isinstance(obj, VisDependentProperty):
                obj.__set_name__(cls, attr)
        return cls


class VisDependentProperty(object):
    """
    VisDependentProperty is a Python data descriptor that standardises the
    behaviour of pipeline Inputs properties and lets them create default values
    more easily.

    On reading a VisDependentProperty (ie. using the dot prefix: inputs.solint),
    one of two things happens:

    1. If a NullMarker is found - signifying that no user input has been
       provided - and a 'getter' function has been defined, the getter function
       will be called to provide a default value for that measurement set.

    2. If a user has overridden the value (eg. inputs.solint = 123), that
       value will be retrieved.

    3. The value, either the default from step 1 or user-provided from step 2,
       is run through the optional postprocess function, which gives a final
       opportunity to change the value depending on the state/value of other
       properties.
    """
    NULL = NullMarker()

    # TODO check whether this can be replaced with NULL
    NO_DEFAULT = NoDefaultMarker()

    @property
    def backing_store_name(self):
        """
        The name of the attribute holding the value for this property.
        """
        return '_' + self.name

    def convert(self, fconvert):
        """
        Set the function used to clean and/or convert user-supplied argument
        values before they are associated with the instance property.

        The provided function should accept one unnamed argument, which when
        passed will be the user input *for this measurement set*. That is,
        after potentially being divided up into per-measurement values.
        """
        return type(self)(self.fdefault, fconvert, self.fpostprocess, default=self.default, hidden=self.hidden)

    def default(self, fdefault):
        """
        Set the function used to get the attribute value when the user has not
        supplied an override value.
        """
        return type(self)(fdefault, self.fconvert, self.fpostprocess, hidden=self.hidden)

    def fget(self, owner):
        """
        Gets the underlying property value from an instance of the class
        owning this property

        :param owner:
        :return:
        """
        return getattr(owner, self.backing_store_name, VisDependentProperty.NULL)

    def fset(self, owner, value):
        """
        Sets the property value on the instance owning this property.

        :param owner:
        :param value:
        :return:
        """
        setattr(owner, self.backing_store_name, value)

    def postprocess(self, fpostprocess):
        """
        Set the function used to process the value that is about to be
        returned. This allows the value to be modified or perhaps a different
        value based on another property to be returned.

        :param owner:
        :return:
        """
        return type(self)(self.fdefault, self.fconvert, fpostprocess, default=self.default, hidden=self.hidden)

    def __init__(self, fdefault=None, fconvert=None, fpostprocess=None, default=NO_DEFAULT, readonly=False,
                 hidden=False):
        self.fdefault = fdefault
        self.fconvert = fconvert
        self.fpostprocess = fpostprocess
        self.default = default
        self.readonly = readonly
        self.hidden = hidden

    def __call__(self, fget, *args, **kwargs):
        # __call__ is executed when decorating a readonly function
        # LOG.info('In __call__ for %s' % fget.func_name)
        self.fget = fget
        return self

    def __get__(self, instance, owner):
        # Return the VisDependentProperty itself when called directly
        if instance is None:
            return self

        instance_val = self.fget(instance)

        if instance_val == VisDependentProperty.NULL:
            if self.fdefault:
                instance_val = self.fdefault(instance)
            elif self.default != VisDependentProperty.NO_DEFAULT:
                instance_val = self.default
            else:
                raise ValueError('Cannot get property with no default and no user value')

        if self.fpostprocess:
            return self.fpostprocess(instance, instance_val)
        else:
            return instance_val

    def __set__(self, instance, value):
        if self.readonly:
            raise AttributeError('can\'t set read-only attribute: {!s}'.format(self.name))

        value = VisDependentProperty.NULL.convert(value)

        # pass non-null values through the user-provided converter
        converted = value
        if self.fconvert is not None:
            if value != VisDependentProperty.NULL:
                converted = self.fconvert(instance, value)

        self.fset(instance, converted)

    def __set_name__(self, owner, name):
        self.name = name


class InputsContainer(object):
    """
    InputsContainer is the top-level container object for all task Inputs.

    InputsContainer contains machinery to let Inputs classes operate purely in
    the scope of a single measurement set, to make both Inputs and Task
    implementation much simpler.

    The InputsContainer operates in the scope of multiple measurement sets,
    and holds one Inputs instance for every measurement set within the context
    At task execution time, the task is executed for each active Inputs
    instance. Not all the Inputs instances held by the InputsContainer need be
    active: the user can reduce the scope of the task to a subset of
    measurement sets by setting vis, which makes an Inputs instance hidden and
    inactive.

    Tasks that operate in the scope of more than one measurement set, e.g,
    imaging and session-aware tasks, can disable the InputsContainer machinery
    by setting is_multi_vis_task to True. For these multivis tasks, one Inputs
    instance is held in an InputsContainer, but all property sets and gets pass
    directly through the one underlying inputs instance.


    For tasks that operate in the scope of a single measurement set, the
    InputsContainer class works in conjunction with VisDependentProperty to
    provide and process user input (eg. inputs.solint = 123) according to a set
    of rules:

    1. If the input is scalar and equal to '' or None, all measurement sets
       will be mapped back to NullMarker, therefore returning the default
       value or custom getter function on subsequent access.

    2. If the input is a list with number of items equal to the number of
       measurement sets, the items will be divided up and treated as mapping
       one value per measurement set.

    3. Otherwise, the user input is considered as the new default value for
       all measurement sets.

    Before the user input is stored in the dictionary, however, the input is
    passed through the convert function, assuming one has been provided. The
    convert function allows the developer to validate or convert user input to
    a standard format before accepting it as a new argument.
    """
    def __init__(self, task_cls, context, **kwargs):
        self._context = context
        self._task_cls = task_cls

        # Inspect the Inputs signature to find which attribute sets the scope
        # (=the MSes to process) for that task.
        constructor_sig = inspect.getargspec(task_cls.Inputs.__init__)
        if 'vis' in constructor_sig.args:
            self._scope_attr = 'vis'
        elif 'infiles' in constructor_sig.args:
            self._scope_attr = 'infiles'
        else:
            msg = 'No scope argument recognised in {!s} constructor' ''.format(self._task_cls.Inputs.__name__)
            raise AttributeError(msg)

        # TODO SJW move fn to this module
        from . import basetask
        imaging_preferred = basetask.get_imaging_preferred(task_cls.Inputs)
        ms_pool = context.observing_run.get_measurement_sets(imaging_preferred=imaging_preferred)

        # task requires all MSes. No processing required.
        self._multivis = task_cls.is_multi_vis_task
        if self._multivis:
            self._cls_instances = {'all': task_cls.Inputs(context)}
            self._active_instances = self._cls_instances.values()

        else:
            # create an instance for every MS in scope. Use the pointer
            # attribute to know which attribute is the MS 'axis'.
            scope_fn = lambda ms: {self._scope_attr: ms.name}

            try:
                self._cls_instances = {ms.basename: task_cls.Inputs(context, **scope_fn(ms)) for ms in ms_pool}
            except TypeError:
                # catch TypeError exceptions from unexpected keyword arguments
                # so that we can add some more context to the debug message
                LOG.error('Error creating {!s}'.format(task_cls.Inputs.__name__))
                raise
            self._active_instances = self._cls_instances.values()

        # TODO find kwargs at runtime so that changes can be reflected?
        self._initargs = dict(kwargs)

        # When importing data for the first time, the ms_pool is empty and so
        # _cls_instances remains empty too. That's ok, as setting vis will
        # create any missing instances for that value of vis. However, vis
        # must be the first argument set otherwise the setting of any
        # properties prior to the setting of vis will not take effect as they
        # had no instance to write to.
        if 'vis' in kwargs and len(ms_pool) is 0:
            self.vis = kwargs['vis']
            del kwargs['vis']

        for k, v in kwargs.iteritems():
            setattr(self, k, v)

    @property
    def _pipeline_casa_task(self):
        # Map the Inputs class to the hif* equivalent. Note that
        # classToCASATask maps Task classes, not Inputs classes, to their hif
        # equivalent. However, Task.Inputs *does* point to an Inputs class so
        # we can compare self against that.
        import pipeline.infrastructure.casataskdict as ctd
        casa_tasks = [casa_cls for task_cls, casa_cls in ctd.classToCASATask.items()
                      if task_cls.Inputs is self._task_cls.Inputs]

        if len(casa_tasks) is not 1:
            return

        # map Python Inputs arguments back to their CASA equivalent
        remapped = argmapper.inputs_to_casa(self._task_cls.Inputs, self._initargs)

        # CAS-6299. Extra request from Liz:
        #
        # "the full directory path of the ASDM location is given from the Pipeline
        # observatory run, so a PI/DRMs would have to edit this. Could it be
        # replaced just by the name of the ASDM/ASDMs?"
        #
        # this means we have to take the basename of the vis argument for the
        # importdata calls
        if '_importdata' in casa_tasks[0]:
            key = self._scope_attr
            remove_slash = lambda x: x.rstrip('/')
            if isinstance(remapped[key], str):
                remapped[key] = os.path.basename(remove_slash(remapped[key]))
            else:
                remapped[key] = [os.path.basename(remove_slash(v)) for v in remapped[key]]

        task_args = ['%s=%r' % (k, v) for k, v in remapped.items()
                     if k not in ['self', 'context']
                     and v is not None]

        # work around CASA problem with globals when no arguments are specified
        if not task_args:
            task_args = ['pipelinemode="automatic"']

        casa_call = '%s(%s)' % (casa_tasks[0], ', '.join(task_args))

        return casa_call

    def __len__(self):
        return len(self._active_instances)

    def __iter__(self):
        return iter(self._active_instances)

    def __getitem__(self, index):
        cls = type(self)
        if isinstance(index, numbers.Integral):
            return self._active_instances[index]
        elif isinstance(index, str):
            return self._cls_instances[index]
        else:
            msg = '{cls.__name__} indices must either be integers or the name of a measurement set'
            raise TypeError(msg.format(cls=cls))

    def __members__(self):
        raise NotImplemented

    def __methods__(self):
        raise NotImplemented

    def __getattr__(self, name):
        # __getattr__ is only called when this object's __dict__ does not
        # contain the requested property. When this happens, we delegate to
        # the currently selected Inputs. First, however, we check whether
        # the requested property is one of the Python magic methods. If so,
        # we return the standard implementation. This is necessary for
        # pickle, which checks for __getnewargs__ and __getstate__.
        if name.startswith('__') and name.endswith('__'):
            # LOG.trace('Implement {!s} in InputsContainer'.format(name))
            return super(InputsContainer, self).__getattr__(name)

        if name in dir(self):
            return super(InputsContainer, self).__getattr__(name)

        if name == self._scope_attr:
            LOG.trace('Retrieving scope from {!s}.{!s}'.format(self._task_cls.Inputs.__name__, name))
            return self._get_scope()

        LOG.trace('InputsContainer.{!s}: delegating to {!s}'.format(name, self._task_cls.Inputs.__name__))
        result = [getattr(i, name) for i in self._active_instances]

        return format_value_list(result)

    def __setattr__(self, name, val):
        # If the property we're trying to set is one of this base class's
        # private variables, add it to our __dict__ using the standard
        # __setattr__ method
        if name in ('_context', '_task_cls', '_cls_instances', '_active_instances', '_scope_attr', '_initargs',
                    '_multivis'):
            if LOG.isEnabledFor(logging.TRACE):
                LOG.trace('Setting {!s}.{!s} = {!r}'.format(self.__class__.__name__, name, val))
            return super(InputsContainer, self).__setattr__(name, val)

        # check whether this class has a getter/setter by this name. If so,
        # allow the write to __dict__
        if name == self._scope_attr:
            if LOG.isEnabledFor(logging.TRACE):
                LOG.trace('Setting scope: {!s}.{!s} = {!r}'.format(self.__class__.__name__, name, val))
            return self._set_scope(val)

        if not isinstance(val, (list, tuple)):
            val = [val] * len(self._active_instances)

        for inputs, user_arg in zip(self._active_instances, val):
            # some properties may be instance values and not data descriptors
            # on the class, hence the None default
            prop = getattr(inputs.__class__, name, None)

            # convert null equivalent values to NULL marker. Only VDP knows
            # how to handle these, so ensure the property is of the
            # appropriate type before conversion.
            if isinstance(prop, VisDependentProperty):
                null_marker = VisDependentProperty.NULL.convert(user_arg)

                # convert property if it's not null
                if getattr(prop, 'fconvert', None) is not None:
                    if null_marker != VisDependentProperty.NULL:
                        user_arg = prop.fconvert(inputs, user_arg)

            if LOG.isEnabledFor(logging.TRACE):
                LOG.trace('Setting {!s}.{!s} = {!r}'.format(inputs.__class__.__name__, name, user_arg))

            setattr(inputs, name, user_arg)

    def _get_scope(self):
        scope = [getattr(i, self._scope_attr) for i in self._active_instances]
        return format_value_list(scope)

    def _set_scope(self, scope):
        # for multivis tasks, all we should do is set vis on the contained
        # inputs. As there only ever one active inputs instance, no further
        # processing is required so we can exit.
        if self._multivis:
            setattr(self._active_instances[0], self._scope_attr, scope)
            return

        # reset to all MSes if the input arg signals a reset, which expands
        # task scope to all MSes
        if VisDependentProperty.NULL.convert(scope) == VisDependentProperty.NULL:
            scope = self._cls_instances.keys()

        # the key for inputs instances is the basename vis
        basenames = [os.path.basename(v) for v in scope]

        # create a new inputs instance if one cannot be found
        for basename, path in zip(basenames, scope):
            if basename in self._cls_instances:
                continue
            kwargs = {self._scope_attr: path}
            self._cls_instances[basename] = self._task_cls.Inputs(self._context, **kwargs)

        # set the task scope to the vises set here
        self._active_instances = [self._cls_instances[n] for n in basenames]

    def __repr__(self):
        return '<InputsContainer({!s}, {!r}>'.format(self._task_cls, self._context.name)

    def __str__(self):
        if not self._active_instances:
            return 'Empty'
        attrs = {name for name in dir(self._task_cls.Inputs) if not name.startswith('_')}
        methods = {fn_name for fn_name, _ in inspect.getmembers(self._task_cls.Inputs, inspect.ismethod)}
        props = attrs - methods
        props.discard('context')
        props.discard('ms')
        vals = {k: getattr(self, k) for k in props}
        return pprint.pformat(vals)

    def as_dict(self):
        properties = {}

        input_dicts = [i.as_dict() for i in self._active_instances]
        all_keys = {key for d in input_dicts for key in d}
        for key in all_keys:
            vals = [d.get(key, []) for d in input_dicts]
            properties[key] = format_value_list(vals)

        return properties



class StandardInputs(api.Inputs):
    __metaclass__ = PipelineInputsMeta

    #- standard non-vis-dependent properties ---------------------------------

    # def __deepcopy__(self, memo):
    #     return selective_deepcopy(self, memo, shallow_copy=('_context',))

    @property
    def context(self):
        """
        Get the context used as the base for the parameters for this Inputs.

        :rtype: :class:`~pipeline.infrastructure.launcher.Context`
        """
        # A product of the prepare/analyse refactoring is that we always need
        # a context. This message is to ensure it was set by the implementing
        # subclass. We could remove this once refactoring is complete, but it
        # should be a cheap comparison so we leave it in.
        #
        # Note that the Context is NOT vis-dependent. The same context is used
        # to source all MeasurementSets.
        if not isinstance(self._context, launcher.Context):
            msg = '{!s} did not set the pipeline context'.format(self.__class__.__name__)
            raise TypeError(msg)
        return self._context
        
    @context.setter
    def context(self, value):
        if not isinstance(value, launcher.Context):
            msg = 'context not a pipeline context: {!s}'.format(value.__class__.__name__)
            raise TypeError(msg)
        self._context = value

    @property
    def vis(self):
        """
        Get the filenames of the measurement sets on which this task should
        operate.
        """ 
        return self._vis

    @vis.setter
    def vis(self, value):
        """
        Set the filenames of the measurement sets on which this task should
        operate.
        
        If value is not set, this defaults to all measurement sets registered
        with the context.
        """ 
        if value is None:
            imaging_preferred = isinstance(self, api.ImagingMeasurementSetsPreferred)
            mses = self.context.observing_run.get_measurement_sets(imaging_preferred=imaging_preferred)
            value = [ms.name for ms in mses]

        if len(value) == 1:
            LOG.warn('Extracting vis from single-value list')
            value = value[0]

        # for compatibility with the old implementation, single-value lists
        # should be kept as lists
        self._vis = value


    #- vis-dependent properties ----------------------------------------------

    @VisDependentProperty(readonly=True, hidden=True)
    def ms(self):
        """
        Return the MeasurementSet for the current value of vis.
     
        :rtype: :class:`~pipeline.domain.MeasurementSet`
        """
        return self.context.observing_run.get_ms(self.vis)

    @VisDependentProperty
    def output_dir(self):
        return self.context.output_dir

    def _get_task_args(self, ignore=None):
        """
        Express this class as a dictionary of CASA arguments, listing all
        inputs except those named in ignore. 
        
        The main purpose of the ignore argument is used to prevent an infinite
        loop in :meth:`~CommonCalibrationInputs.caltable`, which determines the
        value of caltable based on the value of the other CASA task arguments.
        """
        if ignore is None:
            ignore = []

        # get the signature of this Inputs class. We want to return a
        # of dictionary of all kw argument names except self, the 
        # pipeline-specific arguments (context, output_dir, run_qa2 etc.) and
        # caltable.
        skip = ['self', 'context', 'output_dir', 'ms', 'calstate']
        skip.extend(ignore)

        kw_names = [a for a in inspect.getargspec(self.__init__).args if a not in skip]
        d = {key: getattr(self, key) for key in kw_names}

        # add any read-only properties too
        for k, v in inspect.getmembers(self.__class__, inspect.isdatadescriptor):
            if k in d or k.startswith('_') or k in skip:
                continue

            try:
                d[k] = getattr(self, k)
            except AttributeError:
                LOG.debug('Could not get input property %s' % k)

        return d

    def to_casa_args(self):
        """
        Express these inputs as a dictionary of CASA arguments. The values
        in the dictionary are in a format suitable for CASA and can be 
        directly passed to the CASA task.
        
        :rtype: a dictionary of string/??? kw/val pairs
        """        
        args = self._get_task_args()

        # spw needs to be a string and not a number
        if 'spw' in args:
            args['spw'] = str(args['spw'])

        # Handle VLA-specific arguments and peculiarities
        ms = self.ms
        if ms.antenna_array.name == 'VLA':
            # CASA believes that VLA data are not labelled with calibration
            # intent, so must remove the intent from the task call
            args['intent'] = None

        if args.get('intent', None) is not None:
            args['intent'] = utils.to_CASA_intent(ms, args['intent'])

        for k, v in args.items():
            if v is None:
                del args[k]        
        return args

    def __str__(self):
        return self.as_dict()

    def __repr__(self):
        return pprint.pformat(self.as_dict())

    def as_dict(self):
        return utils.collect_properties(self)


class ModeInputs(api.Inputs):
    """
    ModeInputs is a facade for Inputs of a common task type, allowing the user
    to switch between task implementations by changing the 'mode' parameter.

    Extending classes should override the _modes dictionary with a set of 
    key/value pairs, each pair mapping the mode name key to the task class
    value.
    """
    __metaclass__ = PipelineInputsMeta

    _modes = {}

    def __init__(self, context, mode=None, **parameters):
        # create a dictionary of Inputs objects, one of each type
        self._delegates = {k: task_cls.Inputs(context, vis=parameters['vis']) for k, task_cls in self._modes.items()}

        # set the mode to the requested mode, thus setting the active Inputs
        self.mode = mode

        # set any parameters provided by the user
        for k, v in parameters.items():
            setattr(self, k, v)

    def __getattr__(self, name):
        # __getattr__ is only called when this object's __dict__ does not
        # contain the requested property. When this happens, we delegate to
        # the currently selected Inputs. First, however, we check whether
        # the requested property is one of the Python magic methods. If so,
        # we return the standard implementation. This is necessary for
        # pickle, which checks for __getnewargs__ and __getstate__.
        if name.startswith('__') and name.endswith('__'):
            return super(ModeInputs, self).__getattr__(name)

        if name in dir(self):
            return super(ModeInputs, self).__getattr__(name)

        LOG.trace('ModeInputs.{!s}: delegating to {!s}'.format(name, self._active.__class__.__name__))
        return getattr(self._active, name)

    def __setattr__(self, name, val):
        # If the property we're trying to set is one of this base class's
        # private variables, add it to our __dict__ using the standard
        # __setattr__ method
        if name in ('_active', '_delegates', '_mode', '_pipeline_casa_task'):
            LOG.trace('Setting {!s}.{!s} = {!r}'.format(self.__class__.__name__, name, val))
            return super(ModeInputs, self).__setattr__(name, val)

#         # check whether this class has a getter/setter by this name. If so,
#         # allow the write to __dict__
#         for (fn_name, _) in inspect.getmembers(self.__class__,
#                                                inspect.isdatadescriptor):
#             # our convention is to prefix the data variable for a
#             # getter/setter with an underscore.
#             if name in (fn_name, '_' + fn_name):
#                 LOG.trace('Getter/setter found on {0}. Setting \'{1}\' '
#                           'attribute to \'{2}\''.format(self.__class__.__name__,
#                                                         name, val))
#                 super(ModeInputs, self).__setattr__(name, val)
#
#                 # overriding defaults of wrapped classes requires us to re-get
#                 # the value after setting it, as the property setter of this
#                 # superclass has probably transformed it, eg. None => 'inf'.
#                 # Furthermore, we do not return early, giving this function a
#                 # chance to set the parameter - with this new value - on the
#                 # wrapped classes too.
#                 val = getattr(self, name)

        # check whether this class has a getter/setter by this name. If so,
        # allow the write to __dict__
        for fn_name in dir(self):
            # our convention is to prefix the data variable for a
            # getter/setter with an underscore.
            if name in (fn_name, '_' + fn_name):
            # if name == fn_name:
                LOG.trace('Getter/setter found: setting {!s}.{!s} = {!r}'.format(self.__class__.__name__, name, val))
                super(ModeInputs, self).__setattr__(name, val)

                # overriding defaults of wrapped classes requires us to re-get
                # the value after setting it, as the property setter of this
                # superclass has probably transformed it, eg. None => 'inf'.
                # Furthermore, we do not return early, giving this function a
                # chance to set the parameter - with this new value - on the
                # wrapped classes too.
                val = getattr(self, name)

        # otherwise, set the said attribute on all of our delegate Inputs. In
        # doing so, the user can switch mode at any time and have the new
        # Inputs present with all their previously set parameters.
        for d in self._delegates.values():
            if hasattr(d, name):
                LOG.trace('Setting {!s}.{!s} = {!r}'.format(self.__class__.__name__, name, val))
                setattr(d, name, val)

    @property
    def mode(self):
        return self._mode

    @mode.setter
    def mode(self, value):
        if value not in self._modes:
            keys = self._modes.keys()
            msg = 'Mode must be one of \'{0}\' but got \'{1}\''.format(
                '\', \''.join(keys[:-1]) + '\' or \'' + keys[-1], value)
            LOG.error(msg)
            raise ValueError(msg)

        self._active = self._delegates[value]
        self._mode = value

    def get_task(self):
        """
        Get the task appropriate to the current Inputs.
        """
        task_cls = self._modes[self._mode]
        return task_cls(self._active)

    def as_dict(self):
        props = utils.collect_properties(self._active)
        props.update(utils.collect_properties(self))
        return props

    def to_casa_args(self):
        return self._active.to_casa_args()

    def __repr__(self):
        return pprint.pformat(self.as_dict())

    @classmethod
    def get_constructor_args(cls, ignore=('self', 'context')):
        """
        Get the union of all arguments accepted by this class's constructor.
        """
        all_args = set()

        # get the arguments for this class's contructor
        args = inspect.getargspec(cls.__init__).args
        # and add them to our collection
        all_args.update(args)

        # now do the same for each inputs class we can switch between
        for task_cls in cls._modes.values():
            # get the arguments of the task Inputs constructor
            args = inspect.getargspec(task_cls.Inputs.__init__).args
            # and add them to our set
            all_args.update(args)

        if ignore is not None:
            for i in ignore:
                all_args.discard(i)

        return all_args


# module utility functions ---------------------------------------------------


def all_unique(o):
    """
    Return True if all elements in the iterable argument are unique.
    """
    # LOG.trace('all_unique(%s)' % str(o))
    if not isinstance(o, collections.Iterable):
        raise ValueError('Cannot determine uniqueness of non-iterables')

    hashes = {gen_hash(e) for e in o}
    return len(hashes) > 1


def gen_hash(o):
    """
    Makes a hash from a dictionary, list, tuple or set to any level, that
    contains only other hashable types (including any lists, tuples, sets,
    and dictionaries).
    """
    # LOG.trace('gen_hash(%s)' % str(o))
    if isinstance(o, (set, tuple, list)):
        return tuple([gen_hash(e) for e in o])

    elif not isinstance(o, dict):
        # h = hash(o)
        # LOG.trace('Hash: %s=%s' % (o, h))
        return hash(o)

    new_o = copy.deepcopy(o)
    for k, v in new_o.items():
        new_o[k] = gen_hash(v)

    return hash(tuple(frozenset(new_o.items())))


# def selective_deepcopy(instance, memo, shallow_copy=None):
#     """
#
#     :param instance:
#     :param memo:
#     :param shallow_copy:
#     :return:
#     """
#     if shallow_copy is None:
#         shallow_copy = []
#
#     # extract the state from the instance to deep copy
#     obj_state = instance.__dict__
#     vdp_state = get_state(instance)
#
#     # create a new instance of the same class
#     cls = instance.__class__
#     result = cls.__new__(cls)
#     memo[id(instance)] = result
#
#     # set shallow copies for the requested attributes by directly copying
#     # across the reference from the old instance
#     for attr_name in shallow_copy:
#         setattr(result, attr_name, getattr(instance, attr_name))
#     # assign deep-copied values for the remaining attributes
#     for k, v in obj_state.iteritems():
#         if k not in shallow_copy:
#             setattr(result, k, copy.deepcopy(v, memo))
#
#     # set the VDP state, noting that this state too needs to be deep-copied to
#     # prevent shared references between the two objects
#     set_state(result, copy.deepcopy(vdp_state))
#
#     return result


def format_value_list(val):
    # return single values where possible, which is when only one value
    # is present because the inputs covers one ms or because the values
    # for each ms are all the same.
    if len(val) is 0:
        return val
    elif len(val) is 1:
        return val[0]
    else:
        if all_unique(val):
            return val
        else:
            return val[0]
