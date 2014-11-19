from __future__ import absolute_import
import pipeline.qa.scorecalculator as qacalc
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import simplescale

LOG = logging.get_logger(__name__)


class SDSimpleScaleQAHandler(pqa.QAResultHandler):    
    result_cls = simplescale.SDSimpleScaleResults
    child_cls = None

    def handle(self, context, result):
        #vis = result.inputs['vis']
        #ms = context.observing_run.get_ms(vis)
        ok = result.outcome['success']
        ok_message = "All scantables are scaled"
        ng_message = "Some of scantables are not scaled"
        scores = [pqa.QAScore(1.0, longmsg=ok_message, shortmsg=ok_message)] if ok \
            else [pqa.QAScore(1.0, longmsg=ng_message, shortmsg=ng_message)]
        result.qa.pool[:] = scores

class SDSimpleScaleListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = simplescale.SDSimpleScaleResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
