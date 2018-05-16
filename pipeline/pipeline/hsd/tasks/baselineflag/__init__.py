from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import baselineflag
from . import qa
from . import renderer
from .baselineflag import SerialSDBLFlag, HpcSDBLFlag
#### A switch to define which task to use
SDBLFlag = HpcSDBLFlag # SerialSDBLFlag or HpcSDBLFlag


qaadapter.registry.register_to_flagging_topic(baselineflag.SDBLFlagResults)

weblog.add_renderer(SerialSDBLFlag, renderer.T2_4MDetailsBLFlagRenderer(always_rerender=False), group_by=weblog.UNGROUPED)
weblog.add_renderer(HpcSDBLFlag, renderer.T2_4MDetailsBLFlagRenderer(always_rerender=False), group_by=weblog.UNGROUPED)
