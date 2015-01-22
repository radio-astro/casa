from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .importdata import ImportData
from . import importdata
from . import qa
from . import renderer

pipelineqa.registry.add_handler(qa.ImportDataQAHandler())
pipelineqa.registry.add_handler(qa.ImportDataListQAHandler())
qaadapter.registry.register_to_dataset_topic(importdata.ImportDataResults)

weblog.add_renderer(ImportData, renderer.T2_4MDetailsImportDataRenderer())
