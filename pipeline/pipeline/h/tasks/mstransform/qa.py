from __future__ import absolute_import

import os

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.qa.scorecalculator as qacalc

from . import mssplit

LOG = logging.get_logger(__name__)


class MsSplitQAHandler(pqa.QAResultHandler):
    result_cls = mssplit.MsSplitResults
    child_cls = None
    generating_task = mssplit.MsSplit

    def handle(self, context, result):

	# Check for existance of the the target MS.
        score1 = self._targetms_exists(os.path.dirname(result.outputvis),
	    os.path.basename(result.outputvis))
        scores = [score1]
	    
        result.qa.pool.extend(scores)
    
    def _targetms_exists(self, output_dir, target_ms):
        '''
        Check for the existence of the target MS
        '''
        return qacalc.score_path_exists(output_dir, target_ms,
	    'science target ms')

class MsSplitListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing MsSplitResults.
    """
    result_cls = basetask.ResultsList
    child_cls = mssplit.MsSplitResults
    generating_task = mssplit.MsSplit

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated
        mses = [r.inputs['vis'] for r in result]
        longmsg = 'No missing target MS(s) for %s' % utils.commafy(mses,
                                                                   quotes=False,
                                                                   conjunction='or')
	result.qa.all_unity_longmsg = longmsg
