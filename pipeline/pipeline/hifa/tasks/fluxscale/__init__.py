from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .gcorfluxscale import GcorFluxscale, GcorFluxscaleResults, SessionGcorFluxscale
from . import qa
from . import renderer

pipelineqa.registry.add_handler(qa.GcorFluxscaleQAHandler())
pipelineqa.registry.add_handler(qa.GcorFluxscaleListQAHandler())
qaadapter.registry.register_to_calibration_topic(GcorFluxscaleResults)

weblog.add_renderer(GcorFluxscale, renderer.T2_4MDetailsGFluxscaleRenderer(), group_by=weblog.UNGROUPED)
weblog.add_renderer(SessionGcorFluxscale, renderer.T2_4MDetailsGFluxscaleRenderer(), group_by=weblog.UNGROUPED)
