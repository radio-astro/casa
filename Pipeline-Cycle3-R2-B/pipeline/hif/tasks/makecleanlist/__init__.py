from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .makecleanlist import MakeCleanList
from . import resultobjects
from . import qa

pipelineqa.registry.add_handler(qa.MakeCleanListQAHandler())
pipelineqa.registry.add_handler(qa.MakeCleanListListQAHandler())
qaadapter.registry.register_to_imaging_topic(resultobjects.MakeCleanListResult)

weblog.add_renderer(MakeCleanList, 
                    basetemplates.T2_4MDetailsDefaultRenderer(uri='makecleanlist.mako',
                                                              description='Compile a list of cleaned images to be calculated'),
                    group_by='ungrouped')
