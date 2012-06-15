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
import regression_utility as tstutl
from __main__ import default
from tasks import *
from taskinit import *


# Initialization
regstate=True

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

''' Version with breakpoints. It will raise an Exception if any task fails.'''

#
#=====================================================================
#
# Import the data from FITS to MS
#
#try:
print '--Importuvfits--'
status = True
taskname = 'importuvfits'
default(taskname)
fitsfile = fitsdata
vis=tmpfile
antnamescheme = 'new'
status = importuvfits()
    
if status == False:
    regstate=False
    raise Exception, "Regression test failed for 3C219D in task \"%s\"" %(taskname)
    
#
#=====================================================================
#
# List data to check
#
#  2 IF's, Field 1,2,3 = 3C219, 3C219NE, 3C219SW
#          Field 0 = 0917+449 main calibrator
#          Field 4 = 3C286 flux/pol calibrator
print '--Listobs--'
status = True
taskname = 'listobs'
default(taskname)
vis=tmpfile
status = listobs()

if status == False:
    regstate=False
    raise Exception, "Regression test failed for 3C219D in task \"%s\"" %(taskname)
    
#
#=====================================================================
#
#  Change data equinox to J2000
#
print '--Fixvis--'
status = True
taskname = 'fixvis'
default (taskname)
vis = tmpfile
outputvis = msfile
refcode = 'J2000'
status  = fixvis()

if status == False:
    regstate=False
    raise Exception, "Regression test failed for 3C219D in task \"%s\"" %(taskname)
    
#
#=====================================================================
#
#  Compare equinox before and after
#
print '--Vishead--'
taskname = 'vishead'
default (vishead)
mode = 'list'
listitems = []
vis = tmpfile
a=vishead()
vis = msfile
b=vishead()

if (a == False or b == False):
    regstate=False
    raise Exception, "Regression test failed for 3C219D in task \"%s\"" %(taskname)

print "original equinox: ", a['ptcs'][1]['MEASINFO']['Ref']
print "     new equinox: ", b['ptcs'][1]['MEASINFO']['Ref']
    
    
#
#=====================================================================
#
#  Normal clean Image field 1 (3C219)
#
print '--Clean 1-field, 1 scale--'
status = True
taskname = 'clean'
default(taskname)
vis = msfile
imagename = testdir+'D-1scale'
field = '1'
imagermode = ''
niter = 4000
imsize = 512
cell = '2arcsec'
mask = rgnmask
weighting = 'briggs'
robust = 0.5
pbcor = T
restoringbeam = ['9arcsec']
threshold = '0.4mJy'
status = clean()

if status == False:
    regstate=False
    raise Exception, "Regression test failed for 3C219D in task \"%s\"" %(taskname)

    
#
#=====================================================================
#
#  Multi-scale Image field 1 (3C219)
#
print '--Clean 1-field, multi-scale--'
status = True
taskname = 'clean'
default(taskname)
vis = msfile
imagename = testdir+'D-3scale'
field = '1'
imagermode=''
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
status = clean()

if status == False:
    regstate=False
    raise Exception, "Regression test failed for 3C219D in task \"%s\"" %(taskname)
    
#
#=====================================================================
#
#  Mosaic on fields 1,2,3
# 3C219+3C219NE+3C219SW
#
print '--Clean 3-field mosaic, 1 scale--'
status = True
taskname = 'clean'
default(taskname)
vis = msfile
imagename = testdir+'D-1scale_mos'
phasecenter = '1'
field = '1,2,3'
imagermode=''
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
status = clean()

if status == False:
    regstate=False
    raise Exception, "Regression test failed for 3C219D in task \"%s\"" %(taskname)
    
#
#=====================================================================
#
#  Multi-scale mosaic
#
print '--Clean 3-field mosaic, multi-scale--'
status = True
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
status = clean()

if status == False:
    regstate=False
    raise Exception, "Regression test failed for 3C219D in task \"%s\"" %(taskname)
    
#
#=====================================================================
#
#  Make difference images of multi-scale
#
print '--Immath--'
status = True
taskname = 'immath'
default (taskname)
imagename = [testdir+'D-3scale.image',testdir+'D-3scale_mos.image']
mode = 'evalexpr'
outfile = testdir+'DIFF1.image'
expr = ' (IM0 - IM1)'
status = immath()

if status == None:
    regstate=False
    raise Exception, "Regression test failed for 3C219D in task \"%s\"" %(taskname)
    
#
#=====================================================================
#
#  Min/max of difference images in source region
#
taskname = 'imstat'
default (taskname)
#region = rgnmask
box = '200,200,300,300'
imagename = testdir+'DIFF1.image'
a1=imstat()

if a1 == False:
    regstate=False
    raise Exception, "Regression test failed for 3C219D in task \"%s\"" %(taskname)
    
#
#=====================================================================
#
#  Make difference images of single-scale
#
status = True
taskname = 'immath'
default (taskname)
imagename = [testdir+'D-1scale.image',testdir+'D-3scale.image']
mode = 'evalexpr'
outfile = testdir+'DIFF2.image'
expr = ' (IM0 - IM1)'
immath()

if status == None:
    regstate=False
    raise Exception, "Regression test failed for 3C219D in task \"%s\"" %(taskname)
    
#
#=====================================================================
#
#  Min/max of difference images in source region
#
taskname = 'imstat'
default (taskname)
#region = rgnmask
box = '200,200,300,300'
imagename = testdir+'DIFF2.image'
a2=imstat()

if a2 == False:
    regstate=False
    raise Exception, "Regression test failed for 3C219D in task \"%s\"" %(taskname)

    
# Do regressions tests
# Reference values:

ref1_rms=0.0005
ref2_rms=0.0002

print "difference between D-3scale.image and D-3scale_mos.image"
print "diff:  rms = ",a1['rms'][0],";  minimum= ",a1['min'][0],": maximum= ",a1['max'][0]    
print "difference between D-1scale.image and D-1scale_mos.image"
print "diff:  rms = ",a2['rms'][0],";  minimum= ",a2['min'][0],": maximum= ",a2['max'][0]    
    
if((abs(ref1_rms-a1['rms']) > 0.001) or (abs(ref2_rms-a2['rms']) > 0.001)):
    print abs(ref1_rms-a1['rms'])
    print abs(ref2_rms-a2['rms'])
    regstate=False
    print ''
    print 'Regression FAILED'
    print ''
    raise Exception, 'Regression test failed for 3C219D. Absolute differences are larger than 1e-03.'
else:
    print ''
    print 'Regression PASSED'
    print ''
        

#  End of 3C219D regression

