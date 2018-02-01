from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .${modulename.lower()} import ${taskname.capitalize()}
from . import ${modulename.lower()}

#qaadapter.registry.register_to_dataset_topic(${modulename.lower()}.${taskname.capitalize()}Results)

weblog.add_renderer(${taskname.capitalize()}, basetemplates.T2_4MDetailsDefaultRenderer(uri='${taskname.lower()}.mako',
                         description='${taskname.capitalize()}'), group_by=weblog.UNGROUPED)