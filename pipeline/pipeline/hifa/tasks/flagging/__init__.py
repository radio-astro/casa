from __future__ import absolute_import

import pipeline.h.tasks.flagging.renderer as super_renderer
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import qa
from . import renderer
from .flagdeteralma import FlagDeterALMA
from .flagtargetsalma import FlagTargetsALMA

qaadapter.registry.register_to_flagging_topic(flagtargetsalma.FlagTargetsALMAResults)

# Use generic deterministic flagging renderer for ALMA interferometry
# deterministic flagging.
weblog.add_renderer(FlagDeterALMA,
                    super_renderer.T2_4MDetailsFlagDeterBaseRenderer(description='ALMA deterministic flagging'),
                    group_by=weblog.UNGROUPED)

# Use locally defined renderer for ALMA interferometry target flagging.
weblog.add_renderer(FlagTargetsALMA,
                    renderer.T2_4MDetailsFlagTargetsALMARenderer(description='ALMA Target flagging'),
                    group_by=weblog.UNGROUPED)
