###############################################
#                                             #
# Regression/Benchmarking Script for L02D     #
#             26 JAN               (U Cam)    #
###############################################
import time
import os
# Data must be pre-filled
# l.ms, l2.ms

pathname=os.environ.get('CASAPATH').split()[0]
datapath=pathname+'/data/regression/ATST1/L02D/l*.ms'

startTime=time.time()
startProc=time.clock()

print '--Copy MS--'
# Fill - really copy over pristine MS
os.system('rm -rf l.* l2.* all.* l1.* *.ms* *.image *.model *.residual')
copystring='cp -r '+datapath+' .'
os.system(copystring)
clearcal(vis='l.ms')
clearcal(vis='l2.ms')
copytime=time.time()

print '--Flag data--'
default('tflagdata')
#flagdata(vis='l.ms',
#	 timerange=['2002/01/27/05:45:47.0', '2002/01/27/07:00:00.0'],
#	 fieldid=-1)
tflagdata(vis='l.ms',
	 timerange='2002/01/27/05:45:47.0~2002/01/27/07:00:00.0')
flagtime=time.time()

#
# l1 calibration 
#
print '--Calibration phase/bandpass (3mm)--'
#setjy
default('setjy')
setjy(vis='l.ms',field='0',spw='6',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l.ms',field='0',spw='14',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l.ms',field='0',spw='18',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l.ms',field='0',spw='7',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l.ms',field='0',spw='15',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l.ms',field='0',spw='19',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l.ms',field='0',spw='3',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l.ms',field='0',spw='11',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l.ms',field='0',spw='26',scalebychan=False,fluxdensity=[6.0,0,0,0])
setjy(vis='l.ms',field='0',spw='30',scalebychan=False,fluxdensity=[6.0,0,0,0])
setjy(vis='l.ms',field='0',spw='27',scalebychan=False,fluxdensity=[6.0,0,0,0])
setjy(vis='l.ms',field='0',spw='31',scalebychan=False,fluxdensity=[6.0,0,0,0])

# 3mm USB
#preliminary time-dependent phase solutions to improve coherent average for bandpass solution
default('gaincal')
gaincal(vis='l.ms',caltable='l.3mmUSB.gcal0',
	field='0',spw='7,15,19:9~118',
	gaintype='GSPLINE',calmode='p',splinetime=10000.,
	refant='1',phasewrap=260,
	preavg=0)

print '--bandpass (3mm)--'
#derive bandpass calibration for 3mm (C34S J=2-1) using USB continuum phase solns
default('bandpass')
bandpass(vis='l.ms',caltable='l.3mmC34S.bpoly',
	 field='0',spw='3',
	 bandtype='BPOLY',degamp=2,degphase=2,solnorm=False,
	 maskcenter=2,maskedge=0,refant='1',
	 gaintable='l.3mmUSB.gcal0')

# Band pass for CH3OH,3mm continuum USB/LSB
default('bandpass')
bandpass(vis='l.ms',caltable='l.3mmch3oh.bpoly',
	 field='0',spw='11',
	 bandtype='BPOLY',degamp=2,degphase=4,solnorm=False,
	 maskcenter=2,maskedge=0,refant='1',
	 gaintable='l.3mmUSB.gcal0')
default('bandpass')
bandpass(vis='l.ms',caltable='l.3mmcont.bpoly',
	 field='0',spw='7,15,19',
	 bandtype='BPOLY',degamp=10,degphase=25,solnorm=False,
	 maskcenter=2,maskedge=0,refant='1',
	 gaintable='l.3mmUSB.gcal0')
default('bandpass')
bandpass(vis='l.ms',caltable='l.3mmcont.bpoly',
	 field='0',spw='6,14,18',
	 bandtype='BPOLY',degamp=10,degphase=25,solnorm=False,
	 maskcenter=2,maskedge=0,refant='1',
	 gaintable='l.3mmUSB.gcal0',append=True)

print '--gaincal phase (3mm)--'
#derive new and better phase solutions for 3mm LSB
default('gaincal')
gaincal(vis='l.ms',caltable='l.3mmcont.gcal',
	field='0,1,2',spw='6,14,18,7,15,19',
	gaintype='GSPLINE',calmode='p',splinetime=3000.,refant='1',
	phasewrap=260,
	gaintable='l.3mmcont.bpoly',preavg=0.)

#Apply all solutions derived so far, determine calibrator's flux densities by solving for T 
#and use fluxscale
default('gaincal')
gaincal(vis='l.ms',caltable='l.3mmcont.temp',
	field='0,1,2',spw='7,15,19,6,14,18',
	solint='600s',refant='1',gaintype='T',
	gaintable=['l.3mmcont.gcal','l.3mmcont.bpoly'])

#fluxscale
default('fluxscale')
fluxscale(vis='l.ms',caltable='l.3mmcont.temp',fluxtable='l.3mmcont.flux',
	  reference='3C273*',transfer='MWC349*,2013+370*')
calphase3mmtime=time.time()
#MWC349 1.14
#2013+370 3.25

