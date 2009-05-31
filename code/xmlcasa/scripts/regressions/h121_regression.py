
###############################################
#                                             #
# Regression/Benchmarking Script for H121     #
#              5 MAY                          #
###############################################
import time
import os
# Data must be pre-filled
# l.ms, l2.ms

pathname=os.environ.get('CASAPATH').split()[0]
datapath=pathname+'/data/regression/ATST1/H121/*3mmmay*.ms'

startTime=time.time()
startProc=time.clock()

print '--Copy MS--'
# Fill - really copy over pristine MS
os.system('rm -rf *3mmmay*.ms h121*.c* h121*.3* 0224* 2200* nrao150*')
copystring='cp -r '+datapath+' .'
os.system(copystring)
clearcal(vis='05-3mmmay-1998-h121.ms')
clearcal(vis='09-3mmmay-1998-h121.ms')
msfile1='05-3mmmay-1998-h121.ms'
msfile2='09-3mmmay-1998-h121.ms'

copytime=time.time()

## Set flux density scale for the flux density calibrater MWC349
default('setjy')
setjy(vis=msfile1,spw='7',field='1',fluxdensity=[1.12,0.,0.,0.])

## Get preliminary time-dependent phase solutions to improve
## coherent average for bandpass solution and derive bandpass
## calibration for 3mm LSB
default('gaincal')
gaincal(vis=msfile1,caltable='h121.3mm.ph.gcal0',
	field='4',spw='7:8~55',
	gaintype='GSPLINE',calmode='p',splinetime=10000.,
	refant='1',phasewrap=260,gaincurve=False,opacity=0.0,preavg=120.)

print '--bandpass (3mm)--'
#derive bandpass calibration 
# solint value
default('bandpass')
bandpass(vis=msfile1,caltable='h121.3mm.bpoly',
	 field='4',spw='7:8~55',
	 bandtype='BPOLY',degamp=6,degphase=12,solnorm=False,
	 maskcenter=6,maskedge=0,refant='1',
	 gaintable='h121.3mm.ph.gcal0',gaincurve=False,opacity=0.0)

# 3mm CO Band
default('gaincal')
gaincal(vis=msfile1,caltable='2200.3mm.ph.gcal0',
	field='0',spw='3:64~191',
	gaintype='GSPLINE',calmode='p',splinetime=10000.,
	refant='1',preavg=10.,phasewrap=260,gaincurve=False,opacity=0.0)
default('bandpass')
bandpass(vis=msfile1,caltable='2200.3mm.bpoly',
	 field='0',spw='3:8~247',
	 bandtype='BPOLY',degamp=6,degphase=12,solnorm=False,
	 maskcenter=6,maskedge=0,refant='1',
	 gaintable='2200.3mm.ph.gcal0',gaincurve=False,opacity=0.0)

# Determine new and better phase solutions for 3mm LSB for all calibrators
default('gaincal')
gaincal(vis=msfile1,caltable='h121.3mm.ph.gcal',
	field='0,1,2,4',spw='7:8~55',
	gaintype='GSPLINE',calmode='p',splinetime=5000.,refant='1',
	phasewrap=260,
	gaincurve=False,opacity=0.0,gaintable='h121.3mm.bpoly',preavg=0.)

#Apply all solutions derived so far, determine calibrator's flux densities by solving for T 
#and use fluxscale
default('gaincal')
gaincal(vis=msfile1,caltable='h121.3mm.temp',
	field='0,1,2,4',spw='7:8~55',
	solint='600s',refant='1',gaintype='T',
	opacity=0.0,gaincurve=False,
	gaintable=['h121.3mm.ph.gcal','h121.3mm.bpoly'])

#fluxscale
default('fluxscale')
fluxscale(vis=msfile1,caltable='h121.3mm.temp',fluxtable='h121.3mm.flux',
	  reference='MWC349*',transfer='NRAO150*,2200+420*,0224+671*')
calphase3mmtime=time.time()
#2200+420: 2.66
#0224+671: 1.14
#NRAO 150: 3.12

#use old values
print '--Set fluxscale (setjy)--'
default('setjy')
setjy(vis=msfile1,field='0',spw='7',fluxdensity=[2.607,0.,0.,0.])
setjy(vis=msfile1,field='2',spw='7',fluxdensity=[1.107,0.,0.,0.])
setjy(vis=msfile1,field='4',spw='7',fluxdensity=[3.034,0.,0.,0.])
setjy3mmtime=time.time()

