from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog


from .imageprecheck import ImagePreCheck
from . import imageprecheck
from . import renderer

from . import qa

pipelineqa.registry.add_handler(qa.ImagePreCheckQAHandler())
pipelineqa.registry.add_handler(qa.ImagePreCheckListQAHandler())
qaadapter.registry.register_to_dataset_topic(imageprecheck.ImagePreCheckResults)


weblog.add_renderer(ImagePreCheck, renderer.T2_4MDetailsCheckProductSizeRenderer(uri='imageprecheck.mako',
                         description='ImagePreCheck'), group_by=weblog.UNGROUPED)
