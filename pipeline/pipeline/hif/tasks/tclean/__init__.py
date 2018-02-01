from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import qa
from . import renderer
from . import resultobjects
from .tclean import Tclean

qaadapter.registry.register_to_imaging_topic(resultobjects.BoxResult)
qaadapter.registry.register_to_imaging_topic(resultobjects.TcleanResult)

weblog.add_renderer(Tclean, renderer.T2_4MDetailsTcleanRenderer(), group_by=weblog.UNGROUPED)
