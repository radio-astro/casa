from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa

from .fluxcalflag import FluxcalFlag
from . import qa

pipelineqa.registry.add_handler(qa.FluxcalflagQAHandler())
pipelineqa.registry.add_handler(qa.FluxcalflagListQAHandler())
