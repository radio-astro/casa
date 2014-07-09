from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .calsky import SDCalSky
from . import calsky

qaadapter.registry.register_to_calibration_topic(calsky.SDCalSkyResults)
