from __future__ import absolute_import
import os
import re
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.hif.tasks.flagging import agentflagger

LOG = infrastructure.get_logger(__name__)


class VLAAgentFlaggerInputs(agentflagger.AgentFlaggerInputs):
    """
    Flagger inputs class for VLA data. It extends the standard inputs with
    extra paraemters quack, clip, and baseband
    """
    # override superclass inputs with VLA-specific values
    edgespw = basetask.property_with_default('edgespw', True)
    fracspw = basetask.property_with_default('fracspw', 0.05)
    template = basetask.property_with_default('template', True)
    
    quack = basetask.property_with_default('quack', True)
    clip = basetask.property_with_default('clip', True)
    baseband = basetask.property_with_default('baseband', True)
    


    def __init__(self, context, vis=None, output_dir=None, flagbackup=None,
		 autocorr=None, shadow=None, scan=None, scannumber=None, quack=None, clip=None, baseband=None,
		 intents=None, edgespw=None, fracspw=None, online=None,
		 fileonline=None, template=None, filetemplate=None):
        self._init_properties(vars())




class VLAAgentFlagger(agentflagger.AgentFlagger):
    """
    Agent flagger class for VLA data.
    """
    Inputs = VLAAgentFlaggerInputs
    
    def _get_flag_commands(self):
        """ Adding quack and clip
        """
        flag_cmds = super(VLAAgentFlagger, self)._get_flag_commands()

        inputs = self.inputs

        # flag_cmds


        # Flag mode clip
        if inputs.clip:
            #flag_cmds = flag_cmds +'\n'
            flag_cmds.append('mode=clip correlation=ABS_ALL clipzeros=True')
        
        #print flag_cmds
        
        # Flag quack
        if inputs.quack: 
            #flag_cmds = flag_cmds + '\n'
            flag_cmds.append(self._get_quack_cmds())
        
        #print flag_cmds
        
        # Flag end 5 percent of each spw or minimum of 3 channels
        
        if inputs.edgespw:
            #flag_cmds = flag_cmds + '\n'
            to_flag = self._get_edgespw_cmds()
            if to_flag:
                spw_arg = ','.join(to_flag)
                flag_cmds.append(spw_arg)
            
        # Flag 10 end channels at edges of basebands
        if inputs.baseband:
            to_flag = self._get_baseband_cmds()
            if to_flag:
                flag_cmds.append(to_flag)
        
        if (flag_cmds[-1]== '') : flag_cmds=flag_cmds[0:-1]
            
        print flag_cmds
        
        return flag_cmds
    
    
    def _get_edgespw_cmds(self):
        
        
        inputs = self.inputs
        
        context = inputs.context
        
        m = context.observing_run.measurement_sets[0]
        numSpws = context.evla['msinfo'][m.name].numSpws
        channels = context.evla['msinfo'][m.name].channels
        
        SPWtoflag=''
        
        for ispw in range(numSpws):
            fivepctch=int(0.05*channels[ispw])
            startch1=0
            startch2=fivepctch-1
            endch1=channels[ispw]-fivepctch
            endch2=channels[ispw]-1
            
            #Minimum number of channels flagged must be three on each end
            if (fivepctch < 3):
                startch2=2
                endch1=channels[ispw]-3
            
            if (ispw<max(range(numSpws))):
                SPWtoflag=SPWtoflag+str(ispw)+':'+str(startch1)+'~'+str(startch2)+';'+str(endch1)+'~'+str(endch2)+','
            else:
                SPWtoflag=SPWtoflag+str(ispw)+':'+str(startch1)+'~'+str(startch2)+';'+str(endch1)+'~'+str(endch2)
                
        edgespw_cmd = ["mode=manual spw=" + SPWtoflag]
        
        return edgespw_cmd
        
    
    def _get_quack_cmds(self):
            """
            Return a flagdata flagging command that will quack, ie
            flagdata_list.append("mode='quack' scan=" + quack_scan_string +
            " quackinterval=" + str(1.5*int_time) + " quackmode='beg' " +
            "quackincrement=False")
            
            :rtype: a string
            """
            inputs = self.inputs
                        
            #get heuristics from the context
            context = inputs.context
            
            m = context.observing_run.measurement_sets[0]
            quack_scan_string = context.evla['msinfo'][m.name].quack_scan_string
            int_time = context.evla['msinfo'][m.name].int_time
            
            quack_mode_cmd = 'mode=quack scan=' + quack_scan_string + \
                ' quackinterval=' + str(1.5*int_time) + ' quackmode=beg quackincrement=False'
            
            return quack_mode_cmd

    def _get_baseband_cmds(self):
        """
        Flag 10 end channels at edges of basebands
        """

        inputs = self.inputs

        #get heuristics from the context
        context = inputs.context
        m = context.observing_run.measurement_sets[0]
        
        bottomSPW=''
        topSPW=''

        #Determination of baseband taken from original EVLA scripted pipeline
        low_spws = context.evla['msinfo'][m.name].low_spws 
        high_spws = context.evla['msinfo'][m.name].high_spws
        channels = context.evla['msinfo'][m.name].channels

        for ii in range(0,len(low_spws)):
            if (ii == 0):
                bspw=low_spws[ii]
                tspw=high_spws[ii]
                endch1=channels[tspw]-10
                endch2=channels[tspw]-1
                bottomSPW=str(bspw)+':0~9'
                topSPW=str(tspw)+':'+str(endch1)+'~'+str(endch2)
            else:
                bspw=low_spws[ii]
                tspw=high_spws[ii]
                endch1=channels[tspw]-10
                endch2=channels[tspw]-1
                bottomSPW=bottomSPW+','+str(bspw)+':0~9'
                topSPW=topSPW+','+str(tspw)+':'+str(endch1)+'~'+str(endch2)

        baseband_cmd = ''

        print bottomSPW
        print topSPW
        
        if (bottomSPW != ''):
            SPWtoflag = bottomSPW + ',' + topSPW
            baseband_cmd = 'mode=manual spw=' + SPWtoflag

        return baseband_cmd
    
    def verify_spw(self, spw):
        # override the default verifier, adding an extra test that bypasses
        # flagging of TDM windows
        super(VLAAgentFlagger, self).verify_spw(spw)

        # Skip if TDM mode where TDM modes are defined to be modes with 
        # <= 256 channels per correlation
        dd = self.inputs.ms.get_data_description(spw=spw)
        ncorr = len(dd.corr_axis)
        if ncorr*spw.num_channels > 256:
            raise ValueError('Skipping edge flagging for FDM spw %s' % spw.id)            
