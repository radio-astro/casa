from __future__ import absolute_import

import collections

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
from . import resultobjects

LOG = logging.get_logger(__name__)


class MakeImListQAHandler(pqa.QAPlugin):
    result_cls = resultobjects.MakeImListResult
    child_cls = None

    def handle(self, context, result):
        # calculate QA score comparing number of targets against expected number
        if result.mitigation_error:
            score = 0.0
            longmsg = 'Size mitigation error. No targets were created.'
            shortmsg = 'Size mitigation error.'
        elif result.max_num_targets == 0:
            score = 1.0
            longmsg = 'No clean targets expected.'
            shortmsg = ''
        else:
            score = float(result.num_targets)/float(result.max_num_targets)
            longmsg, shortmsg = ('All clean targets defined', '') if score == 1.0 else \
                ('Expected %d clean targets but got only %d.' % \
                 (result.max_num_targets, result.num_targets), \
                 'Expected %d clean targets' % (result.max_num_targets))
        result.qa.pool[:] = [pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)]


class MakeImListListQAHandler(pqa.QAPlugin):
    result_cls = collections.Iterable
    child_cls = resultobjects.MakeImListResult

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
