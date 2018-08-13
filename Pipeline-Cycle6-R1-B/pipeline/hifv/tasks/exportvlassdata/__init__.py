from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog


from .exportvlassdata import Exportvlassdata
from . import exportvlassdata

#from . import qa

#pipelineqa.registry.add_handler(qa.ExportvlassdataQAHandler())
#pipelineqa.registry.add_handler(qa.ExportvlassdataListQAHandler())
#qaadapter.registry.register_to_dataset_topic(exportvlassdata.ExportvlassdataResults)


weblog.add_renderer(Exportvlassdata, basetemplates.T2_4MDetailsDefaultRenderer(uri='exportvlassdata.mako',
                         description='Exportvlassdata'), group_by=weblog.UNGROUPED)