from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import qa
from . import renderer
from . import resultobjects
from .lowgainflag import Lowgainflag

qaadapter.registry.register_to_flagging_topic(resultobjects.LowgainflagResults)

weblog.add_renderer(Lowgainflag, renderer.T2_4MDetailsLowgainFlagRenderer(), group_by=weblog.UNGROUPED)
