from __future__ import absolute_import

import pipeline.h.tasks.importdata.qa as importdataqa
import pipeline.infrastructure.logging as logging
import pipeline.qa.scorecalculator as qacalc
from pipeline.infrastructure.pipelineqa import QAPlugin
from . import importdata

LOG = logging.get_logger(__name__)


# Extending another QAHandler does not automatically register the extending
# implemention with the pipeline's QA handling system, even if the class being
# extended extends QAPlugin. Extending classes must also extend QAPlugin to be
# registered with the handler.
class SDImportDataQAHandler(importdataqa.ImportDataQAHandler, QAPlugin):
    result_cls = importdata.SDImportDataResults
    child_cls = None
    generating_task = importdata.SDImportData

    def _check_intents(self, mses):
        """
        Check each measurement set in the list for a set of required intents.
        
        TODO Should we terminate execution on missing intents?        
        """
        return qacalc.score_missing_intents(mses, array_type='ALMA_TP')


class SDImportDataListQAHandler(importdataqa.ImportDataListQAHandler, QAPlugin):
    child_cls = importdata.SDImportDataResults
