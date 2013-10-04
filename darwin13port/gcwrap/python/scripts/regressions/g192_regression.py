###############################################
#                                             #
# Regression/Benchmarking Script for G192     #
#                                             #
###############################################

import time
import os

os.system('rm -rf g192*.ms g192_a* g192.*.im')

startTime = time.time()
startProc = time.clock()

print '--Import--'
datapath=os.environ.get('CASAPATH').split()[0] +'/data/regression/ATST1/G192/'

default('importvla')
importvla(archivefiles=[datapath + 'AS758_C030425.xp1',datapath+'AS758_C030425.xp2',datapath+'AS758_C030425.xp3',datapath+'AS758_C030426.xp4',datapath+'AS758_C030426.xp5'],
	  vis='g192_a.ms',bandname='K',frequencytol=10000000.0)
importtime = time.time() 
print '--Observation summary--'
default('listobs')
listobs(vis='g192_a.ms')
#listtime = time.time()
print '--Flag auto-correlations--'
default('tflagdata')
tflagdata(vis='g192_a.ms',mode='manual',autocorr=True)
flagtime = time.time()
print '--Setjy--'
default('setjy')
setjy(vis='g192_a.ms',field='4',scalebychan=False,standard='Perley-Taylor 99') #set flux density for 1331+305 (3C286)
setjytime = time.time()

print '--Gencal(opac)--'
# make opacity caltable
default('gencal')
gencal(vis='g192_a.ms',caltable='g192_a.opac',caltype='opac',parameter=[0.062])
gencaltime = time.time()

print '--Gaincal--'
#Select data for gain calibrators and drop outer channesl that may bias the solution
default('gaincal')
gaincal(vis='g192_a.ms',caltable='g192_a.gcal',
	field='0,2,3,4',spw='0:3~117', gaintype='G',
	solint='inf',combine='',refant='VA05',
	gaintable=['g192_a.opac'])
gaintime = time.time()
print '--Bandpass--'
#Select bandpass calibrator. Arrange to solve for a single solution over the entire observation 
default('bandpass')
bandpass(vis='g192_a.ms',caltable='g192_a.bcal',
	 field='3',  
	 gaintable=['g192_a.opac','g192_a.gcal'],gainfield=['','3'],interp=['','nearest'],
	 solint='inf',combine='scan',
	 refant='VA05')
bptime = time.time()
print '--Fluxscale--'
#Transfer the flux density scale from the flux calibrator to the gain calibrators
#Solutions are written to a table (g192_a.fluxcal) on disk.
default('fluxscale')
fluxscale(vis='g192_a.ms',caltable='g192_a.gcal',fluxtable='g192_a.fluxcal',
	  reference=['1331+305'],transfer=['0530+135','05309+13319'])
fstime = time.time()

print '--Correct--'
#Apply calibration solutions to the data
default('applycal')
applycal(vis='g192_a.ms',
	 field='0,1,2', 
	 gaintable=['g192_a.opac','g192_a.fluxcal','g192_a.bcal'],gainfield=['','0,2']);
correcttime = time.time()

print '--Split (Cal/src data)--'
#split out the data of interest into a new visibility data set
default('split')
split(vis='g192_a.ms',outputvis='g192_cal.split.ms',
 #     field=4,spw=0,nchan=100,start=9,step=1,datacolumn='CORRECTED_DATA')
	field='4',spw='0:9~108',datacolumn='corrected')
splitcaltime = time.time()
default('split')
split(vis='g192_a.ms',outputvis='g192_src.split.ms',
#      field=1,spw=0,nchan=100,start=9,step=1,datacolumn='CORRECTED_DATA')
	field='1',spw='0:9~108',datacolumn='corrected')
splitsrctime = time.time()

print '--Flag bad time range--'
#flag data in the specified time range for the source and spw
tflagdata(vis="g192_src.split.ms", field="0", spw="0", 
	  timerange="2003/04/26/02:45:00.0~2003/04/26/02:46:30.0")
flagsrctime=time.time()

