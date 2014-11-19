from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as super_renderer

from .simplescale import SDSimpleScale
from . import qa
#from . import renderer

pipelineqa.registry.add_handler(qa.SDSimpleScaleQAHandler())
pipelineqa.registry.add_handler(qa.SDSimpleScaleListQAHandler())
qaadapter.registry.register_to_miscellaneous_topic(simplescale.SDSimpleScaleResults)

weblog.add_renderer(SDSimpleScale,
                    super_renderer.T2_4MDetailsDefaultRenderer(uri='hsd_simplescale.mako',
                                                               description='Non-linearity correction', 
                                                               always_rerender=True))
