from __future__ import absolute_import

import collections
import os
import shutil

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.utils as utils
from pipeline.h.heuristics import fieldnames as fieldnames
from pipeline.h.tasks.flagging.flagdatasetter import FlagdataSetter
from pipeline.hif.tasks import applycal
from pipeline.hif.tasks import correctedampflag
from pipeline.hif.tasks import gaincal
from pipeline.hifa.tasks import bandpass
from pipeline.infrastructure import casa_tasks
from .resultobjects import BandpassflagResults

LOG = infrastructure.get_logger(__name__)


class BandpassflagInputs(bandpass.ALMAPhcorBandpass.Inputs,
                         correctedampflag.Correctedampflag.Inputs):

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None,
                 intent=None, field=None, spw=None, **parameters):
        bandpass.ALMAPhcorBandpass.Inputs.__init__(
            self, context, output_dir=output_dir, vis=vis, intent=intent,
            field=field, spw=spw, **parameters)
        correctedampflag.Correctedampflag.Inputs.__init__(
            self, context, output_dir=output_dir, vis=vis, intent=intent,
            field=field, spw=spw, **parameters)

    @property
    def intent(self):
        if isinstance(self.vis, list):
            return self._handle_multiple_vis('intent')

        if not self._intent:
            self._intent = 'BANDPASS'

        return self._intent

    @intent.setter
    def intent(self, value):
        self._intent = value

    @property
    def field(self):
        if not callable(self._field):
            return self._field

        if isinstance(self.vis, list):
            return self._handle_multiple_vis('field')

        # this will give something like '0542+3243,0343+242'
        intent_fields = self._field(self.ms, self.intent)

        # run the answer through a set, just in case there are duplicates
        fields = set()
        fields.update(utils.safe_split(intent_fields))

        return ','.join(fields)

    @field.setter
    def field(self, value):
        if value is None:
            value = fieldnames.IntentFieldnames()
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


