from __future__ import absolute_import

import collections
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
        if self._field is not None:
            return self._field

        if isinstance(self.vis, list):
            return self._handle_multiple_vis('field')

        # By default, return the fields corresponding to the input
        # intent.
        fieldids = [str(field.id) for field in self.ms.fields
                    if self.intent in field.intents]
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
    # timestamps" that an antenna may be a part of; equivalent to:
    # tooManyBaselinesFraction
    @property
    def tmbl(self):
        return self._tmbl

    @tmbl.setter
    def tmbl(self, value):
        if value is None:
            value = 0.18
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

        # Set "default" scale factor by which the thresholds
        # tmint and tmbl should be scaled.
        thresh_scale_factor = 1.0

        # Set threshold for maximum fraction of outlier baseline scans that
        # a single antenna can be involved in.
        ant_in_outlier_baseline_scans_thresh = 0.5

        # Set threshold for maximum fraction of antennas that can be involved
        # in outlier baseline scans.
        ants_in_outlier_baseline_scans_thresh = 0.33333

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

        # Get number of scans in MS for this intent.
        nscans = len(ms.get_scans(scan_intent=inputs.intent))

        # If there are multiple scans for this intent, then double the
        # threshold for timestamps with outliers.
        if nscans > 1:
            inputs.tmantint *= 2

        # Initialize list of newly found flags.
        newflags = []

        # Evaluate flagging heuristics separately for each field.
        for fieldid in utils.safe_split(inputs.field):

            # Evaluate flagging heuristics separately for each spw.
            for spwid in spwids:

                LOG.info(
                    'Reading data for spw {0}, intent {1}, and field'
                    ' {2}'.format(spwid, inputs.intent, fieldid))

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
                with casatools.MSReader(ms.name) as openms:
                    try:
                        # Select data for current field, intent, spw.
                        openms.msselect(
                            {'field': fieldid,
                             'scanintent': '*%s*' % inputs.intent,
                             'spw': str(spwid)})

                        # Set channel selection to take the average of all
                        # channels.
                        openms.selectchannel(1, 0, nchans, 1)
                    except:
                        LOG.warning(
                            'Unable to compute flagging for '
                            'spw {0}, intent {1}, and field {2}'.format(
                                spwid, inputs.intent, fieldid))
                        openms.close()
                        # Continue to next field.
                        continue

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
                cmetric_all = np.abs(corrdata)-np.abs(modeldata)

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
                    n_time_with_highsig_thresh_frac = inputs.tmantint * len(np.unique(time))
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
                    if inputs.tmantint > 0:
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
                        if inputs.tmantint <= 0:
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
                                # one antenna dominates, or if a majority are
                                # affected, and set flags accordingly.
                                for timestamp in time_sel_highsig_uniq:

                                    # Identify baseline scans within this timestamp.
                                    id_scans_with_timestamp = np.where(
                                        time_sel_highsig == timestamp)[0]
                                    n_scans_with_timestamp = len(id_scans_with_timestamp)

                                    # Get a list of antennas involved in baseline
                                    # scans matching the timestamp.
                                    ants_in_timestamp = np.concatenate(
                                        [ant1_sel[id_highsig][id_scans_with_timestamp],
                                         ant2_sel[id_highsig][id_scans_with_timestamp]])

                                    # Identify the antenna most commonly involved in
                                    # outlier baseline scans.
                                    ant_in_most_bad_timestamps = max(set(ants_in_timestamp),
                                                                     key=ants_in_timestamp.count)

                                    # If the most common antenna involved in outlier baseline scans
                                    # equals-or-exceeds the threshold, then flag this antenna.
                                    if (ants_in_timestamp.count(ant_in_most_bad_timestamps) >=
                                            ant_in_outlier_baseline_scans_thresh * n_scans_with_timestamp):
                                        # Create a flagging command for this antenna
                                        newflags.append(
                                            FlagCmd(
                                                filename=ms.name,
                                                spw=spwid,
                                                antenna=ant_in_most_bad_timestamps,
                                                intent=utils.to_CASA_intent(ms, inputs.intent),
                                                pol=icorr,
                                                time=timestamp,
                                                field=fieldid,
                                                reason='bad antenna'))
                                    # If there was not a single antenna that was involved
                                    # in more than the threshold fraction of outlier baseline
                                    # scans, then continue checking whether a significant fraction of
                                    # antennas are involved, and flag entire timestamp (for all
                                    # ants, for given spw, intent, pol) if so.
                                    else:
                                        antcnts = np.bincount(ants_in_timestamp, minlength=nants)
                                        id_affected_ants = np.where(antcnts >= max([1, antcnts.max()-1]))[0]
                                        if len(id_affected_ants) > ants_in_outlier_baseline_scans_thresh * nants:
                                            # Create a flagging command for all antennas
                                            # in this timestamp (for given spw, intent, pol).
                                            newflags.append(
                                                FlagCmd(
                                                    filename=ms.name,
                                                    spw=spwid,
                                                    intent=utils.to_CASA_intent(ms, inputs.intent),
                                                    pol=icorr,
                                                    time=timestamp,
                                                    field=fieldid,
                                                    reason='bad timestamp'))
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

                        # Compute final threshold for maximum fraction of "bad
                        # baselines" over "all baselines" that an antenna may be
                        # a part of.
                        tmbl_scaled = inputs.tmbl * thresh_scale_factor

                        # Compute for each antenna how many "bad baselines" it is
                        # a part of.
                        ant_in_bad_bl_count = np.bincount(sum(bad_bls, ()),
                                                          minlength=nants)

                        # TODO: equivalent to line 579  (remove before final commit)
                        # Identify "bad antennas" as those antennas involved in a number of
                        # "bad baselines" that equals-or-exceeds the fraction threshold
                        # of all baselines that this antenna is part of.
                        bad_ants = np.where(ant_in_bad_bl_count >= tmbl_scaled * (nants-1))[0]

                        # Create flagging command for each identified bad antenna.
                        for bad_ant in bad_ants:
                            newflags.append(
                                FlagCmd(
                                    filename=ms.name,
                                    spw=spwid,
                                    antenna=bad_ant,
                                    intent=utils.to_CASA_intent(ms, inputs.intent),
                                    pol=icorr,
                                    field=fieldid,
                                    reason='bad antenna'))

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
                                        intent=utils.to_CASA_intent(ms, inputs.intent),
                                        pol=icorr,
                                        field=fieldid,
                                        reason='bad baseline'))

        # TODO: add consolidation of flagging commands?

        # Apply newly found flags.
        stats_before, stats_after = {}, {}
        if newflags:
            LOG.warning('Evaluation of {0} raised {1} flagging '
                        'command(s)'.format(os.path.basename(inputs.vis),
                                            len(newflags)))

            LOG.info('Applying newly found flags.')

            # Add before/after summary:
            allflagcmds = ["mode='summary' name='before'"]
            allflagcmds.extend(newflags)
            allflagcmds.append("mode='summary' name='after'")

            # Run flag setting.
            fsinputs = FlagdataSetter.Inputs(
                context=inputs.context, vis=inputs.vis, table=inputs.vis,
                inpfile=[])
            fstask = FlagdataSetter(fsinputs)
            fstask.flags_to_set(allflagcmds)
            fsresult = self._executor.execute(fstask)

            # Extract "before" and/or "after" summary
            # Go through dictionary of reports...
            for report in fsresult.results[0].keys():
                if fsresult.results[0][report]['name'] == 'before':
                    stats_before = fsresult.results[0][report]
                if fsresult.results[0][report]['name'] == 'after':
                    stats_after = fsresult.results[0][report]
        else:
            LOG.info('Evaluation of {0} raised 0 flagging commands'.format(
                os.path.basename(inputs.vis)))

        # Store newly identified flags in result.
        result.addflags(newflags)

        # Attach flagging summaries to result
        result.summaries = [stats_before, stats_after]

        return result

    def analyse(self, result):
        return result
