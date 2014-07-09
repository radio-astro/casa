from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
from pipeline.hif.tasks.common import commonfluxresults


from .vlasetjy import VLASetjy
from .setmodel import SetModel
from .fluxgains import Fluxgains
from . import qa

pipelineqa.registry.add_handler(qa.VLASetjyQAHandler())
pipelineqa.registry.add_handler(qa.VLASetjyListQAHandler())
qaadapter.registry.register_to_calibration_topic(commonfluxresults.FluxCalibrationResults)