class Bandpassflag(basetask.StandardTaskTemplate):
    Inputs = BandpassflagInputs

    def prepare(self):
        inputs = self.inputs

        # Initialize results.
        result = BandpassflagResults()

        # Store the vis in the result
        result.vis = inputs.vis
        result.plots = dict()

        # Export the current calstate.
        LOG.info('Creating back-up of calibration state')
        calstate_backup_name = 'before_bpflag.calstate'
        inputs.context.callibrary.export(calstate_backup_name)

        # Create back-up of flags.
        LOG.info('Creating back-up of "pre-bandpassflag" flagging state')
        flag_backup_name_prebpf = 'before_bpflag'
        task = casa_tasks.flagmanager(
            vis=inputs.vis, mode='save', versionname=flag_backup_name_prebpf)
        self._executor.execute(task)

        # Run applycal to apply pre-existing caltables and propagate their
        # corresponding flags (should typically include Tsys, WVR, antpos).
        LOG.info('Applying pre-existing cal tables.')
        acinputs = applycal.IFApplycalInputs(
            context=inputs.context, vis=inputs.vis, field=inputs.field,
            intent=inputs.intent, flagsum=False, flagbackup=False)
        actask = applycal.IFApplycal(acinputs)
        acresult = self._executor.execute(actask, merge=True)

        # Create back-up of "after pre-calibration" state of flags.
        LOG.info('Creating back-up of "pre-calibrated" flagging state')
        flag_backup_name_after_precal = 'after_precal'
        task = casa_tasks.flagmanager(
            vis=inputs.vis, mode='save', versionname=flag_backup_name_after_precal)
        self._executor.execute(task)

        # Make "pre-calibrated" plots for the weblog
        LOG.info('Creating "pre-calibrated" plots')
        result.plots['raw'] = self.create_plots('raw', 'data')

        # Do standard phaseup and bandpass calibration.
        LOG.info('Creating initial phased-up bandpass calibration.')
        bpinputs = bandpass.ALMAPhcorBandpass.Inputs(
            context=inputs.context, vis=inputs.vis, caltable=inputs.caltable,
            field=inputs.field, intent=inputs.intent, spw=inputs.spw,
            antenna=inputs.antenna, hm_phaseup=inputs.hm_phaseup,
            phaseupbw=inputs.phaseupbw, phaseupsnr=inputs.phaseupsnr,
            phaseupnsols=inputs.phaseupnsols,
            phaseupsolint=inputs.phaseupsolint, hm_bandpass=inputs.hm_bandpass,
            solint=inputs.solint, maxchannels=inputs.maxchannels,
            evenbpints=inputs.evenbpints, bpsnr=inputs.bpsnr,
            bpnsols=inputs.bpnsols, combine=inputs.combine,
            refant=inputs.refant, solnorm=inputs.solnorm,
            minblperant=inputs.minblperant, minsnr=inputs.minsnr)
        # Modify output table filename to append "prelim".
        if bpinputs.caltable.endswith('.tbl'):
            bpinputs.caltable = bpinputs.caltable[:-4] + '.prelim.tbl'
        else:
            bpinputs.caltable += '.prelim'
        # Create and execute task.
        bptask = bandpass.ALMAPhcorBandpass(bpinputs)
        bpresult = self._executor.execute(bptask)

        # Add the phase-up table produced by the bandpass task to the callibrary.
        LOG.debug('Adding phase-up and bandpass table to temporary context.')
        for prev_result in bpresult.preceding:
            for calapp in prev_result:
                inputs.context.callibrary.add(calapp.calto, calapp.calfrom)
        # Accept the bandpass result to add the bandpass table to the callibrary.
        bpresult.accept(inputs.context)

        # Do amplitude solve on scan interval.
        LOG.info('Create amplitude gaincal table.')
        gacalinputs = gaincal.GTypeGaincal.Inputs(
            context=inputs.context, vis=inputs.vis, intent=inputs.intent,
            gaintype='T', antenna='', calmode='a', solint='inf')
        gacaltask = gaincal.GTypeGaincal(gacalinputs)
        gacalresult = self._executor.execute(gacaltask, merge=True)

        # Ensure that flags that may be set by applycal are restored, even in
        # case of exceptions.
        LOG.info('Applying phase-up, bandpass, and amplitude cal tables.')
        try:
            # Apply the calibrations.
            acinputs = applycal.IFApplycalInputs(
                context=inputs.context, vis=inputs.vis, field=inputs.field,
                intent=inputs.intent, flagsum=False, flagbackup=False)
            actask = applycal.IFApplycal(acinputs)
            acresult = self._executor.execute(actask)

        finally:
            # Restore flags that may have come from the latest applycal.
            LOG.info('Restoring back-up of "pre-calibrated" flagging state.')
            task = casa_tasks.flagmanager(
                vis=inputs.vis, mode='restore', versionname=flag_backup_name_after_precal)
            self._executor.execute(task)

        # Make "after calibration, before flagging" plots for the weblog
        LOG.info('Creating "after calibration, before flagging" plots')
        result.plots['before'] = self.create_plots('before', 'corrected')

        # Find amplitude outliers and flag data
        LOG.info('Running correctedampflag to identify outliers to flag.')
        cafinputs = correctedampflag.Correctedampflag.Inputs(
            context=inputs.context, vis=inputs.vis, intent=inputs.intent,
            field=inputs.field, spw=inputs.spw, antnegsig=inputs.antnegsig,
            antpossig=inputs.antpossig, tmantint=inputs.tmantint,
            tmint=inputs.tmint, tmbl=inputs.tmbl,
            antblnegsig=inputs.antblnegsig,
            antblpossig=inputs.antblpossig,
            relaxed_factor=inputs.relaxed_factor)
        caftask = correctedampflag.Correctedampflag(cafinputs)
        cafresult = self._executor.execute(caftask)

        # If flags were found in the bandpass calibrator, create the
        # "after calibration, after flagging" plots for the weblog
        cafflags = cafresult.flagcmds()
        if cafflags:
            LOG.info('Creating "after calibration, after flagging" plots')
            result.plots['after'] = self.create_plots('after', 'corrected')

        # Restore the "pre-bandpassflag" backup of the flagging state.
        LOG.info('Restoring back-up of "pre-bandpassflag" flagging state.')
        task = casa_tasks.flagmanager(
            vis=inputs.vis, mode='restore', versionname=flag_backup_name_prebpf)
        self._executor.execute(task)

        if cafflags:
            # Re-apply the newly found flags from correctedampflag.
            LOG.info('Re-applying flags from correctedampflag.')
            fsinputs = FlagdataSetter.Inputs(
                context=inputs.context, vis=inputs.vis, table=inputs.vis,
                inpfile=[])
            fstask = FlagdataSetter(fsinputs)
            fstask.flags_to_set(cafflags)
            fsresult = self._executor.execute(fstask)

            # Import the calstate before BPFLAG
            LOG.info('Restoring back-up of calibration state.')
            inputs.context.callibrary.import_state(calstate_backup_name)

            # If flags were found in the bandpass calibrator,
            # recompute the phase-up and bandpass calibration table.
            LOG.info('Creating final phased-up bandpass calibration.')
            bpinputs = bandpass.ALMAPhcorBandpass.Inputs(
                context=inputs.context, vis=inputs.vis, caltable=inputs.caltable,
                field=inputs.field, intent=inputs.intent, spw=inputs.spw,
                antenna=inputs.antenna, hm_phaseup=inputs.hm_phaseup,
                phaseupbw=inputs.phaseupbw, phaseupsnr=inputs.phaseupsnr,
                phaseupnsols=inputs.phaseupnsols,
                phaseupsolint=inputs.phaseupsolint, hm_bandpass=inputs.hm_bandpass,
                solint=inputs.solint, maxchannels=inputs.maxchannels,
                evenbpints=inputs.evenbpints, bpsnr=inputs.bpsnr,
                bpnsols=inputs.bpnsols, combine=inputs.combine,
                refant=inputs.refant, solnorm=inputs.solnorm,
                minblperant=inputs.minblperant, minsnr=inputs.minsnr)
            # Modify output table filename to append "prelim".
            if bpinputs.caltable.endswith('.tbl'):
                bpinputs.caltable = bpinputs.caltable[:-4] + '.final.tbl'
            else:
                bpinputs.caltable += '.final'
            # Create and execute task.
            bptask = bandpass.ALMAPhcorBandpass(bpinputs)
            bpresult = self._executor.execute(bptask)
        else:
            # If no flags were found in the bandpass calibrator,
            # create a copy of preliminary table and label it
            # as final.
            fn_bp_prelim = bpresult.final[0].gaintable
            if '.prelim' in fn_bp_prelim:
                fn_bp_final = '.final'.join(fn_bp_prelim.rpartition('.prelim')[0::2])
            else:
                fn_bp_final = fn_bp_prelim + '.final'
            shutil.copytree(fn_bp_prelim, fn_bp_final)
            LOG.info('No new flags found, created copy of preliminary '
                     'phased-up bandpass table as final version: '
                     '{0}'.format(fn_bp_final))

            # Update CalApplication in bandpass result with a new CalFrom
            # that points to the final bp table.
            bpresult.final[0].calfrom[0] = self._copy_calfrom_with_gaintable(
                bpresult.final[0].calfrom[0], fn_bp_final)

        # TODO: add plots to result

        # Store bandpass task result.
        result.bpresult = bpresult

        # # Store flagging task result.
        result.cafresult = cafresult

        return result

    def analyse(self, result):
        return result

    @staticmethod
    def _copy_calfrom_with_gaintable(old_calfrom, gaintable):
        return callibrary.CalFrom(gaintable=gaintable,
                                  gainfield=old_calfrom.gainfield,
                                  interp=old_calfrom.interp,
                                  spwmap=old_calfrom.spwmap,
                                  caltype=old_calfrom.caltype,
                                  calwt=old_calfrom.calwt)

    def create_plots(self, prefix, plottype):

        # Initialize output.
        plots = collections.defaultdict(list)

        # Exit early if weblog generation has been disabled,
        # returning empty plot dictionary.
        if basetask.DISABLE_WEBLOG:
            return plots

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
                'ydatacolumn': plottype,
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
                'ydatacolumn': plottype,
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
