from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .cleanlist import CleanList
from . import resultobjects
from ..clean import renderer as clean_renderer

from . import qa

pipelineqa.registry.add_handler(qa.CleanListQAHandler())
pipelineqa.registry.add_handler(qa.CleanListListQAHandler())
qaadapter.registry.register_to_imaging_topic(resultobjects.CleanListResult)

weblog.add_renderer(CleanList, clean_renderer.T2_4MDetailsCleanRenderer(description='Calculate clean products'))
