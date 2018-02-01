from __future__ import absolute_import

import collections

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
from . import resultobjects

LOG = logging.get_logger(__name__)


class MakeImagesQAHandler(pqa.QAPlugin):    
    result_cls = resultobjects.MakeImagesResult
    child_cls = None

    def handle(self, context, result):
        # calculate QA score as minimum of all sub-scores
        if len(result.results) > 0:
            score_objects = reduce(lambda x,y: x+y, [item.qa.pool for item in result.results])
            result.qa.pool[:] = score_objects
        else:
            if len(result.targets) == 0:
                result.qa.pool[:] = [pqa.QAScore(1.0, longmsg='No imaging targets were defined', shortmsg='Nothing to image')]
            else:
                result.qa.pool[:] = [pqa.QAScore(0.0, longmsg='No imaging results found. Expected %d.' % (len(result.targets)), shortmsg='No imaging results')]


class MakeImagesListQAHandler(pqa.QAPlugin):
    result_cls = collections.Iterable
    child_cls = resultobjects.MakeImagesResult

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
