from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.pipelineqa as pipelineqa

from .importdata import SDImportData
from . import qa
from . import importdata

pipelineqa.registry.add_handler(qa.SDImportDataQAHandler())
pipelineqa.registry.add_handler(qa.SDImportDataListQAHandler())
qaadapter.registry.register_to_dataset_topic(importdata.SDImportDataResults)
