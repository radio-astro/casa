###############################
#
# FLS3a HI Reduction Script
# Frequency-Switched data
# Formatted for wide screen (150
# characters wide)
#
###############################
#
import time
import os

os.system('rm -rf FLS3a_HI.image FLS3a_HI.asap FLS3b_HI.asap FLS3a_calfs')

casapath=os.environ['CASAPATH'].split()[0]
datapath=casapath+'/data/regression/ATST5/FLS3/FLS3_all_newcal_SP'
#copystring='cp -r '+datapath+' .'
#os.system(copystring)

startTime=time.time()
startProc=time.clock()

# Project: AGBT02A_007_01
# Observation: GBT(1 antennas)
# 
#   Telescope Observation Date    Observer       Project
#   GBT       [                   4.57539e+09, 4.5754e+09]Lockman        AGBT02A_007_01
#   GBT       [                   4.57574e+09, 4.57575e+09]Lockman        AGBT02A_007_02
#   GBT       [                   4.5831e+09, 4.58313e+09]Lockman        AGBT02A_031_12
# 
# Thu Feb 1 23:15:15 2007    NORMAL ms::summary:
# Data records: 76860       Total integration time = 7.74277e+06 seconds
#    Observed from   22:05:41   to   12:51:56
# 
# Thu Feb 1 23:15:15 2007    NORMAL ms::summary:
# Fields: 2
#   ID   Name          Right Ascension  Declination   Epoch
#   0    FLS3a         17:18:00.00      +59.30.00.00  J2000
#   1    FLS3b         17:18:00.00      +59.30.00.00  J2000
# 
# Thu Feb 1 23:15:15 2007    NORMAL ms::summary:
# Spectral Windows:  (2 unique spectral windows and 1 unique polarization setups)
#   SpwID  #Chans Frame Ch1(MHz)    Resoln(kHz) TotBW(kHz)  Ref(MHz)    Corrs
#   0        1024 LSRK  1421.89269  2.44140625  2500        1420.64269  XX  YY
#   1        1024 LSRK  1419.39269  2.44140625  2500        1418.14269  XX  YY


# FLS3 data calibration
# this is calibration part of FLS3 data
#

#Enable ASAP functionality by importing the library
casapath=os.environ['CASAPATH']
import asap as sd
os.environ['CASAPATH']=casapath

print '--Import--'
#Load MeasurementSet data into an ASAP scantable (this takes a while)
storage_sav=sd.rcParams['scantable.storage']
sd.rc('scantable',storage='disk')		# Note this enables handling of large datasets with limited memory
#s=sd.scantable('FLS3_all_newcal_SP',false)	# the 'false' indicates that no averaging should be done - this is
s=sd.scantable(datapath,average=false,getpt=false)	# the 'false' indicates that no averaging should be done - this is
						# always the case for data that hasn't been calibrated
importproc=time.clock()
importtime=time.time()

print '--Split & Save--'
# split out the data for the field of interest
s0=s.get_scan('FLS3a*')				# get all scans with FLS3a source
s0.save('FLS3a_HI.asap')			# save this data to an ASAP dataset on disk
del s						# delete scantables that will not be used any further
del s0
splitproc=time.clock()
splittime=time.time()

print '--Calibrate--'
s=sd.scantable('FLS3a_HI.asap',average=False)   # load in the saved ASAP dataset with FLS3a
s.set_fluxunit('K')				# set the fluxunit to 'K'; ASAP currently doesn't know about
						# the GBT and mislabels the data as 'Jy'
scanns = s.getscannos()				# get a list of the scan numbers in the scantable
sn=list(scanns)
print "No. scans to be processed:", len(scanns)
res=sd.calfs(s,sn)				# Do a frequency switched calibration on the scans
del s
calproc=time.clock()
caltime=time.time()

print '--Save calibrated data--'
res.save('FLS3a_calfs', 'MS2')			# Save the calibrated data to a MeasurementSet (CASA) format
del res
saveproc=time.clock()
savetime=time.time()

