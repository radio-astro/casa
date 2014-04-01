from __future__ import absolute_import

import pipeline.hif.tasks.importdata.importdata as importdata
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.qa.scorecalculator as qacalc
from . import almaimportdata

LOG = logging.get_logger(__name__)


class ALMAImportDataQAHandler(pqa.QAResultHandler):
    result_cls = importdata.ImportDataResults
    child_cls = None
    generating_task = almaimportdata.ALMAImportData
    
    def handle(self, context, result):
        # replace this with results of calls to ALMA-specific functions in qacalc
        #score = pqa.QAScore(0.1, longmsg='Hello from ALMA-specific QA', shortmsg='ALMA QA') 
        #scores = [score]
	score1 = self._check_polintents(result.mses)

	scores = [score1]

        result.qa.pool.extend(scores)

    def _check_polintents(self, mses):
        '''
	Check each measurement set for polarization inents
	'''
	return qacalc.score_polintents(mses)

