from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .tsyscal import Tsyscal
from . import resultobjects
from . import qa

pipelineqa.registry.add_handler(qa.TsyscalQAHandler())
pipelineqa.registry.add_handler(qa.TsyscalListQAHandler())
qaadapter.registry.register_to_calibration_topic(resultobjects.TsyscalResults)
