from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog


from .transformimagedata import Transformimagedata
from . import transformimagedata

#from . import qa

#pipelineqa.registry.add_handler(qa.TransformimagedataQAHandler())
#pipelineqa.registry.add_handler(qa.TransformimagedataListQAHandler())
#qaadapter.registry.register_to_dataset_topic(transformimagedata.TransformimagedataResults)


weblog.add_renderer(Transformimagedata, basetemplates.T2_4MDetailsDefaultRenderer(uri='transformimagedata.mako',
                         description='Transformimagedata'), group_by=weblog.UNGROUPED)