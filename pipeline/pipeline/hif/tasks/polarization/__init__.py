from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog


from .polarization import Polarization
from . import polarization

#from . import qa

#pipelineqa.registry.add_handler(qa.PolQAHandler())
#pipelineqa.registry.add_handler(qa.PolListQAHandler())
#qaadapter.registry.register_to_dataset_topic(pol.PolResults)


weblog.add_renderer(Polarization, basetemplates.T2_4MDetailsDefaultRenderer(uri='polarization.mako',
                         description='Polarization'), group_by=weblog.UNGROUPED)