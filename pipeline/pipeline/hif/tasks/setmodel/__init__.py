from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .setjy import Setjy
from .setmodel import SetModels
from ..common import commonfluxresults
from . import qa
from . import renderer

pipelineqa.registry.add_handler(qa.SetjyQAHandler())
pipelineqa.registry.add_handler(qa.SetjyListQAHandler())
pipelineqa.registry.add_handler(qa.SetModelsQAHandler())
pipelineqa.registry.add_handler(qa.SetModelsListQAHandler())
qaadapter.registry.register_to_calibration_topic(commonfluxresults.FluxCalibrationResults)

weblog.add_renderer(Setjy, renderer.T2_4MDetailsSetjyRenderer(), group_by=weblog.UNGROUPED)
weblog.add_renderer(SetModels, renderer.T2_4MDetailsSetjyRenderer(), group_by=weblog.UNGROUPED)
