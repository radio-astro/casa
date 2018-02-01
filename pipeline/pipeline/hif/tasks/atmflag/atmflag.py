from __future__ import absolute_import

import os

import numpy as np

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.vdp as vdp
from pipeline.h.tasks.common import commonresultobjects
from pipeline.h.tasks.common import viewflaggers
from pipeline.h.tasks.flagging.flagdatasetter import FlagdataSetter
from pipeline.infrastructure import task_registry
from .resultobjects import AtmflagResults, AtmflagDataResults, AtmflagViewResults

__all__ = [
    'Atmflag',
    'AtmflagInputs'
]

LOG = infrastructure.get_logger(__name__)


class AtmflagInputs(vdp.StandardInputs):
    """
    AtmflagInputs defines the inputs for the Atmflag pipeline task.
    """
    flag_minabs = vdp.VisDependentProperty(default=False)
    flag_nmedian = vdp.VisDependentProperty(default=False)
    fmin_limit = vdp.VisDependentProperty(default=0.1)
    fnm_lo_limit = vdp.VisDependentProperty(default=0.5)
    fnm_hi_limit = vdp.VisDependentProperty(default=1E9)
    intent = vdp.VisDependentProperty(default='*AMP*,*BANDPASS*,*PHASE*')

    def __init__(self, context, output_dir=None, vis=None, intent=None, flag_minabs=None, fmin_limit=None,
                 flag_nmedian=None, fnm_lo_limit=None, fnm_hi_limit=None):
        super(AtmflagInputs, self).__init__()

        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis
        self.output_dir = output_dir

        # data selection arguments
        self.intent = intent

        # flagging parameters
        self.flag_minabs = flag_minabs
        self.fmin_limit = fmin_limit
        self.flag_nmedian = flag_nmedian
        self.fnm_lo_limit = fnm_lo_limit
        self.fnm_hi_limit = fnm_hi_limit


@task_registry.set_equivalent_casa_task('hif_atmflag')
class Atmflag(basetask.StandardTaskTemplate):
    Inputs = AtmflagInputs

    def prepare(self):
        inputs = self.inputs

        # Initialize result and store vis in result
        result = AtmflagResults(vis=inputs.vis)

        # Construct the task that will prepare/read the data necessary to
        # create a flagging view.
        datainputs = AtmflagDataInputs(context=inputs.context, vis=inputs.vis)
        datatask = AtmflagData(datainputs)

        # Construct the generator that will create the views of the data
        # that is the basis for flagging.
        viewtask = AtmflagView(context=inputs.context)

        # Construct the task that will set any flags raised in the
        # underlying data.
        flagsetterinputs = FlagdataSetter.Inputs(
            context=inputs.context, vis=inputs.vis, table=inputs.vis,
            inpfile=[])
        flagsettertask = FlagdataSetter(flagsetterinputs)

        # Define which type of flagger to use.
        flagger = viewflaggers.VectorFlagger

        # Translate the input flagging parameters to a more compact
        # list of rules.
        rules = flagger.make_flag_rules(
            flag_minabs=inputs.flag_minabs, fmin_limit=inputs.fmin_limit,
            flag_nmedian=inputs.flag_nmedian, fnm_lo_limit=inputs.fnm_lo_limit,
            fnm_hi_limit=inputs.fnm_hi_limit)

        # Construct the flagger task around the data view task  and the
        # flagger task.
        flaggerinputs = flagger.Inputs(
            context=inputs.context, output_dir=inputs.output_dir,
            vis=inputs.vis, datatask=datatask, viewtask=viewtask,
            flagsettertask=flagsettertask, rules=rules, niter=1)
        flaggertask = flagger(flaggerinputs)

        # Execute the flagger task.
        flaggerresult = self._executor.execute(flaggertask)

        # Import views, flags, and "measurement set or caltable to flag"
        # into final result
        result.importfrom(flaggerresult)

        # Copy flagging summaries to final result
        result.summaries = flaggerresult.summaries

        return result

    def analyse(self, result):
        return result


class AtmflagDataInputs(vdp.StandardInputs):
    def __init__(self, context, vis=None):
        super(AtmflagDataInputs, self).__init__()

        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis


class AtmflagData(basetask.StandardTaskTemplate):
    Inputs = AtmflagDataInputs

    def __init__(self, inputs):
        super(AtmflagData, self).__init__(inputs)

    def prepare(self):
        """
        Atmflag operates directly on the MS and requires no extra data tables
        to be created.
        """
        result = AtmflagDataResults(vis=self.inputs.vis)
        result.table = self.inputs.vis

        return result

    def analyse(self, result):
        return result


class AtmflagView(object):
    def __init__(self, context):
        self.context = context

    def __call__(self, data):
        # Initialize result structure
        self.result = AtmflagViewResults(vis=data.table)

        # Atmflag operates on the MS, does not require anything from the
        # data task result.
        if data.table is not None:
            LOG.info('Computing flagging views for {0}'.format(
                os.path.basename(data.table)))
            self.calculate_view(data.table)

        return self.result

    def calculate_view(self, table):
        """
        Method to calculate the flagging view.

        Input parameters:

        table  -- Name of the MS to be analysed.
        """

        # Get the MS object.
        ms = self.context.observing_run.get_ms(name=table)

        # and from it the science spectral windows
        spws = ms.get_spectral_windows(science_windows_only=True)
        centre_freq = []
        width = []
        resolution = []
        spw_to_band = {}

        for ispw, spw in enumerate(spws):
            spw_to_band[spw.id] = ispw

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

        casatools.atmosphere.initAtmProfile(
            humidity=H,
            temperature=casatools.quanta.quantity(T, "K"),
            altitude=casatools.quanta.quantity(5059, "m"),
            pressure=casatools.quanta.quantity(P, 'mbar'),
            atmType=midLatitudeWinter)
        casatools.atmosphere.initSpectralWindow(
            len(centre_freq), fcentre, fwidth, fresolution)
        casatools.atmosphere.setUserWH2O(casatools.quanta.quantity(pwv, 'mm'))

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
                freqs[i] = reffreq['value'][0] + \
                           float(i - refchan) * chansep['value'][0]
            axis = commonresultobjects.ResultAxis(name='Frequency',
                                                  units='GHz', data=freqs)

            dry = np.array(casatools.atmosphere.getDryOpacitySpec(band)[1])
            wet = np.array(casatools.atmosphere.getWetOpacitySpec(
                band)[1]['value'])

            transmission = np.exp(-airmass * (wet + dry))

            transmission = commonresultobjects.SpectrumResult(
                axis=axis,
                data=transmission,
                datatype='zenith transmission',
                spw=spw.id,
                filename=table)

            self.result.addview(transmission.description,
                                transmission)
