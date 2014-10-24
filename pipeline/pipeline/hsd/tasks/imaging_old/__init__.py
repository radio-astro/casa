from __future__ import absolute_import
import pipeline.infrastructure.renderer.weblog as weblog

from .imaging import SDImaging as SDImagingOld
from ..imaging import renderer as super_renderer

weblog.add_renderer(SDImagingOld, super_renderer.T2_4MDetailsSingleDishImagingRenderer())
