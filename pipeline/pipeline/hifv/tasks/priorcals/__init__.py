from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .priorcals import Priorcals
from . import priorcals
from . import renderer
from . import resultobjects

from . import qa

pipelineqa.registry.add_handler(qa.PriorcalsQAHandler())
pipelineqa.registry.add_handler(qa.PriorcalsListQAHandler())
qaadapter.registry.register_to_dataset_topic(resultobjects.PriorcalsResults)

weblog.add_renderer(Priorcals, renderer.T2_4MDetailspriorcalsRenderer(), group_by='ungrouped')