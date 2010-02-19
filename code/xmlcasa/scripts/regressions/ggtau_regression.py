###############################################
#                                             #
# Regression/Benchmarking Script for GG TAU   #
#                                             #
###############################################
import time
import os
# Data must be pre-filled
# ggtau_07feb97.ms

pathname=os.environ.get('CASAPATH').split()[0]
datapath=pathname+'/data/regression/ATST1/GGTAU/07feb97-g067.ms'

startTime=time.time()
startProc=time.clock()

print '--Copy MS--'
# Fill - really copy over pristine MS
os.system('rm -rf ggtau.3mm* ggtau.1mm* ggtau.hco.* ggtau.co.* ggtau_07feb97.ms ggtau.cal.split.ms')
#os.system('cp -r 07feb97-g067.ms ggtau_07feb97.ms')
copystring='cp -r '+datapath+' ggtau_07feb97.ms'
os.system(copystring)
clearcal('ggtau_07feb97.ms')
copytime=time.time()

print '--Flag data--'
default('flagdata')

# Setup four flagging specifications:
#
# 1.   Flag PdBI central data channels (32,33) that suffer ringing due to Gibbs phenomenon
# 2.   Flag channels 33,34 which are low for source 0415+379 (fieldid=0)
# 3-4. Flag channels 34-37 for CRL 618 (fieldid=3) and baseline 1-3,2-4
###  antenna names :   1  2  3  4  5
###  antenna ids   :   0  1  2  3  4

# Flag spec. 1          2          3          4

field   = ['0,1,3',   '0',       '3',       '3'      ]
spw     = ['2:31~32', '2:33~34', '2:34~37', '2:34~37']
antenna = ['',        '',        '1&3',     '2&4'    ]

# Use flagdata() in vector mode
vis='ggtau_07feb97.ms'
mode='manualflag'

flagdata()

# It is equivalent to call flagdata() four times but slower
#flagdata(vis='ggtau_07feb97.ms',field='0,1,3',spw='2:31~32')
#flagdata(vis='ggtau_07feb97.ms',field='0',spw='2:33~34')
#flagdata(vis='ggtau_07feb97.ms',field='3',antenna='1&3',spw='2:34~37')
#flagdata(vis='ggtau_07feb97.ms',field='3',antenna='2&4',spw='2:34~37')


# flag three end channels for spectral window 2, fieldid=2
#flagdata('ggtau_07feb97.ms',fieldid=[2],spwid=2,chans=[0,1,2,61,62,63])

# flag channels 250-256 for spectral window 7
#flagdata(vis='ggtau_07feb97.ms',spwid=[6])
	
flagtime=time.time()

#
# 3 mm Continuum calibration
#
print '--Continuum calibration phase/bandpass (3mm)--'
#setjy
default('setjy')
setjy('ggtau_07feb97.ms',field='3',spw='2',fluxdensity=[1.55,0.,0.,0.])

#preliminary time-dependent phase solutions to improve coherent average for bandpass solution
default('gaincal')
gaincal(vis='ggtau_07feb97.ms',caltable='ggtau.3mm.ph.gcal0',
	field='1',spw='2:14~43',
	gaintype='GSPLINE',calmode='p',splinetime=10000.,refant='1',phasewrap=260,
	gaincurve=False,opacity=0.0,preavg=120)

print '--bandpass (3mm)--'
#derive bandpass calibration for 3mm LSB
default('bandpass')
bandpass(vis='ggtau_07feb97.ms',caltable='ggtau.3mm.bpoly',
	 field='1',spw='2:3~62',
	 bandtype='BPOLY',degamp=6,degphase=12,
	 solnorm=False,maskcenter=4,maskedge=0,refant='1',
	 gaintable='ggtau.3mm.ph.gcal0',
	 gaincurve=False,opacity=0.0)

print '--gaincal phase (3mm)--'
#derive new and better phase solutions for 3mm LSB
default('gaincal')
gaincal(vis='ggtau_07feb97.ms',caltable='ggtau.3mm.ph.gcal',
	field='0,1,3',spw='2:2~61',
	gaintype='GSPLINE',calmode='p',splinetime=10000.,refant='1',phasewrap=260,
	gaintable='ggtau.3mm.bpoly',
	gaincurve=False,opacity=0.0,preavg=0.)

