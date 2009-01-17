###############################################
#                                             #
# Regression/Benchmarking Script for NGC 4826 #
#                                             #
###############################################
import time
import os

#Data must be pre-filled
#n4826_16apr98.ms, n4826_22apr98.ms

#N4826 - BIMA SONG Data
#16apr98
#	source=ngc4826
#	phasecal=1310+323
#	fluxcal=3c273, Flux = 23 Jy on 16apr98
#	passcal= none - apparently, this is considered optional... odd
#22apr98
#	source=ngc4826
#	phasecal=1230+123
#	fluxcal=3c273, Flux = 23 Jy on 22apr98
#	passcal= none
#
## miriad: source Vlsr = 408; delta is 20 km/s 
#
#NOTE: This data has been filled into MIRIAD, line-length correction 
#	done, and then exported as separate files for each source.
#	3c273 was not line length corrected since it was observed
#	for such a short amount of time that it did not need it.  
#
###########################################################
## Fill Data:
##

 # miriad files copied from /home/cluster3/jmcmulli/ALMATST2/Data/N4826/
 #	98apr16/ has 1310+323.ll/  3c273/  ngc4826.ll/
 #	98apr22/ has 1230+123.ll/  3c273/  ngc4826.ll/

###########################################################
##          Mosaic field spacing looks like:
##
##          F3 (fieldid 4)         F2 (fieldid 3)
##
##   F4 (fieldid 5)      F0 (fieldid 1)        F1 (fieldid 2)
##
##          F5 (fieldid 6)         F6 (fieldid 7)
##
## 4x64 channels = 256 channels 
##
## Primary Beam should be about 1.6' FWHM (7m dishes, 2.7mm wavelength)
## Resolution should be about 5-8"
#############################################################

pathname=os.environ.get('CASAPATH').split()[0]
datapath=pathname+'/data/regression/ATST2/NGC4826/'

os.system('rm -rf n4826_22apr.* n4826_16apr.* n4826_tboth.ms')
os.system('rm -rf srca.split.ms src.split.ms')
os.system('rm -rf tmosaic* gcal*.split.* tgcal* n4826_t*')

startTime=time.time()
startProc=time.clock()

print '--Copy/reinitialize--'
#22 APR Calibration
copystring22apr='cp -r '+datapath+'n4826_22apr98.ms n4826_22apr.ms'
os.system(copystring22apr)
###reset the data as pre calibrated state
clearcal(vis='n4826_22apr.ms')
## Set the flux density of 3C273 to 23 Jy
copytime=time.time()

####set flux scale fo calibrater
print '--setjy - 22apr98--'
default('setjy')
setjy(vis='n4826_22apr.ms',field='0',fluxdensity=[23.0,0.,0.,0.])
setjy1time=time.time()
## Flag bad data non-interactively
print '--flag data - 22apr97--'
default('flagdata')
#flagdata(vis="n4826_22apr.ms",
#	 antennaid=-1,baseline=[-1],chans=[-1],
#	 clipfunction="ABS",clipcorr="YY",
#	 clipminmax=[0.0, 80.0],
#	 fieldid=-1,field="",spwid=-1,timerange="",unflag=False)
flagdata(vis="n4826_22apr.ms", mode='manualflag',
                clipexpr='ABS YY',
                clipminmax=[0.0,80.0],
                clipoutside=True)
#setclip=["ABS YY",[0.0, 80.0],True])
flag1time=time.time()
## Derive gain calibration solutions, try VLA-like calibration:
print '--gaincal - 22par98--'
default('gaincal')
gaincal(vis='n4826_22apr.ms',caltable='n4826_22apr.gcal',
	field='0,1',spw='0', gaintype='G',
	minsnr=2.0,
	refant='ANT5',gaincurve=False,opacity=0.0,solint='inf',combine='')
gaincal1time=time.time()
## Transfer the flux density scale:
print '--fluxscale - 22apr98--'
default('fluxscale')
fluxscale(vis='n4826_22apr.ms',caltable='n4826_22apr.gcal',
	  fluxtable='n4826_22apr.fcal',
	  reference='3C273-F0',transfer=['1230+123-F0']); 
fluxscale1time=time.time()
#
## Correct the calibrater/target source data:
## Use spwmap to apply gain solutions derived from spwid1 to all other spwids... 
##
print '--correct 22apr98--'
default('applycal')
applycal(vis='n4826_22apr.ms',
	field='1',spw='0',
	gaincurve=False,opacity=0.0,gaintable='n4826_22apr.fcal')
applycal(vis='n4826_22apr.ms',
	field='2~8',spw='1~4',
	gaincurve=False,opacity=0.0,gaintable='n4826_22apr.fcal',spwmap=[0])
