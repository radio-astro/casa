from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import baselineflag
from . import qa
from . import renderer
from .baselineflag import SDBLFlag

qaadapter.registry.register_to_flagging_topic(baselineflag.SDBLFlagResults)

weblog.add_renderer(SDBLFlag, renderer.T2_4MDetailsBLFlagRenderer(), group_by=weblog.UNGROUPED)

# weblog.add_renderer(SDBLFlag, 
#                     super_renderer.T2_4MDetailsDefaultRenderer(uri='hsd_blflag.mako', 
#                                                                description='Flag data by Tsys, weather, and statistics of spectra',
#                                                                always_rerender=False),
#                     group_by=weblog.UNGROUPED)
#### No weblog
# import pipeline.infrastructure.renderer.basetemplates as basetemplates
# weblog.add_renderer(SDBLFlag, basetemplates.T2_4MDetailsDefaultRenderer(always_rerender=True), group_by=weblog.UNGROUPED)
