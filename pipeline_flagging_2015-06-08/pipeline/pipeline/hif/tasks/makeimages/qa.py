from __future__ import absolute_import
import pipeline.qa.scorecalculator as qacalc
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

import pipeline.qa.utility.scorers as scorers

from . import resultobjects

from casac import casac

LOG = logging.get_logger(__name__)


class MakeImagesQAHandler(pqa.QAResultHandler):    
    result_cls = resultobjects.MakeImagesResult
    child_cls = None

    def handle(self, context, result):
        # calculate QA score as minimum of all sub-scores
        if (len(result.results) > 0):
            minScore = pqa.QAScore(min([item.qa.representative.score for item in result.results]), \
                                   longmsg='Image RMS outside mask vs. threshold', shortmsg='RMS vs. threshold')
        else:
            minScore = pqa.QAScore(-0.1, longmsg='No imaging results found', shortmsg='No imaging results')
        result.qa.pool[:] = [minScore]


class MakeImagesListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = resultobjects.MakeImagesResult

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