correct1time=time.time()

## Split out calibrated target source  and calibrater data:
print '--split - 22apr98--'
default('split')
split(vis='n4826_22apr.ms',outputvis='gcal.split.ms',
#      field=1,spw=0,nchan=1,start=0,step=1,datacolumn='corrected')
	field='1',spw='0:0~1',datacolumn='corrected')
default('split')
split(vis='n4826_22apr.ms',outputvis='src.split.ms',
#      field=[2,3,4,5,6,7,8],spw=[1,2,3,4],nchan=64,start=0,step=1,
	field='2~8',spw='1~4:0~63',
      datacolumn='corrected')
split1time=time.time()

## Image the calibrater data:
##
print '--image cal - 22apr98--'
default('clean')
clean(vis='gcal.split.ms',imagename='tgcal',
      cell=[1.,1.],imsize=[256,256],
      field='0',spw='0',threshold=10.,
      psfmode='clark',niter=100,stokes='I')
imagecal1time=time.time()
## Image the target source mosaic:
##
print '--image src - 22apr98--'
default('clean')
clean(vis='src.split.ms',imagename='tmosaicb',
       nchan=30,start=47,width=4,
#       spw=range(0,3),field=range(7),
       spw='0,1,2',field='0,1,2,3,4,5,6',
       cell=[1.,1.],imsize=[256,256],
       stokes='I',mode='channel',
       psfmode='clark',imagermode='mosaic', niter=300,scaletype='SAULT', pbcor=False, minpb=0.01,cyclefactor=0.1)
imagesrc1time=time.time()
## Write image to a fits file:
##
print '--write fits image--'
default('exportfits')
exportfits(imagename='tmosaicb.image',fitsimage='tmosaicb.fits')
writefits1time=time.time()


##################
#16 APR Calibration
###########################################################
## Concatenate the separate sources
##
print '--copy/initialize - 16apr98 --'
copystring16apr='cp -r '+datapath+'n4826_16apr98.ms n4826_16apr.ms'
os.system(copystring16apr)
clearcal(vis='n4826_16apr.ms')
copy2time=time.time()
## Set the flux density of 3C273 to 23 Jy
print '--setjy - 16apr98--'
default('setjy')
setjy(vis='n4826_16apr.ms',field='0',fluxdensity=[23.0,0.,0.,0.])
setjy2time=time.time()
## Flag end channels
print '--flagdata - 16apr98 --'
default('flagdata')
#flagdata(vis='n4826_16apr.ms',chans=[0,1,62,63],spwid=[2,3,4,5],fieldid=-1)
flagdata(vis='n4826_16apr.ms',spw='2~5:0;1;62;63', mode='manualflag')
flagdata2time=time.time()
## Derive gain calibration solutions, try VLA-like calibration:
print '--gaincal - 16apr98 --'
default('gaincal')
gaincal(vis='n4826_16apr.ms',caltable='n4826_16apr.gcal',
	field='0,1',spw='0,1', gaintype='G',
	minsnr=2.0,
	refant='ANT5',gaincurve=False,opacity=0.0,solint='inf',combine='')
gaincal2time=time.time()
 #     Found 14 good G Jones solutions.
## Transfer the flux density scale:
print '--fluxscale - 16apr98 --'
default('fluxscale')
fluxscale(vis='n4826_16apr.ms',caltable='n4826_16apr.gcal',
	  fluxtable='n4826_16apr.fcal',
	  reference='3C273-F0',transfer=['1310+323-F0'],refspwmap=[0,0]);
fluxscale2time=time.time()
#  Found reference field(s): 3C273-F0
#  Found transfer field(s):  1310+323-F0
# Spw=2 will be referenced to spw=1
# Flux density for 1310+323-F0 in SpW=2 (ref SpW=1) is: 
	  #1.47184 +/- 0.00787043 (SNR = 187.009)
## Correct the calibrater/target source data:
## Use new parm spwmap to apply gain solutions derived from spwid1
## to all other spwids... 
print '--correct - 16apr98 --'
default('applycal')
applycal(vis='n4826_16apr.ms',
	field='1',spw='1',
        gaincurve=False,opacity=0.0,gaintable='n4826_16apr.fcal')
applycal(vis='n4826_16apr.ms',
	field='2~8',spw='2~5',
        gaincurve=False,opacity=0.0,gaintable='n4826_16apr.fcal',spwmap=[1])
correct2time=time.time()
## Split out calibrated target source  and calibrater data:
print '--split - 16apr98 --'
default('split')
split(vis='n4826_16apr.ms',outputvis='gcala.split.ms',
#      field=1,spw=1,nchan=1,start=0,step=1,datacolumn='corrected')
	field='1',spw='1:0~1',datacolumn='corrected')
