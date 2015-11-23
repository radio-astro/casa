from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog

from .plotsummary import PlotSummary
from .import plotsummary


'''
from .hanning import Hanning
from . import hanning
'''

weblog.add_renderer(PlotSummary,
                    basetemplates.T2_4MDetailsDefaultRenderer(uri='plotsummary.mako',
                                                              description='VLA Plot Summary'),
                    group_by='ungrouped')

