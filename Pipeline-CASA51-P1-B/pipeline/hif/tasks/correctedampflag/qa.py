from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc

from . import resultobjects

LOG = logging.get_logger(__name__)


class CorrectedampflagQAHandler(pqa.QAResultHandler):
    """
    QA handler for an uncontained CorrectedampflagResults.
    """
    result_cls = resultobjects.CorrectedampflagResults
    child_cls = None

    def handle(self, context, result):
        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)

        score = qacalc.linear_score_fraction_newly_flagged(
            ms.basename, result.summaries, ms.basename)
        new_origin = pqa.QAOrigin(
            metric_name='CorrectedampflagQAHandler',
            metric_score=score.origin.metric_score,
            metric_units='Percentage of newly flagged data')
        score.origin = new_origin

        # Gather scores, store in result.
        scores = [score]
        result.qa.pool[:] = scores

        result.qa.all_unity_longmsg = 'No extra data was flagged in %s' % vis


class CorrectedampflagListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing CorrectedampflagResults.
    """
    result_cls = list
    child_cls = resultobjects.CorrectedampflagResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
