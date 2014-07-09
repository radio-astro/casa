from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .applycal import SDApplyCal
from . import applycal

qaadapter.registry.register_to_calibration_topic(applycal.SDApplyCalResults)
