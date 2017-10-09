from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .wvrgcalflag import Wvrgcalflag
from . import qa
from . import resultobjects
from . import renderer

pipelineqa.registry.add_handler(qa.WvrgcalflagQAHandler())
pipelineqa.registry.add_handler(qa.WvrgcalflagListQAHandler())
qaadapter.registry.register_to_calibration_topic(resultobjects.WvrgcalflagResults)

weblog.add_renderer(Wvrgcalflag, renderer.T2_4MDetailsWvrgcalflagRenderer(), group_by=weblog.UNGROUPED)
