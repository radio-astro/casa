from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import qa
from . import renderer
from . import resultobjects
from .gfluxscaleflag import Gfluxscaleflag

qaadapter.registry.register_to_calibration_topic(resultobjects.GfluxscaleflagResults)

weblog.add_renderer(Gfluxscaleflag, renderer.T2_4MDetailsGfluxscaleflagRenderer(), group_by=weblog.UNGROUPED)
