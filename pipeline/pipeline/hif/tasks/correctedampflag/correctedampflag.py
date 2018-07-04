from __future__ import absolute_import

import collections
import copy
import os

import numpy as np

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.vdp as vdp
from pipeline.h.tasks.common import commonhelpermethods
from pipeline.h.tasks.common.arrayflaggerbase import FlagCmd
from pipeline.h.tasks.flagging.flagdatasetter import FlagdataSetter
from pipeline.infrastructure import task_registry
from .resultobjects import CorrectedampflagResults

LOG = infrastructure.get_logger(__name__)


class CorrectedampflagInputs(vdp.StandardInputs):
    """
    CorrectedampflagInputs defines the inputs for the Correctedampflag pipeline task.
    """
    # Lower sigma threshold for identifying outliers as a result of "bad
    # baselines" and/or "bad antennas" within baselines (across all
    # timestamps); equivalent to:
    # catchNegativeOutliers['scalardiff']
    antblnegsig = vdp.VisDependentProperty(default=3.4)

    # Upper sigma threshold for identifying outliers as a result of "bad
    # baselines" and/or "bad antennas" within baselines (across all
    # timestamps); equivalent to:
    # flag_nsigma['scalardiff']
    antblpossig = vdp.VisDependentProperty(default=3.2)

    # Lower sigma threshold for identifying outliers as a result of bad
    # antennas within individual timestamps; equivalent to:
    # relaxationSigma
    antnegsig = vdp.VisDependentProperty(default=4.0)

    # Upper sigma threshold for identifying outliers as a result of bad
    # antennas within individual timestamps; equivalent to:
    # positiveSigmaAntennaBased
    antpossig = vdp.VisDependentProperty(default=4.6)

    @vdp.VisDependentProperty
    def field(self):
        # By default, return the fields corresponding to the input
        # intents.
        fieldids = [field.name
                    for field in self.ms.get_fields(intent=self.intent)]
        return ','.join(fieldids)

    intent = vdp.VisDependentProperty(default='BANDPASS')

    # Relaxed value to set the threshold scaling factor to under certain
    # conditions; equivalent to:
    # relaxationFactor
    relaxed_factor = vdp.VisDependentProperty(default=2.0)

    @vdp.VisDependentProperty
    def spw(self):
        science_spws = self.ms.get_spectral_windows(
            science_windows_only=True)
        return ','.join([str(spw.id) for spw in science_spws])

    # Threshold for maximum fraction of timestamps that are allowed
    # to contain outliers; equivalent to:
    # checkForAntennaBasedBadIntegrations
    tmantint = vdp.VisDependentProperty(default=0.063)

    # Initial threshold for maximum fraction of "bad baselines" over "all
    # baselines" that an antenna may be a part of; equivalent to:
    # tooManyBaselinesFraction
    tmbl = vdp.VisDependentProperty(default=0.175)

    # Initial threshold for maximum fraction of "outlier timestamps" over
    # "total timestamps" that a baseline may be a part of; equivalent to:
    # tooManyIntegrationsFraction
    tmint = vdp.VisDependentProperty(default=0.085)

    def __init__(self, context, output_dir=None, vis=None, intent=None, field=None, spw=None, antnegsig=None,
                 antpossig=None, tmantint=None, tmint=None, tmbl=None, antblnegsig=None, antblpossig=None,
                 relaxed_factor=None):
        super(CorrectedampflagInputs, self).__init__()

        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis
        self.output_dir = output_dir

        # data selection arguments
        self.field = field
        self.intent = intent
        self.spw = spw

        # flagging parameters
        self.antnegsig = antnegsig
        self.antpossig = antpossig
        self.tmantint = tmantint
        self.tmint = tmint
        self.tmbl = tmbl
        self.antblnegsig = antblnegsig
        self.antblpossig = antblpossig
        self.relaxed_factor = relaxed_factor


