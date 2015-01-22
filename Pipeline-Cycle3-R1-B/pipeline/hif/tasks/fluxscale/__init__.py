from __future__ import absolute_import

from .fluxcal import Fluxcal
from .fluxscale import Fluxscale
from .normflux import NormaliseFlux

import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as basetemplates

weblog.add_renderer(NormaliseFlux, basetemplates.T2_4MDetailsDefaultRenderer(uri='normflux.mako',
               description='Normalise flux scales accross ASDMs'))

weblog.add_renderer(Fluxscale, basetemplates.T2_4MDetailsDefaultRenderer(uri='fluxscale.mako',
               description='Compute calibrator source fluxes'))
