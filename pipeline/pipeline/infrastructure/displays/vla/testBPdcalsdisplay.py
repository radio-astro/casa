from __future__ import absolute_import
import collections
import json
import os

import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from pipeline.infrastructure import casa_tasks
import casa
import numpy as np
import math

LOG = infrastructure.get_logger(__name__)


class testBPdcalsSummaryChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        #self.caltable = result.final[0].gaintable

    def plot(self):
        ##science_spws = self.ms.get_spectral_windows(science_windows_only=True)
        plots = [self.get_plot_wrapper('BPcal'), self.get_plot_wrapper('delaycal')]
        return [p for p in plots if p is not None]

    def create_plot(self, prefix):
        figfile = self.get_figfile(prefix)
        
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = context.evla['msinfo'][m.name].numAntenna
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        corrstring = context.evla['msinfo'][m.name].corrstring
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        calibrator_field_select_string = context.evla['msinfo'][m.name].calibrator_field_select_string
        field_ids = context.evla['msinfo'][m.name].field_ids
        field_names = context.evla['msinfo'][m.name].field_names
        channels = context.evla['msinfo'][m.name].channels
        
        ms_active = m.name


        if (prefix == 'BPcal'):
            casa.plotms(vis=ms_active, xaxis='freq', yaxis='amp', ydatacolumn='corrected',  selectdata=True, field=bandpass_field_select_string,    scan=bandpass_scan_select_string,  correlation=corrstring,  averagedata=True, avgtime='1e8s', avgscan=True, transform=False,    extendflag=False, iteraxis='', coloraxis='antenna2',  plotrange=[], title='', xlabel='',  ylabel='', showmajorgrid=False,    showminorgrid=False, plotfile=figfile, overwrite=True)


        if ((delay_scan_select_string != bandpass_scan_select_string) and prefix == 'delaycal'):
            casa.plotms(vis=ms_active, xaxis='freq',   yaxis='amp',  ydatacolumn='corrected',  selectdata=True,  scan=delay_scan_select_string,        correlation=corrstring,  averagedata=True,  avgtime='1e8s',  avgscan=True, transform=False,  extendflag=False,  iteraxis='',        coloraxis='antenna2',  plotrange=[],  title='',  xlabel='',  ylabel='',  showmajorgrid=False,  showminorgrid=False,        plotfile=figfile, overwrite=True)


    def get_figfile(self, prefix):
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            'testcalibrated'+prefix+'-%s-summary.png' % self.ms.basename)

    def get_plot_wrapper(self, prefix):
        figfile = self.get_figfile(prefix)
        
        context = self.context
        m = context.observing_run.measurement_sets[0]
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string


        if (prefix == 'BPcal' or ((delay_scan_select_string != bandpass_scan_select_string) and prefix == 'delaycal')):
            wrapper = logger.Plot(figfile,
                              x_axis='freq',
                              y_axis='amp',
                              parameters={'vis'      : self.ms.basename,
                                          'type'     : prefix,
                                          'spw'      : ''})

            if not os.path.exists(figfile):
                LOG.trace('testBPdcals summary plot not found. Creating new '
                          'plot.')
                try:
                    self.create_plot(prefix)
                except Exception as ex:
                    LOG.error('Could not create '+prefix+ ' plot.')
                    LOG.exception(ex)
                    return None
            
            return wrapper
        
        return None
    
    
class testDelaysPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        ms = self.ms
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'testdelays-%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = context.evla['msinfo'][m.name].numAntenna
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        corrstring = context.evla['msinfo'][m.name].corrstring
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        calibrator_field_select_string = context.evla['msinfo'][m.name].calibrator_field_select_string
        field_ids = context.evla['msinfo'][m.name].field_ids
        field_names = context.evla['msinfo'][m.name].field_names
        channels = context.evla['msinfo'][m.name].channels
        
        ms_active = m.name
        
        plots = []
        
        LOG.info("Plotting test delays")
    
	nplots=int(numAntenna/3)
	
	if ((numAntenna%3)>0):
	    nplots = nplots + 1
	
	for ii in range(nplots):
	
	    filename='testdelay'+str(ii)+'.png'
	    ####syscommand='rm -rf '+filename
	     ####os.system(syscommand)
	    antPlot=str(ii*3)+'~'+str(ii*3+2)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)
	
	    if not os.path.exists(figfile):
	        try:
	            casa.plotcal(caltable='testdelay.k', xaxis='freq', yaxis='delay', poln='',  field='', antenna=antPlot, spw='', timerange='', subplot=311, overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[], showflags=False, plotsymbol='o', plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=figfile)
	            #plots.append(figfile)

	        except:
	            LOG.warn("Unable to plot " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                plot = logger.Plot(figfile, x_axis='Frequency', y_axis='Delay',
		        field='',
                        parameters={ 'spw': '',
                        'pol': '',
                        'ant': antPlot,
                        'type': 'testdelay',
                        'file': os.path.basename(figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]

class ampGainPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        ms = self.ms
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'ampgain-%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = context.evla['msinfo'][m.name].numAntenna
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        corrstring = context.evla['msinfo'][m.name].corrstring
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        calibrator_field_select_string = context.evla['msinfo'][m.name].calibrator_field_select_string
        field_ids = context.evla['msinfo'][m.name].field_ids
        field_names = context.evla['msinfo'][m.name].field_names
        channels = context.evla['msinfo'][m.name].channels
        
        ms_active = m.name
        
        plots = []
        
        LOG.info("Plotting amplitude gain solutions")
    
	nplots=int(numAntenna/3)
	
	with casatools.TableReader('testBPdinitialgain.g') as tb:
            cpar=tb.getcol('CPARAM')
            flgs=tb.getcol('FLAG')
        amps=np.abs(cpar)
        good=np.logical_not(flgs)
        maxamp=np.max(amps[good])
        plotmax=maxamp
	
	
	if ((numAntenna%3)>0):
	    nplots = nplots + 1
	
	for ii in range(nplots):
	
	    filename='testBPdinitialgainamp'+str(ii)+'.png'
	    ####syscommand='rm -rf '+filename
	     ####os.system(syscommand)
	    antPlot=str(ii*3)+'~'+str(ii*3+2)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)
	
	    if not os.path.exists(figfile):
	        try:
	            casa.plotcal(caltable='testBPdinitialgain.g', xaxis='time', yaxis='amp', poln='', field='', antenna=antPlot, spw='', timerange='', subplot=311, overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[0,0,0,plotmax], showflags=False, plotsymbol='o',plotcolor='blue',markersize=5.0,fontsize=10.0,showgui=False,figfile=figfile)
	            #plots.append(figfile)

	        except:
	            LOG.warn("Unable to plot " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                plot = logger.Plot(figfile, x_axis='Time', y_axis='Amp',
		        field='',
                        parameters={ 'spw': '',
                        'pol': '',
                        'ant': antPlot,
                        'type': 'ampgain',
                        'file': os.path.basename(figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]



class phaseGainPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        ms = self.ms
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'phasegain-%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = context.evla['msinfo'][m.name].numAntenna
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        corrstring = context.evla['msinfo'][m.name].corrstring
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        calibrator_field_select_string = context.evla['msinfo'][m.name].calibrator_field_select_string
        field_ids = context.evla['msinfo'][m.name].field_ids
        field_names = context.evla['msinfo'][m.name].field_names
        channels = context.evla['msinfo'][m.name].channels
        
        ms_active = m.name
        
        plots = []
        
        LOG.info("Plotting phase gain solutions")
    
	nplots=int(numAntenna/3)
	
	with casatools.TableReader('testBPdinitialgain.g') as tb:
            cpar=tb.getcol('CPARAM')
            flgs=tb.getcol('FLAG')
        amps=np.abs(cpar)
        good=np.logical_not(flgs)
        maxamp=np.max(amps[good])
        plotmax=maxamp
	
	
	if ((numAntenna%3)>0):
	    nplots = nplots + 1
	
	for ii in range(nplots):
	
	    filename='testBPdinitialgainphase'+str(ii)+'.png'
	    ####syscommand='rm -rf '+filename
	     ####os.system(syscommand)
	    antPlot=str(ii*3)+'~'+str(ii*3+2)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)
	
	    if not os.path.exists(figfile):
	        try:
	            casa.plotcal(caltable='testBPdinitialgain.g', xaxis='time', yaxis='phase', poln='', field='',  antenna=antPlot, spw='', timerange='',        subplot=311, overplot=False, clearpanel='Auto', iteration='antenna',  plotrange=[0,0,-180,180], showflags=False, plotsymbol='o-',        plotcolor='blue',  markersize=5.0, fontsize=10.0, showgui=False, figfile=figfile)
	            #plots.append(figfile)

	        except:
	            LOG.warn("Unable to plot " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                plot = logger.Plot(figfile, x_axis='Time', y_axis='Phase',
		        field='',
                        parameters={ 'spw': '',
                        'pol': '',
                        'ant': antPlot,
                        'type': 'phasegain',
                        'file': os.path.basename(figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]




class bpSolAmpPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        ms = self.ms
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'bpsolamp-%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = context.evla['msinfo'][m.name].numAntenna
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        corrstring = context.evla['msinfo'][m.name].corrstring
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        calibrator_field_select_string = context.evla['msinfo'][m.name].calibrator_field_select_string
        field_ids = context.evla['msinfo'][m.name].field_ids
        field_names = context.evla['msinfo'][m.name].field_names
        channels = context.evla['msinfo'][m.name].channels
        
        ms_active = m.name
        
        plots = []
        
        LOG.info("Plotting amp bandpass solutions")
    
	nplots=int(numAntenna/3)
	
	with casatools.TableReader('testBPdinitialgain.g') as tb:
            cpar=tb.getcol('CPARAM')
            flgs=tb.getcol('FLAG')
        amps=np.abs(cpar)
        good=np.logical_not(flgs)
        maxamp=np.max(amps[good])
        plotmax=maxamp
        
        
        with casatools.TableReader('testBPcal.b') as tb:
            dataVarCol = tb.getvarcol('CPARAM')
            flagVarCol = tb.getvarcol('FLAG')
    
        rowlist = dataVarCol.keys()
        nrows = len(rowlist)
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
        phaseplotmax=maxmaxphase
        
	
	
	if ((numAntenna%3)>0):
	    nplots = nplots + 1
	
	for ii in range(nplots):
	
	    filename='testBPcal_amp'+str(ii)+'.png'
	    ####syscommand='rm -rf '+filename
	     ####os.system(syscommand)
	    antPlot=str(ii*3)+'~'+str(ii*3+2)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)
	
	    if not os.path.exists(figfile):
	        try:
	            casa.plotcal(caltable='testBPcal.b', xaxis='freq',  yaxis='amp', poln='', field='', antenna=antPlot, spw='', timerange='', subplot=311,      overplot=False, clearpanel='Auto', iteration='antenna',  plotrange=[0,0,0,ampplotmax],  showflags=False, plotsymbol='o',        plotcolor='blue',  markersize=5.0, fontsize=10.0, showgui=False, figfile=figfile)
	            #plots.append(figfile)

	        except:
	            LOG.warn("Unable to plot " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                plot = logger.Plot(figfile, x_axis='Freq', y_axis='Amp',
		        field='',
                        parameters={ 'spw': '',
                        'pol': '',
                        'ant': antPlot,
                        'type': 'bpsolamp',
                        'file': os.path.basename(figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]


class bpSolPhasePerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        ms = self.ms
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'bpsolphase-%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = context.evla['msinfo'][m.name].numAntenna
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        corrstring = context.evla['msinfo'][m.name].corrstring
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        calibrator_field_select_string = context.evla['msinfo'][m.name].calibrator_field_select_string
        field_ids = context.evla['msinfo'][m.name].field_ids
        field_names = context.evla['msinfo'][m.name].field_names
        channels = context.evla['msinfo'][m.name].channels
        
        ms_active = m.name
        
        plots = []
        
        LOG.info("Plotting phase bandpass solutions")
    
	nplots=int(numAntenna/3)
	
	with casatools.TableReader('testBPdinitialgain.g') as tb:
            cpar=tb.getcol('CPARAM')
            flgs=tb.getcol('FLAG')
        amps=np.abs(cpar)
        good=np.logical_not(flgs)
        maxamp=np.max(amps[good])
        plotmax=maxamp
        
        
        with casatools.TableReader('testBPcal.b') as tb:
            dataVarCol = tb.getvarcol('CPARAM')
            flagVarCol = tb.getvarcol('FLAG')
    
        rowlist = dataVarCol.keys()
        nrows = len(rowlist)
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
        phaseplotmax=maxmaxphase
        
	
	
	if ((numAntenna%3)>0):
	    nplots = nplots + 1
	
	for ii in range(nplots):
	
	    filename='testBPcal_phase'+str(ii)+'.png'
	    ####syscommand='rm -rf '+filename
	     ####os.system(syscommand)
	    antPlot=str(ii*3)+'~'+str(ii*3+2)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)
	
	    if not os.path.exists(figfile):
	        try:
	            casa.plotcal(caltable='testBPcal.b',  xaxis='freq', yaxis='phase', poln='',  field='',  antenna=antPlot, spw='',  timerange='',      subplot=311,  overplot=False, clearpanel='Auto', iteration='antenna',  plotrange=[0,0,-phaseplotmax,phaseplotmax], showflags=False,        plotsymbol='o', plotcolor='blue',  markersize=5.0, fontsize=10.0,  showgui=False,  figfile=figfile)
	            #plots.append(figfile)

	        except:
	            LOG.warn("Unable to plot " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                plot = logger.Plot(figfile, x_axis='Freq', y_axis='Phase',
		        field='',
                        parameters={ 'spw': '',
                        'pol': '',
                        'ant': antPlot,
                        'type': 'bpsolphase',
                        'file': os.path.basename(figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]


##TsysStat = collections.namedtuple('TsysScore', 'median rms median_max')
'''
class ScoringtestBPdcalsPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.plotter = testBPdcalsPerAntennaChart(context, result)
        ms = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'testBPdcals-%s.json' % ms)
        
    def plot(self):
        plots = self.plotter.plot()
            
        return plots
'''