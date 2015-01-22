from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
#import pipeline.infrastructure.renderer.basetemplates as basetemplates

from . bpsolint import BpSolint
from . bpsolint import BpSolintResults
from . import qa
from . import renderer

pipelineqa.registry.add_handler(qa.BpSolintQAHandler())
pipelineqa.registry.add_handler(qa.BpSolintListQAHandler())
qaadapter.registry.register_to_dataset_topic(BpSolintResults)

#weblog.add_renderer(BpSolint, basetemplates.T2_4MDetailsDefaultRenderer( \
#      description='Compute best bandpass solution interval'))
weblog.add_renderer(BpSolint, renderer.T2_4MDetailsBpSolintRenderer())



