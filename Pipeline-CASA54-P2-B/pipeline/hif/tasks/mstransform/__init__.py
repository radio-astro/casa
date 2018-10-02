from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import mstransform
from . import qa
from . import renderer
from .mstransform import Mstransform

qaadapter.registry.register_to_dataset_topic(mstransform.MstransformResults)

weblog.add_renderer(Mstransform,
    renderer.T2_4MDetailsMstransformRenderer(),
    group_by=weblog.UNGROUPED)
