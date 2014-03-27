from __future__ import absolute_import
import pipeline.qa.scorecalculator as qacalc
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import agentflagger

LOG = logging.get_logger(__name__)


class AgentFlaggerQAHandler(pqa.QAResultHandler):    
    result_cls = agentflagger.AgentFlaggerResults
    child_cls = None

    def handle(self, context, result):
        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)
    
        # calculate QA scores from agentflagger summary dictionary, adopting
        # the minimum score as the representative score for this task
        scores = [qacalc.score_online_shadow_agents(ms, result.summaries),
                  qacalc.score_total_data_flagged(ms.basename, result.summaries)]
        result.qa.pool[:] = scores


class AgentFlaggerListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = agentflagger.AgentFlaggerResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
