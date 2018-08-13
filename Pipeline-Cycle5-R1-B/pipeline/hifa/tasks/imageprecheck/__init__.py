from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import imageprecheck
from . import qa
from . import renderer
from .imageprecheck import ImagePreCheck

qaadapter.registry.register_to_dataset_topic(imageprecheck.ImagePreCheckResults)


weblog.add_renderer(ImagePreCheck, renderer.T2_4MDetailsCheckProductSizeRenderer(uri='imageprecheck.mako',
                         description='ImagePreCheck'), group_by=weblog.UNGROUPED)
