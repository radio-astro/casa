from __future__ import absolute_import
import pipeline.qa.scorecalculator as qacalc
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import applycal

LOG = logging.get_logger(__name__)


class ApplycalQAHandler(pqa.QAResultHandler):    
    result_cls = applycal.ApplycalResults
    child_cls = None

    def handle(self, context, result):
        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)
    
        # calculate QA scores from agentflagger summary dictionary, adopting
        # the minimum score as the representative score for this task
        #scores = [qacalc.score_total_data_flagged(ms.basename, result.summaries)]
        
        #CAS-7059
        #for applycal, change the column label from After to Additional, 
        #  and base the QA on that number instead of the total. 
        scores = [qacalc.score_data_flagged_by_agents(ms, result.summaries, 0.05, 0.6, agents=['applycal'])]
        result.qa.pool[:] = scores


class ApplycalListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = applycal.ApplycalResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