#use old values
print '--Set fluxscale (setjy)--'
default('setjy')
setjy(vis='l.ms',field='1',spw='6',scalebychan=False,fluxdensity=[1.01,0.,0.,0.])
setjy(vis='l.ms',field='1',spw='7',scalebychan=False,fluxdensity=[1.01,0.,0.,0.])
setjy(vis='l.ms',field='1',spw='14',scalebychan=False,fluxdensity=[1.01,0.,0.,0.])
setjy(vis='l.ms',field='1',spw='15',scalebychan=False,fluxdensity=[1.01,0.,0.,0.])
setjy(vis='l.ms',field='1',spw='18',scalebychan=False,fluxdensity=[1.01,0.,0.,0.])
setjy(vis='l.ms',field='1',spw='19',scalebychan=False,fluxdensity=[1.01,0.,0.,0.])
setjy(vis='l.ms',field='2',spw='6',scalebychan=False,fluxdensity=[2.79,0.,0.,0.])
setjy(vis='l.ms',field='2',spw='7',scalebychan=False,fluxdensity=[2.79,0.,0.,0.])
setjy(vis='l.ms',field='2',spw='14',scalebychan=False,fluxdensity=[2.79,0.,0.,0.])
setjy(vis='l.ms',field='2',spw='15',scalebychan=False,fluxdensity=[2.79,0.,0.,0.])
setjy(vis='l.ms',field='2',spw='18',scalebychan=False,fluxdensity=[2.79,0.,0.,0.])
setjy(vis='l.ms',field='2',spw='19',scalebychan=False,fluxdensity=[2.79,0.,0.,0.])
setjy3mmtime=time.time()

## Amplitude calibration of 3mm continuum:
##
##  phase solutions will be pre-applied as well as carried forward 
##   to the output solution table.
print '--gaincal amp (3mm)--'
default('gaincal')
gaincal(vis='l.ms',caltable='l.3mmcont.amp.gcal',
	field='0,1,2',spw='7,15,19,6,14,18',
	gaintype='GSPLINE',calmode='a',splinetime=3000.,refant='1',
	phasewrap=260,
	preavg=0.,
	gaintable=['l.3mmcont.gcal','l.3mmcont.bpoly'])
calamp3mmtime=time.time()

## Correct the target source C34S (J=2-1)
##
##  note that only the 60 central channels will be calibrated
##   since the BPOLY solution is only defined for these
default('applycal')
applycal(vis='l.ms',
	 field='3',spw='3',
	 gaintable=['l.3mmcont.gcal','l.3mmcont.amp.gcal','l.3mmC34S.bpoly']);
# Correct Target other 3mm line data (CH3OH)
applycal(vis='l.ms',
	 field='3',spw='11',
	 gaintable=['l.3mmcont.gcal','l.3mmcont.amp.gcal','l.3mmch3oh.bpoly'])
# Correct Target source 3mm continuum 
applycal(vis='l.ms',
	 field='3',spw='7,15,19,6,14,18',
	 gaintable=['l.3mmcont.gcal','l.3mmcont.amp.gcal','l.3mmcont.bpoly'])
correct3mmtime=time.time()

# 1mm Calibration

###########################################################
## Get first cut phase solutions to improve S/N for BPass determination:
## 
default('gaincal')
gaincal(vis='l.ms',caltable='l.1mmUSB.gcal0',
	field='0',spw='27,31:9~118',
	gaintype='GSPLINE',calmode='p',splinetime=7000.,refant='1',
	phasewrap=260,preavg=0.)

## Derive bandpass calibration for 1mm 
##
bandpass(vis='l.ms',caltable='l.1mmcont.bpoly',
	 field='0',spw='27,31',
	 bandtype='BPOLY',degamp=10,degphase=20,visnorm=False,solnorm=False,
	 maskcenter=2,maskedge=0,refant='1',
	 gaintable='l.1mmUSB.gcal0')
bandpass(vis='l.ms',caltable='l.1mmcont.bpoly',
	 field='0',spw='26,30',
	 bandtype='BPOLY',degamp=10,degphase=20,visnorm=False,solnorm=False,
	 maskcenter=2,maskedge=0,refant='1',
	 gaintable='l.1mmUSB.gcal0',append=True)
bandpass(vis='l.ms',caltable='l.1mmUSB.bpoly',
	 field='0',spw='23',
	 bandtype='BPOLY',degamp=10,degphase=20,visnorm=False,solnorm=False,
	 maskcenter=2,maskedge=0,refant='1',
	 gaintable='l.1mmUSB.gcal0')

## Determine phase solutions for 1mm LSB & USB 
##
print '--Phase solutions--'
default('gaincal')
gaincal(vis='l.ms',caltable='l.1mmcont.gcal',
	field='0,1,2',spw='26,27,30,31',
	gaintype='GSPLINE',calmode='p',splinetime=3000.,refant='1',
	phasewrap=260,
	gaintable='l.1mmcont.bpoly',preavg=0.)

##  Apply all solutions derived so far, determine
##  calibrators' flux densities using a solve for T and
##  fluxscale
print '--Solve for solutions, fluxscale--'
default('gaincal')
gaincal(vis='l.ms',caltable='l.1mmcont.temp',
	field='0,1,2',spw='26,27,30,31',
	solint='1200s',refant='1',gaintype='T',
	gaintable=['l.1mmcont.gcal','l.1mmcont.bpoly'])
