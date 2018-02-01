from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import qa
from . import renderer
from . import resultobjects
from .checkproductsize import CheckProductSize

qaadapter.registry.register_to_imaging_topic(resultobjects.CheckProductSizeResult)

weblog.add_renderer(CheckProductSize, renderer.T2_4MDetailsCheckProductSizeRenderer(), group_by=weblog.UNGROUPED)
