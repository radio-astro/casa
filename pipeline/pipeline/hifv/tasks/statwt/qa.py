from __future__ import absolute_import

import collections

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc
from . import statwt

LOG = logging.get_logger(__name__)

class StatwtQAHandler(pqa.QAPlugin):
    result_cls = statwt.StatwtResults
    child_cls = None
    generating_task = statwt.Statwt

    def handle(self, context, result):
        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)

        # Score based on incremental flag fraction
        score0 = qacalc.score_data_flagged_by_agents(ms, result.summaries,
                                                     0.05, 0.6, agents=[ 'statwt' ])
        new_origin = pqa.QAOrigin(metric_name='%StatwtFlagging',
                                  metric_score=score0.origin.metric_score,
                                  metric_units=score0.origin.metric_units)
        score0.origin = new_origin
        
        scores = [ score0 ]

        result.qa.pool.extend(scores)

class StatwtListQAHandler(pqa.QAPlugin):
    """
    QA handler for a list containing StatwtResults.
    """
    result_cls = collections.Iterable
    child_cls = statwt.StatwtResults
    generating_task = statwt.Statwt

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated