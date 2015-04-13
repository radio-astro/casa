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

    autocorr = basetask.property_with_default('autocorr', True)
    edgespw = basetask.property_with_default('edgespw', False)
    flagbackup = basetask.property_with_default('flagbackup', False)
    fracspw = basetask.property_with_default('fracspw', 0.05)
    online = basetask.property_with_default('online', True)
    scan = basetask.property_with_default('scan', True)
    scannumber = basetask.property_with_default('scannumber', '')
    shadow = basetask.property_with_default('shadow', True)
    template = basetask.property_with_default('template', False)

    def __init__(self, context, vis=None, output_dir=None, flagbackup=None,
                 autocorr=None, shadow=None, scan=None, scannumber=None,
                 intents=None, edgespw=None, fracspw=None, fracspwfps=None, 
                 online=None, fileonline=None, template=None, filetemplate=None, 
                 hm_tbuff=None, tbuff=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

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

        return self._filetemplate

    @filetemplate.setter
    def filetemplate(self, value):
        if value in (None, ''):
            value = []
        elif type(value) is types.StringType:
            value = list(value.replace('[','').replace(']','').replace("'","").split(','))
        self._filetemplate = value

    @property
    def hm_tbuff(self):
        return self._hm_tbuff
    
    @hm_tbuff.setter
    def hm_tbuff(self, value=None):
        if value is None: 
            value = 'halfint'
        if value not in ('halfint', '1.5int', 'manual'):
            LOG.warning('Unexpected value for hm_tbuff: %s. Using halfint.', 
                        value)
            value = 'halfint'
        self._hm_tbuff = value

    @property
    def inpfile(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('inpfile')

        vis_root = os.path.splitext(self.vis)[0]
        return os.path.join(self.output_dir, vis_root + '_flagcmds.txt')

    @property
    def intents(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('intents')

        if self._intents is not None:
            return self._intents

        # return just the unwanted intents that are present in the MS
        intents_to_flag = set(['POINTING','FOCUS','ATMOSPHERE','SIDEBAND','UNKNOWN', 'SYSTEM_CONFIGURATION'])
        return ','.join(self.ms.intents.intersection(intents_to_flag))

    @intents.setter
    def intents(self, value):
        self._intents = value        

    @property
    def tbuff(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('tbuff')

        if self.hm_tbuff == 'halfint':
            if any([a.diameter == 7.0 for a in self.ms.antennas]):
                return 0.0

            median_ints = [self.ms.get_median_science_integration_time(intent=intent)
                           for intent in ('AMPLITUDE','BANDPASS','PHASE','TARGET','CHECK')
                           if intent in self.ms.intents]
            if not median_ints:
                return 0.0
            return 0.5 * max(median_ints)

        elif self.hm_tbuff == '1.5int':
            if hasattr(self.context, 'evla'):
                t = self.ms.get_vla_max_integration_time()
            else:
                t = self.ms.get_median_integration_time()
            return 1.5 * t

        else:
            return self._tbuff
            
    @tbuff.setter
    def tbuff(self, value=None):
        if value == None:
            value = 0.0
        self._tbuff = value                
        
    def to_casa_args(self):
        """
        Translate the input parameters of this class to task parameters 
        required by the CASA task flagdata. The returned object is a 
        dictionary of flagdata arguments as keyword/value pairs.
        
        :rtype: dict        
        """
        return {'vis'        : self.vis,
                'mode'       : 'list',
                'action'     : 'apply',                     
                'inpfile'    : self.inpfile,
                'tbuff'      : self.tbuff,
                'savepars'   : False,
                'flagbackup' : self.flagbackup}


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
        flag_str = '\n'.join(flag_cmds)

        # write the flag commands to the file
        with open(inputs.inpfile, 'w') as stream:
            stream.writelines(flag_str)

        # to save inspecting the file, also log the flag commands
        LOG.debug('Flag commands for %s:\n%s', inputs.vis, flag_str)

        # Map the pipeline inputs to a dictionary of CASA task arguments 
        task_args = inputs.to_casa_args()

        # create and execute a flagdata job using these task arguments
        job = casa_tasks.flagdata(**task_args)
        summary_dict = self._executor.execute(job)

        agent_summaries = dict((v['name'], v) for v in summary_dict.values())
 
        ordered_agents = ['before', 'anos', 'online', 'template', 'autocorr',
                          'shadow', 'intents', 'edgespw', 'clip', 'quack', 
                          'baseband']

        summary_reps = [agent_summaries[agent] 
                        for agent in ordered_agents 
                        if agent in agent_summaries]

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
        if inputs.autocorr:
            flag_cmds.append('mode=manual autocorr=True reason=autocorr')
            flag_cmds.append('mode=summary name=autocorr')
    
        # Flag shadowed antennas?
        if inputs.shadow:
            flag_cmds.append('mode=shadow reason=shadow')
            flag_cmds.append('mode=summary name=shadow')
        
        # Flag according to scan numbers and intents?
        if inputs.scan and inputs.scannumber != '':
            flag_cmds.append('mode=manual scan=%s reason=scans' % inputs.scannumber)
            flag_cmds.append('mode=summary name=scans')

        # These must be separated due to the way agent flagging works
        if inputs.intents != '':
            #for intent in inputs.intents.split(','):
            #    if '*' not in intent:
            #        intent = '*%s*' % intent
            #    flag_cmds.append('mode=manual intent=%s reason=intents' % intent)
            #flag_cmds.append('mode=summary name=intents')
            for intent in inputs.intents.split(','):
                #if '*' not in intent:
                    #intent = '*%s*' % intent
                intentlist = list(inputs.ms.get_original_intent(intent))
                for intent_item in intentlist:
                    flag_cmds.append('mode=manual intent=%s reason=intents' % intent_item)
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
            frac_chan = int(round(fracspw * spw.num_channels))
            if 2*frac_chan >= spw.num_channels:
                LOG.debug('Too many flagged channels %s for spw %s '
                          '' % (spw.num_channels, spw.id))
                continue

            # calculate the channel ranges to flag. No need to calculate the
            # left minimum as it is always channel 0.
            l_max = frac_chan - 1
            r_min = spw.num_channels - frac_chan
            r_max = spw.num_channels - 1

            # state the spw and channels to flag in flagdata format, adding
            # the statement to the list of flag commands
            cmd = '{0}:0~{1};{2}~{3}'.format(spw.id, l_max, r_min, r_max)
            to_flag.append(cmd)

        return to_flag

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
