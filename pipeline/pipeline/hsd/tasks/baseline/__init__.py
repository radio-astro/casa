from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import baseline
from . import qa
from . import renderer
from .baseline import SDBaseline

qaadapter.registry.register_to_miscellaneous_topic(baseline.SDBaselineResults)

weblog.add_renderer(SDBaseline, renderer.T2_4MDetailsSingleDishBaselineRenderer(always_rerender=False), group_by=weblog.UNGROUPED)
