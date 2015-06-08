from __future__ import absolute_import
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc

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
            wvr_score = result.qa_wvr.overall_score
            if wvr_score is not None:
                result.qa.pool[:] = [qacalc.score_wvrgcal(ms_name, result.qa_wvr.overall_score)]
            else:
                # If wvr_score was not available, check if this is caused 
                # by lack of 12m data in MS. If lack of wvr_score was not 
                # due to lack of WVR data, then set QA score to 0.
                ms = context.observing_run.get_ms(result.inputs['vis'])
                if not all([a for a in ms.antennas if a.diameter != 12.0]):
                    result.qa.pool[:] = [pqa.QAScore(0.0, longmsg='No WVR scores available',
                                         shortmsg='No wvr', vis=ms_name)]
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
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
