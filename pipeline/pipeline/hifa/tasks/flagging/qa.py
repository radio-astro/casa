from __future__ import absolute_import
import collections

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc
from . import flagtargetsalma

LOG = logging.get_logger(__name__)


class FlagTargetsALMAQAHandler(pqa.QAResultHandler):
    result_cls = flagtargetsalma.FlagTargetsALMAResults
    child_cls = None

    def handle(self, context, result):
        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)

        # calculate QA scores from agentflagger summary dictionary, adopting
        # the minimum score as the representative score for this task
        # Leave in the flag summary off option
        try:
            scores = [qacalc.score_almatargets_agents(ms, result.summaries)]
        except:
            scores = [pqa.QAScore(1.0, longmsg='Flag Summary off',
                                  shortmsg='Flag Summary off')]

        result.qa.pool[:] = scores


class FlagTargetsALMAListQAHandler(pqa.QAResultHandler):
    result_cls = collections.Iterable
    child_cls = flagtargetsalma.FlagTargetsALMAResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated
