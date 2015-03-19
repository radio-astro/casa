# Regression Tests
# Masaya Kuniyoshi
#
# svn info
#
# Path: .
# URL: https://svn.cv.nrao.edu/svn/casa/trunk
# Repository Root: https://svn.cv.nrao.edu/svn/casa
# Repository UUID: ac9c0da5-131f-0410-9009-a9c195f183e6
# Revision: 31543
# Node Kind: directory
# Schedule: normal
# Last Changed Author: kumar.golap
# Last Changed Rev: 31542
# Last Changed Date: 2014-11-07 08:56:52 +0900 (Fri, 07 Nov 2014)



import time
import os
import filltsys
import numpy
from recipes.almahelpers import tsysspwmap
import shutil

casapath = os.environ['CASAPATH']
datapath = casapath.split()[0]+'/data/regression/alma-sd/M100/uid___A002_X6218fb_X264'
print datapath
shutil.copytree(datapath, 'uid___A002_X6218fb_X264') 



startTime=time.time()
startProc=time.clock()



# Check CASA Version #

version = casadef.casa_version

print "You are using " + version

if (version < '4.3.0'):
    print "YOUR VERSION OF CASA IS TOO OLD FOR THIS GUIDE."
    print "PLEASE UPDATE IT BEFORE PROCEEDING."
else:
    print "Your version of CASA is appropriate for this guide."




# ASDM Data #

basename = ['uid___A002_X6218fb_X264']


# Create Measurement Sets from ASDM Data ##
# importasdm func converts ASDM format to MS format

print '--Import--'

for name in basename:
    importasdm(asdm = name)

importproc=time.clock()
importtime=time.time()

# listobs task generates detailed information of the MS
# ~.listobs.txt

for name in basename:
    default(listobs)
    vis = name + '.ms'
    listfile = name + '.ms.listobs.txt'
    listobs()



# Initial inspection of the data with plotms task. 
# First plot amplitude versus channel, 
# averaging over time in order to speed up the plotting process.

default(plotms)
vis   = 'uid___A002_X6218fb_X264.ms'
xaxis ='channel'
yaxis ='amp'
averagedata = T
avgtime = '1e8'
avgscan = T
iteraxis = 'antenna'
plotms()



# Convert MS to ASAP format #

print '--Split & Save--'

default(sdsave)
infile= 'uid___A002_X6218fb_X264.ms'
splitant = True
#outform = 'asap'
sdsave()

split_save_proc=time.clock()
split_save_time=time.time()


default(sdlist)
infile  = 'uid___A002_X6218fb_X264.ms_saved.PM03'
outfile = 'uid___A002_X6218fb_X264.ms_saved.PM03.sdlist'
sdlist()


default(sdlist)
infile  = 'uid___A002_X6218fb_X264.ms_saved.PM04'
outfile = 'uid___A002_X6218fb_X264.ms_saved.PM04.sdlist'
sdlist()


default(sdlist)
infile  = 'uid___A002_X6218fb_X264.ms_saved.CM03'
outfile = 'uid___A002_X6218fb_X264.ms_saved.CM03.sdlist'
sdlist()

default(sdlist)
infile  = 'uid___A002_X6218fb_X264.ms_saved.CM05'
outfile = 'uid___A002_X6218fb_X264.ms_saved.CM05.sdlist'
sdlist()



# View Spectra #

default(sdplot)
infile = 'uid___A002_X6218fb_X264.ms_saved.PM03'
spw = '15'
scanaverage = True
plottype = 'spectra'
stack = 'pol'
panel = 'scan'
outfile = 'uid___A002_X6218fb_X264.ms_saved.PM03.png'
overwrite = True
sdplot()

default(sdplot)
infile = 'uid___A002_X6218fb_X264.ms_saved.PM04'
spw = '15'
scanaverage = True
plottype = 'spectra'
stack = 'pol'
panel = 'scan'
outfile = 'uid___A002_X6218fb_X264.ms_saved.PM04.png'
overwrite = True
sdplot()

default(sdplot)
infile = 'uid___A002_X6218fb_X264.ms_saved.CM03'
spw = '15'
scanaverage = True
plottype = 'spectra'
stack = 'pol'
panel = 'scan'
outfile = 'uid___A002_X6218fb_X264.ms_saved.CM03.png'
overwrite = True
sdplot()

