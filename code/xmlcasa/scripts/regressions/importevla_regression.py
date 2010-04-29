##########################################################################
#                                                                        #
# Script for EVLA Widar Data                                             #
#                                                                        #
# Updated      STM 2010-04-23 (Release R0.2)     fill/export test        #
#                                                                        #
##########################################################################

import time
import os
import pickle

print 'Fill EVLA data'
scriptvers = 'Version 2010-04-23 (CASA 3.0.2)'
print scriptvers
print ''

# This script may have some interactive commands: scriptmode = True
# if you are running it and want it to stop during interactive parts.
scriptmode = False

prefix = 'importevla_regression'

# Clean up old files
print 'Removing old files'
os.system('rm -rf '+prefix+'.*')
print 'Setting up control parameters'
# 
#=====================================================================
# SET UP THE SCRIPT CONTROL PARAMETERS HERE
#=====================================================================
sdmpath = './'
sdmpath=os.environ.get('CASAPATH').split()[0]+'/data/regression/evla/'
sdmfile = 'TOSR0001_sb1308595_1.55294.83601028935'
msfile = prefix+'.ms'

# importevla controls
doapply = True
dozero = False
doshadow = False

# exportuvfits controls
exportspw = '0,1'   # Q band only

#---------------------------------------------------------
#Open an output logfile
#---------------------------------------------------------
import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())

outfile='out.'+prefix+'.'+datestring+'.log'
logfile=open(outfile,'w')
print 'Opened log file '+outfile

print >>logfile,'Processing SDM data from '+sdmfile
print >>logfile,'Script '+scriptvers

# Some date and version info
myvers = casalog.version()
myuser = os.getenv('USER')
myhost = str( os.getenv('HOST') )
mycwd = os.getcwd()
myos = os.uname()

# Print version info to outfile
print >>logfile,'Running '+myvers+' on host '+myhost
print >>logfile,'at '+datestring
print >>logfile,''

#
stagename = []
stagetime = []

startTime=time.time()
startProc=time.clock()

prevTime = startTime

#---------------------------------------------------------
#Fill from SDM and split from filled MS
#---------------------------------------------------------

print '---ImportEVLA---'
print 'This is a EVLA dataset, running importevla'
default('importevla')
asdm = sdmpath+sdmfile
vis = msfile
applyflags = doapply
flagzero = dozero
shadow = doshadow
verbose = True
importevla()
verbose = False

currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('import')
fillTime = currTime-prevTime
prevTime = currTime

# get dataset sizes in MB (1024x1024 bytes!)
f = os.popen('du -ms '+asdm)
fstr = f.readline()
f.close()
sdmsize = float( fstr.split("\t")[0] )

f = os.popen('du -ms '+msfile)
fstr = f.readline()
f.close()
fillsize = float( fstr.split("\t")[0] )

#---------------------------------------------------------
# MS statistics
#---------------------------------------------------------
print '---Listobs---'
listobs(vis=msfile,verbose=True)

currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('listobs')
fillTime = currTime-prevTime
prevTime = currTime

print '---Visstat---'
myvisstat = visstat(vis=msfile,useflags=True)

n_vis = int( myvisstat['DATA']['npts'] )
ampmax = myvisstat['DATA']['max']
ampmean = myvisstat['DATA']['mean']

# Values from CASA Version 3.0.2 (build #11181) 2010-04-23
n_vis_prev = 99737600
ampmax_prev = 1.22975087166
ampmean_prev = 0.00687524320367

print ' MS unflagged npts = ',n_vis,' ( prev = ',n_vis_prev,' )  frac diff = ',(n_vis-n_vis_prev)/float(n_vis)
print >>logfile,' MS unflagged npts = ',n_vis,' ( prev = ',n_vis_prev,' )  frac diff = ',(n_vis-n_vis_prev)/float(n_vis)

print ' MS unflagged vis max = ',ampmax,' ( prev = ',ampmax_prev,' )  frac diff = ',(ampmax-ampmax_prev)/ampmax
print >>logfile,' MS unflagged vis max = ',ampmax,' ( prev = ',ampmax_prev,' )  frac diff = ',(ampmax-ampmax_prev)/ampmax

print ' MS unflagged mean = ',ampmean,' ( prev = ',ampmean_prev,' )  frac diff = ',(ampmean-ampmean_prev)/ampmean
print >>logfile,' MS unflagged mean = ',ampmean,' ( prev = ',ampmean_prev,' )  frac diff = ',(ampmean-ampmean_prev)/ampmean

currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('visstat')
fillTime = currTime-prevTime
prevTime = currTime

#---------------------------------------------------------
#Export as UVFITS
#---------------------------------------------------------
print '---ExportUVFITS---'
default('exportuvfits')
vis = msfile
fitsfile = msfile+'.uvfits'
os.system('rm -rf '+fitsfile)
async = False
writestation = True
# For AIPS compatibility
combinespw = True
padwithflags = False
# Choose relevant spw
spw = exportspw
print '  Exporting spw '+exportspw+' to UVFITS '+fitsfile
print >>logfile,'  Exporting spw '+exportspw+' to UVFITS '+fitsfile
exportuvfits()
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('exportuvfits')
exportTime = currTime-prevTime
prevTime = currTime

# get dataset sizes in MB (1024x1024 bytes!)
f = os.popen('du -ms '+fitsfile)
fstr = f.readline()
f.close()
fitssize = float( fstr.split("\t")[0] )

# Done
endProc=time.clock()
endTime=time.time()
print ''
print 'Total wall clock time was: '+str(endTime - startTime)
print 'Total CPU        time was: '+str(endProc - startProc)
print >>logfile,''
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)

datasize_raw =  sdmsize
datasize_fill = fillsize          # (after fill)
datasize_fits = fitssize          # (after export)
print ''
print 'SDM datasize (MB)               : '+str(datasize_raw)
print 'Filled MS datasize (MB)         : '+str(datasize_fill)
print 'Export UVFITS datasize (MB)     : '+str(datasize_fits)
print 'Reported sizes are in MB from "du -ms" (1024x1024 bytes)'
print >>logfile,''
print >>logfile,'SDM datasize (MB)               : '+str(datasize_raw)
print >>logfile,'Filled MS datasize (MB)         : '+str(datasize_fill)
print >>logfile,'Export UVFITS datasize (MB)     : '+str(datasize_fits)
print >>logfile,'Reported sizes are in MB from "du -ms" (1024x1024 bytes)'

# Save timing to regression dictionary
timing = {}

total = {}
total['wall'] = (endTime - startTime)
total['cpu'] = (endProc - startProc)
total['rate_raw'] = (datasize_raw/(endTime - startTime))
total['rate_ms'] = (datasize_fill/(endTime - startTime))

nstages = stagetime.__len__()
timing['total'] = total
timing['nstages'] = nstages
timing['stagename'] = stagename
timing['stagetime'] = stagetime

print ''
print 'Net SDMtoMS filling I/O rate MB/s   : '+str((datasize_raw+datasize_fill)/fillTime)
print 'Net MStoUVFITS export I/O rate MB/s : '+str((datasize_fill+datasize_fits)/exportTime)
print ''
print '* Breakdown:                               *'
print >>logfile,''
print >>logfile,'Net SDMtoMS filling I/O rate MB/s   : '+str((datasize_raw+datasize_fill)/fillTime)
print >>logfile,'Net MStoUVFITS export I/O rate MB/s : '+str((datasize_fill+datasize_fits)/exportTime)
print >>logfile,''
print >>logfile,'* Breakdown:                               *'

for i in range(nstages):
    print '* %40s * time was: %10.3f ' % (stagename[i],stagetime[i])
    print >>logfile,'* %40s * time was: %10.3f ' % (stagename[i],stagetime[i])

logfile.close()

