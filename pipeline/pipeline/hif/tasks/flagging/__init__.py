from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from . import agentflagger
from . import flagdeterbase
from . import qa
from . import renderer

from .flagdeterbase import FlagDeterBase
from .flagdatasetter import FlagdataSetter
from .agentflagger import AgentFlagger

pipelineqa.registry.add_handler(qa.AgentFlaggerQAHandler())
pipelineqa.registry.add_handler(qa.AgentFlaggerListQAHandler())
qaadapter.registry.register_to_flagging_topic(agentflagger.AgentFlaggerResults)

pipelineqa.registry.add_handler(qa.FlagDeterBaseQAHandler())
pipelineqa.registry.add_handler(qa.FlagDeterBaseListQAHandler())
qaadapter.registry.register_to_flagging_topic(flagdeterbase.FlagDeterBaseResults)

weblog.add_renderer(AgentFlagger, renderer.T2_4MDetailsAgentFlaggerRenderer())
