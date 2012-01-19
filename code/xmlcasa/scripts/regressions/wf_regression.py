#############################################################################
## $Id:$
# Test Name:                                                                #
#    Regression Test Script to reduce a wide-field image                    #
#                                                                           #
# Rationale for Inclusion:                                                  #
#    It ensures that the the mosaic is successfully created                 #
#    taking into account the outliers. The data has been                    #
#    already calibrated.                                                    #
#                                                                           # 
# Features tested:                                                          #
#    1) Is the task working properly?                                       #
#    2) Is the task producing the same results as the reference?            #
#   The script will as well test the following features:                    #
#                                                                           #
#    Input Data               Process          Output Data                  #
# widefield_model.fits--->  importuvfits ----> widefield.ms                 #
#                               |                                           #
#                               v                                           #
#   classic mode              clean      ----> wf_h.*                       #
#                               |                                           #
#                               v                                           #
#   wide-field mode           clean   ------>  wf_x.*                       #
#                               |                                           #
#                               v                                           #
#                             imstat                                        #
#                                                                           #
#                                                                           #
# Input data:                                                               #
#    widefield_model.fits, outliers.txt                                     #
#                                                                           #
# Author: Ed Fomalont                                                       #
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
# This is where the NGC1333 UVFITS data will be
fitsdata='widefield_model.uvfits'

# The testdir where all output files will be kept
testdir='wf_regression'

# The prefix to use for output files.
#prefix=testdir+"/"+'widefield'
prefix='widefield'
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
try:
    
    print '--Import--'
    
    # Safest to start from task defaults
    default('importuvfits')
    
    # Set up the MS filename and save as new global variable
    msfile = prefix + '.ms'
    
    # Set up image names
    nimage = 'wf_h'
    wimage = 'wf_x'
    
    # Use task importuvfits
    fitsfile = fitsdata
    vis = msfile
    antnamescheme="new"
    importuvfits()
    
    # Record import time
    if benchmarking:
        importtime = time.time()
   
    #
    #=====================================================================
    #
    # Run normal clean and create 5 images
    #
    print '--Clean in classic mode--'
    default('clean')
    vis = msfile
    outlierfile = 'outliers.txt'
    niter = 1000
    imagename = nimage
    field = '0'
    imsize = 128
    cell = '0.3arcsec'
    weighting = 'briggs'
    pbcor = False
    usescratch = False
#    async = T
    clean()

    # Record clean completion time
    if benchmarking:
        clean1time = time.time()
    
    
    #
    #=====================================================================
    #
    # Run clean using widefield mode with 5 outliers
    #
    print '--Clean in widefield mode--'
    taskname = 'clean'
    default(taskname)
    vis = msfile
    imagename = wimage
    gridmode = 'widefield'
    wprojplanes = 64
    niter = 500
    interactive = False
    imsize = 4096
    cell = '0.4arcsec'
    weighting = 'briggs'
    usescratch = False
    async = False
    clean()  
    
    # Record clean completion time
    if benchmarking:
        clean2time = time.time()
    
    # It will create the following files:
    #wf_h_0.flux, wf_h_0.image, wf_h_0.model, wf_h_0.psf
    #wf_h_1.flux, wf_h_1.image, wf_h_1.model, wf_h_1.psf
    #wf_h_2.flux, wf_h_2.image, wf_h_2.model, wf_h_2.psf
    #wf_h_3.flux, wf_h_3.image, wf_h_3.model, wf_h_3.psf
    #wf_h_4.flux, wf_h_4.image, wf_h_4.model, wf_h_4.psf

    # and for the widefield case:
    # wf_x.flux, wf_x.image, wf_x.model, wf_x.residual, wf_x.mask
    # wf_x.psf
       
    # Get statistics of individual images
    print '--Imstat--'
    default('imstat')
    imagename = wimage+'.image'
    wstats =imstat()

    print ""
    print ' Wide-field results '
    print ' =============== '
    
    print ''
    print ' --Regression Tests--'
    print ''
    
    sumnpts = 0
    for i in range(5):
        imagename = nimage+'_%s'%i+'.image'
        nstats = imstat()
        
        # Compare the number of points in wide-field 
        # image and in individual images
        print nstats['max'][0]
        print nstats['min'][0]
#        sumnpts += nstats['npts'][0]
    
#    print "sum_n_npts=%s, w_npts=%s" %(sumnpts,wstats['npts'][0])
    print 'wide-field stats'
    print wstats['max'][0]
    print wstats['min'][0]
     
    # Pull the max from the nstats dictionary
    # created above using imstat
#    thistest_immax=nstats['max'][0]
#    oldtest_immax = 7.50
#    print ' Clean image max should be ',oldtest_immax
#    print ' Found : Image Max = ',thistest_immax
#    diff_immax = abs((oldtest_immax-thistest_immax)/oldtest_immax)
#    print ' Difference (fractional) = ',diff_immax
#
#    print ''
#    # Pull the rms from the nstats dictionary
#    thistest_imrms=nstats['rms'][0]
#    oldtest_imrms = 0.134
#    print ' Clean image rms should be ',oldtest_imrms
#    print ' Found : Image rms = ',thistest_imrms
#    diff_imrms = abs((oldtest_imrms-thistest_imrms)/oldtest_imrms)
#    print ' Difference (fractional) = ',diff_imrms

    # Record processing completion time
    if benchmarking:
        endProc = time.clock()
        endTime = time.time()

except Exception, instance:
    print >> sys.stderr, "Regression test failed for wide-field instance = ", instance

