from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .tsysflagspectra import Tsysflagspectra, TsysflagspectraInputs
from . import qa
from . import resultobjects
from . import renderer
from .resultobjects import TsysflagspectraResults

pipelineqa.registry.add_handler(qa.TsysflagspectraQAHandler())
pipelineqa.registry.add_handler(qa.TsysflagspectraListQAHandler())
qaadapter.registry.register_to_flagging_topic(resultobjects.TsysflagspectraResults)

weblog.add_renderer(Tsysflagspectra, renderer.T2_4MDetailsTsysflagspectraRenderer())
