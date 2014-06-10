
# ------------------------------------------------------------------------------

# flagdetervla.py

# NB: THESE FlagDeteVLA*() CLASSES INHERIT FlagDeterBase*() CLASSES.  AT
# PRESENT THE FlagDeterVLA*() CLASSES HAVE NO ADDITIONAL INPUT PARAMETERS, SO
# THEY ACT IN EXACTLY THE SAME MANNER AS THE FlagDeterBase*() CLASSES.

# Description:
# ------------
# This file contains the classes to perform VLA deterministic flagging.

# In a nutshell:
# --------------
# * This class performs all of the deterministic flagging types in the
#   FlagDeterBase*() classes.

# To test these classes by themselves without the rest of the pipeline, enter
# these commands:
#
# import pipeline
#
# vis = [ '<MS name>' ]
# context = pipeline.Pipeline( vis ).context
#
# inputs = pipeline.tasks.flagging.FlagDeterVLA.Inputs( context, vis=vis,
#   output_dir='.', autocorr=True, shadow=True, scan=True, scannumber='4,5,8',
#   intents='*AMPLI*', edgespw=True, fracspw=0.1, fracspwfps=0.1 )
#
# task = pipeline.tasks.flagging.FlagDeterVLA( inputs )
# jobs = task.analyse()
#
# status = task.execute(dry_run=False)
#
# In other words, create a context, create the inputs (which sets the public
# variables to the correct values and creates the temporary flag command file),
# convert the class arguments to arguments to the CASA task tflagdata), create
# the FlatDeterVLA() instance, perform FlatDeterVLA.analyse(), and execute the
# class.

# Classes:
# --------
# FlagDeterVLA        - This class represents the pipeline interface to the
#                        CASA task tflagdata.
# FlagDeterVLAInputs  - This class manages the inputs for the FlagDeterVLA()
#                        class.
# FlagDeterVLAResults - This class manages the results from the FlagDeterVLA()
#                        class.

# Modification history:
# ---------------------
# 2012 May 10 - Nick Elias, NRAO
#               Initial version, identical behavior to FlagDeterBase.py.
# 2012 May 16 - Lindsey Davis, NRAO
#               Changed file name from FlagDeterALMA.py to flagdeteralma.py.
# 2013 May - Brian Kent, NRAO
#             New Deterministic flagging for the VLA

# ------------------------------------------------------------------------------

# Imports
# -------

from __future__ import absolute_import

#import casac

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures

from pipeline.hif.tasks.flagging import flagdeterbase 

#import pipeline.tasks.flagging.FlagDeterBase as gronk

# ------------------------------------------------------------------------------

# Initialize a logger
# -------------------

LOG = infrastructure.get_logger(__name__)

# ------------------------------------------------------------------------------
# class FlagDeterVLAInputs
# ------------------------------------------------------------------------------

# FlagDeterVLAInputs

# Description:
# ------------
# This class manages the inputs for the FlagDeterVLA() class.

# Inherited classes:
# ------------------
# FlagDeterBaseInputs - This is the base class that handles the inputs for
# deterministic flagging.

# Public member variables:
# ------------------------
# None.

# Public member functions:
# ------------------------
# __init__     - This public member function constructs an instance of the
#                FlagDeterVLAInputs() class.  It is overloaded.
# clone        - This public member function creates a cloned instance of an
#                existing instance.  It is overloaded.
# to_casa_args - This public member function translates the input parameters of
#                this class to task parameters and file-based flag commands
#                required by CASA task tflagdata.  It is overloaded.

# Static public member functions:
# -------------------------------
# create_from_context - This static public member function creates an instance
#                       of this class from a context.  It is overloaded.

# Modification history:
# ---------------------
# 2012 May 10 - Nick Elias, NRAO
#               Initial version created with public member functions __init__(),
#               clone(), and to_casa_args(); and static public member function
#               create_from_context().  All functions are overloaded.

# ------------------------------------------------------------------------------

class FlagDeterVLAInputs( flagdeterbase.FlagDeterBaseInputs ):

# ------------------------------------------------------------------------------

# FlagDeterVLAInputs::__init__

# Description:
# ------------
# This public member function constructs an instance of the
# FlagDeterVLAInputs() class.

# The primary purpose of this class is to initialize the public member
# variables.  The defaults for all parameters (except context) are None.

# NB: This public member function is overloaded.

# Inherited classes:
# ------------------
# FlagDeterBaseInputs - This class manages the inputs for the
#                       FlagDeterBaseInputs() parent class.

