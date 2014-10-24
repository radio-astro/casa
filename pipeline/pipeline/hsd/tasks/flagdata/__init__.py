from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as super_renderer

from .flagdata import SDFlagData
from . import qa
from . import flagdata

pipelineqa.registry.add_handler(qa.SDFlagDataQAHandler())
pipelineqa.registry.add_handler(qa.SDFlagDataListQAHandler())
qaadapter.registry.register_to_flagging_topic(flagdata.SDFlagDataResults)

weblog.add_renderer(SDFlagData, super_renderer.T2_4MDetailsDefaultRenderer(uri='hsd_flagdata.mako', 
                                                                           description='Flag data by Tsys, weather, and statistics of spectra', 
                                                                           always_rerender=True))
