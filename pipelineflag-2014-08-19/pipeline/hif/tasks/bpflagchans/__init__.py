from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .bpflagchans import Bandpassflagchans
from . import resultobjects

from . import qa

pipelineqa.registry.add_handler(qa.BandpassQAHandler())
pipelineqa.registry.add_handler(qa.BandpassListQAHandler())

qaadapter.registry.register_to_calibration_topic(resultobjects.BandpassflagResults)