## Amplitude calibration of 3mm LSB:
##
##  phase solutions will be pre-applied as well as carried forward 
##   to the output solution table.
print '--gaincal amp (3mm)--'
default('gaincal')
gaincal(vis=msfile1,caltable='h121.3mm.amp.gcal',
	field='0,1,2,4',spw='7:8~55',
	gaintype='GSPLINE',calmode='a',splinetime=20000.,refant='1',
	phasewrap=260,gaincurve=False,opacity=0.0,
	preavg=2500.,
	gaintable=['h121.3mm.ph.gcal','h121.3mm.bpoly'])
calamp3mmtime=time.time()

## Correct the target source and all other 3mm LSB data
##
##  note that only the 60 central channels will be calibrated
##   since the BPOLY solution is only defined for these
default('applycal')
applycal(vis=msfile1,
	 spw='7',
	 gaincurve=False,opacity=0.0,
	 gaintable=['h121.3mm.ph.gcal','h121.3mm.amp.gcal','h121.3mm.bpoly'])
# Correct Target CO (1-0)
default('applycal')
applycal(vis=msfile1,
	 spw='3',
	 gaincurve=False,opacity=0.0,
	 gaintable=['h121.3mm.ph.gcal','h121.3mm.amp.gcal','2200.3mm.bpoly'])

# Split calibrated target source data
print '--split calibrater--'
default('split')
split(vis=msfile1,outputvis='h121.3mm.split.ms',
#      field=3,spw=7,nchan=48,start=8,step=1,datacolumn='corrected')
	field='3',spw='7:8~55',datacolumn='corrected')
default('split')
split(vis=msfile1,outputvis='nrao150.3mm.split.ms',
#      field=4,spw=7,nchan=48,start=8,step=1,datacolumn='corrected')
	field='4',spw='7:8~55',datacolumn='corrected')
default('split')
split(vis=msfile1,outputvis='h121.co10.split.ms',
#      field=3,spw=3,nchan=240,start=8,step=1,datacolumn='corrected')
	field='3',spw='3:8~247',datacolumn='corrected')
splitsrctime=time.time()

## Get a first image the target source in 3 mm continuum emission:
default('clean')
clean(vis='nrao150.3mm.split.ms',imagename='nrao150.3mm',
      psfmode='clark',niter=500,gain=0.1,nchan=1,start=0,width=48,
      spw=0,field='0',stokes='I',
      weighting='briggs',robust=2.,cell=[1.,1.],
      imsize=[64,64],mode='mfs')
default('clean')
clean(vis='h121.co10.split.ms',imagename='h121.co10',
      psfmode='clark',niter=1000,gain=0.1,threshold=20.,
      nchan=80,start=0,width=3,spw=0,
      field='0',stokes='I',
      weighting='briggs',robust=2.,
      cell=[0.5,0.5],imsize=[128,128],mode='channel')
image1time=time.time()

###############################################
#                                             #
# Regression/Benchmarking Script for H121     #
#            9 MAY                            #
###############################################

## Set flux density scale for the flux density calibrater MWC349
setjy(vis=msfile2,spw='7',field='4',fluxdensity=[1.12,0.,0.,0.])

## Get preliminary time-dependent phase solutions to improve
## coherent average for bandpass solution and derive bandpass
## calibration for 3mm LSB
default('gaincal')
gaincal(vis=msfile2,caltable='h121b.3mm.ph.gcal0',
	field='1',spw='7:8~55',
	gaintype='GSPLINE',calmode='p',splinetime=10000.,
	refant='1',phasewrap=260,
	gaincurve=False,opacity=0.0,preavg=120.)

print '--bandpass (3mm)--'
#derive bandpass calibration 
# solint value
default('bandpass')
bandpass(vis=msfile2,caltable='h121b.3mm.bpoly',
	 field='1',spw='7:8~55',
	 bandtype='BPOLY',degamp=6,degphase=12,solnorm=False,
	 maskcenter=6,maskedge=0,refant='1',
	 gaintable='h121.3mm.ph.gcal0',gaincurve=False,opacity=0.0)

# 3mm CO Band
default('gaincal')
gaincal(vis=msfile2,caltable='2200b.3mm.ph.gcal0',
	field='0',spw='3:64~191',
	gaintype='GSPLINE',calmode='p',splinetime=10000.,
	refant='1',preavg=10.,phasewrap=260,
	gaincurve=False,opacity=0.0)
default('bandpass')
bandpass(vis=msfile2,caltable='2200b.3mm.bpoly',
	 field='0',spw='3:8~247',
	 bandtype='BPOLY',degamp=6,degphase=12,solnorm=False,
	 maskcenter=6,maskedge=0,refant='1',
	 gaintable='2200b.3mm.ph.gcal0',gaincurve=False,opacity=0.0)

