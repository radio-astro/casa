from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.qa.scorecalculator as qacalc

LOG = logging.get_logger(__name__)

from . import gcorfluxscale
from pipeline.hif.tasks.common import commonfluxresults


class GcorFluxscaleQAHandler(pqa.QAResultHandler):
    result_cls = commonfluxresults.FluxCalibrationResults
    child_cls = None
    generating_task = gcorfluxscale.GcorFluxscale

    def handle(self, context, result):

        vis= result.inputs['vis']
	ms = context.observing_run.get_ms(vis)

	# Check for existance of field / spw combinations for which
	# the derived fluxes are missing.
        score1 = self._missing_derived_fluxes(ms, result.inputs['transfer'],
	    result.inputs['transintent'], result.measurements)

#        self._check_flagged_calibrator_data(result.mses)
#        score3 = self._check_model_data_column(result.mses)
#        score4 = self._check_history_column(result.mses)

        scores = [score1]
        result.qa.pool.extend(scores)
    
    def _missing_derived_fluxes(self, ms, field, intent, measurements):
        '''
        Check whether there are missing derived fluxes. 
        '''
        return qacalc.score_missing_derived_fluxes(ms, field, intent, measurements)
    
class GcorFluxscaleListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing FluxCalibrationResults.
    """
    result_cls = list
    child_cls = commonfluxresults.FluxCalibrationResults
    generating_task = gcorfluxscale.GcorFluxscale

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