# Inputs to initialize the FlagDeterBaseInputs() class:
# -----------------------------------------------------
# context      - This python dictionary contains the pipeline context (state).
#                It has no default.
#
# vis          - This python string contains the MS name.
#
# output_dir   - This python string contains the output directory name.
#
# flagbackup   - This python boolean determines whether the existing flags are
#                backed up before the new flagging begins.
#
# autocorr     - This python boolean determines whether autocorrelations are
#                flagged or not.
#
# shadow       - This python boolean determines whether shadowed antennas are
#                flagged or not.
#
# scan         - This python boolean determines whether scan flagging is
#                performed.
# scannumber   - This python string contains the comma-delimited scan numbers.
#                In the task interface, it is a subparameter of the scan
#                parameter.  Standard data selection syntax is valid.
# intents      - This python string contains the comma-delimited intents.  In
#                the task interface, it is a subparameter of the scan parameter.
#                Wildcards (* character) are allowed.
#
# edgespw      - This python boolean determines whether edge channels are
#                flagged.
# fracspw      - This python float contains the fraction (between 0.0 and 1.0)
#                of channels removed from the edge for the ALMA baseline correlator.
#                In the task interface, it is a subparameter of the edgespw parameter.
#
# fracspwfps    - This python float contains the fraction (between 0.0 and 1.0)
#                of channels removed from the edge for the ACS correlator.  In the
#                task interface, it it is a subparameter of the edgespw parameter.
#
# online       - This python boolean determines whether the online flags are
#                applied.
# fileonline   - This python string contains the name of the ASCII file that
#                has the flagging commands.  It is a subparameter of the
#                online parameter.
#
# template     - This python boolean determines whether flagging templates are
#                applied.
# filetemplate - This python string contains the name of the ASCII file that
#                has the flagging template (for RFI, birdies, telluric lines,
#                etc.).  It is a subparameter of the template parameter.

# Inputs:
# -------
# None.

# Outputs:
# --------
# None, returned via the function value.

# Modification history:
# ---------------------
# 2012 May 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------
    edgespw  = basetask.property_with_default('edgespw', True)
    fracspw  = basetask.property_with_default('fracspw', 0.05)
    quack = basetask.property_with_default('quack', True)
    clip = basetask.property_with_default('clip', True)
    baseband = basetask.property_with_default('baseband', True)
    #fracspwfps  = basetask.property_with_default('fracspwfps', 0.04837)
        

    def __init__( self, context, vis=None, output_dir=None, flagbackup=None,
        autocorr=None, shadow=None, scan=None, scannumber=None, quack=None, clip=None, baseband=None,
        intents=None, edgespw=None, fracspw=None, fracspwfps=None, online=None,
        fileonline=None, template=None, filetemplate=None, hm_tbuff=None, tbuff=None ):

        # Initialize the public member variables of the inherited class
        # FlagDeterBaseInputs()
	
	super( FlagDeterVLAInputs, self ).__init__( context, vis=vis,
		    output_dir=output_dir, flagbackup=flagbackup, autocorr=autocorr,
		    shadow=shadow, scan=scan, scannumber=scannumber, intents=intents,
		    edgespw=edgespw, fracspw=fracspw, fracspwfps=fracspwfps, online=online,
		    fileonline=fileonline, template=template, filetemplate=filetemplate,
		    hm_tbuff=hm_tbuff, tbuff=tbuff)


        self._init_properties(vars())
        
    @property
    def hm_tbuff(self):
        return self._hm_tbuff
        
    @hm_tbuff.setter
    def hm_tbuff(self, value):
        if value is None:
            value = 'manual'
        if value in 'halfint | 1.5int | manual':
            self._hm_tbuff = value
        else:
            self._hm_tbuff = 'manual'
        
    @property
    def tbuff(self):
        return self._tbuff
        
    @tbuff.setter
    def tbuff(self, value):
        if value is None:
            value = 0.0
        self._tbuff = value

    
    

# ------------------------------------------------------------------------------

# FlagDeterVLAInputs::to_casa_args

# Description:
# ------------
# This public member function translates the input parameters of this class to
# task parameters and file-based flag commands required by CASA task tflagdata.

# NB: This public member function is overloaded.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The python dictionary containing the arguments (and their values) for CASA
# task tflagdata, returned via the function value.  The teend 5 percent of each spw or minimum of 3 channelsmporary file that
# contains the flagging commands for the tflagdata task, located in the output
# directory.

# Modification history:
# ---------------------
# 2012 May 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

    def to_casa_args( self ):

        # Initialize the arguments from the inherited
        # FlagDeterBaseInputs() class

        task_args = super( FlagDeterVLAInputs, self ).to_casa_args()


        # Return the tflagdata task arguments

        return task_args

