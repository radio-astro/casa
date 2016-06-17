from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog

from .imaging import SDImaging as SDMSImaging
from . import imaging
from . import sdcombine
# from . import renderer
# from . import qa

# pipelineqa.registry.add_handler(qa.SDImagingQAHandler())
# pipelineqa.registry.add_handler(qa.SDImagingVariableListQAHandler())
# 
# qaadapter.registry.register_to_imaging_topic(imaging.SDImagingResults)

# weblog.add_renderer(SDMSImaging, renderer.T2_4MDetailsSingleDishMSImagingRenderer(always_rerender=True), group_by='ungrouped')
