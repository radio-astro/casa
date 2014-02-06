from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa

from . import qa
from .flagdeterbase import FlagDeterBase
from .flagdatasetter import FlagdataSetter
from .agentflagger import AgentFlagger

pipelineqa.registry.add_handler(qa.AgentFlaggerQAHandler())
pipelineqa.registry.add_handler(qa.AgentFlaggerListQAHandler())
