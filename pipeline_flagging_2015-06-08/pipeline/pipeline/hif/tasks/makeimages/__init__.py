from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .makeimages import MakeImages
from . import resultobjects
from ..tclean import renderer as clean_renderer

from . import qa

pipelineqa.registry.add_handler(qa.MakeImagesQAHandler())
pipelineqa.registry.add_handler(qa.MakeImagesListQAHandler())
qaadapter.registry.register_to_imaging_topic(resultobjects.MakeImagesResult)

weblog.add_renderer(MakeImages, 
                    clean_renderer.T2_4MDetailsTcleanRenderer(description='Calculate clean products'),
                    group_by='ungrouped')
