from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.renderer.logger as logger
from pipeline.h.tasks.flagging.flagdatasetter import FlagdataSetter
from pipeline.hif.tasks import applycal
from pipeline.hif.tasks import correctedampflag
from pipeline.hif.tasks import gaincal
from pipeline.hifa.tasks import bandpass
from pipeline.infrastructure import casa_tasks
from .resultobjects import BandpassflagResults

LOG = infrastructure.get_logger(__name__)


class BandpassflagInputs(basetask.StandardInputs):

    # FIXME: propagate more bandpass parameters
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, field=None,
                 spw=None, refant=None, hm_phaseup=None, phaseupbw=None,
                 phaseupsolint=None, phaseupsnr=None, phaseupnsols=None,
                 hm_bandpass=None, solint=None, maxchannels=None,
                 evenbpints=None, bpsnr=None, bpnsols=None,
                 antnegsig=None, antpossig=None,
                 toomanyantbasedintfracthr=None, toomanyintfracthr=None,
                 toomanyblfracthr=None, antblnegsig=None, antblpossig=None,
                 relaxed_factor=None):
        self._init_properties(vars())

    @property
    def field(self):
        if self._field is not None:
            return self._field

        if isinstance(self.vis, list):
            return self._handle_multiple_vis('field')

        # By default, return the fields corresponding to the BANDPASS intent.
        fieldids = [str(field.id) for field in self.ms.fields
                    if 'BANDPASS' in field.intents]
        return ','.join(fieldids)

    @field.setter
    def field(self, value):
        self._field = value

    @property
    def spw(self):
        if self._spw is not None:
            return str(self._spw)

        if isinstance(self.vis, list):
            return self._handle_multiple_vis('spw')

        science_spws = self.ms.get_spectral_windows(
            self._spw, with_channels=True)
        return ','.join([str(spw.id) for spw in science_spws])

    @spw.setter
    def spw(self, value):
        self._spw = value

    # TODO: allow refant as parameter?

    #
    # Parameters for bandpass task.
    #

    @property
    def hm_phaseup(self):
        return self._hm_phaseup

    @hm_phaseup.setter
    def hm_phaseup(self, value):
        if value is None:
            value = 'snr'
        self._hm_phaseup = value

    @property
    def phaseupbw(self):
        return self._phaseupbw

    @phaseupbw.setter
    def phaseupbw(self, value):
        if value is None:
            value = ''
        self._phaseupbw = value

    @property
    def phaseupsolint(self):
        return self._phaseupsolint

    @phaseupsolint.setter
    def phaseupsolint(self, value):
        if value is None:
            value = 'int'
        self._phaseupsolint = value

    @property
    def phaseupsnr(self):
        return self._phaseupsnr

    @phaseupsnr.setter
    def phaseupsnr(self, value):
        if value is None:
            value = 20.0
        self._phaseupsnr = value

    @property
    def phaseupnsols(self):
        return self._phaseupnsols

    @phaseupnsols.setter
    def phaseupnsols(self, value):
        if value is None:
            value = 2
        self._phaseupnsols = value

    # Bandpass heuristics, options are 'fixed', 'smoothed', and 'snr'
    @property
    def hm_bandpass(self):
        return self._hm_bandpass

    @hm_bandpass.setter
    def hm_bandpass(self, value):
        if value is None:
            value = 'snr'
        self._hm_bandpass = value

    @property
    def solint(self):
        return self._solint

    @solint.setter
    def solint(self, value):
        if value is None:
            value = 'inf'
        self._solint = value

    @property
    def maxchannels(self):
        return self._maxchannels

    @maxchannels.setter
    def maxchannels(self, value):
        if value is None:
            value = 240
        self._maxchannels = value

    @property
    def evenbpints(self):
        return self._evenbpints

    @evenbpints.setter
    def evenbpints(self, value):
        if value is None:
            value = True
        self._evenbpints = value

    @property
    def bpsnr(self):
        return self._bpsnr

    @bpsnr.setter
    def bpsnr(self, value):
        if value is None:
            value = 50.0
        self._bpsnr = value

    @property
    def bpnsols(self):
        return self._bpnsols

    @bpnsols.setter
    def bpnsols(self, value):
        if value is None:
            value = 8
        self._bpnsols = value

    #
    # Parameters for correctedampflag task.
    #

    # Lower sigma threshold for identifying outliers as a result of bad
    # antennas within individual timestamps; equivalent to:
    # relaxationSigma
    @property
    def antnegsig(self):
        return self._antnegsig

    @antnegsig.setter
    def antnegsig(self, value):
        if value is None:
            value = 8.0
        self._antnegsig = value

    # Upper sigma threshold for identifying outliers as a result of bad
    # antennas within individual timestamps; equivalent to:
    # positiveSigmaAntennaBased
    @property
    def antpossig(self):
        return self._antpossig

    @antpossig.setter
    def antpossig(self, value):
        if value is None:
            value = 5.8
        self._antpossig = value

    # Threshold for maximum fraction of timestamps that are allowed
    # to contain outliers; equivalent to:
    # checkForAntennaBasedBadIntegrations
    @property
    def toomanyantbasedintfracthr(self):
        return self._toomanyantbasedintfracthr

    @toomanyantbasedintfracthr.setter
    def toomanyantbasedintfracthr(self, value):
        if value is None:
            value = 0.06
        self._toomanyantbasedintfracthr = value

    # Initial threshold for maximum fraction of "outlier timestamps" over
    # "total timestamps" that a baseline may be a part of; equivalent to:
    # tooManyIntegrationsFraction
    @property
    def toomanyintfracthr(self):
        return self._toomanyintfracthr

    @toomanyintfracthr.setter
    def toomanyintfracthr(self, value):
        if value is None:
            value = 0.09
        self._toomanyintfracthr = value

    # Initial threshold for maximum fraction of "bad baselines" over "all
    # timestamps" that an antenna may be a part of; equivalent to:
    # tooManyBaselinesFraction
    @property
    def toomanyblfracthr(self):
        return self._toomanyblfracthr

    @toomanyblfracthr.setter
    def toomanyblfracthr(self, value):
        if value is None:
            value = 0.18
        self._toomanyblfracthr = value

    # Lower sigma threshold for identifying outliers as a result of "bad
    # baselines" and/or "bad antennas" within baselines (across all
    # timestamps); equivalent to:
    # catchNegativeOutliers['scalardiff']
    @property
    def antblnegsig(self):
        return self._antblnegsig

    @antblnegsig.setter
    def antblnegsig(self, value):
        if value is None:
            value = 3.7
        self._antblnegsig = value

    # Upper sigma threshold for identifying outliers as a result of "bad
    # baselines" and/or "bad antennas" within baselines (across all
    # timestamps); equivalent to:
    # flag_nsigma['scalardiff']
    @property
    def antblpossig(self):
        return self._antblpossig

    @antblpossig.setter
    def antblpossig(self, value):
        if value is None:
            value = 3.0
        self._antblpossig = value

    # Relaxed value to set the threshold scaling factor to under certain
    # conditions; equivalent to:
    # relaxationFactor
    @property
    def relaxed_factor(self):
        return self._relaxed_factor

    @relaxed_factor.setter
    def relaxed_factor(self, value):
        if value is None:
            value = 2.0
        self._relaxed_factor = value


