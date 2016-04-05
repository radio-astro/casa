from __future__ import absolute_import
import pipeline.qa.scorecalculator as qacalc
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import k2jycal

LOG = logging.get_logger(__name__)

class K2JyCalQAHandler(pqa.QAResultHandler):
    result_cls = k2jycal.SDK2JyCalResults
    child_cls = None

    def handle(self, context, result):
        is_missing_factor = True#result.outcome['factormissing']
        shortmsg = "Missing Jy/K factors for some data" if is_missing_factor else "Jy/K factor is applied to all data"
        longmsg = "Missing Jy/K factors for some data. Images and their units may be wrong." if is_missing_factor else shortmsg
        score = 0.0 if is_missing_factor else 1.0
        scores = [ pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg) ]
        result.qa.pool.extend(scores)
        #result.qa.pool[:] = scores

