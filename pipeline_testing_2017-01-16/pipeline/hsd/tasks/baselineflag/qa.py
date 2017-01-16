from __future__ import absolute_import
import pipeline.qa.scorecalculator as qacalc
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa

from . import baselineflag
from .renderer import accumulate_flag_per_source_spw

LOG = logging.get_logger(__name__)

class SDBLFlagListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = baselineflag.SDBLFlagResults

    def handle(self, context, result):
        # Accumulate flag per field, spw to a dictionary
        # accum_flag[field][spw] = {'additional': # of flagged in task, 'total': # of total}
        accum_flag = accumulate_flag_per_source_spw(result)
        # Now define score per field, spw
        scores = []
        for field, spwflag in accum_flag.items():
            for spw, flagval in spwflag.items():
                frac_flagged = flagval['additional']/ float(flagval['total'])
                label = ("Field %s Spw %s" % (field, spw))
                scores.append(qacalc.score_sdtotal_data_flagged(label, frac_flagged))

        result.qa.pool[:] = scores
