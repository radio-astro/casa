from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .inspection import SDInspectData
from . import inspection

qaadapter.registry.register_to_dataset_topic(inspection.SDInspectDataResults)
