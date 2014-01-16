###############################################
#                                             #
# Regression/Benchmarking Script for B0319    #
#                                             #
###############################################

import time
import os

os.system('rm -rf B0319_0317.ms B0319.K B0319.Mt B0319.MFt B0319*.png n1333.ms')

pathname=os.environ.get('CASAPATH').split()[0]
datapath = os.environ.get('CASAPATH').split()[0]+'/data/regression/ATST4/B0319/N1333_1.UVFITS'

startTime = time.time()
startProc = time.clock()


# Baseline-based calibration of N1333 calibrater  
# VLA baseline 3-17 02-May-2003
#           MeasurementSet Name:  B0319_0317.ms      MS Version 2
#
#   Observer: AW602     Project:
#Observation: VLA
#Data records: 62       Total integration time = 609.999 seconds
#   Observed from   02-May-2003/20:09:40   to   02-May-2003/20:19:50
#
#   ObservationID = 1         ArrayID = 1
#  Date        Timerange                Scan  FldId FieldName      DataDescIds
#  02-May-2003/20:09:40.0 - 20:19:50.0    49      1 0319+415_1     [1]
#Fields: 1
#  ID   Name          Right Ascension  Declination   Epoch
#  1    0319+415_1    03:19:48.16      +41.30.42.10  J2000
#Data descriptions: 1 (1 spectral windows and 1 polarization setups)
#  ID  #Chans Frame Ch1(MHz)    Resoln(kHz) TotBW(kHz)  Ref(MHz)    Corrs
#  1       63 LSRK  43416.2392  97.65625    6250        43419.2666  RR  LL
#Feeds: 29: printing first row only
#  Antenna   Spectral Window     # Receptors    Polarizations
#  1         -1                  2              [         R, L]
#Antennas: 2:
#  ID   Name  Station   Diam.    Long.         Lat.
#  3    3     VLA:E2    25.0 m   -107.37.04.4  +33.54.01.1
#  17   17    VLA:E3    25.0 m   -107.37.02.8  +33.54.00.5

# Examine observed data


#   N1333
#   =====
#FIELDID  NAME       PURPOSE              COMMENT
#-------  ----       -------              -------
#1        0336_323_1 Gain Calibrater
#13       0542+498_1 Flux Calibrater      (3C147)
#15       0319+415_1 Band Pass Calibrater (3C84)

startTime = time.time()
startProc = time.clock()

print '--Import--'
default('importuvfits')
importuvfits(fitsfile=datapath,vis='n1333.ms',antnamescheme='new')
importtime=time.time()
print '--Split Data--'
default('split')
split(vis='n1333.ms',outputvis='B0319_0317.ms',datacolumn='data',field='14',antenna='VA03 & VA17')
splittime=time.time()

#Calibrate data
clearcal(vis='B0319_0317.ms')
default('blcal')
blcal(vis='B0319_0317.ms',caltable='B0319.Mt',
      solint='3s',combine='')
default('blcal')
blcal(vis='B0319_0317.ms',caltable='B0319.MFt',
      gaintable='B0319.Mt',interp='nearest',
      solint='inf',combine='scan',
      freqdep=True)
default('applycal')
applycal(vis='B0319_0317.ms',
	 gaintable=['B0319.Mt','B0319.MFt'])
calibratetime=time.time()

#Examine the calibration solutions
default('plotcal')
plotcal(caltable='B0319.Mt',yaxis='phase',subplot=121,plotsymbol='bo',clearpanel='All')
plotcal(caltable='B0319.Mt',yaxis='amp',subplot=122,plotsymbol='bo',
	overplot=True,clearpanel='Auto')

default('plotcal')
plotcal(caltable='B0319.MFt',yaxis='phase',
	subplot=121,plotsymbol='bo',clearpanel='All')
plotcal(caltable='B0319.MFt',yaxis='amp',
	subplot=122,plotsymbol='bo',overplot=True,clearpanel='Auto')
plotcaltime=time.time()

#default('plotcal')
#plotcal('B0319.K','delay',subplot=121,plotsymbol='go',clearpanel=True)
#plotcal('B0319.K','delayrate',subplot=122,plotsymbol='go',clearpanel=False)

# uv model fit the data
default('uvmodelfit')
uvmodelfit(vis='B0319_0317.ms',niter=5,comptype='P',
	   sourcepar=[0.5,.1,.1],outfile='test.cl')
uvmodelfittime=time.time()

