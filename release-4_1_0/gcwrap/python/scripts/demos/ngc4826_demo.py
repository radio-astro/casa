##########################################################################
#                                                                        #
# Demo Script for NGC 4826 (BIMA line data)                              #
#                                                                        #
# Converted by STM 2008-05-27 (Beta Patch 2.0) new tasking/clean/cal     #
# Updated by   STM 2008-06-01                  add off-src cube rms      #
#                                                                        #
#                                                                        #
# First day of observations only                                         #
# Trial for summer school                                                #
#                                                                        #
# Script Notes:                                                          #
#    o the "default" commands are not necessary, but are included        #
#      in case we want to change from function calls to globals          #
#                                                                        #
##########################################################################
import time
import os

#N4826 - BIMA SONG Data
#16apr98
#	source=ngc4826
#	phasecal=1310+323
#	fluxcal=3c273, Flux = 23 Jy on 16apr98
#	passcal= none - apparently, this is considered optional... odd

## miriad: source Vlsr = 408; delta is 20 km/s 

#NOTE: This data has been filled into MIRIAD, line-length correction 
#	done, and then exported as separate files for each source.
#	3c273 was not line length corrected since it was observed
#	for such a short amount of time that it did not need it.  
#

#                                                                        #
##########################################################################
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
##########################################################################
#                                                                        #
# Set up some variables for control
#
pathname=os.environ.get('CASAPATH').split()[0]

#datapath=pathname+'/data/regression/ATST2/NGC4826/'
datapath='/home/ballista/casa/devel/data/regression/ATST2/NGC4826/'

# The prefix to use for all output files
prefix='ngc4826.demo'

# Clear out previous run results
os.system('rm -rf ngc4826.demo.*')

msfile = prefix + '.16apr98.ms'

#
##########################################################################
#
startTime=time.time()
startProc=time.clock()

#Data has been pre-filled
#n4826_16apr98.ms, n4826_22apr98.ms

##########################################################################
# 16 APR Calibration
##########################################################################
## Concatenate the separate sources
##
print '--Copy/initialize (16apr98)--'
copystring16apr='cp -r '+datapath+'n4826_16apr98.ms '+msfile
os.system(copystring16apr)

#
# Clear any previous calibration
#
clearcal(vis=msfile)

copy2time=time.time()

#
##########################################################################
#
# List contents of MS
#
listobs(vis=msfile)

# Should see the listing included at the end of this script
#
##########################################################################
#
# Set the flux density of 3C273 to 23 Jy
#
print '--Setjy (16apr98)--'
default('setjy')

setjy(vis=msfile,field='0',fluxdensity=[23.0,0.,0.,0.])

setjy2time=time.time()

#
##########################################################################
#
# Flag end channels
#
print '--Flagdata (16apr98)--'
default('flagdata')

flagdata(vis=msfile, spw='2~5:0;1;62;63', mode='manualflag')

flagdata2time=time.time()

#
##########################################################################
#
# Derive gain calibration solutions, try VLA-like calibration:
#
print '--Gaincal (16apr98) --'
default('gaincal')

gcaltable = prefix + '.16apr98.gcal'

gaincal(vis=msfile, caltable=gcaltable,
	field='0,1',spw='0,1', gaintype='G',
	minsnr=2.0,
	refant='ANT5',gaincurve=False,opacity=0.0,solint='inf',combine='')

gaincal2time=time.time()

# Found 14 good G Jones solutions.

#
##########################################################################
#
# Transfer the flux density scale:
#
print '--Fluxscale (16apr98)--'
default('fluxscale')

fcaltable = prefix + '.16apr98.fcal'

fluxscale(vis=msfile, caltable=gcaltable,
	  fluxtable=fcaltable,
	  reference='3C273-F0',transfer=['1310+323-F0'],refspwmap=[0,0]);

fluxscale2time=time.time()

# Found reference field(s): 3C273-F0
# Found transfer field(s):  1310+323-F0
# Spw=2 will be referenced to spw=1
# Flux density for 1310+323-F0 in SpW=2 (ref SpW=1) is: 
#    1.47184 +/- 0.00787043 (SNR = 187.009)

#
##########################################################################
#
# Correct the calibrater/target source data:
# Use new parm spwmap to apply gain solutions derived from spwid1
# to all other spwids... 
print '--Applycal (16apr98)--'
default('applycal')

applycal(vis=msfile,
	field='1', spw='1',
        gaincurve=False, opacity=0.0, gaintable=fcaltable)

applycal(vis=msfile,
	field='2~8',spw='2~5',
        gaincurve=False, opacity=0.0, gaintable=fcaltable, spwmap=[1])

correct2time=time.time()

#
##########################################################################
#
# Split out calibrated target source and calibrater data:
#
print '--Split (16apr98)--'
default('split')

calsplitms = prefix + '.16apr98.cal.split.ms'

split(vis=msfile, outputvis=calsplitms,
	field='1',spw='1:0~1',datacolumn='corrected')

default('split')

srcsplitms = prefix + '.16apr98.src.split.ms'

split(vis=msfile,outputvis=srcsplitms,
      field='2~8',spw='2~5:0~63',
      datacolumn='corrected')

split2time=time.time()

#
##########################################################################
#
# Extra flagging
#
print '--More Flagdata (16apr98)--'
default('flagdata')

flagdata(vis=srcsplitms, mode='manualflag',
	 antenna='5',
         clipexpr="ABS I",
         clipminmax=[0.0,0.0],
         clipoutside=True,
	 timerange='1998/04/16/09:42:39.0~1998/04/16/10:24:46.0',
	 unflag=False)
#setclip=["ABS I",[0.0,0.0],True],

#
##########################################################################
#
# Image the calibrater data:
#
print '--Clean (cal)--'
default('clean')

calimage = prefix + '.16apr98.cal.clean'

clean(vis=calsplitms, imagename=calimage,
      cell=[1.,1.],imsize=[256,256],
      field='0',spw='0',threshold=10.,
      psfmode='clark',niter=100,stokes='I')

calclnimage = calimage + '.image'

imagecal2time=time.time()

#
##########################################################################
#
# Image the target source mosaic:
#
print '--Clean (src)--'
default('clean')

### mosaic data ...Sault weighting implies a noise unform image
### hence flux scale across image needed to be corrected to get right value

srcimage = prefix + '.16apr98.src.clean'

clean(vis=srcsplitms,imagename=srcimage,
       nchan=30,start=47,width=4,
       spw='0~2',field='0~6',
       cell=[1.,1.],imsize=[256,256],
       stokes='I',mode='channel',
       psfmode='clark',niter=300,imagermode='mosaic',scaletype='SAULT',cyclefactor=0.1)

srcclnimage = srcimage + '.image'

#
##########################################################################
#
# Do first and second moments
#
#miriad:source velocity is 408; delta is 20 km/s; 24 maps
#
print '--ImMoments--'
default('immoments')

momfile = prefix + '.16apr98.moments'
momzeroimage = momfile + '.integrated'
momoneimage = momfile + '.weighted_coord'

immoments(imagename=srcclnimage,
	  moments=0,axis=3,includepix=[0.070,1000.0],
	  planes='4~25',outfile=momzeroimage) 

immoments(imagename=srcclnimage,
	  moments=1,axis=3,includepix=[0.007,1000.0],
	  planes='4~25',outfile=momoneimage) 

endProc=time.clock()
endTime=time.time()

#
##########################################################################
#
# Regression
# 16 APR
print '--Statistics (16apr98)--'

#
# Get MS stats
ms.open(calsplitms)
thistest_cal_16apr=pl.mean(ms.range(["amplitude"]).get("amplitude"))
ms.close()
ms.open(srcsplitms)
thistest_src_16apr=pl.mean(ms.range(["amplitude"]).get("amplitude"))
ms.close

#
# Image stats
#
calstat = imstat(calclnimage)

im_calmax16=calstat['max'][0]

srcstat = imstat(srcclnimage)

im_srcmax16=srcstat['max'][0]

offbox = '106,161,153,200'
offstat = imstat(srcclnimage,box=offbox)

im_offrms16=offstat['sigma'][0]

#
# Moment stats
#
momzerostat=imstat(momzeroimage)

thistest_immax=momzerostat['max'][0]
thistest_imrms=momzerostat['rms'][0]

#
# Report a few key stats
#
print '  NGC4826 Image Cube Max = '+str(srcstat['max'][0])
print "          At ("+str(srcstat['maxpos'][0])+","+str(srcstat['maxpos'][1])+") Channel "+str(srcstat['maxpos'][3])
print '          '+srcstat['maxposf']
print ''
print '          Off-Source Rms = '+str(offstat['sigma'][0])
print '          Signal-to-Noise ratio = '+str(im_srcmax16/im_offrms16)
#
##########################################################################
#
# Previous regression values
calmean16=4.3269
srcmean16=156.992
srcmax16=1.39
calmax16=1.43
immax=151.4
#imrms=14.96
imrms=15.51

offrms16=0.0486

# old (pre-minsnr=2)
#calmean16=75.292
#srcmean16=155.521
#srcmax16=1.45
#calmax16=1.43
#srcmean=233.5284
#immax=157.6
#imrms=16.18

diff_cal16apr=abs((calmean16-thistest_cal_16apr)/calmean16)
diff_src16apr=abs((srcmean16-thistest_src_16apr)/srcmean16)
diff_calmax16=abs((calmax16-im_calmax16)/calmax16)
diff_srcmax16=abs((srcmax16-im_srcmax16)/srcmax16)

diff_immax=abs((immax-thistest_immax)/immax)
diff_imrms=abs((imrms-thistest_imrms)/imrms)

# New test added STM 2008-06-01
diff_offrms16=abs((offrms16-im_offrms16)/offrms16)

#
# Pass/Fail Status
#
calvis_status = 'Failed'
if (diff_cal16apr<0.08): calvis_status = 'Passed'

srcvis_status = 'Failed'
if (diff_src16apr<0.08): srcvis_status = 'Passed'

calmax_status = 'Failed'
if (diff_calmax16<0.08): calmax_status = 'Passed'

srcmax_status = 'Failed'
if (diff_srcmax16<0.08): srcmax_status = 'Passed'

mom0max_status = 'Failed'
if (diff_immax<0.08): mom0max_status = 'Passed'

mom0rms_status = 'Failed'
if (diff_imrms<0.08): mom0rms_status = 'Passed'

offrms_status = 'Failed'
if (diff_offrms16<0.08): offrms_status = 'Passed'

#
##########################################################################
#
import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
#outfile='n4826.'+datestring+'.log'
outfile='out.'+prefix+'.'+datestring+'.log'
logfile=open(outfile,'w')

print ''
print '**************** NGC4826 Results *****************'
print '*                                                *'
print '--Cal visamp av (16apr98): '+str(thistest_cal_16apr)+' was '+str(calmean16)+' '+calvis_status
print '--Src visamp av (16apr98): '+str(thistest_src_16apr)+' was '+str(srcmean16)+' '+srcvis_status
print '--Cal image max (16apr98): '+str(im_calmax16)+' was '+str(calmax16)+' '+calmax_status
print '--Src image max (16apr98): '+str(im_srcmax16)+' was '+str(srcmax16)+' '+srcmax_status
print '--Off-src   rms (16apr98): '+str(im_offrms16)+' was '+str(offrms16)+' '+offrms_status
print '--Moment  0 max (16apr98): '+str(thistest_immax)+' was '+str(immax)+' '+mom0max_status
print '--Moment  0 rms (16apr98): '+str(thistest_imrms)+' was '+str(imrms)+' '+mom0rms_status
print ''

