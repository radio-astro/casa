from __future__ import absolute_import

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from . import uvcontfit
from .uvcontfit import UVcontFit
#from . import qa
#from . import renderer

#pipelineqa.registry.add_handler(qa.MsTransformQAHandler())
#pipelineqa.registry.add_handler(qa.MstransformListQAHandler())
qaadapter.registry.register_to_dataset_topic(uvcontfit.UVcontFitResults)

weblog.add_renderer(UVcontFit, basetemplates.T2_4MDetailsDefaultRenderer(description='Continuum subtract the TARGET source MS'), group_by=weblog.UNGROUPED)