default(sdplot)
infile = 'uid___A002_X6218fb_X264.ms_saved.CM05'
spw = '15'
scanaverage = True
plottype = 'spectra'
stack = 'pol'
panel = 'scan'
outfile = 'uid___A002_X6218fb_X264.ms_saved.CM05.png'
overwrite = True
sdplot()



# Flagging #
print '--Flagging--'

default(sdflag)
infile = 'uid___A002_X6218fb_X264.ms_saved.PM03'
spw = '9; 11; 13; 15: 0~119;3960~4079'
overwrite = True
sdflag()

default(sdflag)
infile = 'uid___A002_X6218fb_X264.ms_saved.PM04'
spw = '9; 11; 13; 15: 0~119;3960~4079'
overwrite = True
sdflag()

default(sdflag)
infile = 'uid___A002_X6218fb_X264.ms_saved.CM03'
spw = '9; 11; 13; 15: 0~119;3960~4079'
overwrite = True
sdflag()

default(sdflag)
infile = 'uid___A002_X6218fb_X264.ms_saved.CM05'
spw = '9; 11; 13; 15: 0~119;3960~4079'
overwrite = True
sdflag()


flagproc = time.clock()
flagtime = time.time()


# Apply Calibration and Inspect #

print '--Calibration sdcal2--'

default(sdcal2)
infile  = 'uid___A002_X6218fb_X264.ms_saved.PM03'
calmode = 'ps,tsys,apply'
tsysspw = '1,3,5,7'
spwmap  = {'1':[9],'3':[11],'5':[13],'7':[15]}
outfile = 'uid___A002_X6218fb_X264.ms_saved.PM03.cal'
overwrite = True
sdcal2()

default(sdcal2)
infile  = 'uid___A002_X6218fb_X264.ms_saved.PM04'
calmode = 'ps,tsys,apply'
tsysspw = '1,3,5,7'
spwmap  = {'1':[9],'3':[11],'5':[13],'7':[15]}
outfile = 'uid___A002_X6218fb_X264.ms_saved.PM04.cal'
overwrite = True
sdcal2()

default(sdcal2)
infile  = 'uid___A002_X6218fb_X264.ms_saved.CM03'
calmode = 'ps,tsys,apply'
tsysspw = '1,3,5,7'
spwmap  = {'1':[9],'3':[11],'5':[13],'7':[15]}
outfile = 'uid___A002_X6218fb_X264.ms_saved.CM03.cal'
overwrite = True
sdcal2()

default(sdcal2)
infile  = 'uid___A002_X6218fb_X264.ms_saved.CM05'
calmode = 'ps,tsys,apply'
tsysspw = '1,3,5,7'
spwmap  = {'1':[9],'3':[11],'5':[13],'7':[15]}
outfile = 'uid___A002_X6218fb_X264.ms_saved.CM05.cal'
overwrite = True
sdcal2()


sdcal2proc=time.clock()
sdcal2time=time.time()



# Baseline Subtraction and Inspect #

print '--Caribration Baseline --'

default(sdbaseline)
infile = 'uid___A002_X6218fb_X264.ms_saved.PM03.cal'
spw = '9, 11, 13, 15'
maskmode = 'auto'
thresh = 3.0
avg_limit = 8
blfunc = 'poly'
order = 1
outfile = 'uid___A002_X6218fb_X264.ms_saved.PM03.cal.bl'
overwrite = True
sdbaseline()

default(sdbaseline)
infile = 'uid___A002_X6218fb_X264.ms_saved.PM04.cal'
spw = '9, 11, 13, 15'
maskmode = 'auto'
thresh = 3.0
avg_limit = 8
blfunc = 'poly'
order = 1
outfile = 'uid___A002_X6218fb_X264.ms_saved.PM04.cal.bl'
overwrite = True
sdbaseline()

default(sdbaseline)
infile = 'uid___A002_X6218fb_X264.ms_saved.CM03.cal'
spw = '9, 11, 13, 15'
maskmode = 'auto'
thresh = 3.0
avg_limit = 8
blfunc = 'poly'
order = 1
outfile = 'uid___A002_X6218fb_X264.ms_saved.CM03.cal.bl'
overwrite = True
sdbaseline()

