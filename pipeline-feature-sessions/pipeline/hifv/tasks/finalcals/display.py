from __future__ import absolute_import
import collections
import json
import os
import shutil

import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from pipeline.infrastructure import casa_tasks
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
        #self.caltable = result.final[0].gaintable

    def plot(self):
        ##science_spws = self.ms.get_spectral_windows(science_windows_only=True)
        plots = [self.get_plot_wrapper()]
        return [p for p in plots if p is not None]

    def create_plot(self):
        figfile = self.get_figfile()
        
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        #numAntenna = context.evla['msinfo'][m.name].numAntenna
        numAntenna = len(m.antennas)
        
        ms_active = m.name

        '''
        task_args = {'vis'         : self.ms.name,
                                 'caltable'    : 'finaldelay.k',
                                 'xaxis'       : 'freq',
                                 'yaxis'       : 'phase',
                                 'interactive' : False,
                                 'spw'         : '',
                                 #'antenna'     : antPlot,
                                 'showatm'     : False,
                                 'showfdm'     : False,
                                 'showlines'   : True,
                                 'subplot'     : 11,
                                 'plotrange'   : [],
                                 'figfile'     : figfile}
                    
        task = casa_tasks.plotbandpass(**task_args)
        task.execute(dry_run=False)
        '''
        
        casa.plotcal(caltable=self.basevis+'.finaldelay.k', xaxis='freq', yaxis='delay', poln='',  field='', antenna='0~2', spw='', timerange='', subplot=311, overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[], showflags=False, plotsymbol='o', plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=figfile)
        


    def get_figfile(self):
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            'finalcalsjunk'+'-%s-summary.png' % self.ms.basename)

    def get_plot_wrapper(self):
        figfile = self.get_figfile()
        
        context = self.context
        m = context.observing_run.measurement_sets[0]
        
        root, ext = os.path.splitext(figfile)
        #real_figfile = '%s.%s.spw%0.2d.%s%s' % (root, 'ea00', 00, 't00',ext)

	wrapper = logger.Plot(figfile, x_axis='freq', y_axis='delay',
			    parameters={'vis'      : self.ms.basename,
					'type'     : 'finalcalsjunk',
					'spw'      : ''})

	if not os.path.exists(figfile):
	    LOG.trace('finalcals summary plot not found. Creating new '
			'plot.')
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
        ms = self.ms
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finaldelays-%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        #numAntenna = context.evla['msinfo'][m.name].numAntenna
        numAntenna = len(m.antennas)
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        #corrstring = context.evla['msinfo'][m.name].corrstring
        #corrstring = m.get_vla_corrstring()
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        calibrator_field_select_string = context.evla['msinfo'][m.name].calibrator_field_select_string
        #field_ids = context.evla['msinfo'][m.name].field_ids
        field_ids = m.get_vla_field_ids()
        #field_names = context.evla['msinfo'][m.name].field_names
        field_names = m.get_vla_field_names()
        #channels = context.evla['msinfo'][m.name].channels
        channels = m.get_vla_numchan()
        
        ms_active = m.name
        
        plots = []
        
        
    
	nplots=int(numAntenna/3)
	
	if ((numAntenna%3)>0):
	    nplots = nplots + 1
	    
	nplots=numAntenna
	
	for ii in range(nplots):
	
	    filename='finaldelay'+str(ii)+'.png'
	    ####syscommand='rm -rf '+filename
	     ####os.system(syscommand)
	    antPlot=str(ii*3)+'~'+str(ii*3+2)
	    antPlot=str(ii)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)
	
	    if not os.path.exists(figfile):
	        try:
	        
	            LOG.info("Plotting final calibration tables")
	        
	            '''
	            task_args = {'vis'         : self.ms.name,
                                 'caltable'    : 'finaldelay.k',
                                 'xaxis'       : 'freq',
                                 'yaxis'       : 'phase',
                                 'interactive' : False,
                                 'spw'         : '',
                                 'antenna'     : antPlot,
                                 'showatm'     : False,
                                 'showfdm'     : False,
                                 'subplot'     : 11,
                                 'plotrange'   : [],
                                 'figfile'     : figfile}
                    
                    task = casa_tasks.plotbandpass(**task_args)
                    task.execute(dry_run=False)
	           '''
	        
	        
	            
	            casa.plotcal(caltable=self.basevis+'.finaldelay.k', xaxis='freq', yaxis='delay', poln='',  field='', antenna=antPlot, spw='', timerange='', subplot=111, overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[], showflags=False, plotsymbol='o', plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=figfile)
	            
	            #plots.append(figfile)

	        except:
	            LOG.warn("Unable to plot " + filename)
	            #print " "
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                #Get antenna name
                antName = antPlot
                if antPlot != '':
                    domain_antennas = self.ms.get_antenna(antPlot)
                    idents = [a.name if a.name else a.id for a in domain_antennas]
                    antName = ','.join(idents)
            
                root, ext = os.path.splitext(figfile)
                #real_figfile = '%s.%s.spw%0.2d.%s%s' % (root, antName, 00, 't00',ext)
                real_figfile = figfile
            
            
                plot = logger.Plot(real_figfile, x_axis='Frequency', y_axis='Delay',
		        field='',
                        parameters={ 'spw': '',
                        'pol': '',
                        'ant': antName,
                        'type': 'Final delay',
                        'file': os.path.basename(real_figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]



class finalphaseGainPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        ms = self.ms
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finalphasegain-%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        #corrstring = context.evla['msinfo'][m.name].corrstring
        #corrstring = m.get_vla_corrstring()
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        calibrator_field_select_string = context.evla['msinfo'][m.name].calibrator_field_select_string
        #field_ids = context.evla['msinfo'][m.name].field_ids
        field_ids = m.get_vla_field_ids()
        #field_names = context.evla['msinfo'][m.name].field_names
        field_names = m.get_vla_field_names()
        #channels = context.evla['msinfo'][m.name].channels
        channels = m.get_vla_numchan()
        
        ms_active = m.name
        
        plots = []
        
        
    
	nplots=int(numAntenna/3)
	
	
	if ((numAntenna%3)>0):
	    nplots = nplots + 1
	
	nplots=numAntenna
	
	for ii in range(nplots):
	
	    filename='finalBPinitialgainphase'+str(ii)+'.png'
	    ####syscommand='rm -rf '+filename
	     ####os.system(syscommand)
	    #antPlot=str(ii*3)+'~'+str(ii*3+2)
	    antPlot=str(ii)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)
	
	    if not os.path.exists(figfile):
	        try:
	        
	            LOG.info("Plotting final phase gain solutions")
	            casa.plotcal(caltable=self.basevis+'.finalBPinitialgain.g', xaxis='time', yaxis='phase', poln='', field='',  antenna=antPlot, spw='', timerange='',        subplot=111, overplot=False, clearpanel='Auto', iteration='antenna',  plotrange=[0,0,-180,180], showflags=False, plotsymbol='o-',        plotcolor='blue',  markersize=5.0, fontsize=10.0, showgui=False, figfile=figfile)
	            #plots.append(figfile)

	        except:
	            LOG.warn("Unable to plot " + filename)
	            #print " "
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
            
                #Get antenna name
                antName = antPlot
                if antPlot != '':
                    domain_antennas = self.ms.get_antenna(antPlot)
                    idents = [a.name if a.name else a.id for a in domain_antennas]
                    antName = ','.join(idents)
            
                plot = logger.Plot(figfile, x_axis='Time', y_axis='Phase',
		        field='',
                        parameters={ 'spw': '',
                        'pol': '',
                        'ant': antName,
                        'type': 'BP initial gain phase',
                        'file': os.path.basename(figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]




class finalbpSolAmpPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        ms = self.ms
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finalbpsolamp-%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        #corrstring = context.evla['msinfo'][m.name].corrstring
        #corrstring = m.get_vla_corrstring()
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        calibrator_field_select_string = context.evla['msinfo'][m.name].calibrator_field_select_string
        #field_ids = context.evla['msinfo'][m.name].field_ids
        field_ids = m.get_vla_field_ids()
        #field_names = context.evla['msinfo'][m.name].field_names
        field_names = m.get_vla_field_names()
        #channels = context.evla['msinfo'][m.name].channels
        channels = m.get_vla_numchan()
        
        ms_active = m.name
        
        plots = []
        
        
    
	nplots=int(numAntenna/3)
        
        
        with casatools.TableReader(self.basevis+'.finalBPcal.b') as tb:
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
	
	nplots=numAntenna
	
	for ii in range(nplots):
	
	    filename='finalBPcal_amp'+str(ii)+'.png'
	    ####syscommand='rm -rf '+filename
	     ####os.system(syscommand)
	    #antPlot=str(ii*3)+'~'+str(ii*3+2)
	    antPlot=str(ii)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)
	
	    if not os.path.exists(figfile):
	        try:
	            LOG.info("Plotting amp bandpass solutions")
	            '''
	            task_args = {'vis'         : self.ms.name,
                                 'caltable'    : 'finalBPcal.b',
                                 'xaxis'       : 'freq',
                                 'yaxis'       : 'amp',
                                 'interactive' : False,
                                 'spw'         : '',
                                 'antenna'     : antPlot,
                                 'showatm'     : False,
                                 'showfdm'     : False,
                                 'showlines'   : True,
                                 'subplot'     : 11,
                                 'plotrange'   : [0,0,0, ampplotmax],
                                 'overlay'     : 'baseband',
                                 'figfile'     : figfile}
                                 
                    task = casa_tasks.plotbandpass(**task_args)
                    task.execute(dry_run=False)
	            '''
	            
	            
	            casa.plotcal(caltable=self.basevis+'.finalBPcal.b', xaxis='freq',  yaxis='amp', poln='', field='', antenna=antPlot, spw='', timerange='', subplot=111,      overplot=False, clearpanel='Auto', iteration='antenna',  plotrange=[0,0,0,ampplotmax],  showflags=False, plotsymbol='o',        plotcolor='blue',  markersize=5.0, fontsize=10.0, showgui=False, figfile=figfile)
	            
	            #plots.append(figfile)

	        except:
	            LOG.warn("Unable to plot " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
            
                #Get antenna name
                antName = antPlot
                if antPlot != '':
                    domain_antennas = self.ms.get_antenna(antPlot)
                    idents = [a.name if a.name else a.id for a in domain_antennas]
                    antName = ','.join(idents)
            
                root, ext = os.path.splitext(figfile)
                #real_figfile = '%s.%s.spw%0.2d.%s%s' % (root, antName, 00, 't00',ext)
                real_figfile = figfile
            
                LOG.info("real_figfile: " + real_figfile)
            
                plot = logger.Plot(real_figfile, x_axis='Freq', y_axis='Amp',
		        field='',
                        parameters={ 'spw': '',
                        'pol': '',
                        'ant': antName,
                        'type': 'BP Amp solution',
                        'file': os.path.basename(real_figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]


class finalbpSolPhasePerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        ms = self.ms
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finalbpsolphase-%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        #corrstring = context.evla['msinfo'][m.name].corrstring
        #corrstring = m.get_vla_corrstring()
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        calibrator_field_select_string = context.evla['msinfo'][m.name].calibrator_field_select_string
        #field_ids = context.evla['msinfo'][m.name].field_ids
        field_ids = m.get_vla_field_ids()
        #field_names = context.evla['msinfo'][m.name].field_names
        field_names = m.get_vla_field_names()
        #channels = context.evla['msinfo'][m.name].channels
        channels = m.get_vla_numchan()
        
        ms_active = m.name
        
        plots = []
        
	nplots=int(numAntenna/3)
	
	with casatools.TableReader(self.basevis+'.finalBPcal.b') as tb:
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
	
	nplots=numAntenna
	
	for ii in range(nplots):
	
	    filename='finalBPcal_phase'+str(ii)+'.png'
	    ####syscommand='rm -rf '+filename
	     ####os.system(syscommand)
	    #antPlot=str(ii*3)+'~'+str(ii*3+2)
	    antPlot=str(ii)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)
	
	    if not os.path.exists(figfile):
	        try:
	            LOG.info("Plotting phase bandpass solutions")
	            
	            '''
	            task_args = {'vis'         : self.ms.name,
                                 'caltable'    : 'finalBPcal.b',
                                 'xaxis'       : 'freq',
                                 'yaxis'       : 'phase',
                                 'interactive' : False,
                                 'spw'         : '',
                                 'antenna'     : antPlot,
                                 'showatm'     : False,
                                 'showfdm'     : False,
                                 'showlines'   : True,
                                 'subplot'     : 11,
                                 'plotrange'   : [0,0,-phaseplotmax, phaseplotmax],
                                 'overlay'     : 'baseband',
                                 'figfile'     : figfile}
	            
	            task = casa_tasks.plotbandpass(**task_args)
                    task.execute(dry_run=False)
	            '''

	            
	            casa.plotcal(caltable=self.basevis+'.finalBPcal.b',  xaxis='freq', yaxis='phase', poln='',  field='',  antenna=antPlot, spw='',  timerange='',      subplot=111,  overplot=False, clearpanel='Auto', iteration='antenna',  plotrange=[0,0,-phaseplotmax,phaseplotmax], showflags=False,        plotsymbol='o', plotcolor='blue',  markersize=5.0, fontsize=10.0,  showgui=False,  figfile=figfile)
	            
	            
	            #plots.append(figfile)

	        except:
	            LOG.warn("Unable to plot " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
            
                #Get antenna name
                antName = antPlot
                if antPlot != '':
                    domain_antennas = self.ms.get_antenna(antPlot)
                    idents = [a.name if a.name else a.id for a in domain_antennas]
                    antName = ','.join(idents)
                    
                root, ext = os.path.splitext(figfile)
                #real_figfile = '%s.%s.spw%0.2d.%s%s' % (root, antName, 00, 't00',ext)
                real_figfile = figfile
            
                plot = logger.Plot(real_figfile, x_axis='Freq', y_axis='Phase',
		        field='',
                        parameters={ 'spw': '',
                        'pol': '',
                        'ant': antName,
                        'type': 'BP Phase solution',
                        'file': os.path.basename(real_figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]


class finalbpSolPhaseShortPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        ms = self.ms
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finalbpsolphaseshort-%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        #corrstring = context.evla['msinfo'][m.name].corrstring
        #corrstring = m.get_vla_corrstring()
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        calibrator_field_select_string = context.evla['msinfo'][m.name].calibrator_field_select_string
        #field_ids = context.evla['msinfo'][m.name].field_ids
        field_ids = m.get_vla_field_ids()
        #field_names = context.evla['msinfo'][m.name].field_names
        field_names = m.get_vla_field_names()
        #channels = context.evla['msinfo'][m.name].channels
        channels = m.get_vla_numchan()
        
        ms_active = m.name
        
        plots = []
        
	nplots=int(numAntenna/3)
	
	with casatools.TableReader(self.basevis+'.finalBPcal.b') as tb:
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
	
	nplots=numAntenna
	
	for ii in range(nplots):
	
	    filename='phaseshortgaincal'+str(ii)+'.png'
	    ####syscommand='rm -rf '+filename
	     ####os.system(syscommand)
	    #antPlot=str(ii*3)+'~'+str(ii*3+2)
	    antPlot=str(ii)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)
	
	    if not os.path.exists(figfile):
	        try:
	            LOG.info("Plotting phase short gaincal")
	            casa.plotcal(caltable=self.basevis+'.phaseshortgaincal.g', xaxis='time', yaxis='phase', poln='', field='', antenna=antPlot, spw='', timerange='',        subplot=111, overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[0,0,-180,180], showflags=False, plotsymbol='o-',        plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=figfile)
	            #plots.append(figfile)

	        except:
	            LOG.warn("Unable to plot " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
            
                #Get antenna name
                antName = antPlot
                if antPlot != '':
                    domain_antennas = self.ms.get_antenna(antPlot)
                    idents = [a.name if a.name else a.id for a in domain_antennas]
                    antName = ','.join(idents)
            
                plot = logger.Plot(figfile, x_axis='Time', y_axis='Phase',
		        field='',
                        parameters={ 'spw': '',
                        'pol': '',
                        'ant': antName,
                        'type': 'Phase (short) gain solution',
                        'file': os.path.basename(figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]



#------------------------------------------------------------------------------


class finalAmpTimeCalPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        ms = self.ms
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finalamptimecal-%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        #corrstring = context.evla['msinfo'][m.name].corrstring
        #corrstring = m.get_vla_corrstring()
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        calibrator_field_select_string = context.evla['msinfo'][m.name].calibrator_field_select_string
        #field_ids = context.evla['msinfo'][m.name].field_ids
        field_ids = m.get_vla_field_ids()
        #field_names = context.evla['msinfo'][m.name].field_names
        field_names = m.get_vla_field_names()
        #channels = context.evla['msinfo'][m.name].channels
        channels = m.get_vla_numchan()
        
        ms_active = m.name
        
        plots = []
        
	nplots=int(numAntenna/3)
	
	with casatools.TableReader(self.basevis+'.finalampgaincal.g') as tb:
            cpar=tb.getcol('CPARAM')
            flgs=tb.getcol('FLAG')
        amps=np.abs(cpar)
        good=np.logical_not(flgs)
        maxamp=np.max(amps[good])
        plotmax=max(2.0,maxamp)
		
	if ((numAntenna%3)>0):
	    nplots = nplots + 1
	    
	nplots=numAntenna
	
	for ii in range(nplots):
	
	    filename='finalamptimecal'+str(ii)+'.png'
	    ####syscommand='rm -rf '+filename
	     ####os.system(syscommand)
	    #antPlot=str(ii*3)+'~'+str(ii*3+2)
	    antPlot=str(ii)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)
	
	    if not os.path.exists(figfile):
	        try:
	            LOG.info("Plotting final amp timecal")
	            casa.plotcal(caltable=self.basevis+'.finalampgaincal.g', xaxis='time', yaxis='amp', poln='', field='', antenna=antPlot, spw='',     timerange='', subplot=111, overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[0,0,0,plotmax], showflags=False,        plotsymbol='o-', plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=figfile)
	            #plots.append(figfile)

	        except:
	            LOG.warn("Unable to plot " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
            
                #Get antenna name
                antName = antPlot
                if antPlot != '':
                    domain_antennas = self.ms.get_antenna(antPlot)
                    idents = [a.name if a.name else a.id for a in domain_antennas]
                    antName = ','.join(idents)
                    
                    
                plot = logger.Plot(figfile, x_axis='Time', y_axis='Amp',
		        field='',
                        parameters={ 'spw': '',
                        'pol': '',
                        'ant': antName,
                        'type': 'Final amp time cal',
                        'file': os.path.basename(figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]



class finalAmpFreqCalPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        ms = self.ms
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finalampfreqcal-%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        #corrstring = context.evla['msinfo'][m.name].corrstring
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        calibrator_field_select_string = context.evla['msinfo'][m.name].calibrator_field_select_string
        #field_ids = context.evla['msinfo'][m.name].field_ids
        field_ids = m.get_vla_field_ids()
        #field_names = context.evla['msinfo'][m.name].field_names
        field_names = m.get_vla_field_names()
        #channels = context.evla['msinfo'][m.name].channels
        channels = m.get_vla_numchan()
        
        ms_active = m.name
        
        plots = []
        
	nplots=int(numAntenna/3)
	
	with casatools.TableReader(self.basevis+'.finalampgaincal.g') as tb:
            cpar=tb.getcol('CPARAM')
            flgs=tb.getcol('FLAG')
        amps=np.abs(cpar)
        good=np.logical_not(flgs)
        maxamp=np.max(amps[good])
        plotmax=max(2.0,maxamp)
		
	if ((numAntenna%3)>0):
	    nplots = nplots + 1
	    
	nplots=numAntenna
	
	for ii in range(nplots):
	
	    filename='finalampfreqcal'+str(ii)+'.png'
	    ####syscommand='rm -rf '+filename
	     ####os.system(syscommand)
	    #antPlot=str(ii*3)+'~'+str(ii*3+2)
	    antPlot=str(ii)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)
	
	    if not os.path.exists(figfile):
	        try:
	            LOG.info("Plotting final amp freqcal")
	            
	            '''
	            task_args = {'vis'         : self.ms.name,
                                 'caltable'    : 'finalampgaincal.g',
                                 'xaxis'       : 'freq',
                                 'yaxis'       : 'amp',
                                 'interactive' : False,
                                 'spw'         : '',
                                 'antenna'     : antPlot,
                                 'showatm'     : False,
                                 'showfdm'     : False,
                                 'subplot'     : 11,
                                 'plotrange'   : [0,0,0, plotmax],
                                 'figfile'     : figfile}
	            
	            task = casa_tasks.plotbandpass(**task_args)
                    task.execute(dry_run=False)
	            '''
	            
	            
	            casa.plotcal(caltable=self.basevis+'.finalampgaincal.g', xaxis='freq', yaxis='amp', poln='', field='', antenna=antPlot, spw='',     timerange='', subplot=111, overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[0,0,0,plotmax], showflags=False,        plotsymbol='o', plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=figfile)
	            
	            #plots.append(figfile)

	        except:
	            LOG.warn("Unable to plot " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
            
                #Get antenna name
                antName = antPlot
                if antPlot != '':
                    domain_antennas = self.ms.get_antenna(antPlot)
                    idents = [a.name if a.name else a.id for a in domain_antennas]
                    antName = ','.join(idents)
            
                root, ext = os.path.splitext(figfile)
                #real_figfile = '%s.%s.spw%0.2d.%s%s' % (root, antName, 00, 't00',ext)
                real_figfile = figfile
            
                plot = logger.Plot(real_figfile, x_axis='freq', y_axis='Amp',
		        field='',
                        parameters={ 'spw': '',
                        'pol': '',
                        'ant': antName,
                        'type': 'Final amp freq cal',
                        'file': os.path.basename(real_figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]



class finalPhaseGainCalPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        ms = self.ms
        self.basevis = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'finalphasegaincal-%s.json' % ms)

    def plot(self):
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        
        numAntenna = len(m.antennas)
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        #corrstring = context.evla['msinfo'][m.name].corrstring
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        calibrator_field_select_string = context.evla['msinfo'][m.name].calibrator_field_select_string
        #field_ids = context.evla['msinfo'][m.name].field_ids
        field_ids = m.get_vla_field_ids()
        #field_names = context.evla['msinfo'][m.name].field_names
        field_names = m.get_vla_field_names()
        #channels = context.evla['msinfo'][m.name].channels
        channels = m.get_vla_numchan()
        
        ms_active = m.name
        
        plots = []
        
        nplots=int(numAntenna/3)

        with casatools.TableReader(self.basevis+'.finalphasegaincal.g') as tb:
            cpar=tb.getcol('CPARAM')
            flgs=tb.getcol('FLAG')
        amps=np.abs(cpar)
        good=np.logical_not(flgs)
        maxamp=np.max(amps[good])
        plotmax=max(2.0,maxamp)

        if ((numAntenna%3)>0):
            nplots = nplots + 1

        nplots=numAntenna

        for ii in range(nplots):

            filename='finalphasegaincal'+str(ii)+'.png'
            antPlot=str(ii)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)

            if not os.path.exists(figfile):
                try:
                    LOG.info("Plotting final amp freqcal")
                    casa.plotcal(caltable=self.basevis+'.finalphasegaincal.g',
                                 xaxis='time', yaxis='phase',  poln='', field='',
                                 antenna=antPlot, spw='',  timerange='',
                                 subplot=111,  overplot=False, clearpanel='Auto',
                                 iteration='antenna',  plotrange=[0,0,-180,180],
                                 showflags=False, plotsymbol='o-', plotcolor='blue',
                                 markersize=5.0, fontsize=10.0, showgui=False, figfile=figfile)
                    # plots.append(figfile)

                except:
                    LOG.warn("Problem with plotting " + filename)
            else:
                LOG.debug('Using existing ' + filename + ' plot.')
            
            try:
                # Get antenna name
                antName = antPlot
                if antPlot != '':
                    domain_antennas = self.ms.get_antenna(antPlot)
                    idents = [a.name if a.name else a.id for a in domain_antennas]
                    antName = ','.join(idents)
            
                plot = logger.Plot(figfile, x_axis='time', y_axis='phase', field='',
                                   parameters={ 'spw': '',
                                                'pol': '',
                                                'ant': antName,
                                                'type': 'Final phase gain cal',
                                                'file': os.path.basename(figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        # Create a dummy plot to release the cal table
        scratchfile = 'scratch.g'
        shutil.copytree(self.basevis + '.finalphasegaincal.g', scratchfile)
        casa.plotcal(caltable=scratchfile,
                     xaxis='time', yaxis='phase', poln='', field='',
                     antenna=str(0), spw='', timerange='',
                     subplot=111, overplot=False, clearpanel='Auto',
                     iteration='antenna', plotrange=[0, 0, -180, 180],
                     showflags=False, plotsymbol='o-', plotcolor='blue',
                     markersize=5.0, fontsize=10.0, showgui=False, figfile="scratch.png")
        shutil.rmtree(scratchfile)

        return [p for p in plots if p is not None]
