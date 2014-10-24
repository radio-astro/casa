from __future__ import absolute_import
import pipeline.infrastructure.renderer.weblog as weblog

from .reduction import SDReduction
import pipeline.infrastructure.renderer.basetemplates as super_renderer

weblog.add_renderer(SDReduction, super_renderer.T2_4MDetailsDefaultRenderer(uri='hsd_reduce.mako'))
