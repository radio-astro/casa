from __future__ import absolute_import
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc

from . import resultobjects

LOG = logging.get_logger(__name__)


class BpflagchansQAHandler(pqa.QAResultHandler):    
    """
    QA handler for an uncontained BandpassflagResults.
    """
    result_cls = resultobjects.BandpassflagResults
    child_cls = None

    def handle(self, context, result):
        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)
    
        scores = [qacalc.linear_score_fraction_newly_flagged(ms.basename, result.summaries)]
        result.qa.pool[:] = scores

        result.qa.all_unity_longmsg = 'No extra data was flagged in %s' % vis


class BpflagchansListQAHandler(pqa.QAResultHandler):
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

        vises = [r.inputs['vis'] for r in result]
        longmsg = 'No extra data was flagged in %s' % utils.commafy(vises, 
                                                                    quotes=False, 
                                                                    conjunction='or')
        result.qa.all_unity_longmsg = longmsg
