from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .imaging2 import SDImaging2 as SDImaging
from . import imaging2

qaadapter.registry.register_to_imaging_topic(imaging2.SDImaging2Results)
