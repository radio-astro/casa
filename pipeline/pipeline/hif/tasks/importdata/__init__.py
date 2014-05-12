from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.pipelineqa as pipelineqa

from .importdata import ImportData
from . import qa
from . import importdata

pipelineqa.registry.add_handler(qa.ImportDataQAHandler())
pipelineqa.registry.add_handler(qa.ImportDataListQAHandler())
qaadapter.registry.register_to_dataset_topic(importdata.ImportDataResults)