print '--Clean src line--'
#image the source
default('clean')
clean(vis='g192_src.split.ms',imagename='g192_a2',mode='channel',
      psfmode='hogbom',niter=5000,gain=0.1,threshold=15.,mask='',
      nchan=40,start=34,width=1,field='0',spw='0',
      imsize=[512,512],cell=[.5,.5],weighting='natural')
cleantime = time.time()

print '--Write FITS--'
#export the data to fits
ia.open(infile='g192_a2.image')
ia.tofits(outfile='g192_a2.fits')
ia.close()
writefitstime=time.time()

print '--Contsub (image plane)--'
#do image plane continuum subtraction; channels specified will be used to make a continuum image: g192_cont.im
ia.open(infile='g192_a2.image')
myim=ia.continuumsub(outline='g192.line.im',outcont='g192.cont.im',
		     channels=range(0,1),fitorder=0)
x=myim.statistics(list=False, verbose=True)
thistest_con=x['rms'][0]
ia.close()
myim.close()
contsubtime=time.time()

endProc = time.clock()
endTime = time.time()

# Regression

test_name_cal = 'G192--Calibrater maximum amplitude test'
test_name_src = 'G192--Source maximum amplitude test'
#test_name_con = 'G192--Continuum subtraction rms test'
test_name_immax = 'G192--Image maximum test'
test_name_imrms = 'G192--Image rms test'

ms.open('g192_cal.split.ms')
thistest_cal=max(ms.range(["amplitude"]).get('amplitude'))
ms.close()
ms.open('g192_src.split.ms')
thistest_src=max(ms.range(["amplitude"]).get('amplitude'))
ms.close()
ia.open('g192_a2.image')
# ia.statistics returns dictionary with 'return','statsout'
# get the second value in the dictionary (statsout)
statistics=ia.statistics(list=True, verbose=True)
ia.close()
# note thistest_immax will be a list with one value 
thistest_immax=statistics['max']
# note thistest_imrms will be a list with one value 
thistest_imrms=statistics['rms']

calmax=2.7573
srcmax= 25.116
contsubrms= 0.00283678
immax=0.026332
imrms= 0.0020570
#data set size     = 634.9 MB - 5 VLA archive (xp) files

diff_cal=abs((calmax-thistest_cal)/calmax)
diff_src=abs((srcmax-thistest_src)/srcmax)
diff_con=abs((contsubrms-thistest_con)/contsubrms)
diff_immax=abs((immax-thistest_immax[0])/immax)
diff_imrms=abs((imrms-thistest_imrms[0])/imrms)

import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='g192.'+datestring+'.log'
logfile=open(outfile,'w')

print >>logfile,''
print >>logfile,'********** Data Summary *********'
print >>logfile,'*    Observer: unavailable     Project: AS758                               *'
print >>logfile,'* Observation: VLA(27 antennas)                                             *'
print >>logfile,'*   Telescope Observation Date    Observer       Project                    *'
print >>logfile,'*   VLA       [              4.55803e+09, 4.55803e+09]unavailable    AS758  *' 
print >>logfile,'*   VLA       [              4.55803e+09, 4.55803e+09]unavailable    AS758  *'       
print >>logfile,'*   VLA       [              4.55803e+09, 4.55803e+09]unavailable    AS758  *'      
print >>logfile,'*   VLA       [              4.55803e+09, 4.55804e+09]unavailable    AS758  *'     
print >>logfile,'*   VLA       [              4.55804e+09, 4.55804e+09]unavailable    AS758  *'     
print >>logfile,'* Data records: 1200015       Total integration time = 19347.5 seconds      *'
print >>logfile,'*    Observed from   25-Apr-2003/22:03:38   to   26-Apr-2003/03:26:05       *'
print >>logfile,'* Fields: 6                                                                 *'
print >>logfile,'*   ID   Name          Right Ascension  Declination   Epoch                 *'
print >>logfile,'*   0    0530+135      05:30:56.42      +13.31.55.15  J2000 (gaincal)       *'
print >>logfile,'*   1    05582+16320   05:58:13.53      +16.31.58.29  J2000 (target)        *'
print >>logfile,'*   2    05309+13319   05:30:56.42      +13.31.55.15  J2000 (gaincal)       *'
print >>logfile,'*   3    0319+415      03:19:48.16      +41.30.42.10  J2000 (bandpass)      *'
print >>logfile,'*   4    1331+305      13:31:08.29      +30.30.32.96  J2000 (fluxcal)       *'
print >>logfile,'*   6    KTIP          21:20:00.00      +60.00.00.00  J2000                 *'
print >>logfile,'* Data descriptions: 3 (3 spectral windows and 2 polarization setups)       *'
print >>logfile,'*   ID  #Chans Frame Ch1(MHz)    Resoln(kHz) TotBW(kHz)  Ref(MHz)    Corrs  *'
print >>logfile,'*   0      127 LSRK  23692.5072  24.4170056  3100.95971  23694.045   RR     *'
print >>logfile,'*   2        1 TOPO  22485.1     50000       50000       22485.1     RR  RL  LR  LL  '
print >>logfile,'*   3        1 TOPO  22435.1     50000       50000       22435.1     RR  RL  LR  LL  '
print >>logfile,'*********************************'
print >>logfile,''
print >>logfile,'********** Regression ***********'
print >>logfile,'*                               *'

