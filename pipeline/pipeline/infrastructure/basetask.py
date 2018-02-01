from __future__ import absolute_import

import abc
import collections
import datetime
import inspect
import os
import pprint
import re
import textwrap
import types
import uuid

from . import api
from . import casatools
from . import filenamer
from . import jobrequest
from . import launcher
from . import logging
from . import pipelineqa
from . import project
from . import task_registry
from . import utils
from . import vdp

try:
    import cPickle as pickle
except:
    import pickle
try:
    import cStringIO as StringIO
except:
    import StringIO

LOG = logging.get_logger(__name__)

# control generation of the weblog
DISABLE_WEBLOG = False
VISLIST_RESET_KEY = '_do_not_reset_vislist'


def timestamp(method):
    def attach_timestamp_to_results(self, *args, **kw):
        start = datetime.datetime.utcnow()
        result = method(self, *args, **kw)
        end = datetime.datetime.utcnow()

        if result is not None:
            result.timestamps = Timestamps(start, end)

        return result

    return attach_timestamp_to_results


def result_finaliser(method):
    """
    Copy some useful properties to the Results object before returning it.
    This is used in conjunction with execute(), where the Result could be
    returned from a number of places but we don't want to set the properties
    in each location.
    
    TODO: refactor so this is done as part of execute!  
    """
    def finalise_pipeline_result(self, *args, **kw):
        result = method(self, *args, **kw)

        if isinstance(result, ResultsList) and len(result) == 0:
            return result

        elif result is not None:
            inputs = self.inputs
            result.inputs = inputs.as_dict()
            result.stage_number = inputs.context.task_counter
            try:
                result.pipeline_casa_task = inputs._pipeline_casa_task
            except AttributeError:
                # sub-tasks may not have pipeline_casa_task, but we only need
                # it for the top-level task
                pass
        return result

    return finalise_pipeline_result


def capture_log(method):
    def capture(self, *args, **kw):
        # get the size of the CASA log before task execution
        logfile = casatools.log.logfile()
        size_before = os.path.getsize(logfile)

        # execute the task
        result = method(self, *args, **kw)
        
        # copy the CASA log entries written since task execution to the result 
        with open(logfile, 'r') as casalog:
            casalog.seek(size_before)
            
            # sometimes we can't write properties, such as for flagdeteralma
            # when the result is a dict
            try:
                result.casalog = casalog.read()
            except:
                LOG.trace('Could not set casalog property on result of type '
                          '%s' % result.__class__)

        # To save space in the pickle, delete any inner CASA logs. The web
        # log will only write the outer CASA log to disk
        if isinstance(result, collections.Iterable):
            for r in result:
                if hasattr(r, 'casalog'):
                    del r.casalog

        return result
    return capture


class MandatoryInputsMixin(object):
    @property
    def context(self):
        # A product of the prepare/analyse refactoring is that we always need
        # a context. This message is to ensure it was set by the implementing
        # subclass. We could remove this once refactoring is complete, but it
        # should be a cheap comparison so we leave it in.
        if not isinstance(self._context, launcher.Context):
            msg = (self.__class__.__name__ + ' did not set the pipeline '
                   'context')
            raise TypeError, msg  
        return self._context
        
    @context.setter
    def context(self, value):
        if not isinstance(value, launcher.Context):
            msg = ('context must be a pipeline context. Got ' + 
                   value.__class__.__name__ + '.')
            raise TypeError, msg  
        self._context = value
    
    @property
    def ms(self):
        """
        Return the MeasurementSet for the current value of vis.
     
        If vis is a list, a list of MeasurementSets will be returned.
        
        :rtype: :class:`~pipeline.domain.MeasurementSet`
        """
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('ms')
        try:
            return self.context.observing_run.get_ms(name=self.vis)
        except KeyError as e:
            LOG.warn('No measurement set found for {!s}'.format(self.vis))
            return None

    @property
    def vis(self):
        """
        Return the filenames of the measurement sets on which this task should
        operate.
        
        If vis is not set, this defaults to all measurement sets registered
        with the context.
        """ 
        if self._vis is not None:
            return self._vis

        imaging_preferred = get_imaging_preferred(self.__class__)
        return [ms.name for ms in self.context.observing_run.get_measurement_sets(imaging_preferred=imaging_preferred)]

    @vis.setter    
    def vis(self, value):
        if value is None:
            imaging_preferred = get_imaging_preferred(self.__class__)
            vislist = [ms.name for ms in
                       self.context.observing_run.get_measurement_sets(imaging_preferred=imaging_preferred)]
        else:
            vislist = value if type(value) is types.ListType else [value, ]

            # check that the context holds each vis specified by the user
            for vis in vislist:
                # get_ms throws a KeyError if the ms is not in the context 
                self.context.observing_run.get_ms(name=vis)

        # VISLIST_RESET_KEY is present when vis is set by handle_multivis.
        # In this case we do not want to reset my_vislist, as handle_multivis is
        # setting vis to the individual measurement sets
        if not hasattr(self, VISLIST_RESET_KEY):
            LOG.trace('Setting Inputs._my_vislist to %s' % vislist)
            self._my_vislist = vislist
        else:
            LOG.trace('Leaving Inputs._my_vislist at current value of %s' 
                      % self._my_vislist)

        self._vis = value

    @property
    def output_dir(self):
        if self._output_dir is None:
            return self.context.output_dir
        return self._output_dir        

    @output_dir.setter
    def output_dir(self, value):
        self._output_dir = value