#Apply all solutions derived so far, determine calibrator's flux densities by solving for T and using fluxscale
default('gaincal')
gaincal(vis='ggtau_07feb97.ms',caltable='ggtau.3mm.temp',
	field='0,1,3',spw='2:2~61',
	solint='600s',refant='1',
	gaintype='T',opacity=0.0,
	gaincurve=False,gaintable=['ggtau.3mm.ph.gcal','ggtau.3mm.bpoly'])

#fluxscale
default('fluxscale')
fluxscale(vis='ggtau_07feb97.ms',caltable='ggtau.3mm.temp',fluxtable='ggtau.3mm.flux',
	  reference='CRL618*',transfer='0415+379*,0528+134*')
### Flux density for 0415+379 (spw=3) is:     5.74898 +/-  0.0267088 Jy
### Flux density for 0528+134 (spw=3) is:     2.56862 +/-  0.011453  Jy
calphase3mmtime=time.time()

print '--Set fluxscale (setjy)--'
default('setjy')
setjy(vis='ggtau_07feb97.ms',field='0',spw='2',fluxdensity=[5.849,0.,0.,0.])
setjy(vis='ggtau_07feb97.ms',field='1',spw='2',fluxdensity=[2.634,0.,0.,0.])
setjy3mmtime=time.time()

## Amplitude calibration of 3mm LSB:
##
##  phase solutions will be pre-applied as well as carried forward 
##   to the output solution table.
print '--gaincal amp (3mm)--'
default('gaincal')
gaincal(vis='ggtau_07feb97.ms',caltable='ggtau.3mm.amp.gcal',
	field='0,1,3',spw='2:2~61',
	gaintype='GSPLINE',calmode='a',splinetime=20000.,refant='1',phasewrap=260,
	gaincurve=False,opacity=0.0,
	gaintable=['ggtau.3mm.ph.gcal','ggtau.3mm.bpoly'],preavg=2500.)

calamp3mmtime=time.time()

## Correct the target source and all other 3mm LSB data: 
##
##  note that only the 60 central channels will be calibrated
##   since the BPOLY solution is only defined for these
default('applycal')
applycal(vis='ggtau_07feb97.ms',
	 spw='2',
	 gaincurve=False,opacity=0.0,
	 gaintable=['ggtau.3mm.ph.gcal','ggtau.3mm.amp.gcal','ggtau.3mm.bpoly'])
correct3mmtime=time.time()

# Split calibrater data
print '--split calibrater--'
default('split')
split(vis='ggtau_07feb97.ms',outputvis='ggtau.cal.split.ms',
	field='3',spw='2:0~63',datacolumn='model')
splitcaltime=time.time()

#Split calibrated target data
print '--split source--'
default('split')
split(vis='ggtau_07feb97.ms',outputvis='ggtau.3mm.split.ms',
      field='2',spw='2:2~61',datacolumn='corrected')
splitsrctime=time.time()

#First image the target source in 3mm continuum emission
print '--Image continuum (3mm)--'
default('clean')
clean(vis='ggtau.3mm.split.ms',imagename='ggtau.3mm',
      psfmode='clark',niter=100,gain=0.1,mode='channel',
      nchan=1,start=2,width=58,spw=0,field='0',stokes='I',
      weighting='briggs',robust=0.5,
      cell=[0.2,0.2],imsize=[256,256])
image3mmtime=time.time()

# 1mm Calibration

print '--Continuum calibration phase/bandpass (1mm)--'
default('setjy')
setjy(vis='ggtau_07feb97.ms',field='3',spw='10',fluxdensity=[2.2,0.,0.,0.])
setjy(vis='ggtau_07feb97.ms',field='3',spw='14',fluxdensity=[2.2,0.,0.,0.])
setjy(vis='ggtau_07feb97.ms',field='3',spw='18',fluxdensity=[2.2,0.,0.,0.])

   ### CRL618    spwid= 11  [I=2.2, Q=0, U=0, V=0] Jy, (user-specified)
   ### CRL618    spwid= 15  [I=2.2, Q=0, U=0, V=0] Jy, (user-specified)
   ### CRL618    spwid= 19  [I=2.2, Q=0, U=0, V=0] Jy, (user-specified)

