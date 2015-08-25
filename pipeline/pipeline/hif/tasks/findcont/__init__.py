from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .findcont import FindCont
from . import renderer
from . import resultobjects
from . import qa

pipelineqa.registry.add_handler(qa.FindContQAHandler())
pipelineqa.registry.add_handler(qa.FindContListQAHandler())
qaadapter.registry.register_to_imaging_topic(resultobjects.FindContResult)

weblog.add_renderer(FindCont, renderer.T2_4MDetailsFindContRenderer(), group_by='ungrouped')
