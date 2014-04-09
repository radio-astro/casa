from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from . import commonfluxresults

qaadapter.registry.register_to_calibration_topic(commonfluxresults.FluxCalibrationResults)
