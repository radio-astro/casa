from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .gaincalworker import GaincalWorker
from .gtypegaincal import GTypeGaincal
from .ktypegaincal import KTypeGaincal
from .gsplinegaincal import GSplineGaincal
from .gaincalmode import GaincalMode
#from .timegaincal import TimeGaincal
#from .phaseupgaincal import PhaseUpGaincal

from . import common

qaadapter.registry.register_to_calibration_topic(common.GaincalResults)
