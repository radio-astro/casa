from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .tsysflagspectra import TsysflagspectraInputs, Tsysflagspectra
from pipeline.hifa.tasks.tsysflagspectra import qa
from .resultobjects import TsysflagspectraResults

pipelineqa.registry.add_handler(qa.TsysflagspectraQAHandler())
pipelineqa.registry.add_handler(qa.TsysflagspectraListQAHandler())
qaadapter.registry.register_to_flagging_topic(resultobjects.TsysflagspectraResults)

