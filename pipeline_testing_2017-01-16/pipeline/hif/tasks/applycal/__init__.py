from __future__ import absolute_import

#import pipeline.infrastructure.pipelineqa as pipelineqa
#import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.h.tasks.applycal.renderer as super_renderer

from .ifapplycal import IFApplycal

weblog.add_renderer(IFApplycal, super_renderer.T2_4MDetailsApplycalRenderer(), group_by=weblog.UNGROUPED)
