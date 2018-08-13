from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .skycal import SerialSDSkyCal
from .skycal import HpcSDSkyCal
from . import skycal
from . import qa
from . import renderer

SDSkyCal = HpcSDSkyCal

qaadapter.registry.register_to_calibration_topic(skycal.SDSkyCalResults)

weblog.add_renderer(SerialSDSkyCal, renderer.T2_4MDetailsSingleDishSkyCalRenderer(), group_by=weblog.UNGROUPED)

# this is necessary when jobs executed in MPIServer throws an exception
weblog.add_renderer(HpcSDSkyCal, renderer.T2_4MDetailsSingleDishSkyCalRenderer(), group_by=weblog.UNGROUPED)
