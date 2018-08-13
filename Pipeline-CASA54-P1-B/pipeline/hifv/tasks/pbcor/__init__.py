from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog


from .pbcor import Pbcor
from . import pbcor
from . import renderer

#from . import qa

#pipelineqa.registry.add_handler(qa.PbcorQAHandler())
#pipelineqa.registry.add_handler(qa.PbcorListQAHandler())
#qaadapter.registry.register_to_dataset_topic(pbcor.PbcorResults)


# weblog.add_renderer(Pbcor, basetemplates.T2_4MDetailsDefaultRenderer(uri='pbcor.mako',
#                          description='Pbcor'), group_by=weblog.UNGROUPED)

weblog.add_renderer(Pbcor, renderer.T2_4MDetailsMakepbcorimagesRenderer(uri='pbcor.mako',
                         description='Pbcor'), group_by=weblog.UNGROUPED)
