from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.qa.scorecalculator as qacalc

LOG = logging.get_logger(__name__)

from . import spwphaseup

class SpwPhaseupQAHandler(pqa.QAResultHandler):
    result_cls = spwphaseup.SpwPhaseupResults
    child_cls = None
    generating_task = spwphaseup.SpwPhaseup

    def handle(self, context, result):
        vis= result.inputs['vis']
        ms = context.observing_run.get_ms(vis)

        # Check first for the combined spw map. Next the
        # narrow to wide spw mzp. Note that the input
        # field and intent parameters are no longer used.
        if result.combine_spwmap:
            score1 = self._phaseup_mapping_fraction(ms, '', 'PHASE',
                result.combine_spwmap)
        else:
            score1 = self._phaseup_mapping_fraction(ms, result.inputs['field'],
                result.inputs['intent'], result.phaseup_spwmap)
	if not result.phaseup_result.final:
	    score2= qacalc.score_path_exists(ms.name,
	    list(result.phaseup_result.error)[0].gaintable, 'caltable')
	else:
	    score2= qacalc.score_path_exists(ms.name,
	        result.phaseup_result.final[0].gaintable, 'caltable')
        scores = [score1, score2]
            
        result.qa.pool.extend(scores)
    
    def _phaseup_mapping_fraction(self, ms, field, intent, phaseup_spwmap):
        '''
        Check whether or not there has been spw phaseup mapping . 
        '''
        return qacalc.score_phaseup_mapping_fraction(ms, field, intent, phaseup_spwmap)


class SpwPhaseupListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing SpwPhaseupResults.
    """
    result_cls = list
    child_cls = spwphaseup.SpwPhaseupResults
    generating_task = spwphaseup.SpwPhaseup

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated

        mses = [r.inputs['vis'] for r in result]
        longmsg = 'No mapped narrow spws in %s' % utils.commafy(mses,
                                                                quotes=False,
                                                                conjunction='or')
        result.qa.all_unity_longmsg = longmsg

