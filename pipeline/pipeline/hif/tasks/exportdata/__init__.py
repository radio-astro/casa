from __future__ import absolute_import
from .exportdata import ExportData
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as basetemplates

weblog.add_renderer(ExportData, basetemplates.T2_4MDetailsDefaultRenderer(uri='exportdata.mako',
               description='Prepare pipeline data products for export'))
