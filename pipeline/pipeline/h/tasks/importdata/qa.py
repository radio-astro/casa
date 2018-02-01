from __future__ import absolute_import

import collections
import datetime
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc
from pipeline.infrastructure import casa_tasks, casatools
from . import importdata
from ..exportdata import aqua

LOG = logging.get_logger(__name__)



class ImportDataQAHandler(pqa.QAPlugin):
    result_cls = importdata.ImportDataResults
    child_cls = None

    def handle(self, context, result):
        score1 = self._check_intents(result.mses)
        new_origin = pqa.QAOrigin(metric_name='MissingIntentsMark',
                                  metric_score=score1.origin.metric_score,
                                  metric_units='Measure of missing scan intents')
        score1.origin = new_origin

        self._check_flagged_calibrator_data(result.mses)
        score3 = self._check_model_data_column(result.mses)
        score4 = self._check_history_column(result.mses, result.inputs)
        
        LOG.todo('How long can MSes be separated and still be considered ' 
                 'contiguous?')
        score5 = self._check_contiguous(result.mses)

        score6 = self._check_coordinates(result.mses)
    
        LOG.todo('ImportData QA: missing source.xml and calibrator unknown to ' 
                 'CASA')
        LOG.todo('ImportData QA: missing BDFs')

        scores = [score1, score3, score4, score5, score6]
        result.qa.pool.extend(scores)
    
    def _check_contiguous(self, mses):
        """
        Check whether observations are contiguous. 
        """
        tolerance = datetime.timedelta(hours=1)
        return qacalc.score_contiguous_session(mses, tolerance=tolerance)
    
    def _check_model_data_column(self, mses):
        """
        Check whether any of the measurement sets contain a MODEL_DATA column,
        complaining if present, and returning an appropriate QA score.
        """
        bad_mses = []
    
        for ms in mses:
            with casatools.TableReader(ms.name) as table:
                if 'MODEL_DATA' in table.colnames():
                    bad_mses.append(ms)
    
        return qacalc.score_ms_model_data_column_present(mses, bad_mses)
    
    def _check_history_column(self, mses, inputs):
        """
        Check whether any of the measurement sets has entries in the history
        column, potentially signifying a non-pristine data set.
        """
        bad_mses = []
        
        createmms = False if not inputs.has_key('createmms') else inputs['createmms']
    
        for ms in mses:
            history_table = os.path.join(ms.name, 'HISTORY')
            with casatools.TableReader(history_table) as table:
                if table.nrows() != 0:
                    origin_col = table.getcol('ORIGIN')
                    if createmms:
                        # special treatment is needed when createmms mode is turned on
                        for i in range(len(origin_col)):
                            if origin_col[i] == 'importasdm' or origin_col[i] == 'partition' or origin_col[i] == 'im::calcuvw()':
                                continue
                            bad_mses.append(ms)
                            break
                    else:
                        for i in range(len(origin_col)):
                            if origin_col[i] == 'importasdm' or origin_col[i] == 'im::calcuvw()':
                                continue
                            bad_mses.append(ms)
                            break
    
        return qacalc.score_ms_history_entries_present(mses, bad_mses)
    
    def _check_flagged_calibrator_data(self, mses):
        """
        Check how much calibrator data has been flagged in the given measurement
        sets, complaining if the fraction of flagged data exceeds a threshold. 
        """
        LOG.todo('What fraction of flagged calibrator data should result in a warning?')
        threshold = 0.10
    
        # which intents should be checked for flagged data
        calibrator_intents = {'AMPLITUDE', 'BANDPASS', 'PHASE'}
        
        # flag for whether to print 'all scans ok' message at end
        all_ok = True
        
        for ms in mses:
            bad_scans = collections.defaultdict(list)
            
            # inspect each MS with flagdata, capturing the dictionary 
            # describing the number of flagged rows 
            flagdata_task = casa_tasks.flagdata(vis=ms.name, mode='summary')
            flagdata_result = flagdata_task.execute(dry_run=False)
    
            for intent in calibrator_intents:
                # collect scans with the calibrator intent
                calscans = [scan for scan in ms.scans if intent in scan.intents]
                for scan in calscans:
                    scan_id = str(scan.id)
                    # read the number of flagged/total integrations from the
                    # flagdata results dictionary
                    flagged = flagdata_result['scan'][scan_id]['flagged']
                    total = flagdata_result['scan'][scan_id]['total']
                    if flagged / total >= threshold:
                        bad_scans[intent].append(scan)
                        all_ok = False
    
            for intent in bad_scans:
                scan_ids = [scan.id for scan in bad_scans[intent]]
                multi = False if len(scan_ids) is 1 else True
                # log something like 'More than 12% of PHASE scans 1, 2, and 7 
                # in vla.ms are flagged'
                LOG.warning('More than %s%% of %s scan%s %s in %s %s flagged'
                            '' % (threshold * 100.0,
                                  intent,
                                  's' if multi else '',
                                  utils.commafy(scan_ids, quotes=False),
                                  ms.basename,
                                  'are' if multi else 'is'))
    
        if all_ok:
            LOG.info('All %s scans in %s have less than %s%% flagged '
                     'data' % (utils.commafy(calibrator_intents, False),
                               utils.commafy([ms.basename for ms in mses]),
                               threshold * 100.0))
    
    def _check_intents(self, mses):
        """
        Check each measurement set in the list for a set of required intents.
        
        TODO Should we terminate execution on missing intents?        
        """
        return qacalc.score_missing_intents(mses)

    def _check_coordinates(self, mses):
        """
        Check each measurement set in the list for zero valued coordinates.
        
        """
        return qacalc.score_ephemeris_coordinates(mses)


class ImportDataListQAHandler(pqa.QAPlugin):
    result_cls = collections.Iterable
    child_cls = importdata.ImportDataResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool[:] for r in result]) 
        result.qa.pool.extend(collated)


aqua_exporter = aqua.xml_generator_for_metric('MissingIntentsMark', '{:0.3f}')
aqua.register_aqua_metric(aqua_exporter)
