from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .atmflag import Atmflag
from . import resultobjects

qaadapter.registry.register_to_flagging_topic(resultobjects.AtmflagResults)
