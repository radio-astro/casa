'''
Created on 23 Oct 2014

@author: sjw
'''
import collections
import decimal
import os

import pipeline.domain.measures as measures
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

        table_rows = make_flux_table(pipeline_context, results)
        
        mako_context.update({'ampuv_allant_plots' : ampuv_allant_plots,
                             'ampuv_ant_plots'    : ampuv_ant_plots,
                             'table_rows'         : table_rows})

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
    
    
    
FluxTR = collections.namedtuple('FluxTR', 'vis field spw i q u v')

def make_flux_table(context, results):
    # will hold all the flux stat table rows for the results
    rows = []

    for single_result in results:
        ms_for_result = context.observing_run.get_ms(single_result.vis)
        vis_cell = os.path.basename(single_result.vis)

        # measurements will be empty if fluxscale derivation failed
        if len(single_result.measurements) is 0:
            continue
            
        for field_arg, measurements in single_result.measurements.items():
            field = ms_for_result.get_fields(field_arg)[0]
            field_cell = '%s (#%s)' % (field.name, field.id)

            for measurement in sorted(measurements, key=lambda m: int(m.spw_id)):
                fluxes = collections.defaultdict(lambda: 'N/A')
                for stokes in ['I', 'Q', 'U', 'V']:
                    try:                        
                        flux = getattr(measurement, stokes)
                        unc = getattr(measurement.uncertainty, stokes)
                        flux_jy = flux.to_units(measures.FluxDensityUnits.JANSKY)
                        unc_jy = unc.to_units(measures.FluxDensityUnits.JANSKY)
                        
                        if flux_jy != 0 and unc_jy != 0:
                            unc_ratio = decimal.Decimal('100')*(unc_jy/flux_jy)
                            uncertainty = ' &#177; %s (%0.1f%%)' % (str(unc),
                                                                    unc_ratio)
                        else:
                            uncertainty = ''
                            
                        fluxes[stokes] = '%s%s' % (flux, uncertainty)
                    except:
                        pass
                                    
                tr = FluxTR(vis_cell, field_cell, measurement.spw_id, 
                            fluxes['I'], fluxes['Q'], fluxes['U'], fluxes['V'])
                rows.append(tr)

    return utils.merge_td_columns(rows)

