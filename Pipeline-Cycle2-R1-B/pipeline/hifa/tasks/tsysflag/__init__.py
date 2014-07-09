from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .tsysflag import Tsysflag
from ..tsysflagchans import Tsysflagchans
from . import qa
from ..tsysflagspectra import resultobjects

pipelineqa.registry.add_handler(qa.TsysflagQAHandler())
pipelineqa.registry.add_handler(qa.TsysflagListQAHandler())
qaadapter.registry.register_to_flagging_topic(resultobjects.TsysflagspectraResults)
