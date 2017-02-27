"""
vdp is a pipeline framework module that allows tasks to handle user 
arguments when multiple measurement sets have been registered with the pipeline. 

Background:    
    
    When set to provide automatic values, the original Inputs classes 
    implementations were intelligent enough to provide different input
    arguments depending on the current value of vis. This functionality scaled
    to multiple measurement sets; so long as the Inputs parameter was not
    overridden, the pipeline could provide ms-specific arguments for each
    measurement set covered by the task call. 
    
    However, the original implementation could not handle user overrides that
    are measurement set dependent, for example, a different solint for each 
    measurement set. This meant all measurement sets had to be reduced with
    the same single override argument, or the measurement sets were
    individually reduced so that the appropriate override argument could be
    provided with its matching measurement set.

    VisDependentProperty is a reworking of pipeline properties that allows
    values for specific measurement sets to be set, also cleaning up and
    standardising their implementation.
    
Implementation details:

    See the documentation on the classes, particularly VisDependentProperty,
    for detailed information on how the framework operates. 
    
Examples:

    There are three common scenarios that use VisDependentProperty. The
    following examples assume the Inputs class extends vdp.StandardInputs: 

    
    1. To provide a default value that can be overriden on a per-MS basis. Use
       the optional 'default' argument to VisDependentProperty, eg: 
    
        myarg = VisDependentProperty(default='some value')
        
    2. To provide a value via custom code, executed whenever a user override is
       not provided. Use the @VisDependentProperty decorator on your custom
       function, eg:

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
import pprint
import weakref

from . import api
from . import launcher
from . import logging
from . import utils

__all__ = ['VisDependentProperty',
           'StandardInputs',
           'OnTheFlyCalibrationMixin',
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
        elif isinstance(val, (list)):
            return val
        elif val in self.__NULL_INPUT:
            return self
        else:
            return val

    def __eq__(self, other):
        return isinstance(other, NullMarker)


class VdpArgument(dict):
    def __repr__(self):
        return '<VdpArgument({!s})>'.format(super(VdpArgument, self).__repr__())


class VisDependentProperty(object):
    """
    VisDependentProperty is a Python data descriptor that lets pipeline
    properties handle multiple user arguments, one per measurement set 
    registered in the context.

    VisDependentProperty can be thought of as a dictionary of dictionaries.
    The first dictionary maps the key, an Inputs instance, to a second 
    dictionary, whose keys are all the measurement sets registered in the 
    Context and whose values are the user arguments that were set for that
    measurement set. In this way each measurement set can have a unique 
    argument value.

    When the Inputs class definition is parsed, a VisDependentProperty 
    instance is created for every VisDependentProperty in the class. 
    Subsequently, whenever the Inputs class is instantiated, a new entry is
    created in the first dictionary with a weak reference to the instance used
    as a key. Weak references are used so that temporary Inputs values are not
    kept alive when the Inputs to which it refers is deleted or fall out of
    scope.       
    
    By default, the second dictionary that maps measurement sets to arguments
    for that Inputs instance is filled with NullMarker objects. On reading a
    VisDependentProperty (ie. using the dot prefix: inputs.solint), one of two
    things happens: 
    
    1. If a NullMarker is found in the dictionary - signifying that no user
       input has been provided - and a 'getter' function has been defined, the
       getter function will be called to provide a value for that measurement 
       set.     
    
    or
    
    2. If a user has overridden the value (eg. inputs.solint = 123), that
       value will be returned.
    
    The VisDependentProperty framework processes user input (eg. inputs.solint
    = 123) according to a set of rules:
    
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
    NULL = NullMarker()

    def getter(self, fget):
        """
        Set the function used to get the attribute value when the user has not
        supplied an override value.
        """
        return type(self)(fget, self.fconvert, default=self.default)

    def convert(self, fconvert):
        """
        Set the function used to clean and/or convert user-supplied argument 
        values before they are associated with the instance property.
        
        The provided function should accept one unnamed argument, which when
        passed will be the user input *for this measurement set*. That is,
        after potentially being divided up into per-measurement values. 
        """ 
        return type(self)(self.fget, fconvert, default=self.default)

    def __init__(self, fget=None, fconvert=None, **kwargs):
        self.fget = fget
        self.fconvert = fconvert
        self.default = kwargs.get('default', None)
        self.readonly = kwargs.get('readonly', False)
        self.data = weakref.WeakKeyDictionary()

    def __call__(self, fget, *args, **kwargs):
        # __call__ is executed when decorating a readonly function 
        # LOG.info('In __call__ for %s' % fget.func_name)
        self.fget = fget
        return self

    def __get__(self, inputs, owner):
        # Return the VisDependentProperty itself when called directly (eg. 
        # VisDependentProperty.X) so that VisDependentProperty properties
        # can be accessed.
        if inputs is None:
            return self

        # create, if necessary, an entry for this Inputs in the value cache
        self.initialise_instance(inputs)

        keys = self.get_cache_keys(inputs)

        result = []
        for k, v in [(k, self.data[inputs][k]) for k in keys]:
            if v == VisDependentProperty.NULL:
                if self.default is not None:
                    v = self.default
                else:
                    original_vis = inputs.vis
                    try:
                        inputs.vis = k
                        v = self.fget(inputs)
                    finally:
                        inputs.vis = original_vis
            result.append(v)

        # return single values where possible, which is when only one value
        # is present because the inputs covers one ms or because the values
        # for each ms are all the same.
        if len(result) is 1:
            return result[0]
        else:
            if all_unique(result):
                return result
            else:
                return result[0]

    # def __set__(self, inputs, value):
    #     if self.readonly:
    #         raise AttributeError
    #
    #     self.initialise_instance(inputs)
    #
    #     # wrap non-lists in a list
    #     if not isinstance(value, (list, tuple)):
    #         value = [value]
    #
    #     # convert null equivalents to the null marker object
    #     for i, v in enumerate(value):
    #         value[i] = VisDependentProperty.NULL.convert(v)
    #
    #     # pass non-null values through the user-provided converter
    #     converted = value
    #     for i, v in enumerate(converted):
    #         if self.fconvert is None:
    #             break
    #         if v != VisDependentProperty.NULL:
    #             converted[i] = self.fconvert(inputs, v)
    #
    #     keys = self.get_cache_keys(inputs)
    #
    #     # 1 value, multi vis = assign value to all
    #     if len(converted) is 1:
    #         for k in keys:
    #             self.data[inputs][k] = converted[0]
    #
    #     # n values, n vises = assign individually
    #     elif len(converted) == len(keys):
    #         for k, v in zip(keys, converted):
    #             if inputs not in self.data:
    #                 self.data[inputs] = {}
    #             self.data[inputs][k] = v
    #
    #     # assign list value to one vis
    #     elif len(keys) is 1:
    #         self.data[inputs][keys[0]] = converted
    #
    #     # mismatch between num values and num keys
    #     else:
    #         raise ValueError

    def __set__(self, inputs_instance, user_arg):
        if self.readonly:
            raise AttributeError

        self.initialise_instance(inputs_instance)

        keys = self.get_cache_keys(inputs_instance)

        if isinstance(user_arg, VdpArgument):
            # sanity check: assert that each vis is present in the cache
            assert(frozenset(self.data[inputs_instance].keys()).issuperset(set(user_arg.keys())))

            # convert null equivalent inputs to NULL marker objects
            null_markers = {k: VisDependentProperty.NULL.convert(v) for k, v in user_arg.iteritems()}

            # convert user arguments if a convert function was given
            if self.fconvert is not None:
                for vis, user_arg in null_markers.iteritems():
                    if user_arg != VisDependentProperty.NULL:
                        # set Inputs to a single vis so that MS-dependent
                        # properties can be resolved properly
                        original_vis = inputs_instance.vis
                        try:
                            inputs_instance.vis = vis
                            null_markers[vis] = self.fconvert(inputs_instance, user_arg)
                        finally:
                            inputs_instance.vis = original_vis

            self.data[inputs_instance].update(null_markers)

        else:
            user_arg = VisDependentProperty.NULL.convert(user_arg)

            # set this value for every vis currently in inputs.vis
            for k in keys:
                self.data[inputs_instance][k] = user_arg

    def initialise_instance(self, instance):
        if instance not in self.data:
            self.data[instance] = collections.defaultdict(NullMarker)

    def get_cache_keys(self, inputs):
        """
        Get the cache keys appropriate to the visibilities specified in the
        inputs argument. 
        
        The cache links measurement sets to unique property values. This 
        function defines the keys that should be used to map visibilities to 
        the property values for that visibility.        
        """
        vis = inputs.vis
        if not isinstance(vis, (list, tuple)):
            vis = [vis]

        return vis

