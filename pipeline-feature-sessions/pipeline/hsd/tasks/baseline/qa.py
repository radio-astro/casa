from __future__ import absolute_import
import pipeline.qa.scorecalculator as qacalc
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import msbaseline

LOG = logging.get_logger(__name__)


class SDBaselineQAHandler(pqa.QAResultHandler):    
    result_cls = msbaseline.SDMSBaselineResults
    child_cls = None

    def handle(self, context, result):
        scores = []
        lines_list = []
        group_id_list = []
        spw_id_list = []
        field_id_list = []
        baselined = result.outcome['baselined']
        for b in baselined:
            reduction_group_id = b['group_id']
            spw_id = b['spw']
            field_id = b['field']
            lines = b['lines']
            lines_list.append(lines)
            group_id_list.append(reduction_group_id)
            spw_id_list.append(spw_id)
            field_id_list.append(field_id)
        scores.append(qacalc.score_sd_line_detection_for_ms(group_id_list, 
                                                            field_id_list, 
                                                            spw_id_list, 
                                                            lines_list))
        result.qa.pool.extend(scores)


class SDBaselineListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = msbaseline.SDMSBaselineResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
