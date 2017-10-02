"""
Created on 01 Jun 2017

@author: Vincent Geers (UKATC)
"""

from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import resultobjects

LOG = logging.get_logger(__name__)


class GfluxscaleflagQAHandler(pqa.QAResultHandler):
    """
    QA handler for an uncontained GfluxscaleflagResults.
    """
    result_cls = resultobjects.GfluxscaleflagResults
    child_cls = None

    def handle(self, context, result):

        # Run correctedampflag QA on correctedampflag result.
        pqa.registry.do_qa(context, result.cafresult)

        # Gather scores, store in result.
        scores = result.cafresult.qa.pool
        result.qa.pool[:] = scores


class GfluxscaleflagListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing GfluxscaleflagResults.
    """
    result_cls = list
    child_cls = resultobjects.GfluxscaleflagResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated
