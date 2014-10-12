from __future__ import absolute_import
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as basetemplates

from . bpsolint import BpSolint

weblog.add_renderer(BpSolint, basetemplates.T2_4MDetailsDefaultRenderer( \
      description='Compute best bandpass solution interval'))



