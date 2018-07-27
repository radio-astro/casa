"""
Created on 23 Oct 2014

@author: sjw
"""
import collections
import decimal
import itertools
import math
import operator
import os

import matplotlib.pyplot as plt

import pipeline.domain.measures as measures
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
from pipeline.domain.measures import FluxDensityUnits, FrequencyUnits
from pipeline.h.tasks.importdata.fluxes import ORIGIN_XML, ORIGIN_ANALYSIS_UTILS
from pipeline.infrastructure.renderer import logger
from . import display as gfluxscale
from ..importdata.dbfluxes import ORIGIN_DB

LOG = logging.get_logger(__name__)


CATALOGUE_SOURCES = (ORIGIN_ANALYSIS_UTILS, ORIGIN_DB, ORIGIN_XML)


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

        flux_comparison_plots = self.create_flux_comparison_plots(pipeline_context, results)

        table_rows = make_flux_table(pipeline_context, results)

        adopted_rows = make_adopted_table(pipeline_context, results)

        mako_context.update({
            'adopted_table': adopted_rows,
            'ampuv_allant_plots': ampuv_allant_plots,
            'ampuv_ant_plots': ampuv_ant_plots,
            'flux_plots': flux_comparison_plots,
            'table_rows': table_rows
        })

    def sort_plots_by_baseband(self, d):
        for vis, plots in d.items():
            plots = sorted(plots, 
                           key=lambda plot: plot.parameters['baseband'])
            d[vis] = plots

    def create_flux_comparison_plots(self, context, results):
        output_dir = os.path.join(context.report_dir, 'stage%s' % results.stage_number)
        d = {}

        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            d[vis] = create_flux_comparison_plots(context, output_dir, result)

        return d

    def create_plots(self, context, results, plotter_cls, intents, renderer_cls=None):
        """
        Create plots and return a dictionary of vis:[Plots].  No antenna or UVrange selection.
        """
        d = {}
        for result in results:
            plots = self.plots_for_result(context, result, plotter_cls, intents, renderer_cls)
            d = utils.dict_merge(d, plots)

        return d
        
    def create_plots_ants(self, context, results, plotter_cls, intents, renderer_cls=None):
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

    def plots_for_result(self, context, result, plotter_cls, intents, renderer_cls=None, ant='', uvrange=''):
        vis = os.path.basename(result.inputs['vis'])

        output_dir = os.path.join(context.report_dir, 'stage%s' % result.stage_number)

        # create a fake CalTo object so we can use the applycal class
        fields = result.inputs['reference']
        calto = callibrary.CalTo(result.inputs['vis'], field=fields)

        plotter = plotter_cls(context, output_dir, calto, intents, ant=ant, uvrange=uvrange)
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

                cat_measurements = [o for o in field.flux_densities if o.origin in CATALOGUE_SOURCES]
                for catmeasurement in cat_measurements:
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
        vis_cell = os.path.basename(adopted_result.vis)

        adopted_fields = adopted_result.measurements.keys()
        field_cell = ', '.join(adopted_fields)

        tr = AdoptedTR(vis_cell, field_cell)
        rows.append(tr)

    return utils.merge_td_columns(rows)


def create_flux_comparison_plots(context, output_dir, result):
    ms = context.observing_run.get_ms(result.vis)

    plots = []

    for field_id, measurements in result.measurements.iteritems():
        fig = plt.figure()
        ax = fig.add_subplot(1, 1, 1)

        fields = ms.get_fields(task_arg=field_id)
        assert len(fields) == 1
        field = fields[0]

        ax.set_title('Flux calibration: {}'.format(field.name))
        ax.set_xlabel('Frequency (GHz)')
        ax.set_ylabel('Flux Density (Jy)')

        colours = itertools.cycle('bgrcmyk')

        x_min = 1e99
        x_max = 0
        for m in sorted(measurements, key=operator.attrgetter('spw_id')):
            # cycle colours so that windows centred on the same frequency are distinguishable
            colour = colours.next()

            spw = ms.get_spectral_window(m.spw_id)
            x = spw.centre_frequency.to_units(FrequencyUnits.GIGAHERTZ)
            x_unc = decimal.Decimal('0.5') * spw.bandwidth.to_units(FrequencyUnits.GIGAHERTZ)

            y = m.I.to_units(FluxDensityUnits.JANSKY)
            y_unc = m.uncertainty.I.to_units(FluxDensityUnits.JANSKY)

            label = 'Derived flux for spw {}'.format(spw.id)
            ax.errorbar(x, y, xerr=x_unc, yerr=y_unc, fmt='{!s}-o'.format(colour), label=label)

            x_min = min(x_min, x - x_unc)
            x_max = max(x_max, x + x_unc)

        catalogue_fluxes = {
            ORIGIN_XML: 'ASDM',
            ORIGIN_DB: 'online catalogue',
            ORIGIN_ANALYSIS_UTILS: 'analysisUtils'
        }

        for origin, label in catalogue_fluxes.iteritems():
            fluxes = [f for f in field.flux_densities if f.origin == origin]
            if not fluxes:
                continue

            spws = [ms.get_spectral_window(f.spw_id) for f in fluxes]
            x = [spw.centre_frequency.to_units(FrequencyUnits.GIGAHERTZ) for spw in spws]
            y = [f.I.to_units(FluxDensityUnits.JANSKY) for f in fluxes]
            spix = [float(f.spix) for f in fluxes]
            # sort by frequency
            x, y, spix = zip(*sorted(zip(x, y, spix)))
            colour = colours.next()
            ax.plot(x, y, marker='o', color=colour, label='Data source: {}'.format(label))

            s_xmin = scale_flux(x[0], y[0], x_min, spix[0])
            s_xmax = scale_flux(x[-1], y[-1], x_max, spix[-1])
            ax.plot([x[0], x_min], [y[0], s_xmin], color=colour, label='Spectral index extrapolation',
                    linestyle='dotted')
            ax.plot([x[-1], x_max], [y[-1], s_xmax], color=colour, label='_nolegend_', linestyle='dotted')

        leg = ax.legend(loc='best', prop={'size': 8})
        leg.get_frame().set_alpha(0.5)
        figfile = '{}-field{}-flux_calibration.png'.format(ms.basename, field_id)

        full_path = os.path.join(output_dir, figfile)
        fig.savefig(full_path)

        parameters = {
            'vis': ms.basename,
            'field': field.name,
            'intent': sorted(set(field.intents))
        }
        wrapper = logger.Plot(full_path, x_axis='frequency', y_axis='Flux Density', parameters=parameters)
        plots.append(wrapper)

    return plots


def scale_flux(f1, s1, f2, spix):
    """Returns flux at a frequency by extrapolating via the spectral index.

    :param f1: frequency 1
    :param s1: flux density at frequency 1
    :param f2: frequency 2
    :param spix: spectral index
    :return: flux density at frequency 2
    """
    return math.pow(10, spix * math.log10(f2/f1) + math.log10(s1))
