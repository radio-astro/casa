from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .caltsys import SDCalTsys
from . import caltsys

qaadapter.registry.register_to_calibration_topic(caltsys.SDCalTsysResults)
