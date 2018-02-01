from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from pipeline.hif.tasks.rawflagchans import qa
from . import renderer
from . import resultobjects
from .rawflagchans import Rawflagchans

qaadapter.registry.register_to_flagging_topic(resultobjects.RawflagchansResults)

weblog.add_renderer(Rawflagchans, renderer.T2_4MDetailsRawflagchansRenderer(), group_by=weblog.UNGROUPED)
