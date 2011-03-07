#####################################
#
# FLS3 HI SDtasks Regression Script
# Freq-switch OTF mapping data
# Version STM 2007-03-02
#
#####################################
import time
import os

casapath=os.environ['CASAPATH']

#os.system('rm -rf FLS3_all_newcal_SP sdregress_FLS3a_HI* ')
os.system('rm -rf sdregress_FLS3a_HI* ')
#datapath=casapath+'/data/regression/ATST5/FLS3/FLS3_all_newcal_SP'
datapath=casapath.split()[0]+'/data/regression/ATST5/FLS3/FLS3_all_newcal_SP'
#copystring='cp -r '+datapath+' .'
#os.system(copystring)

#restore()

##########################
#
# FLS3 HI 
# FS-OTF data
#
##########################
startTime=time.time()
startProc=time.clock()

importtime=time.time()
splittime=time.time()
print '--Import and Calibrate --'

storage_sav=sd.rcParams['scantable.storage']
sd.rc('scantable',storage='disk')               # Note this enables handling of large datasets with limited memory

# Set parameters
default('sdcal')
#sdfile = 'FLS3_all_newcal_SP'
sdfile = datapath
telescope = 'FIX'
fluxunit = 'K'
specunit = 'channel'
calmode = 'fsotf'
average=False
scanaverage = False
timeaverage = False
polaverage = False
tau = 0.0
scanlist = []
iflist = []
field = 'FLS3a*'
kernel = 'none'
blmode = 'none'
outfile = 'sdregress_FLS3a_HI.ms'
outform = 'ms'
plotlevel = 0

sdcal()
caltime=time.time()
savetime=time.time()
#
# WARNING: This tops out at 1.9GB in
# resident memory during the calibration
#

# Imaging using toolkit
im.open('sdregress_FLS3a_HI.ms')
# set up spectral channels for cube
im.selectvis(nchan=901,start=30,step=1,spw=0,field=0)
# set map center direction	
dir='J2000 17:18:29 +59.31.23'
#define image parameters
im.defineimage(nx=150,cellx='1.5arcmin',phasecenter=dir,mode='channel',start=30,nchan=901,step=1)
# choose SD gridding, gridding cache size
im.setoptions(ftmachine='sd',cache=1000000000)
im.setsdoptions(convsupport=4)
#make the image
im.makeimage(type='singledish',image='sdregress_FLS3a_HI.image')
imagetime=time.time()
#
# WARNING: the imaging peaks out at 1.8GB resident in memory
#

endProc = time.clock()
endTime = time.time()

# Regression
project = 'FLS3a-HI'
prolog  = 'sdregress.fls3a.hi'
# Compare to Joe's regression values
# hi_max=24.144
# hi_rms=0.957
#
# My new values: 25.577 1.0136
#
prev_max=25.577
prev_rms=1.0136

ia.open('sdregress_FLS3a_HI.image')
statistics=ia.statistics()
maxl = statistics['max'][0]
rmsl = statistics['rms'][0]
ia.close()
#
diff_max = abs((prev_max-maxl)/prev_max)
diff_rms = abs((prev_rms-rmsl)/prev_rms)

if (diff_max < 0.05): print '* Passed image max test '
if (diff_rms < 0.05): print '* Passed image rms test '
if ((diff_max<0.05) & (diff_rms<0.05)):
        print '---Passed Stat test for '+project
else:
        print '---FAILED Stat test for '+project
print ' '
print 'Total wall clock time was: '+str(endTime - startTime)
print 'Total CPU        time was: '+str(endProc - startProc)
print 'Processing rate MB/s  was: '+str(4100/(endTime - startTime))

#
# NOW TO REGRESSION LOGFILE 
#
import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile=prolog+datestring+'.log'
logfile=open(outfile,'w')

print >>logfile,''
print >>logfile,'************ Regression ****************'
print >>logfile,'*                                      *'
if (diff_max < 0.05): print >>logfile,'* Passed image max test '
print >>logfile,'*  Image max '+str(maxl)
if (diff_rms < 0.05): print >>logfile,'* Passed image rms test '
print >>logfile,'*  Image rms '+str(rmsl)
if ((diff_max<0.05) & (diff_rms<0.05)):
        print >>logfile,'---'
        print >>logfile,'Passed Stat test for '+project
else:
        print >>logfile,'---'
        print >>logfile,'FAILED Stat test for '+project
print >>logfile,'****************************************'
print >>logfile,''
print >>logfile,''
print >>logfile,'************ Benchmarking **************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
print >>logfile,'Processing rate MB/s  was: '+str(4100/(endTime - startTime))
print >>logfile,'* Breakdown: '
print >>logfile,'*   import       time was: '+str(importtime-startTime)
print >>logfile,'*   split        time was: '+str(splittime-importtime)
print >>logfile,'*   calibration  time was: '+str(caltime-splittime)
print >>logfile,'*   save         time was: '+str(savetime-caltime)
print >>logfile,'*   image        time was: '+str(imagetime-savetime)
print >>logfile,'*****************************************'

logfile.close()

### Resore the previous storage setting
sd.rc('scantable',storage=storage_sav)

##########################
#
# End FLS3-HI Regression
#
##########################
