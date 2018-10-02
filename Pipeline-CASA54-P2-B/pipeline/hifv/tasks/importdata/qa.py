from __future__ import absolute_import

import collections

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.qa.scorecalculator as qacalc
import pipeline.infrastructure.utils as utils
from pipeline.h.tasks.importdata import qa as hqa
from . import importdata

LOG = logging.get_logger(__name__)


class VLAImportDataQAHandler(hqa.ImportDataQAHandler, pqa.QAPlugin):
    result_cls = importdata.VLAImportDataResults
    child_cls = None
    generating_task = importdata.VLAImportData

    def handle(self, context, result):
        score1 = self._check_intents(result.mses)
        score2 = self._check_history_column(result.mses, result.inputs)

        scores = [score1, score2]
        result.qa.pool.extend(scores)

    @staticmethod
    def _check_history_column(mses, inputs):
        """
        Check whether any of the measurement sets has entries in the history
        column, potentially signifying a non-pristine data set.
        """
        qascore = hqa.ImportDataQAHandler._check_history_column(mses, inputs)
        if qascore.score < 1.0:
            qascore.longmsg += " If hanning smooth has already been applied in previous execution, invoking hiv_hanning in a following stage means smoothing twice."
        return qascore

    def _check_intents(self, mses):
        """
        Check each measurement set in the list for a set of required intents.
        
        TODO Should we terminate execution on missing intents?        
        """
        return qacalc.score_missing_intents(mses, array_type='VLA')
    

class VLAImportDataListQAHandler(pqa.QAPlugin):
    result_cls = collections.Iterable
    child_cls = importdata.VLAImportDataResults
 
    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool[:] for r in result])
        result.qa.pool.extend(collated)
