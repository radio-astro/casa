###############################################
#                                             #
# Regression/Benchmarking Script for NGC 7538 #
#                                             #
###############################################

import time
import os

os.system('rm -rf ngc7538d* 1328.* 2229.cont2* ap314.* ngc7538*.ms')

startTime = time.time()
startProc = time.clock()

datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/ATST1/NGC7538/'

print '--Import--'
default('importvla')
importvla(archivefiles=[datapath+'AP314_A950519.xp1',
                        datapath+'AP314_A950519.xp2',
                        datapath+'AP314_A950519.xp3'],
	  vis='ngc7538.ms', bandname='K', frequencytol=10000000.0)
importtime = time.time() 
print '--Observation summary--'
listobs(vis='ngc7538.ms')
#listtime = time.time()
print '--Flagautocorr--'
default('flagautocorr')
flagautocorr(vis='ngc7538.ms')
flagtime = time.time()
print '--Setjy--'
default('setjy')
setjy(vis='ngc7538.ms',field='0') #set flux density for 1331+305 (3C286)
setjytime = time.time()
print '--Gaincal--'
default('gaincal')
gaincal(vis='ngc7538.ms', caltable='ap314.gcal',
	field='<2', spw='0~1:2~56', gaintype='G',
	opacity=0.08,solint='inf', combine='', refant='VA19')
gaintime = time.time()
print '--Bandpass--'
default('bandpass')
bandpass(vis='ngc7538.ms', caltable='1328.bcal',
	 field='0', opacity=0.08,
	 gaintable='ap314.gcal', interp='nearest',
	 refant='VA19')
bptime = time.time()
print '--Fluxscale--'
default('fluxscale')
fluxscale(vis='ngc7538.ms', caltable='ap314.gcal', fluxtable='ap314.fluxcal',
	  reference=['1328+307'], transfer=['2229+695'])
fstime = time.time()
print '--Apply Cal--'
default('applycal')
applycal(vis='ngc7538.ms',
	 field='1~5',
	 opacity=0.08,
	 gaintable=['ap314.fluxcal', '1328.bcal'],
	 gainfield='1')
	 
correcttime = time.time()

print '--Split (fluxcal data)--'
default('split')
split(vis='ngc7538.ms', outputvis='ngc7538_cal.split.ms',
#      field=0,spw=0,nchan=62,start=0,step=1,datacolumn='MODEL_DATA')
	field='0',spw='0:0~61', datacolumn='model')
print '--Split (continuum)--'
default('split')
split(vis='ngc7538.ms', outputvis='ngc7538d.cont.ms',
      field='3',spw='0:2~56^55', datacolumn='corrected')
	#,nchan=1,start=2,step=55,datacolumn='CORRECTED_DATA')
print '--Split (mf cont,)--'
default('split')
split(vis='ngc7538.ms', outputvis='ngc7538.cont.ms',
	field='3,4,5',spw='0:2~56^55', datacolumn='corrected')
      #field=[3,4,5],spw=0,nchan=1,start=2,step=55,datacolumn='CORRECTED_DATA')
print '--Split (bandcal data)--'
default('split')
split(vis='ngc7538.ms', outputvis='2229.cont2.ms',
#      field=1,spw=[0,1],nchan=1,start=2,step=55,datacolumn='CORRECTED_DATA')
	field='1', spw='0:2~56^55,1:2~56^55', datacolumn='corrected')
splitcaltime = time.time()
default('split')
split(vis='ngc7538.ms',outputvis='ngc7538d.line.ms',
#      field=3,spw=[0,1],nchan=55,start=2,step=1,datacolumn='CORRECTED_DATA')
	field='3',spw='0:2~56,1:2~56',datacolumn='corrected')
splitsrctime = time.time()

print '--Clean cal--'
default('clean')
clean(vis='2229.cont2.ms',imagename='2229.cont2',mode='channel',
      psfmode='hogbom',niter=6000,gain=0.1,threshold=8.,mask='',
      nchan=1,start=0,width=1,field='0',spw=0,
      imsize=[256,256],cell=[0.5,0.5],
      weighting='briggs',robust=0.5)
cleantime1 = time.time()
print '--Clean src cont--'
default('clean')
clean(vis='ngc7538d.cont.ms',imagename='ngc7538d.cont',mode='channel',
      psfmode='hogbom',niter=5000,gain=0.1,threshold=3.,mask='',
      nchan=1,start=0,width=1,field='0',spw=0,
      imsize=[1024,1024],cell=[0.5,0.5],
      weighting='briggs',robust=0.5)
cleantime2 = time.time()
print '--Clean src line--'
default('clean')
clean(vis='ngc7538d.line.ms',imagename='ngc7538d.cube',mode='channel',
      psfmode='hogbom',niter=5000,gain=0.1,threshold=30.,mask='',
      nchan=48,start=2,width=1,field='0',spw=0,
      imsize=[128,128],cell=[4.,4.],
      weighting='briggs',robust=2.,
      uvtaper=True, outertaper=['12.0arcsec','12.0arcsec', '0deg'])
cleantime3 = time.time()
# -- Not done in old regression but should be
#print '--Contsub (image plane)--'
#ia.open('ngc7538d.cube.image')
#myim=ia.continuumsub('ngc7538d_subed.line.im','ngc7538d_res.cont.im',channels=range(0,48),fitorder=1)
#ia.close()
#myim.close()
#contsubtime=time.time()
#print '--View image--'
#viewer('ngc5921_task.image')

endProc = time.clock()
endTime = time.time()

# Regression

test_name_cal = 'NGC7538--Calibrater maximum amplitude test'
test_name_src = 'NGC7538--Source maximum amplitude test'
test_name_immax = 'NGC7538--Image maximum test'
test_name_imrms = 'NGC7538--Image rms test'

