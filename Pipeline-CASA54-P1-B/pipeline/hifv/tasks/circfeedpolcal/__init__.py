from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog


from .circfeedpolcal import Circfeedpolcal
from . import circfeedpolcal
from . import renderer

#from . import qa

#pipelineqa.registry.add_handler(qa.CircfeedpolcalQAHandler())
#pipelineqa.registry.add_handler(qa.CircfeedpolcalListQAHandler())
#qaadapter.registry.register_to_dataset_topic(circfeedpolcal.CircfeedpolcalResults)


# weblog.add_renderer(Circfeedpolcal, basetemplates.T2_4MDetailsDefaultRenderer(uri='circfeedpolcal.mako',
#                          description='Circfeedpolcal'), group_by=weblog.UNGROUPED)

weblog.add_renderer(Circfeedpolcal,
                    renderer.T2_4MDetailsCircfeedpolcalRenderer(always_rerender=False,
                                                                uri='circfeedpolcal.mako'), group_by=weblog.UNGROUPED)
