from __future__ import absolute_import
import pipeline.qa.scorecalculator as qacalc
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from ..exportdata import aqua
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
        try:
            score = qacalc.score_applycal_agents(ms, result.summaries)
            new_origin = pqa.QAOrigin(metric_name='%ApplycalFlags',
                                  metric_score=score.origin.metric_score,
                                  metric_units='Percentage newly flagged by applycal')
            score.origin = new_origin

            scores = [score]
        except:
            score = pqa.QAScore(1.0,longmsg='Flag Summary off', shortmsg='Flag Summary off')
            new_origin = pqa.QAOrigin(metric_name='%ApplycalFlags',
                                  metric_score=score.origin.metric_score,
                                  metric_units='Percentage newly flagged by applycal')
            score.origin = new_origin
            scores = [score]
        
        result.qa.pool[:] = scores


class ApplycalListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = applycal.ApplycalResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated


aqua_exporter = aqua.xml_generator_for_metric('%ApplycalFlags', '{:0.3%}')
aqua.register_aqua_metric(aqua_exporter)
