from __future__ import absolute_import

import numpy as np
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from pipeline.hif.tasks.common import commonresultobjects

LOG = infrastructure.get_logger(__name__)


class AtmHeuristics(object):
    def __init__(self, context, vis):
        self.context = context
        self.vis = vis
        self.calculated = False

        ms = context.observing_run.get_ms(name=vis)
        self.science_spws = ms.get_spectral_windows(science_windows_only=True)

    def _calculate(self):
        # get channel information for each spw
        centre_freq = []
        width = []
        resolution = []
        spw_to_band = {}

        for i,spw in enumerate(self.science_spws):
            spw_to_band[spw.id] = i

            channels = spw.channels

            # translate from Stewart's domain measures to CASA quantities
            freqs = np.zeros([len(channels)], np.double)
            channel_unit = channels[0].low.units['symbol']
            freq = np.zeros([len(channels)], np.double)
            channel_width = np.zeros([len(channels)], np.double)
            res = np.zeros([len(channels)], np.double)

            for i in np.arange(len(freqs)):
                freq[i] = float(channels[i].low.value + channels[i].high.value) /\
                  2.0
                channel_width[i] = float(channels[i].low.value -
                  channels[i].high.value)
                res[i] = float(channels[i].low.value - channels[i].high.value)
      
            centre_frequency = spw.centre_frequency
            centre_freq.append(float(centre_frequency.value))
            centre_freq_unit = centre_frequency.units['symbol']
            resolution.append(abs(channel_width[0]))
            resolution_unit = channel_unit
            bandwidth = spw.bandwidth
            width.append(float(bandwidth.value))
            width_unit = bandwidth.units['symbol']

        # canonical atmospheric params
        P = 563.0
        H = 20.0
        T = 273.0
        pwv = 1.0

        tropical = 1
        midLatitudeSummer = 2
        midLatitudeWinter = 3

        fcentre = casatools.quanta.quantity(centre_freq, centre_freq_unit)
        fresolution = casatools.quanta.quantity(resolution, resolution_unit)
        fwidth = casatools.quanta.quantity(width, width_unit)

        # setup atm
        casatools.atmosphere.initAtmProfile(humidity=H, 
          temperature=casatools.quanta.quantity(T,"K"),
          altitude=casatools.quanta.quantity(5059,"m"),
          pressure=casatools.quanta.quantity(P,'mbar'), 
          atmType=midLatitudeWinter)
        casatools.atmosphere.initSpectralWindow(len(centre_freq), fcentre,
          fwidth, fresolution)
        casatools.atmosphere.setUserWH2O(casatools.quanta.quantity(pwv,'mm'))

        self.opacities = {}
        for spw in self.science_spws:
            band = spw_to_band[spw.id]

            numchan = casatools.atmosphere.getNumChan(band)
            refchan = casatools.atmosphere.getRefChan(band)
            reffreq = casatools.atmosphere.getRefFreq(band)
            reffreq = casatools.quanta.convert(reffreq, 'GHz')
            chansep = casatools.atmosphere.getChanSep(band)
            chansep = casatools.quanta.convert(chansep, 'GHz')

            # axis object describing channel/freq axis
            freqs = np.zeros([numchan], np.double)
            for i in np.arange(numchan):
                freqs[i] = reffreq['value'][0] + float(i - refchan) * \
                  chansep['value'][0] 
            axis = commonresultobjects.ResultAxis(name='Frequency',
              units='GHz', data=freqs)

            # calculate opacities
            dry = np.array(casatools.atmosphere.getDryOpacitySpec(band)[1])
            wet = np.array(casatools.atmosphere.getWetOpacitySpec(band)[1]\
              ['value'])

            # object containing result
            opacity = commonresultobjects.SpectrumResult(
              axis=axis,
              data=wet+dry,
              datatype='opacity',
              spw=spw.id)

            self.opacities[spw.id] = opacity

        self.calculated = True

    def highest_opacity_spwid(self):
        if not self.calculated:
            self._calculate()

        median_opacity = np.zeros([len(self.science_spws)])
        spw_ids = np.array([spw.id for spw in self.science_spws])

        for ispw, spw_id in enumerate(spw_ids):
            median_opacity[ispw] = np.median(self.opacities[spw_id].data)

        result = str(spw_ids[np.argmax(median_opacity)])

        for ispw, spw_id in enumerate(spw_ids):
            LOG.info('spw: %s median opacity: %s' % (spw_id, median_opacity[ispw]))
        LOG.info('selected spw: %s' % result)

        return result

    def highest_freq_spwid(self):
        """Return the spw id of the science spw with highest centre
           frequency.
        """
        # construction of spw_freqs assumes freqs for all spws have
        # in same units
        spw_freqs = [float(spw.centre_frequency.value) for spw in 
          self.science_spws]
        spw_ids = [spw.id for spw in self.science_spws]
            
        result = str(spw_ids[np.argmax(spw_freqs)])

        for ispw, spw_id in enumerate(spw_ids):
            LOG.info('spw: %s median opacity: %s' % (spw_id, spw_freqs[ispw]))
        LOG.info('selected spw: %s' % result)

        return result