# Determine new and better phase solutions for 3mm LSB for all calibrators
default('gaincal')
gaincal(vis=msfile2,caltable='h121b.3mm.ph.gcal',
	field='0,1,3,4',spw='7:8~55',
	gaintype='GSPLINE',calmode='p',splinetime=5000.,refant='1',
	phasewrap=260,
	gaincurve=False,opacity=0.0,gaintable='h121b.3mm.bpoly',preavg=0.)

#Apply all solutions derived so far, determine calibrator's flux densities by solving for T 
#and use fluxscale
default('gaincal')
gaincal(vis=msfile2,caltable='h121b.3mm.temp',
	field='0,1,3,4',spw='7:8~55',
	solint='600s',refant='1',gaintype='T',
	opacity=0.0,gaincurve=False,
	gaintable=['h121b.3mm.ph.gcal','h121b.3mm.bpoly'])

#fluxscale
default('fluxscale')
fluxscale(vis=msfile2,caltable='h121b.3mm.temp',fluxtable='h121b.3mm.flux',
	  reference='MWC349*',transfer='3C454*,2200+420*,0224+671*')
calphase3mmtime=time.time()
#2200+420: 3.29
#0224+671: 1.53
#C454.3 : 6.127

#use old values
print '--Set fluxscale (setjy)--'
default('setjy')
setjy(vis=msfile2,field='0',spw='7',fluxdensity=[3.235,0.,0.,0.])
setjy(vis=msfile2,field='1',spw='7',fluxdensity=[1.507,0.,0.,0.])
setjy(vis=msfile2,field='3',spw='7',fluxdensity=[5.982,0.,0.,0.])
setjy3mmtime=time.time()

## Amplitude calibration of 3mm LSB:
##
##  phase solutions will be pre-applied as well as carried forward 
##   to the output solution table.
print '--gaincal amp (3mm)--'
default('gaincal')
gaincal(vis=msfile2,caltable='h121b.3mm.amp.gcal',
	field='0,1,3,4',spw='7:8~55',
	gaintype='GSPLINE',calmode='a',splinetime=20000.,refant='1',
	phasewrap=260,
	gaincurve=False,opacity=0.0,
	preavg=2500.,
	gaintable=['h121b.3mm.ph.gcal','h121b.3mm.bpoly'])
calamp3mmtime=time.time()

## Correct the target source and all other 3mm LSB data
##
##  note that only the 60 central channels will be calibrated
##   since the BPOLY solution is only defined for these
default('applycal')
applycal(vis=msfile2,
	 spw='7',
	 gaincurve=False,opacity=0.0,
	 gaintable=['h121b.3mm.ph.gcal','h121b.3mm.amp.gcal','h121b.3mm.bpoly'])
# Correct Target CO (1-0)
default('applycal')
applycal(vis=msfile2,
	 spw='3',
	 gaincurve=False,opacity=0.0,
	 gaintable=['h121b.3mm.ph.gcal','h121b.3mm.amp.gcal','2200b.3mm.bpoly'])

# Split calibrated target source data
print '--split calibrater--'
default('split')
split(vis=msfile2,outputvis='h121b.3mm.split.ms',
#      field=2,spw=7,nchan=48,start=8,step=1,datacolumn='corrected')
	field='2',spw='7:8~55',datacolumn='corrected')
default('split')
split(vis=msfile2,outputvis='0224b.3mm.split.ms',
#      field=1,spw=7,nchan=48,start=8,step=1,datacolumn='corrected')
	field='1',spw='7:8~55',datacolumn='corrected')
default('split')
split(vis=msfile2,outputvis='h121b.co10.split.ms',
#      field=2,spw=3,nchan=240,start=8,step=1,datacolumn='corrected')
	field='2',spw='3:8~247',datacolumn='corrected')	
splitsrctime=time.time()

## Get a first image the target source in 3 mm continuum emission:
default('clean')
clean(vis='0224b.3mm.split.ms',imagename='0224b.3mm',
      psfmode='clark',niter=500,gain=0.1,nchan=1,start=0,width=48,
      spw=0,field='0',stokes='I',
      weighting='briggs',robust=2.,
      cell=[1.,1.],imsize=[64,64],mode='mfs')
default('clean')
clean(vis='h121b.co10.split.ms',imagename='h121b.co10',
      psfmode='clark',niter=1000,gain=0.1,threshold=20.,
      nchan=80,start=0,width=3,
      spw=0,field='0',stokes='I',
      weighting='briggs',robust=2.,
      cell=[1.,1.],imsize=[64,64],mode='channel')
