from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog

from .linpolcal import Linpolcal
from . import resultobjects

qaadapter.registry.register_to_calibration_topic(resultobjects.LinpolcalResult)

weblog.add_renderer(Linpolcal,
                    basetemplates.T2_4MDetailsDefaultRenderer(
                                                              description='Calibrate linear polarization'),
                    group_by='session')
