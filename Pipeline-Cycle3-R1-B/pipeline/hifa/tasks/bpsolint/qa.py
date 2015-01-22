from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.qa.scorecalculator as qacalc

LOG = logging.get_logger(__name__)

from . import bpsolint

class BpSolintQAHandler(pqa.QAResultHandler):

    result_cls = bpsolint.BpSolintResults
    child_cls = None
    generating_task = bpsolint.BpSolint

    def handle(self, context, result):

        vis= result.inputs['vis']
	minphaseupints = result.inputs['minphaseupints']
	minbpnchans = result.inputs['minbpnchan']
	ms = context.observing_run.get_ms(vis)

	# Check for existance of spws combinations for which
	# SNR estimates are missing. ms argument not really
	# needed for this but include for the moment.
        score1 = self._missing_phaseup_snrs(ms, result.spwids,
	    result.phsolints)
	score2 = self._missing_bandpass_snrs (ms, result.spwids,
	    result.bpsolints)
        score3 = self._poor_phaseup_solutions(ms, result.spwids,
	    result.nphsolutions, minphaseupints)
        score4 = self._poor_bandpass_solutions(ms, result.spwids,
	    result.nbpsolutions, minbpnchans)
        scores = [score1, score2, score3, score4]
	    
        result.qa.pool.extend(scores)
    
    def _missing_phaseup_snrs(self, ms, spwids, phsolints):
        '''
        Check whether there are missing phaseup solints. 
        '''
        return qacalc.score_missing_phaseup_snrs(ms, spwids, phsolints)

    def _missing_bandpass_snrs(self, ms, spwids, bpsolints):
        '''
        Check whether there are missing bandpass solints. 
        '''
        return qacalc.score_missing_bandpass_snrs(ms,  spwids, bpsolints)

    def _poor_phaseup_solutions(self, ms, spwids, nphsolutions, min_nsolutions):
        '''
        Check whether there are phaseup solutiosn with fewer than the
	minimum number of solutions
        '''
        return qacalc.score_poor_phaseup_solutions(ms, spwids, nphsolutions, 
	   min_nsolutions)

    def _poor_bandpass_solutions(self, ms, spwids, nbpsolutions, min_nsolutions):
        '''
        Check whether there are bandpass solutions with fewer than the
	minimum number of solutions
        '''
        return qacalc.score_poor_bandpass_solutions(ms, spwids, nbpsolutions, 
	   min_nsolutions)

    
class BpSolintListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing BpSolintResults.
    """
    result_cls = list
    child_cls = bpsolint.BpSolintResults
    generating_task = bpsolint.BpSolint

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


