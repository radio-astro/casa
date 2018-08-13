from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
from . import imageprecheck

LOG = logging.get_logger(__name__)


class ImagePreCheckQAHandler(pqa.QAPlugin):    
    result_cls = imageprecheck.ImagePreCheckResults
    child_cls = None

    def handle(self, context, result):
        if result.score is not None:
            score, longmsg, shortmsg = result.score
        else:
            score = 0.0
            longmsg = 'No score information'
            shortmsg = 'No score information'
        result.qa.pool[:] = [pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)]


class ImagePreCheckListQAHandler(pqa.QAPlugin):
    result_cls = basetask.ResultsList
    child_cls = imageprecheck.ImagePreCheckResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