###########################################################
## Get first cut phase solutions to improve S/N for BPass determination:
## 
default('gaincal')
gaincal(vis='ggtau_07feb97.ms',caltable='ggtau.1mm.ph.gcal0',
	field='1',spw='10,14,18:14~43',
	gaintype='GSPLINE',calmode='p',
	splinetime=10000.,refant='1',phasewrap=260,preavg=120,
	gaincurve=False,opacity=0.0)

## Derive bandpass calibration for 1mm LSB:
##
##  Avoid edge channels in each spw using setdata, not maskedge
print '--Bandpass solution (1mm)--'
default('bandpass')
bandpass(vis='ggtau_07feb97.ms',caltable='ggtau.1mm.bpoly',
	 field='1',spw='10,14,18:2~61',
	 bandtype='BPOLY',degamp=6,degphase=6,
	 visnorm=False,solnorm=False,maskcenter=4,maskedge=0,refant='1',
	 gaintable='ggtau.1mm.ph.gcal0',gaincurve=False,opacity=0.0)

## Determine phase solutions for 1mm LSB for all calibrators
##
##  Note, we are using the 3mm phase solution, transferred
##   to the 1mm bands, to reduce the phase rate.  The
##   net solution (transferred 3mm + 1mm) is written
##   to the output table
print '--Phase solutions--'
default('gaincal')
gaincal(vis='ggtau_07feb97.ms',caltable='ggtau.1mm.ph.gcal',
	field='0,1,3',spw='10,14,18:2~61',
	gaintype='GSPLINE',calmode='p',splinetime=10000.,refant='1',phasewrap=260,
	gaincurve=False,opacity=0.0,
	gaintable=['ggtau.3mm.ph.gcal','ggtau.1mm.bpoly'],preavg=0.)

##  Apply all solutions derived so far, determine
##  calibrators' flux densities using a solve for T and
##  fluxscale
print '--Solve for solutions, fluxscale--'
default('gaincal')
gaincal(vis='ggtau_07feb97.ms',caltable='ggtau.1mm.temp',
	field='0,1,3',spw='10,14,18:2~61',
	solint='600s',refant='1',gaintype='T',
	opacity=0.0,gaincurve=False,
	gaintable=['ggtau.1mm.ph.gcal','ggtau.1mm.bpoly'])
#
default('fluxscale')
fluxscale(vis='ggtau_07feb97.ms',caltable='ggtau.1mm.temp',
	  fluxtable='ggtau.1mm.flux',
	  reference='CRL618*',transfer='0415+379*,0528+134*')
calphase1mmtime=time.time()

## Record flux values from logger window.  Manually insert
## fluxes with imgr.setjy:
print '--Setjy 1mm --'
setjy(vis='ggtau_07feb97.ms',field='0',spw='10',fluxdensity=[4.310,0.,0.,0.])
setjy(vis='ggtau_07feb97.ms',field='0',spw='14',fluxdensity=[4.310,0.,0.,0.])
setjy(vis='ggtau_07feb97.ms',field='0',spw='18',fluxdensity=[4.310,0.,0.,0.])
setjy(vis='ggtau_07feb97.ms',field='1',spw='10',fluxdensity=[1.842,0.,0.,0.])
setjy(vis='ggtau_07feb97.ms',field='1',spw='14',fluxdensity=[1.842,0.,0.,0.])
setjy(vis='ggtau_07feb97.ms',field='1',spw='18',fluxdensity=[1.842,0.,0.,0.])
setjy1mmtime=time.time()


## Amplitude calibration of 1mm LSB:
default('gaincal')
gaincal(vis='ggtau_07feb97.ms',caltable='ggtau.1mm.amp.gcal',
	field='0,1,3',spw='10,14,18:2~61',
	gaintype='GSPLINE',calmode='a',splinetime=20000.,refant='1',phasewrap=260,
	gaincurve=False,opacity=0.0,preavg=2500.,
	gaintable=['ggtau.1mm.ph.gcal','ggtau.1mm.bpoly'])
calamp1mmtime=time.time()

