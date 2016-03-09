from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
import casa

LOG = infrastructure.get_logger(__name__)


class plotsummarySummaryChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        #self.caltable = result.final[0].gaintable

    def plot(self):
        plots = []
        context = self.context
        result = self.result
        
        m = context.observing_run.measurement_sets[0]
        numAntenna = len(m.antennas)
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        corrstring = m.get_vla_corrstring()
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        calibrator_field_select_string = context.evla['msinfo'][m.name].calibrator_field_select_string
        field_ids = m.get_vla_field_ids()
        field_names = m.get_vla_field_names()
        channels = m.get_vla_numchan()
    
        ms_active=m.name
        
        #create phase time plot for all calibrators
        figfile = self.get_figfile('all_calibrators_phase_time')
                            
        plot = logger.Plot(figfile, x_axis='time', y_axis='phase',
                          parameters={'vis'      : self.ms.basename,
                                      'type'     : 'All calibrators',
                                      'spw'      : ''})

        if not os.path.exists(figfile):
            LOG.trace('Plotting phase vs. time for all calibrators. Creating new '
                      'plot.')
            try:
                casa.plotms(vis=ms_active, xaxis='time', yaxis='phase', ydatacolumn='corrected', selectdata=True,
                            field=calibrator_field_select_string,    correlation=corrstring, averagedata=True,
                            avgchannel=str(max(channels)), avgtime='1e8s', avgscan=False, transform=False,
                            extendflag=False,iteraxis='', coloraxis='antenna2', plotrange=[],
                            title='Calibrated phase vs. time, all calibrators', xlabel='',  ylabel='',
                            showmajorgrid=False,  showminorgrid=False, plotfile=figfile, overwrite=True,
                            clearplots=True, showgui=False)

            except Exception as ex:
                LOG.error('Could not create fluxboot plot.')
                LOG.exception(ex)
                plot = None
        
        plots.append(plot)
        
        #create amp vs. UVwave plots of each field
        for ii in field_ids:
            figfile = self.get_figfile('field'+str(field_ids[ii])+'_amp_uvdist')
            #figfile = self.get_figfile('targetflag')
            
            plot = logger.Plot(figfile, x_axis='uvwave', y_axis='amp',
                          parameters={'vis'      : self.ms.basename,
                                      'type'     : 'Field '+str(field_ids[ii])+', '+field_names[ii],
                                      'field'    : str(field_ids[ii]),
                                      'spw'      : ''})
                                      
            if not os.path.exists(figfile):
                LOG.trace('Plotting amp vs. uvwave for field id='+str(field_ids[ii])+'.  Creating new '
                      'plot.')
                      
                try:
                    casa.plotms(vis=ms_active,  xaxis='uvwave',  yaxis='amp',  ydatacolumn='corrected',
                                selectdata=True, field=str(field_ids[ii]),  correlation=corrstring,
                                averagedata=True,    avgchannel=str(max(channels)),   avgtime='1e8s',
                                avgscan=False,   transform=False,        extendflag=False,   iteraxis='',
                                coloraxis='spw',  plotrange=[],
                                title='Field '+str(field_ids[ii])+', '+field_names[ii],   xlabel='',
                                ylabel='',  showmajorgrid=False,  showminorgrid=False,  plotfile=figfile,
                                overwrite=True, clearplots=True, showgui=False)

                except Exception as ex:
                    LOG.error('Could not create plot for field '+str(field_ids[ii]))
                    LOG.exception(ex)
                    plot = None
                    
            plots.append(plot)

        return [p for p in plots if p is not None]

    def get_figfile(self, prefix):
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            prefix+'-%s-summary.png' % self.ms.basename)