# now use component list to generate model data
default('ft')
ft(vis='B0319_0317.ms',complist='test.cl')
fttime=time.time()


endProc = time.clock()
endTime = time.time()

#Regression

#test_name_amp = 'B0319 -- visibility max amplitude test'
#test_name_ph  = 'B0319 -- visibility max phase test'
#test_name_mod = 'B0319 -- visibility max model test'

ms.open('B0319_0317.ms')
#thistest_amp=pl.mean(ms.range(['corrected_amplitude']).get('corrected_amplitude'))
#thistest_ph =pl.mean(ms.range(['corrected_phase']).get('corrected_phase'))
thistest_mod=pl.mean(ms.range(['model_amplitude']).get('model_amplitude'))
ms.close()

#model amplitude
model_amp=1.0

diff_mod=abs((model_amp-thistest_mod)/model_amp)

print '***'
print 'model_amp ',model_amp
print 'thistest_mod ',thistest_mod
print '***'

import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='B0319.'+datestring+'.log'
logfile=open(outfile,'w')

print >>logfile,' Baseline-based calibration of N1333 calibrater'
print >>logfile,' VLA baseline 3-17 02-May-2003'
print >>logfile,'           MeasurementSet Name:  B0319_0317.ms      MS Version 2'
print >>logfile,''
print >>logfile,'   Observer: AW602     Project:'
print >>logfile,'Observation: VLA'
print >>logfile,'Data records: 62       Total integration time = 609.999 seconds'
print >>logfile,'   Observed from   02-May-2003/20:09:40   to   02-May-2003/20:19:50'
print >>logfile,''
print >>logfile,'   ObservationID = 1         ArrayID = 1'
print >>logfile,'  Date        Timerange                Scan  FldId FieldName      DataDescIds'
print >>logfile,'  02-May-2003/20:09:40.0 - 20:19:50.0    49      1 0319+415_1     [1]'
print >>logfile,'Fields: 1'
print >>logfile,'  ID   Name          Right Ascension  Declination   Epoch'
print >>logfile,'  1    0319+415_1    03:19:48.16      +41.30.42.10  J2000'
print >>logfile,'Data descriptions: 1 (1 spectral windows and 1 polarization setups)'
print >>logfile,'  ID  #Chans Frame Ch1(MHz)    Resoln(kHz) TotBW(kHz)  Ref(MHz)    Corrs'
print >>logfile,'  1       63 LSRK  43416.2392  97.65625    6250        43419.2666  RR  LL'
print >>logfile,'Feeds: 29: printing first row only'
print >>logfile,'  Antenna   Spectral Window     # Receptors    Polarizations'
print >>logfile,'  1         -1                  2              [         R, L]'
print >>logfile,'Antennas: 2:'
print >>logfile,'  ID   Name  Station   Diam.    Long.         Lat.'
print >>logfile,'  3    3     VLA:E2    25.0 m   -107.37.04.4  +33.54.01.1'
print >>logfile,'  17   17    VLA:E3    25.0 m   -107.37.02.8  +33.54.00.5'
print >>logfile,''
print >>logfile,'********** Regression ***********'
print >>logfile,'*                               *'
if (diff_mod<0.05): print >>logfile,'* Passed Model data test'
print >>logfile,'* Model data mean'+str(thistest_mod)+','+str(model_amp)

if (diff_mod<0.05):
	regstate=True
	print >>logfile,'---'
	print >>logfile,'Passed Regression test for B0319'
	print >>logfile,'---'
else:
	regstate=False
	print >>logfile,'----FAILED Regression test for B0319'
print >>logfile,'*********************************'

print >>logfile,''
print >>logfile,''
print >>logfile,'********* Benchmarking *****************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
#print >>logfile,'Processing rate MB/s  was: '+str(5.0/(endTime - startTime)
print >>logfile,'* Breakdown:                           *'
print >>logfile,'*   import       time was: '+str(importtime-startTime)
print >>logfile,'*   split        time was: '+str(splittime-importtime)
print >>logfile,'*   calibrate    time was: '+str(calibratetime-splittime)
print >>logfile,'*   plotcal      time was: '+str(plotcaltime-calibratetime)
print >>logfile,'*   uvmodelfit   time was: '+str(uvmodelfittime-plotcaltime)
print >>logfile,'*   ft           time was: '+str(fttime-uvmodelfittime)
print >>logfile,'*****************************************'

logfile.close()
