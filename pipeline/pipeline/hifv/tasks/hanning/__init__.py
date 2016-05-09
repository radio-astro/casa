from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog


from .hanning import Hanning
from . import hanning

from . import qa

pipelineqa.registry.add_handler(qa.HanningQAHandler())
pipelineqa.registry.add_handler(qa.HanningListQAHandler())
qaadapter.registry.register_to_dataset_topic(hanning.HanningResults)
#qaadapter.registry.register_to_dataset_topic(applycal.ApplycalResults)

weblog.add_renderer(Hanning,
                    basetemplates.T2_4MDetailsDefaultRenderer(uri='hanning.mako',
                                                              description='VLA Hanning Smoothing'),
                    group_by='ungrouped')
