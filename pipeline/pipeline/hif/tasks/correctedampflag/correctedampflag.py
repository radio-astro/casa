from __future__ import absolute_import

import collections
import copy
import os

import numpy as np

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils
from pipeline.h.tasks.common import commonhelpermethods
from pipeline.h.tasks.common.arrayflaggerbase import FlagCmd
from pipeline.h.tasks.flagging.flagdatasetter import FlagdataSetter
from .resultobjects import CorrectedampflagResults

LOG = infrastructure.get_logger(__name__)


class CorrectedampflagInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, intent=None,
                 field=None, spw=None, antnegsig=None, antpossig=None,
                 tmantint=None, tmint=None, tmbl=None, antblnegsig=None,
                 antblpossig=None, relaxed_factor=None):
        self._init_properties(vars())

    @property
    def intent(self):
        if self._intent is not None:
            return self._intent

        if isinstance(self.vis, list):
            return self._handle_multiple_vis('intent')

        return 'BANDPASS'

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

    # Lower sigma threshold for identifying outliers as a result of bad
    # antennas within individual timestamps; equivalent to:
    # relaxationSigma
    @property
    def antnegsig(self):
        return self._antnegsig

    @antnegsig.setter
    def antnegsig(self, value):
        if value is None:
            value = 6.5
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
    def tmantint(self):
        return self._tmantint

    @tmantint.setter
    def tmantint(self, value):
        if value is None:
            value = 0.06
        self._tmantint = value

    # Initial threshold for maximum fraction of "outlier timestamps" over
    # "total timestamps" that a baseline may be a part of; equivalent to:
    # tooManyIntegrationsFraction
    @property
    def tmint(self):
        return self._tmint

    @tmint.setter
    def tmint(self, value):
        if value is None:
            value = 0.09
        self._tmint = value

    # Initial threshold for maximum fraction of "bad baselines" over "all
    # baselines" that an antenna may be a part of; equivalent to:
    # tooManyBaselinesFraction
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

        # Get number of antennas.
        antenna_names, antenna_ids = \
            commonhelpermethods.get_antenna_names(ms)
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

                    flags_for_intent_field_spw = self.evaluate_heuristic(
                        ms, intent, field, spwid, nants,
                        antenna_id_to_name)
                    newflags.extend(flags_for_intent_field_spw)

        if newflags:
            # Propagate PHASE 'bad baseline' flags to TARGET.
            newflags = self._propagate_phase_flags(newflags, ms, antenna_id_to_name)

            # Consolidate flagging commands that differ only by polarisation.
            newflags = self._consolidate_flags_with_same_pol(newflags)

        # Apply flags and get before/after summary.
        stats_before, stats_after = self.apply_flags(newflags)

        # Store newly identified flags in result.
        result.addflags(newflags)

        # Attach flagging summaries to result
        result.summaries = [stats_before, stats_after]

        return result

    def analyse(self, result):
        return result

    def evaluate_heuristic(self, ms, intent, field, spwid, nants,
                           antenna_id_to_name):

        inputs = self.inputs

        # Set "default" scale factor by which the thresholds
        # tmint and tmbl should be scaled.
        thresh_scale_factor = 1.0

        # Set threshold for maximum fraction of outlier baseline scans that
        # a single antenna can be involved in.
        max_frac_outlier_scans = 0.5

        # Set threshold for maximum fraction of antennas that can be involved
        # in outlier baseline scans.
        ants_in_outlier_baseline_scans_thresh = 1.0/3.0

        # Set threshold for minimum number of "bad baselines" than an antenna
        # may be a part of without getting flagged.
        tmbl_minbadnr = 4.0

        # Initialize flags.
        newflags = []

        # Get number of scans in MS for this intent.
        nscans = len(ms.get_scans(scan_intent=intent))

        # If there are multiple scans for this intent, then double the
        # threshold for timestamps with outliers.
        if nscans > 1:
            tmantint = inputs.tmantint * 2
        else:
            tmantint = inputs.tmantint

        # Get number of channels and correlations for this spw.
        nchans = ms.get_spectral_windows(str(spwid))[0].num_channels
        ncorrs = len(commonhelpermethods.get_corr_products(ms, spwid))

        # Read in data from MS.
        #
        # TODO: compare data selection approaches:
        #
        # Option 1 (used by Todd):
        #
        # with casatools.MSMDReader(ms.name) as openmsmd:
        #     scans = openmsmd.scansforintent('*'+inputs.intent+'*')
        #     scans_string = ','.join([str(scan) for scan in scans])
        #
        # with casatools.MSReader(ms.name) as openms:
        #     try:
        #         # Select data for current field, intent, spw.
        #         openms.msselect({'field': fieldid,
        #                          'intent': str(inputs.intent),
        #                          'spw': str(spwid),
        #                          'scan': scans_string})
        #
        # Option 2:
        #
        LOG.info(
            'Reading data for intent {}, field {}, and spw '
            '{}'.format(intent, field, spwid))
        with casatools.MSReader(ms.name) as openms:
            try:
                # Select data for current field, intent, spw.
                openms.msselect(
                    {'field': field,
                     'scanintent': '*%s*' % utils.to_CASA_intent(ms, intent),
                     'spw': str(spwid)})

                # Set channel selection to take the average of all
                # channels.
                openms.selectchannel(1, 0, nchans, 1)
            except:
                LOG.warning(
                    'Unable to compute flagging for intent {}, field {}, spw '
                    ' {}'.format(intent, field, spwid))
                openms.close()
                # Return early.
                return newflags

            # Extract data from MS.
            data = openms.getdata(
                ['corrected_data', 'model_data', 'antenna1',
                 'antenna2', 'flag', 'time'])

        # Remove the channel dimension (should be of length 1 as we asked
        # for average across all channels).
        corrdata = np.squeeze(data['corrected_data'])
        modeldata = np.squeeze(data['model_data'])
        flag_all = np.squeeze(data['flag'])

        # Compute "scalar difference" between corrected data and
        # model data.
        cmetric_all = np.abs(corrdata) - np.abs(modeldata)

        # Select non-autocorrelations.
        id_nonac = np.where(data['antenna1'] != data['antenna2'])
        time = data['time'][id_nonac]
        ant1 = data['antenna1'][id_nonac]
        ant2 = data['antenna2'][id_nonac]

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

            # TODO: equivalent to line 406  (remove before final commit)
            # Select for non-flagged data and non-NaN data.
            id_nonbad = np.where(np.logical_and(
                np.logical_not(flag),
                np.isfinite(cmetric)))
            cmetric_sel = cmetric[id_nonbad]
            time_sel = time[id_nonbad]
            ant1_sel = ant1[id_nonbad]
            ant2_sel = ant2[id_nonbad]

            # TODO: check whether an empty selection from this point on would
            # be handled correctly.

            # Compute the median. Assuming the distribution is normal,
            # compute a robust estimate of the standard deviation as
            # 1.4826 x the median absolute deviation from the median.
            med = np.median(cmetric_sel)
            mad = np.median(np.abs(cmetric_sel - np.median(cmetric_sel))) * 1.4826

            # The following part checks for each timestamp whether the
            # outliers are either
            #  a.) mostly due to a single antenna => flag this ant for this timestamp, or
            #  b.) due to a significant fraction of antennas => flag entire timestamp

            # If an antenna based bad integrations fraction was
            # provided, then identify both negative and positive
            # outliers; otherwise just identify negative outliers.
            if tmantint > 0:
                id_highsig = np.where(
                    np.logical_or(
                        cmetric_sel < (med - mad * inputs.antnegsig),
                        cmetric_sel > (med + mad * inputs.antpossig)))[0]
            else:
                id_highsig = np.where(
                    cmetric_sel < (med - mad * inputs.antnegsig))[0]

            # TODO: equivalent to line 467  (remove before final commit)
            # If outliers were found...
            if len(id_highsig) > 0:

                # If not testing for positive outliers...
                if tmantint <= 0:
                    # Set the scale factor for the maximum threshold
                    # for outlier timestamps per baseline to a
                    # relaxed value.
                    thresh_scale_factor = inputs.relaxed_factor

                else:
                    # Check whether the outliers were concentrated in only one or a small
                    # fraction (set by bad_int_frac) of timestamps.

                    # Identify timestamps with outliers
                    time_sel_highsig = time_sel[id_highsig]
                    time_sel_highsig_uniq = np.unique(time_sel_highsig)

                    # If all outliers were concentrated within small number of timestamps
                    # set by n_max_highsig, then check for each of those timestamps whether either
                    # a.) most outliers involved just one antenna, in which case flag this
                    # ant for that timestamp, or
                    # b.) a significant fraction of antennas were involved in outlier
                    # baselines, in which case flag the timestamp.
                    # TODO: equivalent to line 485  (remove before final commit)
                    if len(time_sel_highsig_uniq) <= n_time_with_highsig_max:

                        # For each of the few bad timestamps, determine if
                        # one antenna dominates among the outlier scans
                        # within the timestamp, or if a majority are
                        # affected, and set flags accordingly.
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
                            # If there was no significant fraction of affected antennas,
                            # the proceed check if the antenna(s) with the highest number of
                            # outlier scans (within this timestamp) equals-or-exceeds the threshold
                            # and flag the corresponding antenna(s).
                            elif antcnts.max() >= max_frac_outlier_scans * n_outlier_scans_in_timestamp:

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
                                            reason='bad antenna',
                                            antenna_id_to_name=antenna_id_to_name))
                            else:
                                pass
                    # If all outliers were not concentrated within a small number
                    # of timestamps...
                    else:
                        # Set the scale factor for the maximum threshold
                        # for outlier timestamps per baseline to a
                        # relaxed value.
                        thresh_scale_factor = inputs.relaxed_factor

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

            # TODO: equivalent to line 531  (remove before final commit)
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
                # TODO: equivalent to line 570  (remove before final commit)
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

                # TODO: equivalent to line 563  (remove before final commit)
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

                # TODO: equivalent to line 579  (remove before final commit)
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

                # TODO: equivalent to line 674  (remove before final commit)
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

    def apply_flags(self, flags):

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
    def _propagate_phase_flags(flags, ms, antenna_id_to_name):

        # Intents to propagate to.
        intents_propto = ["TARGET", "CHECK"]

        # Check for presence of intents in current MS.
        casa_intents_propto = [utils.to_CASA_intent(ms, intent)
                               for intent in intents_propto]
        valid_intents = [intent for intent in casa_intents_propto
                         if intent]

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
                                field=flag.field,
                                reason='bad baseline propagated from PHASE',
                                antenna_id_to_name=antenna_id_to_name))

        if propagated_flags:
            LOG.info('Propagated {} flagging command(s) with reason '
                     '\"bad baseline\" from PHASE intent to TARGET '
                     'and CHECK intent (where present).'.format(nr_propagated_flags))
            flags.extend(propagated_flags)

        return flags
