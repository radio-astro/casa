from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .inspection import SDInspectData
from . import inspection
from . import renderer

qaadapter.registry.register_to_dataset_topic(inspection.SDInspectDataResults)

weblog.add_renderer(SDInspectData, renderer.T2_4MDetailsSingleDishInspectDataRenderer())