default('split')
split(vis='n4826_16apr.ms',outputvis='srca.split.ms',
#      field=[2,3,4,5,6,7,8],spw=[2,3,4,5],nchan=64,start=0,step=1,
	field='2~8',spw='2~5:0~63',
      datacolumn='corrected')
split2time=time.time()

# Extra flagging
#flagdata(vis="srca.split.ms",
#	 antennaid=[5],baseline=[-1],chans=[-1],
#	 clipfunction="ABS",clipcorr="I",clipminmax=[0.0, 0.0],
#	 fieldid=[-1],field="",spwid=-1,
#	 timerange=['16-APR-1998/09:42:39.0', '16-APR-1998/10:24:46.0'],
#	 unflag=False)
default('flagdata')
flagdata(vis="srca.split.ms", mode='manualflag',
	 antenna='5',
         clipexpr="ABS I",
         clipminmax=[0.0,0.0],
         clipoutside=True,
	 timerange='1998/04/16/09:42:39.0~1998/04/16/10:24:46.0',
	 unflag=False)
#setclip=["ABS I",[0.0,0.0],True],

## Image the calibrater data:
print '--image cal - 16apr98 --'
default('clean')
clean(vis='gcala.split.ms',imagename='tgcala',
      cell=[1.,1.],imsize=[256,256],
      field='0',spw='0',threshold=10.,
      psfmode='clark',niter=100,stokes='I')
imagecal2time=time.time()
#############################################################
## Image the target source mosaic:
##
print '--image src - 16apr98 --'
default('clean')
### mosaic data ...Sault weighting implies a noise unform image
### hence flux scale across image needed to be corrected to get right value
clean(vis='srca.split.ms',imagename='tmosaica',
       nchan=30,start=47,width=4,
       spw='0~2',field='0~6',
       cell=[1.,1.],imsize=[256,256],
       stokes='I',mode='channel',
       psfmode='clark',niter=300,imagermode='mosaic',scaletype='SAULT',cyclefactor=0.1)
###################################################
## Write image to a fits file:
##
exportfits('tmosaica.image','tmosaica.fits')

#Combine and Image
print '-- Concatenate --'
os.system('cp -r srca.split.ms n4826_tboth.ms');
ms.open(thems='n4826_tboth.ms',nomodify=False);
ms.concatenate(msfile='src.split.ms',freqtol='50MHz')
ms.close()


#miriad:source velocity is 408; delta is 20 km/s; 24 maps
default('clean')
clean(vis='n4826_tboth.ms',imagename='tmosaic',
	 nchan=30,start=46,width=4,
	 spw='0~2',field='0~6',
	 cell=[1.,1.],imsize=[256,256],
	 stokes='I',mode='channel',
	 psfmode='clark',niter=500,imagermode='mosaic',scaletype='SAULT',
	 cyclefactor=0.1)

ia.open(infile='tmosaic.image');
ia.moments(outfile='n4826_tmom0.im',
	   moments=0,axis=3,includepix=[0.070,1000.0],
	   mask='indexin(3,[3:24])') 
ia.moments(outfile='n4826_tmom1.im',
	   moments=1,axis=3,includepix=[0.007,1000.0],
	   mask='indexin(3,[3:24])') 
ia.close()

###################################################
## Write image to a fits file:
##
exportfits(imagename='tmosaic.image',fitsimage='tmosaic.fits')

endProc=time.clock()
endTime=time.time()

# Regression
# 22 APR
ms.open('gcal.split.ms')
thistest_cal_22apr=pl.mean(ms.range(["amplitude"]).get("amplitude"))
ms.close()
ms.open('src.split.ms')
thistest_src_22apr=pl.mean(ms.range(["amplitude"]).get("amplitude"))
ms.close
ia.open('tgcal.image')
statistics=ia.statistics()
im_calmax22=statistics['max'][0]
ia.close()
ia.open('tmosaicb.image')
statistics=ia.statistics()
im_srcmax22=statistics['max'][0]

# 16 APR
ms.open('gcala.split.ms')
thistest_cal_16apr=pl.mean(ms.range(["amplitude"]).get("amplitude"))
ms.close()
ms.open('srca.split.ms')
thistest_src_16apr=pl.mean(ms.range(["amplitude"]).get("amplitude"))
ms.close
ia.open('tgcala.image')
statistics=ia.statistics()
ia.close()
im_calmax16=statistics['max'][0]
ia.open('tmosaica.image')
statistics=ia.statistics()
ia.close()
im_srcmax16=statistics['max'][0]

# COMBINED
ms.open('n4826_tboth.ms')
thistest_src=pl.mean(ms.range(["amplitude"]).get("amplitude"))
ms.close()
ia.open('n4826_tmom0.im')
statistics=ia.statistics()
ia.close()
thistest_immax=statistics['max'][0]
thistest_imrms=statistics['rms'][0]