#
default('fluxscale')
fluxscale(vis='l.ms',caltable='l.1mmcont.temp',fluxtable='l.1mmcont.flux',
	  reference='3C273*',transfer='MWC349*,2013+370*')
#MWC349   2.7	1.74
#2013+370 3.8	2.44
calphase1mmtime=time.time()

## Record flux values from logger window.  Manually insert
## fluxes with imgr.setjy:
print '--Setjy 1mm --'
setjy(vis='l.ms',field='1',spw='27',scalebychan=False,fluxdensity=[1.7,0.,0.,0.])
setjy(vis='l.ms',field='1',spw='31',scalebychan=False,fluxdensity=[1.7,0.,0.,0.])
setjy(vis='l.ms',field='2',spw='27',scalebychan=False,fluxdensity=[1.8,0.,0.,0.])
setjy(vis='l.ms',field='2',spw='31',scalebychan=False,fluxdensity=[1.8,0.,0.,0.])
setjy(vis='l.ms',field='1',spw='27',scalebychan=False,fluxdensity=[1.7,0.,0.,0.])
setjy(vis='l.ms',field='1',spw='31',scalebychan=False,fluxdensity=[1.7,0.,0.,0.])
setjy(vis='l.ms',field='2',spw='27',scalebychan=False,fluxdensity=[1.8,0.,0.,0.])
setjy(vis='l.ms',field='2',spw='31',scalebychan=False,fluxdensity=[1.8,0.,0.,0.])
setjy1mmtime=time.time()


## Amplitude calibration of 1mm LSB:
default('gaincal')
gaincal(vis='l.ms',caltable='l.1mmcont.amp.gcal',
	field='0,1,2',spw='26,27,30,31',
	gaintype='GSPLINE',calmode='a',splinetime=3000.,refant='1',
	phasewrap=260,
	preavg=0.,
	gaintable=['l.1mmcont.gcal','l.1mmcont.bpoly'])
calamp1mmtime=time.time()

## Correct the target source 1mm LSB & USB continuum data:
##
##  Note that edge channels in won't be calibrated because
##   BPOLY solution is not defined for them
default('applycal')
applycal(vis='l.ms',
	 field='3',spw='26,27,30,31',
	 gaintable=['l.1mmcont.gcal','l.1mmcont.amp.gcal','l.1mmcont.bpoly'])
default('applycal')
applycal(vis='l.ms',
	 field='3',spw='23',
	 gaintable=['l.1mmcont.gcal','l.1mmcont.amp.gcal','l.1mmcont.bpoly'],
	 spwmap=[[-1],[-1],[26]])
correct1mmtime=time.time()

## Split out calibrated target source 1 mm continuum data:
default('split')
split(vis='l.ms',outputvis='l.1mm.split.ms',
      field='3',spw='26:8~117,27:8~117,30:8~117,31:8~117',
      datacolumn='corrected')
default('split')
split(vis='l.ms',outputvis='l.1mmc34s5_4.split.ms',
      field='3',spw='23:0~511',
      datacolumn='corrected')
splitsrc1mmtime=time.time()

print '--Image 1mm continuum--'
## Get a first image the target source in 1 mm continuum emission:
default('clean')
clean(vis='l.1mm.split.ms',imagename='l.1mm',
      psfmode='clark',imagermode='',niter=100,gain=0.1,nchan=1,start=0,width=1,
      spw='0~3',field='0',stokes='I',interpolation='nearest',
      weighting='natural',
      cell=[0.2,0.2],imsize=[128,128],mode='mfs')
image1mmtime=time.time()

print '--Resplit data--'
default('split')
split(vis='l.ms',outputvis='l1.c34s.split.ms',
      field='3',spw='3:0~511',datacolumn='corrected')
default('split')
split(vis='l.ms',outputvis='l1.ch3oh.split.ms',
      field='3',spw='11:0~511',datacolumn='corrected')
default('split')
split(vis='l.ms',outputvis='l1.3mmcont.split.ms',
      field='3',
      spw='6:9~108,7:9~108,14:9~108,15:9~108,18:9~108,19:9~108',
      width=[100,100,100,100,100,100],
      datacolumn='corrected')
default('split')
split(vis='l.ms',outputvis='l1.1mmcont.split.ms',
      field='3',
      spw='26:9~118,27:9~118,30:9~118,31:9~118',
      width=[110,110,110,110],
      datacolumn='corrected')
default('split')
split(vis='l.ms',outputvis='l1.1mmc34s5_4.split.ms',
      field='3',spw='23:0~511',datacolumn='corrected')
splitsrc1mmtime=time.time()



###############################################
#                                             #
# Regression/Benchmarking Script for L02D     #
#            29 JAN                (U Cam)    #
###############################################