# End of script
#
#================================================================================
# MeasurementSet Name:  test_importevla.ms      MS Version 2
#================================================================================
#   Observer: TOSR0001_sb1308595_1     Project: T.B.D.  
#Observation: EVLA
#Data records: 466050       Total integration time = 716 seconds
#   Observed from   08-Apr-2010/20:03:53.5   to   08-Apr-2010/20:15:49.5 (UTC)
#
#   ObservationID = 0         ArrayID = 0
#  Date        Timerange (UTC)          Scan  FldId FieldName    nVis   Int(s)   SpwIds
#  08-Apr-2010/20:03:53.5 - 20:04:50.5     1      0 J1800+7828   37700  1        [0, 1]
#              20:04:51.5 - 20:05:50.5     2      0 J1800+7828   39000  1        [2, 3]
#              20:05:51.5 - 20:05:55.5     3      1 J1800+7828   3250   1        [2, 3]
#              20:05:56.5 - 20:10:49.5     4      1 J1800+7828   191100 1        [2, 3]
#              20:10:50.5 - 20:11:49.5     5      0 J1800+7828   39000  1        [0, 1]
#              20:11:50.5 - 20:12:49.5     6      0 J1800+7828   39000  1        [0, 1]
#              20:12:50.5 - 20:13:49.5     7      0 J1800+7828   39000  1        [0, 1]
#              20:13:50.5 - 20:14:49.5     8      0 J1800+7828   39000  1        [0, 1]
#              20:14:50.5 - 20:15:49.5     9      0 J1800+7828   39000  1        [0, 1]
#           (nVis = Total number of time/baseline visibilities per scan) 
#Fields: 2
#  ID   Code Name         RA            Decl           Epoch   SrcId nVis   
#  0    I    J1800+7828   18:00:45.6839 +78.28.04.0185 J2000   0     271700 
#  1    Z    J1800+7828   18:00:45.6839 +78.28.04.0185 J2000   1     194350 
#   (nVis = Total number of time/baseline visibilities per field) 
#Spectral Windows:  (4 unique spectral windows and 1 unique polarization setups)
#  SpwID  #Chans Frame Ch1(MHz)    ChanWid(kHz)TotBW(kHz)  Ref(MHz)    Corrs           
#  0          64 TOPO  46936       2000        128000      46936       RR  RL  LR  LL  
#  1          64 TOPO  46936       2000        128000      46936       RR  RL  LR  LL  
#  2          64 TOPO  4960        2000        128000      4960        RR  RL  LR  LL  
#  3          64 TOPO  5088        2000        128000      5088        RR  RL  LR  LL  
#Sources: 6
#  ID   Name         SpwId RestFreq(MHz)  SysVel(km/s) 
#  0    J1800+7828   0     -              -            
#  0    J1800+7828   1     -              -            
#  0    J1800+7828   2     -              -            
#  0    J1800+7828   3     -              -            
#  1    J1800+7828   2     -              -            
#  1    J1800+7828   3     -              -            
#Antennas: 26:
#  ID   Name  Station   Diam.    Long.         Lat.         
#  0    ea01  W09       25.0 m   -107.37.25.2  +33.53.51.0  
#  1    ea02  E02       25.0 m   -107.37.04.4  +33.54.01.1  
#  2    ea03  E09       25.0 m   -107.36.45.1  +33.53.53.6  
#  3    ea04  W01       25.0 m   -107.37.05.9  +33.54.00.5  
#  4    ea05  W08       25.0 m   -107.37.21.6  +33.53.53.0  
#  5    ea06  N03       25.0 m   -107.37.06.3  +33.54.04.8  
#  6    ea07  N06       25.0 m   -107.37.06.9  +33.54.10.3  
#  7    ea08  N01       25.0 m   -107.37.06.0  +33.54.01.8  
#  8    ea09  E06       25.0 m   -107.36.55.6  +33.53.57.7  
#  9    ea11  E04       25.0 m   -107.37.00.8  +33.53.59.7  
#  10   ea13  N07       25.0 m   -107.37.07.2  +33.54.12.9  
#  11   ea14  E05       25.0 m   -107.36.58.4  +33.53.58.8  
#  12   ea15  W06       25.0 m   -107.37.15.6  +33.53.56.4  
#  13   ea16  W02       25.0 m   -107.37.07.5  +33.54.00.9  
#  14   ea17  W07       25.0 m   -107.37.18.4  +33.53.54.8  
#  15   ea18  N09       25.0 m   -107.37.07.8  +33.54.19.0  
#  16   ea19  W04       25.0 m   -107.37.10.8  +33.53.59.1  
#  17   ea20  N05       25.0 m   -107.37.06.7  +33.54.08.0  
#  18   ea21  E01       25.0 m   -107.37.05.7  +33.53.59.2  
#  19   ea22  N04       25.0 m   -107.37.06.5  +33.54.06.1  
#  20   ea23  E07       25.0 m   -107.36.52.4  +33.53.56.5  
#  21   ea24  W05       25.0 m   -107.37.13.0  +33.53.57.8  
#  22   ea25  N02       25.0 m   -107.37.06.2  +33.54.03.5  
#  23   ea26  W03       25.0 m   -107.37.08.9  +33.54.00.1  
#  24   ea27  E03       25.0 m   -107.37.02.8  +33.54.00.5  
#  25   ea28  N08       25.0 m   -107.37.07.5  +33.54.15.8  
