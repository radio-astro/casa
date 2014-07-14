from __future__ import absolute_import
import os
import shutil

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.gpcal as gpcal

from pipeline.hif.tasks.gaincal import common

LOG = logging.get_logger(__name__)

class TimegaincalQAPool(pqa.QAScorePool):
    score_types = {'PHASE_SCORE_XY'   : 'X-Y phase deviation',
                   'PHASE_SCORE_X2X1' : 'X2-X1 phase deviation'}

    short_msg = {'PHASE_SCORE_XY'   : 'Large X-Y deviation',
                 'PHASE_SCORE_X2X1' : 'Large X2-X1 deviation'}

    def __init__(self, qa_results_dict):
        super(TimegaincalQAPool, self).__init__()
        self.qa_results_dict = qa_results_dict

    def update_scores(self, ms, phase_field_ids):

        self.pool[:] = [self._get_qascore(ms, phase_field_ids, t) for t in self.score_types.iterkeys()]

    def _get_qascore(self, ms, phase_field_ids, score_type):
        (min_score, table_name, ant_id, spw_id) = self._get_min(phase_field_ids, score_type)
        longmsg = 'Lowest score for %s is %0.2f (%s %s spw %s)' % (self.score_types[score_type],
            min_score,
            ms.basename,
            ant_id,
            spw_id)
        shortmsg = self.short_msg[score_type]
        return pqa.QAScore(min_score, longmsg=longmsg, shortmsg=shortmsg)

    def _get_min(self, phase_field_ids, score_type):

        # attrs to hold score and QA identifiers
        min_score = 1.0
        min_table_name = None
        min_ant_id = None
        min_spw_id = None

        for table_name in self.qa_results_dict.iterkeys():
            qa_result = self.qa_results_dict[table_name]
            for field_id in phase_field_ids:
                for spw_id in qa_result['QASCORES']['SPWS'].iterkeys():
                    for ant_id in qa_result['QASCORES']['ANTENNAS'].iterkeys():
                        qa_score = qa_result['QASCORES']['SCORES'][field_id][spw_id][ant_id][score_type]
                        if (qa_score != 'C/C'):
                            if (qa_score < min_score):
                                min_score = qa_score
                                min_table_name = table_name
                                min_ant_id = qa_result['QASCORES']['ANTENNAS'][ant_id]
                                min_spw_id = qa_result['QASCORES']['SPWS'][spw_id]

        return (min_score, min_table_name, min_ant_id, min_spw_id)


class TimegaincalQAHandler(pqa.QAResultHandler):
    """
    QA handler for an uncontained TimegaincalResult.
    """
    result_cls = common.GaincalResults
    child_cls = None

    def handle(self, context, result):
        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)
        phase_field_ids = [field.id for field in ms.get_fields(intent='PHASE')]

        qa_dir = os.path.join(context.report_dir,
                              'stage%s' % result.stage_number,
                              'qa')

        if not os.path.exists(qa_dir):
            os.makedirs(qa_dir)

        qa_results_dict = {}
        try:
            for calapp in result.final:
                solint = calapp.origin.inputs['solint']
                calmode = calapp.origin.inputs['calmode']
                if ((solint == 'int') and (calmode == 'p')):
                    qa_results_dict[calapp.gaintable] = gpcal.gpcal(calapp.gaintable)

            result.qa = TimegaincalQAPool(qa_results_dict)
            result.qa.update_scores(ms, phase_field_ids)
        except Exception as e:
            LOG.error('Problem occurred running QA analysis. QA '
                      'results will not be available for this task')
            LOG.exception(e)


class TimegaincalListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing TimegaincalResults.
    """
    result_cls = list
    child_cls = common.GaincalResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated
