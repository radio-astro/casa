#############################################################################
## $Id:$
# Test Name:                                                                #
#    Regression Test Script for 3C219D ()                                   #
#                                                                           #
# Rationale for Inclusion:                                                  #
#    It ensures that the task is working properly.                          #
#                                                                           # 
# Features tested:                                                          #
#    1) Is the task working properly?                                       #
#    2) Is the task producing the same results as the reference?            #
#   The script will as well test the following features:                    #
#                                                                           #
#    Input Data                  Process              Output Data           #
# 3C219D_CAL.UVFITS.fits--->  importuvfits ----> 3C219Dtemp.ms.ms           #
#                                  |                                        #
#                                  v                                        #
#                               fixvis     ----> 3C219D.ms                  #
#                                  |                                        #
#                                  v                                        #
#                               vishead    ----> print on screen            #
#                                  |                                        #
#                                  v                                        #
#   1-field, one scale           clean     ----> D-1scale                   #
#   mask  rgn3C219D.rgn            |                                        #
#                                  |                                        #
#                                  v                                        #
#   1-field, multi-scale         clean     ----> D-3scale                   #
#   mask  rgn3C219D.rgn            |                                        #
#                                  |                                        #
#                                  v                                        #
#   3-field mosaic, one scale    clean     ----> D-1scale_moz               #
#   mask  rgn3C219D.rgn            |                                        #
#                                  |                                        #
#                                  v                                        #
#   3-field mosaic, multi-scale  clean   ----> D-3scale_moz                 #
#   mask  rgn3C219D.rgn                                                     #
#                                                                           #
#                                                                           #
#                                                                           #
# Input data:                                                               #
#    3C219D_CAL.UVFITS, rgn3C219D.rgn                                       #
#                                                                           #
# Description:                                                              #
#    3C219 VLA data, D-configuration                                        #                               
#                                                                           #
# Author: Ed Fomalont                                                       #
#                                                                           #
#                                                                           #
# Note: all input data have relative paths to the local directory           #
#############################################################################

import os
import time
import regression_utility as tstutl
from __main__ import default
from tasks import *
from taskinit import *


# Enable benchmarking?
benchmarking = True
usedasync = False

#
# Set up some useful variables
#
# FITS data and mask
fitsdata='3C219D_CAL.UVFITS'
rgnmask='rgn3C219D.rgn'

# The testdir where all output files will be kept
testdir='3C219D_regression/'

# The prefix to use for output files.
prefix=testdir+'3C219D'
tmpfile = prefix+'temp.ms'
msfile=prefix+'.ms'

# Make new test directory
# (WARNING! Removes old test directory of the same name if one exists)
tstutl.maketestdir(testdir)

# Start benchmarking
if benchmarking:
    startTime = time.time()
    startProc = time.clock()

#
#=====================================================================
#
# Import the data from FITS to MS
#
print '--Importuvfits--'
taskname = 'importuvfits'
default(taskname)
fitsfile = fitsdata
vis=tmpfile
antnamescheme = 'new'
importuvfits()

# Record import time
if benchmarking:
    importtime = time.time()

#
#=====================================================================
#
# List data to check
#
#  2 IF's, Field 1,2,3 = 3C219, 3C219NE, 3C219SW
#          Field 0 = 0917+449 main calibrator
#          Field 4 = 3C286 flux/pol calibrator

print '--Listobs--'
listobs(vis=tmpfile)

#
#=====================================================================
#
#  Change data equinox to J2000
#

print '--Fixvis--'
taskname = 'fixvis'
default (taskname)
vis = tmpfile
outputvis = msfile
refcode = 'J2000'
fixvis()

#
#=====================================================================
#
#  Compare equinox before and after

print '--Vishead--'
taskname = 'vishead'
default (vishead)
mode = 'list'
listitems = []
vis = tmpfile
a=vishead()
vis = msfile
b=vishead()
print "original equinox: ", a['ptcs'][1]['MEASINFO']['Ref']
print "     new equinox: ", b['ptcs'][1]['MEASINFO']['Ref']


#
#=====================================================================
#
#  Normal clean Image field 1 (3C219)

print '--Clean 1-field, 1 scale--'
taskname = 'clean'
default(taskname)
vis = msfile
imagename = testdir+'D-1scale'
field = '1'
niter = 4000
imsize = 512
cell = '2arcsec'
mask = rgnmask
weighting = 'briggs'
robust = 0.5
pbcor = T
restoringbeam = ['9arcsec']
threshold = '0.4mJy'
clean()

#
#=====================================================================
#
#  Multi-scale Image field 1 (3C219)
#

print '--Clean 1-field, multi-scale--'
taskname = 'clean'
default(taskname)
vis = msfile
imagename = testdir+'D-3scale'
field = '1'
niter = 2000
imsize = 512
multiscale = [0,5,15]
cell = '2arcsec'
mask = rgnmask
weighting = 'briggs'
robust = 0.5
pbcor = T
restoringbeam = ['9arcsec']
threshold = '0.4mJy'
clean()

#
#=====================================================================
#
#  Mosaic on fields 1,2,3
# 3C219+3C219NE+3C219SW
#

print '--Clean 3-field mosaic, 1 scale--'
taskname = 'clean'
default(taskname)
vis = msfile
imagename = testdir+'D-1scale_mos'
phasecenter = '1'
field = '1,2,3'
niter = 4000
imsize = 512
imagermode = 'mosaic'
cell = '2arcsec'
mask = rgnmask
weighting = 'briggs'
robust = 0.5
restoringbeam = ['9arcsec']
pbcor = T
threshold = '0.4mJy'
clean()

#
#=====================================================================
#
#  Multi-scale mosaic
#

print '--Clean 3-field mosaic, multi-scale--'
taskname = 'clean'
default(taskname)
vis = msfile
imagename = testdir+'D-3scale_mos'
imagermode = 'mosaic'
phasecenter = '1'
field = '1,2,3'
niter = 4000
imsize = 512
multiscale = [0,5,15]
imagermode = 'mosaic'
cell = '2arcsec'
mask = rgnmask
weighting = 'briggs'
robust = 0.5
restoringbeam = ['9arcsec']
pbcor = T
threshold = '0.4mJy'
clean()

#
#=====================================================================
#
#  Make difference images
#

print '--Immath--'
taskname = 'immath'
default (taskname)
imagename = [testdir+'D-3scale.image',testdir+'D-3scale_mos.image']
mode = 'evalexpr'
outfile = testdir+'DIFF1.image'
expr = ' (IM0 - IM1)'
immath()

#
#=====================================================================
#
#  Min/max of difference images in source region
#

taskname = 'imstat'
default (taskname)
region = rgnmask
imagename = testdir+'DIFF1.image'
a=imstat()
print "difference between D-3scale.image and D-3scale_mos.image"
print "diff:  rms = ",a['rms'][0],";  minimum= ",a['min'][0],": maximum= ",a['max'][0]    

taskname = 'immath'
default (taskname)
imagename = [testdir+'D-1scale.image',testdir+'D-3scale.image']
mode = 'evalexpr'
outfile = testdir+'DIFF2.image'
expr = ' (IM0 - IM1)'
immath()

#
#=====================================================================
#
#  Min/max of difference images in source region
#

taskname = 'imstat'
default (taskname)
region = rgnmask
imagename = testdir+'DIFF2.image'
a=imstat()
print "difference between D-1scale.image and D-1scale_moz.image"
print "diff:  rms = ",a['rms'][0],";  minimum= ",a['min'][0],": maximum= ",a['max'][0]    


#  End of 3C219D regression
