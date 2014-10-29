from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .timegaincal import TimeGaincal

from . import qa
from pipeline.hif.tasks.gaincal import renderer

pipelineqa.registry.add_handler(qa.TimegaincalQAHandler())
pipelineqa.registry.add_handler(qa.TimegaincalListQAHandler())

weblog.add_renderer(TimeGaincal, renderer.T2_4MDetailsGaincalRenderer())
