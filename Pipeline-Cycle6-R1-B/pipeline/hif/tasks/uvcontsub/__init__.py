from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
# from ..applycal import applycal
from pipeline.h.tasks.applycal import applycal
from . import qa
from . import renderer
from . import uvcontfit
from . import uvcontsub
from .uvcontfit import UVcontFit
from .uvcontsub import UVcontSub

qaadapter.registry.register_to_dataset_topic(uvcontfit.UVcontFitResults)
qaadapter.registry.register_to_dataset_topic(applycal.ApplycalResults)

weblog.add_renderer(UVcontFit, renderer.T2_4MDetailsUVcontFitRenderer(), group_by=weblog.UNGROUPED)
weblog.add_renderer(UVcontSub, renderer.T2_4MDetailsUVcontSubRenderer(), group_by=weblog.UNGROUPED)
#weblog.add_renderer(UVcontSub, basetemplates.T2_4MDetailsDefaultRenderer(description='Continuum subtract the TARGET data'), group_by=weblog.UNGROUPED)