@task_registry.set_equivalent_casa_task('hif_correctedampflag')
@task_registry.set_casa_commands_comment(
    'This task identifies, for one or more specified calibrator source intents, baselines and antennas with a '
    'significant fraction of outlier integrations, by statistically examining the scalar difference of the corrected '
    'amplitudes minus model amplitudes.'
)
class Correctedampflag(basetask.StandardTaskTemplate):
    Inputs = CorrectedampflagInputs

    def prepare(self):
        inputs = self.inputs

        # Initialize results.
        result = CorrectedampflagResults()

        # Store the vis in the result
        result.vis = inputs.vis

        # Get the spws to use.
        spwids = map(int, inputs.spw.split(','))

        # Get the MS object.
        ms = inputs.context.observing_run.get_ms(name=inputs.vis)

        # Get translation dictionary for antenna id to name.
        antenna_id_to_name = self._get_ant_id_to_name_dict(ms)

        # Initialize list of newly found flags.
        newflags = []

        # Evaluate flagging heuristics separately for each intent.
        for intent in inputs.intent.split(','):

            # For current intent, identify which fields from inputs are valid.
            valid_fields = [field.name
                            for field in ms.get_fields(intent=intent)
                            if field.name in list(utils.safe_split(inputs.field))]

            # If no valid fields were found, raise warning, and continue to
            # next intent.
            if not valid_fields:
                LOG.warning("Invalid data selection for given intent(s) and "
                            "field(s): fields {} do not include intent "
                            "\'{}\'.".format(utils.commafy(utils.safe_split(inputs.field)),
                                             intent))
                continue

            # Evaluate heuristic for each valid field.
            for field in valid_fields:

                # Evaluate flagging heuristics separately for each spw.
                for spwid in spwids:

                    flags_for_intent_field_spw = self._evaluate_heuristic(
                        ms, intent, field, spwid, antenna_id_to_name)
                    newflags.extend(flags_for_intent_field_spw)

        if newflags:
            # Consolidate flagging commands.
            newflags = self._consolidate_flags(newflags)

            # Propagate PHASE 'bad baseline' flags to TARGET.
            newflags = self._propagate_phase_flags(newflags, ms, antenna_id_to_name)

        # Apply flags and get before/after summary.
        stats_before, stats_after = self._apply_flags(newflags)

        # Store newly identified flags in result.
        result.addflags(newflags)

        # Attach flagging summaries to result
        result.summaries = [stats_before, stats_after]

        return result

    def analyse(self, result):
        return result

    @staticmethod
    def _get_ant_id_to_name_dict(ms):
        """
        Return dictionary with antenna ID mapped to antenna name.
        If no unique antenna name can be assigned to each antenna ID,
        then return empty dictionary.

        :param ms: MeasurementSet
        :return: dictionary
        """
        # Create an antenna id-to-name translation dictionary.
        antenna_id_to_name = {ant.id: ant.name
                              for ant in ms.antennas
                              if ant.name.strip()}

        # Check that each antenna ID is represented by a unique non-empty
        # name, by testing that the unique set of antenna names is same
        # length as list of IDs. If not, then set the translation
        # dictionary to an empty dictionary (to revert back to flagging by ID.)
        if len(set(antenna_id_to_name.values())) != len(ms.antennas):
            LOG.info('No unique name available for each antenna ID:'
                     ' flagging by antenna ID instead of by name.')
            antenna_id_to_name = {}

        return antenna_id_to_name

    def _evaluate_heuristic(self, ms, intent, field, spwid, antenna_id_to_name):

        # Initialize flags.
        allflags = []

        # Determine which baseline sets to evaluate separately.
        baseline_sets = self._identify_baseline_sets(ms)

        # If no baseline sets were received, then evaluate heuristics for
        # all baselines at once.
        if not baseline_sets:
            allflags.extend(self._evaluate_heuristic_for_baseline_set(
                ms, intent, field, spwid, antenna_id_to_name))
        else:
            # Evaluate heuristic for each set of baselines.
            for baseline_set in baseline_sets:
                newflags = self._evaluate_heuristic_for_baseline_set(
                    ms, intent, field, spwid, antenna_id_to_name, baseline_set)
                allflags.extend(newflags)

        return allflags

    @staticmethod
    def _identify_baseline_sets(ms):

        # Determine unique antenna diameters.
        uniq_diams = {ant.diameter for ant in ms.antennas}

        if len(uniq_diams) <= 1:
            # If dataset contains only antennas with the same antenna diameter,
            # then heuristics should be evaluated for all baselines together.
            baseline_sets = []
        else:
            # If the dataset contains antennas of different diameters, then
            # check that these are ALMA with the expected diameters.
            if ms.antenna_array.name == 'ALMA' and uniq_diams == {7.0, 12.0}:

                # For 12m antennas, identify number of "PM*" and "D*" antennas.
                ant_names = [ant.name for ant in ms.antennas]
                n_pm = len([name for name in ant_names if "PM" in name])
                n_d = len([name for name in ant_names if "D" in name])

                # Always add the mixed-array 7m-12m as a baseline set.
                bl_str = []
                if n_pm > 0:
                    bl_str.append("CM*&PM*")
                if n_d > 0:
                    bl_str.append("CM*&D*")
                baseline_sets = [('7m-12m', ';'.join(bl_str))]

                # If more than one 7m antenna is present, add these as a
                # separate baseline set.
                if len([ant for ant in ms.antennas if ant.diameter == 7.0]) > 1:
                    baseline_sets.append(('7m-7m', "CM*&CM*"))

                # If more than one 12m antenna is present, add these as a
                # separate baseline set.
                if n_pm + n_d > 1:
                    bl_str = []
                    if n_pm > 1:
                        bl_str.append("PM*&PM*")
                    if n_d > 1:
                        bl_str.append("D*&D*")
                    if n_pm > 0 and n_d > 0:
                        bl_str.append("PM*&D*")
                    baseline_sets.append(('12m-12m', ';'.join(bl_str)))
            # If the mixed-array dataset is not recognized as ALMA diameters,
            # then continue with evaluating all baselines at once.
            else:
                baseline_sets = []
                LOG.warning("Found mixed-array with multiple antenna diameters, but unanticipated non-ALMA diameters"
                            " (do not know how to select data). Continuing with heuristic evaluation for all baselines"
                            " at once.")

        return baseline_sets

    def _evaluate_heuristic_for_baseline_set(self, ms, intent, field, spwid, antenna_id_to_name, baseline_set=None):

        inputs = self.inputs

        # Set "default" scale factor by which the thresholds
        # tmint and tmbl should be scaled.
        thresh_scale_factor = 1.0

        # Set threshold for maximum fraction of outlier baseline scans that
        # a single antenna can be involved in.
        max_frac_outlier_scans = 0.5

        # Set threshold for maximum fraction of antennas that can be
        # "affected", by being involved in the most, or close to the most,
        # number of outlier baseline scans.
        ants_in_outlier_baseline_scans_thresh = 1.0/3.0

        # Set threshold for maximum fraction of antennas that can be either
        # "affected" or "partially affected", where the latter are antennas
        # that are not "affected" but still involved in at least one outlier
        # baseline scan.
        ants_in_outlier_baseline_scans_partial_thresh = 0.5

        # Set threshold for minimum number of "bad baselines" than an antenna
        # may be a part of without getting flagged.
        tmbl_minbadnr = 4.0

        # Set sigma threshold for identifying very high outliers.
        antveryhighsig = 10.0

        # Set sigma thresholds for identifying ultra low/high outliers.
        antultrahighsig = 12.0
        antultralowsig = 13.0

        # Set sigma outlier threshold for purpose of evaluating
        # whether to relax the threshold scaling factor.
        relaxsig = 6.5

        # Get number of scans in MS for this intent.
        nscans = len(ms.get_scans(scan_intent=intent))

        # Get number of antennas.
        nants = len(ms.antennas)

        # If there are multiple scans for this intent, then scale up the
        # threshold for timestamps with outliers.
        if nscans > 1:
            tmantint = inputs.tmantint * 2 * (1 + np.log10(nscans))
        else:
            tmantint = inputs.tmantint

        # Initialize flags.
        newflags = []

        # Read in data from MS. If no valid data could be read, return early with no flags.
        data = self._read_data_from_ms(ms, intent, field, spwid, baseline_set=baseline_set)
        if not data:
            return newflags

        # Remove the channel dimension (should be of length 1 as we asked
        # for average across all channels).
        corrdata = np.squeeze(data['corrected_data'], axis=1)
        modeldata = np.squeeze(data['model_data'], axis=1)
        flag_all = np.squeeze(data['flag'], axis=1)

        # Compute "scalar difference" between corrected data and
        # model data.
        cmetric_all = np.abs(corrdata) - np.abs(modeldata)

        # Select non-autocorrelations.
        id_nonac = np.where(data['antenna1'] != data['antenna2'])
        time = data['time'][id_nonac]
        ant1 = data['antenna1'][id_nonac]
        ant2 = data['antenna2'][id_nonac]

        # Get number of correlations for this spw.
        ncorrs = len(commonhelpermethods.get_corr_products(ms, spwid))

        # Evaluate flagging heuristics separately for each polarisation.
        for icorr in range(ncorrs):

            # Select non-autocorrelations from corrected and model
            # data.
            cmetric = cmetric_all[icorr][id_nonac]
            flag = flag_all[icorr][id_nonac]

            # Compute the threshold for maximum number of timestamps that are allowed
            # to contain outliers, based on maximum fractional threshold and
            # number of unique timestamps, while setting to a minimum of 1.
            n_time_with_highsig_thresh_min = 1
            n_time_with_highsig_thresh_frac = tmantint * len(np.unique(time))
            n_time_with_highsig_max = np.max([n_time_with_highsig_thresh_min, n_time_with_highsig_thresh_frac])

            # Select for non-flagged data and non-NaN data.
            id_nonbad = np.where(np.logical_and(
                np.logical_not(flag),
                np.isfinite(cmetric)))
            cmetric_sel = cmetric[id_nonbad]
            time_sel = time[id_nonbad]
            ant1_sel = ant1[id_nonbad]
            ant2_sel = ant2[id_nonbad]

            # Compute the median. Assuming the distribution is normal,
            # compute a robust estimate of the standard deviation as
            # 1.4826 x the median absolute deviation from the median.
            med = np.median(cmetric_sel)
            mad = np.median(np.abs(cmetric_sel - np.median(cmetric_sel))) * 1.4826

            #
            # Evaluate whether the threshold scaling factor should be
            # set to the relaxed value.
            #

            # Based on the "relaxation" sigma outlier threshold, identify
            # outliers. If an antenna-based bad integrations fraction was
            # provided, then identify both negative and positive outliers,
            # otherwise just identify negative outliers.
            if tmantint > 0:
                id_relaxsig = np.where(
                    np.logical_or(
                        cmetric_sel < (med - mad * relaxsig),
                        cmetric_sel > (med + mad * relaxsig)))[0]
            else:
                id_relaxsig = np.where(
                    cmetric_sel < (med - mad * inputs.relaxsig))[0]

            # If any outliers were found...
            if len(id_relaxsig) > 0:
                # Relax threshold scale factor if not testing for positive
                # outliers.
                if tmantint <= 0:
                    thresh_scale_factor = inputs.relaxed_factor
                else:
                    # Identify number of unique outlier timestamps.
                    time_sel_relaxsig = time_sel[id_relaxsig]
                    time_sel_relaxsig_uniq = np.unique(time_sel_relaxsig)

                    # Set maximum threshold for outlier timestamps.
                    # Currently set equal to the threshold for high sigma
                    # outlier timestamps used in flagging heuristic.
                    n_time_with_relaxsig_max = n_time_with_highsig_max

                    # If the number of unique outlier timestamps exceeds the
                    # threshold, then relax the threshold scale factor.
                    if len(time_sel_relaxsig_uniq) > n_time_with_relaxsig_max:
                        thresh_scale_factor = inputs.relaxed_factor

            #
            # Start with evaluation of flagging heuristics.
            #

            #
            # The following part considers timestamps separately, evaluating a
            # series of antenna-based heuristics to flag one or more antennas
            # within a timestamp or to flag an entire timestamp if necessary.
            #
            # If outliers are identified, but not flagged by the antenna-based
            # heuristics, then any existing ultra high outlier
            # baseline-timestamp combination will be flagged.
            #

            # Based on the input antenna based negative and positive sigma
            # outlier thresholds, identify outliers. If an antenna-based
            # bad integrations fraction was provided, then identify both
            # negative and positive outliers, otherwise just identify
            # negative outliers.
            if tmantint > 0:
                id_highsig = np.where(
                    np.logical_or(
                        cmetric_sel < (med - mad * inputs.antnegsig),
                        cmetric_sel > (med + mad * inputs.antpossig)))[0]
            else:
                id_highsig = np.where(
                    cmetric_sel < (med - mad * inputs.antnegsig))[0]

            # Based on the "very high" sigma outlier threshold, identify
            # both negative and positive outliers.
            id_veryhighsig = np.where(
                np.logical_or(
                    cmetric_sel < (med - mad * antveryhighsig),
                    cmetric_sel > (med + mad * antveryhighsig)))[0]

            # Based on the "ultra low/high" sigma outlier thresholds, identify
            # both negative and positive outliers.
            id_ultrahighsig = np.where(
                np.logical_or(
                    cmetric_sel < (med - mad * antultralowsig),
                    cmetric_sel > (med + mad * antultrahighsig)))[0]

            # If outliers were found and checking for positive outliers...
            if len(id_highsig) > 0 and tmantint > 0:
                # Check whether the outliers were concentrated in only one or a small
                # fraction (set by bad_int_frac) of timestamps.

                # Identify timestamps with outliers
                time_sel_highsig = time_sel[id_highsig]
                time_sel_highsig_uniq = np.unique(time_sel_highsig)

                # Identify timestamps with very high outliers.
                time_sel_veryhighsig = time_sel[id_veryhighsig]
                time_sel_veryhighsig_uniq = np.unique(time_sel_veryhighsig)

                # Set maximum threshold for "very high" outlier timestamps.
                # Currently set equal to the threshold for high sigma
                # outlier timestamps used in flagging heuristic.
                n_time_with_veryhighsig_max = n_time_with_highsig_max

                # If all outliers were concentrated within a small number of
                # timestamps set by a threshold, then evaluate the antenna
                # based heuristics for those timestamps.
                if 0 < len(time_sel_highsig_uniq) <= n_time_with_highsig_max:
                    new_antbased_flags = self._evaluate_antbased_heuristics(
                        ms, spwid, intent, icorr, field,
                        ants_in_outlier_baseline_scans_thresh,
                        ants_in_outlier_baseline_scans_partial_thresh,
                        max_frac_outlier_scans,
                        antenna_id_to_name, ant1_sel, ant2_sel, nants,
                        id_highsig, time_sel_highsig, time_sel_highsig_uniq)
                    newflags.extend(new_antbased_flags)

                # If all very high outliers were concentrated within a small
                # number of timestamps set by a threshold, then evaluate the
                # antenna based heuristics for those timestamps.
                elif 0 < len(time_sel_veryhighsig_uniq) <= n_time_with_veryhighsig_max:
                    new_antbased_flags = self._evaluate_antbased_heuristics(
                        ms, spwid, intent, icorr, field,
                        ants_in_outlier_baseline_scans_thresh,
                        ants_in_outlier_baseline_scans_partial_thresh,
                        max_frac_outlier_scans,
                        antenna_id_to_name, ant1_sel, ant2_sel, nants,
                        id_veryhighsig, time_sel_veryhighsig, time_sel_veryhighsig_uniq)
                    newflags.extend(new_antbased_flags)

                # Flag any ultra high outliers for corresponding baseline/timestamp.
                if len(id_ultrahighsig) > 0:
                    bad_timestamps = time_sel[id_ultrahighsig]
                    bad_bls = zip(ant1_sel[id_ultrahighsig], ant2_sel[id_ultrahighsig])
                    newflags.extend(
                        self._create_flags_for_ultrahigh_baselines_timestamps(
                            ms, spwid, intent, icorr, field, bad_timestamps, bad_bls, antenna_id_to_name))

            #
            # The following part considers all timestamps at once, and
            # identifies "bad baselines" as the baselines that contain
            # contain outliers in a number of timestamps that exceeds
            # the maximum threshold (set by scale factor).
            # For each of these "bad" baselines, it then:
            #
            #  a.) identifies "bad antennas" as those antennas that are
            #  part of a number of "bad baselines" that exceeds a
            #  maximum threshold. Each of these "bad antennas" are
            #  entirely flagged for all timestamps.
            #
            #  b.) identifies remaining "bad baselines" as those that
            #  do not contain one of the "bad antennas", but that do
            #  contain outliers in a number of timestamps that exceeds
            #  the maximum threshold set by the relaxed scale factor.
            #  Each of these baselines are flagged for all timestamps.
            #

            # If requested, identify both positive and negative
            # outliers, otherwise just identify positive outliers; also
            # select for non-flagged and non-NaN data.
            if inputs.antblnegsig > 0:
                id_flagsig = np.where(
                    np.all((np.logical_not(flag),
                            np.isfinite(cmetric),
                            np.logical_or(
                                cmetric < (med - mad * inputs.antblnegsig),
                                cmetric > (med + mad * inputs.antblpossig))), axis=0))[0]
            else:
                id_flagsig = np.where(
                    np.all((np.logical_not(flag),
                            np.isfinite(cmetric),
                            cmetric > (med + mad * inputs.antblpossig)), axis=0))[0]

            # Proceed if outliers were found...
            if len(id_flagsig) > 0:
                # Identify baselines involved in each baseline/timestamp
                # outlier; this list may contain multiples of the same
                # baseline when it was an outlier in more than one
                # timestamp.
                outlier_ant1 = ant1[id_flagsig]
                outlier_ant2 = ant2[id_flagsig]
                outlier_bl = zip(outlier_ant1, outlier_ant2)

                # Compute for each baseline how many outlier timestamps
                # it is a part of. This creates a dictionary with
                # baselines as keys, and number of outlier timestamps
                # as values.
                outlier_bl_counts = collections.Counter(outlier_bl)

                # Compute for each baseline how many timestamps
                # it is a part of. This creates a dictionary with
                # baselines as keys, and number of timestamps
                # as values.
                baselines = zip(ant1, ant2)
                bl_counts = collections.Counter(baselines)

                # Compute final threshold for maximum fraction of "outlier
                # timestamps" over "total timestamps" that a baseline can
                # be a part of.
                tmint_scaled = inputs.tmint * thresh_scale_factor

                # Identify "bad baselines" as those baselines whose number
                # of timestamps with outliers exceeds the threshold.
                bad_bls = [bl for bl, count in outlier_bl_counts.items()
                           if count > np.max([1, bl_counts[bl] * tmint_scaled])]

                # Compute for each antenna how many "bad baselines" it is
                # a part of.
                ant_in_bad_bl_count = np.bincount(sum(bad_bls, ()),
                                                  minlength=nants)

                # Compute final threshold for maximum number of "bad
                # baselines" that an antenna may be a part of:
                # this is based on the scaled fractional threshold
                # times the number of baselines that the antenna is
                # part of, with a minimum number threshold set by
                # "tmbl_minbadnr".
                tmbl_nr_thresh = max(
                    tmbl_minbadnr,
                    inputs.tmbl * thresh_scale_factor * (nants - 1))

                # Identify "bad antennas" as those antennas involved in a number of
                # "bad baselines" that exceeds the threshold.
                bad_ants = [ant for ant, count in enumerate(ant_in_bad_bl_count)
                            if count > tmbl_nr_thresh]

                # Create flagging command for each identified bad antenna.
                for bad_ant in bad_ants:
                    newflags.append(
                        FlagCmd(
                            filename=ms.name,
                            spw=spwid,
                            antenna=bad_ant,
                            intent=utils.to_CASA_intent(ms, intent),
                            pol=icorr,
                            field=field,
                            reason='bad antenna',
                            antenna_id_to_name=antenna_id_to_name))

                # Compute final outlier timestamps per baseline threshold,
                # forcibly always using the relaxed threshold scale factor,
                # and setting the minimum fraction to 1, such that a
                # baseline with 100% outlier timestamps will get flagged
                # (even if dynamic threshold exceeded beyond 1.0).
                tmint_relaxed = np.min(
                    [1.0,
                     inputs.tmint * inputs.relaxed_factor])

                # Compute fraction of outlier timestamps for each bad baseline.
                bad_bls_timestamp_fraction = {
                    bl: float(outlier_bl_counts[bl]) / bl_counts[bl]
                    for bl in bad_bls}

                # For each bad baseline, check if it was already covered by
                # one of the bad antennas, and otherwise flag it explicitly
                # if the fraction of outlier timestamps for this baseline
                # equals-or-exceeds the threshold.
                for bl in bad_bls:
                    if (bl[0] not in bad_ants
                            and bl[1] not in bad_ants
                            and bad_bls_timestamp_fraction[bl] >= tmint_relaxed):
                        newflags.append(
                            FlagCmd(
                                filename=ms.name,
                                spw=spwid,
                                antenna="%s&%s" % bl,
                                intent=utils.to_CASA_intent(ms, intent),
                                pol=icorr,
                                field=field,
                                reason='bad baseline',
                                antenna_id_to_name=antenna_id_to_name))

        return newflags

    @staticmethod
    def _read_data_from_ms(ms, intent, field, spwid, baseline_set=None):

        # Initialize data selection.
        data_selection = {'field': field,
                          'scanintent': '*%s*' % utils.to_CASA_intent(ms, intent),
                          'spw': str(spwid)}

        # Add baseline set to data selection if provided; log selection.
        if baseline_set:
            LOG.info('Reading data for intent {}, field {}, spw {}, and {} baselines ({})'.format(
                intent, field, spwid, baseline_set[0], baseline_set[1]))
            data_selection['baseline'] = baseline_set[1]
        else:
            LOG.info('Reading data for intent {}, field {}, spw {}, and all baselines'.format(intent, field, spwid))

        # Get number of channels for this spw.
        nchans = ms.get_spectral_windows(str(spwid))[0].num_channels

        # Read in data from MS.
        with casatools.MSReader(ms.name) as openms:
            try:
                # Apply data selection, and set channel selection to take the
                # average of all channels.
                openms.msselect(data_selection)
                openms.selectchannel(1, 0, nchans, 1)

                # Extract data from MS.
                data = openms.getdata(['corrected_data', 'model_data', 'antenna1', 'antenna2', 'flag', 'time'])
            except:
                LOG.warning('Unable to compute flagging for intent {}, field {}, spw {}'.format(intent, field, spwid))
                data = None
                openms.close()

        return data

    @staticmethod
    def _create_flags_for_ultrahigh_baselines_timestamps(
            ms, spwid, intent, icorr, field, timestamps, baselines, antenna_id_to_name):

        newflags = []
        for idx in range(len(baselines)):
            newflags.append(
                FlagCmd(
                    filename=ms.name,
                    spw=spwid,
                    antenna="%s&%s" % baselines[idx],
                    intent=utils.to_CASA_intent(ms, intent),
                    pol=icorr,
                    time=timestamps[idx],
                    field=field,
                    reason='ultrahigh baseline timestamp',
                    antenna_id_to_name=antenna_id_to_name))

        return newflags

    @staticmethod
    def _evaluate_antbased_heuristics(
            ms, spwid, intent, icorr, field,
            ants_in_outlier_baseline_scans_thresh,
            ants_in_outlier_baseline_scans_partial_thresh,
            max_frac_outlier_scans,
            antenna_id_to_name, ant1_sel, ant2_sel, nants,
            id_highsig, time_sel_highsig, time_sel_highsig_uniq):

        # Initialize flags.
        newflags = []

        # For each of the few bad timestamps...
        for timestamp in time_sel_highsig_uniq:

            # Identify baseline scans within this timestamp.
            id_outlier_scans_in_timestamp = np.where(
                time_sel_highsig == timestamp)[0]
            n_outlier_scans_in_timestamp = len(id_outlier_scans_in_timestamp)

            # Get a list of antennas involved in outlier scans
            # matching the timestamp.
            ants_in_olscans_in_tstamp = list(np.concatenate(
                [ant1_sel[id_highsig][id_outlier_scans_in_timestamp],
                 ant2_sel[id_highsig][id_outlier_scans_in_timestamp]]))

            # Identify number of outlier scans within current
            # timestamp that each antenna is involved in.
            antcnts = np.bincount(ants_in_olscans_in_tstamp, minlength=nants)

            # Identify the ants involved in the largest number
            # of outliers as well as 1 count less (while ignoring
            # ants involved in 0 outliers).
            id_affected_ants = np.where(antcnts >= max([1, antcnts.max() - 1]))[0]

            # Identify the ants that are at least partially affected,
            # by being involved in at least one outlier, but excluding
            # antennas already identified as "affected".
            id_partly_affected_ants = np.setdiff1d(
                np.where(antcnts >= 1)[0],
                id_affected_ants)

            # If the number of affected antennas is a significant fraction of
            # all antennas, then flag the entire timestamp.
            if len(id_affected_ants) > ants_in_outlier_baseline_scans_thresh * nants:
                # Create a flagging command for all antennas
                # in this timestamp (for given spw, intent, pol).
                newflags.append(
                    FlagCmd(
                        filename=ms.name,
                        spw=spwid,
                        intent=utils.to_CASA_intent(ms, intent),
                        pol=icorr,
                        time=timestamp,
                        field=field,
                        reason='bad timestamp'))
            # Evaluate a slightly more restrictive threshold, this time
            # on the total number of affected and partly affected antennas,
            # while still requiring half the original threshold.
            elif (len(id_affected_ants) > 0.5 * ants_in_outlier_baseline_scans_thresh * nants
                    and len(id_affected_ants) + len(id_partly_affected_ants) >
                    ants_in_outlier_baseline_scans_partial_thresh * nants):

                # Create a flagging command for all antennas
                # in this timestamp (for given spw, intent, pol).
                newflags.append(
                    FlagCmd(
                        filename=ms.name,
                        spw=spwid,
                        intent=utils.to_CASA_intent(ms, intent),
                        pol=icorr,
                        time=timestamp,
                        field=field,
                        reason='bad timestamp'))
            # If there was no significant fraction of affected antennas,
            # the proceed check if the antenna(s) with the highest number of
            # outlier scans (within this timestamp) equals-or-exceeds the threshold
            # and flag the corresponding antenna(s).
            elif (antcnts.max() >= max_frac_outlier_scans * n_outlier_scans_in_timestamp
                    and n_outlier_scans_in_timestamp > 5):

                # Identify which antennas matched the highest counts,
                # and create a flagging command for each.
                id_ants_highest_cnts = np.where(antcnts == antcnts.max())[0]
                for ant in id_ants_highest_cnts:
                    # Create a flagging command for this antenna
                    newflags.append(
                        FlagCmd(
                            filename=ms.name,
                            spw=spwid,
                            antenna=ant,
                            intent=utils.to_CASA_intent(ms, intent),
                            pol=icorr,
                            time=timestamp,
                            field=field,
                            reason='bad antenna timestamp',
                            antenna_id_to_name=antenna_id_to_name))
            # Heuristic for catching cross-CAI-dependent issues:
            # If there are affected antennas, and total number of affected
            # and partially affected antennas exceeds the larger of 6 or
            # 20% of the antennas, then flag the timestamp. The minimum
            # threshold of 6 is there to prevent over-application on ACA 7m
            # datasets.
            elif (len(id_affected_ants) + len(id_partly_affected_ants) > np.max([6, 0.2 * nants])
                    and len(id_affected_ants) > 0):

                # Create flags only for the "affected" antennas for this timestamp
                # because CAI-dependent issues do not affect all antennas.
                for ant in id_affected_ants:
                    # Create a flagging command for this antenna.
                    newflags.append(
                        FlagCmd(
                            filename=ms.name,
                            spw=spwid,
                            antenna=ant,
                            intent=utils.to_CASA_intent(ms, intent),
                            pol=icorr,
                            time=timestamp,
                            field=field,
                            reason='bad CAI-dependent data',
                            antenna_id_to_name=antenna_id_to_name))

        return newflags

    def _apply_flags(self, flags):

        inputs = self.inputs

        # Initialize flagging summaries.
        stats_before, stats_after = {}, {}

        # Create a list of flagdata commands, always add the "before" summary.
        allflagcmds = ["mode='summary' name='before'"]

        # If new flags were found, apply these as part of the flagdata call,
        # and add an "after" summary.
        if flags:
            LOG.warning('Evaluation of {0} raised {1} flagging '
                        'command(s)'.format(os.path.basename(inputs.vis),
                                            len(flags)))

            LOG.info('Applying newly found flags.')
            allflagcmds.extend(flags)
            allflagcmds.append("mode='summary' name='after'")
        else:
            LOG.info('Evaluation of {0} raised 0 flagging commands'.format(
                os.path.basename(inputs.vis)))

        # Run flagdata to create summaries and set flags.
        fsinputs = FlagdataSetter.Inputs(
            context=inputs.context, vis=inputs.vis, table=inputs.vis,
            inpfile=[])
        fstask = FlagdataSetter(fsinputs)
        fstask.flags_to_set(allflagcmds)
        fsresult = self._executor.execute(fstask)

        # Extract "before" and/or "after" summary
        if all(['report' in k for k in fsresult.results[0].keys()]):
            # Go through dictionary of reports...
            for report in fsresult.results[0].keys():
                if fsresult.results[0][report]['name'] == 'before':
                    stats_before = fsresult.results[0][report]
                if fsresult.results[0][report]['name'] == 'after':
                    stats_after = fsresult.results[0][report]
        else:
            # Go through single report.
            if fsresult.results[0]['name'] == 'before':
                stats_before = fsresult.results[0]
            if fsresult.results[0]['name'] == 'after':
                stats_after = fsresult.results[0]

        # If no new flags were found, then no "after" summary was created,
        # so instead make a copy of the "before" summary.
        if not flags:
            stats_after = copy.deepcopy(stats_before)

        return stats_before, stats_after

    def _consolidate_flags(self, flags):
        """
        Method to consolidate a list of FlagCmd objects ("flags").
        """

        # Consolidate by polarisation.
        flags = self._consolidate_flags_with_same_pol(flags)

        # Consolidate by timestamps.
        flags = self._consolidate_flags_by_timestamps(flags)

        return flags

    @staticmethod
    def _consolidate_flags_with_same_pol(flags):
        """
        Method to consolidate a list of FlagCmd objects ("flags") by removing
        flags that differ only in polarisation.

        This method belongs to correctedampflag, by making assumptions on which
        properties of the FlagCmd it needs to compare.
        """

        # Get flag commands.
        flagcmds = [flag.flagcmd for flag in flags]

        # If all flag commands are unique, then there is nothing to
        # consolidate.
        if len(flagcmds) == len(set(flagcmds)):
            cflags = flags

        # If duplicate flag commands exist, go through each one, verify that
        # the duplication is just due to difference in polaristion, and for
        # those where this is true, replace them with a single flag.
        else:
            # Identify the flags that have non-unique flagging commands:
            uval, uind, ucnt = np.unique(flagcmds, return_inverse=True,
                                         return_counts=True)

            # Build new list of flags.
            cflags = []
            for ind, cnt in enumerate(ucnt):
                # For flags that appear twice...
                if cnt == 2:
                    # Identify which flags these were.
                    flag1, flag2 = [flags[i]
                                    for i, val in enumerate(uind)
                                    if val == ind]
                    # Check that the flag commands differ only in polarisation
                    if (flag1.filename == flag2.filename and
                            flag1.spw == flag2.spw and
                            flag1.antenna == flag2.antenna and
                            flag1.intent == flag2.intent and
                            flag1.time == flag2.time and
                            flag1.field == flag2.field and
                            flag1.reason == flag2.reason and
                            flag1.pol != flag2.pol):

                        # Copy across just first flag, but set its polarisation
                        # to empty.
                        flag1.pol = ''
                        cflags.append(flag1)
                        LOG.trace('Consolidated 2 duplicate flags that '
                                  'differed only in polarisation.')
                    # If they differed in a non-anticipated manner, copy across
                    # both.
                    else:
                        cflags.extend([flag1, flag2])
                        LOG.trace('Unable to consolidate 2 flags with same flag '
                                  'command, appear to differ in unanticipated '
                                  'manner.')

                # If flags do not appear twice, they either appear once
                # (commonly expected) or more than twice (never expected).
                # Either way, don't attempt any consolidation for these cases
                # and just copy them to the output array.
                else:
                    cflags.extend([flags[i]
                                   for i, val in enumerate(uind)
                                   if val == ind])
                    if cnt > 2:
                        # If the flag appeared more than twice, something must
                        # have gone wrong, insofar that flags differed by a
                        # metric that is not the polarisation but that was not
                        # included in flagging command. This should not happen,
                        # but log it as a "trace" message for potential
                        # debugging.
                        LOG.trace('Unable to consolidate 3+ flags with same flag '
                                  'command, unanticipated case.')

        return cflags

    @staticmethod
    def _consolidate_flags_by_timestamps(flags):
        """
        Method to consolidate a list of FlagCmd objects ("flags") by removing
        flags with timestamps if the same (ant, spw, field) is covered by
        a flagging command without timestamp (i.e. across all timestamps).

        This method belongs to correctedampflag, by making assumptions on which
        properties of the FlagCmd it needs to compare.
        """

        # Identify list of properties of flag commands without timestamps.
        flags_without_timestamps = [
            (flag.filename, flag.spw, flag.antenna, flag.intent, flag.field)
            for flag in flags if flag.time is None]

        # If flag commands without timestamp exist, go through every flag
        # command and remove the ones that are cover the same
        # (filename, intent, field, spw, antenna), ignoring differences in
        # reason and polarisation.
        if flags_without_timestamps:

            # Build new list of flags, and preserve skipped flag commands to
            # report.
            cflags = []
            skipped_flagcmds = []

            # Go through list of flags.
            for flag in flags:

                # If the current flag matches in properties with one of the
                # flagging commands without timestamps:
                if (flag.filename, flag.spw, flag.antenna, flag.intent, flag.field) in flags_without_timestamps:

                    # Preserve flag if it has no timestamp.
                    if flag.time is None:
                        cflags.append(flag)
                    # Skip flags that have explicit timestamps.
                    else:
                        skipped_flagcmds.append(flag.flagcmd)

                # If the current flag does not match with any of the flagging
                # commands without timestamps, then preserve it.
                else:
                    cflags.append(flag)

            # Log a summary of consolidated flag commands:
            LOG.info('The following time-specific flag commands were '
                     'consolidated into one or more flag commands '
                     'without a timestamp:\n'
                     '{}'.format('\n'.join(skipped_flagcmds)))

        # If no flag commands with timestamp exist, then there is nothing to
        # consolidate => return flags unmodified.
        else:
            cflags = flags

        return cflags

    @staticmethod
    def _propagate_phase_flags(flags, ms, antenna_id_to_name):
        """
        Method to take a list of FlagCmd objects ("flags") and propagate
        flags with reason = 'bad baseline' and intent = PHASE to intents
        TARGET and CHECK.
        """

        # Intents to propagate to.
        intents_propto = ["TARGET", "CHECK"]

        # Check for presence of intents in current MS, and if valid intent,
        # retrieve corresponding fields from MS.
        valid_intents = []
        valid_intents_fields = {}
        for intent in intents_propto:
            casa_intent = utils.to_CASA_intent(ms, intent)
            if casa_intent:
                valid_intents.append(casa_intent)
                fields = {field.name
                          for field in ms.get_fields(intent=intent)}
                valid_intents_fields[casa_intent] = ','.join(fields)

        # Proceed if there are valid intents to propagate to.
        propagated_flags = []
        nr_propagated_flags = 0
        if valid_intents:

            # Go through each flag, looking for 'bad baseline' reason and
            # "PHASE" intent...
            for flag in flags:
                if (flag.reason == 'bad baseline'
                        and flag.intent == utils.to_CASA_intent(ms, "PHASE")):

                    nr_propagated_flags += 1

                    # If a match was found, propagate to each of the valid
                    # intents.
                    for intent in valid_intents:
                        propagated_flags.append(
                            FlagCmd(
                                filename=flag.filename,
                                spw=flag.spw,
                                antenna=flag.antenna,
                                intent=intent,
                                pol=flag.pol,
                                field=valid_intents_fields[intent],
                                reason='bad baseline propagated from PHASE',
                                antenna_id_to_name=antenna_id_to_name))

        if propagated_flags:
            LOG.info('Propagated {} flagging command(s) with reason '
                     '\"bad baseline\" from PHASE intent to TARGET '
                     'and CHECK intent (where present).'.format(nr_propagated_flags))
            flags.extend(propagated_flags)

        return flags
