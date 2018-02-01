from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from pipeline.hif.tasks.correctedampflag import qa
from . import renderer
from . import resultobjects
from .correctedampflag import Correctedampflag

qaadapter.registry.register_to_flagging_topic(resultobjects.CorrectedampflagResults)

weblog.add_renderer(Correctedampflag, renderer.T2_4MDetailsCorrectedampflagRenderer(), group_by=weblog.UNGROUPED)