print >>logfile,'********** NGC4826 Regression ***********'
print >>logfile,'*                                       *'
print >>logfile,'* '+calvis_status+' cal mean amp (16apr)'
print >>logfile,'--Cal mean amp (16apr) '+str(thistest_cal_16apr)+','+str(calmean16)
print >>logfile,'* '+srcvis_status+' src mean amp (16apr)'
print >>logfile,'--Src mean amp (16apr) '+str(thistest_src_16apr)+','+str(srcmean16)
print >>logfile,'* '+calmax_status+' cal image max (16apr)'
print >>logfile,'--Image max (cal;16apr): '+str(im_calmax16)+','+str(calmax16)
print >>logfile,'* '+srcmax_status+' src image max (16apr)'
print >>logfile,'--Image max (src;16apr): '+str(im_srcmax16)+','+str(srcmax16)
print >>logfile,'* '+offrms_status+'   off-src rms (16apr)'
print >>logfile,'--Image rms (off;16apr): '+str(im_offrms16)+','+str(offrms16)
print >>logfile,'* '+mom0max_status+' mom 0 max test'
print >>logfile,'--Image max '+str(thistest_immax)+','+str(immax)
print >>logfile,'* '+mom0max_status+' mom 0 rms test'
print >>logfile,'--Image rms '+str(thistest_imrms)+','+str(imrms)

if ((diff_cal16apr<0.08) &(diff_src16apr<0.08) &(diff_calmax16<0.08) &(diff_srcmax16<0.08)
    &(diff_offrms16<0.08) &(diff_immax<0.08) &(diff_imrms<0.08) ):
	regstate=True
	print >>logfile,'---'
	print >>logfile,'Passed Regression test for NGC 4826 Mosaic'
	print >>logfile,'---'
	print 'Passed Regression test for NGC 4826 Mosaic'
else:
	regstate=False
	print >>logfile,'----FAILED Regression test for NGC 4826 Mosaic'
	print '----FAILED Regression test for NGC 4826 Mosaic'
print >>logfile,'*********************************'
print >>logfile,''
print >>logfile,'********* Benchmarking *****************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
print >>logfile,'Processing rate MB/s  was: '+str(300./(endTime - startTime))
print >>logfile,'* Breakdown:                           *'

logfile.close()

