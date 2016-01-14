from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .almaphcorbandpass import ALMAPhcorBandpass
from . import renderer
from pipeline.hif.tasks.bandpass import qa
from pipeline.hif.tasks.bandpass import common

pipelineqa.registry.add_handler(qa.BandpassQAHandler())
pipelineqa.registry.add_handler(qa.BandpassListQAHandler())
qaadapter.registry.register_to_calibration_topic(common.BandpassResults)

weblog.add_renderer(ALMAPhcorBandpass, renderer.T2_4MDetailsBandpassRenderer(), group_by=weblog.UNGROUPED)