def get_imaging_preferred(inputs):
    return issubclass(inputs, api.ImagingMeasurementSetsPreferred)


class OnTheFlyCalibrationMixin(object):
    """
    OnTheFlyCalibrationMixin provides a shared implementation for on-the-fly
    calibration parameters (gaintable, spwmap, gaincurve, etc.) required by
    some pipeline inputs.
    
    As a mixin, this class is not intended to be instantiated; rather, it is
    inherited by an Inputs that specifies on-the-fly calibration parameters.
    Getting and setting any of these on-the-fly parameters will then use the
    shared functionality defined here. 
    """
    @property
    def opacity(self):
        return self._opacity
    
    @opacity.setter
    def opacity(self, value):
        self._opacity = value


class StandardInputs(api.Inputs, MandatoryInputsMixin):
    """
    StandardInputsTemplate is the standard base class for task Inputs classes.
    
    StandardInputs provides a standard implementation for the the api.Inputs
    interface. Inputs class with a
    common implementation pattern and some common functionality. While it 
    demands that subclasses implement some additional methods, in return it
    allows tasks to manipulate these Inputs objects more easily.
    """
    __metaclass__ = abc.ABCMeta

    def __init__(self, context, vis=None, output_dir=None):
        super(StandardInputs, self).__init__()

        # set MandatoryInputs properties
        self.context = context
        self.vis = vis
        self.output_dir = output_dir

    def _init_properties(self, properties=None, kw_ignore=None):
        """
        Set the instance properties using a dictionary of keyword/value pairs.
        Properties named in kw_ignore will not be set.
        """
        if properties is None:
            properties = {}
        if kw_ignore is None:
            kw_ignore = []
        kw_ignore.append('self')

        # set the value of each parameter to that given in the input arguments
        # force context to be set first as some of the others depend on it.
        setattr(self, 'context', properties['context'])
        for k, v in properties.items():
            if k not in kw_ignore:
                try:
                    setattr(self, k, v)
                except AttributeError:
                    # AttributeError is raised when attempting to set value of
                    # read-only properties
                    pass

    def _get_task_args(self, ignore=()):
        """
        Express this class as a dictionary of CASA arguments, listing all
        inputs except those named in ignore. 
        
        The main purpose of the ignore argument is used to prevent an infinite
        loop in :meth:`~CommonCalibrationInputs.caltable`, which determines the
        value of caltable based on the value of the other CASA task arguments.
        """
        # get the signature of this Inputs class. We want to return a 
        # of dictionary of all kw argument names except self, the 
        # pipeline-specific arguments (context, output_dir, etc.) and
        # caltable.
        skip = ['self', 'context', 'output_dir', 'ms', 'calto', 'calstate']
        skip.extend(ignore)
        kw_names = [a for a in inspect.getargspec(self.__init__).args
                    if a not in skip]
        d = {}
        for key in kw_names:
            d[key] = getattr(self, key)

        # add any read-only properties too
        for k, v in inspect.getmembers(self.__class__, inspect.isdatadescriptor):
            if k in d or k.startswith('_') or k in skip:
                continue
            try:
                d[k] = v.fget(self)
            except:
                LOG.debug('Could not get input property %s' % k)
        
        return d

    def _handle_multiple_vis(self, property_name):
        """
        Return a list of property values, one for each measurement set given
        in the Inputs.
                
        Some Inputs properties are ms-dependent. This utility function is used
        when multiple measurement sets are specified in the inputs. A list
        will be returned containing a list of property values, one for each
        measurement set specified in the inputs.
        """
        original = self.vis
        # tell vis not to reset my_vislist when setting vis to the individual
        # measurement sets
        LOG.trace('setting VISLIST_RESET_KEY for _handle_multiple_vis(' + property_name + ')')
        setattr(self, VISLIST_RESET_KEY, True)
        try:
            result = []
            for vis in original: 
                self.vis = vis
                result.append(getattr(self, property_name))
            return result
        finally:
            self.vis = original
            LOG.trace('Deleting VISLIST_RESET_KEY after _handle_multiple_vis(' + property_name + ')')
            delattr(self, VISLIST_RESET_KEY)
    
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
        self._delegates = {}
        for k, task_cls in self._modes.items():
            self._delegates[k] = task_cls.Inputs(context)

        # set the mode to the requested mode, thus setting the active Inputs
        self.mode = mode
        
        # set any parameters provided by the user
        for k, v in parameters.items():
            setattr(self, k, v)

    def _handle_multiple_vis(self, property_name):
        """
        Return a list of property values, one for each measurement set given
        in the Inputs.
                
        Some Inputs properties are ms-dependent. This utility function is used
        when multiple measurement sets are specified in the inputs. A list
        will be returned containing a list of property values, one for each
        measurement set specified in the inputs.
        """
        # _handle_multiple_vis must be present on the ModeInputs to allow 
        # multi-vis properties on the top-level ModeInputs class. Failure to do
        # so results in delegation to active._handle_multiple_vis for the
        # multi-vis property, which then raises an AttributeError as the
        # named top-level attribute does not exist in the lower-level delegate
        # class.  
        original = self.vis
        try:
            result = []
            for vis in original: 
                self.vis = vis
                result.append(getattr(self, property_name))
            return result
        finally:
            self.vis = original

    def __getattr__(self, name):
        # __getattr__ is only called when this object's __dict__ does not
        # contain the requested property. When this happens, we delegate to
        # the currently selected Inputs. First, however, we check whether 
        # the requested property is one of the Python magic methods. If so, 
        # we return the standard implementation. This is necessary for
        # pickle, which checks for __getnewargs__ and __getstate__.
        if name.startswith('__') and name.endswith('__'):
            return super(ModeInputs, self).__getattr__(name)

        LOG.trace('getattr delegating to %s for attribute \'%s\''
                  '' % (self._active.__class__.__name__, name))
        return getattr(self._active, name)

    def __setattr__(self, name, val):
        # If the property we're trying to set is one of this base class's
        # private variables, add it to our __dict__ using the standard
        # __setattr__ method
        if name in ('_active', '_delegates', '_pipeline_casa_task', 
                    VISLIST_RESET_KEY):
            LOG.trace('Setting \'{0}\' attribute to \'{1}\' on \'{2}'
                      '\' object'.format(name, val, self.__class__.__name__))
            return super(ModeInputs, self).__setattr__(name, val)

        # check whether this class has a getter/setter by this name. If so,
        # allow the write to __dict__
        for (fn_name, _) in inspect.getmembers(self.__class__, 
                                               inspect.isdatadescriptor):
            # our convention is to prefix the data variable for a 
            # getter/setter with an underscore. 
            if name in (fn_name, '_' + fn_name):
                LOG.trace('Getter/setter found on {0}. Setting \'{1}\' '
                          'attribute to \'{2}\''.format(self.__class__.__name__,
                                                        name, val))
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
                LOG.trace(
                    'Setting \'{0}\' attribute to \'{1}\' on \'{2}'
                    '\' object'.format(name, val, d.__class__.__name__))
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
        
    def to_casa_args(self):
        return self._active.to_casa_args()
    
    def as_dict(self):
        props = utils.collect_properties(self._active)
        props.update(utils.collect_properties(self))
        return props  
    
    def __repr__(self):
        # get the arguments for this class's contructor
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


