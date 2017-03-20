from __future__ import absolute_import

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from . import mssplit
from .mssplit import MsSplit
from . import qa
from . import renderer

pipelineqa.registry.add_handler(qa.MsSplitQAHandler())
pipelineqa.registry.add_handler(qa.MsSplitListQAHandler())
qaadapter.registry.register_to_dataset_topic(mssplit.MsSplitResults)

weblog.add_renderer(MsSplit, renderer.T2_4MDetailsMsSplitRenderer(),
    group_by=weblog.UNGROUPED)
