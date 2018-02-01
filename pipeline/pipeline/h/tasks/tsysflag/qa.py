from __future__ import absolute_import

import collections
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc
from . import resultobjects
from ..exportdata import aqua

LOG = logging.get_logger(__name__)


class TsysflagQAHandler(pqa.QAPlugin):
    """
    QA handler for an uncontained TsysflagResult.
    """
    result_cls = resultobjects.TsysflagResults
    child_cls = None

    def handle(self, context, result):
        caltable = os.path.basename(result.inputs['caltable'])
        vis = os.path.basename(result.inputs['vis'])
        ms = context.observing_run.get_ms(vis)

        if result.task_incomplete_reason:
            score = pqa.QAScore(0.0, longmsg='Task ended prematurely',
                shortmsg='Task ended prematurely', vis=vis)
            new_origin = pqa.QAOrigin(metric_name='%TsysCaltableFlags',
                metric_score=score.origin.metric_score,
                metric_units='Percentage Tsys caltable newly flagged')
            score.origin = new_origin
            scores = [score]
        else:
            try:
                score = qacalc.score_fraction_newly_flagged(caltable,
                    result.summaries, ms.basename)
                new_origin = pqa.QAOrigin(metric_name='%TsysCaltableFlags',
                    metric_score=score.origin.metric_score,
                    metric_units='Percentage Tsys caltable newly flagged')
                score.origin = new_origin
                scores = [score]
            except AttributeError:
                score = pqa.QAScore(0.0, longmsg='No flagging summaries available',
                    shortmsg='No flag summaries', vis=vis)
                new_origin = pqa.QAOrigin(metric_name='%TsysCaltableFlags',
                    metric_score=score.origin.metric_score,
                    metric_units='Percentage Tsys caltable newly flagged')
                score.origin = new_origin
                scores = [score]
            
        result.qa.pool[:] = scores

        result.qa.all_unity_longmsg = 'No extra data was flagged in %s' % caltable


class TsysflagListQAHandler(pqa.QAPlugin):
    """
    QA handler for a list containing TsysflagResults.
    """
    result_cls = collections.Iterable
    child_cls = resultobjects.TsysflagResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated

        caltables = [r.inputs['caltable'] for r in result]
        longmsg = 'No extra data was flagged in %s'.format(
            utils.commafy(caltables, quotes=False, conjunction='or'))
        result.qa.all_unity_longmsg = longmsg


aqua_exporter = aqua.xml_generator_for_metric('%TsysCaltableFlags', '{:0.3%}')
aqua.register_aqua_metric(aqua_exporter)