#         # MeasurementSet instances themselves are unique, and so are the 
#         # default and safest key values to use. However, ImportData operates 
#         # before any MeasurementSet instances have been created, so as an 
#         # alternative we use the vis strings themselves.
#         if inputs.__class__.__name__ == 'ImportDataInputs':
#             return vis        
#          
#         return [inputs.context.observing_run.get_ms(v) for v in vis]
    


class StandardInputs(api.Inputs):
    #- standard non-vis-dependent properties ---------------------------------

    def __deepcopy__(self, memo):
        return selective_deepcopy(self, memo, shallow_copy=('_context',))

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
        if isinstance(value, list) and len(value) is 1:
            value = value[0]
        self._vis = value

    #- vis-dependent properties ----------------------------------------------

    @VisDependentProperty(readonly=True)
    def ms(self):
        """
        Return the MeasurementSet for the current value of vis.
     
        :rtype: :class:`~pipeline.domain.MeasurementSet`
        """
        return self.context.observing_run.get_ms(self.vis)

    @VisDependentProperty
    def output_dir(self):
        return self.context.output_dir

    #- code for getting and setting pickle state -----------------------------

    def __getstate__(self):
        # The state for each VisDependentProperty is held outside the Inputs
        # instance, inside the VisDependentProperty instance itself. The 
        # default __getstate__ implementation returns just __dict__, which would omit 
        # the VisDependentProperty. The pickled state
        # is extended to return not just this instance's __dict__ but also a
        # dictionary of the VisDependentProperty states extracted from the
        # external VisDependentProperty instance.  
        return self.__dict__, get_state(self)

    def __setstate__(self, state):
        obj_state, vdp_state = state
        self.__dict__.update(obj_state)
        set_state(self, vdp_state)

    #- utility functions added to base class ---------------------------------

    # def _init_properties(self, properties=None, kw_ignore=None):
    #     """
    #     Set the instance properties using a dictionary of keyword/value pairs.
    #     Properties named in kw_ignore will not be set.
    #     """
    #     if properties is None:
    #         properties = {}
    #     if kw_ignore is None:
    #         kw_ignore = []
    #     if 'self' not in kw_ignore:
    #         kw_ignore.append('self')
    #
    #     # set the value of each parameter to that given in the input arguments
    #     # set context first as other properties may depend on it.
    #     self.context = properties['context']
    #
    #     if 'vis' in properties:
    #         self.vis = properties['vis']
    #         del properties['vis']
    #
    #     for k, v in properties.items():
    #         if k not in kw_ignore:
    #             LOG.trace('%s: setting %s to %s' % (self.__class__.__name__, k, v))
    #             try:
    #                 setattr(self, k, v)
    #             except AttributeError:
    #                 LOG.warning('Trying to set read-only variable: {!s} = {!r}'.format(k, v))

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

    def to_casa_args(self, ignore=None):
        """
        Express these inputs as a dictionary of CASA arguments. The values
        in the dictionary are in a format suitable for CASA and can be 
        directly passed to the CASA task.
        
        :rtype: a dictionary of string/??? kw/val pairs
        """        
        args = self._get_task_args(ignore)

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

        for k,v in args.items():
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
    _modes = {}
    
    def __init__(self, context, mode=None, **parameters):
        # create a dictionary of Inputs objects, one of each type
        self._delegates = {k: task_cls.Inputs(context) for k, task_cls in self._modes.items()}

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

    def __getstate__(self):
        return self.__dict__, get_state(self)

    def __setstate__(self, state):
        obj_state, vdp_state = state
        self.__dict__.update(obj_state)
        set_state(self, vdp_state)


