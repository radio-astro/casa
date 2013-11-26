import numpy as np
import os
import sys
import traceback
import inspect

import math
import pipeline
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.api as api
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure.jobrequest import casa_tasks
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.jobrequest as jobrequest

import casa


# Make sure CASA exceptions are rethrown
try:
    if not  __rethrow_casa_exceptions:
        def_rethrow = False
    else:
        def_rethrow = __rethrow_casa_exceptions
except:
    def_rethrow = False

__rethrow_casa_exceptions = True



# Setup paths
sys.path.insert (0, os.path.expandvars("$SCIPIPE_HEURISTICS"))
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/h/cli/mytasks.py"))
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/hif/cli/mytasks.py"))
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/hifv/cli/mytasks.py"))


def finalcals_plots():

    context = pipeline.Pipeline(context='last').context
    
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
    
    ms_active=m.name
    
    #FINALCALS

    print("Plotting final calibration tables")
    
    # do some plotting
    
    nplots=int(numAntenna/3)
    
    if ((numAntenna%3)>0):
        nplots = nplots + 1
    
    with casatools.TableReader('finaldelay.k') as tb:
        fpar = tb.getcol('FPARAM')

    delays = np.abs(fpar)
    maxdelay = np.max(delays)
    
    for ii in range(nplots):
        filename='finaldelay'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    

        casa.plotcal(caltable='finaldelay.k', xaxis='freq',yaxis='delay', poln='', field='', antenna=antPlot, spw='',timerange='', subplot=311, overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[], showflags=False, plotsymbol='o', plotcolor='blue', markersize=5.0,fontsize=10.0, showgui=False, figfile=filename)

    
    for ii in range(nplots):
        filename='finalBPinitialgainphase'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    

        casa.plotcal(caltable='finalBPinitialgain.g', xaxis='time',  yaxis='phase', poln='', field='', antenna=antPlot,        spw='', timerange='', subplot=311, overplot=False, clearpanel='Auto',  iteration='antenna', plotrange=[0,0,-180,180],      showflags=False, plotsymbol='o-',  plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=filename)
    
    with casatools.TableReader('finalBPcal.b') as tb:
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
    
    for ii in range(nplots):
        filename='finalBPcal_amp'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    

        casa.plotcal(caltable='finalBPcal.b', xaxis='freq', yaxis='amp', poln='', field='', antenna=antPlot, spw='',        timerange='', subplot=311,  overplot=False, clearpanel='Auto',  iteration='antenna',  plotrange=[0,0,0,ampplotmax],        showflags=False, plotsymbol='o',  plotcolor='blue',  markersize=5.0, fontsize=10.0,  showgui=False, figfile=filename)

    
    for ii in range(nplots):
        filename='finalBPcal_phase'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    

        casa.plotcal(caltable='finalBPcal.b', xaxis='freq', yaxis='phase', poln='', field='',  antenna=antPlot, spw='',        timerange='',  subplot=311, overplot=False,  clearpanel='Auto',  iteration='antenna', plotrange=[0,0,-phaseplotmax,phaseplotmax],        showflags=False,  plotsymbol='o',  plotcolor='blue',  markersize=5.0,  fontsize=10.0, showgui=False, figfile=filename)
    
    for ii in range(nplots):
        filename='phaseshortgaincal'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    

        casa.plotcal(caltable='phaseshortgaincal.g', xaxis='time', yaxis='phase', poln='', field='', antenna=antPlot, spw='', timerange='',        subplot=311, overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[0,0,-180,180], showflags=False, plotsymbol='o-',        plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=filename)
    
    with casatools.TableReader('finalampgaincal.g') as tb:
        cpar=tb.getcol('CPARAM')
        flgs=tb.getcol('FLAG')
    amps=np.abs(cpar)
    good=np.logical_not(flgs)
    maxamp=np.max(amps[good])
    plotmax=max(2.0,maxamp)
    
    for ii in range(nplots):
        filename='finalamptimecal'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    

        casa.plotcal(caltable='finalampgaincal.g', xaxis='time', yaxis='amp', poln='', field='', antenna=antPlot, spw='',     timerange='', subplot=311, overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[0,0,0,plotmax], showflags=False,        plotsymbol='o-', plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=filename)
    
    for ii in range(nplots):
        filename='finalampfreqcal'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    

        casa.plotcal(caltable='finalampgaincal.g', xaxis='freq', yaxis='amp', poln='', field='', antenna=antPlot,        spw='', timerange='', subplot=311, overplot=False, clearpanel='Auto',  iteration='antenna', plotrange=[0,0,0,plotmax],      showflags=False, plotsymbol='o',  plotcolor='blue', markersize=5.0,  fontsize=10.0, showgui=False, figfile=filename)
    
    for ii in range(nplots):
        filename='finalphasegaincal'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    

        casa.plotcal(caltable='finalphasegaincal.g', xaxis='time', yaxis='phase',  poln='', field='', antenna=antPlot, spw='',  timerange='',        subplot=311,  overplot=False, clearpanel='Auto',  iteration='antenna',  plotrange=[0,0,-180,180],  showflags=False,       plotsymbol='o-', plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=filename)  
        
    return True

