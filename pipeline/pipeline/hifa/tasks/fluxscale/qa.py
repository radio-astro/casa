from __future__ import absolute_import

import collections

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc
from pipeline.h.tasks.common import commonfluxresults
from . import gcorfluxscale

LOG = logging.get_logger(__name__)


class GcorFluxscaleQAHandler(pqa.QAPlugin):
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
        score2 = self._low_snr_fluxes(ms, result.measurements)
        scores = [score1, score2]
            
        result.qa.pool.extend(scores)
    
    def _missing_derived_fluxes(self, ms, field, intent, measurements):
        '''
        Check whether there are missing derived fluxes. 
        '''
        return qacalc.score_missing_derived_fluxes(ms, field, intent, measurements)

    def _low_snr_fluxes(self, ms, measurements):
        '''
        Check whether there are low SNR derived fluxes. 
        '''
        return qacalc.score_derived_fluxes_snr(ms,  measurements)


class GcorFluxscaleListQAHandler(pqa.QAPlugin):
    """
    QA handler for a list containing FluxCalibrationResults.
    """
    result_cls = collections.Iterable
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
