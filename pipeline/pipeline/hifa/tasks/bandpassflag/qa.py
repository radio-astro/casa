from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc

from . import resultobjects

LOG = logging.get_logger(__name__)


class BandpassflagQAHandler(pqa.QAResultHandler):
    """
    QA handler for an uncontained BandpassflagResults.
    """
    result_cls = resultobjects.BandpassflagResults
    child_cls = None

    def handle(self, context, result):
        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)

        # Run correctedampflag QA on correctedampflag result.
        pqa.registry.do_qa(context, result.cafresult)

        # Run bandpass QA on bandpass result.
        pqa.registry.do_qa(context, result.bpresult)

        # Create bandpassflag specific score, set to a multiplication of
        # the representative score in correctedampflag QA pool and the
        # representative score in the bandpass QA pool.
        score = qacalc.score_multiply(
            [result.cafresult.qa.representative.score,
             result.bpresult.qa.representative.score])

        # Update score.
        longmsg = 'Combined score, based on flagging score and bandpass score'
        shortmsg = 'Combined score'
        new_origin = pqa.QAOrigin(
            metric_name='BandpassflagQAHandler',
            metric_score=bool(score),
            metric_units='Presence of combined score.')
        score.longmsg = longmsg
        score.shortmsg = shortmsg
        score.origin = new_origin

        # Gather scores, store in result.
        scores = [score] + result.bpresult.qa.pool + result.cafresult.qa.pool
        result.qa.pool[:] = scores


class BandpassflagListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing BandpassflagResults.
    """
    result_cls = list
    child_cls = resultobjects.BandpassflagResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated
