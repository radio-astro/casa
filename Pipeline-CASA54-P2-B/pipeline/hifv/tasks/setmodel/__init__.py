from __future__ import absolute_import
from pipeline.h.tasks.common import commonfluxresults
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .vlasetjy import VLASetjy
from . import renderer

from . import qa

qaadapter.registry.register_to_calibration_topic(commonfluxresults.FluxCalibrationResults)

weblog.add_renderer(VLASetjy, renderer.T2_4MDetailsVLASetjyRenderer(uri="vlasetjy.mako"), group_by=weblog.UNGROUPED)
