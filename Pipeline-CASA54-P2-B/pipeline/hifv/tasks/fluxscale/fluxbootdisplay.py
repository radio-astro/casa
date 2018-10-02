from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger

import casa

LOG = infrastructure.get_logger(__name__)


class fluxbootSummaryChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        # self.caltable = result.final[0].gaintable

    def plot(self):
        plots = [self.get_plot_wrapper()]
        return [p for p in plots if p is not None]

    def create_plot(self):
        figfile = self.get_figfile()
        
        context = self.context
        m = context.observing_run.measurement_sets[0]
        corrstring = m.get_vla_corrstring()
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        ms_active = m.name

        casa.plotms(vis=ms_active, xaxis='freq', yaxis='amp', ydatacolumn='model', selectdata=True,
                    scan=calibrator_scan_select_string, correlation=corrstring, averagedata=True,
                    avgtime='1e8', avgscan=True, transform=False,    extendflag=False, iteraxis='',
                    coloraxis='field', plotrange=[], title='', xlabel='', ylabel='',  showmajorgrid=False,
                    showminorgrid=False, plotfile=figfile, overwrite=True, clearplots=True, showgui=False)

    def get_figfile(self):
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            'bootstrappedFluxDensities-%s-summary.png' % self.ms.basename)

    def get_plot_wrapper(self):
        figfile = self.get_figfile()
       
        wrapper = logger.Plot(figfile, x_axis='freq', y_axis='amp', parameters={'vis': self.ms.basename,
                                                                                'type': 'fluxboot',
                                                                                'spw': '',
                                                                                'figurecaption':'Model calibrator'})

        if not os.path.exists(figfile):
            LOG.trace('Plotting model calibrator flux densities. Creating new plot.')
            try:
                self.create_plot()
            except Exception as ex:
                LOG.error('Could not create fluxboot plot.')
                LOG.exception(ex)
                return None
            
        return wrapper


class fluxgaincalSummaryChart(object):
    def __init__(self, context, result, caltable):
        self.context = context
        self.result = result
        self.caltable = caltable
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        # self.caltable = result.final[0].gaintable

    def plot(self):
        plots = [self.get_plot_wrapper()]
        return [p for p in plots if p is not None]

    def create_plot(self):
        figfile = self.get_figfile()

        casa.plotms(vis=self.caltable, xaxis='freq', yaxis='amp', ydatacolumn='', selectdata=True,
                    scan='', correlation='', averagedata=True,
                    avgtime='', avgscan=False, transform=False, extendflag=False, iteraxis='',
                    coloraxis='field', plotrange=[], title='', xlabel='', ylabel='', showmajorgrid=False,
                    showminorgrid=False, plotfile=figfile, overwrite=True, clearplots=True, showgui=False)

    def get_figfile(self):
        return os.path.join(self.context.report_dir,
                            'stage%s' % self.result.stage_number,
                            'fluxgaincalFluxDensities-%s-summary.png' % self.ms.basename)

    def get_plot_wrapper(self):
        figfile = self.get_figfile()

        wrapper = logger.Plot(figfile, x_axis='freq', y_axis='amp',
                              parameters={'vis': self.ms.basename,
                                          'type': 'fluxgaincal',
                                          'spw': '',
                                          'figurecaption': 'Caltable: {!s}'.format(self.caltable)})

        if not os.path.exists(figfile):
            LOG.trace('Plotting amp vs. freq for fluxgaincal. Creating new plot.')
            try:
                self.create_plot()
            except Exception as ex:
                LOG.error('Could not create fluxgaincal plot.')
                LOG.exception(ex)
                return None

        return wrapper
