from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .rawflagchans import Rawflagchans
from . import resultobjects
from . import renderer

from pipeline.hif.tasks.rawflagchans import qa

pipelineqa.registry.add_handler(qa.RawflagchansQAHandler())
pipelineqa.registry.add_handler(qa.RawflagchansListQAHandler())
qaadapter.registry.register_to_flagging_topic(resultobjects.RawflagchansResults)

weblog.add_renderer(Rawflagchans, renderer.T2_4MDetailsRawflagchansRenderer())