class ModeTask(api.Task):
    def __init__(self, inputs):
        super(ModeTask, self).__init__()

        # complain if we were given the wrong type of inputs
        if not isinstance(inputs, self.Inputs):
            msg = '{0} requires inputs of type {1} but got {2}.'.format(
                self.__class__.__name__,
                self.Inputs.__name__,
                inputs.__class__.__name__)
            raise TypeError(msg)

        self.inputs = inputs
        self._delegate = inputs.get_task()

    def execute(self, dry_run=True, **parameters):
        self._check_delegate()
        return self._delegate.execute(dry_run, **parameters)
    
    def __getattr__(self, name):
        self._check_delegate()
        return getattr(self._delegate, name)

    def _check_delegate(self):
        """
        Update, if necessary, the delegate task so that it matches the
        mode specified in the Inputs.
        
        This function is necessary as the value of Inputs.mode can change 
        after the task has been constructed. Therefore, we cannot rely on any 
        delegate set at construction time. Instead, the delegate must be
        updated on every execution. 
        """
        # given two modes, A and B, it's possible that A is a subclass of B,
        # eg. PhcorChannelBandpass extends ChannelBandpass, therefore we
        # cannot test whether the current instance is the correct type using
        # isinstance. Instead, we need to compare class names.        
        mode = self.inputs.mode
        mode_cls = self.inputs._modes[mode]
        mode_cls_name = mode_cls.__name__
                
        delegate_cls_name = self._delegate.__class__.__name__
                
        if mode_cls_name != delegate_cls_name:
            self._delegate = self.inputs.get_task()