regstate=True
if (diff_cal < 0.05):
	print >>logfile,'* Passed cal max amplitude test *'
else:
	regstate=False
	print >>logfile,'* Failed cal max amplitude test *'
print >>logfile,'   Cal max amp '+str(thistest_cal)+' ('+str(calmax)+')'

if (diff_src < 0.05):
	print >>logfile,'* Passed src max amplitude test *'
else:
	regstate=False
	print >>logfile,'* Failed src max amplitude test *'
print >>logfile,'   Src max amp '+str(thistest_src)+' ('+str(srcmax)+')'

if (diff_con < 0.05):
	print >>logfile,'* Passed contsub rms test         *'
else:
	regstate=False
	print >>logfile,'* Failed contsub rms test         *'
print >>logfile,'   Contsub rms '+str(thistest_con)+' ('+str(contsubrms)+')'

if (diff_immax < 0.05):
	print >>logfile,'* Passed image max test         *'
else:
	regstate=False
	print >>logfile,'* Failed image max test         *'
print >>logfile,'   Image max '+str(thistest_immax)+' ('+str(immax)+')'

if (diff_imrms < 0.05):
	print >>logfile,'* Passed image rms test         *'
else:
	regstate=False
	print >>logfile,'* Failed image rms test         *'
print >>logfile,'   Image rms '+str(thistest_imrms)+' ('+str(imrms)+')'


if (regstate):
	print >>logfile,'---'
	print >>logfile,'Passed Regression test for G192'
	print >>logfile,'---'
else: 
	print >>logfile,'----FAILED Regression test for G192'
print >>logfile,'*********************************'

print >>logfile,''
print >>logfile,''
print >>logfile,'********* Benchmarking *****************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
print >>logfile,'Processing rate MB/s  was: '+str(634.9/(endTime - startTime))
print >>logfile,'* Breakdown:                           *'
print >>logfile,'*   import       time was: '+str(importtime-startTime)
print >>logfile,'*   tflagdata    time was: '+str(flagtime-importtime)
print >>logfile,'*   setjy        time was: '+str(setjytime-flagtime)
print >>logfile,'*   gencal       time was: '+str(gencaltime-setjytime)
print >>logfile,'*   gaincal      time was: '+str(gaintime-gencaltime)
print >>logfile,'*   bandpass     time was: '+str(bptime-gaintime)
print >>logfile,'*   fluxscale    time was: '+str(fstime-bptime)
print >>logfile,'*   applycal     time was: '+str(correcttime-fstime)
print >>logfile,'*   split-cal    time was: '+str(splitcaltime-correcttime)
print >>logfile,'*   split-src    time was: '+str(splitsrctime-splitcaltime)
print >>logfile,'*   flag-src     time was: '+str(flagsrctime-splitsrctime)
print >>logfile,'*   clean-src    time was: '+str(cleantime-flagsrctime)
#print '*   exportfits   tiem was: ',exportfitstime-cleantime
#print '*   contsub      time was: ',contsubtime-exportfitstime
print >>logfile,'*****************************************'
print >>logfile,'basho (test cpu) time was: 1500 seconds'

logfile.close()
