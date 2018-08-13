from __future__ import absolute_import
# import pipeline.infrastructure.pipelineqa as pipelineqa
# import pipeline.infrastructure.renderer.qaadapter as qaadapter
# import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog

from .plotsummary import PlotSummary
from .import plotsummary
from . import renderer

# from . import qa

#pipelineqa.registry.add_handler(qa.PlotSummaryQAHandler())
#pipelineqa.registry.add_handler(qa.PlotSummaryListQAHandler())
#qaadapter.registry.register_to_dataset_topic(plotsummary.PlotSummaryResults)

weblog.add_renderer(PlotSummary,
                    renderer.T2_4MDetailsplotsummaryRenderer(always_rerender=False,
                                                             uri='plotsummary.mako'), group_by=weblog.UNGROUPED)

