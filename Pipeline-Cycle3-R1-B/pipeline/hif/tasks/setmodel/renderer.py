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

        table_rows = make_flux_table(context, result)

        ctx.update({'amp_vs_uv_plots' : amp_vs_uv_summary_plots,
                    'table_rows'      : table_rows})

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
    
FluxTR = collections.namedtuple('FluxTR', 'vis field spw freq band i q u v')
    
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
                        fluxes[stokes] = '%s' % flux
                    except:
                        pass

                spw = ms_for_result.get_spectral_window(measurement.spw_id)
                                                   
                tr = FluxTR(vis_cell, field_cell, str(spw.id),
                            str(spw.centre_frequency), spw.band, fluxes['I'],
                            fluxes['Q'], fluxes['U'], fluxes['V'])
                rows.append(tr)
    
    return utils.merge_td_columns(rows)
