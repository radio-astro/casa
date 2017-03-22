from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog


from .editimlist import Editimlist
from . import editimlist

#from . import qa

#pipelineqa.registry.add_handler(qa.EditimlistQAHandler())
#pipelineqa.registry.add_handler(qa.EditimlistListQAHandler())
#qaadapter.registry.register_to_dataset_topic(editimlist.EditimlistResults)


weblog.add_renderer(Editimlist, basetemplates.T2_4MDetailsDefaultRenderer(uri='editimlist.mako',
                    description='Editimlist', always_rerender=False), group_by=weblog.UNGROUPED)