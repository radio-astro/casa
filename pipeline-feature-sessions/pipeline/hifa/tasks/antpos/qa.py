from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.qa.scorecalculator as qacalc

from . import almaantpos
from pipeline.hif.tasks.antpos import antpos

LOG = logging.get_logger(__name__)


class ALMAAntposQAHandler(pqa.QAResultHandler):
    result_cls = antpos.AntposResults
    child_cls = None
    generating_task = almaantpos.ALMAAntpos

    def handle(self, context, result):

        vis= result.inputs['vis']
        ms = context.observing_run.get_ms(vis)

        # Check for existance of field / spw combinations for which
        # the derived fluxes are missing.
        score1 = self._number_antenna_offsets(ms, result.antenna,
                result.offsets)
        scores = [score1]
            
        result.qa.pool.extend(scores)
    
    def _number_antenna_offsets(self, ms, antenna, offsets):
        '''
        Check whether there are antenna position corrections
        '''
        return qacalc.score_number_antenna_offsets(ms, antenna, offsets)


class ALMAAntposListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing AntResults.
    """
    result_cls = list
    child_cls = antpos.AntposResults
    generating_task = almaantpos.ALMAAntpos

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated

        mses = [r.inputs['vis'] for r in result]
        longmsg = 'No antenna position corrections in %s' % utils.commafy(mses,
                                                                    quotes=False,
                                                                    conjunction='or')
        result.qa.all_unity_longmsg = longmsg
