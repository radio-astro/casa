from __future__ import absolute_import
import os
import re
import types

import flaghelper

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)


class AgentFlaggerResults(basetask.Results):
    def __init__(self, summaries, flagcmds):
        super(AgentFlaggerResults, self).__init__()
        self.summaries = summaries
        self.flagcmds = flagcmds

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


class AgentFlaggerInputs(basetask.StandardInputs):
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
                  intents=None, edgespw=None, fracspw=None, online=None,
                  fileonline=None, template=None, filetemplate=None):
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
    def intents(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('intents')

        if self._intents is not None:
            return self._intents

        # default intents
        intents_to_flag = set(['POINTING','FOCUS','ATMOSPHERE','SIDEBAND'])
        return ','.join(self.ms.intents.intersection(intents_to_flag))

    @intents.setter
    def intents(self, value):
        self._intents = value

    def to_casa_args(self):
        # this task uses tools rather than CASA tasks, so there are no
        # equivalent task arguments
        raise NotImplementedError


class AgentFlagger(basetask.StandardTaskTemplate):
    Inputs = AgentFlaggerInputs

    def prepare(self):
        af = casatools.agentflagger
        af.open(msname=self.inputs.ms.name);
        af.selectdata();

        agentcmds = self._get_flag_commands()

        # Unflag (This is only for testing)
#         if agentcmds:
#             LOG.warning('Unflagging all data before processing!')
#             self._add_agent('mode=unflag')

        for cmd in agentcmds:
            self._add_agent(cmd)

        # Run all these agents, and get the combined report.
        af.init()
        summary_stats_list = af.run(writeflags=True)
        af.done()

        # Parse the output summary lists and extract only 'type==summary'
        # Iterate through the list in the correct order. Do not follow default
        # 'dictionary-key' ordering.
        summary_reps=[];
        for rep in range(0, summary_stats_list['nreport']):
            repname = 'report' + str(rep)
            if summary_stats_list[repname]['type'] == 'summary':
                summary_reps.append(summary_stats_list[repname])

        return AgentFlaggerResults(summary_reps, agentcmds)

    def analyse(self, results):
        return results

    def _add_agent(self, cmdstring):
        agent_cmd = {'apply'      : True,
                     'sequential' : True}
        agent_cmd.update(self._string_to_dict(cmdstring))

        # add mode=manual for flags that do not specify the mode, such as the
        # online flags  
        if 'mode' not in agent_cmd:
            agent_cmd['mode'] = 'manual'

        casatools.agentflagger.parseagentparameters(agent_cmd)

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

    def get_fracspw(self, spw):
        """
        Get the fraction of data to flag for the given spw, expressed as a
        floating point number between 0 and 1.
        """
        return self.inputs.fracspw

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

    def _get_flag_commands(self):
        # create a local variable for the inputs associated with this instance
        inputs = self.inputs

        # the empty list which will hold the flagging commands
        cmds = []

        # flag online?
        if inputs.online:
            if not os.path.exists(inputs.fileonline):
                LOG.warning('Online flag file \'%s\' was not found. Online '
                            'flagging for %s disabled.' % (inputs.fileonline, 
                                                           inputs.ms.basename))
            else:
                for cmddict in self._read_flagfile(inputs.fileonline):
                    cmds.append(cmddict['command'])
                cmds.append('mode=summary name=online')

        # flag template?
        if inputs.template:
            if not os.path.exists(inputs.filetemplate):
                LOG.warning('Template flag file \'%s\' was not found. Template '
                            'flagging for %s disabled.' % (inputs.filetemplate, 
                                                           inputs.ms.basename))
            else:
                for cmddict in self._read_flagfile(inputs.filetemplate):
                    cmds.append(cmddict['command'])
                cmds.append('mode=summary name=template')

        # Flag autocorrelations?
        if inputs.autocorr:
            cmds.append('mode=manual autocorr=True reason=autocorr')
            cmds.append('mode=summary name=autocorr')

        # Flag shadowed antennas?
        if inputs.shadow:
            cmds.append('mode=shadow reason=shadow')
            cmds.append('mode=summary name=shadow')

        # Flag according to scan numbers and intents?
        if inputs.scan and inputs.scannumber != '':
            cmds.append('mode=manual scan=%s reason=scans' % inputs.scannumber)
            cmds.append('mode=summary name=scans')

        # These must be separated due to the way agent flagging works
        if inputs.intents != '':
            for intent in inputs.intents.split(','):
                if '*' not in intent:
                    intent = '*%s*' % intent
                cmds.append('mode=manual intent=%s reason=intents' % intent)
            cmds.append('mode=summary name=intents')

        # Flag spectral window edge channels?
        if inputs.edgespw: 
            to_flag = self._get_edgespw_cmds()
            if to_flag:
                spw_arg = ','.join(to_flag)
                cmds.append('mode=manual spw=%s reason=edgespw' % spw_arg)
                cmds.append('mode=summary name=edgespw')

        # summarise the state before flagging rather than assuming the initial
        # state is unflagged
        if cmds:
            cmds.insert(0, 'mode=summary name=before')

        LOG.trace('Flag commands for %s:\n%s' % (inputs.ms.basename, 
                                                 '\n'.join(cmds)))

        return cmds

    def _read_flagfile(self, filename):
        if not os.path.exists(filename):
            LOG.warning('%s does not exist' % filename)
            return []

        f = flaghelper.readFile(filename)
        flagcmds = flaghelper.makeDict(f)
        return [flagcmds[k] for k in sorted(flagcmds.keys())]

    def _string_to_dict(self, cmdstring):
        d = {}
        for arg in re.split('\s+', cmdstring.strip()):
            k, v = re.split('=', arg, maxsplit=1)
            # convert boolean strings to boolean objects
            v = True if v in ('True', '"True"', "'True'") else v
            v = False if v in ('False', '"False"', "'False'") else v
            d[k] = v
        return d


#class ALMAAgentFlaggerInputs(AgentFlaggerInputs):
#    """
#    Flagger inputs class for ALMA data. It extends the standard inputs with
#    an extra parameter (fracspwfps) used when processing ACA windows.
#    """
#    # override superclass inputs with ALMA-specific values
#    edgespw = basetask.property_with_default('edgespw', True)
#    fracspw = basetask.property_with_default('fracspw', 0.0625)
#    template = basetask.property_with_default('template', True)
#    
#    # new property for ACA correlator
#    fracspwfps = basetask.property_with_default('fracspwfps', 0.048387)
#
#    def __init__(self, context, vis=None, output_dir=None, flagbackup=None,
#                  autocorr=None, shadow=None, scan=None, scannumber=None,
#                  intents=None, edgespw=None, fracspw=None, fracspwfps=None,
#                  online=None, fileonline=None, template=None,
#                  filetemplate=None):
#        self._init_properties(vars())
#
#
#class ALMAAgentFlagger(AgentFlagger):
#    """
#    Agent flagger class for ALMA data.
#    """
#    Inputs = ALMAAgentFlaggerInputs
#    
#    def get_fracspw(self, spw):    
#        # override the default fracspw getter with our ACA-aware code
#        if spw.num_channels in (62, 124, 248):
#            return self.inputs.fracspwfps
#        else:
#            return self.inputs.fracspw
#    
#    def verify_spw(self, spw):
#        # override the default verifier, adding an extra test that bypasses
#        # flagging of TDM windows
#        super(ALMAAgentFlagger, self).verify_spw(spw)
#
#        # Skip if TDM mode where TDM modes are defined to be modes with 
#        # <= 256 channels per correlation
#        dd = self.inputs.ms.get_data_description(spw=spw)
#        ncorr = len(dd.corr_axis)
#        if ncorr*spw.num_channels > 256:
#            raise ValueError('Skipping edge flagging for FDM spw %s' % spw.id)            
