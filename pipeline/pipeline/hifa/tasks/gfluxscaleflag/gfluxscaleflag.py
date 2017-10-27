"""
Created on 01 Jun 2017

@author: Vincent Geers (UKATC)
"""

from __future__ import absolute_import

import functools

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
from pipeline.h.tasks.flagging.flagdatasetter import FlagdataSetter
from pipeline.hif.tasks import applycal
from pipeline.hif.tasks import correctedampflag
from pipeline.hif.tasks import gaincal
from pipeline.infrastructure import casa_tasks
from .resultobjects import GfluxscaleflagResults

#import pipeline.infrastructure.displays.applycal as applycal_displays
from pipeline.h.tasks.common.displays import applycal as applycal_displays

__all__ = [
    'GfluxscaleflagInputs',
    'GfluxscaleflagResults',
    'Gfluxscaleflag'
]


LOG = infrastructure.get_logger(__name__)


class GfluxscaleflagInputs(basetask.StandardInputs):

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None,
                 intent=None, field=None, spw=None,
                 solint=None, phaseupsolint=None, minsnr=None, refant=None,
                 antnegsig=None, antpossig=None, tmantint=None, tmint=None,
                 tmbl=None, antblnegsig=None, antblpossig=None,
                 relaxed_factor=None):
        self._init_properties(vars())

    #
    # Parameters for data selection.
    #
    @property
    def intent(self):
        if self._intent is not None:
            return self._intent

        if isinstance(self.vis, list):
            return self._handle_multiple_vis('intent')

        # By default, this task will run for both FLUX and PHASE intent.
        intents_to_flag = 'AMPLITUDE,PHASE'

        # Check if any of the AMPLITUDE intent fields were also used for
        # BANDPASS, in which case it has already been flagged by
        # hifa_bandpassflag, and this task will just do PHASE fields.
        # This assumes that there will only be 1 field for BANDPASS and
        # 1 field for AMPLITUDE (which can be the same), which is valid as
        # of Cycle 5.
        for field in self.ms.get_fields(intent='AMPLITUDE'):
            for fieldintent in field.intents:
                if 'BANDPASS' in fieldintent:
                    intents_to_flag = 'PHASE'

        return intents_to_flag

    @intent.setter
    def intent(self, value):
        self._intent = value

    @property
    def field(self):
        if self._field is not None:
            return self._field

        if isinstance(self.vis, list):
            return self._handle_multiple_vis('field')

        # By default, return the fields corresponding to the input
        # intents.
        fieldids = [field.name
                    for field in self.ms.get_fields(intent=self.intent)]

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
            science_windows_only=True)

        return ','.join([str(spw.id) for spw in science_spws])

    @spw.setter
    def spw(self, value):
        self._spw = value

    #
    # Parameters for gaincal.
    #
    @property
    def solint(self):
        if self._solint is None:
            return 'inf'
        return self._solint

    @solint.setter
    def solint(self, value):
        self._solint = value

    @property
    def phaseupsolint(self):
        if self._phaseupsolint is None:
            return 'int'
        return self._phaseupsolint

    @phaseupsolint.setter
    def phaseupsolint(self, value):
        self._phaseupsolint = value

    @property
    def minsnr (self):
        if self._minsnr is None:
            return 2.0
        return self._minsnr

    @minsnr.setter
    def minsnr(self, value):
        self._minsnr = value

    @property
    def refant(self):
        if self._refant is None:
            return ''
        return self._refant

    @refant.setter
    def refant(self, value):
        self._refant = value

    #
    # Parameters for hif_correctedampflag.
    #

    # Lower sigma threshold for identifying outliers as a result of bad
    # antennas within individual timestamps.
    @property
    def antnegsig(self):
        return self._antnegsig

    @antnegsig.setter
    def antnegsig(self, value):
        if value is None:
            value = 4.0
        self._antnegsig = value

    # Upper sigma threshold for identifying outliers as a result of bad
    # antennas within individual timestamps.
    @property
    def antpossig(self):
        return self._antpossig

    @antpossig.setter
    def antpossig(self, value):
        if value is None:
            value = 4.6
        self._antpossig = value

    # Threshold for maximum fraction of timestamps that are allowed
    # to contain outliers.
    @property
    def tmantint(self):
        return self._tmantint

    @tmantint.setter
    def tmantint(self, value):
        if value is None:
            value = 0.063
        self._tmantint = value

    # Initial threshold for maximum fraction of "outlier timestamps" over
    # "total timestamps" that a baseline may be a part of.
    @property
    def tmint(self):
        return self._tmint

    @tmint.setter
    def tmint(self, value):
        if value is None:
            value = 0.085
        self._tmint = value

    # Initial threshold for maximum fraction of "bad baselines" over "all
    # baselines" that an antenna may be a part of.
    @property
    def tmbl(self):
        return self._tmbl

    @tmbl.setter
    def tmbl(self, value):
        if value is None:
            value = 0.175
        self._tmbl = value

    # Lower sigma threshold for identifying outliers as a result of "bad
    # baselines" and/or "bad antennas" within baselines (across all
    # timestamps).
    @property
    def antblnegsig(self):
        return self._antblnegsig

    @antblnegsig.setter
    def antblnegsig(self, value):
        if value is None:
            value = 3.4
        self._antblnegsig = value

    # Upper sigma threshold for identifying outliers as a result of "bad
    # baselines" and/or "bad antennas" within baselines (across all
    # timestamps).
    @property
    def antblpossig(self):
        return self._antblpossig

    @antblpossig.setter
    def antblpossig(self, value):
        if value is None:
            value = 3.2
        self._antblpossig = value

    # Relaxed value to set the threshold scaling factor to under certain
    # conditions.
    @property
    def relaxed_factor(self):
        return self._relaxed_factor

    @relaxed_factor.setter
    def relaxed_factor(self, value):
        if value is None:
            value = 2.0
        self._relaxed_factor = value


