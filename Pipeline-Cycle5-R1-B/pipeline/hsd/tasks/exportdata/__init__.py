from __future__ import absolute_import

import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as super_renderer

from .exportdata import SDExportData

# NOTE: QA handling is not necessary since SDExportData produces 
#       same results class as ExportData so that results are handled
#       in the same way as ExportData.

weblog.add_renderer(SDExportData, 
                    super_renderer.T2_4MDetailsDefaultRenderer(uri='exportdata.mako',
                                                               description='Prepare pipeline data products for export'),
                    group_by=weblog.UNGROUPED)
