from __future__ import absolute_import

# import pipeline.hif.tasks.importdata.renderer as super_renderer
import pipeline.infrastructure.renderer.weblog as weblog
from . import qa
from . import renderer
from .importdata import SDImportData as SDImportData

# # use the standard ImportData renderer to render ALMAImportData results
# weblog.add_renderer(SDImportData, super_renderer.T2_4MDetailsImportDataRenderer(uri='hsd_importdata.mako'), group_by=weblog.UNGROUPED)
weblog.add_renderer(SDImportData, renderer.T2_4MDetailsSingleDishImportDataRenderer(uri='hsd_importdata.mako'), group_by=weblog.UNGROUPED)