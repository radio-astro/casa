from __future__ import absolute_import
import functools
import shutil

import collections

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.displays.applycal as applycal_displays
import pipeline.infrastructure.utils as utils
from pipeline.h.tasks.common import commonhelpermethods
from pipeline.h.tasks.flagging.flagdatasetter import FlagdataSetter
from pipeline.hif.tasks import applycal
from pipeline.hif.tasks import correctedampflag
from pipeline.hif.tasks import gaincal
from pipeline.hifa.tasks import bandpass
from pipeline.infrastructure import casa_tasks
from .resultobjects import BandpassflagResults

__all__ = [
    'BandpassflagInputs',
    'BandpassflagResults',
    'Bandpassflag'
]


LOG = infrastructure.get_logger(__name__)


class BandpassflagInputs(bandpass.ALMAPhcorBandpass.Inputs):
    # Lower sigma threshold for identifying outliers as a result of bad
    # antennas within individual timestamps; equivalent to:
    # relaxationSigma
    antnegsig = basetask.property_with_default('antnegsig', 4.0)

    # Upper sigma threshold for identifying outliers as a result of bad
    # antennas within individual timestamps; equivalent to:
    # positiveSigmaAntennaBased
    antpossig = basetask.property_with_default('antpossig', 4.6)

    # Threshold for maximum fraction of timestamps that are allowed
    # to contain outliers; equivalent to:
    # checkForAntennaBasedBadIntegrations
    tmantint = basetask.property_with_default('tmantint', 0.063)

    # Initial threshold for maximum fraction of "outlier timestamps" over
    # "total timestamps" that a baseline may be a part of; equivalent to:
    # tooManyIntegrationsFraction
    tmint = basetask.property_with_default('tmint', 0.085)

    # Initial threshold for maximum fraction of "bad baselines" over "all
    # baselines" that an antenna may be a part of; equivalent to:
    # tooManyBaselinesFraction
    tmbl = basetask.property_with_default('tmbl', 0.175)

    # Lower sigma threshold for identifying outliers as a result of "bad
    # baselines" and/or "bad antennas" within baselines (across all
    # timestamps); equivalent to:
    # catchNegativeOutliers['scalardiff']
    antblnegsig = basetask.property_with_default('antblnegsig', 3.4)

    # Upper sigma threshold for identifying outliers as a result of "bad
    # baselines" and/or "bad antennas" within baselines (across all
    # timestamps); equivalent to:
    # flag_nsigma['scalardiff']
    antblpossig = basetask.property_with_default('antblpossig', 3.2)

    # Relaxed value to set the threshold scaling factor to under certain
    # conditions; equivalent to:
    # relaxationFactor
    relaxed_factor = basetask.property_with_default('relaxed_factor', 2.0)

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, caltable=None, intent=None, field=None, spw=None,
                 antenna=None, hm_phaseup=None, phaseupsolint=None, phaseupbw=None, phaseupsnr=None, phaseupnsols=None,
                 hm_bandpass=None, solint=None, maxchannels=None, evenbpints=None, bpsnr=None, bpnsols=None,
                 combine=None, refant=None, minblperant=None, minsnr=None, solnorm=None, antnegsig=None, antpossig=None,
                 tmantint=None, tmint=None, tmbl=None, antblnegsig=None, antblpossig=None, relaxed_factor=None):

        super(BandpassflagInputs, self).__init__(context, output_dir=output_dir, vis=vis, caltable=caltable,
                                                 intent=intent, field=field, spw=spw, antenna=antenna,
                                                 hm_phaseup=hm_phaseup, phaseupsolint=phaseupsolint,
                                                 phaseupbw=phaseupbw, phaseupsnr=phaseupsnr, phaseupnsols=phaseupnsols,
                                                 hm_bandpass=hm_bandpass, solint=solint, maxchannels=maxchannels,
                                                 evenbpints=evenbpints, bpsnr=bpsnr, bpnsols=bpnsols, combine=combine,
                                                 refant=refant, minblperant=minblperant, minsnr=minsnr, solnorm=solnorm)
        self.antnegsig = antnegsig
        self.antpossig = antpossig
        self.tmantint = tmantint
        self.tmint = tmint
        self.tmbl = tmbl
        self.antblnegsig = antblnegsig
        self.antblpossig = antblpossig
        self.relaxed_factor = relaxed_factor


