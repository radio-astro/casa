from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .bpflagchans import Bandpassflagchans
from . import renderer
from . import resultobjects
from . import qa

pipelineqa.registry.add_handler(qa.BandpassFlagChansQAHandler())
pipelineqa.registry.add_handler(qa.BandpassFlagChansListQAHandler())

qaadapter.registry.register_to_flagging_topic(
  resultobjects.BandpassflagResults)

weblog.add_renderer(Bandpassflagchans, renderer.T2_4MDetailsBandpassFlagRenderer())