def fluxboot_plots():
    
    context = pipeline.Pipeline(context='last').context
    
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
    
    ms_active=m.name
    
    #FLUXBOOT
    
    print("Plotting model calibrator flux densities")
    
    
    syscommand='rm -rf bootstrappedFluxDensities.png'
    os.system(syscommand)
    
    casa.clearstat()
    

    casa.plotms(vis=ms_active, xaxis='freq', yaxis='amp', ydatacolumn='model', selectdata=True,   scan=calibrator_scan_select_string, correlation=corrstring, averagedata=True, avgtime='1e8s', avgscan=True, transform=False,    extendflag=False, iteraxis='', coloraxis='field', plotrange=[], title='', xlabel='', ylabel='',  showmajorgrid=False, showminorgrid=False,    plotfile='bootstrappedFluxDensities.png', overwrite=True)
    
    return True


def semifinalBPdcals_plots1():
    
    context = pipeline.Pipeline(context='last').context
    
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
    
    ms_active=m.name
    
    print("Plotting delays")
    
    nplots=int(numAntenna/3)
    
    if ((numAntenna%3)>0):
        nplots = nplots + 1
    
    for ii in range(nplots):
        filename='delay'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    

        casa.plotcal(caltable='delay.k', xaxis='freq', yaxis='delay',  poln='', field='',  antenna=antPlot, spw='',        timerange='', subplot=311,  overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[],  showflags=False, plotsymbol='o',        plotcolor='blue',  markersize=5.0, fontsize=10.0, showgui=False, figfile=filename)

    print("Plotting initial phase gain calibration on BP calibrator")
    
    
    for ii in range(nplots):
        filename='BPinitialgainphase'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    
        casa.plotcal(caltable='BPinitialgain.g', xaxis='time', yaxis='phase', poln='', field='', antenna=antPlot, spw='',        timerange='', subplot=311, overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[0,0,-180,180], showflags=False,        plotsymbol='o-', plotcolor='blue', markersize=5.0, fontsize=10.0,  showgui=False, figfile=filename)
        
    
    
    # Plot BP solutions
    
    print("Plotting bandpass solutions")
    
    with casatools.TableReader('BPcal.b') as tb:
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
    
    for ii in range(nplots):
        filename='BPcal_amp'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    
        casa.plotcal(caltable='BPcal.b', xaxis='freq', yaxis='amp', poln='', field='', antenna=antPlot, spw='',        timerange='',  subplot=311, overplot=False, clearpanel='Auto',  iteration='antenna', plotrange=[0,0,0,ampplotmax], showflags=False,        plotsymbol='o', plotcolor='blue', markersize=5.0, fontsize=10.0,  showgui=False, figfile=filename)
    
    for ii in range(nplots):
        filename='BPcal_phase'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    
        casa.plotcal(caltable='BPcal.b', xaxis='freq', yaxis='phase', poln='', field='', antenna=antPlot, spw='',        timerange='', subplot=311,  overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[0,0,-phaseplotmax,phaseplotmax],        showflags=False, plotsymbol='o',  plotcolor='blue',  markersize=5.0, fontsize=10.0, showgui=False, figfile=filename)

    #Get BPcal.b to close...
    casa.plotcal(caltable='testBPcal.b', xaxis='freq', yaxis='phase', poln='', field='', antenna='0', spw='',        timerange='',  subplot=311,  overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[0,0,-phaseplotmax,phaseplotmax],        showflags=False, plotsymbol='o', plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile='junk.png')
    
    print("Plotting complete")
    
    
    print("Plot calibrated calibrators to check for further flagging/RFI")
    
    # NB: have to find a way to get plotms to reload data to show
    # flagged result on second run
    
    syscommand='rm -rf semifinalcalibratedcals1.png'
    os.system(syscommand)
    
    casa.clearstat()
    
    casa.plotms(vis=ms_active, xaxis='freq', yaxis='amp', ydatacolumn='corrected', selectdata=True,    scan=calibrator_scan_select_string, correlation=corrstring, averagedata=True, avgtime='1e8s', avgscan=False, transform=False,    extendflag=False,iteraxis='',  coloraxis='antenna2', plotrange=[], title='', xlabel='', ylabel='', showmajorgrid=False,    showminorgrid=False, plotfile='semifinalcalibratedcals1.png', interactive=False,  overwrite=True)
    
    return True

