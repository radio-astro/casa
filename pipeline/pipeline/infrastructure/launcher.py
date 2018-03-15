"""
The launcher module contains classes to initialize the pipeline, potentially
from a saved context state.
"""
from __future__ import absolute_import
import datetime
import os
try:
    import cPickle as pickle
except:
    import pickle
import pprint

from . import callibrary
from . import casatools
from . import imagelibrary
from . import logging
from . import project
from . import utils

LOG = logging.get_logger(__name__)


# minimum allowed CASA revision. Set to 0 or None to disable
MIN_CASA_REVISION = [5,3,0,91]
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
    def __init__(self, output_dir=None, name=None):
        # initialise the context name with something reasonable: a current
        # timestamp
        now = datetime.datetime.utcnow()
        self.name = name if name else now.strftime('pipeline-%Y%m%dT%H%M%S')

        # domain depends on infrastructure.casatools, so infrastructure cannot
        # depend on domain hence the run-time import
        import pipeline.domain as domain
        self.observing_run = domain.ObservingRun()

        self.callibrary = callibrary.CalLibrary(self)
        self.calimlist = imagelibrary.ImageLibrary()
        self.sciimlist = imagelibrary.ImageLibrary()
        self.rmsimlist = imagelibrary.ImageLibrary()
        self.subimlist = imagelibrary.ImageLibrary()

        self.project_summary = project.ProjectSummary()
        self.project_structure = project.ProjectStructure()
        self.project_performance_parameters = project.PerformanceParameters()

        self.output_dir = output_dir
        self.products_dir = None
        self.task_counter = 0
        self.subtask_counter = 0
        self.results = []
        self.logs = {}
        self.contfile = None
        self.linesfile = None
        self.size_mitigation_parameters = {}
        self.imaging_parameters = {}
        self.clean_list_pending = []
        self.clean_list_info = {}
        self.sensitivities = []

        LOG.trace('Creating report directory \'%s\'' % self.report_dir)
        utils.mkdir_p(self.report_dir)

        LOG.trace('Setting products directory to \'%s\'' % self.products_dir)

        LOG.trace('Pipeline stage counter set to {0}'.format(self.stage))
        LOG.todo('Add OUS registration task. Hard-coding log type to MOUS')
        self.logtype = 'MOUS'

        self.logs['casa_commands'] = 'casa_commands.log'
        self.logs['pipeline_script'] = 'casa_pipescript.py'
        self.logs['pipeline_restore_script'] = 'casa_piperestorescript.py'

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

    def __str__(self):
        ms_names = [ms.name 
                    for ms in self.observing_run.measurement_sets]
        return ('Context(name=\'{0}\', output_dir=\'{1}\')\n'
                'Registered measurement sets:\n{2}'
                ''.format(self.name, self.output_dir,
                          pprint.pformat(ms_names)))

    def __repr__(self):
        return '<Context(name={!r})>'.format(self.name)

    def set_state(self, cls, name, value):
        """
        Set a context property using the class name, property name and property
        value. The class name should be one of:

         1. 'ProjectSummary'
         2. 'ProjectStructure'
         3. 'PerformanceParameters'

        Background: see CAS-9497 - add infrastructure to translate values from
        intent.xml to setter functions in casa_pipescript.

        :param cls: class identifier
        :param name: property to set
        :param value: value to set
        :return:
        """
        m = {
            'ProjectSummary': self.project_summary,
            'ProjectStructure': self.project_structure,
            'PerformanceParameters': self.project_performance_parameters
        }
        instance = m[cls]
        setattr(instance, name, value)


class Pipeline(object):
    """
    Pipeline is the entry point for initialising the pipeline. It is
    responsible for the creation of new ~Context objects and for loading 
    saved Contexts from disk.

    TODO replace this class with a static factory method on Context? 
    """

    def __init__(self, context=None, output_dir='./', loglevel='info',
                 casa_version_check=True, name=None, plotlevel='default',
                 path_overrides={}):
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
            if MIN_CASA_REVISION and casatools.utils.compare_version('<', MIN_CASA_REVISION):
                msg = ('Minimum CASA revision for the pipeline is %s, '
                       'got CASA %s.' % (MIN_CASA_REVISION, casatools.utils.version()))
                LOG.critical(msg)
            if MAX_CASA_REVISION and casatools.utils.compare_version('>', MAX_CASA_REVISION):
                msg = ('Maximum CASA revision for the pipeline is %s, '
                       'got CASA %s.' % (MAX_CASA_REVISION, casatools.utils.version()))
                LOG.critical(msg)

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
                last_context = utils.pickle_load(context_file)
                self.context = last_context
                
            for k,v in path_overrides.items():
                setattr(self.context, k, v)

        self._link_casa_log(self.context)

        # define the plot level as a global setting rather than on the 
        # context, as we want it to be a session-wide setting and adjustable
        # mid-session for interactive use. 
        import pipeline.infrastructure as infrastructure 
        infrastructure.set_plot_level(plotlevel)

    def _link_casa_log(self, context):
        report_dir = context.report_dir
        
        # create a hard-link to the current CASA log in the report directory 
        src = casatools.log.logfile()
        dst = os.path.join(report_dir, os.path.basename(src))
        if not os.path.exists(dst):
            try:
                os.link(src, dst)
            except OSError:
                LOG.error('Error creating hard link to CASA log')
                LOG.warning('Reverting to symbolic link to CASA log. This is unsupported!')
                os.symlink(src, dst)

        # the web log creates links to each casa log. The name of each CASA
        # log is appended to the context.
        if 'casalogs' not in context.logs:
            # list as one casa log will be created per CASA session 
            context.logs['casalogs'] = []
        if src not in context.logs['casalogs']:
            context.logs['casalogs'].append(os.path.basename(dst))
    
    def _find_most_recent_session(self, directory='./'):
        # list all the files in the directory..
        files = [f for f in os.listdir(directory) if f.endswith('.context')]

        # .. and from these matches, create a dict mapping files to their 
        # modification timestamps, ..
        name_n_timestamp = dict([(f, os.stat(directory+f).st_mtime) 
                                 for f in files])
 
        # .. then return the file with the most recent timestamp
        return max(name_n_timestamp, key=name_n_timestamp.get)
    
    def __repr__(self):
        ms_names = [ms.name 
                    for ms in self.context.observing_run.measurement_sets]
        return 'Pipeline({0})'.format(ms_names)

    def close(self):
        filename = self.context.name
        with open(filename, 'r+b') as session:            
            pickle.dump(self.context, session, protocol=-1)
