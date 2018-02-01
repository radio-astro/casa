from __future__ import absolute_import

# import pipeline.hif.tasks.importdata.renderer as super_renderer
import pipeline.h.tasks.importdata.renderer as super_renderer
import pipeline.infrastructure.renderer.weblog as weblog
from . import qa
from .almaimportdata import ALMAImportData

# use the standard ImportData renderer to render ALMAImportData results
weblog.add_renderer(ALMAImportData, super_renderer.T2_4MDetailsImportDataRenderer(), group_by=weblog.UNGROUPED)
