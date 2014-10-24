from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .imaging import SDImaging
from . import imaging
from . import renderer

qaadapter.registry.register_to_imaging_topic(imaging.SDImagingResults)

weblog.add_renderer(SDImaging, renderer.T2_4MDetailsSingleDishImagingRenderer())