calmean22=10.51
srcmean22=111.52
calmax22=3.0459
srcmax22=1.54
calmean16=4.3269
srcmean16=156.992
srcmax16=1.39
calmax16=1.43
srcmean=156.9898
immax=151.4
imrms=14.96

# old (pre-minsnr=2)
#calmean22=17.99
#srcmean22=233.11
#calmax22=2.9977
#srcmax22=1.63
#calmean16=75.292
#srcmean16=155.521
#srcmax16=1.45
#calmax16=1.43
#srcmean=233.5284
#immax=157.6
#imrms=16.18

diff_cal22apr=abs((calmean22-thistest_cal_22apr)/calmean22)
diff_src22apr=abs((srcmean22-thistest_src_22apr)/srcmean22)
diff_calmax22=abs((calmax22-im_calmax22)/calmax22)
diff_srcmax22=abs((srcmax22-im_srcmax22)/srcmax22)
diff_cal16apr=abs((calmean16-thistest_cal_16apr)/calmean16)
diff_src16apr=abs((srcmean16-thistest_src_16apr)/srcmean16)
diff_calmax16=abs((calmax16-im_calmax16)/calmax16)
diff_srcmax16=abs((srcmax16-im_srcmax16)/srcmax16)

diff_src=abs((srcmean-thistest_src)/srcmean)
diff_immax=abs((immax-thistest_immax)/immax)
diff_imrms=abs((imrms-thistest_imrms)/imrms)

import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='n4826.'+datestring+'.log'
logfile=open(outfile,'w')

print >>logfile,'********** Regression ***********'
print >>logfile,'*                               *'
if (diff_cal22apr<0.08): print >>logfile,'* Passed cal mean amp (22apr)'
print >>logfile,'--Cal mean amp (22apr) '+str(thistest_cal_22apr)+','+str(calmean22)
if (diff_src22apr<0.08): print >>logfile,'* Passed src mean amp (22apr)'
print >>logfile,'--Src mean amp (22apr) '+str(thistest_src_22apr)+','+str(srcmean22)
if (diff_calmax22<0.08): print >>logfile,'* Passed cal image max (22apr)'
print >>logfile,'--Image max (cal;22apr) '+str(im_calmax22)+','+str(calmax22)
if (diff_srcmax22<0.08): print >>logfile,'* Passed src image max (22apr)'
print >>logfile,'--Image max (src;22apr): '+str(im_srcmax22)+','+str(srcmax22)
if (diff_cal16apr<0.08): print >>logfile,'* Passed cal mean amp (16apr)'
print >>logfile,'--Cal mean amp (16apr) '+str(thistest_cal_16apr)+','+str(calmean16)
if (diff_src16apr<0.08): print >>logfile,'* Passed src mean amp (16apr)'
print >>logfile,'--Src mean amp (16apr) '+str(thistest_src_16apr)+','+str(srcmean16)
if (diff_calmax16<0.08): print >>logfile,'* Passed cal image max (16apr)'
print >>logfile,'--Image max (cal;16apr): '+str(im_calmax16)+','+str(calmax16)
if (diff_srcmax16<0.08): print >>logfile,'* Passed src image max (16apr)'
print >>logfile,'--Image max (src;16apr): '+str(im_srcmax16)+','+str(srcmax16)
if (diff_src<0.08): print >>logfile,'* Passed src mean amplitude test'
print >>logfile,'--Src mean amp '+str(thistest_src)+','+str(srcmean)
if (diff_immax<0.08): print >>logfile,'* Passed image max test'
print >>logfile,'--Image max '+str(thistest_immax)+','+str(immax)
if (diff_imrms<0.08): print >>logfile,'* Passed image rms test'
print >>logfile,'--Image rms '+str(thistest_imrms)+','+str(imrms)

if ((diff_cal22apr<0.08) & (diff_src22apr<0.08) & (diff_cal16apr<0.08) & (diff_src16apr<0.08) &(diff_src<0.08) & (diff_immax<0.08) & (diff_imrms<0.08)):
	regstate=True
	print >>logfile,'---'
	print >>logfile,'Passed Regression test for NGC 4826 Mosaic'
	print >>logfile,'---'
else:
	regstate=False
	print >>logfile,'----FAILED Regression test for NGC 4826 Mosaic'
print >>logfile,'*********************************'
print >>logfile,''
print >>logfile,'********* Benchmarking *****************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
print >>logfile,'Processing rate MB/s  was: '+str(300./(endTime - startTime))
print >>logfile,'* Breakdown:                           *'

logfile.close()
