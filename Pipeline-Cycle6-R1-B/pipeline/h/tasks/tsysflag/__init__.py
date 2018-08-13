from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import qa
from . import renderer
from . import resultobjects
from .tsysflag import Tsysflag

qaadapter.registry.register_to_flagging_topic(resultobjects.TsysflagResults)

weblog.add_renderer(Tsysflag, renderer.T2_4MDetailsTsysflagRenderer(), group_by=weblog.UNGROUPED)