# A simple named tuple to hold the start and end timestamps 
Timestamps = collections.namedtuple('Timestamps', ['start', 'end'])


class Results(api.Results):
    """
    Results is the base implementation of the Results API. 
    
    In practice, all results objects should subclass this object to take 
    advantage of the shared functionality.
    """    
    def __init__(self):
        super(Results, self).__init__()
        
        # set the value used to uniquely identify this object. This value will
        # be used to determine whether this results has already been merged 
        # with the context 
        self._uuid = uuid.uuid4()

        # property used to hold pipeline QA values
        self.qa = pipelineqa.QAScorePool()

        # property used to hold metadata and presentation-focused values
        # destined for the weblog. Currently a dict, but could change.
        self._metadata = {}

    @property
    def uuid(self):
        """
        The unique identifier for this results object.
        """
        return self._uuid

    @property
    def metadata(self):
        """
        Object holding presentation-related values destined for the web log
        """
        return self._metadata

    def merge_with_context(self, context):
        """
        Merge these results with the given context.
        
        This method will be called during the execution of accept(). For 
        calibration tasks, a typical implementation will register caltables
        with the pipeline callibrary.  

        At this point the result is deemed safe to merge, so no further checks
        on the context need be performed. 
                
        :param context: the target
            :class:`~pipeline.infrastructure.launcher.Context`
        :type context: :class:`~pipeline.infrastructure.launcher.Context`
        """
        LOG.debug('Null implementation of merge_with_context used for %s'
                  '' % self.__class__.__name__)

    def accept(self, context=None):
        """
        Accept these results, registering objects with the context and incrementing
        stage counters as necessary in preparation for the next task.
        """
        if context is None:
            # context will be none when called from a CASA interactive 
            # session. When this happens, we need to locate the global context
            # from the stack
            import pipeline.h.cli.utils
            context = pipeline.h.cli.utils.get_context()

        self._check_for_remerge(context)

        # execute our template function
        self.merge_with_context(context)

        # find whether this result is being accepted as part of a task 
        # execution or whether it's being accepted after task completion
        task_completed = utils.task_depth() == 0

        # perform QA if accepting this result from a top-level task
        if task_completed:
            pipelineqa.registry.do_qa(context, self)

        if task_completed:
            # If accept() is called at the end of a task, create a proxy for
            # this result and pickle it to the appropriate weblog stage 
            # directory. This keeps the context size at a minimum.
            proxy = ResultsProxy(context)
            proxy.write(self)
            result = proxy
        else:
            result = self
    
        # with no exceptions thrown by this point, we can safely add this 
        # results object to the results list
        context.results.append(result)

        # having called the old constructor, we know that self.context is set.
        # Use this context to find the report directory and write to the log
        if task_completed:
            # this needs to come before web log generation as the filesizes of
            # various logs and scripts are calculated during web log generation
            write_pipeline_casa_tasks(context)

        # generate weblog if accepting a result from outside a task execution
        if task_completed and not DISABLE_WEBLOG:
            # cannot import at initial import time due to cyclic dependency
            import pipeline.infrastructure.renderer.htmlrenderer as htmlrenderer
            htmlrenderer.WebLogGenerator.render(context)

        if task_completed and LOG.isEnabledFor(logging.DEBUG):
            basename = 'context-stage%s.pickle' % self.stage_number
            path = os.path.join(context.output_dir,
                                context.name,
                                'saved_state',
                                basename)

            utils.mkdir_p(os.path.dirname(path))
            with open(path, 'wb') as outfile:
                pickle.dump(result, outfile, -1)


    def _check_for_remerge(self, context):
        """
        Check whether this result has already been added to the given context. 
        """
        # context.results contains the list of results that have been merged 
        # with the context. Check whether the UUID of any result or sub-result
        # in that list matches the UUID of this result.
        for result in context.results:
            if self._is_uuid_in_result(result):    
                msg = 'This result has already been added to the context'
                LOG.error(msg)
                raise ValueError(msg)
        
    def _is_uuid_in_result(self, result):
        """
        Return True if the UUID of this result matches the UUID of the given
        result or any sub-result contained within. 
        """
        for subtask_result in getattr(result, 'subtask_results', ()):
            if self._is_uuid_in_result(subtask_result):
                return True

        if result.uuid == self.uuid:
            return True
        
        return False


