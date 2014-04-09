from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .bpflagchans import Bandpassflagchans
from . import resultobjects

qaadapter.registry.register_to_flagging_topic(resultobjects.BandpassflagResults)
