from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import qa
from . import renderer
from . import resultobjects
from .tsyscal import Tsyscal

qaadapter.registry.register_to_calibration_topic(resultobjects.TsyscalResults)

weblog.add_renderer(Tsyscal, renderer.T2_4MDetailsTsyscalRenderer(), group_by=weblog.UNGROUPED)
