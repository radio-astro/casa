# Regression Tests of M100 SV data
# Measurementset version (2016/10/03) updated from ASAP one
#
# ASDM: uid___A002_X6218fb_X264
# Position-Switching observation of M100
# Tsys SPW: 1, 3, 5, 7
# Science SPW: 9, 11, 13, 15
#
# Task Sequence:
# importasdm
# plotms
# flagdata
# sdcal
# sdbaseline
# plotms
# sdimaging
# immoments
# exportfits
# imstat
#



import time
import os
import filltsys
import numpy
from recipes.almahelpers import tsysspwmap
import shutil

rawname = 'uid___A002_X6218fb_X264'
msname = rawname + '.ms'
listname = msname + '.listobs.txt'
blname = rawname + '.ms_bl'
target_spws = ['9', '11', '13', '15']

for name in [rawname, listname, msname, blname]:
    if os.path.exists(name):
        if os.path.isdir(name): shutil.rmtree(name)
        else: os.remove(name)

casapath = os.environ['CASAPATH']
datapath = casapath.split()[0]+'/data/regression/alma-sd/M100/'+rawname
print 'Copying ASDM from '+datapath
shutil.copytree(datapath, rawname) 

startTime=time.time()
startProc=time.clock()



# Check CASA Version #


print "You are using " + cu.version_info( )

if (cu.compare_version('<',[4,3,0])):
    print "YOUR VERSION OF CASA IS TOO OLD FOR THIS GUIDE."
    print "PLEASE UPDATE IT BEFORE PROCEEDING."
else:
    print "Your version of CASA is appropriate for this guide."




# ASDM Data #

basename = [rawname]


# Create Measurement Sets from ASDM Data ##
# importasdm func converts ASDM format to MS format

print '--Import--'

importasdm(asdm = rawname, vis=msname, overwrite=True)

importproc=time.clock()
importtime=time.time()

# listobs task generates detailed information of the MS
# ~.listobs.txt

default(listobs)
vis = msname
listfile = listname
listobs()



# Initial inspection of the data with plotms task. 
# First plot amplitude versus channel, 
# averaging over time in order to speed up the plotting process.

default(plotms)
vis   = msname
xaxis ='channel'
yaxis ='amp'
averagedata = True
avgtime = '1e8'
avgscan = True
spw='1,3,5,7,9,11,13,15'
iteraxis = 'spw'
coloraxis = 'antenna1'
gridrows = 3
gridcols = 3
showgui = False
plotfile = 'raw_spectrum.png'
overwrite = True
plotms()

# View Spectra #

for antname in ['PM03', 'PM04', 'CM03', 'CM05']:
    default(plotms)
    vis   = msname
    xaxis ='channel'
    yaxis ='amp'
    averagedata = True
    avgtime = '1e8'
    spw='15'
    antenna = antname + '&&&'
    iteraxis = 'scan'
    coloraxis = 'corr'
    gridrows = 3
    gridcols = 3
    showgui = False
    plotfile = ('raw_spectrum.%s.spw%s.png' % (antname, spw))
    overwrite = True
    plotms()


# Flagging #
print '--Flagging--'

default(flagdata)
vis = msname
mode = 'manual'
spw = '9; 11; 13; 15: 0~119;3960~4079'
antenna = 'PM03&&&;PM04&&&;CM03&&&;CM05&&&'
action = 'apply'
flagdata()

flagproc = time.clock()
flagtime = time.time()


# Apply Calibration and Inspect #

print '--Calibration sdcal--'

default(sdcal)
infile  = msname
calmode = 'ps,tsys,apply'
spwmap  = {'1':[9],'3':[11],'5':[13],'7':[15]}
sdcal()

sdcalproc=time.clock()
sdcaltime=time.time()



# Baseline Subtraction and Inspect #

print '--Caribration Baseline --'

default(sdbaseline)
infile = msname
datacolumn = 'corrected'
spw = str(',').join(target_spws)
maskmode = 'auto'
thresh = 3.0
avg_limit = 8
blfunc = 'poly'
order = 1
outfile = blname
overwrite = True
sdbaseline()

sdbaselineproc = time.clock()
sdbaselinetime = time.time()

# Plot the calibrated spectra, using the plotms task. 
# The commands below will plot one spectrum per scan, spw and polarization. 

for i in target_spws:
    for antname  in ['PM03', 'PM04', 'CM03', 'CM05']:
        default(plotms)
        vis   = msname
        ydatacolumn = 'corrected'
        xaxis ='frequency'
        yaxis ='real'
        averagedata = True
        avgtime = '1e8'
        spw = i
        antenna = antname + '&&&'
        intent='OBSERVE_TARGET#ON_SOURCE'
        iteraxis = 'scan'
        coloraxis = 'corr'
        gridrows = 3
        gridcols = 3
        showgui = False
        plotfile = ('calibrated_spectrum.%s.spw%s.png' % (antname, spw))
        plotms()

# View Baselined Spectra with the sdplot task #

for i in range(len(target_spws)):
    org_spw = target_spws[i]
    for antname  in ['PM03', 'PM04', 'CM03', 'CM05']:
        default(plotms)
        vis   = blname
        ydatacolumn = 'data'
        xaxis ='frequency'
        yaxis ='real'
        averagedata = True
        avgtime = '1e8'
        spw = str(i)
        antenna = antname + '&&&'
        intent='OBSERVE_TARGET#ON_SOURCE'
        iteraxis = 'scan'
        coloraxis = 'corr'
        gridrows = 3
        gridcols = 3
        showgui = False
        plotfile = ('baselined_spectrum.%s.spw%s.png' % (antname, org_spw))
        overwrite = True
        plotms()

