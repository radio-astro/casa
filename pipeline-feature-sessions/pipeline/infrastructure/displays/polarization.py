from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
import casa

LOG = infrastructure.get_logger(__name__)


class PolarizationPlotCalChart(object):
    def __init__(self, context, result, caltable='', yaxis='delay', xaxis='freq', antenna=''):
        self.context = context
        self.result = result
        self.ms = context.observing_run.measurement_sets[0]
        self.yaxis = yaxis
        self.xaxis = xaxis
        self.antenna = antenna
        self.caltable = caltable

        # self.caltable = result.final[0].gaintable

    def plot(self):
        plots = [self.get_plot_wrapper('plotsummary')]
        return [p for p in plots if p is not None]

    def create_plot(self, prefix):
        figfile = self.get_figfile(prefix)

        LOG.info("Creating PlotSummary Polarization PlotCal plot")
        casa.plotcal(caltable=self.caltable, xaxis=self.xaxis, yaxis=self.yaxis, poln='',
                     field='', antenna=self.antenna, spw='', timerange='', subplot=111, overplot=False,
                     clearpanel='Auto', iteration='', plotrange=[], showflags=False,
                     plotsymbol='o', plotcolor='blue', markersize=5.0, fontsize=10.0,
                     showgui=False, figfile=figfile)

    def get_figfile(self, prefix):
        return os.path.join(self.context.report_dir,
                            'stage%s' % self.result.stage_number,
                            prefix + '_' + self.yaxis + '_vs_' + self.xaxis + '-%s-plotcal.png' % self.ms.basename)

    def get_plot_wrapper(self, prefix):
        figfile = self.get_figfile(prefix)

        typeentry = ''
        if self.antenna:
            typeentry = ' for RefAnt {!s}'.format(self.antenna)

        wrapper = logger.Plot(figfile, x_axis=self.xaxis, y_axis=self.yaxis,
                              parameters={'vis': self.ms.basename,
                                          'caltable': self.caltable,
                                          'type': typeentry,
                                          'antenna': self.antenna,
                                          'spw': '',
                                          'figfile': figfile})

        if not os.path.exists(figfile):
            LOG.trace('plotsummary Plotcal plot not found. Creating new plot.')
            try:
                self.create_plot(prefix)
            except Exception as ex:
                LOG.error('Could not create ' + prefix + ' plot.')
                LOG.exception(ex)
                return None

        return wrapper
