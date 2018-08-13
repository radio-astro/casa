from __future__ import absolute_import
import collections
import datetime
import os

from pipeline.infrastructure import casa_tasks, casatools
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.qa.scorecalculator as qacalc

#import pipeline.hif.tasks.importdata.qa as importdataqa
import pipeline.h.tasks.importdata.qa as importdataqa

LOG = logging.get_logger(__name__)

from . import importdata


class SDImportDataQAHandler(importdataqa.ImportDataQAHandler):
    result_cls = importdata.SDImportDataResults
    child_cls = None
    generating_task = importdata.SDImportData
    
    def _check_intents(self, mses):
        '''
        Check each measurement set in the list for a set of required intents.
        
        TODO Should we terminate execution on missing intents?        
        '''
        return qacalc.score_missing_intents(mses, array_type='ALMA_TP')


class SDImportDataListQAHandler(importdataqa.ImportDataListQAHandler):
    child_cls = importdata.SDImportDataResults
