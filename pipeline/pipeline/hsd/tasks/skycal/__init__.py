from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .skycal import SDMSSkyCal
from . import skycal
from . import renderer

qaadapter.registry.register_to_calibration_topic(skycal.SDMSSkyCalResults)

weblog.add_renderer(SDMSSkyCal, renderer.T2_4MDetailsSingleDishCalSkyRenderer(), group_by='ungrouped')
