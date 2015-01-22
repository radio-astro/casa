from __future__ import absolute_import
import pipeline.qa.scorecalculator as qacalc
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import baselineflag

LOG = logging.get_logger(__name__)


class SDBLFlagQAHandler(pqa.QAResultHandler):    
    result_cls = baselineflag.SDBLFlagResults
    child_cls = None

    def handle(self, context, result):
        #vis = result.inputs['vis']
        #ms = context.observing_run.get_ms(vis)
        summaries = result.outcome['summary']
        scores = []
        for summary in summaries:
            name = summary['name']
            ant = summary['antenna']
            spw = summary['spw']
            pol = summary['pol']
            frac_flagged = summary['nflags'][0] / float(summary['nrow'])
            scores.append(qacalc.score_sdtotal_data_flagged(name, ant, spw, pol, frac_flagged))
#         frac_flagged = result.rows_flagged / result.num_rows
#         frac_flagged = 0.25

#         scores = [qacalc.score_sdtotal_data_flagged(name, frac_flagged)]
        result.qa.pool[:] = scores


class SDBLFlagListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = baselineflag.SDBLFlagResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
