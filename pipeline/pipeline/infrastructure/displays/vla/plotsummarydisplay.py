from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
import casa
import itertools
import collections

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
                            avgchannel=str(max(channels)), avgtime='1e8', avgscan=False, transform=False,
                            extendflag=False,iteraxis='', coloraxis='antenna2', plotrange=[],
                            title='Calibrated phase vs. time, all calibrators', xlabel='',  ylabel='',
                            showmajorgrid=False,  showminorgrid=False, plotfile=figfile, overwrite=True,
                            clearplots=True, showgui=False)

            except Exception as ex:
                LOG.error('Could not create plotsummary plot.')
                LOG.exception(ex)
                plot = None
        
        plots.append(plot)
        
        # create amp vs. UVwave plots of each cal field and then max 30 targets

        calfields = m.get_fields(intent='BANDPASS,PHASE,AMPLITUDE')
        alltargetfields = m.get_fields(intent='TARGET')

        plotfields = calfields

        # Nplots = 30
        # if len(alltargetfields) > 100 and len(alltargetfields) < 1000: Nplots = 30
        # if len(alltargetfields) > 1000: Nplots = 1000

        Nplots = (len(alltargetfields)/30)+1

        targetfields = [field for field in alltargetfields[0:len(alltargetfields):Nplots]]

        plotfields.extend(targetfields)

        # get IDs for all science spectral windows
        intent = 'TARGET'
        ms = m
        spw_ids = set()
        for scan in ms.get_scans(scan_intent=intent):
            scan_spw_ids = set([dd.spw.id for dd in scan.data_descriptions])
            spw_ids.update(scan_spw_ids)

        if intent == 'TARGET':
            science_ids = set([spw.id for spw in ms.get_spectral_windows()])
            spw_ids = spw_ids.intersection(science_ids)

        by_source_id = lambda field: field.source.id
        fields_by_source_id = sorted(ms.get_fields(intent=intent),
                                     key=by_source_id)

        resultfields = collections.OrderedDict()

        for source_id, source_fields in itertools.groupby(fields_by_source_id, by_source_id):
            fields = list(source_fields)

            field = fields[0]
            resultfields[source_id] = field

        Nplots = (len(resultfields.items())/30)+1

        # for field in plotfields:
        for source_id, brightest_field in resultfields.items()[0:len(resultfields.items()):Nplots]:


            for field in [brightest_field.id]:

                figfile = self.get_figfile('field'+str(source_id)+'_amp_uvdist')
                #figfile = self.get_figfile('targetflag')

                plot = logger.Plot(figfile, x_axis='uvwave', y_axis='amp',
                              parameters={'vis'      : self.ms.basename,
                                          'type'     : 'Field '+str(source_id)+', '+str(m.get_fields(field_id=field)[0].name),
                                          'field'    : str(field),
                                          'spw'      : ''})

                if not os.path.exists(figfile):
                    LOG.trace('Plotting amp vs. uvwave for field id='+str(source_id)+'.  Creating new '
                          'plot.')

                    try:
                        LOG.info("PLOTSUMMARYPLOTTING: "+'Field '+str(source_id)+', '+str(m.get_fields(field_id=field)[0].name))
                        casa.plotms(vis=ms_active,  xaxis='uvwave',  yaxis='amp',  ydatacolumn='corrected',
                                    selectdata=True, field=str(field),  correlation=corrstring,
                                    averagedata=True,    avgchannel=str(max(channels)),   avgtime='1e8',
                                    avgscan=False,   transform=False,        extendflag=False,   iteraxis='',
                                    coloraxis='spw',  plotrange=[],
                                    title='Field '+str(source_id)+', '+str(m.get_fields(field_id=field)[0].name),   xlabel='',
                                    ylabel='',  showmajorgrid=False,  showminorgrid=False,  plotfile=figfile,
                                    overwrite=True, clearplots=True, showgui=False)

                    except Exception as ex:
                        LOG.error('Could not create plot for field '+str(field))
                        LOG.exception(ex)
                        plot = None

                plots.append(plot)


        return [p for p in plots if p is not None]

    def get_figfile(self, prefix):
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            prefix+'-%s-summary.png' % self.ms.basename)