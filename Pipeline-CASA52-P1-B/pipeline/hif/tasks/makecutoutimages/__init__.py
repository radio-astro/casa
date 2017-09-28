from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog


from .makecutoutimages import Makecutoutimages
from . import makecutoutimages
from . import renderer

#from . import qa

#pipelineqa.registry.add_handler(qa.MakecutoutimagesQAHandler())
#pipelineqa.registry.add_handler(qa.MakecutoutimagesListQAHandler())
#qaadapter.registry.register_to_dataset_topic(makecutoutimages.MakecutoutimagesResults)


weblog.add_renderer(Makecutoutimages, renderer.T2_4MDetailsMakecutoutimagesRenderer(uri='makecutoutimages.mako',
                         description='Makecutoutimages'), group_by=weblog.UNGROUPED)