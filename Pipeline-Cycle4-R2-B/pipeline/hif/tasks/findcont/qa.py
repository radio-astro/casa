from __future__ import absolute_import
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import resultobjects

LOG = logging.get_logger(__name__)


class FindContQAHandler(pqa.QAResultHandler):    
    result_cls = resultobjects.FindContResult
    child_cls = None

    def handle(self, context, result):
        score = float(result.num_found) / float(result.num_total)
        longmsg, shortmsg = ('Found continuum ranges', '') if score == 1.0 else \
            ('Found only %d of %d continuum ranges' % (result.num_found, result.num_total), 'Missing continuum ranges')
        result.qa.pool[:] = [pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)]


class FindContListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = resultobjects.FindContResult

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