default(sdbaseline)
infile = 'uid___A002_X6218fb_X264.ms_saved.CM05.cal'
spw = '9, 11, 13, 15'
maskmode = 'auto'
thresh = 3.0
avg_limit = 8
blfunc = 'poly'
order = 1
outfile = 'uid___A002_X6218fb_X264.ms_saved.CM05.cal.bl'
overwrite = True
sdbaseline()


sdbaselineproc = time.clock()
sdbaselinetime = time.time()


os.system('rm -rf uid___A002_X6218fb_X264.ms_saved.PM03.cal.plots')
os.system('rm -rf uid___A002_X6218fb_X264.ms_saved.PM04.cal.plots')
os.system('rm -rf uid___A002_X6218fb_X264.ms_saved.CM03.cal.plots')
os.system('rm -rf uid___A002_X6218fb_X264.ms_saved.CM05.cal.plots')

os.system('mkdir uid___A002_X6218fb_X264.ms_saved.PM03.cal.plots')
os.system('mkdir uid___A002_X6218fb_X264.ms_saved.PM04.cal.plots')
os.system('mkdir uid___A002_X6218fb_X264.ms_saved.CM03.cal.plots')
os.system('mkdir uid___A002_X6218fb_X264.ms_saved.CM05.cal.plots')



# Plot the calibrated spectra, using the sdplot task. 
# The commands below will plot one spectrum per scan, spw and polarization. 

for i in [9, 11, 13, 15]:

    default(sdplot)
    infile = 'uid___A002_X6218fb_X264.ms_saved.PM03.cal'
    spw = str(i)
    plottype = 'spectra'
    stack = 'pol'
    panel = 'scan'
    outfile = 'uid___A002_X6218fb_X264.ms_saved.PM03.cal.plots/uid___A002_X6218fb_X264.ms_saved.PM03.cal.spectra.spw'+str(i)+'.png'
    overwrite = True
    sdplot()

    default(sdplot)
    infile = 'uid___A002_X6218fb_X264.ms_saved.PM04.cal'
    spw = str(i)
    plottype = 'spectra'
    stack = 'pol'
    panel = 'scan'
    outfile = 'uid___A002_X6218fb_X264.ms_saved.PM04.cal.plots/uid___A002_X6218fb_X264.ms_saved.PM04.cal.spectra.spw'+str(i)+'.png'
    overwrite = True
    sdplot()


for i in [9, 11, 13, 15]:

    default(sdplot)
    infile = 'uid___A002_X6218fb_X264.ms_saved.CM03.cal'
    spw = str(i)
    plottype = 'spectra'
    stack = 'pol'
    panel = 'scan'
    outfile = 'uid___A002_X6218fb_X264.ms_saved.CM03.cal.plots/uid___A002_X6218fb_X264.ms_saved.CM03.cal.spectra.spw'+str(i)+'.png'
    overwrite = True
    sdplot()

    default(sdplot)
    infile = 'uid___A002_X6218fb_X264.ms_saved.CM05.cal'
    spw = str(i)
    plottype = 'spectra'
    stack = 'pol'
    panel = 'scan'
    outfile = 'uid___A002_X6218fb_X264.ms_saved.CM05.cal.plots/uid___A002_X6218fb_X264.ms_saved.CM05.cal.spectra.spw'+str(i)+'.png'
    overwrite = True
    sdplot()







# View Baselined Spectra with the sdplot task #

default(sdplot)
infile = 'uid___A002_X6218fb_X264.ms_saved.PM03.cal.bl'
spw = '15'
specunit = 'GHz'
scanaverage = True
stack = 'pol'
panel = 'scan'
outfile = 'uid___A002_X6218fb_X264.ms_saved.PM03.cal.bl.png'
overwrite = True
sdplot()

default(sdplot)
infile = 'uid___A002_X6218fb_X264.ms_saved.PM04.cal.bl'
spw = '15'
specunit = 'GHz'
scanaverage = True
stack = 'pol'
panel = 'scan'
outfile = 'uid___A002_X6218fb_X264.ms_saved.PM04.cal.bl.png'
overwrite = True
sdplot()

default(sdplot)
infile = 'uid___A002_X6218fb_X264.ms_saved.CM03.cal.bl'
spw = '15'
specunit = 'GHz'
scanaverage = True
stack = 'pol'
panel = 'scan'
outfile = 'uid___A002_X6218fb_X264.ms_saved.CM03.cal.bl.png'
overwrite = True
sdplot()

