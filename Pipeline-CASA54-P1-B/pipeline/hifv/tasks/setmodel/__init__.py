from __future__ import absolute_import
from pipeline.h.tasks.common import commonfluxresults
#import pipeline.infrastructure.pipelineqa as pipelineqa
#import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .vlasetjy import VLASetjy
#from .setmodel import SetModel
# from . import qa
from . import renderer

#pipelineqa.registry.add_handler(qa.VLASetjyQAHandler())
#pipelineqa.registry.add_handler(qa.VLASetjyListQAHandler())
#qaadapter.registry.register_to_calibration_topic(commonfluxresults.FluxCalibrationResults)

#weblog.add_renderer(VLASetjy, super_renderer.T2_4MDetailsSetjyRenderer(uri="vlasetjy.mako"), group_by=weblog.UNGROUPED)
weblog.add_renderer(VLASetjy, renderer.T2_4MDetailsVLASetjyRenderer(uri="vlasetjy.mako"), group_by=weblog.UNGROUPED)