class Bandpassflag(basetask.StandardTaskTemplate):
    Inputs = BandpassflagInputs

    def prepare(self):
        inputs = self.inputs

        # Initialize results.
        result = BandpassflagResults()

        # Store the vis in the result
        result.vis = inputs.vis
        result.plots = dict()

        # create a shortcut to the plotting function that pre-supplies the inputs and context
        plot_fn = functools.partial(create_plots, inputs, inputs.context)

        # Create back-up of current calibration state.
        LOG.info('Creating back-up of calibration state')
        calstate_backup_name = 'before_bpflag.calstate'
        inputs.context.callibrary.export(calstate_backup_name)

        # Create back-up of flags.
        LOG.info('Creating back-up of "pre-bandpassflag" flagging state')
        flag_backup_name_prebpf = 'before_bpflag'
        task = casa_tasks.flagmanager(
            vis=inputs.vis, mode='save', versionname=flag_backup_name_prebpf)
        self._executor.execute(task)

        # Ensure that any flagging applied to the MS by this or the next
        # applycal are reverted at the end, even in the case of exceptions.
        try:
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

            # Make "apriori calibrations applied" plots for the weblog
            LOG.info('Creating "apriori calibrations applied" plots')
            result.plots['apriorical'] = plot_fn(suffix='apriorical')

            # Restore the calibration state to ensure the "apriori" cal tables
            # are included in pre-apply during creation of new caltables.
            LOG.info('Restoring back-up of calibration state.')
            inputs.context.callibrary.import_state(calstate_backup_name)

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

            # Apply the new caltables to the MS.
            LOG.info('Applying phase-up, bandpass, and amplitude cal tables.')
            # Apply the calibrations.
            acinputs = applycal.IFApplycalInputs(
                context=inputs.context, vis=inputs.vis, field=inputs.field,
                intent=inputs.intent, flagsum=False, flagbackup=False)
            actask = applycal.IFApplycal(acinputs)
            acresult = self._executor.execute(actask)

            # Make "after calibration, before flagging" plots for the weblog
            LOG.info('Creating "after calibration, before flagging" plots')
            result.plots['before'] = plot_fn(suffix='before')

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
                result.plots['after'] = plot_fn(suffix='after')

        finally:
            # Restore the "pre-bandpassflag" backup of the flagging state.
            LOG.info('Restoring back-up of "pre-bandpassflag" flagging state.')
            task = casa_tasks.flagmanager(
                vis=inputs.vis, mode='restore', versionname=flag_backup_name_prebpf)
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

        # If no flags were found in the bandpass calibrator, and a
        # preliminary bandpass table was created, then create a copy and
        # label it as the final table.
        elif bpresult.final:
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
            # that points to the final bp table. It is assumed here that
            # hifa_bandpass returns a single CalApplication containing a single
            # CalFrom caltable.
            bpresult.final[0].calfrom[0] = self._copy_calfrom_with_gaintable(
                bpresult.final[0].calfrom[0], fn_bp_final)
        else:
            LOG.warning('No bandpass table could be created.')

        # Store bandpass task result.
        result.bpresult = bpresult

        # Store flagging task result.
        result.cafresult = cafresult

        return result

    def analyse(self, result):
        """
        Analyses the Bandpassflag result:

        If new flags were found, run evaluation of flagging commands for
        identifying updates to the reference antenna list.
        """

        # Retrieve flagging commands from correctedampflag result.
        flags = result.cafresult.flagcmds()

        # If new flags were found, evaluate them for updates to the
        # refant list.
        if flags:
            result = self._evaluate_refant_update(result, flags)

        return result

    def _evaluate_refant_update(self, result, flags):
        """
        Identifies "bad" antennas as those that got flagged in all spws
        (entire timestamp) which are to be removed from the reference antenna
        list. Identifies "poor" antennas as those that got flagged in at least
        one spw, but not all, which are to be moved to the end of the reference
        antenna list.
        """

        # Get the MS object.
        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)

        # Retrieve which intents and spws were evaluated by
        # correctedampflag.
        intents = result.cafresult.inputs['intent'].split(',')
        spwids = map(int, result.cafresult.inputs['spw'].split(','))

        # Get number of antennas.
        antenna_names, antenna_ids = commonhelpermethods.get_antenna_names(ms)
        nants = len(antenna_names)

        # Create an antenna id-to-name translation dictionary.
        antenna_id_to_name = {ant.id: ant.name
                              for ant in ms.antennas
                              if ant.name.strip()}

        # Check that each antenna ID is represented by a unique non-empty
        # name, by testing that the unique set of antenna names is same
        # length as list of IDs. If not, then unset the translation
        # dictionary to revert back to flagging by ID.
        if len(set(antenna_id_to_name.values())) != len(ms.antennas):
            LOG.info('No unique name available for each antenna ID:'
                     ' flagging by antenna ID instead of by name.')
            antenna_id_to_name = None

        # Initialize flagging state
        ants_fully_flagged = collections.defaultdict(set)

        # Create a summary of the flagging state by going through each flagging
        # command.
        for flag in flags:

            # Only consider flagging commands with a specified antenna and
            # without a specified timestamp.
            if flag.antenna is not None and flag.time is None:
                # Skip flagging commands for baselines.
                if '&' in str(flag.antenna):
                    continue
                ants_fully_flagged[(flag.intent, flag.field, flag.spw)].update([flag.antenna])

        # For each combination of intent, field, and spw that were found to
        # have antennas flagged, raise a warning.
        sorted_keys = sorted(
            sorted(ants_fully_flagged.keys(), key=lambda keys: keys[2]),
            key=lambda keys: keys[0])
        for (intent, field, spwid) in sorted_keys:
            ants_flagged = ants_fully_flagged[(intent, field, spwid)]

            # Convert antenna IDs to names and create a string.
            ants_str = ", ".join(map(str, [antenna_id_to_name[iant] for iant in ants_flagged]))

            # Convert CASA intent from flagging command to pipeline intent.
            intent_str = utils.to_pipeline_intent(ms, intent)

            # Log a warning.
            LOG.warning(
                "{msname} - for intent {intent} (field "
                "{fieldname}) and spw {spw}, the following antennas "
                "are fully flagged: {ants}".format(
                    msname=ms.basename, intent=intent_str,
                    fieldname=field, spw=spwid,
                    ants=ants_str))

        # Initialize set of antennas that are fully flagged for all spws, for any intent
        ants_fully_flagged_in_all_spws_any_intent = set()

        # Check if any antennas were found to be fully flagged in all
        # spws, for any intent.

        # Identify the field and intent combinations for which fully flagged
        # antennas were found.
        intent_field_found = set([key[0:2] for key in ants_fully_flagged.keys()])
        for (intent, field) in intent_field_found:

            # Identify the spws for which fully flagged antennas were found (for current
            # intent and field).
            spws_found = set([key[2] for key in ants_fully_flagged.keys()
                              if key[0:2] == (intent, field)])

            # Only proceed if the set of spws for which flagged antennas were found
            # matches the set of spws for which correctedampflag ran.
            if spws_found == set(spwids):
                # Select the fully flagged antennas for current intent and field.
                ants_fully_flagged_for_intent_field = [
                    ants_fully_flagged[key]
                    for key in ants_fully_flagged.keys()
                    if key[0:2] == (intent, field)
                ]

                # Identify which antennas are fully flagged in all spws, for
                # current intent and field, and store these for later warning
                # and/or updating of refant.
                ants_fully_flagged_in_all_spws_any_intent.update(
                    set.intersection(*ants_fully_flagged_for_intent_field))

        # For the antennas that were found to be fully flagged in all
        # spws for one or more fields belonging to one or more of the intents,
        # raise a warning.
        if ants_fully_flagged_in_all_spws_any_intent:
            # Convert antenna IDs to names and create a string.
            ants_str = ", ".join(
                map(str, [antenna_id_to_name[iant]
                          for iant in ants_fully_flagged_in_all_spws_any_intent]))

            # Log a warning.
            LOG.warning(
                '{0} - the following antennas are fully flagged in all spws '
                'for one or more fields with intents among '
                '{1}: {2}'.format(ms.basename, ', '.join(intents), ants_str))

        # The following will assess if/how the list of reference antennas
        # needs to be updated based on antennas that were found to be
        # fully flagged.

        # Store the set of antennas that are fully flagged for all spws
        # in any of the intents in the result as a list of antenna
        # names.
        ants_to_remove_as_refant = {
            antenna_id_to_name[iant]
            for iant in ants_fully_flagged_in_all_spws_any_intent}

        # Store the set of antennas that were fully flagged in at least
        # one spw, for any of the fields for any of the intents.
        ants_to_demote_as_refant = {
            antenna_id_to_name[iant]
            for iants in ants_fully_flagged.values()
            for iant in iants}

        # If any reference antennas were found to be candidates for
        # removal or demotion (move to end of list), then proceed...
        if ants_to_remove_as_refant or ants_to_demote_as_refant:

            # If a list of reference antennas was registered with the MS..
            if (hasattr(ms, 'reference_antenna') and
                    isinstance(ms.reference_antenna, str)):

                # Create list of current refants
                refant = ms.reference_antenna.split(',')

                # Identify intersection between refants and fully flagged
                # and store in result.
                result.refants_to_remove = {
                    ant for ant in refant
                    if ant in ants_to_remove_as_refant}

                # If any refants were found to be removed...
                if result.refants_to_remove:

                    # Create string for log message.
                    ant_msg = utils.commafy(result.refants_to_remove, quotes=False)

                    # Check if removal of refants would result in an empty refant list,
                    # in which case the refant update is skipped.
                    if result.refants_to_remove == set(refant):

                        # Log warning that refant list should have been updated, but
                        # will not be updated so as to avoid an empty refant list.
                        LOG.warning(
                            '{0} - the following reference antennas became fully flagged '
                            'in all spws for one or more fields with intents among {1}, '
                            'but are *NOT* removed from the refant list because doing so '
                            'would result in an empty refant list: '
                            '{2}'.format(ms.basename, ', '.join(intents), ant_msg))

                        # Reset the refant removal list in the result to be empty.
                        result.refants_to_remove = set()
                    else:
                        # Log a warning if any antennas are to be removed from
                        # the refant list.
                        LOG.warning(
                            '{0} - the following reference antennas are '
                            'removed from the refant list because they became '
                            'fully flagged in all spws for one of the intents '
                            'among {1}: {2}'.format(ms.basename, ', '.join(intents), ant_msg))

                # Identify intersection between refants and candidate
                # antennas to demote, skipping those that are to be
                # removed entirely, and store this list in the result.
                # These antennas should be moved to the end of the refant
                # list (demoted) upon merging the result into the context.
                result.refants_to_demote = {
                    ant for ant in refant
                    if ant in ants_to_demote_as_refant
                    and ant not in result.refants_to_remove}

                # If any refants were found to be demoted...
                if result.refants_to_demote:

                    # Create string for log message.
                    ant_msg = utils.commafy(result.refants_to_demote, quotes=False)

                    # Check if the list of refants-to-demote comprises all
                    # refants, in which case the re-ordering of refants is
                    # skipped.
                    if result.refants_to_demote == set(refant):

                        # Log warning that refant list should have been updated, but
                        # will not be updated so as to avoid an empty refant list.
                        LOG.warning(
                            '{0} - the following antennas are fully flagged '
                            'for one or more spws, in one or more fields '
                            'with intents among {1}, but since these comprise all '
                            'refants, the refant list is *NOT* updated to '
                            're-order these to the end of the refant list: '
                            '{2}'.format(ms.basename, ', '.join(intents), ant_msg))

                        # Reset the refant demotion list in the result to be empty.
                        result.refants_to_demote = set()
                    else:
                        # Log a warning if any antennas are to be demoted from
                        # the refant list.
                        LOG.warning(
                            '{0} - the following antennas are moved to the end '
                            'of the refant list because they are fully '
                            'flagged for one or more spws, in one or more '
                            'fields with intents among {1}: '
                            '{2}'.format(ms.basename, ', '.join(intents), ant_msg))

            # If no list of reference antennas was registered with the MS,
            # raise a warning.
            else:
                LOG.warning(
                    '{0} - no reference antennas found in MS, cannot update '
                    'the reference antenna list.'.format(ms.basename))

        return result

    @staticmethod
    def _copy_calfrom_with_gaintable(old_calfrom, gaintable):
        return callibrary.CalFrom(gaintable=gaintable,
                                  gainfield=old_calfrom.gainfield,
                                  interp=old_calfrom.interp,
                                  spwmap=old_calfrom.spwmap,
                                  caltype=old_calfrom.caltype,
                                  calwt=old_calfrom.calwt)


