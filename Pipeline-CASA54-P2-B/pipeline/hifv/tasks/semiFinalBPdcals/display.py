from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
import casa
import numpy as np
import math

LOG = infrastructure.get_logger(__name__)


class semifinalBPdcalsSummaryChart(object):
    def __init__(self, context, result, suffix=''):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.suffix = suffix
        # self.caltable = result.final[0].gaintable

    def plot(self):
        # science_spws = self.ms.get_spectral_windows(science_windows_only=True)
        plots = [self.get_plot_wrapper()]
        return [p for p in plots if p is not None]

    def create_plot(self):
        figfile = self.get_figfile()
        
        context = self.context
        m = context.observing_run.measurement_sets[0]
        
        corrstring = m.get_vla_corrstring()
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string

        casa.plotms(vis=m.name, xaxis='freq', yaxis='amp', ydatacolumn='corrected', selectdata=True,
                    scan=calibrator_scan_select_string, correlation=corrstring, averagedata=True, avgtime='1e8',
                    avgscan=False, transform=False,    extendflag=False,iteraxis='',  coloraxis='antenna2',
                    plotrange=[], title='', xlabel='', ylabel='', showmajorgrid=False,    showminorgrid=False,
                    plotfile=figfile, overwrite=True, clearplots=True, showgui=False)

    def get_figfile(self):
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            'semifinalcalibrated_'+self.suffix+'-%s-summary.png' % self.ms.basename)

    def get_plot_wrapper(self):
        figfile = self.get_figfile()
        wrapper = logger.Plot(figfile, x_axis='freq', y_axis='amp',
                              parameters={'vis'      : self.ms.basename,
                                          'type'     : 'semifinalcalibratedcals'+self.suffix,
                                          'spw'      : ''})

        if not os.path.exists(figfile):
            LOG.trace('semifinalBPdcals summary plot not found. Creating new plot.')
            try:
                self.create_plot()
            except Exception as ex:
                LOG.error('Could not create plot.')
                LOG.exception(ex)
                return None
        return wrapper

    
