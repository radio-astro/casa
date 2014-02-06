from __future__ import absolute_import
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa2.scorecalculator as qa2calc

from . import resultobjects

LOG = logging.get_logger(__name__)


class WvrgcalflagQAHandler(pqa.QAResultHandler):    
    """
    QA handler for an uncontained WvrgcalflagResult.
    """
    result_cls = resultobjects.WvrgcalflagResult
    child_cls = None

    def handle(self, context, result):
        ms_name = os.path.basename(result.inputs['vis'])

        try:
            wvr_score = result.qa2.overall_score
            if wvr_score is not None:
                scores = [qa2calc.score_wvrgcal(ms_name, result.qa2.overall_score)]
                result.qa.pool[:] = scores
        except AttributeError:
            pass
    

class WvrgcalflagListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing WvrgcalflagResults.
    """
    result_cls = list
    child_cls = resultobjects.WvrgcalflagResult

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QA2score list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
