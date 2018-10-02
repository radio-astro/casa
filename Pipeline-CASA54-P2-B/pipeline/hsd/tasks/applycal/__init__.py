from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
# from pipeline.hif.tasks.applycal import qa
# from pipeline.hif.tasks.applycal import ApplycalResults
#from pipeline.h.tasks.applycal import ApplycalResults

from . import renderer
from .applycal import SDApplycal, HpcSDApplycal, SDApplycalResults
#from .applycal import SDApplycal, HpcSDApplycal

qaadapter.registry.register_to_flagging_topic(SDApplycalResults)
#qaadapter.registry.register_to_flagging_topic(ApplycalResults)

weblog.add_renderer(SDApplycal, renderer.T2_4MDetailsSDApplycalRenderer(always_rerender=False), group_by=weblog.UNGROUPED)
weblog.add_renderer(HpcSDApplycal, renderer.T2_4MDetailsSDApplycalRenderer(always_rerender=False), group_by=weblog.UNGROUPED)
