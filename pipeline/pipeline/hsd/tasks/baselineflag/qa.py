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
        for field, spwflag in accum_flag.iteritems():
            for spw, flagval in spwflag.iteritems():
                frac_flagged = flagval['additional']/ float(flagval['total'])
                label = ("Field %s Spw %s" % (field, spw))
                scores.append(qacalc.score_sdtotal_data_flagged(label, frac_flagged))

        result.qa.pool[:] = scores


class SDBLFlagQAHandler(pqa.QAResultHandler):
    result_cls = baselineflag.SDBLFlagResults
    child_cls = None

    def handle(self, context, result):
        # temporarily encapsulate result in a list so that we can use the same
        # QA scoring function as the aggregate ResultsList
        accum_flag = accumulate_flag_per_source_spw([result])

        vis = result.inputs['vis']

        # Now define score per field, spw
        scores = []
        for field, spwflag in accum_flag.iteritems():
            for spw, flagval in spwflag.iteritems():
                frac_flagged = flagval['additional'] / float(flagval['total'])
                label = '{!s} Field {!s} Spw {!s}'.format(vis, field, spw)
                scores.append(qacalc.score_sdtotal_data_flagged(label, frac_flagged))

        result.qa.pool[:] = scores


# from pipeline.h.tasks.exportdata import aqua
# aqua_exporter = aqua.xml_generator_for_metric('score_sdtotal_data_flagged', '{:0.3%}')
# aqua.register_aqua_metric(aqua_exporter)
