from __future__ import absolute_import

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from . import mstransform
from .mstransform import Mstransform
from . import qa
from . import renderer

pipelineqa.registry.add_handler(qa.MstransformQAHandler())
pipelineqa.registry.add_handler(qa.MstransformListQAHandler())
qaadapter.registry.register_to_dataset_topic(mstransform.MstransformResults)

weblog.add_renderer(Mstransform,
    renderer.T2_4MDetailsMstransformRenderer(),
    group_by=weblog.UNGROUPED)
