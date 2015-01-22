from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .tsysflag import Tsysflag
from ..tsysflagchans import Tsysflagchans
from . import qa
from . import renderer
from ..tsysflagspectra import resultobjects

pipelineqa.registry.add_handler(qa.TsysflagQAHandler())
pipelineqa.registry.add_handler(qa.TsysflagListQAHandler())
qaadapter.registry.register_to_flagging_topic(resultobjects.TsysflagspectraResults)

weblog.add_renderer(Tsysflag, renderer.T2_4MDetailsTsysflagRenderer())