from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .referenceantenna import RefAnt, HpcRefAnt
from . import renderer
from . import qa
from . import referenceantenna

pipelineqa.registry.add_handler(qa.RefantQAHandler())
pipelineqa.registry.add_handler(qa.RefantListQAHandler())
qaadapter.registry.register_to_miscellaneous_topic(referenceantenna.RefAntResults)

weblog.add_renderer(RefAnt, renderer.T2_4MDetailsRefantRenderer(), group_by=weblog.UNGROUPED)
weblog.add_renderer(HpcRefAnt, renderer.T2_4MDetailsRefantRenderer(), group_by=weblog.UNGROUPED)
