from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .gfluxscaleflag import Gfluxscaleflag
from . import resultobjects
from . import renderer
from . import qa

pipelineqa.registry.add_handler(qa.GfluxscaleflagQAHandler())
pipelineqa.registry.add_handler(qa.GfluxscaleflagListQAHandler())
qaadapter.registry.register_to_calibration_topic(resultobjects.GfluxscaleflagResults)

weblog.add_renderer(Gfluxscaleflag, renderer.T2_4MDetailsGfluxscaleflagRenderer(), group_by=weblog.UNGROUPED)
