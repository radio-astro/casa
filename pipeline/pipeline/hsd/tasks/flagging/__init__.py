from __future__ import absolute_import

import pipeline.h.tasks.flagging.renderer as super_renderer
import pipeline.infrastructure.renderer.weblog as weblog
from .flagdeteralmasd import FlagDeterALMASingleDish, HpcFlagDeterALMASingleDish

# Use generic deterministic flagging renderer for ALMA SD
# deterministic flagging.
weblog.add_renderer(FlagDeterALMASingleDish,
                    super_renderer.T2_4MDetailsFlagDeterBaseRenderer(description='ALMA SD deterministic flagging'),
                    group_by=weblog.UNGROUPED)
weblog.add_renderer(HpcFlagDeterALMASingleDish,
                    super_renderer.T2_4MDetailsFlagDeterBaseRenderer(description='ALMA SD deterministic flagging'),
                    group_by=weblog.UNGROUPED)
