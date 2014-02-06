from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa

from .applycal import Applycal
from . import qa

pipelineqa.registry.add_handler(qa.ApplycalQAHandler())
pipelineqa.registry.add_handler(qa.ApplycalListQAHandler())
