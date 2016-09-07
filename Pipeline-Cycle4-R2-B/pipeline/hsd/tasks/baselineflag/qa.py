from __future__ import absolute_import
import pipeline.qa.scorecalculator as qacalc
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import baselineflag

LOG = logging.get_logger(__name__)

class SDBLFlagListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = baselineflag.SDBLFlagResults

    def handle(self, context, result):
        # Accumulate flag per field, spw to a dictionary
        # accum_flag[field][spw] = {'flagged': # of flagged, 'total': # of total}
        accum_flag = {}
        for r in result:
            before, after = r.outcome['flagdata_summary']
            if not before['name']=='before' or not after['name']=='after':
                raise RuntimeError, "Got unexpected flag summary"
            for field, fieldflag in after.items():
                if not isinstance(fieldflag, dict) or not fieldflag.has_key('spw'):
                    continue
                if not accum_flag.has_key(field):
                    accum_flag[field] = {}
                spwflag = fieldflag['spw']
                for spw, flagval in spwflag.items():
                    if not accum_flag[field].has_key(spw):
                        accum_flag[field][spw] = dict(flagged=0, total=0)
                    # sum up incremental flags
                    accum_flag[field][spw]['flagged'] += (flagval['flagged']-before[field]['spw'][spw]['flagged'])
                    accum_flag[field][spw]['total'] += flagval['total']
        # Now define score per field, spw
        scores = []
        for field, spwflag in accum_flag.items():
            for spw, flagval in spwflag.items():
                frac_flagged = flagval['flagged']/ float(flagval['total'])
                label = ("Field %s Spw %s" % (field, spw))
                scores.append(qacalc.score_sdtotal_data_flagged(label, frac_flagged))

        result.qa.pool[:] = scores

### for old QA metric ###
class SDBLFlagQAHandlerOld(pqa.QAResultHandler):    
    result_cls = baselineflag.SDBLFlagResults
    child_cls = None

    def handle(self, context, result):
        summaries = result.outcome['summary']
        scores = []
        for summary in summaries:
            name = summary['name']
            ant = summary['antenna']
            spw = summary['spw']
            pol = summary['pol']
            field = summary['field']
            frac_flagged = summary['nflags'][0] / float(summary['nrow'])
            scores.append(qacalc.score_sdtotal_data_flagged_old(name, ant, spw, pol, frac_flagged, field))
        result.qa.pool[:] = scores


class SDBLFlagListQAHandlerOld(pqa.QAResultHandler):
    result_cls = list
    child_cls = baselineflag.SDBLFlagResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
