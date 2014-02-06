"""
The launcher module contains classes to initialize the pipeline, potentially
from a saved context state.
"""
from __future__ import absolute_import
import datetime
import errno
import os
try:
    import cPickle as pickle
except:
    import pickle
import pprint

import casadef

from . import callibrary
from . import imagelibrary
from . import logging
from . import project

LOG = logging.get_logger(__name__)


# minimum allowed CASA revision. Set to 0 or None to disable
MIN_CASA_REVISION = 28322
# maximum allowed CASA revision. Set to 0 or None to disable
MAX_CASA_REVISION = None


class Context(object):
    """
    Context holds all pipeline state, consisting of metadata describing the
    data set, objects describing the pipeline calibration state, the tree of
    Results objects summarising the results of each pipeline task, and a
    small number of internal pipeline variables and objects.

    The aim of the Context class is to provide one central object to which all
    pipeline state is attached. Keeping all state in one object makes it easy
    to persist this one object, and thus all state, to disk as a Python pickle,
    allowing pipeline sessions to be interrupted and resumed.

    ... py:attribute:: project_summary

        project summary information.
    
    ... py:attribute:: project_structure

        ALMA project structure information.
    
    .. py:attribute:: observing_run

        the top-level (:class:`~pipeline.domain.observingrun.ObservingRun`)
        through which all other pipeline.domain objects can be accessed

    .. py:attribute:: callibrary
    
        the (:class:`~pipeline.infrastructure.callibrary.CalLibrary`) object
        holding the calibration state for registered measurement sets
        
    .. py:attribute:: calimlist
    
        the (:class:`~pipeline.infrastructure.imagelibrary.ImageLibrary`)
        object holding final images of calibrators

    .. py:attribute:: sciimlist
    
        the (:class:`~pipeline.infrastructure.imagelibrary.ImageLibrary`)
        object holding final images of science targets

    .. py:attribute:: results
    
        the list of (:class:`~pipeline.infrastructure.api.Result`) objects
        holding summaries of the pipeline run, one 
        (:class:`~pipeline.infrastructure.api.Result`) for each task.
        
    .. py:attribute:: output_dir
    
        the directory to which pipeline data should be sent

    .. py:attribute:: raw_dir

        the directory holding the raw ASDMs or measurement sets (not used)

    .. py:attribute:: report_dir
    
        the directory where pipeline HTML reports should be sent

    .. py:attribute:: task_counter
    
        integer holding the index of the last task to complete

    .. py:attribute:: subtask_counter
    
        integer holding the index of the last subtask to complete
    
    .. py:attribute:: name
    
        name of the context; also forms the root of the filename used for the
        pickled state 

    """
    def __init__(self, measurement_sets=[], sessions=[], output_dir=None,
                 name=None):
        # initialise the context name with something reasonable: a current
        # timestamp
        now = datetime.datetime.utcnow()
        self.name = name if name else now.strftime('pipeline-%Y%m%dT%H%M%S')

        self.callibrary = callibrary.CalLibrary(self)
        self.calimlist = imagelibrary.ImageLibrary()
        self.sciimlist = imagelibrary.ImageLibrary()
        self.project_summary = project.ProjectSummary()
        self.project_structure = project.ProjectStructure()
        self.output_dir = output_dir 
        self.products_dir = None
        self.task_counter = 0
        self.subtask_counter = 0
        self.results = []

        # domain depends on infrastructure.casatools, so infrastructure cannot
        # depend on domain hence the run-time import
        import pipeline.domain as domain
        self.observing_run = domain.ObservingRun()

        try:
            LOG.trace('Creating report directory \'%s\'' % self.report_dir)
            os.makedirs(self.report_dir)
        except OSError as exc:
            if exc.errno == errno.EEXIST:
                pass
            else: raise

        try:
            LOG.trace('Setting products directory to \'%s\'' % self.products_dir)
            #os.makedirs(self.products_dir)
        except OSError as exc:
            if exc.errno == errno.EEXIST:
                pass
            else: raise

        LOG.trace('Pipeline stage counter set to {0}'.format(self.stage))
        LOG.todo('Add OUS registration task. Hard-coding log type to MOUS')
        self.logtype = 'MOUS'

    @property
    def subtask_counter(self):
        return self._subtask_counter
    
    @subtask_counter.setter
    def subtask_counter(self, value):
        self._subtask_counter = value
        if value is 0:
            self.task_counter += 1

        LOG.trace('Modifying counters: #task=%s #subtask=%s'
                  '' % (self.task_counter, self.subtask_counter))

    @property
    def stage(self):
        return '%s_%s' % (self.task_counter, self.subtask_counter) 

    @property
    def report_dir(self):
        return os.path.join(self.output_dir, self.name, 'html')
    
    @property
    def output_dir(self):
        return self._output_dir

    @output_dir.setter
    def output_dir(self, value):
        if value is None:
            value = './'

        value = os.path.abspath(value)
        LOG.trace('Setting output_dir to \'%s\'' % value)
        self._output_dir = value

    @property
    def products_dir(self):
        return self._products_dir

    @products_dir.setter
    def products_dir(self, value):
        if value is None:
            (root_dir, _) = os.path.split(self.output_dir)
            value = os.path.join(root_dir, 'products')
        
        value = os.path.abspath(value)
        LOG.trace('Setting products_dir to \'%s\'' % value)
        self._products_dir = value
        
    def save(self, filename=None):
        if filename in ('', None):
            filename = '%s.context' % self.name

        with open(filename, 'wb') as context_file:
            LOG.info('Saving context to \'{0}\''.format(filename))          
            pickle.dump(self, context_file, protocol=-1)

    def __repr__(self):
        ms_names = [ms.name 
                    for ms in self.observing_run.measurement_sets]
        return ('Context(name=\'{0}\', output_dir=\'{1}\')\n'
                'Registered measurement sets:\n{2}'
                ''.format(self.name, self.output_dir,
                          pprint.pformat(ms_names)))


