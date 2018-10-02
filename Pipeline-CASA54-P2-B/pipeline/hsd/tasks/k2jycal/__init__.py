from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import jyperkreader
from . import qa
from . import renderer
from .k2jycal import SDK2JyCal

qaadapter.registry.register_to_calibration_topic(k2jycal.SDK2JyCalResults)

weblog.add_renderer(SDK2JyCal, renderer.T2_4MDetailsSingleDishK2JyCalRenderer(), group_by=weblog.UNGROUPED)
