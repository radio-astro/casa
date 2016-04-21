from __future__ import absolute_import

import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.qa.scorecalculator as qacalc

from . import uvcontfit

LOG = logging.get_logger(__name__)

class UVcontFitQAHandler(pqa.QAResultHandler):
    result_cls = uvcontfit.UVcontFitResults
    child_cls = None
    generating_task = uvcontfit.UVcontFit

    def handle(self, context, result):

	# Check for existance of the the target MS.
        score1 = self._uvtable_exists(result.inputs['output_dir'],
	    os.path.basename(result.inputs['caltable']))
        scores = [score1]
	    
        result.qa.pool.extend(scores)
    
    def _uvtable_exists(self, output_dir, caltable):
        '''
        Check for the existence of the target MS
        '''
        return qacalc.score_path_exists(output_dir, caltable,
	    'uv continuum fit table')

class UVcontFitListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing UVcontFitResults.
    """
    result_cls = list
    child_cls = uvcontfit.UVcontFitResults
    generating_task = uvcontfit.UVcontFit

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
