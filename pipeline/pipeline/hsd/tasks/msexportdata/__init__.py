from __future__ import absolute_import

import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as super_renderer

from .exportdata import SDMSExportData

weblog.add_renderer(SDMSExportData, 
                    super_renderer.T2_4MDetailsDefaultRenderer(uri='hsd_exportdata.mako',
                                                               description='Prepare pipeline data products for export'),
                    group_by='ungrouped')                 