class ResultsProxy(object):
    def __init__(self, context):
        self._context = context

    def write(self, result):
        """
        Write the pickled result to disk.
        """
        # adopt the result's UUID protecting against repeated addition to the
        # context
        self.uuid = result.uuid
        
        self._write_stage_logs(result)

        # only store the basename to allow for relocation between save and
        # restore
        self._basename = 'result-stage%s.pickle' % result.stage_number        
        path = os.path.join(self._context.output_dir,
                            self._context.name,
                            'saved_state',
                            self._basename)
        
        utils.mkdir_p(os.path.dirname(path))        
        with open(path, 'wb') as outfile:
            pickle.dump(result, outfile, -1)

    def read(self):
        """
        Read the pickle from disk, returning the unpickled object.
        """
        path = os.path.join(self._context.output_dir,
                            self._context.name, 
                            'saved_state',
                            self._basename)
        with open(path) as infile:
            return utils.pickle_load(infile)

    def _write_stage_logs(self, result):
        """
        Take the CASA log snippets attached to each result and write them to
        the appropriate weblog directory. The log snippet is deleted from the
        result after a successful write to keep the pickle size down. 
        """
        if not hasattr(result, 'casalog'):
            return

        stage_dir = os.path.join(self._context.report_dir,
                                 'stage%s' % result.stage_number)
        if not os.path.exists(stage_dir):                
            os.makedirs(stage_dir)

        stagelog_entries = result.casalog
        start = result.timestamps.start
        end = result.timestamps.end

        stagelog_path = os.path.join(stage_dir, 'casapy.log')
        with open(stagelog_path, 'w') as stagelog:
            LOG.debug('Writing CASA log entries for stage %s (%s -> %s)' %
                      (result.stage_number, start, end))                          
            stagelog.write(stagelog_entries)
                
        # having written the log entries, the CASA log entries have no 
        # further use. Remove them to keep the size of the pickle small
        delattr(result, 'casalog')


class ResultsList(Results):
    def __init__(self, results=None):
        super(ResultsList, self).__init__()
        self.__results = []
        if results:
            self.__results.extend(results)

    def __getitem__(self, item):
        return self.__results[item]

    def __iter__(self):
        return self.__results.__iter__()

    def __len__(self):
        return len(self.__results)

    def __str__(self):
        return 'ResultsList({!s})'.format(str(self.__results))

    def __repr__(self):
        return 'ResultsList({!s})'.format(repr(self.__results))

    def append(self, other):
        self.__results.append(other)

    def accept(self, context=None):
        return super(ResultsList, self).accept(context)

    def extend(self, other):
        for o in other:
            self.append(o)

    def merge_with_context(self, context):
        for result in self.__results:
            result.merge_with_context(context)


