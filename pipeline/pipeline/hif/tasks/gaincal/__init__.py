from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .gaincalworker import GaincalWorker
from .gtypegaincal import GTypeGaincal
from .ktypegaincal import KTypeGaincal
from .gsplinegaincal import GSplineGaincal
from .gaincalmode import GaincalMode

from . import common
from . import renderer

qaadapter.registry.register_to_calibration_topic(common.GaincalResults)

weblog.add_renderer(GTypeGaincal, renderer.T2_4MDetailsGaincalRenderer())
weblog.add_renderer(KTypeGaincal, renderer.T2_4MDetailsGaincalRenderer())
weblog.add_renderer(GSplineGaincal, renderer.T2_4MDetailsGaincalRenderer())
weblog.add_renderer(GaincalMode, renderer.T2_4MDetailsGaincalRenderer())
