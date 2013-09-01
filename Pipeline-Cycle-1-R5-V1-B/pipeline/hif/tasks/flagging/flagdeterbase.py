"""
The flagdeterbase module provides base classes for deterministic flagging
tasks in the pipeline.

To test these classes, register some data with the pipeline using ImportData,
then execute:

import pipeline
vis = [ '<MS name>' ]

# Create a pipeline context and register some data
context = pipeline.Pipeline().context
inputs = pipeline.tasks.ImportData.Inputs(context, vis=vis)
task = pipeline.tasks.ImportData(inputs)
results = task.execute(dry_run=False)
results.accept(context)

# Execute the flagging task
inputs = pipeline.tasks.flagging.FlagDeterBase.Inputs(context,\
      autocorr=True, shadow=True, scan=True, scannumber='4,5,8',\
      intents='*AMPLI*', edgespw=True, fracspw=0.1)
task = pipeline.tasks.flagging.FlagDeterBase(inputs)
result = task.execute(dry_run=True)

In other words, create a context, create the inputs (which sets the public
variables to the correct values and creates the temporary flag command file),
convert the class arguments to arguments to the CASA task flagdata), create
the FlagDeterBase() instance, perform FlagDeterBase.analyse(), and execute the
class.

"""
from __future__ import absolute_import
import os
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks

# the logger for this module
LOG = infrastructure.get_logger(__name__)


