from __future__ import absolute_import
import collections
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc

import pipeline.h.tasks.exportdata.aqua as aqua
from . import resultobjects

LOG = logging.get_logger(__name__)


class WvrgcalflagQAHandler(pqa.QAResultHandler):    
    """
    QA handler for an uncontained WvrgcalflagResults.
    """
    result_cls = resultobjects.WvrgcalflagResults
    child_cls = None

    def handle(self, context, result):
        ms_name = os.path.basename(result.inputs['vis'])

        # If too few unflagged antennas were left over after flagging,
        # then return a fixed very low score:
        if result.too_few_wvr_post_flagging:
            score_object = pqa.QAScore(
                0.1, longmsg='Not enough unflagged WVR available',
                shortmsg='Not enough unflagged WVR', vis=ms_name)
            new_origin = pqa.QAOrigin(
                metric_name='PhaseRmsRatio',
                metric_score=score_object.origin.metric_score,
                metric_units='Phase RMS improvement after applying WVR correction')
            score_object.origin = new_origin
            result.qa.pool[:] = [score_object]
        else:
            # Try to retrieve WVR QA score from result.
            try:
                wvr_score = result.flaggerresult.dataresult.qa_wvr.overall_score
                # If a WVR QA score was available, then adopt this as the
                # final QA score for the task.
                if wvr_score:
                    score_object = qacalc.score_wvrgcal(ms_name, result.flaggerresult.dataresult.qa_wvr.overall_score)
                    new_origin = pqa.QAOrigin(
                        metric_name='PhaseRmsRatio',
                        metric_score=score_object.origin.metric_score,
                        metric_units='Phase RMS improvement after applying WVR correction')
                    score_object.origin = new_origin
                    result.qa.pool[:] = [score_object]
                else:
                    # If wvr_score was not available, check if this is caused
                    # by too few antennas with WVR (set by threshold). If so,
                    # then no QA score is necessary; if not, then set task QA
                    # score to 0.
                    if not result.too_few_wvr:
                        score_object = pqa.QAScore(
                            0.0, longmsg='No WVR scores available',
                            shortmsg='No WVR', vis=ms_name)
                        new_origin = pqa.QAOrigin(
                            metric_name='PhaseRmsRatio',
                            metric_score=score_object.origin.metric_score,
                            metric_units='Phase RMS improvement after applying WVR correction')
                        score_object.origin = new_origin
                        result.qa.pool[:] = [score_object]
            except AttributeError:
                pass
    

class WvrgcalflagListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing WvrgcalflagResults.
    """
    result_cls = collections.Iterable
    child_cls = resultobjects.WvrgcalflagResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated


aqua_exporter = aqua.xml_generator_for_metric('PhaseRmsRatio', '{:0.3f}')
aqua.register_aqua_metric(aqua_exporter)
