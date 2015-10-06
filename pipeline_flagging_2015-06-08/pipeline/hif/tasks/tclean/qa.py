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
        # calculate QA score comparing RMS against clean threshold and/or sensitivity
        imageScorer = scorers.erfScorer(1.0, 5.0)
        qaTool = casac.quanta()
        try:
            score = imageScorer(result.rms / qaTool.convert(result.threshold, 'Jy')['value'])
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