class FlagDeterBaseInputs(basetask.StandardInputs):
    """
    FlagDeterBaseInputs manages the inputs for the FlagDeterBase task.
    
    .. py:attribute:: context

        the (:class:`~pipeline.infrastructure.launcher.Context`) holding all
        pipeline state

    .. py:attribute:: vis
    
        a string or list of strings containing the MS name(s) on which to
        operate
        
    .. py:attribute:: output_dir
    
        the directory to which pipeline data should be sent

    .. py:attribute:: flagbackup
    
        a boolean indicating whether whether existing flags should be backed
        up before new flagging begins.
        
    .. py:attribute:: autocorr

        a boolean indicating whether autocorrelations are to be flagged.

    .. py:attribute:: shadow
    
        a boolean indicating whether shadowed antennas are to be flagged.
        
    .. py:attribute:: scan
    
        a boolean indicating whether scan flagging is to be performed.

    .. py:attribute:: scannumber

        A comma-delimited string stating the scans to flag. Standard data
        selection syntax is valid.
        
    .. py:attribute:: intents
    
        A comma-delimited string stating the intents to flag. Wildcards (*
        character) are allowed.

    .. py:attribute:: edgespw
    
        A boolean stating whether edge channels are flagged.
        
    .. py:attribute:: fracspw
    
        A float contains the fraction (between 0.0 and 1.0) of channels
        to removed from the edge.
        
    .. py:attribute:: online
        
        A boolean indicating whether online flags are to be applied.
        
    .. py:attribute:: fileonline
    
        The filename of the ASCII file containing online flagging commands.

    .. py:attribute:: template
    
        A boolean indicating whether flagging templates are to be applied.

    .. py:attribute:: filetemplate

        The filename of the ASCII file that has the flagging template (for 
        RFI, birdies, telluric lines, etc.).    
    """    
    def __init__(self, context, vis=None, output_dir=None, flagbackup=None,
                 autocorr=None, shadow=None, scan=None, scannumber=None,
                 intents=None, edgespw=None, fracspw=None, online=None,
                 fileonline=None, template=None, filetemplate=None):
        """
        Initialise the Inputs, initialising any property values to those given
        here.
        
        :param context: the pipeline Context state object
        :type context: :class:`~pipeline.infrastructure.launcher.Context`
        :param vis: the measurement set(s) to flag
        :type vis: a string or list of strings
        :param output_dir: the output directory for pipeline data
        :type output_dir: string
        :param flagbackup: backup existing flags before flagging
        :type flagbackup: boolean
        :param autocorr: flag autocorrelations
        :type autocorr: boolean
        :param shadow: flag shadowed antennas
        :type shadow: boolean
        :param scan: flag scans specified by scannumber and intents
        :type scan: boolean
        :param scannumber: the scans to flag
        :type scannumber: comma-delimited string
        :param intents: the intents to flag
        :type intents: comma-delimited string
        :param edgespw: flag edge channels
        :type edgespw: boolean
        :param fracspw: the fraction of edge channels to remove
        :type fracspw: float between 0.0-1.0
        :param online: apply online flags
        :type online: boolean
        :param fileonline: filename of the online flagging commands template
        :type fileonline: string
        :param template: apply flagging template
        :type template: boolean
        :param filetemplate: filename of flagging commands template
        :type filetemplate: string
        """        
        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def autocorr(self):
        return self._autocorr
    
    @autocorr.setter
    def autocorr(self, value):
        if value is None:
            value = True
        self._autocorr = value

    @property
    def edgespw(self):
        return self._edgespw
    
    @edgespw.setter
    def edgespw(self, value):
        if value is None:
            #value = True
            value = False
        self._edgespw = value

    @property
    def fileonline(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('fileonline')
        
        if self._fileonline is None:
            vis_root = os.path.splitext(self.vis)[0]
            return vis_root + '_flagonline.txt'
        return self._fileonline
    
    @fileonline.setter
    def fileonline(self, value):
        self._fileonline = value

    @property
    def filetemplate(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('filetemplate')

        if not self._filetemplate:
            vis_root = os.path.splitext(self.vis)[0]
            return vis_root + '_flagtemplate.txt'

        if type(self._filetemplate) is types.ListType:
            idx = self._my_vislist.index(self.vis)
            return self._filetemplate[idx]

        #if not isinstance(self.vis, list):
	    #if not hasattr (self, '_my_vislist'):
	        #idx = 0
	    #else:
                #idx = self._my_vislist.index(self.vis)
            #return self._filetemplate[idx]
        
        #return self._filetemplate[0]
        return self._filetemplate

    #Force filetemplate to be a list.
    @filetemplate.setter
    def filetemplate(self, value):
	if value is None:
	    value = []
	elif type(value) is types.StringType:
	    if value == '':
	        value = []
	    else:
	        value = list(value.replace('[','').replace(']','').replace("'","").split(','))
        self._filetemplate = value

    @property
    def flagbackup(self):
        return self._flagbackup
    
    @flagbackup.setter
    def flagbackup(self, value):
        if value is None:
            value = False
        self._flagbackup = value

    @property
    def fracspw(self):
        return self._fracspw
    
    @fracspw.setter
    def fracspw(self, value):
        if value is None:
            value = 0.05
        self._fracspw = value

    @property
    def inpfile(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('inpfile')

        vis_root = os.path.splitext(self.vis)[0]
        return os.path.join(self.output_dir, vis_root + '_flagcmds.txt')

    @property
    def intents(self):
        return self._intents
    
    @intents.setter
    def intents(self, value):
        if value is None:
            value = '*POINTING*,*FOCUS*,*ATMOSPHERE*,*SIDEBAND_RATIO*'
        self._intents = value
        
    @property
    def online(self):
        return self._online
    
    @online.setter
    def online(self, value):
        if value is None:
            value = True
        self._online = value

    @property
    def scan(self):
        return self._scan
    
    @scan.setter
    def scan(self, value):
        if value is None:
            value = True
        self._scan = value

    @property
    def scannumber(self):
        return self._scannumber
    
    @scannumber.setter
    def scannumber(self, value):
        if value is None:
            value = ''
        self._scannumber = value

    @property
    def shadow(self):
        return self._shadow
    
    @shadow.setter
    def shadow(self, value):
        if value is None:
            value = True
        self._shadow = value

    @property
    def template(self):
        return self._template
    
    @template.setter
    def template(self, value):
        if value is None:
            value = False
        self._template = value        

    def to_casa_args(self):
        """
        Translate the input parameters of this class to task parameters 
        required by the CASA task flagdata. The returned object is a 
        dictionary of flagdata arguments as keyword/value pairs.
        
        :rtype: dict        
        """

	# Generate file list.
	filelist = []
	if self.online:
	    if os.path.isfile(self.fileonline):
	        filelist.append(self.fileonline)
	    else:
	        LOG.warning('Online flags file %s does not exist' % (self.fileonline))
	filelist.append(self.inpfile)
	if self.template:
	    if os.path.isfile(self.filetemplate):
	        filelist.append(self.filetemplate)
	    else:
	        LOG.warning('Template flags file %s does not exist' % (self.filetemplate))

        return {'vis'        : self.vis,
                'mode'       : 'list',
                'action'     : 'apply',                     
                #'inpfile'    : self.inpfile,
                'inpfile'    : filelist,
                'savepars'   : False,
                'flagbackup' : self.flagbackup}


class FlagDeterBaseResults(basetask.Results):
    def __init__(self, jobs=[]):
        """
        Initialise the results object with the given list of JobRequests.
        """
        super(FlagDeterBaseResults, self).__init__()
        self.jobs = jobs

    def __repr__(self):
        s = 'Deterministic flagging results:\n'
        for job in self.jobs:
            s += '%s performed. Statistics to follow?' % str(job)
        return s 


class FlagDeterBase(basetask.StandardTaskTemplate):
    """
    FlagDeterBase is the base class for deterministic flagging. It can perform
    many different types of deterministic flagging:
    
    - Autocorrelations
    - Shadowed antennas
    - Scan and intents
    - Edge channels
    - Online flags
    - Template flags
    
    FlagDeterBase outputs flagdata flagging commands to a temporary ASCII 
    file located in the pipeline working directory; flagdata is then invoked
    using this command file as input.
    """
    # link the accompanying inputs to this task 
    Inputs = FlagDeterBaseInputs

    def prepare(self):
        """
        Prepare and execute a flagdata flagging job appropriate to the
        task inputs.
        
        This method generates, overwriting if necessary, an ASCII file 
        containing flagdata flagging commands. A flagdata task is then
        executed, using this ASCII file as inputs. 
        """
        # create a local alias for inputs, so we're not saying 'self.inputs'
        # everywhere
        inputs = self.inputs
        
        # get the flagdata command string, ready for the flagdata input file
        flag_cmds = self._get_flag_commands()

        # write the flag commands to the file
        with open(inputs.inpfile, 'w') as stream:
            stream.write(flag_cmds)

        # to save inspecting the file, also log the flag commands
        LOG.debug('Flag commands for %s:\n%s' % (inputs.vis, flag_cmds))

        # Map the pipeline inputs to a dictionary of CASA task arguments 
        task_args = inputs.to_casa_args()

        # create and execute a flagdata job using these task arguments
        job = casa_tasks.flagdata(**task_args)
        self._executor.execute(job)

        # return the results object, which will be used for the weblog
        return FlagDeterBaseResults([job])

    def analyse(self, results):
        """
        Analyse the results of the flagging operation.
        
        This method does not perform any analysis, so the results object is
        returned exactly as-is, with no data massaging or results items
        added. If additional statistics needed to be calculated based on the
        post-flagging state, this would be a good place to do it.
        
        :rtype: :class:~`FlagDeterBaseResults`        
        """
        return results

    def _get_flag_commands(self):
        """
        Get the flagging commands as a string suitable for flagdata.
        
        This method analyses the inputs associated with this instance, parsing
        the input parameters and converting them into a list of matching 
        flagdata flagging commands. This list of commands is then converted
        to one unified string, which can be written to a file and used as 
        input for flagdata.

        :rtype: a string
        """
        # create a local variable for the inputs associated with this instance
        inputs = self.inputs
        
        # the empty list which will hold the flagging commands
        flag_cmds = []

        # Flag autocorrelations?
        if inputs.autocorr:
            #flag_cmds.append('mode=manual antenna=*&&&')
            flag_cmds.append(self._get_autocorr_cmd())
    
        # Flag shadowed antennas?
        if inputs.shadow:
            flag_cmds.append('mode=shadow')
            
        # Flag according to scan numbers and intents?
        if inputs.scan:
	    if inputs.scannumber != '':
                flag_cmds.append('mode=manual scan={0}'
                             ''.format(inputs.scannumber))
	    # These must be separated due to the way agent flagging works
	    if inputs.intents != '':
	        intentslist = inputs.intents.split(',')
	        for item in intentslist:
                    flag_cmds.append('mode=manual intent={0}'
                             ''.format(item))

        # Flag spectral window edge channels?
        if inputs.edgespw: 
            flag_cmds.append(self._get_edgespw_cmds())


        return '\n'.join(flag_cmds)

    def _get_autocorr_cmd (self):
        #return 'mode=manual antenna=*&&&'
        return 'mode=manual autocorr=True'

    def _get_edgespw_cmds(self):
        """
        Return a flagdata flagging command that will flag the edge channels
        of spectral windows in this measurement set.

        The number of channels to flag is calculated using the 'fracspw' value
        on this task's associated Inputs.
        
        :rtype: a string         
        """
        inputs = self.inputs

        # to_flag is the list to which flagging commands will be appended        
        to_flag = []

        # loop over the spectral windows, generate a flagging command for each
        # spw in the ms. Calling get_spectral_windows() with no arguments 
        # returns just the science windows, which is exactly what we want.
        for spw in inputs.ms.get_spectral_windows():
            
            # If the twice the number of flagged channels is greater than the
            # number of channels for a given spectral window, skip it.
            frac_chan = int(round(inputs.fracspw * spw.num_channels + 0.5))
            if 2*frac_chan >= spw.num_channels:
                LOG.debug('Too many flagged channels for spw %s ' % spw.id)
                continue

            # calculate the channel ranges to flag. No need to calculate the
            # left minimum as it is always channel 0.
            l_max = frac_chan - 1
            r_min = spw.num_channels - frac_chan - 1
            r_max = spw.num_channels - 1
            
            # state the spw and channels to flag in flagdata format, adding
            # the statement to the list of flag commands
            cmd = '{0}:0~{1};{2}~{3}'.format(spw.id, l_max, r_min, r_max)
            to_flag.append(cmd)

        return 'mode=manual spw={0}'.format(','.join(to_flag))

    def _add_file(self, filename):
        """
        Read and return the contents of a file or list of files.
        """
        # If the input is a list of flagging command file names, call this
        # function recursively.  Otherwise, read in the file and return its
        # contents
        if type(filename) is types.ListType:
            return ''.join([self._add_file(f) for f in filename])
        else:
            with open(filename) as stream:
                return stream.read().rstrip('\n')
