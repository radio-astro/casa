from __future__ import absolute_import

import os.path
import numpy as np 
import re

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
from pipeline.hif.tasks.flagging.flagdatasetter import FlagdataSetter

from .resultobjects import AtmflagResults
from ..common import commonresultobjects
from ..common import viewflaggers

LOG = infrastructure.get_logger(__name__)


class AtmflagInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None,
      intent=None, flag_minabs=None, fmin_limit=None,
      flag_nmedian=None, fnm_limit=None):

        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def intent(self):
        if self._intent is None:
            return '*AMP*,*BANDPASS*,*PHASE*'
        return self._intent

    @intent.setter
    def intent(self, value):
        self._intent = value

    @property
    def flag_minabs(self):
        if self._flag_minabs is None:
            return False
        return self._flag_minabs

    @flag_minabs.setter
    def flag_minabs(self, value):
        self._flag_minabs = value

    @property
    def fmin_limit(self):
        if self._fmin_limit is None:
            return 0
        return self._fmin_limit

    @fmin_limit.setter
    def fmin_limit(self, value):
        self._fmin_limit = value

    @property
    def flag_nmedian(self):
        if self._flag_nmedian is None:
            return False
        return self._flag_nmedian

    @flag_nmedian.setter
    def flag_nmedian(self, value):
        self._flag_nmedian = value

    @property
    def fnm_limit(self):
        if self._fnm_limit is None:
            return 0
        return self._fnm_limit

    @fnm_limit.setter
    def fnm_limit(self, value):
        self._fnm_limit = value


class Atmflag(basetask.StandardTaskTemplate):
    Inputs = AtmflagInputs

    def prepare(self):
        inputs = self.inputs

        # Construct the task that will read the data and create the
        # view of the data that is the basis for flagging.
        datainputs = AtmflagWorkerInputs(context=inputs.context,
          output_dir=inputs.output_dir,
          vis=inputs.vis)
        datatask = AtmflagWorker(datainputs)

        # Construct the task that will set any flags raised in the
        # underlying data.
        flagsetterinputs = FlagdataSetter.Inputs(context=inputs.context,
          vis=inputs.vis, table=inputs.vis, inpfile=[])
        flagsettertask = FlagdataSetter(flagsetterinputs)

	# Translate the input flagging parameters to a more compact
        # list of rules.
        rules = viewflaggers.VectorFlagger.make_flag_rules(
          flag_minabs=inputs.flag_minabs, fmin_limit=inputs.fmin_limit,
          flag_nmedian=inputs.flag_nmedian, fnm_limit=inputs.fnm_limit)
        flagger = viewflaggers.VectorFlagger

        # Construct the flagger task around the data view task  and the
        # flagger task. When executed this will:
        #   loop:
        #     execute datatask to obtain view from underlying data
        #     examine view, raise flags
        #     execute flagsetter task to set flags in underlying data        
        #     exit loop if no flags raised or if # iterations > niter 
        flaggerinputs = flagger.Inputs(
          context=inputs.context, output_dir=inputs.output_dir,
          vis=inputs.vis, datatask=datatask, flagsettertask=flagsettertask,
          rules=rules, intent=inputs.intent, niter=1)
        flaggertask = flagger(flaggerinputs)

	# Execute it to flag the data view
        result = self._executor.execute(flaggertask)
        return result

    def analyse(self, result):
        return result


class AtmflagWorkerInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None):
        self._init_properties(vars())


class AtmflagWorker(basetask.StandardTaskTemplate):
    Inputs = AtmflagWorkerInputs

    def __init__(self, inputs):
        super(AtmflagWorker, self).__init__(inputs)
        self.result = AtmflagResults(vis=inputs.vis)

    def prepare(self):
        inputs = self.inputs

        # Get the MS object.
        ms = self.inputs.context.observing_run.get_ms(name=inputs.vis)

        # and from it the science spectral windows
        spws = ms.get_spectral_windows(science_windows_only=True)
        centre_freq = []
        width = []
        resolution = []
        spw_to_band = {}

        for i,spw in enumerate(spws):
            spw_to_band[spw.id] = i

            channels = spw.channels

            # translate from Stewart's domain measures to CASA quantities
            freqs = np.zeros([len(channels)], np.double)
            channel_unit = channels[0].low.units['symbol']
            freq = np.zeros([len(channels)], np.double)
            channel_width = np.zeros([len(channels)], np.double)
            res = np.zeros([len(channels)], np.double)
            for i in np.arange(len(freqs)):
                freq[i] = float(channels[i].low.value + channels[i].high.value) / 2.0
                channel_width[i] = float(channels[i].low.value - channels[i].high.value)
                res[i] = float(channels[i].low.value - channels[i].high.value)
      
            centre_frequency = spw.centre_frequency
            centre_freq.append(float(centre_frequency.value))
            centre_freq_unit = centre_frequency.units['symbol']
            resolution.append(abs(channel_width[0]))
            resolution_unit = channel_unit
            bandwidth = spw.bandwidth
            width.append(float(bandwidth.value))
            width_unit = bandwidth.units['symbol']

        airmass = 1.0
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

        casatools.atmosphere.initAtmProfile(humidity=H, 
          temperature=casatools.quanta.quantity(T,"K"),
          altitude=casatools.quanta.quantity(5059,"m"),
          pressure=casatools.quanta.quantity(P,'mbar'), 
          atmType=midLatitudeWinter)
        casatools.atmosphere.initSpectralWindow(len(centre_freq), fcentre,
          fwidth, fresolution)
        casatools.atmosphere.setUserWH2O(casatools.quanta.quantity(pwv,'mm'))

        for spw in spws:
            band = spw_to_band[spw.id]

            numchan = casatools.atmosphere.getNumChan(band)
            refchan = casatools.atmosphere.getRefChan(band)
            reffreq = casatools.atmosphere.getRefFreq(band)
            reffreq = casatools.quanta.convert(reffreq, 'GHz')
            chansep = casatools.atmosphere.getChanSep(band)
            chansep = casatools.quanta.convert(chansep, 'GHz')

            freqs = np.zeros([numchan], np.double)
            for i in np.arange(numchan):
                freqs[i] = reffreq['value'][0] + float(i - refchan) * \
                  chansep['value'][0] 
            axis = commonresultobjects.ResultAxis(name='Frequency',
              units='GHz', data=freqs)

            dry = np.array(casatools.atmosphere.getDryOpacitySpec(band)[1])
            wet = np.array(casatools.atmosphere.getWetOpacitySpec(
              band)[1]['value'])

            transmission = np.exp(-airmass*(wet+dry))

            transmission = commonresultobjects.SpectrumResult(
              axis=axis,
              data=transmission,
              datatype='zenith transmission',
              spw=spw.id,
              filename=inputs.vis)

            self.result.addview(transmission.description,
              transmission)

        return self.result

    def analyse(self, result):
        return result
