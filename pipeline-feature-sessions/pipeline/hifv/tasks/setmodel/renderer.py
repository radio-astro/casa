'''
Created on 11 Sep 2014

@author: sjw
         bkent April 2015
'''
import collections
import os

import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
#import pipeline.infrastructure.displays.setjy as setjy_display
from pipeline.hif.tasks.setmodel import display as setjy_display

LOG = logging.get_logger(__name__)


class T2_4MDetailsVLASetjyRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='vlasetjy.mako',
                 description='Set calibrator model visibilities',
                 always_rerender=False):
        super(T2_4MDetailsVLASetjyRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, result):
        amp_vs_uv_summary_plots = collections.defaultdict(list)
        for intents in ['AMPLITUDE']:
            plots = self.create_plots(context, 
                                      result, 
                                      setjy_display.AmpVsUVSummaryChart, 
                                      intents, correlation='LL,RR')

            for vis, vis_plots in plots.items():
                amp_vs_uv_summary_plots[vis].extend(vis_plots)

        table_rows = make_flux_table(context, result)

        ctx.update({
            'amp_vs_uv_plots': amp_vs_uv_summary_plots,
            'table_rows': table_rows
        })

    def create_plots(self, context, results, plotter_cls, intents, renderer_cls=None, **kwargs):
        """
        Create plots and return a dictionary of vis:[Plots].
        """
        d = {}
        for result in results:
            plots = self.plots_for_result(context, result, plotter_cls, intents, renderer_cls, **kwargs)
            d = utils.dict_merge(d, plots)
        return d

    def plots_for_result(self, context, result, plotter_cls, intents, renderer_cls=None, **kwargs):
        output_dir = os.path.join(context.report_dir, 'stage%s' % result.stage_number)

        # create a fake CalTo object so we can use the applycal class
        spws = []
        for fieldname in result.measurements.keys():
            for fluxm in result.measurements[fieldname]:
                spws.append(fluxm.spw_id)
        spwlist = ','.join([str(i) for i in spws])
        calto = callibrary.CalTo(result.inputs['vis'], spw=spwlist)

        plotter = plotter_cls(context, output_dir, calto, intents, **kwargs)
        plots = plotter.plot()

        d = collections.defaultdict(dict)
        vis = os.path.basename(result.inputs['vis'])        
        d[vis] = plots

        if renderer_cls is not None:
            renderer = renderer_cls(context, result, plots, **kwargs)
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
            fields = ms_for_result.get_fields(field_arg)
            field = ms_for_result.get_fields(field_arg)[0]
            try:
                if (len(fields) > 1):
                    for f in fields:
                        for intent in f.intents:
                            if 'AMPLITUDE' in intent:
                                field = f
            except:
                LOG.info("Single amplitude calibrator")
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
