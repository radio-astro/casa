from __future__ import absolute_import

import numpy

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc
import pipeline.h.tasks.exportdata.aqua as aqua
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
        ###
        #ms_index = context.observing_run.measurement_sets.index(ms)
        #if ms_index == 0:
        #    resultdict[1][0][19].eldiff0[-16] = 3.78
        #if ms_index == 1:
        #    resultdict[1][1][19].eldiff1[2] = -4.0
        ###
        threshold = skycal.SerialSDSkyCal.ElevationDifferenceThreshold
        scores = qacalc.score_sd_skycal_elevation_difference(ms, resultdict,
                                                             threshold=threshold)
        result.qa.pool.append(scores)


class SDSkyCalListQAHandler(pqa.QAPlugin):
    result_cls = basetask.ResultsList
    child_cls = skycal.SDSkyCalResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated

aqua_exporter = aqua.xml_generator_for_metric('OnOffElevationDifference', '{:0.3f}deg')
aqua.register_aqua_metric(aqua_exporter)