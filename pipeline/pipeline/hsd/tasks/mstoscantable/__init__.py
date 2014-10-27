from __future__ import absolute_import

import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as super_renderer

from .mstoscantable import SDMsToScantable as SDMsToScantable

weblog.add_renderer(SDMsToScantable, 
                    super_renderer.T2_4MDetailsDefaultRenderer(description='Convert MS to Scantable', 
                                                               always_rerender=True))
