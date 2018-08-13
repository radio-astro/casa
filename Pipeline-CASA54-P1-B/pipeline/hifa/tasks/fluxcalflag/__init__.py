from __future__ import absolute_import

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import fluxcalflag
from . import qa
from .fluxcalflag import FluxcalFlag

qaadapter.registry.register_to_flagging_topic(fluxcalflag.FluxcalFlagResults)

weblog.add_renderer(FluxcalFlag, 
                    basetemplates.T2_4MDetailsDefaultRenderer(uri='fluxcalflag.mako',
                                                              description='Flag spectral features in solar system flux calibrators'),
                    group_by=weblog.UNGROUPED)