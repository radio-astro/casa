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

# Make some plots of the calibrated data

default('plotms')
vis=ms_active
xaxis='time'
yaxis='phase'
ydatacolumn='corrected'
selectdata=True
field=calibrator_field_select_string
correlation=corrstring
averagedata=True
avgchannel=str(max(channels))
avgtime='1e8s'
avgscan=False
transform=False
extendflag=False
iteraxis=''
coloraxis='antenna2'
plotrange=[]
title='Calibrated phase vs. time, all calibrators'
xlabel=''
ylabel=''
showmajorgrid=False
showminorgrid=False
plotfile='all_calibrators_phase_time.png'
overwrite=True
async=False
plotms()
    
    

for ii in field_ids:
    print ii
    default('plotms')
    vis=ms_active
    xaxis='uvwave'
    yaxis='amp'
    ydatacolumn='corrected'
    selectdata=True
#    field=str(calibrator_field_list[ii])
    field=str(field_ids[ii])
    correlation=corrstring
    averagedata=True
    avgchannel=str(max(channels))
    avgtime='1e8s'
    avgscan=False
    transform=False
    extendflag=False
    iteraxis=''
    coloraxis='spw'
    plotrange=[]
    title='Field '+field+', '+field_names[ii]
    xlabel=''
    ylabel=''
    showmajorgrid=False
    showminorgrid=False
    plotfile='field'+field+'_amp_uvdist.png'
    overwrite=True
    async=False
    plotms()

