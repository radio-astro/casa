from __future__ import absolute_import
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import imageprecheck

LOG = logging.get_logger(__name__)


class ImagePreCheckQAHandler(pqa.QAResultHandler):    
    result_cls = imageprecheck.ImagePreCheckResults
    child_cls = None

    def handle(self, context, result):
        if result.real_repr_target:
            score = 1.0
            longmsg = 'Representative target found. Applied robust and uvtaper heuristics.'
            shortmsg = 'robust and uvtaper computed.'
        else:
            score = 0.5
            longmsg = 'No representative target found. Picking first in list.'
            shortmsg = 'No representative target'
        result.qa.pool[:] = [pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)]


class ImagePreCheckListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = imageprecheck.ImagePreCheckResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