class StandardTaskTemplate(api.Task):
    """
    StandardTaskTemplate is a template class for pipeline reduction tasks whose 
    execution can be described by a common four-step process:

    #. prepare(): examine the measurement set and prepare a list of\
        intermediate job requests to be executed.
    #. execute the jobs
    #. analyze(): analyze the output of the intermediate job requests,\
        deciding if necessary which parameters provide the best results, and\
        return these results.
    #. return a final list of jobs to be executed using these best-fit\
        parameters.

    Simpletask implements the :class:`Task` interface and steps 2 and 4 in the
    above process, leaving subclasses to implement
    :func:`~SimpleTask.prepare` and :func:`~SimpleTask.analyse`.    


    A Task and its :class:`Inputs` are closely aligned. It is anticipated that 
    the Inputs for a Task will be created using the :attr:`Task.Inputs`
    reference rather than locating and instantiating the partner class
    directly, eg.::

        i = ImplementingTask.Inputs.create_from_context(context)


    """
    __metaclass__ = abc.ABCMeta

    # HeadTail is an internal class used to associate properties with their
    # associated measurement sets
    HeadTail = collections.namedtuple('HeadTail', ('head', 'tail'))

    def __init__(self, inputs):
        """
        Create a new Task with an initial state based on the given inputs.

        :param Inputs inputs: inputs required for this Task.
        """
        super(StandardTaskTemplate, self).__init__()

        # complain if we were given the wrong type of inputs
        if isinstance(inputs, vdp.InputsContainer):
            error = (inputs._task_cls.Inputs != self.Inputs)
        else:
            error = not isinstance(inputs, self.Inputs)

        if error:
            msg = '{0} requires inputs of type {1} but got {2}.'.format(
                self.__class__.__name__,
                self.Inputs.__name__,
                inputs.__class__.__name__)
            raise TypeError(msg)

        self.inputs = inputs

    is_multi_vis_task = False

    @abc.abstractmethod
    def prepare(self, **parameters):
        """
        Prepare job requests for execution.

        :param parameters: the parameters to pass through to the subclass.
            Refer to the implementing subclass for specific information on
            what these parameters are.            
        :rtype: a class implementing :class:`~pipeline.api.Result`        
        """
        raise NotImplementedError

    @abc.abstractmethod
    def analyse(self, result):
        """
        Determine the best parameters by analysing the given jobs before
        returning any final jobs to execute.

        :param jobs: the job requests generated by :func:`~SimpleTask.prepare`
        :type jobs: a list of\ 
            :class:`~pipeline.infrastructure.jobrequest.JobRequest`
        :rtype: \
            :class:`~pipeline.api.Result`
        """
        raise NotImplementedError

    @timestamp
    @capture_log
    @result_finaliser
    def execute(self, dry_run=True, **parameters):
        # The filenamer deletes any identically named file when constructing
        # the filename, which is desired when really executing a task but not
        # when performing a dry run. This line disables the 
        # 'delete-on-generate' behaviour.
        filenamer.NamingTemplate.dry_run = dry_run

        if utils.is_top_level_task():
            # Set the task name, but only if this is a top-level task. This 
            # name will be prepended to every data product name as a sign of
            # their origin
            try:
                name = task_registry.get_casa_task(self.__class__)
            except KeyError:
                name = self.__class__.__name__
            filenamer.NamingTemplate.task = name
            
            # initialise the subtask counter, which will be subsequently 
            # incremented for every execute within this top-level task  
            self.inputs.context.task_counter += 1
            LOG.info('Starting execution for stage %s', 
                     self.inputs.context.task_counter)
            self.inputs.context.subtask_counter = 0

            # log the invoked pipeline task and its comment to 
            # casa_commands.log
            _log_task(self, dry_run)

        else:
            self.inputs.context.subtask_counter += 1

        # Create a copy of the inputs - including the context - and attach
        # this copy to the Inputs. Tasks can then merge results with this
        # duplicate context at will, as we'll later restore the originals.
        original_inputs = self.inputs
        self.inputs = utils.pickle_copy(original_inputs)

        # create a job executor that tasks can use to execute subtasks
        self._executor = Executor(self.inputs.context, dry_run)

        # create a new log handler that will capture all messages above
        # WARNING level.
        handler = logging.CapturingHandler(logging.WARNING)

        try:
            # if this task does not handle multiple input mses but was 
            # invoked with multiple mses in its inputs, call our utility
            # function to invoke the task once per ms.
            if not self.is_multi_vis_task:
                if isinstance(self.inputs, vdp.InputsContainer) or isinstance(self.inputs.vis, list):
                    return self._handle_multiple_vis(dry_run, **parameters)

            # We should not pass unused parameters to prepare(), so first
            # inspect the signature to find the names the arguments and then
            # create a dictionary containing only those parameters
            prepare_args = set(inspect.getargspec(self.prepare).args)
            prepare_parameters = dict(parameters)
            for arg in parameters:
                if arg not in prepare_args:
                    del prepare_parameters[arg]

            # register the capturing log handler, buffering all messages so that
            # we can add them to the result - and subsequently, the weblog
            logging.add_handler(handler)

            # get our result
            result = self.prepare(**prepare_parameters)

            # analyse them..
            result = self.analyse(result)

            # tag the result with the class of the originating task
            result.task = self.__class__

            # add the log records to the result
            result.logrecords = handler.buffer[:]

            return result

        finally:
            # restore the context to the original context
            self.inputs = original_inputs

            # now the task has completed, we tell the namer not to delete again
            filenamer.NamingTemplate.dry_run = True

            # delete the task name once the top-level task is complete
            if utils.is_top_level_task():
                filenamer.NamingTemplate.task = None

            # now that the WARNING and above messages have been attached to the
            # result, remove the capturing logging handler from all loggers
            if handler:
                logging.remove_handler(handler)

            # delete the executor so that the pickled context can be released
            self._executor = None

    def _handle_multiple_vis(self, dry_run, **parameters):
        """
        Handle a single task invoked for multiple measurement sets.

        This function handles the case when the vis parameter on the Inputs
        specifies multiple measurement sets. In this situation, we want to 
        invoke the task for each individual MS. We could do this by having
        each task iterate over the measurement sets involved, but in order to
        keep the task implementations as simple as possible, that complexity
        (unless overridden) is handled by the template task instead.

        If the task wants to handle the multiple measurement sets
        itself it should override is_multi_vis_task.
        """
        # The following code loops through the MSes specified in vis, 
        # executing the task for the first value (head) and then appending the
        # results of executing the remainder of the MS list (tail).
        if len(self.inputs.vis) is 0:
            # we don't return an empty list as the timestamp decorator wants
            # to set attributes on this value, which it can't on a built-in 
            # list
            return ResultsList()

        if isinstance(self.inputs, (StandardInputs, ModeInputs)):
            to_split = ('calphasetable', 'targetphasetable', 'offsetstable')
            split_properties = self._get_handled_headtails(to_split)

            for name, ht in split_properties.items():
                setattr(self.inputs, name, ht.head)

            refant_tail = None
            if hasattr(self.inputs, 'refant'):
                if type(self.inputs.refant) is types.ListType and self.inputs.refant:
                    refant_head = self.inputs.refant[0]
                    refant_tail = self.inputs.refant[1:]
                    self.inputs.refant = refant_head

            head = self.inputs.vis[0]
            tail = self.inputs.vis[1:]
            try:
                LOG.trace('Setting VISLIST_RESET_KEY prior to task execution')
                setattr(self.inputs, VISLIST_RESET_KEY, True)
                self.inputs.vis = head
                results = ResultsList()
                results.append(self.execute(dry_run=dry_run, **parameters))

                self.inputs.vis = tail
            finally:
                LOG.trace('Deleting VISLIST_RESET_KEY after task execution')
                delattr(self.inputs, VISLIST_RESET_KEY)

            for name, ht in split_properties.items():
                setattr(self.inputs, name, ht.tail)

            if hasattr(self.inputs, 'refant') and refant_tail is not None:
                self.inputs.refant = refant_tail

            results.extend(self.execute(dry_run=dry_run, **parameters))

        elif isinstance(self.inputs, vdp.InputsContainer):
            container = self.inputs
            LOG.info('Equivalent CASA call: %s', container._pipeline_casa_task)

            results = ResultsList()
            try:
                for inputs in container:
                    self.inputs = inputs
                    single_result = self.execute(dry_run=dry_run, **parameters)

                    if isinstance(single_result, ResultsList):
                        results.extend(single_result)
                    else:
                        results.append(single_result)
                return results
            finally:
                self.inputs = container

        return results

    def _get_handled_headtails(self, names=None):
        handled = collections.OrderedDict()

        if names is None:
            # no names to get so return empty dict
            return handled

        for name in names:
            if hasattr(self.inputs, name):
                property_value = getattr(self.inputs, name)            
                
                head = property_value[0]
                tail = property_value[1:]
                ht = StandardTaskTemplate.HeadTail(head=head, tail=tail)

                handled[name] = ht

        return handled