#############################
#
# listobs output:
#
#############################
###  Begin Task: listobs  ###
#
# MeasurementSet Name:  /home/sandrock/smyers/Testing/Patch2/N4826/ngc4826.demo.16apr98.ms
# MS Version 2
# 
#    Observer:      Project: t108c115.n48  
# Observation: BIMA
# 
#   Telescope Observation Date    Observer       Project        
#   BIMA      [                   4.39941e+09, 4.39942e+09]               t108c115.n48   
#   BIMA      [                   4.39942e+09, 4.39944e+09]               t108c115.n48   
#   BIMA      [                   4.39942e+09, 4.39944e+09]               t108c115.n48   
#   BIMA      [                   4.39942e+09, 4.39944e+09]               t108c115.n48   
#   BIMA      [                   4.39942e+09, 4.39944e+09]               t108c115.n48   
# Data records: 142155       Total integration time = 28464 seconds
#    Observed from   03:33:53   to   11:28:17
# 
#    ObservationID = 0         ArrayID = 0
#   Date        Timerange                Scan  FldId FieldName      SpwIds
#   16-Apr-1998/03:33:53.1 - 03:48:02.0     1      0 3C273-F0       [0]
# 
#    ObservationID = 1         ArrayID = 0
#   Date        Timerange                Scan  FldId FieldName      SpwIds
#   16-Apr-1998/03:56:19.6 - 04:01:34.5     1      1 1310+323-F0    [1]
#               04:36:40.1 - 04:41:55.3     2      1 1310+323-F0    [1]
#               05:17:03.4 - 05:22:18.3     3      1 1310+323-F0    [1]
#               05:57:33.5 - 06:02:49.0     4      1 1310+323-F0    [1]
#               06:38:15.7 - 06:43:31.0     5      1 1310+323-F0    [1]
#               07:19:15.0 - 07:24:30.5     6      1 1310+323-F0    [1]
#               07:59:47.3 - 08:05:02.5     7      1 1310+323-F0    [1]
#               08:40:09.0 - 08:45:24.2     8      1 1310+323-F0    [1]
#               09:20:20.9 - 09:25:36.1     9      1 1310+323-F0    [1]
#               10:00:35.0 - 10:05:50.1    10      1 1310+323-F0    [1]
#               10:40:40.7 - 10:45:55.7    11      1 1310+323-F0    [1]
#               11:20:49.7 - 11:28:17.0    12      1 1310+323-F0    [1]
# 
#    ObservationID = 2         ArrayID = 0
#   Date        Timerange                Scan  FldId FieldName      SpwIds
#   16-Apr-1998/04:04:30.6 - 04:04:30.6     1      2 NGC4826-F0     [2, 3, 4, 5]
#               04:05:32.1 - 04:05:32.1     1      3 NGC4826-F1     [2, 3, 4, 5]
#               04:06:38.4 - 04:06:38.4     1      4 NGC4826-F2     [2, 3, 4, 5]
#               04:07:44.2 - 04:07:44.2     1      5 NGC4826-F3     [2, 3, 4, 5]
#               04:08:49.2 - 04:08:49.2     1      6 NGC4826-F4     [2, 3, 4, 5]
#               04:09:56.5 - 04:09:56.5     1      7 NGC4826-F5     [2, 3, 4, 5]
#               04:11:00.1 - 04:11:00.1     1      8 NGC4826-F6     [2, 3, 4, 5]
#               04:12:06.4 - 04:12:06.4     1      2 NGC4826-F0     [2, 3, 4, 5]
#               04:13:10.4 - 04:13:10.4     1      3 NGC4826-F1     [2, 3, 4, 5]
#               04:14:16.3 - 04:14:16.3     1      4 NGC4826-F2     [2, 3, 4, 5]
#               04:15:25.4 - 04:15:25.4     1      5 NGC4826-F3     [2, 3, 4, 5]
#               04:16:27.1 - 04:16:27.1     1      6 NGC4826-F4     [2, 3, 4, 5]
#               04:17:32.1 - 04:17:32.1     1      7 NGC4826-F5     [2, 3, 4, 5]
#               04:18:35.5 - 04:18:35.5     1      8 NGC4826-F6     [2, 3, 4, 5]
#               04:20:20.1 - 04:20:20.1     1      2 NGC4826-F0     [2, 3, 4, 5]
#               04:21:28.3 - 04:21:28.3     1      3 NGC4826-F1     [2, 3, 4, 5]
#               04:22:31.0 - 04:22:31.0     1      4 NGC4826-F2     [2, 3, 4, 5]
#               04:23:36.9 - 04:23:36.9     1      5 NGC4826-F3     [2, 3, 4, 5]
#               04:24:41.9 - 04:24:41.9     1      6 NGC4826-F4     [2, 3, 4, 5]
#               04:25:48.4 - 04:25:48.4     1      7 NGC4826-F5     [2, 3, 4, 5]
#               04:26:53.1 - 04:26:53.1     1      8 NGC4826-F6     [2, 3, 4, 5]
#               04:27:59.0 - 04:27:59.0     1      2 NGC4826-F0     [2, 3, 4, 5]
#               04:29:03.9 - 04:29:03.9     1      3 NGC4826-F1     [2, 3, 4, 5]
#               04:30:08.9 - 04:30:08.9     1      4 NGC4826-F2     [2, 3, 4, 5]
#               04:31:15.8 - 04:31:15.8     1      5 NGC4826-F3     [2, 3, 4, 5]
#               04:32:23.9 - 04:32:23.9     1      6 NGC4826-F4     [2, 3, 4, 5]
#               04:33:28.9 - 04:33:28.9     1      7 NGC4826-F5     [2, 3, 4, 5]
#               04:34:32.9 - 04:34:32.9     1      8 NGC4826-F6     [2, 3, 4, 5]
#               04:44:53.5 - 04:44:53.5     2      2 NGC4826-F0     [2, 3, 4, 5]
#               04:45:58.7 - 04:45:58.7     2      3 NGC4826-F1     [2, 3, 4, 5]
#               04:47:04.9 - 04:47:04.9     2      4 NGC4826-F2     [2, 3, 4, 5]
#               04:48:06.4 - 04:48:06.4     2      5 NGC4826-F3     [2, 3, 4, 5]
#               04:49:11.0 - 04:49:11.0     2      6 NGC4826-F4     [2, 3, 4, 5]
#               04:50:18.8 - 04:50:18.8     2      7 NGC4826-F5     [2, 3, 4, 5]
#               04:51:22.5 - 04:51:22.5     2      8 NGC4826-F6     [2, 3, 4, 5]
#               04:52:28.7 - 04:52:28.7     2      2 NGC4826-F0     [2, 3, 4, 5]
#               04:53:32.7 - 04:53:32.7     2      3 NGC4826-F1     [2, 3, 4, 5]
#               04:54:37.7 - 04:54:37.7     2      4 NGC4826-F2     [2, 3, 4, 5]
#               04:55:46.8 - 04:55:46.8     2      5 NGC4826-F3     [2, 3, 4, 5]
#               04:56:48.6 - 04:56:48.6     2      6 NGC4826-F4     [2, 3, 4, 5]
#               04:57:53.6 - 04:57:53.6     2      7 NGC4826-F5     [2, 3, 4, 5]
#               04:58:58.8 - 04:58:58.8     2      8 NGC4826-F6     [2, 3, 4, 5]
#               05:00:41.5 - 05:00:41.5     2      2 NGC4826-F0     [2, 3, 4, 5]
#               05:01:51.1 - 05:01:51.1     2      3 NGC4826-F1     [2, 3, 4, 5]
#               05:02:57.2 - 05:02:57.2     2      4 NGC4826-F2     [2, 3, 4, 5]
#               05:04:02.2 - 05:04:02.2     2      5 NGC4826-F3     [2, 3, 4, 5]
#               05:05:07.5 - 05:05:07.5     2      6 NGC4826-F4     [2, 3, 4, 5]
#               05:06:12.1 - 05:06:12.1     2      7 NGC4826-F5     [2, 3, 4, 5]
#               05:07:20.3 - 05:07:20.3     2      8 NGC4826-F6     [2, 3, 4, 5]
#               05:08:25.2 - 05:08:25.2     2      2 NGC4826-F0     [2, 3, 4, 5]
#               05:09:28.3 - 05:09:28.3     2      3 NGC4826-F1     [2, 3, 4, 5]
#               05:10:34.2 - 05:10:34.2     2      4 NGC4826-F2     [2, 3, 4, 5]
#               05:11:39.5 - 05:11:39.5     2      5 NGC4826-F3     [2, 3, 4, 5]
#               05:12:47.4 - 05:12:47.4     2      6 NGC4826-F4     [2, 3, 4, 5]
#               05:13:51.4 - 05:13:51.4     2      7 NGC4826-F5     [2, 3, 4, 5]
#               05:14:53.1 - 05:14:53.1     2      8 NGC4826-F6     [2, 3, 4, 5]
#               05:25:18.0 - 05:25:18.0     3      2 NGC4826-F0     [2, 3, 4, 5]
#               05:26:22.0 - 05:26:22.0     3      3 NGC4826-F1     [2, 3, 4, 5]
#               05:27:27.0 - 05:27:27.0     3      4 NGC4826-F2     [2, 3, 4, 5]
#               05:28:34.1 - 05:28:34.1     3      5 NGC4826-F3     [2, 3, 4, 5]
#               05:29:39.1 - 05:29:39.1     3      6 NGC4826-F4     [2, 3, 4, 5]
#               05:30:47.3 - 05:30:47.3     3      7 NGC4826-F5     [2, 3, 4, 5]
#               05:31:49.0 - 05:31:49.0     3      8 NGC4826-F6     [2, 3, 4, 5]
#               05:32:54.0 - 05:32:54.0     3      2 NGC4826-F0     [2, 3, 4, 5]
#               05:33:58.0 - 05:33:58.0     3      3 NGC4826-F1     [2, 3, 4, 5]
#               05:35:03.0 - 05:35:03.0     3      4 NGC4826-F2     [2, 3, 4, 5]
#               05:36:11.1 - 05:36:11.1     3      5 NGC4826-F3     [2, 3, 4, 5]
#               05:37:12.9 - 05:37:12.9     3      6 NGC4826-F4     [2, 3, 4, 5]
#               05:38:17.2 - 05:38:17.2     3      7 NGC4826-F5     [2, 3, 4, 5]
#               05:39:19.0 - 05:39:19.0     3      8 NGC4826-F6     [2, 3, 4, 5]
#               05:41:01.7 - 05:41:01.7     3      2 NGC4826-F0     [2, 3, 4, 5]
#               05:42:09.9 - 05:42:09.9     3      3 NGC4826-F1     [2, 3, 4, 5]
#               05:43:14.8 - 05:43:14.8     3      4 NGC4826-F2     [2, 3, 4, 5]
#               05:44:20.7 - 05:44:20.7     3      5 NGC4826-F3     [2, 3, 4, 5]
#               05:45:26.0 - 05:45:26.0     3      6 NGC4826-F4     [2, 3, 4, 5]
#               05:46:28.7 - 05:46:28.7     3      7 NGC4826-F5     [2, 3, 4, 5]
#               05:47:36.9 - 05:47:36.9     3      8 NGC4826-F6     [2, 3, 4, 5]
#               05:48:41.9 - 05:48:41.9     3      2 NGC4826-F0     [2, 3, 4, 5]
#               05:49:45.8 - 05:49:45.8     3      3 NGC4826-F1     [2, 3, 4, 5]
#               05:50:51.8 - 05:50:51.8     3      4 NGC4826-F2     [2, 3, 4, 5]
#               05:51:56.7 - 05:51:56.7     3      5 NGC4826-F3     [2, 3, 4, 5]
#               05:53:05.9 - 05:53:05.9     3      6 NGC4826-F4     [2, 3, 4, 5]
#               05:54:10.8 - 05:54:10.8     3      7 NGC4826-F5     [2, 3, 4, 5]
#               05:55:15.8 - 05:55:15.8     3      8 NGC4826-F6     [2, 3, 4, 5]
#               06:05:54.2 - 06:05:54.2     4      2 NGC4826-F0     [2, 3, 4, 5]
#               06:06:59.1 - 06:06:59.1     4      3 NGC4826-F1     [2, 3, 4, 5]
#               06:08:04.1 - 06:08:04.1     4      4 NGC4826-F2     [2, 3, 4, 5]
#               06:09:09.1 - 06:09:09.1     4      5 NGC4826-F3     [2, 3, 4, 5]
#               06:10:14.1 - 06:10:14.1     4      6 NGC4826-F4     [2, 3, 4, 5]
#               06:11:20.3 - 06:11:20.3     4      7 NGC4826-F5     [2, 3, 4, 5]
#               06:12:24.9 - 06:12:24.9     4      8 NGC4826-F6     [2, 3, 4, 5]
#               06:13:30.8 - 06:13:30.8     4      2 NGC4826-F0     [2, 3, 4, 5]
#               06:14:34.8 - 06:14:34.8     4      3 NGC4826-F1     [2, 3, 4, 5]
#               06:15:41.1 - 06:15:41.1     4      4 NGC4826-F2     [2, 3, 4, 5]
#               06:16:48.9 - 06:16:48.9     4      5 NGC4826-F3     [2, 3, 4, 5]
#               06:17:53.9 - 06:17:53.9     4      6 NGC4826-F4     [2, 3, 4, 5]
#               06:18:56.9 - 06:18:56.9     4      7 NGC4826-F5     [2, 3, 4, 5]
#               06:20:00.9 - 06:20:00.9     4      8 NGC4826-F6     [2, 3, 4, 5]
#               06:21:40.5 - 06:21:40.5     4      2 NGC4826-F0     [2, 3, 4, 5]
#               06:22:48.9 - 06:22:48.9     4      3 NGC4826-F1     [2, 3, 4, 5]
#               06:23:53.9 - 06:23:53.9     4      4 NGC4826-F2     [2, 3, 4, 5]
#               06:24:59.8 - 06:24:59.8     4      5 NGC4826-F3     [2, 3, 4, 5]
#               06:26:04.7 - 06:26:04.7     4      6 NGC4826-F4     [2, 3, 4, 5]
#               06:27:08.7 - 06:27:08.7     4      7 NGC4826-F5     [2, 3, 4, 5]
#               06:28:15.9 - 06:28:15.9     4      8 NGC4826-F6     [2, 3, 4, 5]
#               06:29:20.9 - 06:29:20.9     4      2 NGC4826-F0     [2, 3, 4, 5]
#               06:30:24.9 - 06:30:24.9     4      3 NGC4826-F1     [2, 3, 4, 5]
#               06:31:30.5 - 06:31:30.5     4      4 NGC4826-F2     [2, 3, 4, 5]
#               06:32:36.7 - 06:32:36.7     4      5 NGC4826-F3     [2, 3, 4, 5]
#               06:33:43.9 - 06:33:43.9     4      6 NGC4826-F4     [2, 3, 4, 5]
#               06:34:49.6 - 06:34:49.6     4      7 NGC4826-F5     [2, 3, 4, 5]
#               06:35:53.5 - 06:35:53.5     4      8 NGC4826-F6     [2, 3, 4, 5]
#               06:46:40.6 - 06:46:40.6     5      2 NGC4826-F0     [2, 3, 4, 5]
#               06:47:44.6 - 06:47:44.6     5      3 NGC4826-F1     [2, 3, 4, 5]
#               06:48:51.8 - 06:48:51.8     5      4 NGC4826-F2     [2, 3, 4, 5]
#               06:49:57.1 - 06:49:57.1     5      5 NGC4826-F3     [2, 3, 4, 5]
#               06:51:01.7 - 06:51:01.7     5      6 NGC4826-F4     [2, 3, 4, 5]
#               06:52:09.9 - 06:52:09.9     5      7 NGC4826-F5     [2, 3, 4, 5]
#               06:53:14.5 - 06:53:14.5     5      8 NGC4826-F6     [2, 3, 4, 5]
#               06:54:20.5 - 06:54:20.5     5      2 NGC4826-F0     [2, 3, 4, 5]
#               06:55:24.7 - 06:55:24.7     5      3 NGC4826-F1     [2, 3, 4, 5]
#               06:56:29.7 - 06:56:29.7     5      4 NGC4826-F2     [2, 3, 4, 5]
#               06:57:37.5 - 06:57:37.5     5      5 NGC4826-F3     [2, 3, 4, 5]
#               06:58:42.5 - 06:58:42.5     5      6 NGC4826-F4     [2, 3, 4, 5]
#               06:59:47.5 - 06:59:47.5     5      7 NGC4826-F5     [2, 3, 4, 5]
#               07:00:53.7 - 07:00:53.7     5      8 NGC4826-F6     [2, 3, 4, 5]
#               07:02:37.5 - 07:02:37.5     5      2 NGC4826-F0     [2, 3, 4, 5]
#               07:03:46.5 - 07:03:46.5     5      3 NGC4826-F1     [2, 3, 4, 5]
#               07:04:52.1 - 07:04:52.1     5      4 NGC4826-F2     [2, 3, 4, 5]
#               07:05:58.0 - 07:05:58.0     5      5 NGC4826-F3     [2, 3, 4, 5]
#               07:07:03.0 - 07:07:03.0     5      6 NGC4826-F4     [2, 3, 4, 5]
#               07:08:07.9 - 07:08:07.9     5      7 NGC4826-F5     [2, 3, 4, 5]
#               07:09:17.1 - 07:09:17.1     5      8 NGC4826-F6     [2, 3, 4, 5]
#               07:10:22.0 - 07:10:22.0     5      2 NGC4826-F0     [2, 3, 4, 5]
#               07:11:28.9 - 07:11:28.9     5      3 NGC4826-F1     [2, 3, 4, 5]
#               07:12:33.9 - 07:12:33.9     5      4 NGC4826-F2     [2, 3, 4, 5]
#               07:13:39.8 - 07:13:39.8     5      5 NGC4826-F3     [2, 3, 4, 5]
#               07:14:48.9 - 07:14:48.9     5      6 NGC4826-F4     [2, 3, 4, 5]
#               07:15:53.9 - 07:15:53.9     5      7 NGC4826-F5     [2, 3, 4, 5]
#               07:16:59.8 - 07:16:59.8     5      8 NGC4826-F6     [2, 3, 4, 5]
#               07:27:30.5 - 07:27:30.5     6      2 NGC4826-F0     [2, 3, 4, 5]
#               07:28:36.4 - 07:28:36.4     6      3 NGC4826-F1     [2, 3, 4, 5]
#               07:29:38.2 - 07:29:38.2     6      4 NGC4826-F2     [2, 3, 4, 5]
#               07:30:44.5 - 07:30:44.5     6      5 NGC4826-F3     [2, 3, 4, 5]
#               07:31:50.4 - 07:31:50.4     6      6 NGC4826-F4     [2, 3, 4, 5]
#               07:32:58.5 - 07:32:58.5     6      7 NGC4826-F5     [2, 3, 4, 5]
#               07:34:05.0 - 07:34:05.0     6      8 NGC4826-F6     [2, 3, 4, 5]
#               07:35:11.0 - 07:35:11.0     6      2 NGC4826-F0     [2, 3, 4, 5]
#               07:36:15.0 - 07:36:15.0     6      3 NGC4826-F1     [2, 3, 4, 5]
#               07:37:20.9 - 07:37:20.9     6      4 NGC4826-F2     [2, 3, 4, 5]
#               07:38:30.0 - 07:38:30.0     6      5 NGC4826-F3     [2, 3, 4, 5]
#               07:39:31.8 - 07:39:31.8     6      6 NGC4826-F4     [2, 3, 4, 5]
#               07:40:38.1 - 07:40:38.1     6      7 NGC4826-F5     [2, 3, 4, 5]
#               07:41:43.9 - 07:41:43.9     6      8 NGC4826-F6     [2, 3, 4, 5]
#               07:43:25.1 - 07:43:25.1     6      2 NGC4826-F0     [2, 3, 4, 5]
#               07:44:32.9 - 07:44:32.9     6      3 NGC4826-F1     [2, 3, 4, 5]
#               07:45:37.9 - 07:45:37.9     6      4 NGC4826-F2     [2, 3, 4, 5]
#               07:46:42.8 - 07:46:42.8     6      5 NGC4826-F3     [2, 3, 4, 5]
#               07:47:48.7 - 07:47:48.7     6      6 NGC4826-F4     [2, 3, 4, 5]
#               07:48:53.7 - 07:48:53.7     6      7 NGC4826-F5     [2, 3, 4, 5]
#               07:50:04.2 - 07:50:04.2     6      8 NGC4826-F6     [2, 3, 4, 5]
#               07:51:09.4 - 07:51:09.4     6      2 NGC4826-F0     [2, 3, 4, 5]
#               07:52:13.7 - 07:52:13.7     6      3 NGC4826-F1     [2, 3, 4, 5]
#               07:53:18.5 - 07:53:18.5     6      4 NGC4826-F2     [2, 3, 4, 5]
#               07:54:24.6 - 07:54:24.6     6      5 NGC4826-F3     [2, 3, 4, 5]
#               07:55:32.5 - 07:55:32.5     6      6 NGC4826-F4     [2, 3, 4, 5]
#               07:56:36.5 - 07:56:36.5     6      7 NGC4826-F5     [2, 3, 4, 5]
#               07:57:42.4 - 07:57:42.4     6      8 NGC4826-F6     [2, 3, 4, 5]
#               08:07:53.9 - 08:07:53.9     7      2 NGC4826-F0     [2, 3, 4, 5]
#               08:08:59.4 - 08:08:59.4     7      3 NGC4826-F1     [2, 3, 4, 5]
#               08:10:05.7 - 08:10:05.7     7      4 NGC4826-F2     [2, 3, 4, 5]
#               08:11:12.6 - 08:11:12.6     7      5 NGC4826-F3     [2, 3, 4, 5]
#               08:12:18.5 - 08:12:18.5     7      6 NGC4826-F4     [2, 3, 4, 5]
#               08:13:25.7 - 08:13:25.7     7      7 NGC4826-F5     [2, 3, 4, 5]
#               08:14:29.4 - 08:14:29.4     7      8 NGC4826-F6     [2, 3, 4, 5]
#               08:15:35.4 - 08:15:35.4     7      2 NGC4826-F0     [2, 3, 4, 5]
#               08:16:39.4 - 08:16:39.4     7      3 NGC4826-F1     [2, 3, 4, 5]
#               08:17:45.2 - 08:17:45.2     7      4 NGC4826-F2     [2, 3, 4, 5]
#               08:18:54.7 - 08:18:54.7     7      5 NGC4826-F3     [2, 3, 4, 5]
#               08:20:00.2 - 08:20:00.2     7      6 NGC4826-F4     [2, 3, 4, 5]
#               08:21:06.2 - 08:21:06.2     7      7 NGC4826-F5     [2, 3, 4, 5]
#               08:22:10.2 - 08:22:10.2     7      8 NGC4826-F6     [2, 3, 4, 5]
#               08:23:52.9 - 08:23:52.9     7      2 NGC4826-F0     [2, 3, 4, 5]
#               08:25:00.1 - 08:25:00.1     7      3 NGC4826-F1     [2, 3, 4, 5]
#               08:26:04.1 - 08:26:04.1     7      4 NGC4826-F2     [2, 3, 4, 5]
#               08:27:11.0 - 08:27:11.0     7      5 NGC4826-F3     [2, 3, 4, 5]
#               08:28:16.9 - 08:28:16.9     7      6 NGC4826-F4     [2, 3, 4, 5]
#               08:29:20.9 - 08:29:20.9     7      7 NGC4826-F5     [2, 3, 4, 5]
#               08:30:29.0 - 08:30:29.0     7      8 NGC4826-F6     [2, 3, 4, 5]
#               08:31:30.5 - 08:31:30.5     7      2 NGC4826-F0     [2, 3, 4, 5]
#               08:32:35.1 - 08:32:35.1     7      3 NGC4826-F1     [2, 3, 4, 5]
#               08:33:39.1 - 08:33:39.1     7      4 NGC4826-F2     [2, 3, 4, 5]
#               08:34:44.7 - 08:34:44.7     7      5 NGC4826-F3     [2, 3, 4, 5]
#               08:35:52.9 - 08:35:52.9     7      6 NGC4826-F4     [2, 3, 4, 5]
#               08:36:56.9 - 08:36:56.9     7      7 NGC4826-F5     [2, 3, 4, 5]
#               08:38:00.9 - 08:38:00.9     7      8 NGC4826-F6     [2, 3, 4, 5]
#               08:48:18.5 - 08:48:18.5     8      2 NGC4826-F0     [2, 3, 4, 5]
#               08:49:23.1 - 08:49:23.1     8      3 NGC4826-F1     [2, 3, 4, 5]
#               08:50:26.2 - 08:50:26.2     8      4 NGC4826-F2     [2, 3, 4, 5]
#               08:51:32.2 - 08:51:32.2     8      5 NGC4826-F3     [2, 3, 4, 5]
#               08:52:38.0 - 08:52:38.0     8      6 NGC4826-F4     [2, 3, 4, 5]
#               08:53:44.9 - 08:53:44.9     8      7 NGC4826-F5     [2, 3, 4, 5]
#               08:54:48.9 - 08:54:48.9     8      8 NGC4826-F6     [2, 3, 4, 5]
#               08:55:53.8 - 08:55:53.8     8      2 NGC4826-F0     [2, 3, 4, 5]
#               08:56:57.8 - 08:56:57.8     8      3 NGC4826-F1     [2, 3, 4, 5]
#               08:58:02.8 - 08:58:02.8     8      4 NGC4826-F2     [2, 3, 4, 5]
#               08:59:11.9 - 08:59:11.9     8      5 NGC4826-F3     [2, 3, 4, 5]
#               09:00:16.9 - 09:00:16.9     8      6 NGC4826-F4     [2, 3, 4, 5]
#               09:01:20.9 - 09:01:20.9     8      7 NGC4826-F5     [2, 3, 4, 5]
#               09:02:24.9 - 09:02:24.9     8      8 NGC4826-F6     [2, 3, 4, 5]
#               09:04:05.7 - 09:04:05.7     8      2 NGC4826-F0     [2, 3, 4, 5]
#               09:05:13.9 - 09:05:13.9     8      3 NGC4826-F1     [2, 3, 4, 5]
#               09:06:15.6 - 09:06:15.6     8      4 NGC4826-F2     [2, 3, 4, 5]
#               09:07:22.9 - 09:07:22.9     8      5 NGC4826-F3     [2, 3, 4, 5]
#               09:08:27.8 - 09:08:27.8     8      6 NGC4826-F4     [2, 3, 4, 5]
#               09:09:31.8 - 09:09:31.8     8      7 NGC4826-F5     [2, 3, 4, 5]
#               09:10:39.0 - 09:10:39.0     8      8 NGC4826-F6     [2, 3, 4, 5]
#               09:11:40.5 - 09:11:40.5     8      2 NGC4826-F0     [2, 3, 4, 5]
#               09:12:43.8 - 09:12:43.8     8      3 NGC4826-F1     [2, 3, 4, 5]
#               09:13:48.7 - 09:13:48.7     8      4 NGC4826-F2     [2, 3, 4, 5]
#               09:14:54.7 - 09:14:54.7     8      5 NGC4826-F3     [2, 3, 4, 5]
#               09:16:02.8 - 09:16:02.8     8      6 NGC4826-F4     [2, 3, 4, 5]
#               09:17:07.8 - 09:17:07.8     8      7 NGC4826-F5     [2, 3, 4, 5]
#               09:18:12.7 - 09:18:12.7     8      8 NGC4826-F6     [2, 3, 4, 5]
#               09:28:31.3 - 09:28:31.3     9      2 NGC4826-F0     [2, 3, 4, 5]
#               09:29:35.6 - 09:29:35.6     9      3 NGC4826-F1     [2, 3, 4, 5]
#               09:30:40.6 - 09:30:40.6     9      4 NGC4826-F2     [2, 3, 4, 5]
#               09:31:46.8 - 09:31:46.8     9      5 NGC4826-F3     [2, 3, 4, 5]
#               09:32:51.8 - 09:32:51.8     9      6 NGC4826-F4     [2, 3, 4, 5]
#               09:33:56.4 - 09:33:56.4     9      7 NGC4826-F5     [2, 3, 4, 5]
#               09:35:01.7 - 09:35:01.7     9      8 NGC4826-F6     [2, 3, 4, 5]
#               09:36:06.7 - 09:36:06.7     9      2 NGC4826-F0     [2, 3, 4, 5]
#               09:37:10.6 - 09:37:10.6     9      3 NGC4826-F1     [2, 3, 4, 5]
#               09:38:15.6 - 09:38:15.6     9      4 NGC4826-F2     [2, 3, 4, 5]
#               09:39:24.7 - 09:39:24.7     9      5 NGC4826-F3     [2, 3, 4, 5]
#               09:40:30.4 - 09:40:30.4     9      6 NGC4826-F4     [2, 3, 4, 5]
#               09:41:33.4 - 09:41:33.4     9      7 NGC4826-F5     [2, 3, 4, 5]
#               09:42:38.7 - 09:42:38.7     9      8 NGC4826-F6     [2, 3, 4, 5]
#               09:44:16.3 - 09:44:16.3     9      2 NGC4826-F0     [2, 3, 4, 5]
#               09:45:24.4 - 09:45:24.4     9      3 NGC4826-F1     [2, 3, 4, 5]
#               09:46:29.4 - 09:46:29.4     9      4 NGC4826-F2     [2, 3, 4, 5]
#               09:47:35.6 - 09:47:35.6     9      5 NGC4826-F3     [2, 3, 4, 5]
#               09:48:40.6 - 09:48:40.6     9      6 NGC4826-F4     [2, 3, 4, 5]
#               09:49:45.6 - 09:49:45.6     9      7 NGC4826-F5     [2, 3, 4, 5]
#               09:50:53.7 - 09:50:53.7     9      8 NGC4826-F6     [2, 3, 4, 5]
#               09:51:55.1 - 09:51:55.1     9      2 NGC4826-F0     [2, 3, 4, 5]
#               09:53:00.4 - 09:53:00.4     9      3 NGC4826-F1     [2, 3, 4, 5]
#               09:54:06.4 - 09:54:06.4     9      4 NGC4826-F2     [2, 3, 4, 5]
#               09:55:12.2 - 09:55:12.2     9      5 NGC4826-F3     [2, 3, 4, 5]
#               09:56:20.4 - 09:56:20.4     9      6 NGC4826-F4     [2, 3, 4, 5]
#               09:57:22.2 - 09:57:22.2     9      7 NGC4826-F5     [2, 3, 4, 5]
#               09:58:27.1 - 09:58:27.1     9      8 NGC4826-F6     [2, 3, 4, 5]
#               10:08:43.5 - 10:08:43.5    10      2 NGC4826-F0     [2, 3, 4, 5]
#               10:09:44.9 - 10:09:44.9    10      3 NGC4826-F1     [2, 3, 4, 5]
#               10:10:46.4 - 10:10:46.4    10      4 NGC4826-F2     [2, 3, 4, 5]
#               10:11:51.6 - 10:11:51.6    10      5 NGC4826-F3     [2, 3, 4, 5]
#               10:12:56.6 - 10:12:56.6    10      6 NGC4826-F4     [2, 3, 4, 5]
#               10:14:04.7 - 10:14:04.7    10      7 NGC4826-F5     [2, 3, 4, 5]
#               10:15:06.2 - 10:15:06.2    10      8 NGC4826-F6     [2, 3, 4, 5]
#               10:16:12.5 - 10:16:12.5    10      2 NGC4826-F0     [2, 3, 4, 5]
#               10:17:15.8 - 10:17:15.8    10      3 NGC4826-F1     [2, 3, 4, 5]
#               10:18:21.7 - 10:18:21.7    10      4 NGC4826-F2     [2, 3, 4, 5]
#               10:19:29.5 - 10:19:29.5    10      5 NGC4826-F3     [2, 3, 4, 5]
#               10:20:34.5 - 10:20:34.5    10      6 NGC4826-F4     [2, 3, 4, 5]
#               10:21:40.5 - 10:21:40.5    10      7 NGC4826-F5     [2, 3, 4, 5]
#               10:22:45.4 - 10:22:45.4    10      8 NGC4826-F6     [2, 3, 4, 5]
#               10:24:27.1 - 10:24:27.1    10      2 NGC4826-F0     [2, 3, 4, 5]
#               10:25:33.7 - 10:25:33.7    10      3 NGC4826-F1     [2, 3, 4, 5]
#               10:26:35.1 - 10:26:35.1    10      4 NGC4826-F2     [2, 3, 4, 5]
#               10:27:41.4 - 10:27:41.4    10      5 NGC4826-F3     [2, 3, 4, 5]
#               10:28:46.4 - 10:28:46.4    10      6 NGC4826-F4     [2, 3, 4, 5]
#               10:29:52.3 - 10:29:52.3    10      7 NGC4826-F5     [2, 3, 4, 5]
#               10:31:00.4 - 10:31:00.4    10      8 NGC4826-F6     [2, 3, 4, 5]
#               10:32:01.9 - 10:32:01.9    10      2 NGC4826-F0     [2, 3, 4, 5]
#               10:33:06.5 - 10:33:06.5    10      3 NGC4826-F1     [2, 3, 4, 5]
#               10:34:07.9 - 10:34:07.9    10      4 NGC4826-F2     [2, 3, 4, 5]
#               10:35:14.5 - 10:35:14.5    10      5 NGC4826-F3     [2, 3, 4, 5]
#               10:36:22.7 - 10:36:22.7    10      6 NGC4826-F4     [2, 3, 4, 5]
#               10:37:28.3 - 10:37:28.3    10      7 NGC4826-F5     [2, 3, 4, 5]
#               10:38:32.3 - 10:38:32.3    10      8 NGC4826-F6     [2, 3, 4, 5]
#               10:48:46.4 - 10:48:46.4    11      2 NGC4826-F0     [2, 3, 4, 5]
#               10:49:49.4 - 10:49:49.4    11      3 NGC4826-F1     [2, 3, 4, 5]
#               10:50:55.3 - 10:50:55.3    11      4 NGC4826-F2     [2, 3, 4, 5]
#               10:52:00.6 - 10:52:00.6    11      5 NGC4826-F3     [2, 3, 4, 5]
#               10:53:05.6 - 10:53:05.6    11      6 NGC4826-F4     [2, 3, 4, 5]
#               10:54:14.7 - 10:54:14.7    11      7 NGC4826-F5     [2, 3, 4, 5]
#               10:55:16.1 - 10:55:16.1    11      8 NGC4826-F6     [2, 3, 4, 5]
#               10:56:21.4 - 10:56:21.4    11      2 NGC4826-F0     [2, 3, 4, 5]
#               10:57:25.4 - 10:57:25.4    11      3 NGC4826-F1     [2, 3, 4, 5]
#               10:58:30.4 - 10:58:30.4    11      4 NGC4826-F2     [2, 3, 4, 5]
#               10:59:39.5 - 10:59:39.5    11      5 NGC4826-F3     [2, 3, 4, 5]
#               11:00:45.4 - 11:00:45.4    11      6 NGC4826-F4     [2, 3, 4, 5]
#               11:01:47.1 - 11:01:47.1    11      7 NGC4826-F5     [2, 3, 4, 5]
#               11:02:50.5 - 11:02:50.5    11      8 NGC4826-F6     [2, 3, 4, 5]
#               11:04:29.1 - 11:04:29.1    11      2 NGC4826-F0     [2, 3, 4, 5]
#               11:05:35.4 - 11:05:35.4    11      3 NGC4826-F1     [2, 3, 4, 5]
#               11:06:42.2 - 11:06:42.2    11      4 NGC4826-F2     [2, 3, 4, 5]
#               11:07:48.1 - 11:07:48.1    11      5 NGC4826-F3     [2, 3, 4, 5]
#               11:08:53.0 - 11:08:53.0    11      6 NGC4826-F4     [2, 3, 4, 5]
#               11:09:56.4 - 11:09:56.4    11      7 NGC4826-F5     [2, 3, 4, 5]
#               11:11:04.2 - 11:11:04.2    11      8 NGC4826-F6     [2, 3, 4, 5]
#               11:12:10.2 - 11:12:10.2    11      2 NGC4826-F0     [2, 3, 4, 5]
#               11:13:14.2 - 11:13:14.2    11      3 NGC4826-F1     [2, 3, 4, 5]
#               11:14:19.2 - 11:14:19.2    11      4 NGC4826-F2     [2, 3, 4, 5]
#               11:15:25.1 - 11:15:25.1    11      5 NGC4826-F3     [2, 3, 4, 5]
#               11:16:33.3 - 11:16:33.3    11      6 NGC4826-F4     [2, 3, 4, 5]
#               11:17:39.1 - 11:17:39.1    11      7 NGC4826-F5     [2, 3, 4, 5]
#               11:18:43.1 - 11:18:43.1    11      8 NGC4826-F6     [2, 3, 4, 5]
# 
#    ObservationID = 3         ArrayID = 0
#   Date        Timerange                Scan  FldId FieldName      SpwIds
#   16-Apr-1998/03:56:19.6 - 04:01:34.5     1      1 1310+323-F0    [1]
#               04:36:40.1 - 04:41:55.3     2      1 1310+323-F0    [1]
#               05:17:03.4 - 05:22:18.3     3      1 1310+323-F0    [1]
#               05:57:33.5 - 06:02:49.0     4      1 1310+323-F0    [1]
#               06:38:15.7 - 06:43:31.0     5      1 1310+323-F0    [1]
#               07:19:15.0 - 07:24:30.5     6      1 1310+323-F0    [1]
#               07:59:47.3 - 08:05:02.5     7      1 1310+323-F0    [1]
#               08:40:09.0 - 08:45:24.2     8      1 1310+323-F0    [1]
#               09:20:20.9 - 09:25:36.1     9      1 1310+323-F0    [1]
#               10:00:35.0 - 10:05:50.1    10      1 1310+323-F0    [1]
#               10:40:40.7 - 10:45:55.7    11      1 1310+323-F0    [1]
#               11:20:49.7 - 11:28:17.0    12      1 1310+323-F0    [1]
# 
#    ObservationID = 4         ArrayID = 0
#   Date        Timerange                Scan  FldId FieldName      SpwIds
#   16-Apr-1998/04:04:30.6 - 04:04:30.6     1      2 NGC4826-F0     [2, 3, 4, 5]
#               04:05:32.1 - 04:05:32.1     1      3 NGC4826-F1     [2, 3, 4, 5]
#               04:06:38.4 - 04:06:38.4     1      4 NGC4826-F2     [2, 3, 4, 5]
#               04:07:44.2 - 04:07:44.2     1      5 NGC4826-F3     [2, 3, 4, 5]
#               04:08:49.2 - 04:08:49.2     1      6 NGC4826-F4     [2, 3, 4, 5]
#               04:09:56.5 - 04:09:56.5     1      7 NGC4826-F5     [2, 3, 4, 5]
#               04:11:00.1 - 04:11:00.1     1      8 NGC4826-F6     [2, 3, 4, 5]
#               04:12:06.4 - 04:12:06.4     1      2 NGC4826-F0     [2, 3, 4, 5]
#               04:13:10.4 - 04:13:10.4     1      3 NGC4826-F1     [2, 3, 4, 5]
#               04:14:16.3 - 04:14:16.3     1      4 NGC4826-F2     [2, 3, 4, 5]
#               04:15:25.4 - 04:15:25.4     1      5 NGC4826-F3     [2, 3, 4, 5]
#               04:16:27.1 - 04:16:27.1     1      6 NGC4826-F4     [2, 3, 4, 5]
#               04:17:32.1 - 04:17:32.1     1      7 NGC4826-F5     [2, 3, 4, 5]
#               04:18:35.5 - 04:18:35.5     1      8 NGC4826-F6     [2, 3, 4, 5]
#               04:20:20.1 - 04:20:20.1     1      2 NGC4826-F0     [2, 3, 4, 5]
#               04:21:28.3 - 04:21:28.3     1      3 NGC4826-F1     [2, 3, 4, 5]
#               04:22:31.0 - 04:22:31.0     1      4 NGC4826-F2     [2, 3, 4, 5]
#               04:23:36.9 - 04:23:36.9     1      5 NGC4826-F3     [2, 3, 4, 5]
#               04:24:41.9 - 04:24:41.9     1      6 NGC4826-F4     [2, 3, 4, 5]
#               04:25:48.4 - 04:25:48.4     1      7 NGC4826-F5     [2, 3, 4, 5]
#               04:26:53.1 - 04:26:53.1     1      8 NGC4826-F6     [2, 3, 4, 5]
#               04:27:59.0 - 04:27:59.0     1      2 NGC4826-F0     [2, 3, 4, 5]
#               04:29:03.9 - 04:29:03.9     1      3 NGC4826-F1     [2, 3, 4, 5]
#               04:30:08.9 - 04:30:08.9     1      4 NGC4826-F2     [2, 3, 4, 5]
#               04:31:15.8 - 04:31:15.8     1      5 NGC4826-F3     [2, 3, 4, 5]
#               04:32:23.9 - 04:32:23.9     1      6 NGC4826-F4     [2, 3, 4, 5]
#               04:33:28.9 - 04:33:28.9     1      7 NGC4826-F5     [2, 3, 4, 5]
#               04:34:32.9 - 04:34:32.9     1      8 NGC4826-F6     [2, 3, 4, 5]
#               04:44:53.5 - 04:44:53.5     2      2 NGC4826-F0     [2, 3, 4, 5]
#               04:45:58.7 - 04:45:58.7     2      3 NGC4826-F1     [2, 3, 4, 5]
#               04:47:04.9 - 04:47:04.9     2      4 NGC4826-F2     [2, 3, 4, 5]
#               04:48:06.4 - 04:48:06.4     2      5 NGC4826-F3     [2, 3, 4, 5]
#               04:49:11.0 - 04:49:11.0     2      6 NGC4826-F4     [2, 3, 4, 5]
#               04:50:18.8 - 04:50:18.8     2      7 NGC4826-F5     [2, 3, 4, 5]
#               04:51:22.5 - 04:51:22.5     2      8 NGC4826-F6     [2, 3, 4, 5]
#               04:52:28.7 - 04:52:28.7     2      2 NGC4826-F0     [2, 3, 4, 5]
#               04:53:32.7 - 04:53:32.7     2      3 NGC4826-F1     [2, 3, 4, 5]
#               04:54:37.7 - 04:54:37.7     2      4 NGC4826-F2     [2, 3, 4, 5]
#               04:55:46.8 - 04:55:46.8     2      5 NGC4826-F3     [2, 3, 4, 5]
#               04:56:48.6 - 04:56:48.6     2      6 NGC4826-F4     [2, 3, 4, 5]
#               04:57:53.6 - 04:57:53.6     2      7 NGC4826-F5     [2, 3, 4, 5]
#               04:58:58.8 - 04:58:58.8     2      8 NGC4826-F6     [2, 3, 4, 5]
#               05:00:41.5 - 05:00:41.5     2      2 NGC4826-F0     [2, 3, 4, 5]
#               05:01:51.1 - 05:01:51.1     2      3 NGC4826-F1     [2, 3, 4, 5]
#               05:02:57.2 - 05:02:57.2     2      4 NGC4826-F2     [2, 3, 4, 5]
#               05:04:02.2 - 05:04:02.2     2      5 NGC4826-F3     [2, 3, 4, 5]
#               05:05:07.5 - 05:05:07.5     2      6 NGC4826-F4     [2, 3, 4, 5]
#               05:06:12.1 - 05:06:12.1     2      7 NGC4826-F5     [2, 3, 4, 5]
#               05:07:20.3 - 05:07:20.3     2      8 NGC4826-F6     [2, 3, 4, 5]
#               05:08:25.2 - 05:08:25.2     2      2 NGC4826-F0     [2, 3, 4, 5]
#               05:09:28.3 - 05:09:28.3     2      3 NGC4826-F1     [2, 3, 4, 5]
#               05:10:34.2 - 05:10:34.2     2      4 NGC4826-F2     [2, 3, 4, 5]
#               05:11:39.5 - 05:11:39.5     2      5 NGC4826-F3     [2, 3, 4, 5]
#               05:12:47.4 - 05:12:47.4     2      6 NGC4826-F4     [2, 3, 4, 5]
#               05:13:51.4 - 05:13:51.4     2      7 NGC4826-F5     [2, 3, 4, 5]
#               05:14:53.1 - 05:14:53.1     2      8 NGC4826-F6     [2, 3, 4, 5]
#               05:25:18.0 - 05:25:18.0     3      2 NGC4826-F0     [2, 3, 4, 5]
#               05:26:22.0 - 05:26:22.0     3      3 NGC4826-F1     [2, 3, 4, 5]
#               05:27:27.0 - 05:27:27.0     3      4 NGC4826-F2     [2, 3, 4, 5]
#               05:28:34.1 - 05:28:34.1     3      5 NGC4826-F3     [2, 3, 4, 5]
#               05:29:39.1 - 05:29:39.1     3      6 NGC4826-F4     [2, 3, 4, 5]
#               05:30:47.3 - 05:30:47.3     3      7 NGC4826-F5     [2, 3, 4, 5]
#               05:31:49.0 - 05:31:49.0     3      8 NGC4826-F6     [2, 3, 4, 5]
#               05:32:54.0 - 05:32:54.0     3      2 NGC4826-F0     [2, 3, 4, 5]
#               05:33:58.0 - 05:33:58.0     3      3 NGC4826-F1     [2, 3, 4, 5]
#               05:35:03.0 - 05:35:03.0     3      4 NGC4826-F2     [2, 3, 4, 5]
#               05:36:11.1 - 05:36:11.1     3      5 NGC4826-F3     [2, 3, 4, 5]
#               05:37:12.9 - 05:37:12.9     3      6 NGC4826-F4     [2, 3, 4, 5]
#               05:38:17.2 - 05:38:17.2     3      7 NGC4826-F5     [2, 3, 4, 5]
#               05:39:19.0 - 05:39:19.0     3      8 NGC4826-F6     [2, 3, 4, 5]
#               05:41:01.7 - 05:41:01.7     3      2 NGC4826-F0     [2, 3, 4, 5]
#               05:42:09.9 - 05:42:09.9     3      3 NGC4826-F1     [2, 3, 4, 5]
#               05:43:14.8 - 05:43:14.8     3      4 NGC4826-F2     [2, 3, 4, 5]
#               05:44:20.7 - 05:44:20.7     3      5 NGC4826-F3     [2, 3, 4, 5]
#               05:45:26.0 - 05:45:26.0     3      6 NGC4826-F4     [2, 3, 4, 5]
#               05:46:28.7 - 05:46:28.7     3      7 NGC4826-F5     [2, 3, 4, 5]
#               05:47:36.9 - 05:47:36.9     3      8 NGC4826-F6     [2, 3, 4, 5]
#               05:48:41.9 - 05:48:41.9     3      2 NGC4826-F0     [2, 3, 4, 5]
#               05:49:45.8 - 05:49:45.8     3      3 NGC4826-F1     [2, 3, 4, 5]
#               05:50:51.8 - 05:50:51.8     3      4 NGC4826-F2     [2, 3, 4, 5]
#               05:51:56.7 - 05:51:56.7     3      5 NGC4826-F3     [2, 3, 4, 5]
#               05:53:05.9 - 05:53:05.9     3      6 NGC4826-F4     [2, 3, 4, 5]
#               05:54:10.8 - 05:54:10.8     3      7 NGC4826-F5     [2, 3, 4, 5]
#               05:55:15.8 - 05:55:15.8     3      8 NGC4826-F6     [2, 3, 4, 5]
#               06:05:54.2 - 06:05:54.2     4      2 NGC4826-F0     [2, 3, 4, 5]
#               06:06:59.1 - 06:06:59.1     4      3 NGC4826-F1     [2, 3, 4, 5]
#               06:08:04.1 - 06:08:04.1     4      4 NGC4826-F2     [2, 3, 4, 5]
#               06:09:09.1 - 06:09:09.1     4      5 NGC4826-F3     [2, 3, 4, 5]
#               06:10:14.1 - 06:10:14.1     4      6 NGC4826-F4     [2, 3, 4, 5]
#               06:11:20.3 - 06:11:20.3     4      7 NGC4826-F5     [2, 3, 4, 5]
#               06:12:24.9 - 06:12:24.9     4      8 NGC4826-F6     [2, 3, 4, 5]
#               06:13:30.8 - 06:13:30.8     4      2 NGC4826-F0     [2, 3, 4, 5]
#               06:14:34.8 - 06:14:34.8     4      3 NGC4826-F1     [2, 3, 4, 5]
#               06:15:41.1 - 06:15:41.1     4      4 NGC4826-F2     [2, 3, 4, 5]
#               06:16:48.9 - 06:16:48.9     4      5 NGC4826-F3     [2, 3, 4, 5]
#               06:17:53.9 - 06:17:53.9     4      6 NGC4826-F4     [2, 3, 4, 5]
#               06:18:56.9 - 06:18:56.9     4      7 NGC4826-F5     [2, 3, 4, 5]
#               06:20:00.9 - 06:20:00.9     4      8 NGC4826-F6     [2, 3, 4, 5]
#               06:21:40.5 - 06:21:40.5     4      2 NGC4826-F0     [2, 3, 4, 5]
#               06:22:48.9 - 06:22:48.9     4      3 NGC4826-F1     [2, 3, 4, 5]
#               06:23:53.9 - 06:23:53.9     4      4 NGC4826-F2     [2, 3, 4, 5]
#               06:24:59.8 - 06:24:59.8     4      5 NGC4826-F3     [2, 3, 4, 5]
#               06:26:04.7 - 06:26:04.7     4      6 NGC4826-F4     [2, 3, 4, 5]
#               06:27:08.7 - 06:27:08.7     4      7 NGC4826-F5     [2, 3, 4, 5]
#               06:28:15.9 - 06:28:15.9     4      8 NGC4826-F6     [2, 3, 4, 5]
#               06:29:20.9 - 06:29:20.9     4      2 NGC4826-F0     [2, 3, 4, 5]
#               06:30:24.9 - 06:30:24.9     4      3 NGC4826-F1     [2, 3, 4, 5]
#               06:31:30.5 - 06:31:30.5     4      4 NGC4826-F2     [2, 3, 4, 5]
#               06:32:36.7 - 06:32:36.7     4      5 NGC4826-F3     [2, 3, 4, 5]
#               06:33:43.9 - 06:33:43.9     4      6 NGC4826-F4     [2, 3, 4, 5]
#               06:34:49.6 - 06:34:49.6     4      7 NGC4826-F5     [2, 3, 4, 5]
#               06:35:53.5 - 06:35:53.5     4      8 NGC4826-F6     [2, 3, 4, 5]
#               06:46:40.6 - 06:46:40.6     5      2 NGC4826-F0     [2, 3, 4, 5]
#               06:47:44.6 - 06:47:44.6     5      3 NGC4826-F1     [2, 3, 4, 5]
#               06:48:51.8 - 06:48:51.8     5      4 NGC4826-F2     [2, 3, 4, 5]
#               06:49:57.1 - 06:49:57.1     5      5 NGC4826-F3     [2, 3, 4, 5]
#               06:51:01.7 - 06:51:01.7     5      6 NGC4826-F4     [2, 3, 4, 5]
#               06:52:09.9 - 06:52:09.9     5      7 NGC4826-F5     [2, 3, 4, 5]
#               06:53:14.5 - 06:53:14.5     5      8 NGC4826-F6     [2, 3, 4, 5]
#               06:54:20.5 - 06:54:20.5     5      2 NGC4826-F0     [2, 3, 4, 5]
#               06:55:24.7 - 06:55:24.7     5      3 NGC4826-F1     [2, 3, 4, 5]
#               06:56:29.7 - 06:56:29.7     5      4 NGC4826-F2     [2, 3, 4, 5]
#               06:57:37.5 - 06:57:37.5     5      5 NGC4826-F3     [2, 3, 4, 5]
#               06:58:42.5 - 06:58:42.5     5      6 NGC4826-F4     [2, 3, 4, 5]
#               06:59:47.5 - 06:59:47.5     5      7 NGC4826-F5     [2, 3, 4, 5]
#               07:00:53.7 - 07:00:53.7     5      8 NGC4826-F6     [2, 3, 4, 5]
#               07:02:37.5 - 07:02:37.5     5      2 NGC4826-F0     [2, 3, 4, 5]
#               07:03:46.5 - 07:03:46.5     5      3 NGC4826-F1     [2, 3, 4, 5]
#               07:04:52.1 - 07:04:52.1     5      4 NGC4826-F2     [2, 3, 4, 5]
#               07:05:58.0 - 07:05:58.0     5      5 NGC4826-F3     [2, 3, 4, 5]
#               07:07:03.0 - 07:07:03.0     5      6 NGC4826-F4     [2, 3, 4, 5]
#               07:08:07.9 - 07:08:07.9     5      7 NGC4826-F5     [2, 3, 4, 5]
#               07:09:17.1 - 07:09:17.1     5      8 NGC4826-F6     [2, 3, 4, 5]
#               07:10:22.0 - 07:10:22.0     5      2 NGC4826-F0     [2, 3, 4, 5]
#               07:11:28.9 - 07:11:28.9     5      3 NGC4826-F1     [2, 3, 4, 5]
#               07:12:33.9 - 07:12:33.9     5      4 NGC4826-F2     [2, 3, 4, 5]
#               07:13:39.8 - 07:13:39.8     5      5 NGC4826-F3     [2, 3, 4, 5]
#               07:14:48.9 - 07:14:48.9     5      6 NGC4826-F4     [2, 3, 4, 5]
#               07:15:53.9 - 07:15:53.9     5      7 NGC4826-F5     [2, 3, 4, 5]
#               07:16:59.8 - 07:16:59.8     5      8 NGC4826-F6     [2, 3, 4, 5]
#               07:27:30.5 - 07:27:30.5     6      2 NGC4826-F0     [2, 3, 4, 5]
#               07:28:36.4 - 07:28:36.4     6      3 NGC4826-F1     [2, 3, 4, 5]
#               07:29:38.2 - 07:29:38.2     6      4 NGC4826-F2     [2, 3, 4, 5]
#               07:30:44.5 - 07:30:44.5     6      5 NGC4826-F3     [2, 3, 4, 5]
#               07:31:50.4 - 07:31:50.4     6      6 NGC4826-F4     [2, 3, 4, 5]
#               07:32:58.5 - 07:32:58.5     6      7 NGC4826-F5     [2, 3, 4, 5]
#               07:34:05.0 - 07:34:05.0     6      8 NGC4826-F6     [2, 3, 4, 5]
#               07:35:11.0 - 07:35:11.0     6      2 NGC4826-F0     [2, 3, 4, 5]
#               07:36:15.0 - 07:36:15.0     6      3 NGC4826-F1     [2, 3, 4, 5]
#               07:37:20.9 - 07:37:20.9     6      4 NGC4826-F2     [2, 3, 4, 5]
#               07:38:30.0 - 07:38:30.0     6      5 NGC4826-F3     [2, 3, 4, 5]
#               07:39:31.8 - 07:39:31.8     6      6 NGC4826-F4     [2, 3, 4, 5]
#               07:40:38.1 - 07:40:38.1     6      7 NGC4826-F5     [2, 3, 4, 5]
#               07:41:43.9 - 07:41:43.9     6      8 NGC4826-F6     [2, 3, 4, 5]
#               07:43:25.1 - 07:43:25.1     6      2 NGC4826-F0     [2, 3, 4, 5]
#               07:44:32.9 - 07:44:32.9     6      3 NGC4826-F1     [2, 3, 4, 5]
#               07:45:37.9 - 07:45:37.9     6      4 NGC4826-F2     [2, 3, 4, 5]
#               07:46:42.8 - 07:46:42.8     6      5 NGC4826-F3     [2, 3, 4, 5]
#               07:47:48.7 - 07:47:48.7     6      6 NGC4826-F4     [2, 3, 4, 5]
#               07:48:53.7 - 07:48:53.7     6      7 NGC4826-F5     [2, 3, 4, 5]
#               07:50:04.2 - 07:50:04.2     6      8 NGC4826-F6     [2, 3, 4, 5]
#               07:51:09.4 - 07:51:09.4     6      2 NGC4826-F0     [2, 3, 4, 5]
#               07:52:13.7 - 07:52:13.7     6      3 NGC4826-F1     [2, 3, 4, 5]
#               07:53:18.5 - 07:53:18.5     6      4 NGC4826-F2     [2, 3, 4, 5]
#               07:54:24.6 - 07:54:24.6     6      5 NGC4826-F3     [2, 3, 4, 5]
#               07:55:32.5 - 07:55:32.5     6      6 NGC4826-F4     [2, 3, 4, 5]
#               07:56:36.5 - 07:56:36.5     6      7 NGC4826-F5     [2, 3, 4, 5]
#               07:57:42.4 - 07:57:42.4     6      8 NGC4826-F6     [2, 3, 4, 5]
#               08:07:53.9 - 08:07:53.9     7      2 NGC4826-F0     [2, 3, 4, 5]
#               08:08:59.4 - 08:08:59.4     7      3 NGC4826-F1     [2, 3, 4, 5]
#               08:10:05.7 - 08:10:05.7     7      4 NGC4826-F2     [2, 3, 4, 5]
#               08:11:12.6 - 08:11:12.6     7      5 NGC4826-F3     [2, 3, 4, 5]
#               08:12:18.5 - 08:12:18.5     7      6 NGC4826-F4     [2, 3, 4, 5]
#               08:13:25.7 - 08:13:25.7     7      7 NGC4826-F5     [2, 3, 4, 5]
#               08:14:29.4 - 08:14:29.4     7      8 NGC4826-F6     [2, 3, 4, 5]
#               08:15:35.4 - 08:15:35.4     7      2 NGC4826-F0     [2, 3, 4, 5]
#               08:16:39.4 - 08:16:39.4     7      3 NGC4826-F1     [2, 3, 4, 5]
#               08:17:45.2 - 08:17:45.2     7      4 NGC4826-F2     [2, 3, 4, 5]
#               08:18:54.7 - 08:18:54.7     7      5 NGC4826-F3     [2, 3, 4, 5]
#               08:20:00.2 - 08:20:00.2     7      6 NGC4826-F4     [2, 3, 4, 5]
#               08:21:06.2 - 08:21:06.2     7      7 NGC4826-F5     [2, 3, 4, 5]
#               08:22:10.2 - 08:22:10.2     7      8 NGC4826-F6     [2, 3, 4, 5]
#               08:23:52.9 - 08:23:52.9     7      2 NGC4826-F0     [2, 3, 4, 5]
#               08:25:00.1 - 08:25:00.1     7      3 NGC4826-F1     [2, 3, 4, 5]
#               08:26:04.1 - 08:26:04.1     7      4 NGC4826-F2     [2, 3, 4, 5]
#               08:27:11.0 - 08:27:11.0     7      5 NGC4826-F3     [2, 3, 4, 5]
#               08:28:16.9 - 08:28:16.9     7      6 NGC4826-F4     [2, 3, 4, 5]
#               08:29:20.9 - 08:29:20.9     7      7 NGC4826-F5     [2, 3, 4, 5]
#               08:30:29.0 - 08:30:29.0     7      8 NGC4826-F6     [2, 3, 4, 5]
#               08:31:30.5 - 08:31:30.5     7      2 NGC4826-F0     [2, 3, 4, 5]
#               08:32:35.1 - 08:32:35.1     7      3 NGC4826-F1     [2, 3, 4, 5]
#               08:33:39.1 - 08:33:39.1     7      4 NGC4826-F2     [2, 3, 4, 5]
#               08:34:44.7 - 08:34:44.7     7      5 NGC4826-F3     [2, 3, 4, 5]
#               08:35:52.9 - 08:35:52.9     7      6 NGC4826-F4     [2, 3, 4, 5]
#               08:36:56.9 - 08:36:56.9     7      7 NGC4826-F5     [2, 3, 4, 5]
#               08:38:00.9 - 08:38:00.9     7      8 NGC4826-F6     [2, 3, 4, 5]
#               08:48:18.5 - 08:48:18.5     8      2 NGC4826-F0     [2, 3, 4, 5]
#               08:49:23.1 - 08:49:23.1     8      3 NGC4826-F1     [2, 3, 4, 5]
#               08:50:26.2 - 08:50:26.2     8      4 NGC4826-F2     [2, 3, 4, 5]
#               08:51:32.2 - 08:51:32.2     8      5 NGC4826-F3     [2, 3, 4, 5]
#               08:52:38.0 - 08:52:38.0     8      6 NGC4826-F4     [2, 3, 4, 5]
#               08:53:44.9 - 08:53:44.9     8      7 NGC4826-F5     [2, 3, 4, 5]
#               08:54:48.9 - 08:54:48.9     8      8 NGC4826-F6     [2, 3, 4, 5]
#               08:55:53.8 - 08:55:53.8     8      2 NGC4826-F0     [2, 3, 4, 5]
#               08:56:57.8 - 08:56:57.8     8      3 NGC4826-F1     [2, 3, 4, 5]
#               08:58:02.8 - 08:58:02.8     8      4 NGC4826-F2     [2, 3, 4, 5]
#               08:59:11.9 - 08:59:11.9     8      5 NGC4826-F3     [2, 3, 4, 5]
#               09:00:16.9 - 09:00:16.9     8      6 NGC4826-F4     [2, 3, 4, 5]
#               09:01:20.9 - 09:01:20.9     8      7 NGC4826-F5     [2, 3, 4, 5]
#               09:02:24.9 - 09:02:24.9     8      8 NGC4826-F6     [2, 3, 4, 5]
#               09:04:05.7 - 09:04:05.7     8      2 NGC4826-F0     [2, 3, 4, 5]
#               09:05:13.9 - 09:05:13.9     8      3 NGC4826-F1     [2, 3, 4, 5]
#               09:06:15.6 - 09:06:15.6     8      4 NGC4826-F2     [2, 3, 4, 5]
#               09:07:22.9 - 09:07:22.9     8      5 NGC4826-F3     [2, 3, 4, 5]
#               09:08:27.8 - 09:08:27.8     8      6 NGC4826-F4     [2, 3, 4, 5]
#               09:09:31.8 - 09:09:31.8     8      7 NGC4826-F5     [2, 3, 4, 5]
#               09:10:39.0 - 09:10:39.0     8      8 NGC4826-F6     [2, 3, 4, 5]
#               09:11:40.5 - 09:11:40.5     8      2 NGC4826-F0     [2, 3, 4, 5]
#               09:12:43.8 - 09:12:43.8     8      3 NGC4826-F1     [2, 3, 4, 5]
#               09:13:48.7 - 09:13:48.7     8      4 NGC4826-F2     [2, 3, 4, 5]
#               09:14:54.7 - 09:14:54.7     8      5 NGC4826-F3     [2, 3, 4, 5]
#               09:16:02.8 - 09:16:02.8     8      6 NGC4826-F4     [2, 3, 4, 5]
#               09:17:07.8 - 09:17:07.8     8      7 NGC4826-F5     [2, 3, 4, 5]
#               09:18:12.7 - 09:18:12.7     8      8 NGC4826-F6     [2, 3, 4, 5]
#               09:28:31.3 - 09:28:31.3     9      2 NGC4826-F0     [2, 3, 4, 5]
#               09:29:35.6 - 09:29:35.6     9      3 NGC4826-F1     [2, 3, 4, 5]
#               09:30:40.6 - 09:30:40.6     9      4 NGC4826-F2     [2, 3, 4, 5]
#               09:31:46.8 - 09:31:46.8     9      5 NGC4826-F3     [2, 3, 4, 5]
#               09:32:51.8 - 09:32:51.8     9      6 NGC4826-F4     [2, 3, 4, 5]
#               09:33:56.4 - 09:33:56.4     9      7 NGC4826-F5     [2, 3, 4, 5]
#               09:35:01.7 - 09:35:01.7     9      8 NGC4826-F6     [2, 3, 4, 5]
#               09:36:06.7 - 09:36:06.7     9      2 NGC4826-F0     [2, 3, 4, 5]
#               09:37:10.6 - 09:37:10.6     9      3 NGC4826-F1     [2, 3, 4, 5]
#               09:38:15.6 - 09:38:15.6     9      4 NGC4826-F2     [2, 3, 4, 5]
#               09:39:24.7 - 09:39:24.7     9      5 NGC4826-F3     [2, 3, 4, 5]
#               09:40:30.4 - 09:40:30.4     9      6 NGC4826-F4     [2, 3, 4, 5]
#               09:41:33.4 - 09:41:33.4     9      7 NGC4826-F5     [2, 3, 4, 5]
#               09:42:38.7 - 09:42:38.7     9      8 NGC4826-F6     [2, 3, 4, 5]
#               09:44:16.3 - 09:44:16.3     9      2 NGC4826-F0     [2, 3, 4, 5]
#               09:45:24.4 - 09:45:24.4     9      3 NGC4826-F1     [2, 3, 4, 5]
#               09:46:29.4 - 09:46:29.4     9      4 NGC4826-F2     [2, 3, 4, 5]
#               09:47:35.6 - 09:47:35.6     9      5 NGC4826-F3     [2, 3, 4, 5]
#               09:48:40.6 - 09:48:40.6     9      6 NGC4826-F4     [2, 3, 4, 5]
#               09:49:45.6 - 09:49:45.6     9      7 NGC4826-F5     [2, 3, 4, 5]
#               09:50:53.7 - 09:50:53.7     9      8 NGC4826-F6     [2, 3, 4, 5]
#               09:51:55.1 - 09:51:55.1     9      2 NGC4826-F0     [2, 3, 4, 5]
#               09:53:00.4 - 09:53:00.4     9      3 NGC4826-F1     [2, 3, 4, 5]
#               09:54:06.4 - 09:54:06.4     9      4 NGC4826-F2     [2, 3, 4, 5]
#               09:55:12.2 - 09:55:12.2     9      5 NGC4826-F3     [2, 3, 4, 5]
#               09:56:20.4 - 09:56:20.4     9      6 NGC4826-F4     [2, 3, 4, 5]
#               09:57:22.2 - 09:57:22.2     9      7 NGC4826-F5     [2, 3, 4, 5]
#               09:58:27.1 - 09:58:27.1     9      8 NGC4826-F6     [2, 3, 4, 5]
#               10:08:43.5 - 10:08:43.5    10      2 NGC4826-F0     [2, 3, 4, 5]
#               10:09:44.9 - 10:09:44.9    10      3 NGC4826-F1     [2, 3, 4, 5]
#               10:10:46.4 - 10:10:46.4    10      4 NGC4826-F2     [2, 3, 4, 5]
#               10:11:51.6 - 10:11:51.6    10      5 NGC4826-F3     [2, 3, 4, 5]
#               10:12:56.6 - 10:12:56.6    10      6 NGC4826-F4     [2, 3, 4, 5]
#               10:14:04.7 - 10:14:04.7    10      7 NGC4826-F5     [2, 3, 4, 5]
#               10:15:06.2 - 10:15:06.2    10      8 NGC4826-F6     [2, 3, 4, 5]
#               10:16:12.5 - 10:16:12.5    10      2 NGC4826-F0     [2, 3, 4, 5]
#               10:17:15.8 - 10:17:15.8    10      3 NGC4826-F1     [2, 3, 4, 5]
#               10:18:21.7 - 10:18:21.7    10      4 NGC4826-F2     [2, 3, 4, 5]
#               10:19:29.5 - 10:19:29.5    10      5 NGC4826-F3     [2, 3, 4, 5]
#               10:20:34.5 - 10:20:34.5    10      6 NGC4826-F4     [2, 3, 4, 5]
#               10:21:40.5 - 10:21:40.5    10      7 NGC4826-F5     [2, 3, 4, 5]
#               10:22:45.4 - 10:22:45.4    10      8 NGC4826-F6     [2, 3, 4, 5]
#               10:24:27.1 - 10:24:27.1    10      2 NGC4826-F0     [2, 3, 4, 5]
#               10:25:33.7 - 10:25:33.7    10      3 NGC4826-F1     [2, 3, 4, 5]
#               10:26:35.1 - 10:26:35.1    10      4 NGC4826-F2     [2, 3, 4, 5]
#               10:27:41.4 - 10:27:41.4    10      5 NGC4826-F3     [2, 3, 4, 5]
#               10:28:46.4 - 10:28:46.4    10      6 NGC4826-F4     [2, 3, 4, 5]
#               10:29:52.3 - 10:29:52.3    10      7 NGC4826-F5     [2, 3, 4, 5]
#               10:31:00.4 - 10:31:00.4    10      8 NGC4826-F6     [2, 3, 4, 5]
#               10:32:01.9 - 10:32:01.9    10      2 NGC4826-F0     [2, 3, 4, 5]
#               10:33:06.5 - 10:33:06.5    10      3 NGC4826-F1     [2, 3, 4, 5]
#               10:34:07.9 - 10:34:07.9    10      4 NGC4826-F2     [2, 3, 4, 5]
#               10:35:14.5 - 10:35:14.5    10      5 NGC4826-F3     [2, 3, 4, 5]
#               10:36:22.7 - 10:36:22.7    10      6 NGC4826-F4     [2, 3, 4, 5]
#               10:37:28.3 - 10:37:28.3    10      7 NGC4826-F5     [2, 3, 4, 5]
#               10:38:32.3 - 10:38:32.3    10      8 NGC4826-F6     [2, 3, 4, 5]
#               10:48:46.4 - 10:48:46.4    11      2 NGC4826-F0     [2, 3, 4, 5]
#               10:49:49.4 - 10:49:49.4    11      3 NGC4826-F1     [2, 3, 4, 5]
#               10:50:55.3 - 10:50:55.3    11      4 NGC4826-F2     [2, 3, 4, 5]
#               10:52:00.6 - 10:52:00.6    11      5 NGC4826-F3     [2, 3, 4, 5]
#               10:53:05.6 - 10:53:05.6    11      6 NGC4826-F4     [2, 3, 4, 5]
#               10:54:14.7 - 10:54:14.7    11      7 NGC4826-F5     [2, 3, 4, 5]
#               10:55:16.1 - 10:55:16.1    11      8 NGC4826-F6     [2, 3, 4, 5]
#               10:56:21.4 - 10:56:21.4    11      2 NGC4826-F0     [2, 3, 4, 5]
#               10:57:25.4 - 10:57:25.4    11      3 NGC4826-F1     [2, 3, 4, 5]
#               10:58:30.4 - 10:58:30.4    11      4 NGC4826-F2     [2, 3, 4, 5]
#               10:59:39.5 - 10:59:39.5    11      5 NGC4826-F3     [2, 3, 4, 5]
#               11:00:45.4 - 11:00:45.4    11      6 NGC4826-F4     [2, 3, 4, 5]
#               11:01:47.1 - 11:01:47.1    11      7 NGC4826-F5     [2, 3, 4, 5]
#               11:02:50.5 - 11:02:50.5    11      8 NGC4826-F6     [2, 3, 4, 5]
#               11:04:29.1 - 11:04:29.1    11      2 NGC4826-F0     [2, 3, 4, 5]
#               11:05:35.4 - 11:05:35.4    11      3 NGC4826-F1     [2, 3, 4, 5]
#               11:06:42.2 - 11:06:42.2    11      4 NGC4826-F2     [2, 3, 4, 5]
#               11:07:48.1 - 11:07:48.1    11      5 NGC4826-F3     [2, 3, 4, 5]
#               11:08:53.0 - 11:08:53.0    11      6 NGC4826-F4     [2, 3, 4, 5]
#               11:09:56.4 - 11:09:56.4    11      7 NGC4826-F5     [2, 3, 4, 5]
#               11:11:04.2 - 11:11:04.2    11      8 NGC4826-F6     [2, 3, 4, 5]
#               11:12:10.2 - 11:12:10.2    11      2 NGC4826-F0     [2, 3, 4, 5]
#               11:13:14.2 - 11:13:14.2    11      3 NGC4826-F1     [2, 3, 4, 5]
#               11:14:19.2 - 11:14:19.2    11      4 NGC4826-F2     [2, 3, 4, 5]
#               11:15:25.1 - 11:15:25.1    11      5 NGC4826-F3     [2, 3, 4, 5]
#               11:16:33.3 - 11:16:33.3    11      6 NGC4826-F4     [2, 3, 4, 5]
#               11:17:39.1 - 11:17:39.1    11      7 NGC4826-F5     [2, 3, 4, 5]
#               11:18:43.1 - 11:18:43.1    11      8 NGC4826-F6     [2, 3, 4, 5]
# Fields: 9
#   ID   Code Name          Right Ascension  Declination   Epoch   
#   0         3C273-F0      12:29:06.70      +02.03.08.60  J2000   
#   1         1310+323-F0   13:10:28.66      +32.20.43.78  J2000   
#   2         NGC4826-F0    12:56:44.24      +21.41.05.10  J2000   
#   3         NGC4826-F1    12:56:41.08      +21.41.05.10  J2000   
#   4         NGC4826-F2    12:56:42.66      +21.41.43.20  J2000   
#   5         NGC4826-F3    12:56:45.82      +21.41.43.20  J2000   
#   6         NGC4826-F4    12:56:47.40      +21.41.05.10  J2000   
#   7         NGC4826-F5    12:56:45.82      +21.40.27.00  J2000   
#   8         NGC4826-F6    12:56:42.66      +21.40.27.00  J2000   
# Spectral Windows:  (6 unique spectral windows and 1 unique polarization setups)
#   SpwID  #Chans Frame Ch1(MHz)    ChanWid(kHz)TotBW(kHz)  Ref(MHz)    Corrs
#   0           1 LSRD  115138.579  400000.006  400000.006  115271.2    YY  
#   1           1 LSRD  115217.017  800000.072  800000.072  115271.2    YY  
#   2          64 LSRD  114950.387  1562.5      100000      115271.2    YY  
#   3          64 LSRD  115040.402  1562.5      100000      115271.2    YY  
#   4          64 LSRD  115130.143  1562.5      100000      115271.2    YY  
#   5          64 LSRD  115220.157  1562.5      100000      115271.2    YY  
# Feeds: 10: printing first row only
#   Antenna   Spectral Window     # Receptors    Polarizations
#   1         -1                  1              [         X]
# Antennas: 10:
#   ID   Name  Station   Diam.    Long.         Lat.         
#   0    ANT1  UNKNOWN   6.0  m   -121.28.08.0  +40.37.38.5  
#   1    ANT2  UNKNOWN   6.0  m   -121.28.09.0  +40.37.37.1  
#   2    ANT3  UNKNOWN   6.0  m   -121.28.08.0  +40.37.38.9  
#   3    ANT4  UNKNOWN   6.0  m   -121.28.07.8  +40.37.37.1  
#   4    ANT5  UNKNOWN   6.0  m   -121.28.05.9  +40.37.37.1  
#   5    ANT6  UNKNOWN   6.0  m   -121.28.08.2  +40.37.37.1  
#   6    ANT7  UNKNOWN   6.0  m   -121.28.06.4  +40.37.37.1  
#   7    ANT8  UNKNOWN   6.0  m   -121.28.08.0  +40.37.37.9  
#   8    ANT9  UNKNOWN   6.0  m   -121.28.09.3  +40.37.37.1  
#   9    ANT10 UNKNOWN   6.0  m   -121.28.08.0  +40.37.37.3  
# 
# 
# Tables:
#    MAIN                  142155 rows     
#    ANTENNA                   10 rows     
#    DATA_DESCRIPTION           6 rows     
#    DOPPLER              <empty>  
#    FEED                      10 rows     
#    FIELD                      9 rows     
#    FLAG_CMD             <empty>  
#    FREQ_OFFSET         <absent>  
#    HISTORY                  155 rows     
#    OBSERVATION                5 rows     
#    POINTING             <empty>  
#    POLARIZATION               1 row      
#    PROCESSOR                  1 row      
#    SOURCE                     5 rows     
#    SPECTRAL_WINDOW            6 rows     
#    STATE                      1 row      
#    SYSCAL                    20 rows     
#    WEATHER             <absent>  
#
###  End Task: listobs  ###
###########################

