from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as basetemplates

from .wvrgcal import Wvrgcal
from . import resultobjects

qaadapter.registry.register_to_calibration_topic(resultobjects.WvrgcalResult)

weblog.add_renderer(Wvrgcal, basetemplates.T2_4MDetailsDefaultRenderer(uri='wvrgcal.mako',
                                                                       description='Calculate WVR calibration'))
