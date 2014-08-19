from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .rawflagchans import Rawflagchans
from . import resultobjects

qaadapter.registry.register_to_flagging_topic(resultobjects.RawflagchansResults)
