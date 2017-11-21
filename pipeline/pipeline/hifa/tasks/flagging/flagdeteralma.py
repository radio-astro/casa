from __future__ import absolute_import

import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.vdp as vdp
from pipeline.h.tasks.flagging import flagdeterbase

__all__ = [
    'FlagDeterALMA',
    'FlagDeterALMAInputs'
]

LOG = infrastructure.get_logger(__name__)


class FlagDeterALMAInputs(flagdeterbase.FlagDeterBaseInputs):
    """
    FlagDeterALMAInputs defines the inputs for the FlagDeterALMA pipeline task.
    """
    edgespw = vdp.VisDependentProperty(default=True)
    flagbackup = vdp.VisDependentProperty(default=True)
    fracspw = vdp.VisDependentProperty(default=0.0625)
    template = vdp.VisDependentProperty(default=True)

    # new property for ACA correlator
    fracspwfps = vdp.VisDependentProperty(default=0.048387)

    # New property for QA0 / QA2 flags
    qa0 = vdp.VisDependentProperty(default=True)
    qa2 = vdp.VisDependentProperty(default=True)

    def __init__(self, context, vis=None, output_dir=None, flagbackup=None, autocorr=None, shadow=None, scan=None,
                 scannumber=None, intents=None, edgespw=None, fracspw=None, fracspwfps=None, online=None,
                 fileonline=None, template=None, filetemplate=None, hm_tbuff=None, tbuff=None, qa0=None, qa2=None):
        super(FlagDeterALMAInputs, self).__init__(
            context, vis=vis, output_dir=output_dir, flagbackup=flagbackup, autocorr=autocorr, shadow=shadow, scan=scan,
            scannumber=scannumber, intents=intents, edgespw=edgespw, fracspw=fracspw, fracspwfps=fracspwfps,
            online=online, fileonline=fileonline, template=template, filetemplate=filetemplate, hm_tbuff=hm_tbuff,
            tbuff=tbuff)

        # solution parameters
        self.qa0 = qa0
        self.qa2 = qa2


class FlagDeterALMA(flagdeterbase.FlagDeterBase):
    Inputs = FlagDeterALMAInputs
        
    def get_fracspw(self, spw):    
        # override the default fracspw getter with our ACA-aware code
        if spw.num_channels in (62, 124, 248):
            return self.inputs.fracspwfps
        else:
            return self.inputs.fracspw
    
    def verify_spw(self, spw):
        # override the default verifier, adding an extra test that bypasses
        # flagging of TDM windows
        super(FlagDeterALMA, self).verify_spw(spw)
    
        # Skip if TDM mode where TDM modes are defined to be modes with 
        # <= 256 channels per correlation
        dd = self.inputs.ms.get_data_description(spw=spw)
        ncorr = len(dd.corr_axis)
            
        if ncorr*spw.num_channels > 256:
            raise ValueError('Skipping edge flagging for FDM spw %s' % spw.id)
        
    def _get_edgespw_cmds(self):
        # Append to the default _get_edgespw_cmds method, and flag the portions
        # of any FDM spw that extend beyond a total width of 1875 MHz, that is, 
        # all channels that lie beyond +-937.5 MHz from the mean frequency 
        # (CAS-5231)
        threshold = measures.Frequency(1875, measures.FrequencyUnits.MEGAHERTZ)
        to_flag = super(FlagDeterALMA, self)._get_edgespw_cmds()

        # loop over the spectral windows, generate a flagging command for each
        # spw in the ms. Calling get_spectral_windows() with no arguments
        # returns just the science windows, which is exactly what we want.
        for spw in self.inputs.ms.get_spectral_windows():
            try:
                # test that this spw should be flagged by assessing number of
                # correlations, TDM/FDM mode etc.
                self.verify_spw(spw)
            except ValueError:
                # this spw has more than 256 chns per corr.  Proceed with FDM flagging.
                
                # Calculate the range outside the FDM spws for all channels that lie 
                # beyond +-937.5 MHz from the mean frequency (937.5=1875/2)                
                if spw.bandwidth > threshold:
                    cen_freq = spw.centre_frequency
                    # channel range lower than threshold
                    LOG.debug('Bandwidth greater than %s. FDM edge channel '
                              'flagging', str(threshold))
                    lo_freq = cen_freq - spw.bandwidth / 2.0
                    hi_freq = cen_freq - threshold / 2.0
                    minchan_lo, maxchan_lo = spw.channel_range(lo_freq, hi_freq)
                    
                    # upper range higher than threshold
                    lo_freq = cen_freq + threshold / 2.0
                    hi_freq = cen_freq + spw.bandwidth / 2.0
                    minchan_hi, maxchan_hi = spw.channel_range(lo_freq, hi_freq)
                    
                    # Append to flag list
                    # Clean up order of channel ranges high to low
                    chan1 = '{0}~{1}'.format(minchan_lo, maxchan_lo)
                    chan2 = '{0}~{1}'.format(minchan_hi, maxchan_hi)
                    chans = [chan1, chan2]
                    chans.sort()
                    cmd = '{0}:{1};{2}'.format(spw.id, chans[0], chans[1])
                    to_flag.append(cmd)

        # Combine spw/channels, ie:
        # 16:0~119,16:3960~4079,18:0~119,18:3960~4079,20:3960~4079,20:0~119,22:3960~4079,22:0~119
        #  needs to be combined as:
        # 16:0~119;3960~4079,18:0~119;3960~4079,20:0~119;3960~4079,22:0~119;3960~4079
        return to_flag
