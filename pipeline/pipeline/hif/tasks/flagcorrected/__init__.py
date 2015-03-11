from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .flagcorrected import Flagcorrected
from . import resultobjects
from . import renderer

from pipeline.hif.tasks.flagcorrected import qa

pipelineqa.registry.add_handler(qa.FlagcorrectedQAHandler())
pipelineqa.registry.add_handler(qa.FlagcorrectedListQAHandler())
qaadapter.registry.register_to_flagging_topic(resultobjects.FlagcorrectedResults)

weblog.add_renderer(Flagcorrected, renderer.T2_4MDetailsFlagcorrectedRenderer())





