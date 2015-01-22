from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .tsysflagchans import TsysflagchansInputs, Tsysflagchans
from . import renderer
from ..tsysflagspectra import qa
from ..tsysflagspectra import resultobjects

pipelineqa.registry.add_handler(qa.TsysflagspectraQAHandler())
pipelineqa.registry.add_handler(qa.TsysflagspectraListQAHandler())
qaadapter.registry.register_to_flagging_topic(resultobjects.TsysflagspectraResults)

weblog.add_renderer(Tsysflagchans, renderer.T2_4MDetailsTsysflagchansRenderer())
