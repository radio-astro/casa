from __future__ import absolute_import

import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.hif.tasks.flagging.renderer as super_renderer

from .flagdeteralma import FlagDeterALMA
from .almaagentflagger import ALMAAgentFlagger
from .flagtargetsalma import FlagTargetsALMA
from . import qa
from . import renderer

pipelineqa.registry.add_handler(qa.FlagTargetsALMAQAHandler())
pipelineqa.registry.add_handler(qa.FlagTargetsALMAListQAHandler())
qaadapter.registry.register_to_flagging_topic(flagtargetsalma.FlagTargetsALMAResults)

# use the standard renderers to render hifa flagging results
#    Note that the ALMA deterministics flagging routines (re)use the generic renderers
weblog.add_renderer(FlagDeterALMA, 
                    super_renderer.T2_4MDetailsAgentFlaggerRenderer(description='ALMA deterministic flagging'),
                    group_by=weblog.UNGROUPED)
weblog.add_renderer(ALMAAgentFlagger,
                    super_renderer.T2_4MDetailsAgentFlaggerRenderer(description='ALMA deterministic flagging'),
                    group_by=weblog.UNGROUPED)
weblog.add_renderer(FlagTargetsALMA, 
                    renderer.T2_4MDetailsFlagTargetsALMARenderer(description='ALMA Target flagging'),
                    group_by=weblog.UNGROUPED)
