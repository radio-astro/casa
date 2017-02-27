from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .checkproductsize import CheckProductSize
from . import renderer
from . import resultobjects
from . import qa

pipelineqa.registry.add_handler(qa.CheckProductSizeQAHandler())
pipelineqa.registry.add_handler(qa.CheckProductSizeListQAHandler())
qaadapter.registry.register_to_imaging_topic(resultobjects.CheckProductSizeResult)

weblog.add_renderer(CheckProductSize, renderer.T2_4MDetailsCheckProductSizeRenderer(), group_by=weblog.UNGROUPED)
