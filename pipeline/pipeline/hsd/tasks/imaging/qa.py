from __future__ import absolute_import
import pipeline.qa.scorecalculator as qacalc
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import imaging, scaling

LOG = logging.get_logger(__name__)

class IntensityScalingQAHandler(pqa.QAResultHandler):    
    result_cls = scaling.IntensityScalingResults
    child_cls = None

    def handle(self, context, result):
        # No QA score if not applying
        if not result.outcome['must_apply']: return
        # Factors should have been applied
        is_missing_factor = result.outcome['factormissing']
        msg = "Missing Jy/K factors for some data" if is_missing_factor else "Jy/K factor is applied to all data"
        score = 0.0 if is_missing_factor else 1.0
        scores = [ pqa.QAScore(score, longmsg=msg, shortmsg=msg) ]
        result.qa.pool.extend(scores)
        #result.qa.pool[:] = scores


class IntensityScalingListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = scaling.IntensityScalingResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
