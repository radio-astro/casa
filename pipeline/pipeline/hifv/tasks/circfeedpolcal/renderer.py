import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.displays.polarization as polarization

LOG = logging.get_logger(__name__)


class T2_4MDetailsCircfeedpolcalRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='circfeedpolcal.mako',
                 description='CircFeed Polarization', always_rerender=False):
        super(T2_4MDetailsCircfeedpolcalRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, result):
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % result.stage_number)

        polarization_plotcal_plots = {}

        for r in result:
            if r.final:
                ktable = r.final[0].gaintable
                Dtable = r.final[1].gaintable
                Xtable = r.final[2].gaintable

                plotter = polarization.PolarizationPlotCalChart(context, r,
                                                                caltable=ktable,
                                                                yaxis='delay', xaxis='freq', antenna=r.refant)
                plots = plotter.plot()
                # -------------
                plotter = polarization.PolarizationPlotCalChart(context, r,
                                                                caltable=ktable,
                                                                yaxis='delay', xaxis='antenna', antenna='')
                plots.extend(plotter.plot())
                # -------------
                plotter = polarization.PolarizationPlotCalChart(context, r,
                                                                caltable=Dtable,
                                                                yaxis='amp', xaxis='antenna', antenna='')
                plots.extend(plotter.plot())
                # -------------
                plotter = polarization.PolarizationPlotCalChart(context, r,
                                                                caltable=Xtable,
                                                                yaxis='phase', xaxis='freq', antenna=r.refant)
                plots.extend(plotter.plot())
                # -------------

                ms = os.path.basename(r.inputs['vis'])
                polarization_plotcal_plots[ms] = plots

            ctx.update({'polarization_plotcal_plots': polarization_plotcal_plots})