print '--Image data--'
#CASA									#AIPS++
im.open('FLS3a_calfs')			#set the data			# myim:=imager('FLS3a_calfs_v4')
im.selectvis(nchan=901,start=30,step=1,	#choose a subset of the dataa   # myim.setdata(mode='channel',start=30,
spw=0,field=0)							# step=1,nchan=901,fieldid=1,spwid=1)
dir='J2000 17:18:29 +59.31.23'		#set map center			# dir=dm.direction('17h18m29s','+59d31m23s')
im.defineimage(nx=150,cellx='1.5arcmin',#define image parameters	# myim.setimage(150,150,cellx='1.5arcmin',
phasecenter=dir,mode='channel',start=30, 				# celly='1.5arcmin',mode='channel',nchan=901,
nchan=901,step=1,spw=0)							# start=30,step=1,phasecenter=dir,doshift=T,
									# spwid=1)
# choose SD gridding, gridding cache size
im.setoptions(ftmachine='sd',cache=1000000000) 				# myim.setoptions(ftmachine='sd',gridfunction='SF')
im.setsdoptions(convsupport=4)						# myim.setsdoptions(convsupport=supportsize)
#make the image
im.makeimage(type='singledish',image='FLS3a_HI.image')			# myim.makeimage(image='test.image',type='singledish')
imagetime=time.time()
im.close()								# myim.close()
#ia.open('test.image')							# ia:=image('test.image')
#ia.setbrightnessunit('K')						# ia.setbrightnessunit('K')
#ia.close()								# ia.close()
imageproc=time.clock()
imagetime = time.time()

#
endProc = imageproc
endTime = imagetime
#
# -- end of FLS3 HI script
#
# Regression
ia.open('FLS3a_HI.image')
statistics=ia.statistics(list=True, verbose=True)
thistest_immax=statistics['max'][0]
thistest_imrms=statistics['rms'][0]
ia.close()

#
#hi_max=25.577
#hi_rms=1.013
# Regression values of CASA 2.3(#6654)+ASAP 2.2.0(#1448)
# on 64bit REL5.2 (2008/12/01)
hi_max=25.58
hi_rms=1.014

diff_immax=abs((hi_max-thistest_immax)/hi_max)
diff_imrms=abs((hi_rms-thistest_imrms)/hi_rms)

import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='fls3a.hi.'+datestring+'.log'
logfile=open(outfile,'w')


print >>logfile,''
print >>logfile,'********** Regression ***********'
print >>logfile,'*                               *'
if (diff_immax < 0.05): print '* Passed image max test '
print >>logfile,'*  Image max ',thistest_immax
if (diff_imrms < 0.05): print '* Passed image rms test '
print >>logfile,'*  Image rms ',thistest_imrms
if ((diff_immax<0.05) & (diff_imrms<0.05)): 
	regstate=True
        print >>logfile,'---'
        print >>logfile,'Passed Regression test for FLS3a HI'
        print >>logfile,'---'
	print ''
	print 'Regression PASSED'
	print ''
else: 
	regstate=False
	print ''
	print 'Regression FAILED'
	print ''
        print >>logfile,'----FAILED Regression test for FLS3a HI'
print >>logfile,'*********************************'
#
print >>logfile,''
print >>logfile,''
print >>logfile,'********* Benchmarking *****************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
print >>logfile,'Processing rate MB/s  was: '+str(4100/(endTime - startTime))
print >>logfile,'* Breakdown: '
print >>logfile,'*   import       time was: '+str(importtime-startTime)
print >>logfile,'*            CPU time was: '+str(importproc-startProc)
print >>logfile,'*   split        time was: '+str(splittime-importtime)
print >>logfile,'*            CPU time was: '+str(splitproc-importproc)
print >>logfile,'*   calibration  time was: '+str(caltime-splittime)
print >>logfile,'*            CPU time was: '+str(calproc-splitproc)
print >>logfile,'*   save         time was: '+str(savetime-caltime)
print >>logfile,'*            CPU time was: '+str(saveproc-calproc)
print >>logfile,'*   image        time was: '+str(imagetime-savetime)
print >>logfile,'*            CPU time was: '+str(imageproc-saveproc)
print >>logfile,'****************************************'


logfile.close()
### Resore the previous storage setting
sd.rc('scantable',storage=storage_sav)
