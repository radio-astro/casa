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
import string

import flaghelper

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
                 intents=None, edgespw=None, fracspw=None, fracspwfps=None, online=None,
                 fileonline=None, template=None, filetemplate=None, hm_tbuff=None, tbuff=None):
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
    def hm_tbuff(self):
        return self._hm_tbuff
        
    @hm_tbuff.setter
    def hm_tbuff(self, value):
        if value is None:
            value = 'halfint'
        if value in 'halfint | 1.5int | manual':
            self._hm_tbuff = value
        else:
            self._hm_tbuff = 'halfint'
        
        
    @property
    def tbuff(self):
        return self._tbuff
        
    @tbuff.setter
    def tbuff(self, value):
        if value is None:
            value = 0.0
        self._tbuff = value
        

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

'''
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
'''        




#New version...
class FlagDeterBaseResults(basetask.Results):
    def __init__(self, summaries, flagcmds):
        super(FlagDeterBaseResults, self).__init__()
        self.summaries = summaries
        self._flagcmds = flagcmds

    def flagcmds(self):
        return self._flagcmds

    def merge_with_context(self, context):
        # nothing to do
        pass



    def __repr__(self):
        # Step through the summary list and print a few things.
        # SUBTRACT flag counts from previous agents, because the counts are
        # cumulative.
        s = 'Deterministic flagging results:\n'
        
        
        for idx in range(0, len(self.summaries)):
            flagcount = int(self.summaries[idx]['flagged'])
            totalcount = int(self.summaries[idx]['total'])

            # From the second summary onwards, subtract counts from the previous
            # one
            if idx > 0:
                flagcount = flagcount - int(self.summaries[idx-1]['flagged'])

            s += '\tSummary %s (%s) :  Flagged : %s out of %s (%0.2f%%)\n' % (
                    idx, self.summaries[idx]['name'], flagcount, totalcount,
                    100.0*flagcount/totalcount)
        
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
        ####with open(inputs.inpfile, 'w') as stream:
        ####    stream.write(flag_cmds)

        # to save inspecting the file, also log the flag commands
        LOG.debug('Flag commands for %s:\n%s' % (inputs.vis, flag_cmds))

        #Set the tbuf parameter
        if inputs._hm_tbuff == 'halfint':
            # compute half of the maximum integration time, see ms.get_median_integration times method (this
            # has an intent parameter so it can be restricted to science intents)
            tbuff = 0.5 * inputs.ms.get_median_integration_time()
        elif inputs._hm_tbuff == '1.5int':
            tbuff = 1.5 * inputs.ms.get_median_integration_time()
        else:
            tbuff = inputs._tbuff

        # Map the pipeline inputs to a dictionary of CASA task arguments 
        ####task_args = inputs.to_casa_args()
        task_args = {'vis' :inputs.vis,
                     'mode' : 'list',
                     'tbuff' : tbuff,
                     'inpfile' : flag_cmds}

        # create and execute a flagdata job using these task arguments
        print "Determining Summary reports"
        job = casa_tasks.flagdata(**task_args)
        summary_dict = self._executor.execute(job)

        sumreps = {}
        

        for key in summary_dict.keys():
            sumreps[summary_dict[key]['name']] = summary_dict[key]

 
        summary_reps = []
        pseudoagents = ['before','online', 'template', 'autocorr', 'shadow', 'intents', 'edgespw',
                        'clip', 'quack', 'baseband']

        for agent in pseudoagents:
            try:
                summary_reps.append(sumreps[agent])
            except:
               LOG.debug('Agent ' + agent + ' not present')



        # return the results object, which will be used for the weblog
        return FlagDeterBaseResults(summary_reps, flag_cmds)

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
        
        
        # flag online?
        if inputs.online:
            if not os.path.exists(inputs.fileonline):
                LOG.warning('Online flag file \'%s\' was not found. Online '
                            'flagging for %s disabled.' % (inputs.fileonline, 
                                                           inputs.ms.basename))
            else:
                flag_cmds.extend(self._read_flagfile(inputs.fileonline))
                flag_cmds.append('mode=summary name=online')
        
        # flag template?
        if inputs.template:
            if not os.path.exists(inputs.filetemplate):
                LOG.warning('Template flag file \'%s\' was not found. Template '
                            'flagging for %s disabled.' % (inputs.filetemplate, 
                                                           inputs.ms.basename))
            else:
                flag_cmds.extend(self._read_flagfile(inputs.filetemplate))
                flag_cmds.append('mode=summary name=template')

        # Flag autocorrelations?
        #if inputs.autocorr:
        #    #flag_cmds.append('mode=manual antenna=*&&&')
        #    flag_cmds.append(self._get_autocorr_cmd())
    
        # Flag autocorrelations?
        if inputs.autocorr:
            flag_cmds.append('mode=manual autocorr=True reason=autocorr')
            flag_cmds.append('mode=summary name=autocorr')
    
        # Flag shadowed antennas?
        if inputs.shadow:
            flag_cmds.append('mode=shadow reason=shadow')
            flag_cmds.append('mode=summary name=shadow')
    
    
        # Flag shadowed antennas?
        ##if inputs.shadow:
        ##    flag_cmds.append('mode=shadow')
        
        
        # Flag according to scan numbers and intents?
        if inputs.scan and inputs.scannumber != '':
            flag_cmds.append('mode=manual scan=%s reason=scans' % inputs.scannumber)
            flag_cmds.append('mode=summary name=scans')

        # These must be separated due to the way agent flagging works
        if inputs.intents != '':
            for intent in inputs.intents.split(','):
                if '*' not in intent:
                    intent = '*%s*' % intent
                flag_cmds.append('mode=manual intent=%s reason=intents' % intent)
            flag_cmds.append('mode=summary name=intents')

        # Flag spectral window edge channels?
        if inputs.edgespw: 
            to_flag = self._get_edgespw_cmds()
            if to_flag:
                spw_arg = ','.join(to_flag)
                flag_cmds.append('mode=manual spw=%s reason=edgespw' % spw_arg)
                flag_cmds.append('mode=summary name=edgespw')

        # summarise the state before flagging rather than assuming the initial
        # state is unflagged
        if flag_cmds:
            flag_cmds.insert(0, 'mode=summary name=before')

        LOG.trace('Flag commands for %s:\n%s' % (inputs.ms.basename, 
                                                 '\n'.join(flag_cmds)))
        
        
        
        '''
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
        '''

        #return '\n'.join(flag_cmds)

        return flag_cmds
        
    

    def _get_autocorr_cmd (self):
        #return 'mode=manual antenna=*&&&'
        return 'mode=manual autocorr=True'

    
    def verify_spw(self, spw):
        """
        Verify that the given spw should be flagged, raising a ValueError if
        it should not.
        
        Checks in this function should be generic. Observatory-dependent tests
        should be added by extending the AgentFlagger and overriding this
        method.
        """
        # Get the data description for this spw
        dd = self.inputs.ms.get_data_description(spw=spw)
        if dd is None:
            raise ValueError('Missing data description for spw %s ' % spw.id)

        ncorr = len(dd.corr_axis)
        if ncorr not in (1, 2, 4):
            raise ValueError('Wrong number of correlations %s for spw %s '
                             '' % (ncorr, spw.id))


    def _get_edgespw_cmds(self):
        inputs = self.inputs

        # to_flag is the list to which flagging commands will be appended
        to_flag = []

        # loop over the spectral windows, generate a flagging command for each
        # spw in the ms. Calling get_spectral_windows() with no arguments
        # returns just the science windows, which is exactly what we want.
        for spw in inputs.ms.get_spectral_windows():
            try:
                # test that this spw should be flagged by assessing number of
                # correlations, TDM/FDM mode etc.
                self.verify_spw(spw)
            except ValueError as e:
                # this spw should not be or is incapable of being flagged
                LOG.debug(e.message)
                continue

            # get fraction of spw to flag from template function
            fracspw = self.get_fracspw(spw)

            # If the twice the number of flagged channels is greater than the
            # number of channels for a given spectral window, skip it.
            frac_chan = int(round(fracspw * spw.num_channels + 0.5))
            if 2*frac_chan >= spw.num_channels:
                LOG.debug('Too many flagged channels %s for spw %s '
                          '' % (spw.num_channels, spw.id))
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

        return to_flag


    '''
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
    '''


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
                
    def _read_flagfile(self, filename):
        if not os.path.exists(filename):
            LOG.warning('%s does not exist' % filename)
            return []

        # strip out comments and empty lines to leave the real commands.
        # This is so we can compare the number of valid commands to the number
        # of commands specified in the file and complain if they differ
        return [cmd for cmd in flaghelper.readFile(filename) 
                if not cmd.strip().startswith('#')
                and not all(c in string.whitespace for c in cmd)]