default(sdplot)
infile = 'uid___A002_X6218fb_X264.ms_saved.CM05.cal.bl'
spw = '15'
specunit = 'GHz'
scanaverage = True
stack = 'pol'
panel = 'scan'
outfile = 'uid___A002_X6218fb_X264.ms_saved.CM05.cal.bl.png'
overwrite = True
sdplot()



# Convert ASAP to MS #

print '--Save calibrated data (from asap to MS2)--'

default(sdsave)
infile = 'uid___A002_X6218fb_X264.ms_saved.PM03.cal.bl'
outfile = 'uid___A002_X6218fb_X264.ms_saved.PM03.cal.bl.ms'
outform = 'MS2'
sdsave()

default(sdsave)
infile = 'uid___A002_X6218fb_X264.ms_saved.PM04.cal.bl'
outfile = 'uid___A002_X6218fb_X264.ms_saved.PM04.cal.bl.ms'
outform = 'MS2'
sdsave()

default(sdsave)
infile = 'uid___A002_X6218fb_X264.ms_saved.CM03.cal.bl'
outfile = 'uid___A002_X6218fb_X264.ms_saved.CM03.cal.bl.ms'
outform = 'MS2'
sdsave()

default(sdsave)
infile = 'uid___A002_X6218fb_X264.ms_saved.CM05.cal.bl'
outfile = 'uid___A002_X6218fb_X264.ms_saved.CM05.cal.bl.ms'
outform = 'MS2'
sdsave()


saveproc=time.clock()
savetime=time.time()


# Combine all MSs to one MS #

print '--Combine MSs to one MS--'

# PM #
default(sdimaging)
infiles = ['uid___A002_X6218fb_X264.ms_saved.PM03.cal.bl.ms', 'uid___A002_X6218fb_X264.ms_saved.PM04.cal.bl.ms']
field='0'
spw='15'
restfreq='115.271204GHz'
nchan=70
start=1800
width=5
gridfunction='gjinc'
imsize=[50,50]
cell=['10arcsec','10arcsec']
phasecenter = 'J2000 12h22m54.9 +15d49m15'
outfile='M100_SD_cube_PM_03_04.image'
sdimaging()

# CM #
default(sdimaging)
infiles = ['uid___A002_X6218fb_X264.ms_saved.CM03.cal.bl.ms', 'uid___A002_X6218fb_X264.ms_saved.CM05.cal.bl.ms']
field='0'
spw='15'
restfreq='115.271204GHz'
nchan=70
start=1800
width=5
gridfunction='gjinc'
imsize=[50,50]
cell=['10arcsec','10arcsec']
phasecenter = 'J2000 12h22m54.9 +15d49m15'
outfile='M100_SD_cube_CM_03_05.image'
sdimaging()


combproc=time.clock()
combtime=time.time()



## Image Analysis : Moment Maps

os.system('rm -rf M100_SD_cube_PM_03_04.image.mom*')
immoments(imagename = 'M100_SD_cube_PM_03_04.image',moments = [0],axis = 'spectral',chans = '1~24',outfile = 'M100_SD_cube_PM_03_04.image.mom0')
#immoments(imagename = 'M100_SD_cube_PM_03_04.image',moments = [1],axis = 'spectral',chans = '1~24',outfile = 'M100_SD_cube_PM_03_04.image.mom1')

os.system('rm -rf M100_SD_cube_CM_03_05.image.mom*')
immoments(imagename = 'M100_SD_cube_CM_03_05.image',moments = [0],axis = 'spectral',chans = '1~24',outfile = 'M100_SD_cube_CM_03_05.image.mom0')
#immoments(imagename = 'M100_SD_cube_CM_03_05.image',moments = [1],axis = 'spectral',chans = '1~24',outfile = 'M100_SD_cube_CM_03_05.image.mom1')



## Export data as fits  

os.system('rm -rf M100_SD_*.fits')
exportfits(imagename='M100_SD_cube_PM_03_04.image', fitsimage='M100_SD_cube_PM_03_04.image.fits')
exportfits(imagename='M100_SD_cube_PM_03_04.image.mom0', fitsimage='M100_SD_cube_PM_03_04.image.mom0.fits')
#exportfits(imagename='M100_SD_cube_PM_03_04.image.mom1', fitsimage='M100_SD_cube_PM_03_04.image.mom1.fits')





#imageproc=time.clock()
#imagetime = time.time()

