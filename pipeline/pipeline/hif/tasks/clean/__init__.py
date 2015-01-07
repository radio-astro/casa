from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .clean import Clean
from . import resultobjects
from . import renderer

qaadapter.registry.register_to_imaging_topic(resultobjects.BoxResult)
qaadapter.registry.register_to_imaging_topic(resultobjects.CleanResult)

weblog.add_renderer(Clean, renderer.T2_4MDetailsCleanRenderer())