## Correct the target source and all other 1mm LSB continuum data:
##
##  Note that edge channels in won't be calibrated because
##   BPOLY solution is not defined for them
default('applycal')
applycal(vis='ggtau_07feb97.ms',
	 spw='10,14,18',
	 gaincurve=False,opacity=0.0,
	 gaintable=['ggtau.1mm.ph.gcal','ggtau.1mm.amp.gcal','ggtau.1mm.bpoly'])
correct1mmtime=time.time()

## Split out calibrated target source 1 mm continuum data:
split(vis='ggtau_07feb97.ms',outputvis='ggtau.1mm.split.ms',
	field='2',spw='10;14;18:2~61',datacolumn='corrected')
splitsrc1mmtime=time.time()

print '--Image 1mm continuum--'
## Get a first image the target source in 1 mm continuum emission:
default('clean')
clean(vis='ggtau.1mm.split.ms',imagename='ggtau.1mm',psfmode='clark',niter=100,gain=0.1, mode='channel', nchan=1,start=3,width=58,spw=[0,1,2],field='0',stokes='I',weighting='briggs',robust=0.5,cell=[0.1,0.1],imsize=[256,256])
image1mmtime=time.time()

# 3mm HCO+(1-0) line calibration
 ### Note: during the continuum reduction above, bandpass and time-dependent
 ### phase and amplitude solutions were derived using the wider bandwidth 3 mm LSB
 ### continuum data. These can be applied directly to the line data.
 ### Since the bandpass solution for the wide, low-resolution continuum band
 ### may not be ideal for the narrow, high-resolution line band.  If there
 ### is sufficient S/R on the bandpass calibrator in the narrow line band,
 ### a bandpass solution may be obtained for it as follows:

print '--Cal HCO and split--'
default('bandpass')
bandpass(vis='ggtau_07feb97.ms',caltable='ggtau.hco.bpoly',
	 field='1',spw='6:25~230',
	 bandtype='BPOLY',degamp=1,degphase=1,visnorm=False,solnorm=False,
	 maskcenter=40,maskedge=0,refant='1',
	 gaintable='ggtau.3mm.ph.gcal0',gaincurve=False,opacity=0.0)

default('applycal')
applycal(vis='ggtau_07feb97.ms',
	 spw='6',
	 gaincurve=False,opacity=0.0,
	 gaintable=['ggtau.3mm.ph.gcal','ggtau.3mm.amp.gcal','ggtau.hco.bpoly'])
default('split')
split(vis='ggtau_07feb97.ms',outputvis='ggtau.hco.split.ms',
	field='2',spw='6:25~230',datacolumn='corrected')
hcocaltime=time.time()

print 'Image HCO--'
default('clean')
clean(vis='ggtau.hco.split.ms',imagename='ggtau.hco',
      psfmode='clark',niter=100,gain=0.1,nchan=14,start=74,width=4,
      spw=0,field='0',stokes='I',
      weighting='briggs',robust=0.5,cell=[0.2,0.2],imsize=[256,256],mode='channel')
   ### Emission in central channels is barely visible (too weak to image
   ###  with only 1 day of data but you get the point).  
imagehcotime=time.time()

# 1mm 13CO(2-1) line calibration
 ### Note: during the continuum reduction above, bandpass and time-dependent
 ### phase and amplitude solutions were derived using the wider bandwidth 1 mm LSB
 ### continuum data. These can be applied directly to the line data.
 ### The bandpass solution for the wide, low-resolution continuum band 
 ### may not be ideal for the narrow, high-resolution line band.  If there
 ### is sufficient S/R on the bandpass calibrator in the narrow line band,
 ### a bandpass solution may be obtained for it as follows:
print '--Cal CO and split--'
default('bandpass')
bandpass(vis='ggtau_07feb97.ms',caltable='ggtau.co.bpoly',
	 field='1',spw='22:8~247',
	 bandtype='BPOLY',degamp=1,degphase=1,visnorm=False,solnorm=False,
	 maskcenter=4,maskedge=0,refant='1',
	 gaintable='ggtau.1mm.ph.gcal0',gaincurve=False,opacity=0.0)
