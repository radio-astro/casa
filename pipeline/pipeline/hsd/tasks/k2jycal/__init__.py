from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog

from .k2jycal import SDK2JyCal
from . import jyperkreader
from . import worker
from . import renderer
from . import qa

pipelineqa.registry.add_handler(qa.SDK2JyCalQAHandler())
pipelineqa.registry.add_handler(qa.SDK2JyCalListQAHandler())

qaadapter.registry.register_to_calibration_topic(k2jycal.SDK2JyCalResults)

weblog.add_renderer(SDK2JyCal, renderer.T2_4MDetailsSingleDishK2JyCalRenderer(always_rerender=True), group_by='ungrouped')
