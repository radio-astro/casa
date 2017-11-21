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

        try:
            wvr_score = result.flaggerresult.dataresult.qa_wvr.overall_score
            if wvr_score is not None:
                score_object = qacalc.score_wvrgcal(ms_name, result.flaggerresult.dataresult.qa_wvr.overall_score)
                new_origin = pqa.QAOrigin(
                    metric_name='PhaseRmsRatio',
                    metric_score=score_object.origin.metric_score,
                    metric_units='Phase RMS improvement after applying WVR correction')
                score_object.origin = new_origin
                result.qa.pool[:] = [score_object]
            else:
                # If wvr_score was not available, check if this is caused 
                # by lack of 12m data in MS. If lack of wvr_score was not 
                # due to lack of WVR data, then set QA score to 0.
                ms = context.observing_run.get_ms(result.inputs['vis'])
                if not all([a for a in ms.antennas if a.diameter != 12.0]):
                    score_object = pqa.QAScore(0.0, longmsg='No WVR scores available',
                                               shortmsg='No wvr', vis=ms_name)
                    score_object.origin.metric_name = 'PhaseRmsRatio'
                    score_object.origin.metric_units = 'Phase RMS improvement after applying WVR correction'
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
