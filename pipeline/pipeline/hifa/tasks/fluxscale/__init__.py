from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from . gcorfluxscale import GcorFluxscale
from pipeline.hif.tasks.common import commonfluxresults
from . import qa

pipelineqa.registry.add_handler(qa.GcorFluxscaleQAHandler())
pipelineqa.registry.add_handler(qa.GcorFluxscaleListQAHandler())
qaadapter.registry.register_to_calibration_topic(commonfluxresults.FluxCalibrationResults)


