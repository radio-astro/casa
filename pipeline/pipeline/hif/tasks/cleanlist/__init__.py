from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .cleanlist import CleanList
from . import resultobjects

qaadapter.registry.register_to_imaging_topic(resultobjects.CleanListResult)