ms.open('ngc7538_cal.split.ms')
thistest_cal=max(ms.range(["amplitude"]).get('amplitude'))
ms.close()
ms.open('ngc7538d.line.ms')
thistest_src=max(ms.range(["amplitude"]).get('amplitude'))
ms.close()
ia.open('ngc7538d.cube.image')
# ia.statistics returns dictionary with 'return','statsout'
# get the second value in the dictionary (statsout)
statistics=ia.statistics()
# note thistest_immax will be a list with one value 
thistest_immax=statistics['max'][0]
# note thistest_imrms will be a list with one value 
thistest_imrms=statistics['rms'][0]

cal_max=2.413
src_max=18.3638
im_max=0.2606
im_rms=0.0127


diff_cal=abs((cal_max-thistest_cal)/cal_max)
diff_src=abs((src_max-thistest_src)/src_max)
diff_immax=abs((im_max-thistest_immax)/im_max)
diff_imrms=abs((im_rms-thistest_imrms)/im_rms)

import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='ngc7538.'+datestring+'.log'
logfile=open(outfile,'w')

print >>logfile,''
print >>logfile,'********** Data Summary *********'
print >>logfile,'* Observer: unavailable     Project: AP314                                  *'
print >>logfile,'* Observation: VLA(27 antennas)                                             *'
print >>logfile,'*  Telescope Observation Date Observer       Project                        *'
print >>logfile,'*  VLA       [                4.30759e+09, 4.30759e+09]unavailable    AP314 *'
print >>logfile,'*  VLA       [                4.30759e+09, 4.30762e+09]unavailable    AP314 *'
print >>logfile,'*  VLA       [                4.30762e+09, 4.30763e+09]unavailable    AP314 *'
print >>logfile,'*Data records: 838404       Total integration time = 36000 seconds          *'
print >>logfile,'*   Observed from   09:23:45   to   19:23:45                                *'
print >>logfile,'*Fields: 6                                                                  *'
print >>logfile,'*  ID   Name          Right Ascension  Declination   Epoch                  *'
print >>logfile,'*  0    1328+307      13:31:08.29      +30.30.33.04  J2000                  *'
print >>logfile,'*  1    2229+695      22:30:36.48      +69.46.28.00  J2000                  *'
print >>logfile,'*  2    NGC7538C      23:14:02.48      +61.27.14.86  J2000                  *'
print >>logfile,'*  3    NGC7538D      23:13:43.82      +61.27.00.18  J2000                  *'
print >>logfile,'*  4    NGC7538E      23:13:34.64      +61.27.26.44  J2000                  *'
print >>logfile,'*  5    NGC7538F      23:13:35.76      +61.28.33.66  J2000                  *'
print >>logfile,'* Data descriptions: 2 (2 spectral windows and 2 polarization setups)       *'
print >>logfile,'*   ID  #Chans Frame Ch1(MHz)    Resoln(kHz) TotBW(kHz)  Ref(MHz)    Corrs  *'
print >>logfile,'*   0       63 TOPO  23691.4682  118.164062  6152.34375  23694.4955  RR     *'
print >>logfile,'*   1       63 TOPO  23719.6063  118.164062  6152.34375  23722.6336  LL     *'
print >>logfile,'*********************************'
print >>logfile,''
print >>logfile,'********** Regression ***********'
print >>logfile,'*                               *'
if (diff_cal < 0.05): print >>logfile,'* Passed cal max amplitude test *'
print >>logfile,'* Cal max amp '+str(thistest_cal)
if (diff_src < 0.05): print >>logfile,'* Passed src max amplitude test *'
print >>logfile,'* Src max amp '+str(thistest_src)
if (diff_immax < 0.05): print >>logfile,'* Passed image max test         *'
print >>logfile,'* Image max '+str(thistest_immax)
if (diff_imrms < 0.05): print >>logfile,'* Passed image rms test         *'
print >>logfile,'* Image rms '+str(thistest_imrms)
if ((diff_src<0.05) & (diff_cal<0.05) & (diff_immax<0.05) & (diff_imrms<0.05)): 
	regstate=True
	print >>logfile,'---'
	print >>logfile,'Passed Regression test for NGC7538'
	print >>logfile,'---'
else: 
	regstate=False
	print >>logfile,'----FAILED Regression test for NGC7538'
print >>logfile,'*********************************'

print >>logfile,''
print >>logfile,''
print >>logfile,'********* Benchmarking *****************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
print >>logfile,'Processing rate MB/s  was: '+str(240.3/(endTime - startTime))
print >>logfile,'* Breakdown:                           *'
print >>logfile,'*   import       time was: '+str(importtime-startTime)
print >>logfile,'*   flagautocorr time was: '+str(flagtime-importtime)
print >>logfile,'*   setjy        time was: '+str(setjytime-flagtime)
print >>logfile,'*   gaincal      time was: '+str(gaintime-setjytime)
print >>logfile,'*   bandpass     time was: '+str(bptime-gaintime)
print >>logfile,'*   fluxscale    time was: '+str(fstime-bptime)
print >>logfile,'*   applycal     time was: '+str(correcttime-fstime)
print >>logfile,'*   split-cal    time was: '+str(splitcaltime-correcttime)
print >>logfile,'*   split-src    time was: '+str(splitsrctime-splitcaltime)
print >>logfile,'*   clean-cal    time was: '+str(cleantime1-splitsrctime)
print >>logfile,'*   clean-src-c  time was: '+str(cleantime2-cleantime1)
print >>logfile,'*   clean-src-l  time was: '+str(cleantime3-cleantime2)
#print '*   contsub      time was: ',contsubtime-cleantime3
print >>logfile,'*****************************************'
print >>logfile,'basho (test cpu) time was: 500 seconds'

logfile.close()

