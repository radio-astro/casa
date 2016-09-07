from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .applycal import SDMSApplycal
from pipeline.hif.tasks.applycal import qa
from pipeline.hif.tasks.applycal import ApplycalResults
from . import renderer

pipelineqa.registry.add_handler(qa.ApplycalQAHandler())
pipelineqa.registry.add_handler(qa.ApplycalListQAHandler())
qaadapter.registry.register_to_flagging_topic(ApplycalResults)

weblog.add_renderer(SDMSApplycal, renderer.T2_4MDetailsSDApplycalRenderer(always_rerender=False), group_by=weblog.UNGROUPED)
