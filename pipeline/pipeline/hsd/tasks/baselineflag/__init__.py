from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as super_renderer

from .baselineflag import SDBLFlag
from . import qa
from . import baselineflag

pipelineqa.registry.add_handler(qa.SDBLFlagQAHandler())
pipelineqa.registry.add_handler(qa.SDBLFlagListQAHandler())
qaadapter.registry.register_to_flagging_topic(baselineflag.SDBLFlagResults)

weblog.add_renderer(SDBLFlag, super_renderer.T2_4MDetailsDefaultRenderer(uri='hsd_blflag.mako', 
                                                                           description='Flag data by Tsys, weather, and statistics of spectra', 
                                                                           always_rerender=True))