def semifinalBPdcals_plots2():
    
    context = pipeline.Pipeline(context='last').context
    
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
    
    ms_active=m.name
    
    # SEMI-2
    
    print("Plotting delays")
    
    nplots=int(numAntenna/3)
    
    if ((numAntenna%3)>0):
        nplots = nplots + 1
    
    for ii in range(nplots):
        filename='delay'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    
        casa.plotcal(caltable='delay.k', xaxis='freq', yaxis='delay',  poln='', field='',  antenna=antPlot, spw='',  timerange='',  subplot=311,        overplot=False,  clearpanel='Auto', iteration='antenna', plotrange=[], showflags=False, plotsymbol='o', plotcolor='blue',        markersize=5.0, fontsize=10.0, showgui=False, figfile=filename)
        
        
    
    
    print("Plotting initial phase gain calibration on BP calibrator")
    
    
    for ii in range(nplots):
        filename='BPinitialgainphase'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    
        casa.plotcal(caltable='BPinitialgain.g', xaxis='time', yaxis='phase', poln='', field='', antenna=antPlot, spw='',  timerange='',       subplot=311,  overplot=False, clearpanel='Auto',  iteration='antenna', plotrange=[0,0,-180,180], showflags=False, plotsymbol='o-',        plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=filename)    
    
    
    
    # Plot BP solutions
    
    print("Plotting bandpass solutions")
    
    with casatools.TableReader('BPcal.b') as tb:
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
    
    for ii in range(nplots):
        filename='BPcal_amp'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    
        casa.plotcal(caltable='BPcal.b', xaxis='freq', yaxis='amp',  poln='', field='', antenna=antPlot, spw='',        timerange='', subplot=311,  overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[0,0,0,ampplotmax], showflags=False,        plotsymbol='o', plotcolor='blue', markersize=5.0, fontsize=10.0,  showgui=False, figfile=filename)
    
    for ii in range(nplots):
        filename='BPcal_phase'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    

        casa.plotcal(caltable='BPcal.b', xaxis='freq', yaxis='phase', poln='', field='', antenna=antPlot, spw='',        timerange='',  subplot=311,  overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[0,0,-phaseplotmax,phaseplotmax],        showflags=False, plotsymbol='o', plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=filename)

    #Get BPcal.b to close...
    casa.plotcal(caltable='testBPcal.b', xaxis='freq', yaxis='phase', poln='', field='', antenna='0', spw='',        timerange='',  subplot=311,  overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[0,0,-phaseplotmax,phaseplotmax],        showflags=False, plotsymbol='o', plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile='junk2.png')
    
    print("Plotting complete")
    
    
    
    
    print("Plot calibrated calibrators to check for further flagging/RFI")
    
    # NB: have to find a way to get plotms to reload data to show
    # flagged result on second run
    
    syscommand='rm -rf semifinalcalibratedcals2.png'
    os.system(syscommand)
    
    casa.clearstat()
    

    casa.plotms(vis=ms_active, xaxis='freq', yaxis='amp', ydatacolumn='corrected',selectdata=True, uvrange='>0klambda',   scan=calibrator_scan_select_string, correlation=corrstring, averagedata=True,  avgtime='1e8s',  avgscan=False,  transform=False,   extendflag=False,  iteraxis='', coloraxis='antenna2', plotrange=[], title='', xlabel='',ylabel='', showmajorgrid=False,   
    showminorgrid=False,plotfile='semifinalcalibratedcals2.png', interactive=False, overwrite=True)
    

