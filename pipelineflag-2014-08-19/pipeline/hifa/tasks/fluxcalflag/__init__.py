from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .fluxcalflag import FluxcalFlag
from . import qa
from . import fluxcalflag

pipelineqa.registry.add_handler(qa.FluxcalflagQAHandler())
pipelineqa.registry.add_handler(qa.FluxcalflagListQAHandler())
qaadapter.registry.register_to_flagging_topic(fluxcalflag.FluxcalFlagResults)
