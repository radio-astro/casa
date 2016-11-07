from __future__ import absolute_import

import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import agentflagger
from . import qa
from . import renderer
from .agentflagger import AgentFlagger

pipelineqa.registry.add_handler(qa.AgentFlaggerQAHandler())
pipelineqa.registry.add_handler(qa.AgentFlaggerListQAHandler())
qaadapter.registry.register_to_flagging_topic(agentflagger.AgentFlaggerResults)

weblog.add_renderer(AgentFlagger, renderer.T2_4MDetailsAgentFlaggerRenderer(), group_by=weblog.UNGROUPED)
