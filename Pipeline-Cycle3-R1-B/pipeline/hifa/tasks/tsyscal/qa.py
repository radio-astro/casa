from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.qa.scorecalculator as qacalc

from . import tsyscal
from . import resultobjects

LOG = logging.get_logger(__name__)


class TsyscalQAHandler(pqa.QAResultHandler):
    result_cls = resultobjects.TsyscalResults
    child_cls = None
    generating_task = tsyscal.Tsyscal
    
    def handle(self, context, result):

	vis= result.inputs['vis']
	ms = context.observing_run.get_ms(vis)

	# Check for  existence of unmapped spws and assign a
	# score based on the number of unmapped windows relative
	# to mapped ones.
	scores = [qacalc.score_tsysspwmap(ms, result.unmappedspws)]
	result.qa.pool[:] = scores
	result.qa.all_unity_longmsg = 'No unmapped science windows in %s' % ms.basename 


class TsyscalListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing TsyscalResults.
    """
    result_cls = list
    child_cls = resultobjects.TsyscalResults

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
                                                                                        