class Pipeline(object):
    """
    Pipeline is the entry point for initialising the pipeline. It is
    responsible for the creation of new ~Context objects and for loading 
    saved Contexts from disk.

    TODO replace this class with a static factory method on Context? 
    """

    def __init__(self, context=None, output_dir='./', loglevel='info',
                 casa_version_check=True, name=None):
        """
        Initialise the pipeline, creating a new ~Context or loading a saved
        ~Context from disk.
    
        :param context: filename of the pickled Context to load from disk.
            Specifying 'last' loads the last-saved Context, while passing None
            creates a new Context.
        :type context: string
        :param output_dir: root directory to which all output will be written
        :type output_dir: string
        :param loglevel: pipeline log level
        :type loglevel: string
        :param casa_version_check: enable (True) or bypass (False) the CASA
            version check. Default is True.
        :type ignore_casa_version: boolean
        """        
        # configure logging with the preferred log level
        logging.set_logging_level(level=loglevel)

        # Prevent users from running the pipeline on old or incompatible
        # versions of CASA by comparing the CASA subversion revision against
        # our expected minimum and maximum
        if casa_version_check is True:
            revision = int(casadef.subversion_revision)
            if MIN_CASA_REVISION and MIN_CASA_REVISION > revision:
                msg = ('Minimum CASA revision for the pipeline is r%s, '
                       'got CASA %s (r%s).' % (MIN_CASA_REVISION, 
                                               casadef.casa_version,
                                               casadef.subversion_revision))
                LOG.critical(msg)
                raise EnvironmentError, msg
            if MAX_CASA_REVISION and MAX_CASA_REVISION < revision:
                msg = ('Maximum CASA revision for the pipeline is r%s, '
                       'got CASA %s (r%s).' % (MAX_CASA_REVISION, 
                                               casadef.casa_version,
                                               casadef.subversion_revision))
                LOG.critical(msg)
                raise EnvironmentError, msg

        # if no previous context was specified, create a new context for the
        # given measurement set
        if context is None:
            self.context = Context(output_dir=output_dir, name=name)

        # otherwise load the context from disk..
        else:
            # .. by finding either last session, or..
            if context == 'last':
                context = self._find_most_recent_session()
        
            # .. the user-specified file
            with open(context, 'rb') as context_file:
                LOG.info ('Reading context from file {0}'.format(context))          
                last_context = pickle.load(context_file)
                self.context = last_context
    
    def _find_most_recent_session(self, directory='./'):
        # list all the files in the directory..
        files = [f for f in os.listdir(directory) if f.endswith('.context')]

        # .. and from these matches, create a dict mapping files to their 
        # modification timestamps, ..
        name_n_timestamp = dict([(f, os.stat(directory+f).st_mtime) 
                                 for f in files])
 
        # .. then return the file with the most recent timestamp
        return max(name_n_timestamp, key=lambda k: name_n_timestamp.get(k))
    
    def __repr__(self):
        ms_names = [ms.name 
                    for ms in self.context.observing_run.measurement_sets]
        return 'Pipeline({0})'.format(ms_names)

    def close(self):
        filename = self.context.name
        with open(filename, 'r+b') as session:            
            pickle.dump(self.context, session, protocol=-1)
