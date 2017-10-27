"""
Created on 11 Sep 2014

@author: sjw
"""
import collections
import os
import shutil

import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
#import pipeline.infrastructure.displays.setjy as setjy_display
from . import display as setjy_display

LOG = logging.get_logger(__name__)


class T2_4MDetailsSetjyRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='setjy.mako', 
                 description='Set calibrator model visibilities',
                 always_rerender=False):
        super(T2_4MDetailsSetjyRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, result):
        amp_vs_uv_summary_plots = collections.defaultdict(list)



        for intents in ['AMPLITUDE']:
            plots = self.create_plots(context, 
                                      result, 
                                      setjy_display.AmpVsUVSummaryChart, 
                                      intents)
            self.sort_plots_by_baseband(plots)

            for vis, vis_plots in plots.items():
                amp_vs_uv_summary_plots[vis].extend(vis_plots)

        table_rows = make_flux_table(context, result)



        ctx.update({'amp_vs_uv_plots': amp_vs_uv_summary_plots,
                    'table_rows': table_rows})

    def sort_plots_by_baseband(self, d):
        for vis, plots in d.items():
            plots = sorted(plots, 
                           key=lambda plot: plot.parameters['baseband'])
            d[vis] = plots

    def create_plots(self, context, results, plotter_cls, intents):
        """
        Create plots and return a dictionary of vis:[Plots].
        """
        d = {}
        for result in results:
            plots = self.plots_for_result(context, result, plotter_cls, intents)
            d = utils.dict_merge(d, plots)
        return d

    def plots_for_result(self, context, result, plotter_cls, intents):
        output_dir = os.path.join(context.report_dir, 'stage%s' % result.stage_number)

        # create a fake CalTo object so we can use the applycal class
        spws = []
        for fieldname in result.measurements.keys():
            for fluxm in result.measurements[fieldname]:
                spws.append(fluxm.spw_id)
        spwlist = ','.join([str(i) for i in spws])
        calto = callibrary.CalTo(result.inputs['vis'], spw=spwlist)

        plotter = plotter_cls(context, output_dir, calto, intents)
        plots = plotter.plot()

        d = collections.defaultdict(dict)
        vis = os.path.basename(result.inputs['vis'])        
        d[vis] = plots

        return d
    
FluxTR = collections.namedtuple('FluxTR', 'vis field spw freq band i q u v spix fluxcsv')


def make_flux_table(context, results):
    # will hold all the flux stat table rows for the results
    rows = []



    for single_result in results:
        ms_for_result = context.observing_run.get_ms(single_result.vis)
        vis_cell = os.path.basename(single_result.vis)

        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % single_result.stage_number)

        # measurements will be empty if fluxscale derivation failed
        if len(single_result.measurements) is 0:
            continue

        # copy flux.csv file across to weblog directory
        fluxcsv_filename = 'flux.csv'
        fluxcsv_path = os.path.join(weblog_dir, fluxcsv_filename)
        fluxcsv_weblink = os.path.join('stage%s' % single_result.stage_number, fluxcsv_filename)
        fluxcsv_path_link = '<a href="{!s}" class="replace-pre" data-title="flux.csv">View</a> or <a href="{!s}" download="{!s}">download</a>'.format(fluxcsv_weblink,fluxcsv_weblink,fluxcsv_weblink)
        if os.path.exists(fluxcsv_filename):
            LOG.trace('Copying %s to %s' % (fluxcsv_filename, weblog_dir))
            shutil.copy(fluxcsv_filename, weblog_dir)


            
        for field_arg, measurements in single_result.measurements.items():
            field = ms_for_result.get_fields(field_arg)[0]
            intents = " ".join(field.intents.intersection({'AMPLITUDE', 'BANDPASS', 'CHECK', 'PHASE'}))
            field_cell = '%s (#%s) %s' % (field.name, field.id, intents)

            for measurement in sorted(measurements, key=lambda m: int(m.spw_id)):
                fluxes = collections.defaultdict(lambda: 'N/A')
                for item in ['I', 'Q', 'U', 'V', 'spix']:
                    try:                        
                        value = getattr(measurement, item)
                        fluxes[item] = '%s' % value
                    except:
                        pass

                spw = ms_for_result.get_spectral_window(measurement.spw_id)
                                                   
                tr = FluxTR(vis_cell, field_cell, str(spw.id),
                            str(spw.centre_frequency), spw.band, fluxes['I'],
                            fluxes['Q'], fluxes['U'], fluxes['V'], fluxes['spix'], fluxcsv_path_link)
                rows.append(tr)
    
    return utils.merge_td_columns(rows)
