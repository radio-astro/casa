from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .lowgainflag import Lowgainflag
from . import qa
from . import resultobjects

pipelineqa.registry.add_handler(qa.LowgainflagQAHandler())
pipelineqa.registry.add_handler(qa.LowgainflagListQAHandler())
qaadapter.registry.register_to_flagging_topic(resultobjects.LowgainflagResults)
