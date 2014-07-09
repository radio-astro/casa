from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.pipelineqa as pipelineqa

from .flagdata import SDFlagData
from . import qa
from . import flagdata

pipelineqa.registry.add_handler(qa.SDFlagDataQAHandler())
pipelineqa.registry.add_handler(qa.SDFlagDataListQAHandler())
qaadapter.registry.register_to_flagging_topic(flagdata.SDFlagDataResults)
