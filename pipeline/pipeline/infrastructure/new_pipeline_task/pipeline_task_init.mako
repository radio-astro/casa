from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog


from .${taskname.lower()} import ${taskname.capitalize()}
from . import ${taskname.lower()}

#from . import qa

#pipelineqa.registry.add_handler(qa.${taskname.capitalize()}QAHandler())
#pipelineqa.registry.add_handler(qa.${taskname.capitalize()}ListQAHandler())
#qaadapter.registry.register_to_dataset_topic(${taskname.lower()}.${taskname.capitalize()}Results)


weblog.add_renderer(${taskname.capitalize()}, basetemplates.T2_4MDetailsDefaultRenderer(uri='${taskname.lower()}.mako',
                         description='${taskname.capitalize()}'), group_by=weblog.UNGROUPED)