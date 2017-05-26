from __future__ import absolute_import

import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.h.tasks.applycal.renderer as super_renderer

from .ifapplycal import IFApplycal, IFApplycalInputs, HpcIFApplycal, HpcIFApplycalInputs

weblog.add_renderer(IFApplycal, super_renderer.T2_4MDetailsApplycalRenderer(), group_by=weblog.UNGROUPED)
weblog.add_renderer(HpcIFApplycal, super_renderer.T2_4MDetailsApplycalRenderer(), group_by=weblog.UNGROUPED)