default('applycal')
applycal(vis='ggtau_07feb97.ms',
	 spw='22',
	 gaincurve=False,opacity=0.0,
	 gaintable=['ggtau.1mm.ph.gcal','ggtau.1mm.amp.gcal','ggtau.co.bpoly'])
default('split')
split(vis='ggtau_07feb97.ms',outputvis='ggtau.co.split.ms',
	field='2',spw='22:0~239',datacolumn='corrected')
cocaltime=time.time()

print '--Image CO--'
default('clean')
clean(vis='ggtau.co.split.ms',imagename='ggtau.co',
      psfmode='clark',niter=100,gain=0.1,nchan=14,start=91,width=4,
      spw=0,field='0',stokes='I',
      weighting='briggs',robust=0.5,cell=[0.1,0.1],imsize=[256,256])
   ### Emission is too weak to image with only 1 day of data.
imagecotime=time.time()

endProc=time.clock()
endTime=time.time()

#Regression

# Avoid cal max test since it was only testing the constant model data
#  (gmoellen 2008Nov12)
#ms.open(thems='ggtau.cal.split.ms')    # CONSTANT?  THE MODEL WAS SPLIT!!!
#thistest_cal=max(ms.range(["amplitude"]).get('amplitude'))
#ms.close()

ms.open(thems='ggtau.3mm.split.ms')    #  EDGE!
ms.selectchannel(1,2,56,1);  # average in channel (excluding edge channels)
thistest_3mm=max(ms.range(["amplitude"]).get('amplitude'))
ms.close()

ms.open(thems='ggtau.1mm.split.ms')    # EDGE!
ms.selectchannel(1,2,56,1);  # average in channel (excluding edge channels)
thistest_1mm=max(ms.range(["amplitude"]).get('amplitude'))
ms.close()

ms.open(thems='ggtau.hco.split.ms')    # OK
ms.selectinit(0,T);   # reset internals
ms.selectchannel(1,0,206,1);  # average in channel (excluding edge channels)
thistest_hco=max(ms.range(["amplitude"]).get('amplitude'))
ms.close()

ms.open(thems='ggtau.co.split.ms')     # OK
ms.selectchannel(1,0,240,1);  # average in channel (excluding edge channels)
thistest_co=max(ms.range(["amplitude"]).get('amplitude'))
ms.close()
ia.open(infile='ggtau.3mm.image')
statistics=ia.statistics()
thistest_immax=statistics['max'][0]
thistest_imrms=statistics['rms'][0]

#calmax=1.55
#src3mmmax=0.941    
src3mmmax=0.14297   # channel average
#src1mmmax=17.333
src1mmmax=1.6692    # channel average
#srchcomax=6.61811
srchcomax=0.49629   # channel average
#srcco21max=56.534
srcco21max=3.0728   # channel average
imgmax=0.0058886
imgrms=0.00071212

#diff_cal=abs((calmax-thistest_cal)/calmax)
diff_3mm=abs((src3mmmax-thistest_3mm)/src3mmmax)
diff_1mm=abs((src1mmmax-thistest_1mm)/src1mmmax)
diff_hco=abs((srchcomax-thistest_hco)/srchcomax)
diff_co=abs((srcco21max-thistest_co)/srcco21max)
diff_immax=abs((imgmax-thistest_immax)/imgmax)
diff_imrms=abs((imgrms-thistest_imrms)/imgrms)

import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='ggtau.'+datestring+'.log'
logfile=open(outfile,'w')

