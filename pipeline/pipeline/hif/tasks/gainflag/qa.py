from __future__ import absolute_import
import collections

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc

import pipeline.h.tasks.exportdata.aqua as aqua
from . import resultobjects

LOG = logging.get_logger(__name__)


class GainflagQAHandler(pqa.QAResultHandler):    
    """
    QA handler for an uncontained GainflagResults.
    """
    result_cls = resultobjects.GainflagResults
    child_cls = None

    def handle(self, context, result):
        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)
    
        # Calculate QA score from presence of flagging views and from the
        # flagging summary in the result, adopting the minimum score as the
        # representative score for this task.
        score1 = qacalc.linear_score_fraction_newly_flagged(ms.basename,
                                                             result.summaries,
                                                             ms.basename)
        new_origin = pqa.QAOrigin(metric_name='%GainFlags',
                                  metric_score=score1.origin.metric_score,
                                  metric_units='Percentage of gain flag data newly flagged')
        score1.origin = new_origin

        score2 = qacalc.score_flagging_view_exists(ms.basename, result)
        new_origin = pqa.QAOrigin(metric_name='ValidFlaggingView',
                                  metric_score=score2.origin.metric_score,
                                  metric_units='Valid flagging view')
        score2.origin = new_origin

        scores = [score1, score2]
        result.qa.pool[:] = scores

        result.qa.all_unity_longmsg = 'No extra data was flagged in %s' % vis


class GainflagListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing GainflagResults.
    """
    result_cls = collections.Iterable
    child_cls = resultobjects.GainflagResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated

        vises = [r.inputs['vis'] for r in result]
        longmsg = 'No extra data was flagged in %s' % utils.commafy(vises, 
                                                                    quotes=False, 
                                                                    conjunction='or')
        result.qa.all_unity_longmsg = longmsg


aqua_exporter = aqua.xml_generator_for_metric('%GainFlags', '{:0.3%}')
aqua.register_aqua_metric(aqua_exporter)
