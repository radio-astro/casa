from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .tsyscal import Tsyscal
from . import qa
from . import renderer
from . import resultobjects
# import to module level required for Tsysflagchans etc.
from .renderer import T2_4MDetailsTsyscalRenderer, TsyscalPlotRenderer

pipelineqa.registry.add_handler(qa.TsyscalQAHandler())
pipelineqa.registry.add_handler(qa.TsyscalListQAHandler())
qaadapter.registry.register_to_calibration_topic(resultobjects.TsyscalResults)

weblog.add_renderer(Tsyscal, renderer.T2_4MDetailsTsyscalRenderer())
