from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .semiFinalBPdcals import semiFinalBPdcals
from .semiFinalBPdcals import semiFinalBPdcalsResults

from . import renderer

from . import qa

pipelineqa.registry.add_handler(qa.semiFinalBPdcalsQAHandler())
pipelineqa.registry.add_handler(qa.semiFinalBPdcalsListQAHandler())
qaadapter.registry.register_to_dataset_topic(semiFinalBPdcalsResults)

weblog.add_renderer(semiFinalBPdcals, renderer.T2_4MDetailssemifinalBPdcalsRenderer(), group_by='ungrouped')