# Combine all MSs to one MS #

print '--Combine MSs to one MS--'

new_spw15 = target_spws.index('15')

# PM #
os.system('rm -rf M100_SD_cube_PM_03_04.image*')
default(sdimaging)
infiles = [blname]
field='0'
spw=str(new_spw15)
antenna='PM03,PM04'
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

# CONVERT image unit to K
imhead(imagename=outfile, mode='put', hdkey='bunit', hdvalue='K')

# CM #
os.system('rm -rf M100_SD_cube_CM_03_05.image*')
default(sdimaging)
infiles = [blname]
field='0'
spw=str(new_spw15)
antenna='CM03,CM05'
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

# CONVERT image unit to K
imhead(imagename=outfile, mode='put', hdkey='bunit', hdvalue='K')


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
## ASAP version
#immax = 0.14828479290008545
#immin = -0.056186217814683914
#imrms = 0.023878581821918488
## imflux = 29511.69662079087 before imstat change 3/19/2015
#imflux = 187393.2531559114
#immean = 0.0076376026451322136
#immedian = 0.0034651397727429867
#imnpts = 38640.0
#imsum = 295.11696620790872
#imsigma = 0.022624476084946776
#immedabsdevmed = 0.010576624423265457
#imquartile = 0.021630318835377693
#imsumsq = 22.0320119621645
### MS tasks ###
immax = 0.149762198329
immin = -0.0560916438699
imrms = 0.0239948061386
imflux = 188834.785818
immean = 0.00775288738976
immedian = 0.00352751777973
imnpts = 38360.0
imsum = 297.400760271
imsigma = 0.0227080800888
immedabsdevmed = 0.0105304607423
imquartile = 0.0215698019601
imsumsq = 22.0857976817
immaxpos = numpy.array([26, 27, 0, 49])
imminpos = numpy.array([14, 27, 0, 64])


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

test_status = True
if all(thistest_immaxpos == immaxpos):
    print '* Passed image maxpos test'
else :
    test_status = False
    print '* FAILED image maxpos test'
print >> logfile, '*  Image maxpos', thistest_immaxpos
    

if all(thistest_imminpos == imminpos):
    print '* Passed image minpos test'
else:
    test_status = False
    print '* FAILED image minpos test'
print >> logfile, '*  Image minpos', thistest_imminpos

if (diff_immax < 0.01): 
    print '* Passed image max test '
else:
    test_status = False
    print '* FAILED image max test '
print >>logfile,'*  Image max ',thistest_immax

if (diff_immin < 0.01): 
    print '* Passed image min test '
else:
    test_status = False
    print '* FAILED image min test '
print >>logfile,'*  Image min ',thistest_immin

if (diff_imrms < 0.01): 
    print '* Passed image rms test '
else:
    test_status = False
    print '* FAILED image rms test '
print >>logfile,'*  Image rms ',thistest_imrms

if (diff_imflux < 0.01):
    print '* Passed image flux test '
else:
    test_status = False
    print '* FAILED image flux test '
print >>logfile,'*  Image flux ',thistest_imflux

if (diff_immean< 0.01):
    print '* Passed image mean test '
else:
    test_status = False
    print '* FAILED image mean test '
print >>logfile,'*  Image mean ',thistest_immean

if (diff_immedian<0.01):
    print '* Passed image median test '
else:
    print '* FAILED image median test '
    test_status = False
print >>logfile,'*  Image median ',thistest_immedian

if (diff_imnpts< 0.01):
    print '* Passed image npts test '
else:
    print '* FAILED image npts test '
    test_status = False
print >>logfile,'*  Image npts ',thistest_imnpts 

if (diff_imsum< 0.01):
    print '* Passed image sum test '
else:
    print '* FAILED image sum test '
    test_status = False
print >>logfile,'*  Image sum ',thistest_imsum

if (diff_imsigma< 0.01):
    print '* Passed image sigma test '
else:
    print '* FAILED image sigma test '
    test_status = False
print >>logfile,'*  Image sigma ',thistest_imsigma

if (diff_immedabsdevmed< 0.01):
    print '* Passed image medabsdevmed test '
else:
    test_status = False
    print '* FAILED image medabsdevmed test '
print >>logfile,'*  Image medabsdevmed ',thistest_immedabsdevmed

if (diff_imquartile< 0.01):
    print '* Passed image quartile test '
else:
    test_status = False
    print '* FAILED image quartile test '
print >>logfile,'*  Image quartile ',thistest_imquartile

if (diff_imsumsq< 0.01):
    print '* Passed image sumsq test '
else:
    test_status = False
    print '* FAILED image sumsq test '
print >>logfile,'*  Image sumsq ',thistest_imsumsq


if (test_status):
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
print >>logfile,'*   flag         time was: '+str(flagtime-importtime)
print >>logfile,'*            CPU time was: '+str(flagproc-importproc)
print >>logfile,'*   sdcal       time was: '+str(sdcaltime-flagtime)
print >>logfile,'*            CPU time was: '+str(sdcalproc-flagproc)
print >>logfile,'*   baseline     time was: '+str(sdbaselinetime-sdcaltime)
print >>logfile,'*            CPU time was: '+str(sdbaselineproc-sdcalproc)
print >>logfile,'*   combine MSs  time was: '+str(combtime-sdbaselinetime)
print >>logfile,'*            CPU time was: '+str(combproc-sdbaselineproc)
print >>logfile,'****************************************'

logfile.close()



