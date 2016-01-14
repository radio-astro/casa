from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog

from .plotsummary import PlotSummary
from .import plotsummary
from . import renderer


'''
from .hanning import Hanning
from . import hanning


weblog.add_renderer(PlotSummary,  basetemplates.T2_4MDetailsDefaultRenderer(uri='plotsummary.mako',
                                      description='VLA Plot Summary'), group_by='ungrouped')
'''

weblog.add_renderer(PlotSummary, renderer.T2_4MDetailsplotsummaryRenderer(), group_by='ungrouped')

