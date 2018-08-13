from __future__ import absolute_import

import collections

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
from . import resultobjects

LOG = logging.get_logger(__name__)


class CheckProductSizeQAHandler(pqa.QAPlugin):    
    result_cls = resultobjects.CheckProductSizeResult
    child_cls = None

    def handle(self, context, result):
        if result.status == 'OK':
            score = 1.0
        elif result.status == 'MITIGATED':
            score = 0.5
        elif result.status == 'ERROR':
            score = 0.0
        longmsg = result.reason['longmsg']
        shortmsg = result.reason['shortmsg']
        result.qa.pool[:] = [pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)]


class CheckProductSizeListQAHandler(pqa.QAPlugin):
    result_cls = collections.Iterable
    child_cls = resultobjects.CheckProductSizeResult

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
