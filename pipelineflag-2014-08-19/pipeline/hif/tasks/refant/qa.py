from __future__ import absolute_import
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import referenceantenna

LOG = logging.get_logger(__name__)


class RefantQAHandler(pqa.QAResultHandler):    
    result_cls = referenceantenna.RefAntResults
    child_cls = None

    def handle(self, context, result):
        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)
    
        if result._refant != '':
            scores = [pqa.QAScore(1.0,
                                  longmsg='Reference antenna for %s was selected successfully' % ms.basename,
                                  shortmsg='Refant OK')]
        else:
            scores = [pqa.QAScore(0.0,
                                  longmsg='Could not select reference antenna for %s' % ms.basename,
                                  shortmsg='No refant')]

        result.qa.pool[:] = scores


class RefantListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = referenceantenna.RefAntResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
