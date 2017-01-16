from __future__ import absolute_import

#import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as basetemplates
#import pipeline.h.tasks.exportdata.renderer as super_renderer

from .almaexportdata import ALMAExportData

# Add QA handers if necesssary
#from . import qa
#
#pipelineqa.registry.add_handler(qa.ALMAExportDataQAHandler())

# Use the standard ExportData renderer to render ALMAExportData results
#weblog.add_renderer(ALMAExportData, super_renderer.T2_4MDetailsExportDataRenderer(), group_by=weblog.UNGROUPED)
weblog.add_renderer(ALMAExportData,
    basetemplates.T2_4MDetailsDefaultRenderer(uri='exportdata.mako',
    description='Prepare pipeline data products for export'),
    group_by=weblog.UNGROUPED)


