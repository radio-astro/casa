from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc
from . import resultobjects

LOG = logging.get_logger(__name__)


class CorrectedampflagQAHandler(pqa.QAPlugin):
    """
    QA handler for an uncontained CorrectedampflagResults.
    """
    result_cls = resultobjects.CorrectedampflagResults
    child_cls = None

    def handle(self, context, result):
        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)
        scores = []
        intents = result.inputs['intent'].split(',')

        # Create a separate flagging score for each intent.
        for intent in intents:
            score = qacalc.linear_score_fraction_unflagged_newly_flagged_for_intent(
                ms, result.summaries, intent)
            new_origin = pqa.QAOrigin(
                metric_name='CorrectedampflagQAHandler',
                metric_score=score.origin.metric_score,
                metric_units='Fraction of unflagged {} data newly '
                             'flagged'.format(intent))
            score.origin = new_origin

            scores.append(score)

        # If multiple intents were present, create a combined score that
        # is the multiplication of the individual intent flagging scores.
        if len(scores) > 1:
            combined_score = qacalc.score_multiply([score.score for score in scores])

            longmsg = 'Combined flagging score for {}: multiplication of '\
                      'flagging scores for individual intents.'.format(ms.basename)
            shortmsg = 'Combined flagging score'

            new_origin = pqa.QAOrigin(
                metric_name='CorrectedampflagQAHandler',
                metric_score=bool(combined_score),
                metric_units='Presence of combined score.')
            combined_score.longmsg = longmsg
            combined_score.shortmsg = shortmsg
            combined_score.origin = new_origin

            scores = [combined_score] + scores

        # Store in result.
        result.qa.pool[:] = scores

        result.qa.all_unity_longmsg = 'No extra data was flagged in %s' % vis


class CorrectedampflagListQAHandler(pqa.QAPlugin):
    """
    QA handler for a list containing CorrectedampflagResults.
    """
    result_cls = basetask.ResultsList
    child_cls = resultobjects.CorrectedampflagResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
