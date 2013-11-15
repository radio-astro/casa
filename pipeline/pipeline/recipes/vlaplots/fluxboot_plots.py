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

#FLUXBOOT

print("Plotting model calibrator flux densities")


syscommand='rm -rf bootstrappedFluxDensities.png'
os.system(syscommand)

clearstat()

default('plotms')
vis=ms_active
xaxis='freq'
yaxis='amp'
ydatacolumn='model'
selectdata=True
scan=calibrator_scan_select_string
correlation=corrstring
averagedata=True
avgtime='1e8s'
avgscan=True
transform=False
extendflag=False
iteraxis=''
coloraxis='field'
plotrange=[]
title=''
xlabel=''
ylabel=''
showmajorgrid=False
showminorgrid=False
plotfile='bootstrappedFluxDensities.png'
overwrite=True
async=False
plotms()
