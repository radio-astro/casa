from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog


from .hanning import Hanning
from . import hanning


weblog.add_renderer(Hanning, basetemplates.T2_4MDetailsDefaultRenderer(uri='hanning.mako',
                                                                      description='VLA Hanning Smoothing', always_rerender=False))
