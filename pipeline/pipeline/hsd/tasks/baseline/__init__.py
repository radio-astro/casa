from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .baseline import SDBaseline
from . import baseline
from . import renderer

#qaadapter.registry.register_to_calibration_topic(baseline.SDBaselineResults)
qaadapter.registry.register_to_miscellaneous_topic(baseline.SDBaselineResults)

weblog.add_renderer(SDBaseline, renderer.T2_4MDetailsSingleDishBaselineRenderer())
