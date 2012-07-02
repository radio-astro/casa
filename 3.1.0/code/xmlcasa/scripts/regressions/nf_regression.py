#############################################################################
## $Id:$
# Test Name:                                                                
#  Narrow field imaging of small-diameter sources
#
#  The ms contains simulated data taken from the u-v coverage
#  for VLA-Bconfiguration data of 3C219.  The source data were
#  replaced by a visibility associated with discrete sources.  One
#  source pair is a point double, another is a displaced core-halo.

#                                                          (Jy)
#  SOURCE      dX        dY      bmaj    bmin   bpa    I    Q     U
#    1        +20"      -50"      0"      0"     0d   1.2  0.00  0.00 core
#    2        +21       -49      2.0     1.5    60    1.5  0.07  0.04 halo
#    3        -40       +30      1.0     1.0     0    0.6  0.02 -0.02
#    4        +54       -27      1.5     0.5    30    0.5 -0.06  0.03
#    5        +60       +30      0.0     0.0     0    0.4  0.03 -0.02 double
#    6        +60.5     +30.2    0.0     0.0     0    0.2 -0.01 +0.03 double
#    7       +300      -150      0.0     0.0     0    1.0  0.10 -0.08 outside pb

#  Gaussian noise of 0.3 Jy has been added to each uv-point,
#  resulting in a map rms of 0.33 mJy.

#  The input files are:
#   1.  model7.ms contains the measurement set
#   2.  model2.txt contains the parameters for the two fields
#   3.  M7_0.mask and M7_1.mask are the cleaning masks for the two fields
#   4.  Double_I.ext and Double_U.ext are imfit estimates for doubles.

#  Image/clean all Stokes parameters; main field an outlier field
#

import os
import time
import regression_utility as tstutl
from __main__ import default
from tasks import *
from taskinit import *

# Enable benchmarking?
benchmarking = True
usedasync = False

# The testdir where all output files will be kept
testdir='nf_regression/'
prefix='narrowfield'

# Input files
msfile = 'model7.ms'
modelfile = 'model2.txt'
mask0 = 'M7_0.mask'
mask1 = 'M7_1.mask'
doubleI = 'Double_I.ext'
doubleU = 'Double_U.ext'

# Make new test directory
# (WARNING! Removes old test directory of the same name if one exists)
tstutl.maketestdir(testdir)

# Start benchmarking
if benchmarking:
    startTime = time.time()
    startProc = time.clock()

###############################################
# Run clean
#
print '--Clean--'
    
taskname = 'clean'
default(taskname)
vis = msfile
imagename = testdir+'MG_IQUV'
outlierfile = modelfile
niter = 2000
threshold = '0.8mJy'
mask = [[mask0],[mask1]]
cell = '0.2arcsec'
weighting = 'briggs'
imagermode = 'csclean'
restoringbeam = ['0.65','0.65','0.0deg'] 
stokes = 'IQUV'
clean()


###############################################
# Run immath
#
print '--Immath--'

#  Make separate images in each stokes

taskname = 'immath'
default(taskname)
imagename = testdir+'MG_IQUV_0.image'
expr = 'IM0'
stokes = 'I'; outfile = testdir+'MG_I_0.im'; immath()
stokes = 'Q'; outfile = testdir+'MG_Q_0.im'; immath()
stokes = 'U'; outfile = testdir+'MG_U_0.im'; immath()
stokes = 'V'; outfile = testdir+'MG_V_0.im'; immath()
imagename = testdir+'MG_IQUV_1.image'
expr = 'IM0'
stokes = 'I'; outfile = testdir+'MG_I_1.im'; immath()
stokes = 'Q'; outfile = testdir+'MG_Q_1.im'; immath()
stokes = 'U'; outfile = testdir+'MG_U_1.im'; immath()
stokes = 'V'; outfile = testdir+'MG_V_1.im'; immath()

###############################################
# Run imfit
#
#  Do selected imfits on some images
#  point source at field center

print '--Point sources--'
print '--Imfit I--'
taskname = 'imfit'
default (taskname)
box = '63,54,74,64'
append = F
stokes='I'
imagename = testdir+'MG_I_1.im'
logfile = testdir+'MG_I_1.log'
newestimates = testdir+'MG_I_1.est'
imfit() 
#!more MG_I_1.log
#  Peak/Int should be about 1.00 Jy, nearly unresolved 
#

print '--Imfit Q--'
imagename = testdir+'MG_Q_1.im'
logfile = testdir+'MG_Q_1.log'
stokes='Q'
imfit() 
#!more MG_Q_1.log
#  Peak/Int should be about 100 mJy, nearly unresolved 
#

print '--Imfit U--'
imagename = testdir+'MG_U_1.im'
logfile = testdir+'MG_U_1.log'
stokes='U'
imfit() 
#!more MG_U_1.log
#  Peak/Int should be about -80 mJy, nearly unresolved 
#

print '--Imfit V--'
imagename = testdir+'MG_V_1.im'
logfile = testdir+'MG_V_1.log'
stokes='V'
imfit() 
#!more MG_V_1.log
#  Pure noise, fit may blow up, but should not

print '--Extended source--'
print '--Imfit I--'
# Fit on an extended source
taskname = 'imfit'
default (taskname)
box = '231,367,252,391'
apend = F
stokes='I'
imagename = testdir+'MG_I_0.im'
logfile = testdir+'MG_I_0.log'
imfit() 
#!more MG_I_0.log
#  Int should be about 0.5 Jy, size 1.5x0.5 in pa 30 
#

print '--Imfit Q--'
imagename = testdir+'MG_Q_0.im'
logfile = testdir+'MG_Q_0.log'
stokes='Q'
imfit() 
#!more MG_Q_0.log
#  Int should be about -0.06 Jy, size 1.5x0.5 in pa 30 
#

print '--Imfit U--'
imagename = testdir+'MG_U_0.im'
logfile = testdir+'MG_U_0.log'
stokes='U'
imfit() 
#!more MG_Q_0.log
#  Int should be about +0.03 Jy, size 1.5x0.5 in pa 30 

print '--Point double source--'
print '--Imfit I--'
# Fit on point double source
taskname = 'imfit'
default (taskname)
box = '203,657,221,671'
append = F
stokes='I'
imagename = testdir+'MG_I_0.im'
logfile = testdir+'Double_I_0.log'
estimates = doubleI
imfit() 
#!more Double_I_0.log
#  Peaks are 0.4 and 0.2, separated by 2.5 xpix and 1.0 ypix

print '--Imfit U--'
imagename = testdir+'MG_U_0.im'
logfile = testdir+'Double_U_0.log'
estimates = doubleU
stokes='U'
imfit() 
#!more Double_U_0.log
#  Peaks are -0.02 and +0.04, separated by 2.5 xpix and 1.0 ypix
