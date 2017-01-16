from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog


from .linfeedpolcal import Linfeedpolcal
from . import linfeedpolcal

#from . import qa

#pipelineqa.registry.add_handler(qa.LinfeedpolcalQAHandler())
#pipelineqa.registry.add_handler(qa.LinfeedpolcalListQAHandler())
#qaadapter.registry.register_to_dataset_topic(linfeedpolcal.LinfeedpolcalResults)


weblog.add_renderer(Linfeedpolcal, basetemplates.T2_4MDetailsDefaultRenderer(uri='linfeedpolcal.mako',
                         description='Linfeedpolcal'), group_by=weblog.UNGROUPED)