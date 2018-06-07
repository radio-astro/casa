from __future__ import absolute_import

import collections

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc
from pipeline.hif.tasks.antpos import antpos
from . import almaantpos

LOG = logging.get_logger(__name__)


class ALMAAntposQAHandler(pqa.QAPlugin):
    result_cls = antpos.AntposResults
    child_cls = None
    generating_task = almaantpos.ALMAAntpos

    def handle(self, context, result):

        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)

        # Check for existence of field / spw combinations for which
        # the derived fluxes are missing.
        score1 = self._number_antenna_offsets(ms, result.antenna, result.offsets)
        scores = [score1]
            
        result.qa.pool.extend(scores)
    
    @staticmethod
    def _number_antenna_offsets(ms, antenna, offsets):
        """
        Check whether there are antenna position corrections
        """
        return qacalc.score_number_antenna_offsets(ms, antenna, offsets)


class ALMAAntposListQAHandler(pqa.QAPlugin):
    """
    QA handler for a list containing AntResults.
    """
    result_cls = collections.Iterable
    child_cls = antpos.AntposResults
    generating_task = almaantpos.ALMAAntpos

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated

        mses = [r.inputs['vis'] for r in result]
        longmsg = 'No antenna position corrections in %s' % utils.commafy(mses, quotes=False, conjunction='or')
        result.qa.all_unity_longmsg = longmsg
