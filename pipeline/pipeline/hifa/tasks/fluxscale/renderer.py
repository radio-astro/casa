'''
Created on 23 Oct 2014

@author: sjw
'''
import collections
import os

import pipeline.infrastructure.displays.gfluxscale as gfluxscale
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)


class T2_4MDetailsGFluxscaleRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='gfluxscale.mako', 
                 description='Transfer fluxscale from amplitude calibrator',
                 always_rerender=False):
        super(T2_4MDetailsGFluxscaleRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):
        #All antenna, sort by baseband
        ampuv_allant_plots = collections.defaultdict(dict)
        for intents in ['AMPLITUDE']:
            plots = self.create_plots(pipeline_context, 
                                      results, 
                                      gfluxscale.GFluxscaleSummaryChart, 
                                      intents)
            self.sort_plots_by_baseband(plots)

            key = intents
            for vis, vis_plots in plots.items():
                ampuv_allant_plots[vis][key] = vis_plots
                
        #List of antenna for the fluxscale result, sorted by baseband
        ampuv_ant_plots = collections.defaultdict(dict)

        for intents in ['AMPLITUDE']:
            plots = self.create_plots_ants(pipeline_context, 
                                      results, 
                                      gfluxscale.GFluxscaleSummaryChart, 
                                      intents)
            self.sort_plots_by_baseband(plots)

            key = intents
            for vis, vis_plots in plots.items():
                ampuv_ant_plots[vis][key] = vis_plots

        mako_context.update({'ampuv_allant_plots' : ampuv_allant_plots,
                             'ampuv_ant_plots'    : ampuv_ant_plots})

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
        
    def create_plots_ants(self, context, results, plotter_cls, intents,
                          renderer_cls=None):
        """
        Create plots and return a dictionary of vis:[Plots].
        """
        d = {}
        for result in results:
            plots = self.plots_for_result(context, result, plotter_cls,
                    intents, renderer_cls, ant=result.resantenna)
            d = utils.dict_merge(d, plots)
        return d

    def plots_for_result(self, context, result, plotter_cls, intents,
                         renderer_cls=None, ant=''):
        vis = os.path.basename(result.inputs['vis'])
        
        plotter = plotter_cls(context, result, intents, ant=ant)
        plots = plotter.plot()

        d = collections.defaultdict(dict)
        d[vis] = plots

        if renderer_cls is not None:
            renderer = renderer_cls(context, result, plots)
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())        

        return d
