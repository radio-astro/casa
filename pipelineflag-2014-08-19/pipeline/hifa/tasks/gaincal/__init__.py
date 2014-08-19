from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .timegaincal import TimeGaincal

from . import qa

pipelineqa.registry.add_handler(qa.TimegaincalQAHandler())
pipelineqa.registry.add_handler(qa.TimegaincalListQAHandler())
