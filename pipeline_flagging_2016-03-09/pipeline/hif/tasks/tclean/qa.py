from __future__ import absolute_import
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

import pipeline.qa.utility.scorers as scorers

from . import resultobjects

from casac import casac

LOG = logging.get_logger(__name__)


class TcleanQAHandler(pqa.QAResultHandler):    
    result_cls = resultobjects.TcleanResult
    child_cls = None

    def handle(self, context, result):
        # calculate QA score comparing RMS against clean threshold

        imageScorer = scorers.erfScorer(1.0, 5.0)

        qaTool = casac.quanta()
        try:
            if (result.intent == 'TARGET'):
                score = imageScorer(result.rms / qaTool.convert(result.threshold, 'Jy')['value'])
            # As of CASA 4.5.0 there is a discrepancy between the sensitivity and
            # the achievable RMS for calibrators (point sources ?). For C3R3
            # we disable the score until this issue is resolved.
            else:
                score = 1.0
        except:
            score = -0.1
        result.qa.pool[:] = [pqa.QAScore(score, longmsg='RMS outside mask vs. threshold', shortmsg='RMS vs. threshold')]


class TcleanListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = resultobjects.TcleanResult

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
