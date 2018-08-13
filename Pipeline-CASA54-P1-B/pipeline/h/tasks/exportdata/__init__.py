from __future__ import absolute_import

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import exportdata
from . import qa
from .exportdata import ExportData

qaadapter.registry.register_to_dataset_topic(exportdata.ExportDataResults)

weblog.add_renderer(ExportData, 
                    basetemplates.T2_4MDetailsDefaultRenderer(uri='exportdata.mako',
                                                              description='Prepare pipeline data products for export'),
                    group_by=weblog.UNGROUPED)
