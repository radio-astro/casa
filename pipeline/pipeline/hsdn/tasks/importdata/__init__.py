from __future__ import absolute_import

import pipeline.hsd.tasks.importdata.renderer as super_renderer
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog

from .importdata import NROImportData as NROImportData
# from . import qa
# from . import renderer
# 
# pipelineqa.registry.add_handler(qa.NROImportDataQAHandler())
# pipelineqa.registry.add_handler(qa.NROImportDataListQAHandler())

# # use the standard ImportData renderer to render ALMAImportData results
weblog.add_renderer(NROImportData, super_renderer.T2_4MDetailsSingleDishImportDataRenderer(uri='hsd_importdata.mako'), group_by=weblog.UNGROUPED)