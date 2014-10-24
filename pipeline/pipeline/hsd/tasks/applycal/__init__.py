from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .applycal import SDApplyCal
from . import applycal
from . import renderer

qaadapter.registry.register_to_calibration_topic(applycal.SDApplyCalResults)

weblog.add_renderer(SDApplyCal, renderer.T2_4MDetailsSingleDishApplycalRenderer())
