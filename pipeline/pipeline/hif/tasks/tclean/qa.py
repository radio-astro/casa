from __future__ import absolute_import
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

import pipeline.qa.utility.scorers as scorers
import pipeline.qa.scorecalculator as scorecalc

from . import resultobjects

from casac import casac

import numpy

LOG = logging.get_logger(__name__)


class TcleanQAHandler(pqa.QAResultHandler):    
    result_cls = resultobjects.TcleanResult
    child_cls = None

    def handle(self, context, result):
        # calculate QA score comparing RMS against clean threshold

        imageScorer = scorers.erfScorer(1.0, 5.0)

        # Basic imaging score
        observatory = context.observing_run.measurement_sets[0].antenna_array.name
        # For the time being the VLA calibrator imaging is generating an error
        # due to the dynamic range limitation. Bypass the real score here.
        if ('VLA' in observatory):
            result.qa.pool[:] = [pqa.QAScore(1.0)]
        elif (result.error != ''):
            result.qa.pool[:] = [pqa.QAScore(0.0, longmsg=result.error.longmsg, shortmsg=result.error.shortmsg)]
        else:
            qaTool = casac.quanta()
            try:
                # The threshold applies to peaks in the residual. To compare to the
                # measured RMS, one needs to translate peak to RMS by about a
                # factor 4.0.
                rms_score = imageScorer(result.image_rms / qaTool.convert(result.threshold, 'Jy')['value'] * 4.0)
            except Exception as e:
                LOG.warning('Exception scoring imaging result by RMS: %s. Setting score to -0.1.' % (e))
                rms_score = -0.1
            if (numpy.isnan(rms_score)):
                result.qa.pool[:] = [pqa.QAScore(0.0, longmsg='Cleaning diverged. Field: %s SPW: %s' % (result.inputs['field'], result.inputs['spw']), shortmsg='Cleaning diverged')]
            else:
                result.qa.pool[:] = [pqa.QAScore(rms_score, longmsg='RMS outside mask vs. threshold. Field: %s SPW: %s' % (result.inputs['field'], result.inputs['spw']), shortmsg='RMS vs. threshold')]

            # Check source score
            #    Be careful about the source name vs field name issue
            if result.intent == 'CHECK' and result.inputs['specmode'] == 'mfs':
                mses = [context.observing_run.get_ms(name=vis) for vis in result.inputs['vis']]
                fieldname = result.sourcename 
                spwid = int(result.spw)
                imagename = result.image 
                checkscore = scorecalc.score_checksources (mses, fieldname, spwid, imagename) 
                result.qa.pool.append (checkscore)


class TcleanListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = resultobjects.TcleanResult

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
