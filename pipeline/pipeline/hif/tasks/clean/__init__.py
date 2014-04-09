from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .clean import Clean
from . import resultobjects

qaadapter.registry.register_to_imaging_topic(resultobjects.BoxResult)
qaadapter.registry.register_to_imaging_topic(resultobjects.CleanResult)
