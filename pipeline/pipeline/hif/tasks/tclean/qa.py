from __future__ import absolute_import

import collections

import os
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
                try:
                    mses = [context.observing_run.get_ms(name=vis) for vis in result.inputs['vis']]
                    fieldname = result.sourcename
                    intent = result.intent
                    spwid = int(result.spw)
                    imagename = result.image
                    rms = result.image_rms
                    checkscore, offset, offset_err, beams, beams_err, fitflux, fitflux_err, fitpeak = scorecalc.score_checksources (mses, fieldname, spwid, imagename, rms)
                    result.qa.pool.append (checkscore)
                    warnings = []
                    if len(result.vis) == 1:
                        try:
                            ms_do = context.observing_run.get_ms(result.vis[0])
                            field_id = [field.id for field in ms_do.fields if utils.dequote(field.name) == utils.dequote(fieldname)][0]
                            fluxresult = [fr for fr in ms_do.derived_fluxes[str(field_id)] if fr.spw_id == str(spwid)][0]
                            gfluxscale = float(fluxresult.I.convert_to(measures.FluxDensityUnits.MILLIJANSKY).value)
                            gfluxscale_err = float(fluxresult.uncertainty.I.convert_to(measures.FluxDensityUnits.MILLIJANSKY).value)
                        except Exception as e:
                            warnings.append('undefined gfluxscale fit')
                            gfluxscale = None
                            gfluxscale_err = None
                    else:
                        gfluxscale = None
                        gfluxscale_err = None

                    if beams is None:
                        warnings.append('unfitted offset')
                    elif beams > 0.15:
                        warnings.append('large fitted offset of %.2f marcsec and %.2f synth beam' % (offset, beams))

                    if gfluxscale is None:
                        warnings.append('undefined gfluxscale result')
                    elif gfluxscale == 0.0:
                        warnings.append('gfluxscale value of 0.0 mJy')
                    else:
                        chk_fitflux_gfluxscale_ratio = fitflux * 1000. / gfluxscale
                        if chk_fitflux_gfluxscale_ratio < 0.8:
                            warnings.append('a low [Fitted / gfluxscale] Flux Density Ratio of %.2f' % (chk_fitflux_gfluxscale_ratio))

                    if fitflux is None:
                        warnings.append('undefined check fit result')
                    elif fitflux == 0.0:
                        warnings.append('Fitted Flux Density value of 0.0 mJy')
                    else:
                        chk_fitpeak_fitflux_ratio = fitpeak / fitflux
                        if chk_fitpeak_fitflux_ratio < 0.7:
                            warnings.append('low Fitted [Peak Intensity / Flux Density] Ratio of %.2f' % (chk_fitpeak_fitflux_ratio))

                    snr_msg = ''
                    if gfluxscale is not None and gfluxscale_err is not None:
                        if gfluxscale_err != 0.0:
                            chk_gfluxscale_snr = gfluxscale / gfluxscale_err
                            if chk_gfluxscale_snr < 20.:
                               snr_msg = ', however, the S/N of the gfluxscale measurement is low'

                    if warnings != []:
                        msnames = ','.join([os.path.basename(ms.name).strip('.ms') for ms in mses])
                        LOG.warn('%s field %s spwid %d: has a %s%s' % (msnames, fieldname, spwid, ' and a '.join(warnings), snr_msg))

                    result.check_source_fit = {'offset': offset, 'offset_err': offset_err, 'beams': beams, 'beams_err': beams_err, 'fitflux': fitflux, 'fitflux_err': fitflux_err, 'fitpeak': fitpeak, 'gfluxscale': gfluxscale, 'gfluxscale_err': gfluxscale_err}
                except Exception as e:
                    result.check_source_fit = {'offset': 'N/A', 'offset_err': 'N/A', 'beams': 'N/A', 'beams_err': 'N/A', 'fitflux': 'N/A', 'fitflux_err': 'N/A', 'fitpeak': 'N/A', 'gfluxscale': 'N/A', 'gfluxscale_err': 'N/A'}


class TcleanListQAHandler(pqa.QAPlugin):
    result_cls = collections.Iterable
    child_cls = resultobjects.TcleanResult

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
