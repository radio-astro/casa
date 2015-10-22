from __future__ import absolute_import
import pipeline.infrastructure.renderer.weblog as weblog

from .flagbaseline import SDFlagBaseline
from .plotflagbaseline import SDPlotFlagBaseline
from . import renderer

weblog.add_renderer(SDFlagBaseline, renderer.T2_4MDetailsSingleDishFlagBaselineRenderer(), group_by='ungrouped')
weblog.add_renderer(SDPlotFlagBaseline, renderer.T2_4MDetailsSingleDishPlotFlagBaselineRenderer(), group_by='ungrouped')
