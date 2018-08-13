from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import mssplit
from . import qa
from . import renderer
from .mssplit import MsSplit

qaadapter.registry.register_to_dataset_topic(mssplit.MsSplitResults)

weblog.add_renderer(MsSplit, renderer.T2_4MDetailsMsSplitRenderer(),
    group_by=weblog.UNGROUPED)
