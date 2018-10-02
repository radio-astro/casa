from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from pipeline.hif.tasks.gainflag import qa
from . import renderer
from . import resultobjects
from .gainflag import Gainflag

qaadapter.registry.register_to_flagging_topic(resultobjects.GainflagResults)

weblog.add_renderer(Gainflag, renderer.T2_4MDetailsGainflagRenderer(), group_by=weblog.UNGROUPED)
