from __future__ import absolute_import
import abc
import collections
import datetime
import inspect
import itertools
import operator
import os
try:
    import cPickle as pickle
except:
    import pickle
import pprint
import re
try:
    import cStringIO as StringIO
except:
    import StringIO
import textwrap
import types
import uuid

from . import api
from . import adapters
from . import casatools
from . import callibrary
from . import filenamer
from . import jobrequest
from . import launcher
from . import logging
from . import pipelineqa
from . import utils
import pipeline.extern.ordereddict as ordereddict

LOG = logging.get_logger(__name__)


# control generation of the weblog
DISABLE_WEBLOG = False
VISLIST_RESET_KEY = '_do_not_reset_vislist'

def timestamp(method):
    def timed(self, *args, **kw):
        start = datetime.datetime.now();
        result = method(self, *args, **kw)
        end = datetime.datetime.now()

        if result is not None:
            result.timestamps = Timestamps(start, end)
            
            inputs = self.inputs
            result.inputs = inputs.as_dict()

            if inputs.context.subtask_counter is 0: 
                result.stage_number = inputs.context.task_counter - 1
            else:
                result.stage_number = inputs.context.task_counter                
        return result

    return timed

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
        return self.context.observing_run.get_ms(name=self.vis)

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

        return [ms.name for ms in self.context.observing_run.measurement_sets]

