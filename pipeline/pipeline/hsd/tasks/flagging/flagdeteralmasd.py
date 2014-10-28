
# Imports
# -------

from __future__ import absolute_import

import collections

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures

from pipeline.hif.tasks.flagging import flagdeterbase 


# ------------------------------------------------------------------------------

# Initialize a logger
# -------------------

LOG = infrastructure.get_logger(__name__)

# ------------------------------------------------------------------------------

class FlagDeterALMASingleDishInputs(flagdeterbase.FlagDeterBaseInputs):


    edgespw = basetask.property_with_default('edgespw', True)
    fracspw = basetask.property_with_default('fracspw', 0.0625)
    template = basetask.property_with_default('template', True)

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None, output_dir=None, flagbackup=None,
                 autocorr=None, shadow=None, scan=None, scannumber=None,
                 intents=None, edgespw=None, fracspw=None, fracspwfps=None, online=None,
                 fileonline=None, template=None, filetemplate=None, hm_tbuff=None, tbuff=None):

        # Initialize the public member variables of the inherited class
        # FlagDeterBaseInputs()

        super(FlagDeterALMASingleDishInputs, self).__init__(context, vis=vis,
            output_dir=output_dir, flagbackup=flagbackup, autocorr=autocorr,
		    shadow=shadow, scan=scan, scannumber=scannumber, intents=intents,
		    edgespw=edgespw, fracspw=fracspw, fracspwfps=fracspwfps, online=online,
		    fileonline=fileonline, template=template, filetemplate=filetemplate,
		    hm_tbuff=hm_tbuff, tbuff=tbuff)

        self.fracspwfps = fracspwfps

    @property
    def fracspwfps(self):
        return self._fracspwfps

    @fracspwfps.setter
    def fracspwfps(self, value):
        if value is None:
            value = 0.048387
        self._fracspwfps = value



    def to_casa_args(self):

        # Initialize the arguments from the inherited
        # FlagDeterBaseInputs() class

        task_args = super(FlagDeterALMASingleDishInputs, self).to_casa_args()


        # Return the tflagdata task arguments

        return task_args



class FlagDeterALMASingleDishResults(flagdeterbase.FlagDeterBaseResults):
    pass



class FlagDeterALMASingleDish(flagdeterbase.FlagDeterBase):

# ------------------------------------------------------------------------------

    # Make the member functions of the FlagDeterALMASingleDishInputs() class member
    # functions of this class

    Inputs = FlagDeterALMASingleDishInputs
    
    # Flag edge channels if bandwidth exceeds bandwidth_limit
    # Currently, default bandwidth limit is set to 1.875GHz but it is 
    # controllable via parameter 'fracspw' 
    @property
    def bandwidth_limit(self):
        if isinstance(self.inputs.fracspw, str):
            return casatools.quanta.convert(self.inputs.fracspw, 'Hz')['value']
        else:
            return 1.875e9 # 1.875GHz
    
    def _get_edgespw_cmds(self):
        inputs = self.inputs
        
        if isinstance(inputs.fracspw, float) or isinstance(inputs.fracspw, str):
            to_flag = super(FlagDeterALMASingleDish, self)._get_edgespw_cmds()
        elif isinstance(inputs.fracspw, collections.Iterable):
            # inputs.fracspw is iterable indicating that the user want to flag 
            # edge channels with different fractions/number of channels for 
            # left and right edges
            

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
                fracspw_org = inputs.fracspw
                try:
                    fracspw_list = []
                    for _frac in fracspw_org:
                        inputs.fracspw = _frac
                        fracspw_list.append(self.get_fracspw(spw))
                finally:
                    inputs.fracspw = fracspw_org
                if len(fracspw_list) == 0:
                    continue
                elif len(fracspw_list) == 1:
                    fracspw_list.append(fracspw_list[0])
    
                # If the twice the number of flagged channels is greater than the
                # number of channels for a given spectral window, skip it.
                #frac_chan = int(round(fracspw * spw.num_channels + 0.5))
                # Make rounding less agressive
                frac_chan_list = map(lambda x: int(round(x * spw.num_channels)), fracspw_list)[:2]
                if sum(frac_chan_list) >= spw.num_channels:
                    LOG.debug('Too many flagged channels %s for spw %s '
                              '' % (spw.num_channels, spw.id))
                    continue
    
                # calculate the channel ranges to flag. No need to calculate the
                # left minimum as it is always channel 0.
                l_max = frac_chan_list[0] - 1
                #r_min = spw.num_channels - frac_chan - 1
                # Fix asymmetry 
                r_min = spw.num_channels - frac_chan_list[1]
                r_max = spw.num_channels - 1
    
                # state the spw and channels to flag in flagdata format, adding
                # the statement to the list of flag commands
                if l_max >= 0 and r_max >= r_min:
                    cmd = '{0}:0~{1};{2}~{3}'.format(spw.id, l_max, r_min, r_max)
                elif l_max < 0:
                    cmd = '{0}:{1}~{2}'.format(spw.id, r_min, r_max)
                elif r_max < r_min:
                    cmd = '{0}:0~{1}'.format(spw.id, l_max)
                else:
                    cmd = ''
                    continue
                to_flag.append(cmd)
                
            LOG.debug('list type edge fraction specification for spw %s' % spw.id)
            LOG.debug('cmd=\'%s\'' % cmd)

        return to_flag
    
    def get_fracspw(self, spw):    
        # override the default fracspw getter with our ACA-aware code
        #if spw.num_channels in (62, 124, 248):
        #    return self.inputs.fracspwfps
        #else:
        #    return self.inputs.fracspw
        if isinstance(self.inputs.fracspw, float):
            return self.inputs.fracspw
        elif isinstance(self.inputs.fracspw, str):
            LOG.debug('bandwidth limited edge flagging for spw %s' % spw.id)
            bandwidth_limit = self.bandwidth_limit
            bandwidth = float(spw.bandwidth.value)
            fracspw = 0.5 * (bandwidth - bandwidth_limit) / bandwidth
            LOG.debug('fraction is %s' % fracspw)
            return max(0.0, fracspw)

    def verify_spw(self, spw):
        # override the default verifier, adding bandwidth check
        super(FlagDeterALMASingleDish, self).verify_spw(spw)

        # Skip if TDM mode where TDM modes are defined to be modes with 
        # <= 256 channels per correlation
        #dd = self.inputs.ms.get_data_description(spw=spw)
        #ncorr = len(dd.corr_axis)
        #if ncorr * spw.num_channels > 256:
        #    raise ValueError('Skipping edge flagging for FDM spw %s' % spw.id)

        # Skip if edge channel flagging is based on bandwidth limit, and 
        # bandwidth is less than bandwidth limit
        if isinstance(self.inputs.fracspw, str) and spw.bandwidth.value <= self.bandwidth_limit:
            raise ValueError('Skipping edge flagging for spw %s' % spw.id)