def create_plots(inputs, context, suffix=''):
    """
    Return amplitude vs time and amplitude vs UV distance plots for the given
    data column.

    :param inputs: pipeline inputs
    :param context: pipeline context
    :param suffix: optional component to add to the plot filenames
    :return: dict of (x axis type => str, [plots,...])
    """
    # Exit early if weblog generation has been disabled
    if basetask.DISABLE_WEBLOG:
        return [], []

    calto = callibrary.CalTo(vis=inputs.vis, spw=inputs.spw)
    output_dir = context.output_dir

    amp_uvdist_plots = AmpVsXChart('uvdist', context, output_dir, calto, suffix=suffix).plot()
    amp_time_plots = AmpVsXChart('time', context, output_dir, calto, suffix=suffix).plot()

    return {'uvdist': amp_uvdist_plots, 'time': amp_time_plots}


class AmpVsXChart(applycal_displays.SpwSummaryChart):
    """
    Plotting class that creates an amplitude vs X plot for each spw, where X
    is given as a constructor argument.
    """
    def __init__(self, xaxis, context, output_dir, calto, **overrides):
        plot_args = {
            'ydatacolumn': 'corrected',
            'avgtime': '',
            'avgscan': False,
            'avgbaseline': False,
            'avgchannel': '9000',
            'coloraxis': 'corr',
            'correlation': 'XX,YY',
            'overwrite': True,
            'plotrange': [0, 0, 0, 0]
        }
        plot_args.update(**overrides)

        super(AmpVsXChart, self).__init__(context, output_dir, calto, xaxis=xaxis, yaxis='amp', intent='BANDPASS',
                                          **plot_args)
