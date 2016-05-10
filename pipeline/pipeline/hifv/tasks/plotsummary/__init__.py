from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog

from .plotsummary import PlotSummary
from .import plotsummary
from . import renderer

from . import qa

pipelineqa.registry.add_handler(qa.PlotSummaryQAHandler())
pipelineqa.registry.add_handler(qa.PlotSummaryListQAHandler())
qaadapter.registry.register_to_dataset_topic(plotsummary.PlotSummaryResults)


'''
from .hanning import Hanning
from . import hanning


weblog.add_renderer(PlotSummary,  basetemplates.T2_4MDetailsDefaultRenderer(uri='plotsummary.mako',
                                      description='VLA Plot Summary'), group_by='ungrouped')
'''

weblog.add_renderer(PlotSummary, renderer.T2_4MDetailsplotsummaryRenderer(), group_by='ungrouped')