# ------------------------------------------------------------------------------
# class FlagDeterVLAResults
# ------------------------------------------------------------------------------

# FlagDeterVLAResults

# Description:
# ------------
# This class manages the results from the FlagDeterVLA() class.

# Inherited classes:
# ------------------
# FlagDeterBaseResults - This class manages the results from the FlagDeterBase()
#                        class.

# Modification history:
# ---------------------
# 2012 May 10 - Nick Elias, NRAO
#               Initial version created with no new member functions.

# ------------------------------------------------------------------------------

class FlagDeterVLAResults( flagdeterbase.FlagDeterBaseResults ):
    pass

# ------------------------------------------------------------------------------
# class FlagDeterVLA
# ------------------------------------------------------------------------------

# FlagDeterVLA

# Description:
# ------------
# This class represents the pipeline interface to the CASA task flagdata.

# Inherited classes:
# ------------------
# FlagDeterBase - This class represents the pipeline interface to the CASA task
#                 flagdata.

# Public member functions:
# ------------------------
# All public member functions from the FlagDeterVLAInputs() class.

# Modification history:
# ---------------------
# 2012 May 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

class FlagDeterVLA( flagdeterbase.FlagDeterBase ):

# ------------------------------------------------------------------------------

    # Make the member functions of the FlagDeterVLAInputs() class member
    # functions of this class

    Inputs = FlagDeterVLAInputs

    def _get_flag_commands(self):
        """ Adding quack and clip
        """
        flag_cmds = super(FlagDeterVLA, self)._get_flag_commands()

        inputs = self.inputs


        # Flag mode clip
        if inputs.clip:
            flag_cmds.append('mode=clip correlation=ABS_ALL clipzeros=True')
        
        # Flag quack
        if inputs.quack: 
            flag_cmds.append(self._get_quack_cmds())
            
        # Flag end 5 percent of each spw or minimum of 3 channels
        if inputs.edgespw:
            flag_cmds.append(self._get_edgespw_cmds())
            
        # Flag 10 end channels at edges of basebands
        if inputs.baseband:
            flag_cmds.append(self._get_baseband_cmds())
        
        
            
        #print flag_cmds
        
        return flag_cmds
            
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
            
            
            # Get the data description for this spw
            dd = inputs.ms.get_data_description(spw=spw)
            if dd is None:
                LOG.debug('Missing data description for spw %s ' % spw.id)
                continue

            # Determine the number of correlations
            #   Check that they are between 1 and 4
            ncorr = len (dd.corr_axis)
            if ncorr not in set ([1, 2, 4]):
                LOG.debug('Wrong number of correlations %s for spw %s ' % (ncorr, spw.id))
                continue

            # Skip if TDM mode where TDM modes are defined to
            # be modes with <= 256 channels per correlation
            if (ncorr * spw.num_channels > 256):
                quanta = casatools.quanta
                bw_quantity = quanta.convert(quanta.quantity('1875MHz'), 'Hz')   
                bandwidth = measures.Frequency(quanta.getvalue(bw_quantity)[0],
                                               measures.FrequencyUnits.HERTZ)
                cen_freq = spw.centre_frequency
                lo_freq = cen_freq - bandwidth / 2.0
                hi_freq = cen_freq + bandwidth / 2.0
                l_max, r_min = spw.channel_range(lo_freq, hi_freq)
                r_max = spw.num_channels - 1
                
                if l_max <= 0 and r_min >= r_max:
                    LOG.debug('Skipping edge flagging for FDM spw %s ' % spw.id)
                    continue
                cmd = '{0}:0~{1};{2}~{3}'.format(spw.id, l_max, r_min, r_max)
                to_flag.append(cmd)

            else:
                
                # If the twice the number of flagged channels is greater than the
                # number of channels for a given spectral window, skip it.
                if spw.num_channels in set([62, 124, 248]):
                    frac_chan = int(round(inputs.fracspwfps * spw.num_channels))
                else:
                    frac_chan = int(round(inputs.fracspw * spw.num_channels))
                    if 2*frac_chan >= spw.num_channels:
                        LOG.debug('Too many flagged channels %s for spw %s '% (spw.num_channels, spw.id))
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
                    
                    if len(to_flag) <= 0:
                        return '# No valid edge spw flagging command'
                    else:
                        return 'mode=manual spw={0}'.format(','.join(to_flag))
                    
    
    '''
    
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
                
        edgespw_cmd = "mode=manual spw='" + SPWtoflag + "'"
                
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
        
         
         

