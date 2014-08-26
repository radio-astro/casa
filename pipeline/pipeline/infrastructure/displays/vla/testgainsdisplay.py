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


class testgainsSummaryChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        #self.caltable = result.final[0].gaintable

    def plot(self):
        ##science_spws = self.ms.get_spectral_windows(science_windows_only=True)
        plots = [self.get_plot_wrapper('testgains_sample')]
        return [p for p in plots if p is not None]

    def create_plot(self, prefix):
        figfile = self.get_figfile(prefix)
        
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        

        ms_active = m.name
        
        antPlot='0~2'
        
        plotmax=100

        #Dummy plot
        casa.plotcal(caltable='testgaincal.g',  xaxis='time', yaxis='amp', poln='', field='', antenna=antPlot, spw='', timerange='',       subplot=311,  overplot=False, clearpanel='Auto',  iteration='antenna',  plotrange=[0,0,0,plotmax],  showflags=False, plotsymbol='o',        plotcolor='blue', markersize=5.0,  fontsize=10.0, showgui=False, figfile=figfile)


    def get_figfile(self, prefix):
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            'testgains'+prefix+'-%s-summary.png' % self.ms.basename)

    def get_plot_wrapper(self, prefix):
        figfile = self.get_figfile(prefix)
        
        context = self.context
        m = context.observing_run.measurement_sets[0]
       
        wrapper = logger.Plot(figfile,
                          x_axis='freq',
                          y_axis='amp',
                          parameters={'vis'      : self.ms.basename,
                                      'type'     : prefix,
                                      'spw'      : ''})

        if not os.path.exists(figfile):
            LOG.trace('testgains summary plot not found. Creating new '
                      'plot.')
            try:
                self.create_plot(prefix)
            except Exception as ex:
                LOG.error('Could not create '+prefix+ ' plot.')
                LOG.exception(ex)
                return None
            
        return wrapper




class testgainsPerAntennaChart(object):
    def __init__(self, context, result, yaxis):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        ms = self.ms
        self.yaxis = yaxis
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          yaxis+'testgains-%s.json' % ms)

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
        
        LOG.info("Plotting gain solutions")
    
	nplots=int(numAntenna/3)
    
        if ((numAntenna%3)>0):
            nplots = nplots + 1
	
	with casatools.TableReader('testgaincal.g') as tb:
            cpar=tb.getcol('CPARAM')
            flgs=tb.getcol('FLAG')
        amps=np.abs(cpar)
        good=np.logical_not(flgs)
        maxamp=np.max(amps[good])
        plotmax=maxamp
	
	
	if ((numAntenna%3)>0):
	    nplots = nplots + 1
	
	nplots=numAntenna
	
	for ii in range(nplots):
	
	    filename='testgaincal_'+self.yaxis+str(ii)+'.png'
	    ####syscommand='rm -rf '+filename
	     ####os.system(syscommand)
	    #antPlot=str(ii*3)+'~'+str(ii*3+2)
	    antPlot=str(ii)
            
            stage = 'stage%s' % result.stage_number
            stage_dir = os.path.join(context.report_dir, stage)
            # construct the relative filename, eg. 'stageX/testdelay0.png'
            
            figfile = os.path.join(stage_dir, filename)
            
            if self.yaxis == 'amp':
                plotrange = [0,0,0,plotmax]
                plotsymbol = 'o'
            
            if self.yaxis == 'phase':
                plotrange = [0,0,-180,180]
                plotsymbol = 'o-'
            
	
	    if not os.path.exists(figfile):
	        try:
	            casa.plotcal(caltable='testgaincal.g',  xaxis='time', yaxis=self.yaxis, poln='', field='', antenna=antPlot, spw='', timerange='',       subplot=111,  overplot=False, clearpanel='Auto',  iteration='antenna',  plotrange=plotrange,  showflags=False, plotsymbol=plotsymbol,        plotcolor='blue', markersize=5.0,  fontsize=10.0, showgui=False, figfile=figfile)
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
            
                plot = logger.Plot(figfile, x_axis='Time', y_axis=self.yaxis.title(),
		        field='',
                        parameters={ 'spw': '',
                        'pol': '',
                        'ant': antName,
                        'type': self.yaxis,
                        'file': os.path.basename(figfile)})
                plots.append(plot)
            except:
                LOG.warn("Unable to add plot to stack")
                plots.append(None)

        return [p for p in plots if p is not None]


