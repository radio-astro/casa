from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .referenceantenna import RefAnt
from . import qa
from . import referenceantenna

pipelineqa.registry.add_handler(qa.RefantQAHandler())
pipelineqa.registry.add_handler(qa.RefantListQAHandler())
qaadapter.registry.register_to_miscellaneous_topic(referenceantenna.RefAntResults)
