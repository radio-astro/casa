'''
Created on 11 Sep 2014

@author: sjw
'''
import collections
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.displays.setjy as setjy_display
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)


class T2_4MDetailsSetjyRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='setjy.mako', 
                 description='Set calibrator model visibilities',
                 always_rerender=False):
        super(T2_4MDetailsSetjyRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, result):
        amp_vs_uv_summary_plots = collections.defaultdict(dict)
        for intents in ['AMPLITUDE']:
            plots = self.create_plots(context, 
                                      result, 
                                      setjy_display.AmpVsUVSummaryChart, 
                                      intents)
            self.sort_plots_by_baseband(plots)

            key = intents
            for vis, vis_plots in plots.items():
                amp_vs_uv_summary_plots[vis][key] = vis_plots

        ctx.update({'amp_vs_uv_plots' : amp_vs_uv_summary_plots})

    def sort_plots_by_baseband(self, d):
        for vis, plots in d.items():
            plots = sorted(plots, 
                           key=lambda plot: plot.parameters['baseband'])
            d[vis] = plots

    def create_plots(self, context, results, plotter_cls, intents, renderer_cls=None):
        """
        Create plots and return a dictionary of vis:[Plots].
        """
        d = {}
        for result in results:
            plots = self.plots_for_result(context, result, plotter_cls, intents, renderer_cls)
            d = utils.dict_merge(d, plots)
        return d

    def plots_for_result(self, context, result, plotter_cls, intents, renderer_cls=None):
        plotter = plotter_cls(context, result, intents)
        plots = plotter.plot()

        d = collections.defaultdict(dict)
        vis = os.path.basename(result.inputs['vis'])        
        d[vis] = plots

        if renderer_cls is not None:
            renderer = renderer_cls(context, result, plots)
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())        

        return d
