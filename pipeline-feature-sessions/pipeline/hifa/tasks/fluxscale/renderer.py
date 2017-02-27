"""
Created on 23 Oct 2014

@author: sjw
"""
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
                if len(vis_plots) > 0:
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
                if len(vis_plots) > 0:
                    ampuv_ant_plots[vis][key] = vis_plots

        table_rows = make_flux_table(pipeline_context, results)

        adopted_rows = make_adopted_table(pipeline_context, results)

        mako_context.update({
            'adopted_table': adopted_rows,
            'ampuv_allant_plots': ampuv_allant_plots,
            'ampuv_ant_plots': ampuv_ant_plots,
            'table_rows': table_rows
        })

    def sort_plots_by_baseband(self, d):
        for vis, plots in d.items():
            plots = sorted(plots, 
                           key=lambda plot: plot.parameters['baseband'])
            d[vis] = plots

    def create_plots(self, context, results, plotter_cls, intents, renderer_cls=None):
        """
        Create plots and return a dictionary of vis:[Plots].  No antenna or UVrange selection.
        """
        d = {}
        for result in results:
            plots = self.plots_for_result(context, result, plotter_cls, intents, renderer_cls)
            d = utils.dict_merge(d, plots)
        return d
        
    def create_plots_ants(self, context, results, plotter_cls, intents,
                          renderer_cls=None):
        """
        Create plots and return a dictionary of vis:[Plots].  Antenna and UVrange selection
                                                              determined by heuristics.
        """
        d = {}
        for result in results:
            plots = self.plots_for_result(context, result, plotter_cls,
                    intents, renderer_cls, ant=result.resantenna, uvrange=result.uvrange)
            d = utils.dict_merge(d, plots)
        return d

    def plots_for_result(self, context, result, plotter_cls, intents,
                         renderer_cls=None, ant='', uvrange=''):
        vis = os.path.basename(result.inputs['vis'])
        
        plotter = plotter_cls(context, result, intents, ant=ant, uvrange=uvrange)
        plots = plotter.plot()

        d = collections.defaultdict(dict)
        d[vis] = plots

        if renderer_cls is not None:
            renderer = renderer_cls(context, result, plots)
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())        

        return d
    
    
FluxTR = collections.namedtuple('FluxTR', 'vis field spw freqbw i q u v fluxratio spix')

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
            intents = ' '. join(field.intents.intersection({'BANDPASS', 'PHASE', 'CHECK'}))
            field_cell = '%s (#%s) %s' % (field.name, field.id, intents)

            for measurement in sorted(measurements, key=lambda m: int(m.spw_id)):
                spw = ms_for_result.get_spectral_window(measurement.spw_id)
                freqbw = '%s %s' % (str(spw.centre_frequency), str(spw.bandwidth))
                fluxes = collections.defaultdict(lambda: 'N/A')

                for stokes in ['I', 'Q', 'U', 'V']:
                    try:                        
                        flux = getattr(measurement, stokes)
                        unc = getattr(measurement.uncertainty, stokes)
                        flux_jy = flux.to_units(measures.FluxDensityUnits.JANSKY)
                        if stokes == 'I':
                            flux_jy_I = flux_jy
                        unc_jy = unc.to_units(measures.FluxDensityUnits.JANSKY)
                        
                        if flux_jy != 0 and unc_jy != 0:
                            unc_ratio = decimal.Decimal('100')*(unc_jy/flux_jy)
                            uncertainty = ' &#177; %s (%0.1f%%)' % (str(unc), unc_ratio)
                        else:
                            uncertainty = ''
                            
                        fluxes[stokes] = '%s%s' % (flux, uncertainty)
                    except:
                        pass

                try:
                    fluxes['spix'] = '%s' % getattr(measurement, 'spix')
                except:
                    fluxes['spix'] = '0.0'

                # Get the corresponding catalog flux
                catfluxes = collections.defaultdict(lambda: 'N/A')
                flux_ratio = 'N/A'
                for catmeasurement in field.flux_densities:
                    if catmeasurement.spw_id != int(measurement.spw_id):
                        continue
                    for stokes in ['I', 'Q', 'U', 'V']:
                        try:                        
                            catflux = getattr(catmeasurement, stokes)
                            catflux_jy = catflux.to_units(measures.FluxDensityUnits.JANSKY)
                            if stokes == 'I':
                                catflux_jy_I = catflux_jy
                            catfluxes[stokes] = ' %s' % (catflux)
                        except:
                            pass
                    try:
                        catfluxes['spix'] = '%s' % getattr(catmeasurement, 'spix')
                    except:
                        catfluxes['spix'] = '0.0'
                    if fluxes['I'] != 'N/A' and catfluxes['I'] != 'N/A':
                        flux_ratio = '%0.3f' % (float(flux_jy_I) / float(catflux_jy_I))
                    break
                                    
                tr = FluxTR(vis_cell, field_cell, measurement.spw_id, freqbw, 
                            fluxes['I'],
                            fluxes['Q'],
                            fluxes['U'],
                            fluxes['V'],
                            flux_ratio,
                            fluxes['spix'])
                rows.append(tr)

                tr = FluxTR(vis_cell, field_cell, measurement.spw_id, freqbw, 
                            catfluxes['I'],
                            catfluxes['Q'],
                            catfluxes['U'],
                            catfluxes['V'],
                            flux_ratio,
                            fluxes['spix'])
                rows.append(tr)

    return utils.merge_td_columns(rows)


AdoptedTR = collections.namedtuple('AdoptedTR', 'vis fields')


def make_adopted_table(context, results):
    # will hold all the flux stat table rows for the results
    rows = []

    for adopted_result in [r for r in results if r.applies_adopted]:
        ms_for_result = context.observing_run.get_ms(adopted_result.vis)
        vis_cell = os.path.basename(adopted_result.vis)

        adopted_fields = adopted_result.measurements.keys()
        field_cell = ', '.join(adopted_fields)

        tr = AdoptedTR(vis_cell, field_cell)
        rows.append(tr)

    return utils.merge_td_columns(rows)
