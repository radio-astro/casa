from __future__ import absolute_import

import pipeline.infrastructure.renderer.weblog as weblog

import pipeline.hif.tasks.flagging.renderer as super_renderer

from .flagdeteralmasd import FlagDeterALMASingleDish
#from .almaagentflagger import ALMAAgentFlagger

# use the standard AgentFlagger renderer to render hifa flagging results
weblog.add_renderer(FlagDeterALMASingleDish, super_renderer.T2_4MDetailsAgentFlaggerRenderer(description='ALMA SD deterministic flagging'))
#weblog.add_renderer(ALMAAgentFlagger, super_renderer.T2_4MDetailsAgentFlaggerRenderer(description='ALMA deterministic flagging'))
