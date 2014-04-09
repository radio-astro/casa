from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .applycal import Applycal
from . import qa
from . import applycal

pipelineqa.registry.add_handler(qa.ApplycalQAHandler())
pipelineqa.registry.add_handler(qa.ApplycalListQAHandler())
qaadapter.registry.register_to_flagging_topic(applycal.ApplycalResults)