print >>logfile,'MeasurementSet Name: /home/rohir2/jmcmulli/ALMATST1/Regression/GGTAU/ggtau_07feb97.ms MS Version 2'
print >>logfile,''
print >>logfile,'   Observer:      Project: G067'
print >>logfile,'Observation: IRAM_PDB(5 antennas)'
print >>logfile,''
print >>logfile,'Data records: 86320       Total integration time = 33045 seconds'
print >>logfile,'   Observed from   14:50:27   to   00:01:12'
print >>logfile,''
print >>logfile,'Fields: 4'
print >>logfile,'  ID   Name          Right Ascension  Declination   Epoch'
print >>logfile,'  0    0415+379      04:18:21.28      +38.01.35.85  J2000'
print >>logfile,'  1    0528+134      05:30:56.41      +13.31.55.08  J2000'
print >>logfile,'  2    GG_TAU        04:32:30.34      +17.31.40.52  J2000'
print >>logfile,'  3    CRL618        04:42:53.56      +36.06.53.59  J2000'
print >>logfile,''
print >>logfile,'Spectral Windows:  (24 unique spectral windows and 1 unique polarization setups)'
print >>logfile,'  SpwID  #Chans Frame Ch1(MHz)    Resoln(kHz) TotBW(kHz)  Ref(MHz)    Corrs'
print >>logfile,'  0           1 LSRK  89188.5209  128000      128000      89188.5209  XX'
print >>logfile,'  1           1 LSRK  92264.5127  -128000     128000      92264.5127  XX'
print >>logfile,'  2          64 LSRK  89109.7709  2500        160000      89189.7709  XX'
print >>logfile,'  3          64 LSRK  92343.2627  -2500       160000      92263.2627  XX'
print >>logfile,'  4           1 LSRK  89188.5209  6000        6000        89188.5209  XX'
print >>logfile,'  5           1 LSRK  92264.5127  -6000       6000        92264.5127  XX'
print >>logfile,'  6         256 LSRK  89183.5404  39.0625     10000       89188.5404  XX'
print >>logfile,'  7         256 LSRK  92269.4932  -39.0625    10000       92264.4932  XX'
print >>logfile,'  8           1 LSRK  220253.658  128000      128000      220253.658  XX'
print >>logfile,'  9           1 LSRK  223632.358  -128000     128000      223632.358  XX'
print >>logfile,'  10         64 LSRK  220174.908  2500        160000      220254.908  XX'
print >>logfile,'  11         64 LSRK  223711.108  -2500       160000      223631.108  XX'
print >>logfile,'  12          1 LSRK  220398.681  128000      128000      220398.681  XX'
print >>logfile,'  13          1 LSRK  223487.336  -128000     128000      223487.336  XX'
print >>logfile,'  14         64 LSRK  220319.931  2500        160000      220399.931  XX'
print >>logfile,'  15         64 LSRK  223566.086  -2500       160000      223486.086  XX'
print >>logfile,'  16          1 LSRK  220543.703  128000      128000      220543.703  XX'
print >>logfile,'  17          1 LSRK  223342.314  -128000     128000      223342.314  XX'
print >>logfile,'  18         64 LSRK  220464.953  2500        160000      220544.953  XX'
print >>logfile,'  19         64 LSRK  223421.064  -2500       160000      223341.064  XX'
print >>logfile,'  20          1 LSRK  220398.681  16000       16000       220398.681  XX'
print >>logfile,'  21          1 LSRK  223487.336  -16000      16000       223487.336  XX'
print >>logfile,'  22        256 LSRK  220388.72   78.125      20000       220398.72   XX'
print >>logfile,'  23        256 LSRK  223497.297  -78.125     20000       223487.297  XX'
print >>logfile,''
print >>logfile,'Antennas: 5'
print >>logfile,'   ID=   1-4: 1=223, 2=323, 3=227, 4=316, 5=129'
print >>logfile,''
print >>logfile,'Tables(rows):   (-1 = table absent)'
print >>logfile,'   MAIN(86320)'
print >>logfile,'   ANTENNA(5)   DATA_DESCRIPTION(24)   DOPPLER(-1)   FEED(5)   FIELD(4)'
print >>logfile,'   FLAG_CMD(0)   FREQ_OFFSET(-1)   HISTORY(213)   OBSERVATION(1)   POINTING(0)'
print >>logfile,'   POLARIZATION(1)   PROCESSOR(0)   SOURCE(4)   SPECTRAL_WINDOW(24)   STATE(0)'
print >>logfile,'   SYSCAL(-1)   WEATHER(-1)'

