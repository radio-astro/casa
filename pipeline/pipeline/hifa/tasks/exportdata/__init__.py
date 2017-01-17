from __future__ import absolute_import

import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as basetemplates

from .almaexportdata import ALMAExportData

# Use the standard ExportData renderer to render ALMAExportData results
weblog.add_renderer(ALMAExportData,
    basetemplates.T2_4MDetailsDefaultRenderer(uri='exportdata.mako',
    description='Prepare pipeline data products for export'),
    group_by=weblog.UNGROUPED)


