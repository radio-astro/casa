from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog

from .statwt import Statwt
from . import statwt


weblog.add_renderer(Statwt, basetemplates.T2_4MDetailsDefaultRenderer(uri='statwt.mako',
                                                                      description='Reweight visibilities', always_rerender=False))