from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa

from .referenceantenna import RefAnt
from . import qa

pipelineqa.registry.add_handler(qa.RefantQAHandler())
pipelineqa.registry.add_handler(qa.RefantListQAHandler())