image1time=time.time()

#Concatenate datasets
print '--concatenate datasets--'
os.system('cp -r h121.3mm.split.ms h121.3mm.concat.ms')
os.system('cp -r h121.co10.split.ms h121.co10.concat.ms')
default('concat')
concat(concatvis='h121.3mm.concat.ms',vis='h121b.3mm.split.ms',
       freqtol='0.1MHz',dirtol='0.1arcsec', timesort=True)
default('concat')
concat(concatvis='h121.co10.concat.ms',vis='h121b.co10.split.ms',
       freqtol='0.1MHz',dirtol='0.1arcsec', timesort=True)
concattime=time.time()

#Image target in 3mm continuum emission
default('clean')
clean(vis='h121.3mm.concat.ms',imagename='h121all.3mm',
      psfmode='clark',niter=2000,gain=0.1,nchan=1,start=0,width=48,
      spw=0,field='0',stokes='I',
      weighting='briggs',robust=2.,
      cell=[1.,1.],imsize=[64,64],mode='mfs')
default('clean')
clean(vis='h121.co10.concat.ms',imagename='h121c.co10',
      psfmode='clark',niter=100,gain=0.1,nchan=80,start=0,width=3,
      spw=0,field='0',stokes='I',
      weighting='briggs',robust=.5,
      cell=[0.2,0.2],imsize=[256,256],mode='channel')

endProc=time.clock()
endTime=time.time()

# Regression

ms.open('h121.3mm.split.ms')
thistest_3mm=max(ms.range(['amplitude']).get('amplitude'))
ms.close()
ms.open('h121b.3mm.split.ms')
thistest_3mmb=max(ms.range(['amplitude']).get('amplitude'))
ms.close()
ia.open('h121.co10.image')
statistics=ia.statistics()
co10a=statistics['max'][0]
co10arms=statistics['rms'][0]
ia.close()
ia.open('h121b.co10.image')
statistics=ia.statistics()
co10b=statistics['max'][0]
co10brms=statistics['rms'][0]
ia.close()
ia.open('h121c.co10.image')
statistics=ia.statistics()
co10c=statistics['max'][0]
co10crms=statistics['rms'][0]
ia.close()
ia.open('h121all.3mm.image')
statistics=ia.statistics()
cont3mmmax=statistics['max'][0]
cont3mmrms=statistics['rms'][0]
ia.close()

srca=3.182
srcb=3.203
coa=0.4728
cob=0.3933
coc=0.4140
contmax=0.00751

diff_3mma=abs((srca-thistest_3mm)/srca)
diff_3mmb=abs((srcb-thistest_3mmb)/srcb)
diff_co10a=abs((coa-co10a)/coa)
diff_co10b=abs((cob-co10b)/cob)
diff_co10c=abs((coc-co10c)/coc)
diff_cont=abs((contmax-cont3mmmax)/contmax)

import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='h121.'+datestring+'.log'
logfile=open(outfile,'w')

print >>logfile,'********** Regression ***********'
print >>logfile,'*                               *'
if (diff_3mma<0.05): print >>logfile,'* Passed 3mm (5may) max amplitude test '
print >>logfile,'--3mm max (5may) amp '+str(thistest_3mm)
if (diff_3mmb<0.05): print >>logfile,'* Passed 3mm (9may) max amplitude test '
print >>logfile,'--3mm max (9may) amp '+str(thistest_3mmb)
if (diff_co10a<0.05): print >>logfile,'* Passed CO 1-0 image max (5may) test '
print >>logfile,'--CO 1-0 (5may) image max '+str(co10a)
if (diff_co10b<0.05): print >>logfile,'* Passed CO 1-0 image max (9may) test '
print >>logfile,'--CO 1-0 (9may) image max '+str(co10b)
if (diff_co10c<0.05): print >>logfile,'* Passed CO 1-0 image max (combined) test '
print >>logfile,'--CO 1-0 (combine) image max '+str(co10c)
if (diff_cont<0.05): print >>logfile,'* Passed 3mm image max test '
print >>logfile,'--3mm image max  '+str(cont3mmmax)



if ((diff_3mma<0.05) & (diff_3mmb<0.05) & (diff_co10a<0.05) & (diff_co10b<0.05) & (diff_co10c<0.05) & (diff_cont<0.05)):
	regstate=True
        print >>logfile,'---'
        print >>logfile,'Passed Regression test for H121'
        print >>logfile,'---'
else:
        regstate=False
        print >>logfile,'----FAILED Regression test for H121'
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
