from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
import casa


LOG = infrastructure.get_logger(__name__)


class checkflagSummaryChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        # self.caltable = result.final[0].gaintable

    def plot(self):
        # Default
        plots = [None]

        if self.result.inputs['checkflagmode'] == 'bpd' or self.result.inputs['checkflagmode'] == 'bpd-vlass':
            plots = [self.get_plot_wrapper('BPcal'), self.get_plot_wrapper('delaycal')]
        if self.result.inputs['checkflagmode'] == 'allcals' or self.result.inputs['checkflagmode'] == 'allcals-vlass':
            plots = [self.get_plot_wrapper('allcals')]
        return [p for p in plots if p is not None]

    def create_plot(self, prefix):
        figfile = self.get_figfile(prefix)

        bandpass_field_select_string = self.context.evla['msinfo'][self.ms.name].bandpass_field_select_string
        bandpass_scan_select_string = self.context.evla['msinfo'][self.ms.name].bandpass_scan_select_string
        corrstring = self.ms.get_vla_corrstring()
        delay_scan_select_string = self.context.evla['msinfo'][self.ms.name].delay_scan_select_string

        if self.result.inputs['checkflagmode'] == 'bpd' or self.result.inputs['checkflagmode'] == 'bpd-vlass':
            if prefix == 'BPcal':
                casa.plotms(vis=self.ms.name, xaxis='freq', yaxis='amp', ydatacolumn='corrected', selectdata=True,
                            field=bandpass_field_select_string, scan=bandpass_scan_select_string,
                            correlation=corrstring, averagedata=True, avgtime='1e8', avgscan=True, transform=False,
                            extendflag=False, iteraxis='', coloraxis='antenna2', plotrange=[], title='',
                            xlabel='', ylabel='', showmajorgrid=False, showminorgrid=False, plotfile=figfile,
                            overwrite=True, clearplots=True, showgui=False)

            if (delay_scan_select_string != bandpass_scan_select_string) and prefix == 'delaycal':
                casa.plotms(vis=self.ms.name, xaxis='freq', yaxis='amp', ydatacolumn='corrected', selectdata=True,
                            scan=delay_scan_select_string, correlation=corrstring, averagedata=True,
                            avgtime='1e8', avgscan=True, transform=False, extendflag=False, iteraxis='',
                            coloraxis='antenna2', plotrange=[], title='', xlabel='', ylabel='', showmajorgrid=False,
                            showminorgrid=False, plotfile=figfile, overwrite=True, clearplots=True, showgui=False)

        if self.result.inputs['checkflagmode'] == 'allcals' or self.result.inputs['checkflagmode'] == 'allcals-vlass':
            calibrator_scan_select_string = self.context.evla['msinfo'][self.ms.name].calibrator_scan_select_string

            casa.plotms(vis=self.ms.name, xaxis='freq', yaxis='amp', ydatacolumn='corrected', selectdata=True,
                        scan=calibrator_scan_select_string, correlation=corrstring, averagedata=True, avgtime='1e8',
                        avgscan=False, transform=False, extendflag=False, iteraxis='', coloraxis='antenna2',
                        plotrange=[], title='', xlabel='', ylabel='', showmajorgrid=False, showminorgrid=False,
                        plotfile=figfile, overwrite=True, clearplots=True, showgui=False)

    def get_figfile(self, prefix):
        return os.path.join(self.context.report_dir,
                            'stage%s' % self.result.stage_number,
                            'checkflag' + prefix + '-%s-summary.png' % self.ms.basename)

    def get_plot_wrapper(self, prefix):
        figfile = self.get_figfile(prefix)

        bandpass_scan_select_string = self.context.evla['msinfo'][self.ms.name].bandpass_scan_select_string
        delay_scan_select_string = self.context.evla['msinfo'][self.ms.name].delay_scan_select_string

        if (prefix == 'BPcal' or ((delay_scan_select_string != bandpass_scan_select_string) and prefix == 'delaycal')
or prefix == 'allcals' or prefix == 'allcals-vlass'):
            wrapper = logger.Plot(figfile, x_axis='freq', y_axis='amp',
                                  parameters={'vis': self.ms.basename,
                                              'type': prefix,
                                              'spw': ''})

            if not os.path.exists(figfile):
                LOG.trace('Checkflag summary plot not found. Creating new plot.')
                try:
                    self.create_plot(prefix)
                except Exception as ex:
                    LOG.error('Could not create ' + prefix + ' plot.')
                    LOG.exception(ex)
                    return None

            return wrapper

        return None
