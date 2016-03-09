from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog

from .imaging import SDImaging
from . import imaging, scaling
from . import renderer
from . import qa

pipelineqa.registry.add_handler(qa.IntensityScalingQAHandler())
pipelineqa.registry.add_handler(qa.SDImagingQAHandler())
pipelineqa.registry.add_handler(qa.SDImagingVariableListQAHandler())

qaadapter.registry.register_to_imaging_topic(imaging.SDImagingResults)
qaadapter.registry.register_to_imaging_topic(scaling.IntensityScalingResults)

weblog.add_renderer(SDImaging, renderer.T2_4MDetailsSingleDishImagingRenderer(always_rerender=False), group_by='ungrouped')
