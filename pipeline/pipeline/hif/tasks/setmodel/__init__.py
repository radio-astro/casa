from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .setjy import Setjy
from .setmodel import SetModel
from ..common import commonfluxresults
from . import qa

pipelineqa.registry.add_handler(qa.SetjyQAHandler())
pipelineqa.registry.add_handler(qa.SetjyListQAHandler())
qaadapter.registry.register_to_calibration_topic(commonfluxresults.FluxCalibrationResults)
