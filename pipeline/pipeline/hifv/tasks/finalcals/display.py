from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
import casa
import numpy as np
import math

LOG = infrastructure.get_logger(__name__)


class finalcalsSummaryChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.basevis = os.path.basename(result.inputs['vis'])
        # self.caltable = result.final[0].gaintable

    def plot(self):
        # science_spws = self.ms.get_spectral_windows(science_windows_only=True)
        plots = [self.get_plot_wrapper()]
        return [p for p in plots if p is not None]

    def create_plot(self):
        figfile = self.get_figfile()

        casa.plotms(vis=self.result.ktypecaltable, xaxis='freq', yaxis='amp', field='',
                    antenna='0~2', spw='', timerange='',
                    plotrange=[], coloraxis='spw',
                    title='K table: finaldelay.tbl   Antenna: {!s}'.format('0~2'),
                    titlefont=8, xaxisfont=7, yaxisfont=7, showgui=False, plotfile=figfile)

    def get_figfile(self):
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            'finalcalsjunk'+'-%s-summary.png' % self.ms.basename)

    def get_plot_wrapper(self):
        figfile = self.get_figfile()

        wrapper = logger.Plot(figfile, x_axis='freq', y_axis='delay',
                              parameters={'vis'      : self.ms.basename,
                                          'type'     : 'finalcalsjunk',
                                          'spw'      : ''})

        if not os.path.exists(figfile):
            LOG.trace('finalcals summary plot not found. Creating new plot.')
            try:
                self.create_plot()
            except Exception as ex:
                LOG.error('Could not create finalcals plot.')
                LOG.exception(ex)
                return None

        return wrapper

    
class finalDelaysPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finaldelays-%s.json' % self.ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        plots = []
        nplots = len(m.antennas)

        LOG.info("Plotting finalDelay calibration tables")

        for ii in range(nplots):

            filename = 'finaldelay'+str(ii)+'.png'
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

                    LOG.debug("Plotting final calibration tables "+antName)

                    casa.plotms(vis=self.result.ktypecaltable, xaxis='freq', yaxis='amp', field='',
                                antenna=antPlot, spw='', timerange='',
                                plotrange=[], coloraxis='spw',
                                title='K table: finaldelay.tbl   Antenna: {!s}'.format(antName),
                                titlefont=8, xaxisfont=7, yaxisfont=7, showgui=False, plotfile=figfile)
                except Exception as ex:
                    LOG.warn("Unable to plot " + filename + str(ex))
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                real_figfile = figfile

                plot = logger.Plot(real_figfile, x_axis='Frequency', y_axis='Delay',field='',
                                   parameters={'spw': '',
                                               'pol': '',
                                               'ant': antName,
                                               'type': 'Final delay',
                                               'file': os.path.basename(real_figfile)})
                plots.append(plot)
            except Exception as ex:
                LOG.warn("Unable to add plot to stack." + str(ex))
                plots.append(None)

        return [p for p in plots if p is not None]


class finalphaseGainPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finalphasegain-%s.json' % self.ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)

        plots = []
        nplots=numAntenna

        LOG.info("Plotting final phase gain solutions")

        for ii in range(nplots):

            filename='finalBPinitialgainphase'+str(ii)+'.png'
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

                    LOG.debug("Plotting final phase gain solutions "+antName)
                    casa.plotms(vis=result.bpdgain_touse, xaxis='time', yaxis='phase', field='',
                                antenna=antPlot, spw='', timerange='',
                                coloraxis='spw', plotrange=[0, 0, -180, 180], symbolshape='circle',
                                title='G table: finalBPinitialgain.tbl   Antenna: {!s}'.format(antName),
                                titlefont=8, xaxisfont=7, yaxisfont=7, showgui=False, plotfile=figfile)

                except Exception as ex:
                    LOG.warn("Unable to plot " + filename + str(ex))
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                plot = logger.Plot(figfile, x_axis='Time', y_axis='Phase',field='',
                                   parameters={'spw': '',
                                               'pol': '',
                                               'ant': antName,
                                               'type': 'BP initial gain phase',
                                               'file': os.path.basename(figfile)})
                plots.append(plot)
            except Exception as ex:
                LOG.warn("Unable to add plot to stack.  " + str(ex))
                plots.append(None)

        return [p for p in plots if p is not None]


class finalbpSolAmpPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finalbpsolamp-%s.json' % self.ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)

        plots = []

        with casatools.TableReader(self.result.bpcaltable) as tb:
            dataVarCol = tb.getvarcol('CPARAM')
            flagVarCol = tb.getvarcol('FLAG')
    
        rowlist = dataVarCol.keys()
        maxmaxamp = 0.0
        maxmaxphase = 0.0
        for rrow in rowlist:
            dataArr = dataVarCol[rrow]
            flagArr = flagVarCol[rrow]
            amps = np.abs(dataArr)
            phases = np.arctan2(np.imag(dataArr),np.real(dataArr))
            good = np.logical_not(flagArr)
            tmparr = amps[good]
            if len(tmparr) > 0:
                maxamp = np.max(amps[good])
                if maxamp > maxmaxamp:
                    maxmaxamp = maxamp
            tmparr = np.abs(phases[good])
            if len(tmparr) > 0:
                maxphase = np.max(np.abs(phases[good]))*180./math.pi
                if maxphase > maxmaxphase:
                    maxmaxphase = maxphase
        ampplotmax = maxmaxamp

        nplots = numAntenna

        LOG.info("Plotting amp bandpass solutions")

        for ii in range(nplots):

            filename='finalBPcal_amp'+str(ii)+'.png'
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

                    LOG.debug("Plotting amp bandpass solutions "+antName)
                    casa.plotms(vis=self.result.bpcaltable, xaxis='freq', yaxis='amp', field='',
                                antenna=antPlot, spw='', timerange='',
                                coloraxis='spw', plotrange=[0, 0, 0, ampplotmax], symbolshape='circle',
                                title='B table: {!s}   Antenna: {!s}'.format('finalBPcal.tbl', antName),
                                titlefont=8, xaxisfont=7, yaxisfont=7, showgui=False, plotfile=figfile)
                except Exception as ex:
                    LOG.warn("Unable to plot " + filename + str(ex))
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                real_figfile = figfile
            
                plot = logger.Plot(real_figfile, x_axis='Freq', y_axis='Amp',field='',
                                   parameters={'spw': '',
                                               'pol': '',
                                               'ant': antName,
                                               'type': 'BP Amp solution',
                                               'file': os.path.basename(real_figfile)})
                plots.append(plot)
            except Exception as ex:
                LOG.warn("Unable to add plot to stack.  " + str(ex))
                plots.append(None)

        return [p for p in plots if p is not None]


class finalbpSolPhasePerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finalbpsolphase-%s.json' % self.ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)

        plots = []

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
        phaseplotmax = maxmaxphase

        nplots = numAntenna

        LOG.info("Plotting phase bandpass solutions")

        for ii in range(nplots):

            filename='finalBPcal_phase'+str(ii)+'.png'
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

                    LOG.debug("Plotting phase bandpass solutions "+antName)
                    casa.plotms(vis=self.result.bpcaltable, xaxis='freq', yaxis='phase', field='',
                                antenna=antPlot, spw='', timerange='',
                                coloraxis='spw', plotrange=[0, 0, -phaseplotmax, phaseplotmax],
                                symbolshape='circle',
                                title='B table: {!s}   Antenna: {!s}'.format('finalBPcal.tbl', antName),
                                titlefont=8, xaxisfont=7, yaxisfont=7, showgui=False, plotfile=figfile)
                except Exception as ex:
                    LOG.warn("Unable to plot " + filename + str(ex))
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                real_figfile = figfile
            
                plot = logger.Plot(real_figfile, x_axis='Freq', y_axis='Phase', field='',
                                   parameters={'spw': '',
                                               'pol': '',
                                               'ant': antName,
                                               'type': 'BP Phase solution',
                                               'file': os.path.basename(real_figfile)})
                plots.append(plot)
            except Exception as ex:
                LOG.warn("Unable to add plot to stack.  " + str(ex))
                plots.append(None)

        return [p for p in plots if p is not None]


class finalbpSolPhaseShortPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finalbpsolphaseshort-%s.json' % self.ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)
        plots = []
        
        nplots = numAntenna

        LOG.info("Plotting phase short gaincal")

        for ii in range(nplots):

            filename='phaseshortgaincal'+str(ii)+'.png'
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

                    LOG.debug("Plotting phase short gaincal "+antName)
                    casa.plotms(vis=self.result.phaseshortgaincaltable, xaxis='time', yaxis='phase', field='',
                                antenna=antPlot, spw='', timerange='',
                                coloraxis='spw', plotrange=[0, 0, -180, 180], symbolshape='circle',
                                title='G table: phaseshortgaincal.tbl   Antenna: {!s}'.format(antName),
                                titlefont=8, xaxisfont=7, yaxisfont=7, showgui=False, plotfile=figfile)
                except Exception as ex:
                    LOG.warn("Unable to plot " + filename + str(ex))
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                plot = logger.Plot(figfile, x_axis='Time', y_axis='Phase', field='',
                                   parameters={'spw': '',
                                               'pol': '',
                                               'ant': antName,
                                               'type': 'Phase (short) gain solution',
                                               'file': os.path.basename(figfile)})
                plots.append(plot)
            except Exception as ex:
                LOG.warn("Unable to add plot to stack.  " + str(ex))
                plots.append(None)

        return [p for p in plots if p is not None]


class finalAmpTimeCalPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finalamptimecal-%s.json' % self.ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)

        plots = []
        
        with casatools.TableReader(self.result.finalampgaincaltable) as tb:
            cpar = tb.getcol('CPARAM')
            flgs = tb.getcol('FLAG')
        amps = np.abs(cpar)
        good = np.logical_not(flgs)
        maxamp = np.max(amps[good])
        plotmax = max(2.0,maxamp)

        nplots = numAntenna

        LOG.info("Plotting final amp timecal")

        for ii in range(nplots):

            filename = 'finalamptimecal'+str(ii)+'.png'
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

                    LOG.debug("Plotting final amp timecal "+antName)
                    casa.plotms(vis=self.result.finalampgaincaltable, xaxis='time', yaxis='amp', field='',
                                antenna=antPlot, spw='', timerange='',
                                coloraxis='spw', plotrange=[0, 0, 0, plotmax], symbolshape='circle',
                                title='G table: finalampgaincal.tbl   Antenna: {!s}'.format(antName),
                                titlefont=8, xaxisfont=7, yaxisfont=7, showgui=False, plotfile=figfile)
                except Exception as ex:
                    LOG.warn("Unable to plot " + filename + str(ex))
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                plot = logger.Plot(figfile, x_axis='Time', y_axis='Amp', field='',
                                   parameters={'spw': '',
                                               'pol': '',
                                               'ant': antName,
                                               'type': 'Final amp time cal',
                                               'file': os.path.basename(figfile)})
                plots.append(plot)
            except Exception as ex:
                LOG.warn("Unable to add plot to stack.  " + str(ex))
                plots.append(None)

        return [p for p in plots if p is not None]


class finalAmpFreqCalPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finalampfreqcal-%s.json' % self.ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)

        plots = []

        with casatools.TableReader(self.result.finalampgaincaltable) as tb:
            cpar = tb.getcol('CPARAM')
            flgs = tb.getcol('FLAG')
        amps = np.abs(cpar)
        good = np.logical_not(flgs)
        maxamp = np.max(amps[good])
        plotmax = max(2.0,maxamp)

        nplots=numAntenna

        LOG.info("Plotting final amp freqcal")

        for ii in range(nplots):

            filename = 'finalampfreqcal'+str(ii)+'.png'
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

                    LOG.debug("Plotting final amp freqcal "+antName)
                    casa.plotms(vis=self.result.finalampgaincaltable, xaxis='freq', yaxis='amp', field='',
                                antenna=antPlot, spw='', timerange='',
                                coloraxis='spw', plotrange=[0, 0, 0, plotmax], symbolshape='circle',
                                title='G table: finalampgaincal.tbl   Antenna: {!s}'.format(antName),
                                titlefont=8, xaxisfont=7, yaxisfont=7, showgui=False, plotfile=figfile)
                    
                except Exception as ex:
                    LOG.warn("Unable to plot " + filename + str(ex))
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                real_figfile = figfile
            
                plot = logger.Plot(real_figfile, x_axis='freq', y_axis='Amp', field='',
                                   parameters={'spw': '',
                                               'pol': '',
                                               'ant': antName,
                                               'type': 'Final amp freq cal',
                                               'file': os.path.basename(real_figfile)})
                plots.append(plot)
            except Exception as ex:
                LOG.warn("Unable to add plot to stack.  " + str(ex))
                plots.append(None)

        return [p for p in plots if p is not None]


class finalPhaseGainCalPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finalphasegaincal-%s.json' % self.ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)

        plots = []

        nplots = numAntenna

        LOG.info("Plotting final phase freqcal")

        for ii in range(nplots):

            filename = 'finalphasegaincal'+str(ii)+'.png'
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

                    LOG.debug("Plotting final phase freqcal "+antName)
                    casa.plotms(vis=self.result.finalphasegaincaltable, xaxis='time', yaxis='phase', field='',
                                antenna=antPlot, spw='', timerange='',
                                coloraxis='spw', plotrange=[0, 0, -180, 180], symbolshape='circle',
                                title='G table: finalphasegaincal.tbl   Antenna: {!s}'.format(antName),
                                titlefont=8, xaxisfont=7, yaxisfont=7, showgui=False, plotfile=figfile)

                except Exception as ex:
                    LOG.warn("Problem with plotting " + filename + str(ex))
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                plot = logger.Plot(figfile, x_axis='time', y_axis='phase', field='',
                                   parameters={'spw': '',
                                               'pol': '',
                                               'ant': antName,
                                               'type': 'Final phase gain cal',
                                               'file': os.path.basename(figfile)})
                plots.append(plot)
            except Exception as ex:
                LOG.warn("Unable to add plot to stack.  " + str(ex))
                plots.append(None)

        return [p for p in plots if p is not None]

