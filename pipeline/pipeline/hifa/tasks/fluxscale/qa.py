from __future__ import absolute_import

import collections
import operator

import numpy

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc
from pipeline.domain.measures import FluxDensityUnits, Frequency, FrequencyUnits
from pipeline.extern.analysis_scripts.analysisUtils import gaincalSNR
from pipeline.h.tasks.common import commonfluxresults
from . import gcorfluxscale
from ..importdata.dbfluxes import ORIGIN_DB

LOG = logging.get_logger(__name__)


class GcorFluxscaleQAHandler(pqa.QAPlugin):
    result_cls = commonfluxresults.FluxCalibrationResults
    child_cls = None
    generating_task = gcorfluxscale.GcorFluxscale

    def handle(self, context, result):
        vis= result.inputs['vis']
        ms = context.observing_run.get_ms(vis)

        # Check for existance of field / spw combinations for which
        # the derived fluxes are missing.
        score1 = self._missing_derived_fluxes(ms, result.inputs['transfer'], result.inputs['transintent'],
                                              result.measurements)
        score2 = self._low_snr_fluxes(ms, result.measurements)
        scores = [score1, score2]

        scores.extend(score_kspw(context, result))

        result.qa.pool.extend(scores)
    
    def _missing_derived_fluxes(self, ms, field, intent, measurements):
        """
        Check whether there are missing derived fluxes. 
        """
        return qacalc.score_missing_derived_fluxes(ms, field, intent, measurements)

    def _low_snr_fluxes(self, ms, measurements):
        """
        Check whether there are low SNR derived fluxes. 
        """
        return qacalc.score_derived_fluxes_snr(ms,  measurements)


class GcorFluxscaleListQAHandler(pqa.QAPlugin):
    """
    QA handler for a list containing FluxCalibrationResults.
    """
    result_cls = collections.Iterable
    child_cls = commonfluxresults.FluxCalibrationResults
    generating_task = gcorfluxscale.GcorFluxscale

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated

        mses = [r.inputs['vis'] for r in result]
        longmsg = 'No missing derived fluxes in %s' % utils.commafy(mses,
                                                                    quotes=False,
                                                                    conjunction='or')
        result.qa.all_unity_longmsg = longmsg


