from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog

from .statwt import Statwt
from . import statwt

from . import qa

pipelineqa.registry.add_handler(qa.StatwtQAHandler())
pipelineqa.registry.add_handler(qa.StatwtListQAHandler())
qaadapter.registry.register_to_dataset_topic(statwt.StatwtResults)


weblog.add_renderer(Statwt, 
                    basetemplates.T2_4MDetailsDefaultRenderer(uri='statwt.mako',
                                                              description='Reweight visibilities'),
                    group_by='ungrouped')
