from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc

from . import fluxcalflag

LOG = logging.get_logger(__name__)


class FluxcalflagQAHandler(pqa.QAResultHandler):    
    """
    QA handler for an uncontained FluxcalflagResult.
    """
    result_cls = fluxcalflag.FluxcalFlagResults
    child_cls = None

    def handle(self, context, result):
        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)
    
        # calculate QA scores from agentflagger summary dictionary, adopting
        # the minimum score as the representative score for this task
        scores = [qacalc.score_fraction_newly_flagged(ms.basename,
                                                      result.summaries,
                                                      ms.basename)]
        result.qa.pool[:] = scores

        result.qa.all_unity_longmsg = 'No extra data was flagged in %s' % ms.basename


class FluxcalflagListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing FluxcalflagResults.
    """
    result_cls = list
    child_cls = fluxcalflag.FluxcalFlagResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated

        mses = [r.inputs['vis'] for r in result]
        longmsg = 'No extra data was flagged in %s' % utils.commafy(mses, 
                                                                    quotes=False, 
                                                                    conjunction='or')
        result.qa.all_unity_longmsg = longmsg