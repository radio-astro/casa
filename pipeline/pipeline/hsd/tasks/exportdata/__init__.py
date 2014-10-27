from __future__ import absolute_import

import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as super_renderer

from .exportdata import SDExportData

weblog.add_renderer(SDExportData, 
                    super_renderer.T2_4MDetailsDefaultRenderer(description='Export Pipeline Products', 
                                                               always_rerender=False))
