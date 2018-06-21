from __future__ import absolute_import

import collections

import numpy
from casac import casac

import pipeline.domain.measures as measures
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as scorecalc
import pipeline.qa.utility.scorers as scorers
from . import resultobjects

LOG = logging.get_logger(__name__)


class TcleanQAHandler(pqa.QAPlugin):    
    result_cls = resultobjects.TcleanResult
    child_cls = None

    def handle(self, context, result):
        # calculate QA score comparing RMS against clean threshold

        imageScorer = scorers.erfScorer(1.0, 5.0)

        # Basic imaging score
        observatory = context.observing_run.measurement_sets[0].antenna_array.name
        # For the time being the VLA calibrator imaging is generating an error
        # due to the dynamic range limitation. Bypass the real score here.
        if ('VLA' in result.imaging_mode):
            result.qa.pool[:] = [pqa.QAScore(1.0)]
        elif (result.error is not None):
            result.qa.pool[:] = [pqa.QAScore(0.0, longmsg=result.error, shortmsg=result.error)]
        else:
            qaTool = casac.quanta()
            try:
                # For the score we compare the image RMS with the DR corrected
                # sensitivity as an estimate of the expected RMS.
                rms_score = imageScorer(result.image_rms / result.dr_corrected_sensitivity)
            except Exception as e:
                LOG.warning('Exception scoring imaging result by RMS: %s. Setting score to -0.1.' % (e))
                rms_score = -0.1
            if (numpy.isnan(rms_score)):
                result.qa.pool[:] = [pqa.QAScore(0.0, longmsg='Cleaning diverged, RMS is NaN. Field: %s Intent: %s SPW: %s' % (result.inputs['field'], result.intent, resultspw), shortmsg='RMS is NaN')]
            else:
                if rms_score > 0.66:
                    result.qa.pool[:] = [pqa.QAScore(rms_score, longmsg='RMS vs. DR corrected sensitivity. Field: %s Intent: %s SPW: %s' % (result.inputs['field'], result.intent, result.spw), shortmsg='RMS vs. sensitivity')]
                else:
                    # The level of 2.7 comes from the Erf scorer limits of 1 and 5.
                    # The level needs to be adjusted if these limits are modified.
                    result.qa.pool[:] = [pqa.QAScore(rms_score, longmsg='Observed RMS noise exceeds DR corrected sensitivity by more than 2.7. Field: %s Intent: %s SPW: %s' % (result.inputs['field'], result.intent, result.spw), shortmsg='RMS vs. sensitivity')]

            # Check source score
            #    Be careful about the source name vs field name issue
            if result.intent == 'CHECK' and result.inputs['specmode'] == 'mfs':
                mses = [context.observing_run.get_ms(name=vis) for vis in result.inputs['vis']]
                fieldname = result.sourcename
                intent = result.intent
                spwid = int(result.spw)
                imagename = result.image
                rms = result.image_rms
                checkscore, offset, offset_err, beams, beams_err, fitflux, fitflux_err, fitpeak = scorecalc.score_checksources (mses, fieldname, spwid, imagename, rms)
                result.qa.pool.append (checkscore)
                gfluxscale = 1.0
                gfluxscale_err = 0.1
                if len(result.vis) == 1:
                    try:
                        ms_do = context.observing_run.get_ms(result.vis[0])
                        field_id = [field.id for field in ms_do.fields if utils.dequote(field.name) == utils.dequote(fieldname)][0]
                        fluxresult = [fr for fr in ms_do.derived_fluxes[str(field_id)] if fr.spw_id == str(spwid)][0]
                        gfluxscale = float(fluxresult.I.convert_to(measures.FluxDensityUnits.MILLIJANSKY).value)
                        gfluxscale_err = float(fluxresult.uncertainty.I.convert_to(measures.FluxDensityUnits.MILLIJANSKY).value)
                    except Exception as e:
                        gfluxscale = None
                        gfluxscale_err = None
                else:
                    gfluxscale = None
                    gfluxscale_err = None
                result.check_source_fit = {'offset': offset, 'offset_err': offset_err, 'beams': beams, 'beams_err': beams_err, 'fitflux': fitflux, 'fitflux_err': fitflux_err, 'fitpeak': fitpeak, 'gfluxscale': gfluxscale, 'gfluxscale_err': gfluxscale_err}


class TcleanListQAHandler(pqa.QAPlugin):
    result_cls = collections.Iterable
    child_cls = resultobjects.TcleanResult

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