class Gfluxscaleflag(basetask.StandardTaskTemplate):
    Inputs = GfluxscaleflagInputs

    def prepare(self):
        inputs = self.inputs

        # Initialize results.
        result = GfluxscaleflagResults()

        # Store the vis in the result
        result.vis = inputs.vis
        result.plots = dict()

        # Create back-up of current calibration state.
        LOG.info('Creating back-up of calibration state')
        calstate_backup_name = 'before_gfsflag.calstate'
        inputs.context.callibrary.export(calstate_backup_name)

        # create a shortcut to the plotting function that pre-supplies the inputs and context
        plot_fn = functools.partial(create_plots, inputs, inputs.context)

        # Create back-up of flags.
        LOG.info('Creating back-up of "pre-gfluxscaleflag" flagging state')
        flag_backup_name_pregfsf = 'before_gfsflag'
        task = casa_tasks.flagmanager(
            vis=inputs.vis, mode='save', versionname=flag_backup_name_pregfsf)
        self._executor.execute(task)

        # Ensure that any flagging applied to the MS by this or the next
        # applycal are reverted at the end, even in the case of exceptions.
        try:
            # Run applycal to apply pre-existing caltables and propagate their
            # corresponding flags (should typically include Tsys, WVR, antpos).
            LOG.info('Applying pre-existing cal tables.')
            self._do_applycal(merge=True)

            # Create back-up of "after pre-calibration" state of flags.
            LOG.info('Creating back-up of "pre-calibrated" flagging state')
            flag_backup_name_after_precal = 'after_precal'
            task = casa_tasks.flagmanager(
                vis=inputs.vis, mode='save', versionname=flag_backup_name_after_precal)
            self._executor.execute(task)

            # Make "apriori calibrations applied" plots for the weblog
            LOG.info('Creating "apriori calibrations applied" plots')
            result.plots['apriorical'] = plot_fn(inputs.intent, suffix='apriorical')

            # Restore the calibration state to ensure the "apriori" cal tables
            # are included in pre-apply during creation of new caltables.
            LOG.info('Restoring back-up of calibration state.')
            inputs.context.callibrary.import_state(calstate_backup_name)

            # Determine the parameters to use for the gaincal to create the
            # phase-only caltable.
            if inputs.ms.combine_spwmap:
                phase_combine = 'spw'
                phaseup_spwmap = inputs.ms.combine_spwmap
                phase_interp = 'linearPD,linear'
                # Note: at present, phaseupsolint is specified as a fixed
                # value, defined in inputs. In the future, phaseupsolint may
                # need to be set based on exposure times; if so, see discussion
                # in CAS-10158 and logic in hifa.tasks.fluxscale.GcorFluxscale.
            else:
                phase_combine = ''
                phaseup_spwmap = inputs.ms.phaseup_spwmap
                phase_interp = None

            # Create phase caltable.
            LOG.info('Compute phase gaincal table.')
            self._do_gaincal(
                intent=inputs.intent, gaintype='G', calmode='p',
                combine=phase_combine, solint=inputs.phaseupsolint,
                minsnr=inputs.minsnr, refant=inputs.refant,
                spwmap=phaseup_spwmap, interp=phase_interp,
                merge=True)

            # Create amplitude caltable
            LOG.info('Compute amplitude gaincal table.')
            self._do_gaincal(
                intent=inputs.intent, gaintype='T', calmode='a',
                combine='', solint=inputs.solint,
                minsnr=inputs.minsnr, refant=inputs.refant,
                interp='linear,linear', merge=True)

            # Apply the new caltables to the MS.
            LOG.info('Applying phase-up, bandpass, and amplitude cal tables.')
            # Apply the calibrations.
            self._do_applycal(merge=False)

            # Make "after calibration, before flagging" plots for the weblog
            LOG.info('Creating "after calibration, before flagging" plots')
            result.plots['before'] = plot_fn(inputs.intent, suffix='before')

            # Run correctedampflag to identify outliers for intents specified in
            # intents_for_flagging; let "field" and "spw" be initialized
            # automatically based on intents and context.
            LOG.info('Running correctedampflag to identify outliers to flag.')
            cafinputs = correctedampflag.Correctedampflag.Inputs(
                context=inputs.context, vis=inputs.vis, intent=inputs.intent,
                antnegsig=inputs.antnegsig, antpossig=inputs.antpossig,
                tmantint=inputs.tmantint, tmint=inputs.tmint, tmbl=inputs.tmbl,
                antblnegsig=inputs.antblnegsig, antblpossig=inputs.antblpossig,
                relaxed_factor=inputs.relaxed_factor)
            caftask = correctedampflag.Correctedampflag(cafinputs)
            cafresult = self._executor.execute(caftask)

            # If flags were found in the bandpass calibrator...
            cafflags = cafresult.flagcmds()
            if cafflags:
                # Create the "after calibration, after flagging" plots for the weblog.
                LOG.info('Creating "after calibration, after flagging" plots')
                result.plots['after'] = plot_fn(inputs.intent, suffix='after')

        finally:
            # Restore the "pre-gfluxscaleflag" backup of the flagging state.
            LOG.info('Restoring back-up of "pre-gfluxscaleflag" flagging state.')
            task = casa_tasks.flagmanager(
                vis=inputs.vis, mode='restore', versionname=flag_backup_name_pregfsf)
            self._executor.execute(task)

        # If new outliers were identified...
        if cafflags:
            # Re-apply the newly found flags from correctedampflag.
            LOG.info('Re-applying flags from correctedampflag.')
            fsinputs = FlagdataSetter.Inputs(
                context=inputs.context, vis=inputs.vis, table=inputs.vis,
                inpfile=[])
            fstask = FlagdataSetter(fsinputs)
            fstask.flags_to_set(cafflags)
            fsresult = self._executor.execute(fstask)

        # Store flagging task result.
        result.cafresult = cafresult

        return result

    def analyse(self, result):
        return result

    def _do_applycal(self, merge):
        inputs = self.inputs

        # Change hifa_gfluxscaleflag so that the applycals are done per intent
        # to prevent unintended cross intent interpolation
        #
        # See https://open-jira.nrao.edu/browse/CAS-10158?focusedCommentId=93725

        # does any field have multiple intents?
        mixed_intents = False
        singular_intents = frozenset(inputs.intent.split(','))
        if len(singular_intents) > 1:
            for field in inputs.ms.get_fields(intent=inputs.intent):
                intent_intersection = field.intents.intersection(singular_intents)
                if len(intent_intersection) > 1:
                    mixed_intents = True
                    break

        if mixed_intents:
            # multiple items, one for each intent. Each intent will result
            # in a separate job
            ac_intents = singular_intents
        else:
            # one item, and hence one job, with 'PHASE,BANDPASS,...'
            ac_intents = [','.join(singular_intents)]

        # SJW - always just one job
        ac_intents = [','.join(singular_intents)]

        applycal_tasks = []
        for intent in ac_intents:
            task_inputs = applycal.IFApplycalInputs(inputs.context, vis=inputs.vis, intent=intent, flagsum=False,
                                                    flagbackup=False)
            task = applycal.IFApplycal(task_inputs)
            applycal_tasks.append(task)

        for task in applycal_tasks:
            self._executor.execute(task, merge=merge)

    def _do_gaincal(self, caltable=None, intent=None, gaintype='G',
                    calmode=None, combine=None, solint=None, antenna=None,
                    uvrange='', minsnr=None, refant=None, minblperant=None,
                    spwmap=None, interp=None, append=None, merge=True):

        inputs = self.inputs

        # Get the science spws
        sci_spwids = [spw.id for spw in inputs.ms.get_spectral_windows(science_windows_only=True)]

        fields_with_intent = inputs.ms.get_fields(intent=intent)

        # boil it down to just the valid spws for these fields
        field_spw_ids = {spw.id for fld in fields_with_intent for spw in fld.valid_spws}
        spw_ids = ','.join([str(spw_id) for spw_id in sorted(field_spw_ids) if spw_id in sci_spwids])

        # of the fields that we are about to process, does any field have
        # multiple intents?
        mixed_intents = False
        singular_intents = frozenset(intent.split(','))
        if len(singular_intents) > 1:
            for field in fields_with_intent:
                intent_intersection = field.intents.intersection(singular_intents)
                if len(intent_intersection) > 1:
                    mixed_intents = True
                    break

        if mixed_intents and solint == 'inf':
            # multiple items, one for each intent. Each intent will result
            # in a separate job
            task_intents = singular_intents
        else:
            # one item, and hence one job, with 'PHASE,BANDPASS,...'
            task_intents = [','.join(singular_intents)]

        for idx, intent in enumerate(task_intents):
            # Initialize gaincal inputs.
            task_inputs = gaincal.GTypeGaincal.Inputs(
                inputs.context,
                vis=inputs.vis,
                caltable=caltable,
                intent=intent,
                spw=spw_ids,
                solint=solint,
                gaintype=gaintype,
                calmode=calmode,
                minsnr=minsnr,
                combine=combine,
                refant=refant,
                antenna=antenna,
                uvrange=uvrange,
                minblperant=minblperant,
                solnorm=False,
                append=append)

            # if we need to generate multiple caltables, make the caltable
            # names unique by inserting the intent to prevent them overwriting
            # each other
            if len(task_intents) > 1:
                import os
                root, ext = os.path.splitext(task_inputs.caltable)
                task_inputs.caltable = '{!s}.{!s}{!s}'.format(root, intent, ext)

            # Modify output table filename to append "prelim".
            if task_inputs.caltable.endswith('.tbl'):
                task_inputs.caltable = task_inputs.caltable[:-4] + '.prelim.tbl'
            else:
                task_inputs.caltable += '.prelim'

            # Initialize and execute gaincal task.
            task = gaincal.GTypeGaincal(task_inputs)
            result = self._executor.execute(task)

            # If requested to merge the result...
            if merge:
                # Adjust the interp if provided.
                if interp:
                    self._mod_last_interp(result.pool[0], interp)
                    self._mod_last_interp(result.final[0], interp)

                # Adjust the spw map if provided.
                if spwmap:
                    self._mod_last_spwmap(result.pool[0], spwmap)
                    self._mod_last_spwmap(result.final[0], spwmap)

                # modify the result so that this caltable is only applied to
                # the intent from which the calibration was derived
                result.pool[0].calto.intent = intent
                result.final[0].calto.intent = intent

                # Merge result to the local context
                result.accept(inputs.context)

    def _mod_last_interp(self, l, interp):
        l.calfrom[-1] = self._copy_with_interp(l.calfrom[-1], interp)

    @staticmethod
    def _copy_with_interp(old_calfrom, interp):
        return callibrary.CalFrom(gaintable=old_calfrom.gaintable,
                                  gainfield=old_calfrom.gainfield,
                                  interp=interp,
                                  spwmap=old_calfrom.spwmap,
                                  caltype=old_calfrom.caltype,
                                  calwt=old_calfrom.calwt)

    def _mod_last_spwmap(self, l, spwmap):
        l.calfrom[-1] = self._copy_with_spwmap(l.calfrom[-1], spwmap)

    @staticmethod
    def _copy_with_spwmap(old_calfrom, spwmap):
        return callibrary.CalFrom(gaintable=old_calfrom.gaintable,
                                  gainfield=old_calfrom.gainfield,
                                  interp=old_calfrom.interp,
                                  spwmap=spwmap,
                                  caltype=old_calfrom.caltype,
                                  calwt=old_calfrom.calwt)


