from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog

from .importdata import SDImportData
from . import qa
from . import importdata
from . import renderer

pipelineqa.registry.add_handler(qa.SDImportDataQAHandler())
pipelineqa.registry.add_handler(qa.SDImportDataListQAHandler())
qaadapter.registry.register_to_dataset_topic(importdata.SDImportDataResults)

weblog.add_renderer(SDImportData, renderer.T2_4MDetailsSingleDishImportDataRenderer())