# -- endl of M100 script
endProc = combproc
endTime = combtime






# Red Hat Enterprise Linux Workstation release 6.3 (Santiago)
# on 64bit (2014/11/05)
# Statistics
# Please select PM or CM
# (r31543)


# --PM-- #
st = imstat('M100_SD_cube_PM_03_04.image')
immax = 0.14828479290008545
immin = -0.056186217814683914
imrms = 0.023878581821918488
# imflux = 29511.69662079087 before imstat change 3/19/2015
imflux = 187393.2531559114
immean = 0.0076376026451322136
immedian = 0.0034651397727429867
imnpts = 38640.0
imsum = 295.11696620790872
imsigma = 0.022624476084946776
immedabsdevmed = 0.010576624423265457
imquartile = 0.021630318835377693
imsumsq = 22.0320119621645
immaxpos = numpy.array([26, 27,  0, 49])
imminpos = numpy.array([27, 37,  0, 23])


"""
# --CM-- #
st = imstat('M100_SD_cube_CM_03_05.image')
immax = 0.085772879421710968
immin = -0.062363740056753159
imrms = 0.01886078342795372
imflux = 25281.257652479439 old before imstat change
immean = 0.0065427685436023394
immedian = 0.004905222449451685
imnpts = 38640.0
imsum = 252.81257652479439
imsigma = 0.017689811242650094
immedabsdevmed = 0.010991888120770454
imquartile = 0.022090356796979904
imsumsq = 13.74537410708181
immaxpos = numpy.array([25, 20,  0, 28])
imminpos = numpy.array([14, 21,  0,  1])
"""


#thistest
thistest_immax = st['max'][0]
thistest_immin = st['min'][0]
thistest_imrms = st['rms'][0]
thistest_imflux = st['flux'][0]
thistest_immean = st['mean'][0]
thistest_immedian =st['median'][0]
thistest_imnpts = st['npts'][0]
thistest_imsum = st['sum'][0]
thistest_imsigma = st['sigma'][0]
thistest_immaxpos = st['maxpos']
thistest_imminpos = st['minpos']
thistest_immedabsdevmed = st['medabsdevmed'][0]
thistest_imquartile = st['quartile'][0]
thistest_imsumsq = st['sumsq'][0]

#diff
diff_immax = abs((immax - thistest_immax)/immax)
diff_immin  = abs((immin - thistest_immin)/immin)
diff_imrms = abs((imrms - thistest_imrms)/imrms)
diff_imflux = abs((imflux - thistest_imflux)/imflux)
diff_immean = abs((immean - thistest_immean)/immean)
diff_immedian = abs((immedian - thistest_immedian)/immedian)
diff_imnpts= abs((imnpts - thistest_imnpts)/imnpts)
diff_imsum = abs((imsum - thistest_imsum)/imsum)
diff_imsigma = abs((imsigma - thistest_imsigma)/imsigma)
diff_immedabsdevmed = abs((immedabsdevmed - thistest_immedabsdevmed)/immedabsdevmed)
diff_imquartile = abs((imquartile - thistest_imquartile)/imquartile)
diff_imsumsq = abs((imsumsq -thistest_imsumsq)/imsumsq)


import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='M100_SD_cube_PM_03_04.'+datestring+'.log'
logfile=open(outfile,'w')

"""
print >>logfile,' *  diff_immax',diff_immax
print >>logfile,' *  diff_immin',diff_immin
print >>logfile,' *  diff_imrms',diff_imrms
print >>logfile,' *  diff_imflux',diff_imflux 
print >>logfile,' *  diff_immean',diff_immean
print >>logfile,' *  diff_immedian',diff_immedian
print >>logfile,' *  diff_imnpts',diff_imnpts
print >>logfile,' *  diff_imsum',diff_imsum
print >>logfile,' *  diff_imsigma',diff_imsigma
print >>logfile,' *  diff_immedabsdevmed',diff_immedabsdevmed
print >>logfile,' *  diff_imquartile',diff_imquartile
print >>logfile,' *  diff_imsumsq',diff_imsumsq
"""

print >>logfile,''
print >>logfile,'********** Regression ***********'
print >>logfile,'*                               *'

if all(thistest_immaxpos == immaxpos):
    print '* Passed image maxpos test'
    print >> logfile, '*  Image maxpos', thistest_immaxpos

