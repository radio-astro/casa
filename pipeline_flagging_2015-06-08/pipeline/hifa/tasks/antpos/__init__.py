from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from pipeline.hif.tasks.antpos import antpos 
from .almaantpos import ALMAAntpos
from . import almaantpos
from . import renderer
from . import qa

pipelineqa.registry.add_handler(qa.ALMAAntposQAHandler())
pipelineqa.registry.add_handler(qa.ALMAAntposListQAHandler())
qaadapter.registry.register_to_calibration_topic(antpos.AntposResults)

# Simple web log
weblog.add_renderer(ALMAAntpos,
                    renderer.T2_4MDetailsALMAAntposRenderer(),
                    group_by=weblog.UNGROUPED)


