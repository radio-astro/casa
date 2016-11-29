from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc
from . import flagdeterbase

LOG = logging.get_logger(__name__)


class FlagDeterBaseQAHandler(pqa.QAResultHandler):
    result_cls = flagdeterbase.FlagDeterBaseResults
    child_cls = None

    def handle(self, context, result):
        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)
    
        # CAS-7059 base the metric (and warnings) on Shadowing+Online, instead
        # of on the Total.
        score = qacalc.score_online_shadow_template_agents(ms, result.summaries)
        new_origin = pqa.QAOrigin(metric_name='%OnlineShadowTemplateFlags',
                                  metric_score=score.origin.metric_score,
                                  metric_units=score.origin.metric_units)
        score.origin = new_origin
                  
        result.qa.pool[:] = [score]


class FlagDeterBaseListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = flagdeterbase.FlagDeterBaseResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