# functions for getting and setting pickle state -----------------------------


def get_state(instance):
    """
    Get the state of VisDependentProperties in the instance as a list of
    (name,value) tuples.
    """
    return [(name, vdp.data[instance])
            for name, vdp in get_vis_dependent_properties(instance)
            if instance in vdp.data]

    # state = []
    # for name, vdp in get_vis_dependent_properties(instance):
    #     if instance in vdp.data:
    #         state.append((name, vdp.data[instance]))
    #
    # return state


def set_state(instance, state):
    """
    Set the state for VisDependentProperties.
    """
    cls = instance.__class__
    for name, val in state:
        getattr(cls, name).data[instance] = val

# module utility functions ---------------------------------------------------


def get_properties_of_type(instance, t):
    """
    Get properties of this instance that are of the given type.
        
    Returns (property name, type instance) tuples.
    """
    cls = instance.__class__
    return [(name, getattr(cls, name)) for name in dir(cls)
            if isinstance(getattr(cls, name), t)]


def get_vis_dependent_properties(instance):
    """
    Get all VisDependentProperty properties of this instance.
    
    Remember that the returned VisDependentProperties will contain the state for
    other instances too!
        
    Returns (property name, type instance) tuples.
    """
    return get_properties_of_type(instance, VisDependentProperty)


def all_unique(o):
    """
    Return True if all elements in the iterable argument are unique.
    """
    # LOG.trace('all_unique(%s)' % str(o))
    if not isinstance(o, collections.Iterable):
        raise ValueError('Cannot determine uniqueness of non-iterables')

    hashes = [gen_hash(e) for e in o]
    return len(hashes) is len(set(hashes))


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


def selective_deepcopy(instance, memo, shallow_copy=None):
    """

    :param instance:
    :param memo:
    :param shallow_copy:
    :return:
    """
    if shallow_copy is None:
        shallow_copy = []

    # extract the state from the instance to deep copy
    obj_state = instance.__dict__
    vdp_state = get_state(instance)

    # create a new instance of the same class
    cls = instance.__class__
    result = cls.__new__(cls)
    memo[id(instance)] = result

    # set shallow copies for the requested attributes by directly copying
    # across the reference from the old instance
    for attr_name in shallow_copy:
        setattr(result, attr_name, getattr(instance, attr_name))
    # assign deep-copied values for the remaining attributes
    for k, v in obj_state.iteritems():
        if k not in shallow_copy:
            setattr(result, k, copy.deepcopy(v, memo))

    # set the VDP state, noting that this state too needs to be deep-copied to
    # prevent shared references between the two objects
    set_state(result, copy.deepcopy(vdp_state))

    return result
