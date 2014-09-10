from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .spwphaseup import SpwPhaseup
from . import qa

pipelineqa.registry.add_handler(qa.SpwPhaseupQAHandler())
pipelineqa.registry.add_handler(qa.SpwPhaseupListQAHandler())
qaadapter.registry.register_to_calibration_topic(spwphaseup.SpwPhaseupResults)


