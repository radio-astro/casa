from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .imaging import SDImaging
from . import imaging

qaadapter.registry.register_to_imaging_topic(imaging.SDImagingResults)
