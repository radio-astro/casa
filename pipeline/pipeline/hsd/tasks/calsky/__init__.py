from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .calsky import SDCalSky
from . import calsky
from . import renderer

qaadapter.registry.register_to_calibration_topic(calsky.SDCalSkyResults)

weblog.add_renderer(SDCalSky, renderer.T2_4MDetailsSingleDishCalSkyRenderer())
