from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

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

        # FIXME: Placeholder QA score is always 1.
        qa_score = 1.0
        longmsg = 'No QA heuristic for Bandpassflag, defaulting to score of 1.'
        shortmsg = 'No QA heuristic'
        origin = pqa.QAOrigin(
            metric_name='BandpassflagQAHandler',
            metric_score=bool(qa_score),
            metric_units='No QA heuristic')
        score = pqa.QAScore(qa_score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)

        # Gather scores, store in result.
        scores = [score]
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
