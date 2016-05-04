from __future__ import absolute_import

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from . import uvcontfit
from .uvcontfit import UVcontFit

from . import uvcontsub
from .uvcontsub import UVcontSub
from ..applycal import applycal

from . import qa
from . import renderer

pipelineqa.registry.add_handler(qa.UVcontFitQAHandler())
pipelineqa.registry.add_handler(qa.UVcontFitListQAHandler())
qaadapter.registry.register_to_dataset_topic(uvcontfit.UVcontFitResults)
qaadapter.registry.register_to_dataset_topic(applycal.ApplycalResults)

#weblog.add_renderer(UVcontFit, basetemplates.T2_4MDetailsDefaultRenderer(description='Continuum fit the TARGET data'), group_by=weblog.UNGROUPED)
weblog.add_renderer(UVcontFit, renderer.T2_4MDetailsUVcontFitRenderer(), group_by=weblog.UNGROUPED)
weblog.add_renderer(UVcontSub, basetemplates.T2_4MDetailsDefaultRenderer(description='Continuum subtract the TARGET data'), group_by=weblog.UNGROUPED)
