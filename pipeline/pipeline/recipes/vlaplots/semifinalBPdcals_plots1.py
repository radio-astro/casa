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

print("Plotting delays")

nplots=int(numAntenna/3)

if ((numAntenna%3)>0):
    nplots = nplots + 1

for ii in range(nplots):
    filename='delay'+str(ii)+'.png'
    syscommand='rm -rf '+filename
    os.system(syscommand)

    antPlot=str(ii*3)+'~'+str(ii*3+2)

    default('plotcal')
    caltable='delay.k'
    xaxis='freq'
    yaxis='delay'
    poln=''
    field=''
    antenna=antPlot
    spw=''
    timerange=''
    subplot=311
    overplot=False
    clearpanel='Auto'
    iteration='antenna'
    plotrange=[]
    showflags=False
    plotsymbol='o'
    plotcolor='blue'
    markersize=5.0
    fontsize=10.0
    showgui=False
    figfile=filename
    async=False
    plotcal()






print("Plotting initial phase gain calibration on BP calibrator")


for ii in range(nplots):
    filename='BPinitialgainphase'+str(ii)+'.png'
    syscommand='rm -rf '+filename
    os.system(syscommand)

    antPlot=str(ii*3)+'~'+str(ii*3+2)

    default('plotcal')
    caltable='BPinitialgain.g'
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
    


# Plot BP solutions

print("Plotting bandpass solutions")

tb.open('BPcal.b')
dataVarCol = tb.getvarcol('CPARAM')
flagVarCol = tb.getvarcol('FLAG')
tb.close()
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
        maxphase=np.max(np.abs(phases[good]))*180./pi
        if (maxphase>maxmaxphase):
            maxmaxphase=maxphase
ampplotmax=maxmaxamp
phaseplotmax=maxmaxphase

for ii in range(nplots):
    filename='BPcal_amp'+str(ii)+'.png'
    syscommand='rm -rf '+filename
    os.system(syscommand)

    antPlot=str(ii*3)+'~'+str(ii*3+2)

    default('plotcal')
    caltable='BPcal.b'
    xaxis='freq'
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
    plotrange=[0,0,0,ampplotmax]
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
    filename='BPcal_phase'+str(ii)+'.png'
    syscommand='rm -rf '+filename
    os.system(syscommand)

    antPlot=str(ii*3)+'~'+str(ii*3+2)

    default('plotcal')
    caltable='BPcal.b'
    xaxis='freq'
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
    plotrange=[0,0,-phaseplotmax,phaseplotmax]
    showflags=False
    plotsymbol='o'
    plotcolor='blue'
    markersize=5.0
    fontsize=10.0
    showgui=False
    figfile=filename
    async=False
    plotcal()


print("Plotting complete")


print("Plot calibrated calibrators to check for further flagging/RFI")

# NB: have to find a way to get plotms to reload data to show
# flagged result on second run

syscommand='rm -rf semifinalcalibratedcals1.png'
os.system(syscommand)

clearstat()

default('plotms')
vis=ms_active
xaxis='freq'
yaxis='amp'
ydatacolumn='corrected'
selectdata=True
scan=calibrator_scan_select_string
correlation=corrstring
averagedata=True
avgtime='1e8s'
avgscan=False
transform=False
extendflag=False
iteraxis=''
coloraxis='antenna2'
plotrange=[]
title=''
xlabel=''
ylabel=''
showmajorgrid=False
showminorgrid=False
plotfile='semifinalcalibratedcals1.png'
interactive=False
overwrite=True
async=False
plotms()
