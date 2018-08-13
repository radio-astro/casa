from __future__ import absolute_import

import collections

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
from . import resultobjects

LOG = logging.get_logger(__name__)


class FindContQAHandler(pqa.QAPlugin):    
    result_cls = resultobjects.FindContResult
    child_cls = None

    def handle(self, context, result):
        if result.mitigation_error:
            score = 0.0
            longmsg = 'Size mitigation error. No targets were processed.'
            shortmsg = 'Size mitigation error.'
        elif result.num_total != 0:
            score = float(result.num_found) / float(result.num_total)
            longmsg, shortmsg = ('Found continuum ranges', '') if score == 1.0 else \
                ('Found only %d of %d continuum ranges' % (result.num_found, result.num_total), 'Missing continuum ranges')
        else:
            score = 0.0
            longmsg = 'No clean targets were defined. Can not run continuum finding.'
            shortmsg = 'No clean targets defined'
        result.qa.pool[:] = [pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)]


class FindContListQAHandler(pqa.QAPlugin):
    result_cls = collections.Iterable
    child_cls = resultobjects.FindContResult

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
