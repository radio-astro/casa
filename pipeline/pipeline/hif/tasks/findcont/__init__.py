from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import qa
from . import renderer
from . import resultobjects
from .findcont import FindCont

qaadapter.registry.register_to_imaging_topic(resultobjects.FindContResult)

weblog.add_renderer(FindCont, renderer.T2_4MDetailsFindContRenderer(), group_by=weblog.UNGROUPED)
