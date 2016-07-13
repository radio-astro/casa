from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as super_renderer

from . import baselineflag
from .baselineflag import SDBLFlag as SDMSBLFlag
from . import qa
### Temporary exported for ASAP procedure
from . import SDFlagPlotter
from . import SDFlagRule
###
pipelineqa.registry.add_handler(qa.SDBLFlagListQAHandler())
qaadapter.registry.register_to_flagging_topic(baselineflag.SDBLFlagResults)

weblog.add_renderer(SDMSBLFlag, 
                    super_renderer.T2_4MDetailsDefaultRenderer(uri='hsd_blflag.mako', 
                                                               description='Flag data by Tsys, weather, and statistics of spectra',
                                                               always_rerender=False),
                    group_by='ungrouped')
#### No weblog
# import pipeline.infrastructure.renderer.basetemplates as basetemplates
# weblog.add_renderer(SDMSBLFlag, basetemplates.T2_4MDetailsDefaultRenderer(always_rerender=True), group_by='ungrouped')