print '--Flag data--'
default('tflagdata')
#flagdata(vis='l2.ms',
#	 timerange=['2002/01/29/03:37:50.0', '2002/01/29/04:02:48.0'],
#	 fieldid=-1)
tflagdata(vis='l2.ms',
	 timerange='2002/01/29/03:37:50.0~2002/01/29/04:02:48.0')
flagtime=time.time()

#
# l2 calibration 
#
print '--Calibration phase/bandpass (3mm)--'
#setjy
default('setjy')
setjy(vis='l2.ms',field='0',spw='6',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l2.ms',field='0',spw='14',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l2.ms',field='0',spw='18',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l2.ms',field='0',spw='7',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l2.ms',field='0',spw='15',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l2.ms',field='0',spw='19',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l2.ms',field='0',spw='3',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l2.ms',field='0',spw='11',scalebychan=False,fluxdensity=[10.87,0.,0.,0.])
setjy(vis='l2.ms',field='0',spw='26',scalebychan=False,fluxdensity=[6.0,0,0,0])
setjy(vis='l2.ms',field='0',spw='30',scalebychan=False,fluxdensity=[6.0,0,0,0])
setjy(vis='l2.ms',field='0',spw='27',scalebychan=False,fluxdensity=[6.0,0,0,0])
setjy(vis='l2.ms',field='0',spw='31',scalebychan=False,fluxdensity=[6.0,0,0,0])

# 3mm USB
#preliminary time-dependent phase solutions to improve coherent average for bandpass solution
default('gaincal')
gaincal(vis='l2.ms',caltable='l2.3mmUSB.gcal0',
	field='0',spw='7,15,19:9~118',
	gaintype='GSPLINE',calmode='p',splinetime=10000.,refant='1',
	phasewrap=260,
	preavg=0)

print '--bandpass (3mm)--'
#derive bandpass calibration for 3mm (C34S J=2-1) using USB continuum phase solns
default('bandpass')
bandpass(vis='l2.ms',caltable='l2.3mmC34S.bpoly',
	 field='0',spw='3',
	 bandtype='BPOLY',degamp=2,degphase=2,solnorm=False,
	 maskcenter=2,maskedge=0,refant='1',
	 gaintable='l2.3mmUSB.gcal0')

# Band pass for CH3OH,3mm continuum USB/LSB
default('bandpass')
bandpass(vis='l2.ms',caltable='l2.3mmch3oh.bpoly',
	 field='0',spw='11',
	 bandtype='BPOLY',degamp=10,degphase=20,solnorm=False,
	 maskcenter=2,maskedge=0,refant='1',
	 gaintable='l2.3mmUSB.gcal0')
default('bandpass')
bandpass(vis='l2.ms',caltable='l2.3mmcont.bpoly',
	 field='0',spw='7,15,19',
	 bandtype='BPOLY',degamp=10,degphase=25,solnorm=False,
	 maskcenter=2,maskedge=0,refant='1',
	 gaintable='l2.3mmUSB.gcal0')
default('bandpass')
bandpass(vis='l2.ms',caltable='l2.3mmcont.bpoly',
	 field='0',spw='6,14,18',
	 bandtype='BPOLY',degamp=10,degphase=25,solnorm=False,
	 maskcenter=2,maskedge=0,refant='1',
	 gaintable='l2.3mmUSB.gcal0',append=True)

print '--gaincal phase (3mm)--'
#derive new and better phase solutions for 3mm LSB
default('gaincal')
gaincal(vis='l2.ms',caltable='l2.3mmcont.gcal',
	field='0,1,2',spw='6,14,18,7,15,19',
	gaintype='GSPLINE',calmode='p',splinetime=10000.,refant='1',
	phasewrap=260,
	gaintable='l2.3mmcont.bpoly',preavg=0.)

#Apply all solutions derived so far, determine calibrator's flux densities by solving for T 
#and use fluxscale
default('gaincal')
gaincal(vis='l2.ms',caltable='l2.3mmcont.temp',
	field='0,1,2',spw='7,15,19,6,14,18',
	solint='600s',refant='1',gaintype='T',
	gaintable=['l2.3mmcont.gcal','l2.3mmcont.bpoly'])

#fluxscale
default('fluxscale')
fluxscale(vis='l2.ms',caltable='l2.3mmcont.temp',fluxtable='l2.3mmcont.flux',
	  reference='3C273*',transfer='MWC349*,2013+370*')
calphase3mmtime=time.time()
#MWC249 = 0.82 Jy
#2013+370 = 2.60 Jy

print '--Set fluxscale (setjy)--'
default('setjy')
setjy(vis='l2.ms',field='1',spw='6',scalebychan=False,fluxdensity=[1.01,0.,0.,0.])
setjy(vis='l2.ms',field='1',spw='7',scalebychan=False,fluxdensity=[1.01,0.,0.,0.])
setjy(vis='l2.ms',field='1',spw='14',scalebychan=False,fluxdensity=[1.01,0.,0.,0.])
setjy(vis='l2.ms',field='1',spw='15',scalebychan=False,fluxdensity=[1.01,0.,0.,0.])
setjy(vis='l2.ms',field='1',spw='18',scalebychan=False,fluxdensity=[1.01,0.,0.,0.])
setjy(vis='l2.ms',field='1',spw='19',scalebychan=False,fluxdensity=[1.01,0.,0.,0.])
setjy(vis='l2.ms',field='2',spw='6',scalebychan=False,fluxdensity=[2.79,0.,0.,0.])
setjy(vis='l2.ms',field='2',spw='7',scalebychan=False,fluxdensity=[2.79,0.,0.,0.])
setjy(vis='l2.ms',field='2',spw='14',scalebychan=False,fluxdensity=[2.79,0.,0.,0.])
setjy(vis='l2.ms',field='2',spw='15',scalebychan=False,fluxdensity=[2.79,0.,0.,0.])
setjy(vis='l2.ms',field='2',spw='18',scalebychan=False,fluxdensity=[2.79,0.,0.,0.])
setjy(vis='l2.ms',field='2',spw='19',scalebychan=False,fluxdensity=[2.79,0.,0.,0.])
setjy3mmtime=time.time()

## Amplitude calibration of 3mm continuum:
##
##  phase solutions will be pre-applied as well as carried forward 
##   to the output solution table.
print '--gaincal amp (3mm)--'
default('gaincal')
gaincal(vis='l2.ms',caltable='l2.3mmcont.amp.gcal',
	field='0,1,2',spw='7,15,19,6,14,18',
	gaintype='GSPLINE',calmode='a',splinetime=10000.,
	refant='1',phasewrap=260,
	preavg=0.,
	gaintable=['l2.3mmcont.gcal','l2.3mmcont.bpoly'])
calamp3mmtime=time.time()

## Correct the target source C34S (J=2-1)
##
##  note that only the 60 central channels will be calibrated
##   since the BPOLY solution is only defined for these
default('applycal')
applycal(vis='l2.ms',
	 field='3',spw='3',
	 gaintable=['l2.3mmcont.gcal','l2.3mmcont.amp.gcal','l2.3mmC34S.bpoly'])
# Correct Target other 3mm line data (CH3OH)
default('applycal')
applycal(vis='l2.ms',
	 field='3',spw='11',
	 gaintable=['l2.3mmcont.gcal','l2.3mmcont.amp.gcal','l2.3mmch3oh.bpoly'])
# Correct Target source 3mm continuum 
default('applycal')
applycal(vis='l2.ms',
	 field='3',spw='7,15,19,6,14,18',
	 gaintable=['l2.3mmcont.gcal','l2.3mmcont.amp.gcal','l2.3mmcont.bpoly'])
correct3mmtime=time.time()

# Split calibrated data
print '--split calibrater--'
default('split')
split(vis='l2.ms',outputvis='l2.3mmconta.split.ms',
      field='3',
      spw='6:10~109,7:10~109,14:10~109,15:10~109,18:10~109,19:10~109',
      width=[100,100,100,100,100,100],
      datacolumn='corrected')
splitsrctime=time.time()

#Image target source in 3mm continuum emission
default('clean')
clean(vis='l2.3mmconta.split.ms',imagename='l2.3mmcont',
      psfmode='clark',imagermode='',niter=100,gain=0.1,nchan=1,start=0,width=1,
      spw='0~5',field='0',stokes='I',
      weighting='natural',interpolation='nearest',
      cell=[0.4,0.4],imsize=[256,256],mode='mfs')


# 1mm Calibration

###########################################################
## Get first cut phase solutions to improve S/N for BPass determination:
## 
default('gaincal')
gaincal(vis='l2.ms',caltable='l2.1mmUSB.gcal0',
	field='0',spw='27,31:9~118',
	gaintype='GSPLINE',calmode='p',splinetime=10000.,
	refant='1',phasewrap=260,preavg=0.)

## Derive bandpass calibration for 1mm 
##
default('bandpass')
bandpass(vis='l2.ms',caltable='l2.1mmcont.bpoly',
	 field='0',spw='27,31',
	 bandtype='BPOLY',degamp=10,degphase=20,visnorm=False,solnorm=False,
	 maskcenter=2,maskedge=0,refant='1',
	 gaintable='l2.1mmUSB.gcal0')
default('bandpass')
bandpass(vis='l2.ms',caltable='l2.1mmcont.bpoly',
	 field='0',spw='26,30',
	 bandtype='BPOLY',degamp=10,degphase=20,visnorm=False,solnorm=False,
	 maskcenter=2,maskedge=0,refant='1',gaintable='l2.1mmUSB.gcal0',append=True)
default('bandpass')
bandpass(vis='l2.ms',caltable='l2.1mmUSB.bpoly',
	 field='0',spw='23',
	 bandtype='BPOLY',degamp=10,degphase=20,visnorm=False,solnorm=False,
	 maskcenter=2,maskedge=0,refant='1',
	 gaintable='l2.1mmUSB.gcal0')

## Determine phase solutions for 1mm LSB & USB 
##
print '--Phase solutions--'
default('gaincal')
gaincal(vis='l2.ms',caltable='l2.1mmcont.gcal',
	field='0,1,2',spw='26,27,30,31',
	gaintype='GSPLINE',calmode='p',splinetime=10000.,refant='1',
	phasewrap=260,
	gaintable='l2.1mmcont.bpoly',preavg=0.)

##  Apply all solutions derived so far, determine
##  calibrators' flux densities using a solve for T and
##  fluxscale
print '--Solve for solutions, fluxscale--'
default('gaincal')
gaincal(vis='l2.ms',caltable='l2.1mmcont.temp',
	field='0,1,2',spw='26,27,30,31',
	solint='1200s',refant='1',gaintype='T',
	gaintable=['l2.1mmcont.gcal','l2.1mmcont.bpoly'])
#
default('fluxscale')
fluxscale(vis='l2.ms',caltable='l2.1mmcont.temp',fluxtable='l2.1mmcont.flux',
	  reference='3C273*',transfer='MWC349*,2013+370*')
calphase1mmtime=time.time()
# MWC349:   0.8x (AIPS++ 0.75)
# 2013+370: 1.3  (AIPS++: 1.3)

## Record flux values from logger window.  Manually insert
## fluxes with imgr.setjy:
print '--Setjy 1mm --'
setjy(vis='l2.ms',field='1',spw='27',scalebychan=False,fluxdensity=[1.75,0.,0.,0.])
setjy(vis='l2.ms',field='1',spw='31',scalebychan=False,fluxdensity=[1.75,0.,0.,0.])
setjy(vis='l2.ms',field='2',spw='27',scalebychan=False,fluxdensity=[1.8,0.,0.,0.])
setjy(vis='l2.ms',field='2',spw='31',scalebychan=False,fluxdensity=[1.8,0.,0.,0.])
setjy(vis='l2.ms',field='1',spw='26',scalebychan=False,fluxdensity=[1.75,0.,0.,0.])
setjy(vis='l2.ms',field='1',spw='30',scalebychan=False,fluxdensity=[1.75,0.,0.,0.])
setjy(vis='l2.ms',field='2',spw='26',scalebychan=False,fluxdensity=[1.8,0.,0.,0.])
setjy(vis='l2.ms',field='2',spw='30',scalebychan=False,fluxdensity=[1.8,0.,0.,0.])
setjy1mmtime=time.time()


## Amplitude calibration of 1mm LSB:
default('gaincal')
gaincal(vis='l2.ms',caltable='l2.1mmcont.amp.gcal',
	field='0,1,2',spw='26,27,30,31',
	gaintype='GSPLINE',calmode='a',splinetime=10000.,refant='1',
	phasewrap=260,
	preavg=0.,
	gaintable=['l2.1mmcont.gcal','l2.1mmcont.bpoly'])
calamp1mmtime=time.time()

## Correct the target source 1mm LSB & USB continuum data:
##
##  Note that edge channels in won't be calibrated because
##   BPOLY solution is not defined for them
default('applycal')
applycal(vis='l2.ms',
	 field='3',spw='26,27,30,31',
	 gaintable=['l2.1mmcont.gcal','l2.1mmcont.amp.gcal','l2.1mmcont.bpoly'])
default('applycal')
applycal(vis='l2.ms',
	 field='3',spw='23',
	 gaintable=['l2.1mmcont.gcal','l2.1mmcont.amp.gcal','l2.1mmcont.bpoly'],
	 spwmap=[[-1],[-1],[26]])
correct1mmtime=time.time()

## Split out calibrated target source 1 mm continuum data:
default('split')
split(vis='l2.ms',outputvis='l2.1mma.split.ms',
      field='3',spw='26:8~117,27:8~117,30:8~117,31:8~117',
      datacolumn='corrected')
default('split')
splitsrc1mmtime=time.time()

print '--Image 1mm continuum--'
## Get a first image the target source in 1 mm continuum emission:
default('clean')
clean(vis='l2.1mma.split.ms',imagename='l2.1mm',
      psfmode='clark',imagermode='',niter=100,gain=0.1,nchan=1,start=0,width=1,
      spw='0~3',field='0',stokes='I',
      weighting='natural',interpolation='nearest',
      cell=[0.2,0.2],imsize=[128,128],mode='mfs')
image1mmtime=time.time()

print '--Resplit data--'
default('split')
split(vis='l2.ms',outputvis='l2.c34s.split.ms',
      field='3',spw='3:0~511',datacolumn='corrected')
default('split')
split(vis='l2.ms',outputvis='l2.ch3oh.split.ms',
      field='3',spw='11:0~511',datacolumn='corrected')
default('split')
split(vis='l2.ms',outputvis='l2.3mmcont.split.ms',
      field='3',
      spw='6:9~108,7:9~108,14:9~108,15:9~108,18:9~108,19:9~108',
      width=[100,100,100,100,100,100],
      datacolumn='corrected')
default('split')
split(vis='l2.ms',outputvis='l2.1mmcont.split.ms',
      field='3',
      spw='26:9~118,27:9~118,30:9~118,31:9~118',
      width=[110,110,110,110],
      datacolumn='corrected')
default('split')
split(vis='l2.ms',outputvis='l2.1mmc34s5_4.split.ms',
      field='3',spw='23:0~511',datacolumn='corrected')
splitsrc1mmtime=time.time()

### COMBINE DATA SETS

print '--Concatenate the data sets--'
os.system('cp -r l1.3mmcont.split.ms l02d.3mmcont.split.ms')
os.system('cp -r l1.c34s.split.ms l02d.c34s.split.ms')
os.system('cp -r l1.ch3oh.split.ms l02d.ch3oh.split.ms')
os.system('cp -r l1.1mmcont.split.ms l02d.1mmcont.split.ms')
default('concat')
concat(concatvis='l02d.3mmcont.split.ms',vis='l2.3mmcont.split.ms',
       freqtol='10MHz',dirtol='0.1arcsec')
concat(concatvis='l02d.1mmcont.split.ms',vis='l2.1mmcont.split.ms',
       freqtol='10MHz',dirtol='0.1arcsec')
concat(concatvis='l02d.c34s.split.ms',vis='l2.c34s.split.ms',
       freqtol='10MHz',dirtol='0.1arcsec')
concat(concatvis='l02d.ch3oh.split.ms',vis='l2.ch3oh.split.ms',
       freqtol='10MHz',dirtol='0.1arcsec')

#image target source in 3mm continuum emission
default('clean')
clean(vis='l02d.3mmcont.split.ms',imagename='l02d.3mmcont',
      psfmode='clark',imagermode='',niter=1000,gain=0.1,nchan=1,start=0,width=1,
      spw='0~5',field='0',stokes='I',
      weighting='natural',interpolation='nearest',
      cell=[0.6,0.6],imsize=[128,128],mode='mfs',threshold=0.7)

#image target source in 1mm continuum emission
default('clean')
clean(vis='l02d.1mmcont.split.ms',imagename='l02d.1mmcont',
      psfmode='clark',imagermode='',niter=1000,gain=0.1,nchan=1,start=0,width=1,
      spw='0~3',field='0',stokes='I',
      weighting='natural',interpolation='nearest',
      cell=[0.2,0.2],imsize=[128,128],mode='mfs',threshold=4.)

#image target source in 3mm c34s (2-1) line
default('clean')
clean(vis='l02d.c34s.split.ms',imagename='l02d.c34s',
      psfmode='clark',imagermode='',niter=1000,gain=0.1,nchan=160,start=20,width=3,
      spw='0',field='0',stokes='I',
      weighting='natural',interpolation='nearest',
      cell=[0.6,0.6],imsize=[128,128],mode='channel',threshold=25.)

#image target source in 3mm ch3oh 
default('clean')
clean(vis='l02d.ch3oh.split.ms',imagename='l02d.ch3oh',
      psfmode='clark',imagermode='',niter=1000,gain=0.1,nchan=160,start=20,width=3,
      spw='0',field='0',stokes='I',
      weighting='natural',interpolation='nearest',
      cell=[0.6,0.6],imsize=[128,128],mode='channel',threshold=18.)

endProc=time.clock()
endTime=time.time()

# Regression

ms.open('l02d.1mmcont.split.ms')
thistest_1mm=max(ms.range(['amplitude']).get('amplitude'))
ms.close()
ms.open('l02d.3mmcont.split.ms')
thistest_3mm=max(ms.range(['amplitude']).get('amplitude'))
ms.close()
ms.open('l02d.ch3oh.split.ms')
thistest_ch3oh=max(ms.range(['amplitude']).get('amplitude'))
ms.close()
ms.open('l02d.c34s.split.ms')
thistest_c34s=max(ms.range(['amplitude']).get('amplitude'))
ms.close()
ia.open('l02d.3mmcont.image')
statistics=ia.statistics(list=True, verbose=True)
cont3mmmax=statistics['max'][0]
cont3mmrms=statistics['rms'][0]
ia.close()
ia.open('l02d.1mmcont.image')
statistics=ia.statistics(list=True, verbose=True)
cont1mmmax=statistics['max'][0]
cont1mmrms=statistics['rms'][0]
ia.close()
ia.open('l02d.c34s.image')
statistics=ia.statistics(list=True, verbose=True)
c34smax=statistics['max'][0]
c34srms=statistics['rms'][0]
ia.close()
ia.open('l02d.ch3oh.image')
statistics=ia.statistics(list=True, verbose=True)
ch3ohmax=statistics['max'][0]
ch3ohrms=statistics['rms'][0]
ia.close()

# new test values following fix to data weights
#  fed to BPOLY solution (2012/04/18 gmoellen)
src3mm=0.4210
src1mm=2.4906
srcch3oh=405.2
srcc34s=309.9
im3mm=0.0238
im1mm=0.2170
imch3oh=0.403
imc34s=0.168


diff_3mm=abs((src3mm-thistest_3mm)/src3mm)
diff_1mm=abs((src1mm-thistest_1mm)/src1mm)
diff_ch3oh=abs((srcch3oh-thistest_ch3oh)/srcch3oh)
diff_c34s=abs((srcc34s-thistest_c34s)/srcc34s)
diff_im3mm=abs((im3mm-cont3mmmax)/im3mm)
diff_im1mm=abs((im1mm-cont1mmmax)/im1mm)
diff_imch3oh=abs((imch3oh-ch3ohmax)/imch3oh)
diff_imc34s=abs((imc34s-c34smax)/imc34s)

import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='l02d.'+datestring+'.log'
logfile=open(outfile,'w')

print >>logfile,'********** Regression ***********'
print >>logfile,'*                               *'
if (diff_3mm<0.05): print >>logfile,'* Passed 3mm max amplitude test: '
print >>logfile,'--3mm max amp '+str(thistest_3mm)+'  ('+str(src3mm)+', '+str(100*diff_3mm)+'%)'
if (diff_1mm<0.05): print >>logfile,'* Passed 1mm max amplitude test '
print >>logfile,'--1mm max amp '+str(thistest_1mm)+'  ('+str(src1mm)+', '+str(100*diff_1mm)+'%)'
if (diff_ch3oh<0.05): print >>logfile,'* Passed CH3OH max amplitude test '
print >>logfile,'--CH3OH max amp '+str(thistest_ch3oh)+'  ('+str(srcch3oh)+', '+str(100*diff_ch3oh)+'%)'
if (diff_c34s<0.05): print >>logfile,'* Passed C34S max amplitude test '
print >>logfile,'--C34S max amp '+str(thistest_c34s)+'  ('+str(srcc34s)+', '+str(100*diff_c34s)+'%)'
if (diff_im3mm<0.05): print >>logfile,'* Passed image (3mm) max test'
print >>logfile,'--Image max (3mm) '+str(cont3mmmax)+'  ('+str(im3mm)+', '+str(100*diff_im3mm)+'%)'
if (diff_im1mm<0.05): print >>logfile,'* Passed image (1mm) max test'
print >>logfile,'--Image max (1mm) '+str(cont1mmmax)+'  ('+str(im1mm)+', '+str(100*diff_im1mm)+'%)'
if (diff_imch3oh<0.05): print >>logfile,'* Passed image (ch3oh) max test'
print >>logfile,'--Image max (CH3OH) '+str(ch3ohmax)+'  ('+str(imch3oh)+', '+str(100*diff_imch3oh)+'%)'
if (diff_imc34s<0.05): print >>logfile,'* Passed image (c34s) max test'
print >>logfile,'--Image max (C34S) '+str(c34smax)+'  ('+str(imc34s)+', '+str(100*diff_imc34s)+'%)'


if ((diff_3mm<0.05) & (diff_1mm<0.05) & (diff_ch3oh<0.05) & (diff_ch3oh<0.05) & (diff_im1mm<0.05) & (diff_im3mm<0.05) & (diff_imch3oh < 0.05) & (diff_imc34s<0.05)):
	regstate=True
        print >>logfile,'---'
        print >>logfile,'Passed Regression test for L02D'
        print >>logfile,'---'
	print ''
	print 'Regression PASSED'
	print ''
else:
        regstate=False
        print >>logfile,'----FAILED Regression test for L02D'
	print ''
	print 'Regression FAILED'
	print ''

print >>logfile,'*********************************'

print >>logfile,''
print >>logfile,''
print >>logfile,'********* Benchmarking *****************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
print >>logfile,'Processing rate MB/s  was: '+str(278./(endTime - startTime))
#print >>logfile,'* Breakdown:                           *'
#print >>logfile,'*   copy         time was: '+str(copytime-startTime)
#print >>logfile,'*   flag         time was: '+str(flagtime-copytime)
#print >>logfile,'*   cal ph 3mm   time was: '+str(calphase3mmtime-flagtime)
#print >>logfile,'*   setjy 3mm    time was: '+str(setjy3mmtime-calphase3mmtime)
#print >>logfile,'*   cal amp 3mm  time was: '+str(calamp3mmtime-setjy3mmtime)
#print >>logfile,'*   correct 3mm  time was: '+str(correct3mmtime-calamp3mmtime)
#print >>logfile,'*   split cal    time was: '+str(splitcaltime-correct3mmtime)
#print >>logfile,'*   split src    time was: '+str(splitsrctime-splitcaltime)
#print >>logfile,'*   image src    time was: '+str(image3mmtime-splitsrctime)
#print >>logfile,'*   cal ph 1mm   time was: '+str(calphase1mmtime-image3mmtime)
#print >>logfile,'*   setjy 1mm    time was: '+str(setjy1mmtime-calphase1mmtime)
#print >>logfile,'*   cal amp 1mm  time was: '+str(calamp1mmtime-setjy1mmtime)
#print >>logfile,'*   correct 1mm  time was: '+str(correct1mmtime-calamp1mmtime)
#print >>logfile,'*   splitsrc 1mm time was: '+str(splitsrc1mmtime-correct1mmtime)
#print >>logfile,'*   image 1mm    time was: '+str(image1mmtime-splitsrc1mmtime)
#print >>logfile,'*   HCO cal      time was: '+str(hcocaltime-image1mmtime)
#print >>logfile,'*   image HCO    time was: '+str(imagehcotime-hcocaltime)
#print >>logfile,'*   CO cal       time was: '+str(cocaltime-imagehcotime)
#print >>logfile,'*   image CO     time was: '+str(imagecotime-cocaltime)

logfile.close()