class DelaysPerAntennaChart(object):
    def __init__(self, context, result, suffix=''):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.suffix=suffix
        ms = self.ms
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'delays-'+self.suffix+'%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)

        plots = []

        nplots = numAntenna

        LOG.info("Plotting semiFinal delays")

        for ii in range(nplots):
            filename='delay'+str(ii)+'_'+self.suffix+'.png'
            antPlot=str(ii)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)

            if not os.path.exists(figfile):
                try:

                    # Get antenna name
                    antName = antPlot
                    if antPlot != '':
                        domain_antennas = self.ms.get_antenna(antPlot)
                        idents = [a.name if a.name else a.id for a in domain_antennas]
                        antName = ','.join(idents)

                    LOG.debug("Plotting semiFinal delays "+antName)

                    casa.plotms(vis=self.result.ktypecaltable, xaxis='freq', yaxis='amp', field='',
                                antenna=antPlot, spw='', timerange='',
                                plotrange=[], coloraxis='spw',
                                title='K table: delay.tbl   Antenna: {!s}'.format(antName),
                                titlefont=8, xaxisfont=7, yaxisfont=7, showgui=False, plotfile=figfile)

                except:
                    LOG.warn("Unable to plot " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                plot = logger.Plot(figfile, x_axis='Frequency', y_axis='Delay', field='',
                                   parameters={'spw': '',
                                               'pol': '',
                                               'ant': antName,
                                               'type': 'delay'+self.suffix,
                                               'file': os.path.basename(figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]


class semifinalphaseGainPerAntennaChart(object):
    def __init__(self, context, result, suffix=''):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.suffix = suffix
        ms = self.ms

        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'phasegain-'+self.suffix+'%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)

        plots = []
        nplots=numAntenna

        LOG.info("Plotting phase gain solutions")

        for ii in range(nplots):
            filename = 'BPinitialgainphase'+str(ii)+'_'+self.suffix+'.png'
            antPlot = str(ii)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)

            if not os.path.exists(figfile):
                try:
                    # Get antenna name
                    antName = antPlot
                    if antPlot != '':
                        domain_antennas = self.ms.get_antenna(antPlot)
                        idents = [a.name if a.name else a.id for a in domain_antennas]
                        antName = ','.join(idents)

                    LOG.debug("Plotting phase gain solutions "+antName)

                    casa.plotms(vis=result.bpdgain_touse, xaxis='time', yaxis='phase', field='',
                                antenna=antPlot, spw='', timerange='',
                                coloraxis='spw', plotrange=[0, 0, -180, 180], symbolshape='circle',
                                title='G table: {!s}   Antenna: {!s}'.format(result.bpdgain_touse, antName),
                                titlefont=8, xaxisfont=7, yaxisfont=7, showgui=False, plotfile=figfile)

                except:
                    LOG.warn("Unable to plot " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:

                plot = logger.Plot(figfile, x_axis='Time', y_axis='Phase', field='',
                                   parameters={'spw': '',
                                               'pol': '',
                                               'ant': antName,
                                               'type': 'phasegain'+self.suffix,
                                               'file': os.path.basename(figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]


class semifinalbpSolAmpPerAntennaChart(object):
    def __init__(self, context, result, suffix=''):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.suffix = suffix
        ms = self.ms
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'bpsolamp-'+self.suffix+'%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)

        plots = []
        
        LOG.info("Plotting amp bandpass solutions")

        with casatools.TableReader(self.result.bpcaltable) as tb:
            dataVarCol = tb.getvarcol('CPARAM')
            flagVarCol = tb.getvarcol('FLAG')
    
        rowlist = dataVarCol.keys()
        maxmaxamp = 0.0
        maxmaxphase = 0.0
        for rrow in rowlist:
            dataArr = dataVarCol[rrow]
            flagArr = flagVarCol[rrow]
            amps=np.abs(dataArr)
            phases=np.arctan2(np.imag(dataArr),np.real(dataArr))
            good=np.logical_not(flagArr)
            tmparr=amps[good]
            if (len(tmparr)>0):
                maxamp=np.max(amps[good])
                if (maxamp>maxmaxamp):
                    maxmaxamp=maxamp
            tmparr=np.abs(phases[good])
            if (len(tmparr)>0):
                maxphase=np.max(np.abs(phases[good]))*180./math.pi
                if (maxphase>maxmaxphase):
                    maxmaxphase=maxphase
        ampplotmax=maxmaxamp

        nplots=numAntenna

        for ii in range(nplots):
            filename = 'BPcal_amp'+str(ii)+'_'+self.suffix+'.png'
            antPlot = str(ii)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)

            if not os.path.exists(figfile):
                try:
                    # Get antenna name
                    antName = antPlot
                    if antPlot != '':
                        domain_antennas = self.ms.get_antenna(antPlot)
                        idents = [a.name if a.name else a.id for a in domain_antennas]
                        antName = ','.join(idents)

                    casa.plotms(vis=self.result.bpcaltable, xaxis='freq', yaxis='amp', field='',
                                antenna=antPlot, spw='', timerange='',
                                coloraxis='spw', plotrange=[0, 0, 0, ampplotmax], symbolshape='circle',
                                title='B table: {!s}   Antenna: {!s}'.format('BPcal.b', antName),
                                titlefont=8, xaxisfont=7, yaxisfont=7, showgui=False, plotfile=figfile)
                except:
                    LOG.warn("Unable to plot " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                plot = logger.Plot(figfile, x_axis='Freq', y_axis='Amp', field='',
                                   parameters={'spw': '',
                                               'pol': '',
                                               'ant': antName,
                                               'type': 'bpsolamp'+self.suffix,
                                               'file': os.path.basename(figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]


class semifinalbpSolPhasePerAntennaChart(object):
    def __init__(self, context, result, suffix=''):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.suffix = suffix
        ms = self.ms
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'bpsolphase-'+self.suffix+'%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)

        plots = []
        
        LOG.info("Plotting phase bandpass solutions")

        with casatools.TableReader(self.result.bpcaltable) as tb:
            dataVarCol = tb.getvarcol('CPARAM')
            flagVarCol = tb.getvarcol('FLAG')
    
        rowlist = dataVarCol.keys()
        maxmaxamp = 0.0
        maxmaxphase = 0.0
        for rrow in rowlist:
            dataArr = dataVarCol[rrow]
            flagArr = flagVarCol[rrow]
            amps=np.abs(dataArr)
            phases=np.arctan2(np.imag(dataArr),np.real(dataArr))
            good=np.logical_not(flagArr)
            tmparr=amps[good]
            if (len(tmparr)>0):
                maxamp=np.max(amps[good])
                if (maxamp>maxmaxamp):
                    maxmaxamp=maxamp
            tmparr=np.abs(phases[good])
            if (len(tmparr)>0):
                maxphase=np.max(np.abs(phases[good]))*180./math.pi
                if (maxphase>maxmaxphase):
                    maxmaxphase=maxphase
        phaseplotmax=maxmaxphase

        nplots=numAntenna

        for ii in range(nplots):
            filename = 'BPcal_phase'+str(ii)+'_'+self.suffix+'.png'
            antPlot = str(ii)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)

            if not os.path.exists(figfile):
                try:
                    # Get antenna name
                    antName = antPlot
                    if antPlot != '':
                        domain_antennas = self.ms.get_antenna(antPlot)
                        idents = [a.name if a.name else a.id for a in domain_antennas]
                        antName = ','.join(idents)

                    casa.plotms(vis=self.result.bpcaltable, xaxis='freq', yaxis='phase', field='',
                                antenna=antPlot, spw='', timerange='',
                                coloraxis='spw', plotrange=[0, 0, -phaseplotmax, phaseplotmax],
                                symbolshape='circle',
                                title='B table: {!s}   Antenna: {!s}'.format('BPcal.tbl', antName),
                                titlefont=8, xaxisfont=7, yaxisfont=7, showgui=False, plotfile=figfile)
                except:
                    LOG.warn("Unable to plot " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                plot = logger.Plot(figfile, x_axis='Freq', y_axis='Phase', field='',
                                   parameters={'spw': '',
                                               'pol': '',
                                               'ant': antName,
                                               'type': 'bpsolphase'+self.suffix,
                                               'file': os.path.basename(figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)
                
        # Get BPcal.b to close...
        '''
        stage = 'stage%s' % result.stage_number
        stage_dir = os.path.join(context.report_dir, stage)
        # construct the relative filename, eg. 'stageX/testdelay0.png'
        figfile = os.path.join(stage_dir, 'junk.png')
        casa.plotcal(caltable='testBPcal.b', xaxis='freq', yaxis='phase',
                     poln='', field='', antenna='0', spw='',        timerange='',
                     subplot=311,  overplot=False, clearpanel='Auto', iteration='antenna',
                     plotrange=[0,0,-180,180],        showflags=False, plotsymbol='o',
                     plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=figfile)
        '''

        return [p for p in plots if p is not None]