print >>logfile,'********** Regression ***********'
print >>logfile,'*                               *'
#if (diff_cal<0.05): print >>logfile,'* Passed cal max amplitude test '
#print >>logfile,'--Cal max amp '+str(thistest_cal)
if (diff_3mm<0.05): print >>logfile,'* Passed 3mm max amplitude test: '
else: print >>logfile,'* FAILED 3mm max amplitude test: '
print >>logfile,'--3mm max amp '+str(thistest_3mm)+' ('+str(src3mmmax)+')'
if (diff_1mm<0.05): print >>logfile,'* Passed 1mm max amplitude test '
else: print >>logfile,'* FAILED 1mm max amplitude test '
print >>logfile,'--1mm max amp '+str(thistest_1mm)+' ('+str(src1mmmax)+')'
if (diff_hco<0.05): print >>logfile,'* Passed HCO max amplitude test '
else: print >>logfile,'* FAILED HCO max amplitude test '
print >>logfile,'--HCO max amp '+str(thistest_hco)+' ('+str(srchcomax)+')'
if (diff_co<0.05): print >>logfile,'* Passed CO max amplitude test '
else: print >>logfile,'* FAILED CO max amplitude test '
print >>logfile,'--CO max amp '+str(thistest_co)+' ('+str(srcco21max)+')'
if (diff_immax<0.05): print >>logfile,'* Passed image max test'
else: print >>logfile,'* FAILED image max test'
print >>logfile,'--Image max '+str(thistest_immax)+' ('+str(imgmax)+')'
if (diff_imrms<0.05): print >>logfile,'* Passed image rms test'
else: print >>logfile,'* FAILED image rms test'
print >>logfile,'--Image rms '+str(thistest_imrms)+' ('+str(imgrms)+')'

#if ((diff_cal<0.05) & (diff_3mm<0.05) & (diff_1mm<0.05) & (diff_hco<0.05) & (diff_co<0.05) & (diff_immax<0.05) & (diff_imrms<0.05)):
if ((diff_3mm<0.05) & (diff_1mm<0.05) & (diff_hco<0.05) & (diff_co<0.05) & (diff_immax<0.05) & (diff_imrms<0.05)):
	regstate=True
        print >>logfile,'---'
        print >>logfile,'Passed Regression test for GG TAU'
        print >>logfile,'---'
else:
	regstate=False
        print >>logfile,'----FAILED Regression test for GG TAU'
print >>logfile,'*********************************'

print >>logfile,''
print >>logfile,''
print >>logfile,'********* Benchmarking *****************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
print >>logfile,'Processing rate MB/s  was: '+str(278./(endTime - startTime))
print >>logfile,'* Breakdown:                           *'
print >>logfile,'*   copy         time was: '+str(copytime-startTime)
print >>logfile,'*   flag         time was: '+str(flagtime-copytime)
print >>logfile,'*   cal ph 3mm   time was: '+str(calphase3mmtime-flagtime)
print >>logfile,'*   setjy 3mm    time was: '+str(setjy3mmtime-calphase3mmtime)
print >>logfile,'*   cal amp 3mm  time was: '+str(calamp3mmtime-setjy3mmtime)
print >>logfile,'*   correct 3mm  time was: '+str(correct3mmtime-calamp3mmtime)
print >>logfile,'*   split cal    time was: '+str(splitcaltime-correct3mmtime)
print >>logfile,'*   split src    time was: '+str(splitsrctime-splitcaltime)
print >>logfile,'*   image src    time was: '+str(image3mmtime-splitsrctime)
print >>logfile,'*   cal ph 1mm   time was: '+str(calphase1mmtime-image3mmtime)
print >>logfile,'*   setjy 1mm    time was: '+str(setjy1mmtime-calphase1mmtime)
print >>logfile,'*   cal amp 1mm  time was: '+str(calamp1mmtime-setjy1mmtime)
print >>logfile,'*   correct 1mm  time was: '+str(correct1mmtime-calamp1mmtime)
print >>logfile,'*   splitsrc 1mm time was: '+str(splitsrc1mmtime-correct1mmtime)
print >>logfile,'*   image 1mm    time was: '+str(image1mmtime-splitsrc1mmtime)
print >>logfile,'*   HCO cal      time was: '+str(hcocaltime-image1mmtime)
print >>logfile,'*   image HCO    time was: '+str(imagehcotime-hcocaltime)
print >>logfile,'*   CO cal       time was: '+str(cocaltime-imagehcotime)
print >>logfile,'*   image CO     time was: '+str(imagecotime-cocaltime)

logfile.close()
