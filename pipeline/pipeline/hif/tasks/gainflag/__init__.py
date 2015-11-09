from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .gainflag import Gainflag
from . import resultobjects
from . import renderer

from pipeline.hif.tasks.gainflag import qa

pipelineqa.registry.add_handler(qa.GainflagQAHandler())
pipelineqa.registry.add_handler(qa.GainflagListQAHandler())
qaadapter.registry.register_to_flagging_topic(resultobjects.GainflagResults)

weblog.add_renderer(Gainflag, renderer.T2_4MDetailsGainflagRenderer(), group_by='session')
