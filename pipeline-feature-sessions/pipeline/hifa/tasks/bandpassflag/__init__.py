from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .bandpassflag import Bandpassflag
from . import resultobjects
from . import renderer
from . import qa

pipelineqa.registry.add_handler(qa.BandpassflagQAHandler())
pipelineqa.registry.add_handler(qa.BandpassflagListQAHandler())
qaadapter.registry.register_to_calibration_topic(resultobjects.BandpassflagResults)

weblog.add_renderer(Bandpassflag, renderer.T2_4MDetailsBandpassflagRenderer(), group_by=weblog.UNGROUPED)