def score_kspw(context, result):
    # Spec from CAS-10792:
    #
    # QA score 1: "internal spw-spw consistency":
    #
    # Use a ratio of the (gflux flux for the SPW in question) / (catalog
    # flux for the SPW in question) / ( the same thing calculated for the
    # highest SNR wide [>= 1 GHz] SPW). More precisely, defining
    # r_SPW = (gflux flux for SPW) / (catalog flux for SPW), I suggest
    # using K_SPW = r_spw / r_max_snr_spw as the metric. If there are no
    # >= 1 GHz SPWs, use the highest SNR SPW which has a bandwidth greater
    # than or equal to the median bandwidth of all SPWs. SNR is to be
    # calculated from au.gaincalSNR() or equivalent, not from the SNR
    # implied by the weblog, which is less reliable in general.

    vis = result.inputs['vis']
    ms = context.observing_run.get_ms(vis)

    # identify the caltable for this measurement set
    for caltable_path in context.callibrary.active.get_caltable(caltypes='tsys'):
        with casatools.TableReader(caltable_path) as table:
            msname = table.getkeyword('MSName')
        if msname in vis:
            break
    else:
        # No matching caltable. That's ok, gaincalSNR will do without.
        caltable_path = ''

    # If there is more than one phase calibrator, then pick the first one that
    # does NOT also have observe_target intent. If all have both intents, then
    # continue to use the first one.
    candidate_phase_fields = [f for f in ms.get_fields(intent='PHASE') if 'TARGET' not in f.intents]
    if not candidate_phase_fields:
        candidate_phase_fields = ms.get_fields(intent='PHASE')
    phase_field = min(candidate_phase_fields, key=lambda f: f.time.min())
    if not phase_field:
        LOG.warning('Error calculating internal spw-spw consistency: no phase calibrator')
        return []

    # take catalogue fluxes, adding fluxes for solar system amplitude
    # calibrators found in the setjy stage
    phase_fluxes = []
    for fd in [fd for fd in phase_field.flux_densities if fd.origin == ORIGIN_DB]:
        spw = ms.get_spectral_window(fd.spw_id)
        phase_fluxes[spw] = (spw.id, spw.frequency.to_units(FrequencyUnits.HERTZ), fd.to_units(FluxDensityUnits.JANSKY))
    if not phase_fluxes:
        LOG.warning('Error calculating internal spw-spw consistency: no catalogue fluxes for phase calibrator ({})'
                    ''.format(utils.dequote(phase_field.name)))
        return []

    # gather spw ID for all measurements in the result
    measurement_spw_ids = {fd.spw_id for measurements in result.measurements.values() for fd in measurements}

    # run gaincalSNR
    gaincalSNR_output = gaincalSNR(vis, tsysTable=caltable_path, spw=list(measurement_spw_ids), field=phase_field.id,
                                   flux=phase_fluxes)

    if not gaincalSNR_output:
        LOG.warning('Error calculating internal spw-spw consistency: no result from aU.gaincalSNR')
        return []

    gaincalSNR_spw_ids = {k for k, v in gaincalSNR_output.iteritems() if k in measurement_spw_ids}
    if not gaincalSNR_spw_ids.issuperset(measurement_spw_ids):
        LOG.error('Error calculating internal spw-spw consistency: could not identify highest SNR spectral window')
        return []

    # this will hold QA scores for all fields
    all_scores = []

    one_ghz = Frequency(1, FrequencyUnits.GIGAHERTZ)

    for field_id, measurements in result.measurements.iteritems():
        # get domain object for the field.
        fields = ms.get_fields(task_arg=field_id)
        assert len(fields) == 1
        field = fields[0]

        # these strings will be used repeatedly in log messages
        msg_intents = ','.join(field.intents)
        msg_fieldname = utils.dequote(field.name)

        # get domain objects for the flux measurement spws
        spw_ids = [m.spw_id for m in measurements]
        measurement_spws = ms.get_spectral_windows(','.join([str(i) for i in spw_ids]))

        # discard narrow windows < 1GHz
        spw_snr_candidates = [spw for spw in measurement_spws if spw.bandwidth >= one_ghz]

        # fall back to median bandwidth selection if all the windows are narrow
        if not spw_snr_candidates:
            LOG.info('No wide (>= 1 GHz) spectral windows identified for {} ({})'.format(msg_fieldname, msg_intents))

            # find median bandwidth of all spws...
            bandwidths = [spw.bandwidth.to_units(FrequencyUnits.HERTZ) for spw in measurement_spws]
            median_bandwidth = Frequency(numpy.median(bandwidths), FrequencyUnits.HERTZ)

            # ... and identify SNR spw candidates accordingly
            LOG.info('Taking highest SNR window from spws with bandwidth >= {} GHz'
                     ''.format(median_bandwidth.to_units(FrequencyUnits.GIGAHERTZ)))
            spw_snr_candidates = [spw for spw in measurement_spws if spw.bandwidth >= median_bandwidth]

        # find the spw with the highest SNR
        highest_SNR_spw = max(spw_snr_candidates, key=lambda spw: gaincalSNR_output[spw.id])

        # now find the measurement for that spw
        highest_SNR_measurement = [m for m in measurements if m.spw_id == highest_SNR_spw.id]
        assert (len(highest_SNR_measurement) == 1)
        highest_SNR_measurement = highest_SNR_measurement[0]
        highest_SNR_I = highest_SNR_measurement.I

        # find the catalogue flux for the highest SNR spw
        catalogue_fluxes = [f for f in field.flux_densities
                            if f.origin == ORIGIN_DB
                            and f.spw_id == highest_SNR_measurement.spw_id]
        if not catalogue_fluxes:
            LOG.warning('Cannot calculate internal spw-spw consistency for {} ({}): no catalogue measurement for '
                        'highest SNR spw ({})'.format(msg_fieldname, msg_intents, highest_SNR_measurement.spw_id))
            continue
        assert (len(catalogue_fluxes) is 1)
        catalogue_flux = catalogue_fluxes[0]

        # r_snr = ratio of derived flux to catalogue flux for highest SNR spw
        r_snr = highest_SNR_I.to_units(FluxDensityUnits.JANSKY) / catalogue_flux.I.to_units(FluxDensityUnits.JANSKY)

        # now calculate r for remaining measurements in other spws
        other_measurements = [m for m in measurements if m is not highest_SNR_measurement]
        # note that we do not include r_snr, as by definition it is the ratio
        # to which all other spws are compared, and hence has a QA score of 1.0
        k_spws = []
        for m in other_measurements:
            catalogue_fluxes = [f for f in field.flux_densities
                                if f.origin == ORIGIN_DB
                                and f.spw_id == highest_SNR_measurement.spw_id]
            if not catalogue_fluxes:
                LOG.info('No catalogue measurement for {} ({}) spw {}'.format(msg_fieldname, msg_intents, m.spw_id))
                continue
            assert (len(catalogue_fluxes) is 1)
            catalogue_flux = catalogue_fluxes[0]
            r_spw = m.I.to_units(FluxDensityUnits.JANSKY) / catalogue_flux.I.to_units(FluxDensityUnits.JANSKY)
            k_spw = r_spw / r_snr
            k_spws.append((m.spw_id, k_spw))

        # sort QA scores by spw
        k_spws.sort(key=operator.itemgetter(0))
        field_qa_scores = [qacalc.score_gfluxscale_k_spw(ms.basename, field, spw_id, k_spw)
                           for spw_id, k_spw in k_spws]
        all_scores.extend(field_qa_scores)

        # Messages should appear for each SPW that does not correspond to
        # perfect score.
        for spw_id, k_spw in k_spws:
            q = abs(1 - k_spw)
            msg = ('The flux density of field {} ({}) in spw {} deviates by {:.0%} from the expected value'
                   ''.format(msg_fieldname, msg_intents, spw_id, k_spw))

            if q < 0.1:
                continue
            elif q < 0.2:
                LOG.info(msg)
            elif q < 0.3:
                LOG.warning(msg)
            else:
                LOG.error(msg)

    return all_scores
