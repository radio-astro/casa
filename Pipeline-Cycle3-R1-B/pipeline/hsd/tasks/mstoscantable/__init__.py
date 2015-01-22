from __future__ import absolute_import

import pipeline.infrastructure.renderer.weblog as weblog

from .mstoscantable import SDMsToScantable as SDMsToScantable
from . import renderer

weblog.add_renderer(SDMsToScantable, 
                    renderer.T2_4MDetailsSingleDishMSToScantableRenderer(always_rerender=True))
