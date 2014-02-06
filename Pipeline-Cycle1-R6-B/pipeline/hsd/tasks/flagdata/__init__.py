from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa

from .flagdata import SDFlagData
from . import qa

pipelineqa.registry.add_handler(qa.SDFlagDataQAHandler())
pipelineqa.registry.add_handler(qa.SDFlagDataListQAHandler())