class Executor(object):
    def __init__(self, context, dry_run=True):
        self._dry_run = dry_run
        self._context = context
        self._cmdfile = os.path.join(context.report_dir, 
                                     context.logs['casa_commands'])

    @capture_log
    def execute(self, job, merge=False, **kwargs):
        """
        Execute the given job or subtask, returning its output.
        
        :param job: a job or subtask
        :type job: an object conforming to the :class:`~pipeline.api.Task`\
            interface

        :rtype: :class:`~pipeline.api.Result`
        """
        # execute the job, capturing its results object                
        result = job.execute(dry_run=self._dry_run, *kwargs)

        if self._dry_run:
            return result

        # if the job was a JobRequest, log it to our command log
        if isinstance(job, jobrequest.JobRequest):
            self._log_jobrequest(job)    

        # if requested, merge the result with the context. No type checking
        # here.
        if merge and not self._dry_run:
            result.accept(self._context)

        return result

    def _log_jobrequest(self, job):
        # CAS-5262: casa_commands.log written by the pipeline should
        # be formatted to be more easily readable.

        # replace the working directory with ''..
        job_str = re.sub('%s/' % self._context.output_dir, '',
                         str(job))

        # wrap the text at the first open bracket
        if '(' in job_str:
            indent = (1+job_str.index('(')) * ' '
        else:
            indent = 10 * ' '

        wrapped = textwrap.wrap(job_str,
                                subsequent_indent=indent,
                                width=80,
                                break_long_words=False)

        with open(self._cmdfile, 'a') as cmdfile:
            cmdfile.write('%s\n' % '\n'.join(wrapped))


