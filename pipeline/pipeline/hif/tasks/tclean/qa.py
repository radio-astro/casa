from __future__ import absolute_import
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

import pipeline.qa.utility.scorers as scorers

from . import resultobjects

from casac import casac

import numpy

LOG = logging.get_logger(__name__)


class TcleanQAHandler(pqa.QAResultHandler):    
    result_cls = resultobjects.TcleanResult
    child_cls = None

    def handle(self, context, result):
        # calculate QA score comparing RMS against clean threshold

        imageScorer = scorers.erfScorer(1.0, 5.0)

        qaTool = casac.quanta()
        try:
            # The threshold applies to peaks in the residual. To compare to the
            # measured RMS, one needs to translate peak to RMS by about a
            # factor 4.0.
            score = imageScorer(result.rms / qaTool.convert(result.threshold, 'Jy')['value'] * 4.0)
        except Exception as e:
            LOG.warning('Exception scoring imaging result by RMS: %s. Setting score to -0.1.' % (e))
            score = -0.1
        if (numpy.isnan(score)):
            result.qa.pool[:] = [pqa.QAScore(0.0, longmsg='Cleaning diverged', shortmsg='Cleaning diverged')]
        else:
            result.qa.pool[:] = [pqa.QAScore(score, longmsg='RMS outside mask vs. threshold', shortmsg='RMS vs. threshold')]


class TcleanListQAHandler(pqa.QAResultHandler):
    result_cls = list
    child_cls = resultobjects.TcleanResult

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