def targetflag_plots():
    
    context = pipeline.Pipeline(context='last').context
    
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
    
    ms_active=m.name
    
    # Make some plots of the calibrated data
    

    casa.plotms(vis=ms_active, xaxis='time', yaxis='phase', ydatacolumn='corrected', selectdata=True, field=calibrator_field_select_string,    correlation=corrstring, averagedata=True, avgchannel=str(max(channels)), avgtime='1e8s', avgscan=False, transform=False,  extendflag=False,
    iteraxis='', coloraxis='antenna2', plotrange=[], title='Calibrated phase vs. time, all calibrators', xlabel='',  ylabel='',   showmajorgrid=False,  showminorgrid=False, plotfile='all_calibrators_phase_time.png', overwrite=True)

        
        
    
    for ii in field_ids:
        print ii
        casa.plotms(vis=ms_active,  xaxis='uvwave',  yaxis='amp',  ydatacolumn='corrected',  selectdata=True, field=str(field_ids[ii]),        correlation=corrstring,   averagedata=True,    avgchannel=str(max(channels)),   avgtime='1e8s',   avgscan=False,   transform=False,        extendflag=False,   iteraxis='',  coloraxis='spw',  plotrange=[],  title='Field '+str(field_ids[ii])+', '+field_names[ii],   xlabel='',       ylabel='',  showmajorgrid=False,  showminorgrid=False,  plotfile='field'+str(field_ids[ii])+'_amp_uvdist.png',  overwrite=True)

