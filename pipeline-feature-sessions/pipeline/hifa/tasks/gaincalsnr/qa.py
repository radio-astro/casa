from __future__ import absolute_import

#import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.qa.scorecalculator as qacalc

from . import gaincalsnr

#LOG = logging.get_logger(__name__)


class GaincalSnrQAHandler(pqa.QAResultHandler):
    result_cls = gaincalsnr.GaincalSnrResults
    child_cls = None
    generating_task = gaincalsnr.GaincalSnr

    def handle(self, context, result):
        vis= result.inputs['vis']
        phasesnr = result.inputs['phasesnr']
        ms = context.observing_run.get_ms(vis)

        # Check for existance of spws combinations for which
        # SNR estimates are missing. ms argument not really
        # needed for this but include for the moment.
        score1 = self._missing_phase_snrs(ms, result.spwids,
            result.snrs)
        score2 = self._poor_phase_snrs(ms, result.spwids,
            phasesnr, result.snrs)
        scores = [score1, score2]
            
        result.qa.pool.extend(scores)
    
    def _missing_phase_snrs(self, ms, spwids, snrs):
        '''
        Check whether there are missing phase snrs. 
        '''
        return qacalc.score_missing_phase_snrs(ms, spwids, snrs)

    def _poor_phase_snrs(self, ms, spwids, phasesnr, snrs):
        '''
        Check whether there are poor snrs values
        '''
        return qacalc.score_poor_phase_snrs(ms, spwids, phasesnr,
            snrs)

    
class GaincalSnrListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing GaincalSnrResults.
    """
    result_cls = list
    child_cls = gaincalsnr.GaincalSnrResults
    generating_task = gaincalsnr.GaincalSnr

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated

        mses = [r.inputs['vis'] for r in result]
        longmsg = 'No missing derived fluxes in %s' % utils.commafy(mses,
                                                                    quotes=False,
                                                                    conjunction='or')
        result.qa.all_unity_longmsg = longmsg

