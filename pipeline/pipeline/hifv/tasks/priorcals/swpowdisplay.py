from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
import casa


LOG = infrastructure.get_logger(__name__)


class swpowSummaryChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.caltable = result.sw_result.final[0].gaintable

    def plot(self):
        # science_spws = self.ms.get_spectral_windows(science_windows_only=True)
        plots = [self.get_plot_wrapper('swpow_sample')]
        return [p for p in plots if p is not None]

    def create_plot(self, prefix):
        figfile = self.get_figfile(prefix)

        antPlot = '0~2'

        plotmax = 100

        # Dummy plot
        casa.plotcal(caltable=self.caltable, xaxis='time', yaxis='amp', poln='', field='', antenna=antPlot,
                     spw='',
                     timerange='', subplot=311, overplot=False, clearpanel='Auto', iteration='antenna',
                     plotrange=[0, 0, 0, plotmax], showflags=False, plotsymbol='o', plotcolor='blue', markersize=5.0,
                     fontsize=10.0, showgui=False, figfile=figfile)

    def get_figfile(self, prefix):
        return os.path.join(self.context.report_dir,
                            'stage%s' % self.result.stage_number,
                            'swpow' + prefix + '-%s-summary.png' % self.ms.basename)

    def get_plot_wrapper(self, prefix):
        figfile = self.get_figfile(prefix)

        wrapper = logger.Plot(figfile,
                              x_axis='freq',
                              y_axis='amp',
                              parameters={'vis': self.ms.basename,
                                          'type': prefix,
                                          'spw': ''})

        if not os.path.exists(figfile):
            LOG.trace('swpow summary plot not found. Creating new '
                      'plot.')
            try:
                self.create_plot(prefix)
            except Exception as ex:
                LOG.error('Could not create ' + prefix + ' plot.')
                LOG.exception(ex)
                return None

        return wrapper


class swpowPerAntennaChart(object):
    def __init__(self, context, result, yaxis):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        ms = self.ms
        self.yaxis = yaxis
        self.caltable = result.sw_result.final[0].gaintable

        self.json = {}
        self.json_filename = os.path.join(context.report_dir,
                                          'stage%s' % result.stage_number,
                                          yaxis + 'swpow-%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        numAntenna = len(m.antennas)
        plots = []

        LOG.info("Plotting swpowcal charts for " + self.yaxis)
        nplots = numAntenna

        for ii in range(nplots):

            filename = 'swpow_' + self.yaxis + str(ii) + '.png'
            antPlot = str(ii)

            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'

            figfile = os.path.join(stage_dir, filename)

            plotrange = []

            if self.yaxis == 'spgain':
                plotrange = [0,0,0,1.0]
            if self.yaxis == 'tsys':
                plotrange = [0,0,0,100]
                spws = m.get_all_spectral_windows()
                freqs = sorted(set([spw.max_frequency.value for spw in spws]))
                if float(max(freqs)) >= 18000000000.0:
                    plotrange = [0,0,0,200]

            if not os.path.exists(figfile):
                try:
                    # Get antenna name
                    antName = antPlot
                    if antPlot != '':
                        domain_antennas = self.ms.get_antenna(antPlot)
                        idents = [a.name if a.name else a.id for a in domain_antennas]
                        antName = ','.join(idents)

                    LOG.debug("Switched Power Plot, using antenna={!s} and spw={!s}".format(antName,
                                                                                           self.result.sw_result.spw))
                    #casa.plotcal(caltable=self.caltable, xaxis='time', yaxis=self.yaxis, poln='', field='',
                    #             antenna=antPlot, spw=self.result.sw_result.spw, timerange='', subplot=111,
                    #             overplot=False, clearpanel='Auto',
                    #             iteration='antenna', plotrange=plotrange, showflags=False, plotsymbol='o',
                    #             plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=figfile)

                    casa.plotms(vis=self.caltable, xaxis='time', yaxis=self.yaxis, field='',
                                antenna=antPlot, spw=self.result.sw_result.spw, timerange='',
                                plotrange=plotrange, coloraxis='spw',
                                title='Switched Power  swpow.tbl   Antenna: {!s}'.format(antName),
                                titlefont=8, xaxisfont=7, yaxisfont=7, showgui=False, plotfile=figfile)

                except:
                    LOG.warn("Unable to plot " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')

            try:
                plot = logger.Plot(figfile, x_axis='Time', y_axis=self.yaxis.title(), field='',
                                   parameters={'spw': '',
                                               'pol': '',
                                               'ant': antName,
                                               'type': self.yaxis,
                                               'file': os.path.basename(figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]
