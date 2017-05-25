from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog


from .makermsimages import Makermsimages
from . import makermsimages

#from . import qa

#pipelineqa.registry.add_handler(qa.MakermsimagesQAHandler())
#pipelineqa.registry.add_handler(qa.MakermsimagesListQAHandler())
#qaadapter.registry.register_to_dataset_topic(makermsimages.MakermsimagesResults)


weblog.add_renderer(Makermsimages, basetemplates.T2_4MDetailsDefaultRenderer(uri='makermsimages.mako',
                         description='Makermsimages'), group_by=weblog.UNGROUPED)