#         ms_names = [ms.name 
#                     for ms in self.context.observing_run.measurement_sets]
#         return ms_names[0] if len(ms_names) == 1 else ms_names

    @vis.setter    
    def vis(self, value):
        if value is None:
            vislist = [ms.name 
                       for ms in self.context.observing_run.measurement_sets]
        else:
            vislist = value if type(value) is types.ListType else [value,]

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
    def calto(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('calto')
        
        return callibrary.CalTo(vis=self.vis,
                                field=self.field, 
                                spw=self.spw,
                                intent=self.intent,
                                antenna=self.antenna)

    @property
    def calstate(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('calstate')
        
        return self.context.callibrary.get_calstate(self.calto, 
                                                    ignore=['calwt',])

    @property
    def gaincurve(self):
        """
        Get the value of gaincurve. 
        
        Unless overridden, the measurement set will be examined to determine
        the appropriate value.
        """
        # gaincurve is ms-dependent. Return a list of gaincurve results if vis
        # refers to multiple measurement sets.
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('gaincurve')

        # call the heuristic to determine the correct value of gaincurve -
        # unless the user has overridden the heuristic with a fixed value.
        if callable(self._gaincurve) and self.ms:
            adapted = adapters.GaincurveAdapter(self._gaincurve)
            return adapted(self.ms)
        return self._gaincurve

    @gaincurve.setter
    def gaincurve(self, value):
        """
        Set the value of gaincurve.
        
        Setting gaincurve to None allows the pipeline to determine the 
        appropriate value.
        """
        if value not in (None, True, False):
            raise TypeError, 'gaincurve must be one of None, True or False'
        if value is None:
            import pipeline.hif
            value = pipeline.hif.heuristics.Gaincurve()
        self._gaincurve = value

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

    def _init_properties(self, properties={}, kw_ignore=['self']):
        """
        Set the instance properties using a dictionary of keyword/value pairs.
        Properties named in kw_ignore will not be set.
        """
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
        skip = ['self', 'context', 'output_dir', 'ms', 
                'to_field', 'to_intent', 'calto', 'calstate']
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

        if args.get('intent', None) != None:
            args['intent'] = utils.to_CASA_intent(ms, args['intent'])

        for unwanted in ('to_intent', 'to_field'):
            if unwanted in args:
                del args[unwanted]
            
        for k,v in args.items():
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
        if name in ('_active', '_delegates', VISLIST_RESET_KEY):
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
                LOG.trace('Setting \'{0}\' attribute to \'{1}\' on \'{2}'
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
    def get_constructor_args(cls, ignore=('self','context')):
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
        super(ModeTask, self).__init__(inputs)
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
        self.qa = pipelineqa.QAScorePool()

    @property
    def uuid(self):
        """
        The unique identifier for this results object.
        """
        return self._uuid

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

    def accept(self, context=None, **other_parameters):
        """
        Accept these results, registering objects with the context and incrementing
        stage counters as necessary in preparation for the next task.
        """
        if context is None:
            # context will be none when called from a CASA interactive 
            # session. When this happens, we need to locate the global context
            # from the             
            import pipeline.h.cli.utils
            context = pipeline.h.cli.utils.get_context()

        self._check_for_remerge(context)

        # execute our template function
        self.merge_with_context(context, **other_parameters)

        # perform QA if accepting this result from a top-level task
        if context.subtask_counter is 0:
            pipelineqa.registry.do_qa(context, self)

        if context.subtask_counter is 0:
            # If accept() is called at the end of a task as signified by the
            # subtask counter, we should create a proxy for this result and
            # pickle it to the appropriate weblog stage directory. This keeps
            # the context size at a minimum.
            proxy = ResultsProxy(context)
            proxy.write(self)
            result = proxy
        else:
            result = self
    
        # with no exceptions thrown by this point, we can safely add this 
        # results object to the results list
        context.results.append(result)

        # generate weblog if accepting a result from a top-level task
        if context.subtask_counter is 0 and not DISABLE_WEBLOG:
            # cannot import at initial import time due to cyclic dependency
            import pipeline.infrastructure.renderer.htmlrenderer as htmlrenderer
            htmlrenderer.WebLogGenerator.render(context)

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
        self._output_dir = os.path.join(context.output_dir, context.name)
        self._report_dir = context.report_dir

    def write(self, result):
        '''
        Write the pickled result to disk.
        '''
        pickle_name = 'result-stage%s.pickle' % result.stage_number
        path = os.path.join(self._output_dir, pickle_name)
        self._path = path
        
        # adopt the result's UUID protecting against repeated addition to the
        # context
        self.uuid = result.uuid
        
        self._write_stage_logs(result)
        
        with open(path, 'wb') as outfile:
            pickle.dump(result, outfile, -1)

    def read(self):
        '''
        Read the pickle from disk, returning the unpickled object.
        '''
        with open(self._path) as infile:
            return pickle.load(infile)

    def _write_stage_logs(self, result):
        """
        Take the CASA log snippets attached to each result and write them to
        the appropriate weblog directory. The log snippet is deleted from the
        result after a successful write to keep the pickle size down. 
        """
        if not hasattr(result, 'casalog'):
            return

        stage_dir = os.path.join(self._report_dir,
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


class ResultsList(Results, list):
    def __init__(self):
        super(ResultsList, self).__init__()

    def merge_with_context(self, context):
        for result in self:
            result.merge_with_context(context)
    
    def accept(self, context):
        return super(ResultsList, self).accept(context)


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
        super(StandardTaskTemplate, self).__init__(inputs)

    def is_multi_vis_task(self):
        return False

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

    def _copy(self, original):
        LOG.trace('Cloning {0} for {1} task'.format(
            original.__class__.__name__, self.__class__.__name__))
        pickled = StringIO.StringIO()
        pickle.dump(original, pickled, -1)
        # rewind to the start of the 'file', allowing it to be read in its
        # entirety - otherwise we get an EOFError
        pickled.seek(0)
        copy = pickle.load(pickled)
        return copy

    def _merge_jobs(self, jobs, task, merge=(), ignore=()):
        """
        Merge jobs that are identical apart from the arguments named in
        ignore. These jobs will be recreated with  

        Identical tasks are identified by creating a hash of the dictionary
        of task keyword arguments, ignoring keywords specified in the 
        'ignore' argument. Jobs with the same hash can be merged; this is done
        by appending the spw argument of job X to the spw argument of memoed
        job Y, whereafter job X can be discarded.  

        :param jobs: the job requests to merge
        :type jobs: a list of\ 
            :class:`~pipeline.infrastructure.jobrequest.JobRequest`
        :param task: the CASA task to recreate
        :type task: a reference to a function on \
            :class:`~pipeline.infrastructure.jobrequest.casa_tasks'
        :param ignore: the task arguments to ignore during hash creation
        :type ignore: an iterable containing strings

        :rtype: a list of \
            :class:`~pipeline.infrastructure.jobrequest.JobRequest`
        """
        # union stores the property names that to ignore while calculating the
        # job hash
        union = frozenset(itertools.chain.from_iterable((merge, ignore)))

        hashes = ordereddict.OrderedDict()
        for job in jobs:
            job_hash = job.hash_code(ignore=union)
            if job_hash not in hashes:
                hashes[job_hash] = job
            else:
                hashed_job_args = hashes[job_hash].kw
                new_job_args = dict(hashed_job_args)
                for prop in merge:
                    if job.kw[prop] not in hashed_job_args[prop]:
                        merged_value = ','.join((hashed_job_args[prop], job.kw[prop]))
                        new_job_args[prop] = merged_value
                    hashes[job_hash] = task(**new_job_args)

        return hashes.values()

    @timestamp
    @capture_log
    def execute(self, dry_run=True, **parameters):
        # The filenamer deletes any identically named file when constructing
        # the filename, which is desired when really executing a task but not
        # when performing a dry run. This line disables the 
        # 'delete-on-generate' behaviour.
        filenamer.NamingTemplate.dry_run = dry_run

        # knowing when to increment the stage counter becomes interesting when
        # the task spawns sub-tasks. In these circumstances, we don't want to
        # increment the stage number, but instead want to increment the
        # sub-stage number.
        self.inputs.context.subtask_counter += 1

        # Set the task name, but only if this is a top-level task. This name
        # will be prepended to every data product name as a sign of their
        # origin
        if self.inputs.context.subtask_counter is 1:
            from . import casataskdict
            name = casataskdict.classToCASATask.get(self.__class__,
                                                    self.__class__.__name__)
            filenamer.NamingTemplate.task = name

        # Create a copy of the inputs - including the context - and attach
        # this copy to the Inputs. Tasks can then merge results with this
        # duplicate context at will, as we'll later restore the originals.
        original_inputs = self.inputs
        self.inputs = self._copy(original_inputs)

        # create a job executor that tasks can use to execute subtasks
        self._executor = Executor(self.inputs.context, dry_run)

        # create a new log handler that will capture all messages above
        # WARNING level.
        handler = logging.CapturingHandler(logging.WARNING)

        try:
            # if this task does not handle multiple input mses but was 
            # invoked with multiple mses in its inputs, call our utility
            # function to invoke the task once per ms.
            if (self.is_multi_vis_task() is False 
                and type(self.inputs.vis) is types.ListType):
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

            # tag the result with the class of the originating task
            result.task = self.__class__

            # analyse them..
            result = self.analyse(result)

            # add the log records to the result
            result.logrecords = handler.buffer[:]

            return result

        finally:
            # restore the context to the original context
            self.inputs = original_inputs

            # now the task has completed, we tell the namer not to delete again
            filenamer.NamingTemplate.dry_run = True

            # decrement the task counter
            self.inputs.context.subtask_counter -= 1

            # delete the task name once the top-level task is complete
            if self.inputs.context.subtask_counter is 0:
                filenamer.NamingTemplate.task = None

            # now that the WARNING and above messages have been attached to the
            # result, remove the capturing logging handler from all loggers
            if handler:
                logging.remove_handler(handler)


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
        itself it should override is_multi_vis_task().
        """
        # The following code loops through the MSes specified in vis, 
        # executing the task for the first value (head) and then appending the
        # results of executing the remainder of the MS list (tail).
        if len(self.inputs.vis) is 0:
            # we don't return an empty list as the timestamp decorator wants
            # to set attributes on this value, which it can't on a built-in 
            # list
            return ResultsList()
        head = self.inputs.vis[0]
        tail = self.inputs.vis[1:]

        to_split = ('to_field', 'to_intent', 'calphasetable', 'targetphasetable')
        split_properties = self._get_handled_headtails(to_split)

        for name, ht in split_properties.items():
            setattr(self.inputs, name, ht.head)

        refant_tail = None
        if hasattr(self.inputs, 'refant'):
            if type(self.inputs.refant) is types.ListType and self.inputs.refant:
                refant_head = self.inputs.refant[0]
                refant_tail = self.inputs.refant[1:]
                self.inputs.refant = refant_head

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

        # Delete the capture log for subtasks as the log will be attached to the
        # outer ResultList.
        for r in results:
            if hasattr(r, 'casalog'):
                del r.casalog

        return results

    def _get_handled_headtails(self, names=[]):
        handled = ordereddict.OrderedDict()

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
        self._cmdfile = os.path.join(context.report_dir, 'casa_commands.log')

    @capture_log
    def execute(self, job, merge=False):
        """
        Execute the given job or subtask, returning its output.
        
        :param job: a job or subtask
        :type job: an object conforming to the :class:`~pipeline.api.Task`\
            interface

        :rtype: :class:`~pipeline.api.Result`
        """
        # if this is a JobRequest, log it to our command log
        if not self._dry_run and isinstance(job, jobrequest.JobRequest):
            with open(self._cmdfile, 'a') as cmdfile:
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

                cmdfile.write('%s\n' % '\n'.join(wrapped))


        # execute the job, capturing its results object                
        result = job.execute(dry_run=self._dry_run)
        
        # if requested, merge the result with the context. No type checking
        # here.
        if merge and not self._dry_run:
            result.accept(self._context)

        return result


def property_with_default(name, default, doc=None):
    '''
    Return a property whose value is reset to a default value when setting the
    property value to None.
    '''
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
