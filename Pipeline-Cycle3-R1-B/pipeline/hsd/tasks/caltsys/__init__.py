from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .caltsys import SDCalTsys
from . import caltsys
from . import renderer

qaadapter.registry.register_to_calibration_topic(caltsys.SDCalTsysResults)

weblog.add_renderer(SDCalTsys, renderer.T2_4MDetailsSingleDishCalTsysRenderer())