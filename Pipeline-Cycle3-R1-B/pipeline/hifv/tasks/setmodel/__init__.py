from __future__ import absolute_import
from pipeline.hif.tasks.common import commonfluxresults
import pipeline.hif.tasks.setmodel.renderer as super_renderer
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .vlasetjy import VLASetjy
from .setmodel import SetModel
from .fluxgains import Fluxgains
from . import qa

pipelineqa.registry.add_handler(qa.VLASetjyQAHandler())
pipelineqa.registry.add_handler(qa.VLASetjyListQAHandler())
qaadapter.registry.register_to_calibration_topic(commonfluxresults.FluxCalibrationResults)

weblog.add_renderer(VLASetjy, super_renderer.T2_4MDetailsSetjyRenderer())
