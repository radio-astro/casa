from __future__ import absolute_import

import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
from . import flagdeterbase
from . import qa
from .flagdatasetter import FlagdataSetter

pipelineqa.registry.add_handler(qa.FlagDeterBaseQAHandler())
pipelineqa.registry.add_handler(qa.FlagDeterBaseListQAHandler())
qaadapter.registry.register_to_flagging_topic(flagdeterbase.FlagDeterBaseResults)
