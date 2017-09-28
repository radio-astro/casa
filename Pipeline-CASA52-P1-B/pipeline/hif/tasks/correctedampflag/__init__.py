from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .correctedampflag import Correctedampflag
from . import resultobjects
from . import renderer

from pipeline.hif.tasks.correctedampflag import qa

pipelineqa.registry.add_handler(qa.CorrectedampflagQAHandler())
pipelineqa.registry.add_handler(qa.CorrectedampflagListQAHandler())
qaadapter.registry.register_to_flagging_topic(resultobjects.CorrectedampflagResults)

weblog.add_renderer(Correctedampflag, renderer.T2_4MDetailsCorrectedampflagRenderer(), group_by=weblog.UNGROUPED)