class Bandpassflag(basetask.StandardTaskTemplate):
    Inputs = BandpassflagInputs

    def prepare(self):
        inputs = self.inputs

        # Initialize results.
        result = BandpassflagResults()

        # Store the vis in the result
        result.vis = inputs.vis
        result.plots = dict()

        # Make "before calibration" plots for the weblog
        LOG.info('Creating "before calibration" plots')
        result.plots['raw'] = self.create_plots('raw')

        # Create back-up of flags.
        LOG.info('Creating back-up of flagging state')
        flag_backup_name = 'before_bpflag'
        task = casa_tasks.flagmanager(
            vis=inputs.vis, mode='save', versionname=flag_backup_name)
        self._executor.execute(task)

        # Export the current calstate.
        LOG.info('Creating back-up of calibration state')
        calstate_backup_name = 'before_bpflag.calstate'
        inputs.context.callibrary.export(calstate_backup_name)

        # Do standard phaseup and bandpass calibration.
        LOG.info('Creating initial phased-up bandpass calibration.')
        bpinputs = bandpass.ALMAPhcorBandpass.Inputs(
            context=inputs.context, vis=inputs.vis,
            hm_phaseup=inputs.hm_phaseup, phaseupbw=inputs.phaseupbw,
            phaseupsnr=inputs.phaseupsnr, phaseupnsols=inputs.phaseupnsols,
            phaseupsolint=inputs.phaseupsolint, hm_bandpass=inputs.hm_bandpass,
            solint=inputs.solint, maxchannels=inputs.maxchannels,
            evenbpints=inputs.evenbpints, bpsnr=inputs.bpsnr,
            bpnsols=inputs.bpnsols)
        bptask = bandpass.ALMAPhcorBandpass(bpinputs)
        bpresult = self._executor.execute(bptask)

        # Add the phase-up table produced by the bandpass task to the callibrary.
        LOG.debug('Adding phase-up and bandpass table to temporary context.')
        for prev_result in bpresult.preceding:
            for calapp in prev_result:
                inputs.context.callibrary.add(calapp.calto, calapp.calfrom)
        # Accept the bandpass result to add the bandpass table to the callibrary.
        # FIXME: is this ok to do, or should it just be a manual add of caltable to
        # FIXME: callibrary? (does accept also cause QA to be created?)
        bpresult.accept(inputs.context)

        # Do amplitude solve on scan interval.
        LOG.info('Create amplitude gaincal table.')
        gacalinputs = gaincal.GTypeGaincal.Inputs(
            context=inputs.context, vis=inputs.vis, intent='BANDPASS',
            gaintype='T', antenna='', calmode='a', solint='inf')
        gacaltask = gaincal.GTypeGaincal(gacalinputs)
        gacalresult = self._executor.execute(gacaltask, merge=True)

        # Ensure that flags that may be set by applycal are restored, even in
        # case of exceptions.
        LOG.info('Applying phase-up, bandpass, and amplitude cal tables.')
        try:
            # TODO: compare input pars with those in proposed recipe.
            # Apply the calibrations.
            acinputs = applycal.IFApplycalInputs(
                context=inputs.context, vis=inputs.vis, field='',
                intent='BANDPASS', flagsum=False, flagbackup=False)
            actask = applycal.IFApplycal(acinputs)
            acresult = self._executor.execute(actask)

        finally:
            # Restore flags that may have come from applycal.
            LOG.info('Restoring back-up of flagging state.')
            task = casa_tasks.flagmanager(
                vis=inputs.vis, mode='restore', versionname=flag_backup_name)
            self._executor.execute(task)

        # Make "after calibration, before flagging" plots for the weblog
        LOG.info('Creating "after calibration, before flagging" plots')
        result.plots['before'] = self.create_plots('before')

        # Find amplitude outliers and flag data
        LOG.info('Run correctedampflag to identify outliers to flag.')
        cafinputs = correctedampflag.Correctedampflag.Inputs(
            context=inputs.context, vis=inputs.vis, intent='*BANDPASS*',
            field=inputs.field, spw=inputs.spw, antnegsig=inputs.antnegsig,
            antpossig=inputs.antpossig,
            toomanyantbasedintfracthr=inputs.toomanyantbasedintfracthr,
            toomanyintfracthr=inputs.toomanyintfracthr,
            toomanyblfracthr=inputs.toomanyblfracthr,
            antblnegsig=inputs.antblnegsig,
            antblpossig=inputs.antblpossig,
            relaxed_factor=inputs.relaxed_factor)
        caftask = correctedampflag.Correctedampflag(cafinputs)
        cafresult = self._executor.execute(caftask)

        # If flags were found in the bandpass calibrator, apply these.
        # TODO: add before/after summary?
        cafflags = cafresult.flagcmds()
        if cafflags:
            LOG.info('Applying newly found flags.')
            fsinputs = FlagdataSetter.Inputs(
                context=inputs.context, vis=inputs.vis, table=inputs.vis,
                inpfile=[])
            fstask = FlagdataSetter(fsinputs)
            fstask.flags_to_set(cafflags)
            fsresult = self._executor.execute(fstask)

        # Make "after calibration, after flagging" plots for the weblog
        LOG.info('Creating "after calibration, after flagging" plots')
        result.plots['after'] = self.create_plots('after')

        # Import the calstate before BPFLAG
        LOG.info('Restoring back-up of calibration state.')
        inputs.context.callibrary.import_state(calstate_backup_name)

        # If flags were found in the bandpass calibrator.
        if cafflags:
            LOG.info('Creating final phased-up bandpass calibration.')
            # TODO: does this preserve the previous table?
            # Recompute the phase-up and bandpass calibration table.
            bpinputs = bandpass.ALMAPhcorBandpass.Inputs(
                context=inputs.context, vis=inputs.vis,
                hm_phaseup=inputs.hm_phaseup, phaseupbw=inputs.phaseupbw,
                phaseupsnr=inputs.phaseupsnr, phaseupnsols=inputs.phaseupnsols,
                phaseupsolint=inputs.phaseupsolint, hm_bandpass=inputs.hm_bandpass,
                solint=inputs.solint, maxchannels=inputs.maxchannels,
                evenbpints=inputs.evenbpints, bpsnr=inputs.bpsnr,
                bpnsols=inputs.bpnsols)
            bptask = bandpass.ALMAPhcorBandpass(bpinputs)
            bpresult = self._executor.execute(bptask)

        # TODO: decide what to add to result.
        #  - plots
        #  - before/after flagging summary ?
        #  - store entire bpresult as child?
        #  - store both initial and final bpresult?
        result.import_bpresult(bpresult)
        result.import_cafresult(cafresult)

        return result

    def analyse(self, result):
        return result

    def create_plots(self, prefix):

        plots = {'time': [],
                 'uvdist': []}

        for spw in self.inputs.spw.split(','):
            title = 'BANDPASS-amp_vs_uvdist_' + prefix + '_spw' + spw
            plotfile = os.path.basename(self.inputs.vis)+'-'+title+'.png'
            task_args = {
                'vis': self.inputs.vis,
                'xaxis': 'uvdist',
                'yaxis': 'amp',
                'field': '',
                'title': title,
                'plotfile': plotfile,
                'plotrange': [0, 0, 0, 0],
                'avgscan': False,
                'avgchannel': '9000',
                'showgui': False,
                'clearplots': True,
                'avgbaseline': False,
                'ydatacolumn': 'data',
                'intent': 'CALIBRATE_BANDPASS#ON_SOURCE',
                'spw': spw,
                'correlation': 'XX,YY',
                'overwrite': True,
                'coloraxis': 'corr'}

            task = casa_tasks.plotms(**task_args)

            if not os.path.exists(plotfile):
                self._executor.execute(task)

            plots['uvdist'].append(
                logger.Plot(
                    plotfile, x_axis='UV Dist', y_axis='Amp',
                    parameters={'vis', self.inputs.vis}, command=str(task)))

            title = 'BANDPASS-amp_vs_time_' + prefix + '_spw' + spw
            plotfile = os.path.basename(self.inputs.vis)+'-'+title+'.png'
            task_args = {
                'vis': self.inputs.vis,
                'xaxis': 'time',
                'yaxis': 'amp',
                'field': '',
                'title': title,
                'plotfile': plotfile,
                'plotrange': [0, 0, 0, 0],
                'avgscan': False,
                'avgchannel': '9000',
                'showgui': False,
                'clearplots': True,
                'avgbaseline': False,
                'ydatacolumn': 'data',
                'intent': 'CALIBRATE_BANDPASS#ON_SOURCE',
                'spw': spw,
                'correlation': 'XX,YY',
                'overwrite': True,
                'coloraxis': 'corr'}

            task = casa_tasks.plotms(**task_args)

            if not os.path.exists(plotfile):
                self._executor.execute(task)

            plots['time'].append(
                logger.Plot(
                    plotfile, x_axis='Time', y_axis='Amp',
                    parameters={'vis', self.inputs.vis}, command=str(task)))

        return plots
