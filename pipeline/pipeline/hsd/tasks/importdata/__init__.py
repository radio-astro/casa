from __future__ import absolute_import
import pipeline.hif.tasks.importdata.renderer as super_renderer
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog

from .importdata import SDImportData as SDImportData
from . import qa
from . import renderer

pipelineqa.registry.add_handler(qa.SDImportDataQAHandler())
pipelineqa.registry.add_handler(qa.SDImportDataListQAHandler())

# use the standard ImportData renderer to render ALMAImportData results
weblog.add_renderer(SDImportData, super_renderer.T2_4MDetailsImportDataRenderer(uri='hsd_importdata.mako'), group_by=weblog.UNGROUPED)
