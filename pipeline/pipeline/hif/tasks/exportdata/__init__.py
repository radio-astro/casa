from __future__ import absolute_import

import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as basetemplates

from . import exportdata
from .exportdata import ExportData
#from .exportdata import ExportDataResults
from . import qa

pipelineqa.registry.add_handler(qa.ExportDataQAHandler())
pipelineqa.registry.add_handler(qa.ExportDataListQAHandler())
qaadapter.registry.register_to_dataset_topic(exportdata.ExportDataResults)

weblog.add_renderer(ExportData, 
                    basetemplates.T2_4MDetailsDefaultRenderer(uri='exportdata.mako',
                                                              description='Prepare pipeline data products for export'),
                    group_by='ungrouped')                    
