from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa

from .lowgainflag import Lowgainflag
from . import qa

pipelineqa.registry.add_handler(qa.LowgainflagQAHandler())
pipelineqa.registry.add_handler(qa.LowgainflagListQAHandler())
