from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .importdata import VLAImportData
from . import importdata
from . import renderer

from . import qa


pipelineqa.registry.add_handler(qa.VLAImportDataQAHandler())
pipelineqa.registry.add_handler(qa.VLAImportDataListQAHandler())
qaadapter.registry.register_to_dataset_topic(importdata.VLAImportDataResults)


weblog.add_renderer(VLAImportData, renderer.T2_4MDetailsVLAImportDataRenderer(), group_by='ungrouped')