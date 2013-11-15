import pipeline
import numpy as np

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

ms_active=vislist[0]+'.ms'

#   TEST GAINS

# Plot solutions


print("Plotting gain solutions")

nplots=int(numAntenna/3)

if ((numAntenna%3)>0):
    nplots = nplots + 1

tb.open('testgaincal.g')
cpar=tb.getcol('CPARAM')
flgs=tb.getcol('FLAG')
tb.close()
amps=np.abs(cpar)
good=np.logical_not(flgs)
maxamp=np.max(amps[good])
plotmax=maxamp

for ii in range(nplots):
    filename='testgaincal_amp'+str(ii)+'.png'
    syscommand='rm -rf '+filename
    os.system(syscommand)

    antPlot=str(ii*3)+'~'+str(ii*3+2)

    default('plotcal')
    caltable='testgaincal.g'
    xaxis='time'
    yaxis='amp'
    poln=''
    field=''
    antenna=antPlot
    spw=''
    timerange=''
    subplot=311
    overplot=False
    clearpanel='Auto'
    iteration='antenna'
    plotrange=[0,0,0,plotmax]
    showflags=False
    plotsymbol='o'
    plotcolor='blue'
    markersize=5.0
    fontsize=10.0
    showgui=False
    figfile=filename
    async=False
    plotcal()

for ii in range(nplots):
    filename='testgaincal_phase'+str(ii)+'.png'
    syscommand='rm -rf '+filename
    os.system(syscommand)

    antPlot=str(ii*3)+'~'+str(ii*3+2)

    default('plotcal')
    caltable='testgaincal.g'
    xaxis='time'
    yaxis='phase'
    poln=''
    field=''
    antenna=antPlot
    spw=''
    timerange=''
    subplot=311
    overplot=False
    clearpanel='Auto'
    iteration='antenna'
    plotrange=[0,0,-180,180]
    showflags=False
    plotsymbol='o-'
    plotcolor='blue'
    markersize=5.0
    fontsize=10.0
    showgui=False
    figfile=filename
    async=False
    plotcal()


print("Plotting finished")