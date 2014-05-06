from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.qa.scorecalculator as qacalc

from . import setjy
from ..common import commonfluxresults

LOG = logging.get_logger(__name__)


class SetjyQAHandler(pqa.QAResultHandler):
    result_cls = commonfluxresults.FluxCalibrationResults
    child_cls = None
    generating_task = setjy.Setjy
    
    def handle(self, context, result):

	vis= result.inputs['vis']
	ms = context.observing_run.get_ms(vis)

	# Check for the existence of the expected flux measurements
	# and assign a score based on the fraction of actual to 
	# expected ones.
	scores = [qacalc.score_setjy_measurements(ms, result.measurements)]
	result.qa.pool[:] = scores
	result.qa.all_unity_longmsg = 'No missing flux measurements in %s' % ms.basename 


class SetjyListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing FluxCalibrationResults.
    """
    result_cls = list
    child_cls = commonfluxresults.FluxCalibrationResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated

        mses = [r.inputs['vis'] for r in result]
        longmsg = 'No unmapped science windows in %s' % utils.commafy(mses,
                                                                    quotes=False,
                                                                    conjunction='or')
        result.qa.all_unity_longmsg = longmsg

