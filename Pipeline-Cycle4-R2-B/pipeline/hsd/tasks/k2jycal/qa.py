from __future__ import absolute_import
import pipeline.qa.scorecalculator as qacalc
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import k2jycal

LOG = logging.get_logger(__name__)

class SDK2JyCalQAHandler(pqa.QAResultHandler):
    result_cls = k2jycal.SDK2JyCalResults
    child_cls = None

    def handle(self, context, result):
        is_missing_factor = (not result.all_ok)
        
        shortmsg = "Missing Jy/K factors for some data." if is_missing_factor else "Jy/K factor is found for all data"
        longmsg = "Missing Jy/K factors for some data. Those data will remain in the unit of Kelvin after applying the calibration tables." if is_missing_factor else shortmsg
        score = 0.0 if is_missing_factor else 1.0
        scores = [ pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg) ]
        result.qa.pool.extend(scores)
        #result.qa.pool[:] = scores

class SDK2JyCalListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = k2jycal.SDK2JyCalResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
