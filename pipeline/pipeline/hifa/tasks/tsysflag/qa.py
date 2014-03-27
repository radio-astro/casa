from __future__ import absolute_import
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc

from . import resultobjects

LOG = logging.get_logger(__name__)


class TsysflagQAHandler(pqa.QAResultHandler):    
    """
    QA handler for an uncontained TsysflagResult.
    """
    result_cls = resultobjects.TsysflagResults
    child_cls = None

    def handle(self, context, result):
        caltable = os.path.basename(result.inputs['caltable'])
    
        scores = [qacalc.score_fraction_newly_flagged(caltable, result.summaries)]
        result.qa.pool[:] = scores

        result.qa.all_unity_longmsg = 'No extra data was flagged in %s' % caltable


class TsysflagListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing TsysflagResults.
    """
    result_cls = list
    child_cls = resultobjects.TsysflagResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated

        caltables = [r.inputs['caltable'] for r in result]
        longmsg = 'No extra data was flagged in %s' % utils.commafy(caltables, 
                                                                    quotes=False, 
                                                                    conjunction='or')
        result.qa.all_unity_longmsg = longmsg
