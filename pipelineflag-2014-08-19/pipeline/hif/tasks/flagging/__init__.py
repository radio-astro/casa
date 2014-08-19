from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from . import agentflagger
from . import qa

from .flagdeterbase import FlagDeterBase
from .flagdatasetter import FlagdataSetter
from .agentflagger import AgentFlagger

pipelineqa.registry.add_handler(qa.AgentFlaggerQAHandler())
pipelineqa.registry.add_handler(qa.AgentFlaggerListQAHandler())
qaadapter.registry.register_to_flagging_topic(agentflagger.AgentFlaggerResults)

pipelineqa.registry.add_handler(qa.FlagDeterBaseQAHandler())
pipelineqa.registry.add_handler(qa.FlagDeterBaseListQAHandler())
qaadapter.registry.register_to_flagging_topic(flagdeterbase.FlagDeterBaseResults)
