from __future__ import absolute_import

import numpy

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc
from . import skycal

LOG = logging.get_logger(__name__)

class SDSkyCalQAHandler(pqa.QAPlugin):    
    result_cls = skycal.SDSkyCalResults
    child_cls = None

    def handle(self, context, result):
        calapps = result.outcome
        resultdict = skycal.compute_elevation_difference(context, result)
        vis = calapps[0].calto.vis
        ms = context.observing_run.get_ms(vis)
        scores = qacalc.score_sd_skycal_elevation_difference(ms, resultdict)
        result.qa.pool.append(scores)


class SDSkyCalListQAHandler(pqa.QAPlugin):
    result_cls = basetask.ResultsList
    child_cls = skycal.SDSkyCalResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
