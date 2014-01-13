from __future__ import absolute_import
import pipeline.qa2.scorecalculator as qa2calc
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import flagdata

LOG = logging.get_logger(__name__)


class SDFlagDataQAHandler(pqa.QAResultHandler):    
    result_cls = flagdata.SDFlagDataResults
    child_cls = None

    def handle(self, context, result):
        vis = result.inputs['vis']
        #ms = context.observing_run.get_ms(vis)
        
#         frac_flagged = result.rows_flagged / result.num_rows
        frac_flagged = 0.25
        
        scores = [qa2calc.score_sdtotal_data_flagged(vis, frac_flagged)]
        result.qa.pool[:] = scores


class SDFlagDataListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = flagdata.SDFlagDataResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QA2score list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