if all(thistest_imminpos == imminpos):
    print '* Passed image minpos test'
    print >> logfile, '*  Image minpos', thistest_imminpos

if (diff_immax < 0.01): 
    print '* Passed image max test '
    print >>logfile,'*  Image max ',thistest_immax

if (diff_immin < 0.01): 
    print '* Passed image min test '
    print >>logfile,'*  Image mmin ',thistest_immin

if (diff_imrms < 0.01): 
    print '* Passed image rms test '
    print >>logfile,'*  Image rms ',thistest_imrms

if (diff_imflux < 0.01):
    print '* Passed image flux test '
    print >>logfile,'*  Image flux ',thistest_imflux

if (diff_immean< 0.01):
    print '* Passed image mean test '
    print >>logfile,'*  Image mean ',thistest_immean

if (diff_immedian<0.01):
    print '* Passed image median test '
    print >>logfile,'*  Image median ',thistest_immedian

if (diff_imnpts< 0.01):
    print '* Passed image npts test '
    print >>logfile,'*  Image npts ',thistest_imnpts 

if (diff_imsum< 0.01):
    print '* Passed image sum test '
    print >>logfile,'*  Image sum ',thistest_imsum

if (diff_imsigma< 0.01):
    print '* Passed image sigma test '
    print >>logfile,'*  Image sigma ',thistest_imsigma

if (diff_immedabsdevmed< 0.01):
    print '* Passed image medabsdevmed test '
    print >>logfile,'*  Image medabsdevmed ',thistest_immedabsdevmed

if (diff_imquartile< 0.01):
    print '* Passed image quartile test '
    print >>logfile,'*  Image quartile ',thistest_imquartile

if (diff_imsumsq< 0.01):
    print '* Passed image sumsq test '
    print >>logfile,'*  Image sumsq ',thistest_imsumsq


if ((diff_immax<0.01) & (diff_imrms<0.01) & (diff_immin<0.01) &(diff_imflux<0.01) & (diff_immean<0.01) & (diff_immedian<0.01) & (diff_imnpts<0.01) & (diff_imsum<0.01) & (diff_imsigma<0.01) & (diff_immedabsdevmed<0.01) & (diff_imquartile<0.01) & (diff_imsumsq<0.01) & all(thistest_imminpos == imminpos) & all(thistest_immaxpos == immaxpos)): 
    regstate=True
    print >>logfile,'---'
    print >>logfile,'Passed Regression test for M100_SD_PM_03_04'
    print >>logfile,'---'
    print ''
    print 'Regression PASSED'
    print ''
else: 
    regstate=False
    print ''
    print 'Regression FAILED'
    print ''
    print >>logfile,'----FAILED Regression test for M100_SD_PM_03_04'

"""
print >>logfile,'*********************************'
print >>logfile,''
print >>logfile,''
print >>logfile,'********* Benchmarking *****************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
print >>logfile,'****************************************'
#print >>logfile,'Processing rate MB/s  was: '+str(4100/(endTime - startTime))
print >>logfile,'* Breakdown: '
print >>logfile,'*   import       time was: '+str(importtime-startTime)
print >>logfile,'*            CPU time was: '+str(importproc-startProc)
print >>logfile,'*   split        time was: '+str(split_save_time-importtime)
print >>logfile,'*            CPU time was: '+str(split_save_proc-importproc)
print >>logfile,'*   flag         time was: '+str(flagtime-split_save_time)
print >>logfile,'*            CPU time was: '+str(flagproc-split_save_proc)
print >>logfile,'*   sdcal2       time was: '+str(sdcal2time-flagtime)
print >>logfile,'*            CPU time was: '+str(sdcal2proc-flagproc)
print >>logfile,'*   baseline     time was: '+str(sdbaselinetime-sdcal2time)
print >>logfile,'*            CPU time was: '+str(sdbaselineproc-sdcal2proc)
print >>logfile,'*   save         time was: '+str(savetime-sdbaselinetime)
print >>logfile,'*            CPU time was: '+str(saveproc-sdbaselineproc)
print >>logfile,'*   combine MSs  time was: '+str(combtime-savetime)
print >>logfile,'*            CPU time was: '+str(combproc-saveproc)
print >>logfile,'****************************************'
"""

logfile.close()


### Resore the previous storage setting
#sd.rc('scantable',storage=storage_sav)