def _log_task(task, dry_run):
    if dry_run:
        return
    
    context = task.inputs.context
    filename = os.path.join(context.report_dir, 
                            context.logs['casa_commands'])
    comment = ''
    
    if not os.path.exists(filename):
        wrapped = textwrap.wrap('# ' + _CASA_COMMANDS_PROLOGUE,
                                subsequent_indent='# ',
                                width=78,
                                break_long_words=False)
        comment = ('raise Error(\'The casa commands log is not executable!\')\n'
                   '\n%s\n' % '\n'.join(wrapped))

    comment += '\n# %s\n#\n' % getattr(task.inputs, '_pipeline_casa_task', 'unknown pipeline task')

    # get the description of how this task functions and add it to the comment
    comment += task_registry.get_comment(task.__class__)
                        
    with open(filename, 'a') as cmdfile:
        cmdfile.write(comment)


def property_with_default(name, default, doc=None):
    """
    Return a property whose value is reset to a default value when setting the
    property value to None.
    """
    # our standard name for the private property backing the public property 
    # is a prefix of one underscore
    varname = '_' + name 

    def getx(self):
        return object.__getattribute__(self, varname)

    def setx(self, value):
        if value is None:
            value = default
        object.__setattr__(self, varname, value)
#    def delx(self):
#        object.__delattr__(self, varname)
    return property(getx, setx, None, doc)


def write_pipeline_casa_tasks(context):
    """
    Write the equivalent pipeline CASA tasks for results in the context to a
    file
    """
    pipeline_tasks = []
    for proxy in context.results:
        result = proxy.read()
        try:
            pipeline_tasks.append(result.pipeline_casa_task)
        except AttributeError:
            pipeline_tasks.append('# stage %s: unknown task generated %s '
                                  'result' % (result.stage_number,
                                              result.__class__.__name__))
    
    task_string = '\n'.join(['    %s' % t for t in pipeline_tasks])
    # replace the working directory with ''
    task_string = re.sub('%s/' % context.output_dir, '', task_string)

    state_commands = []
    for o in (context.project_summary, context.project_structure, context.project_performance_parameters):
        state_commands += ['context.set_state({!r}, {!r}, {!r})'.format(cls, name, value)
                           for cls, name, value in project.get_state(o)]

    template = '''__rethrow_casa_exceptions = True
context = h_init()
%s
try:
%s
finally:
    h_save()
''' % ('\n'.join(state_commands), task_string)
            
    f = os.path.join(context.report_dir, context.logs['pipeline_script'])
    with open(f, 'w') as casatask_file: 
        casatask_file.write(template)


_CASA_COMMANDS_PROLOGUE = (
    'This file contains CASA commands run by the pipeline. Although all commands required to calibrate the data are '
    'included here, this file cannot be executed, nor does it contain heuristic and flagging calculations performed by '
    'pipeline code. This file is useful to understand which CASA commands are being run by each pipeline task. If one '
    'wishes to re-run the pipeline, one should use the pipeline script linked on the front page or By Task page of the '
    'weblog. Some stages may not have any commands listed here, e.g. hifa_importdata if conversion from ASDM to MS is '
    'not required.'
)
