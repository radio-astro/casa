from __future__ import absolute_import
import collections
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.qa.scorecalculator as qacalc

from .semiFinalBPdcals import semiFinalBPdcalsResults
from .semiFinalBPdcals import semiFinalBPdcals

LOG = logging.get_logger(__name__)

class semiFinalBPdcalsQAHandler(pqa.QAResultHandler):
    result_cls = semiFinalBPdcalsResults
    child_cls = None
    generating_task = semiFinalBPdcals

    def handle(self, context, result):

        # Check for existence of the the target MS.
        score1 = self._ms_exists(os.path.dirname(result.inputs['vis']), os.path.basename(result.inputs['vis']))
        scores = [score1]

        result.qa.pool.extend(scores)

    def _ms_exists(self, output_dir, ms):
        '''
        Check for the existence of the target MS
        '''
        return qacalc.score_path_exists(output_dir, ms, 'semiFinalBPdcals')

class semiFinalBPdcalsListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing semiFinalBPdcalsResults.
    """
    result_cls = collections.Iterable
    child_cls = semiFinalBPdcalsResults
    generating_task = semiFinalBPdcals

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated
        mses = [r.inputs['vis'] for r in result]
        longmsg = 'No missing target MS(s) for %s' % utils.commafy(mses, quotes=False, conjunction='or')
        result.qa.all_unity_longmsg = longmsg
