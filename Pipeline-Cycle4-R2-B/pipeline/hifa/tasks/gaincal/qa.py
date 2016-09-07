from __future__ import absolute_import
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.gpcal as gpcal

from pipeline.hif.tasks.gaincal import common

LOG = logging.get_logger(__name__)

class TimegaincalQAPool(pqa.QAScorePool):
    score_types = {'PHASE_SCORE_XY'   : ('XY_TOTAL', 'X-Y phase deviation'),
                   'PHASE_SCORE_X2X1' : ('X2X1_TOTAL', 'X2-X1 phase deviation')}

    short_msg = {'PHASE_SCORE_XY'   : 'X-Y deviation',
                 'PHASE_SCORE_X2X1' : 'X2-X1 deviation'}

    def __init__(self, qa_results_dict):
        super(TimegaincalQAPool, self).__init__()
        self.qa_results_dict = qa_results_dict

    def update_scores(self, ms, phase_field_ids):

        self.pool[:] = [self._get_qascore(ms, phase_field_ids, t) for t in self.score_types.iterkeys()]

    def _get_qascore(self, ms, phase_field_ids, score_type):
        (total_score, table_name, field_name, ant_name, spw_name) = self._get_total(phase_field_ids, self.score_types[score_type][0])
        longmsg = 'Total score for %s is %0.2f (%s field %s %s spw %s)' % (self.score_types[score_type][1], total_score, ms.basename, field_name, ant_name, spw_name)
        shortmsg = self.short_msg[score_type]
        return pqa.QAScore(total_score, longmsg=longmsg, shortmsg=shortmsg, 
                           vis=ms.basename)

    def _get_total(self, phase_field_ids, score_key):

        # attrs to hold score and QA identifiers
        total_score = 1.0
        total_table_name = None
        total_field_name = 'N/A'
        total_ant_name = 'N/A'
        total_spw_name = 'N/A'

        for table_name in self.qa_results_dict.iterkeys():
            qa_result = self.qa_results_dict[table_name]
            for field_id in phase_field_ids:
                qa_context_score = qa_result['QASCORES']['SCORES'][field_id][score_key]
                if (qa_context_score['SCORE'] != 'C/C'):
                    if (qa_context_score['SCORE'] < total_score):
                        total_score = qa_context_score['SCORE']
                        total_field_name = qa_result['QASCORES']['FIELDS'][qa_context_score['FIELD']]
                        total_ant_name = qa_result['QASCORES']['ANTENNAS'][qa_context_score['ANTENNA']]
                        total_spw_name = qa_result['QASCORES']['SPWS'][qa_context_score['SPW']]
                        total_table_name = table_name

        return (total_score, total_table_name, total_field_name, total_ant_name, total_spw_name)


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
                    qa_results_dict[calapp.gaintable]['PHASE_FIELDS'] = phase_field_ids

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
