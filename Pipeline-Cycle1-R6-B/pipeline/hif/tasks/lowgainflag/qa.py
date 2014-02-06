from __future__ import absolute_import
import pipeline.qa2.scorecalculator as qa2calc
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import resultobjects

LOG = logging.get_logger(__name__)


class LowgainflagQAHandler(pqa.QAResultHandler):    
    result_cls = resultobjects.LowgainflagResults
    child_cls = None

    def handle(self, context, result):
        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)
    
        # calculate QA2 scores from agentflagger summary dictionary, adopting
        # the minimum score as the representative score for this task
        scores = [qa2calc.score_fraction_newly_flagged(ms.basename, result.summaries)]
        result.qa.pool[:] = scores


class LowgainflagListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = resultobjects.LowgainflagResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QA2score list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
