from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .testBPdcals import testBPdcals
from .testBPdcals import testBPdcalsResults
from . import renderer

from . import qa

pipelineqa.registry.add_handler(qa.testBPdcalsQAHandler())
pipelineqa.registry.add_handler(qa.testBPdcalsListQAHandler())
qaadapter.registry.register_to_dataset_topic(testBPdcalsResults)

weblog.add_renderer(testBPdcals, renderer.T2_4MDetailstestBPdcalsRenderer(), group_by='ungrouped')