def testBPdcals_plots():

    context = h_resume()
    
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
    
    ms_active=m.name
    
    print ("Plotting test delays")
    
    nplots=int(numAntenna/3)
    
    if ((numAntenna%3)>0):
        nplots = nplots + 1
    
    for ii in range(nplots):
        filename='testdelay'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    

        casa.plotcal(caltable='testdelay.k', xaxis='freq', yaxis='delay', poln='',  field='', antenna=antPlot, spw='', timerange='', subplot=311, overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[], showflags=False, plotsymbol='o', plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=filename)
        
       
        
    
    
    print("Plotting amplitude gain solutions")
    
    with casatools.TableReader('testBPdinitialgain.g') as tb:
        cpar=tb.getcol('CPARAM')
        flgs=tb.getcol('FLAG')
    amps=np.abs(cpar)
    good=np.logical_not(flgs)
    maxamp=np.max(amps[good])
    plotmax=maxamp
    
    for ii in range(nplots):
        filename='testBPdinitialgainamp'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    #
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    #

        casa.plotcal(caltable='testBPdinitialgain.g', xaxis='time', yaxis='amp', poln='', field='', antenna=antPlot, spw='', timerange='', subplot=311, overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[0,0,0,plotmax], showflags=False, plotsymbol='o',plotcolor='blue',markersize=5.0,fontsize=10.0,showgui=False,figfile=filename)
    
    
    print("Plotting phase gain solutions")
    
    for ii in range(nplots):
        filename='testBPdinitialgainphase'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    #
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    #

        casa.plotcal(caltable='testBPdinitialgain.g', xaxis='time', yaxis='phase', poln='', field='',  antenna=antPlot, spw='', timerange='',        subplot=311, overplot=False, clearpanel='Auto', iteration='antenna',  plotrange=[0,0,-180,180], showflags=False, plotsymbol='o-',        plotcolor='blue',  markersize=5.0, fontsize=10.0, showgui=False, figfile=filename)

        
        
    
    
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
    
    for ii in range(nplots):
        filename='testBPcal_amp'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    #
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    #

        casa.plotcal(caltable='testBPcal.b', xaxis='freq',  yaxis='amp', poln='', field='', antenna=antPlot, spw='', timerange='', subplot=311,      overplot=False, clearpanel='Auto', iteration='antenna',  plotrange=[0,0,0,ampplotmax],  showflags=False, plotsymbol='o',        plotcolor='blue',  markersize=5.0, fontsize=10.0, showgui=False, figfile=filename)
    
    for ii in range(nplots):
        filename='testBPcal_phase'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    #
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    #

        casa.plotcal(caltable='testBPcal.b',  xaxis='freq', yaxis='phase', poln='',  field='',  antenna=antPlot, spw='',  timerange='',      subplot=311,  overplot=False, clearpanel='Auto', iteration='antenna',  plotrange=[0,0,-phaseplotmax,phaseplotmax], showflags=False,        plotsymbol='o', plotcolor='blue',  markersize=5.0, fontsize=10.0,  showgui=False,  figfile=filename)
    
    print("Plotting of test bandpass solutions complete")
    
    
    print("Plot calibrated bandpass and delay calibrators")
    
    syscommand='rm -rf testcalibratedBPcal.png'
    os.system(syscommand)
    

    casa.plotms(vis=ms_active, xaxis='freq', yaxis='amp', ydatacolumn='corrected',  selectdata=True, field=bandpass_field_select_string,    scan=bandpass_scan_select_string,  correlation=corrstring,  averagedata=True, avgtime='1e8s', avgscan=True, transform=False,    extendflag=False, iteraxis='', coloraxis='antenna2',  plotrange=[], title='', xlabel='',  ylabel='', showmajorgrid=False,    showminorgrid=False, plotfile='testcalibratedBPcal.png', overwrite=True)

    
    # Plot calibrated delay calibrator, if different from BP cal
    
    if (delay_scan_select_string != bandpass_scan_select_string):
        syscommand='rm -rf testcalibrated_delaycal.png'
        os.system(syscommand)
    

        casa.plotms(vis=ms_active, xaxis='freq',   yaxis='amp',  ydatacolumn='corrected',  selectdata=True,  scan=delay_scan_select_string,        correlation=corrstring,  averagedata=True,  avgtime='1e8s',  avgscan=True, transform=False,  extendflag=False,  iteraxis='',        coloraxis='antenna2',  plotrange=[],  title='',  xlabel='',  ylabel='',  showmajorgrid=False,  showminorgrid=False,        plotfile='testcalibrated_delaycal.png', overwrite=True)


def testgains_plots():
    
    context = pipeline.Pipeline(context='last').context
    
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
    
    ms_active=m.name
    
    #   TEST GAINS
    
    # Plot solutions
    
    
    print("Plotting gain solutions")
    
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
    
    for ii in range(nplots):
        filename='testgaincal_amp'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    

        casa.plotcal(caltable='testgaincal.g',  xaxis='time', yaxis='amp', poln='', field='', antenna=antPlot, spw='', timerange='',       subplot=311,  overplot=False, clearpanel='Auto',  iteration='antenna',  plotrange=[0,0,0,plotmax],  showflags=False, plotsymbol='o',        plotcolor='blue', markersize=5.0,  fontsize=10.0, showgui=False, figfile=filename)
    
    for ii in range(nplots):
        filename='testgaincal_phase'+str(ii)+'.png'
        syscommand='rm -rf '+filename
        os.system(syscommand)
    
        antPlot=str(ii*3)+'~'+str(ii*3+2)
    

        casa.plotcal(caltable='testgaincal.g', xaxis='time', yaxis='phase', poln='', field='',  antenna=antPlot, spw='', timerange='',        subplot=311,  overplot=False, clearpanel='Auto', iteration='antenna', plotrange=[0,0,-180,180], showflags=False, plotsymbol='o-',        plotcolor='blue', markersize=5.0, fontsize=10.0, showgui=False, figfile=filename)
    

    
    print("Plotting finished")