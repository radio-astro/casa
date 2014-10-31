from __future__ import absolute_import
import pipeline.infrastructure.renderer.weblog as weblog

from .importdata import VLAImportData
from . import importdata
from . import renderer

weblog.add_renderer(VLAImportData, renderer.T2_4MDetailsVLAImportDataRenderer())