def create_plots(inputs, context, intents, suffix=''):
    """
    Return amplitude vs time and amplitude vs UV distance plots for the given
    intents.

    :param inputs: pipeline inputs
    :param context: pipeline context
    :param intents: intents to plot
    :param suffix: optional component to add to the plot filenames
    :return: dict of (x axis type => str, [plots,...])
    """
    # Exit early if weblog generation has been disabled
    if basetask.DISABLE_WEBLOG:
        return [], []

    calto = callibrary.CalTo(vis=inputs.vis, spw=inputs.spw, field=inputs.field)
    output_dir = context.output_dir

    amp_uvdist_plots, amp_time_plots = [], []
    for intent in intents.split(','):
        amp_uvdist_plots.extend(
            AmpVsXChart('uvdist', intent, context, output_dir, calto, suffix=suffix).plot())
        amp_time_plots.extend(
            AmpVsXChart('time', intent, context, output_dir, calto, suffix=suffix).plot())

    return {
        'uvdist': amp_uvdist_plots,
        'time': amp_time_plots
    }


class AmpVsXChart(applycal_displays.PlotmsFieldSpwComposite):
    """
    Plotting class that creates an amplitude vs X plot for each field and spw,
    where X is given as a constructor argument.
    """
    def __init__(self, xaxis, intent, context, output_dir, calto, **overrides):
        plot_args = {
            'ydatacolumn': 'corrected',
            'avgtime': '',
            'avgscan': False,
            'avgbaseline': False,
            'avgchannel': '9000',
            'coloraxis': 'corr',
            'overwrite': True,
            'plotrange': [0, 0, 0, 0]
        }
        plot_args.update(**overrides)

        super(AmpVsXChart, self).__init__(context, output_dir, calto, xaxis=xaxis, yaxis='amp', intent=intent,
                                          **plot_args)

