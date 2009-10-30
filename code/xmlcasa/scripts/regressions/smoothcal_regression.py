#############################################################################
## $Id:$
# Test Name:                                                                #
#    Regression Test Script for smoothcal()                                 #
#                                                                           #
# Rationale for Inclusion:                                                  #
#    It ensures that the task is working properly.                          #
#                                                                           # 
# Features tested:                                                          #
#    1) Is the task working properly?                                       #
#    2) Is the task producing the same results as the reference?            #
#   The script will as well test the following features:                    #
#                                                                           #
#    Input Data           Process              Output Data                  #
#                      used data from------>  ngc1333.ms  +                 #
#                                             ngc1333.gcal                  #
#                            |                                              #
#                            v                                              #
#                        smoothcal   ------>  ngc1333.smoothed              #
#                                                                           #
# Input data:                                                               #
#    ngc1333.ms and ngc1333.gcal                                                         #
#                                                                           #
#############################################################################

import os
import string
import sys
import time

from __main__ import default
from tasks import *
from taskinit import *

import regression_utility as tstutl

# Enable benchmarking?
benchmarking = True

# Start benchmarking
if benchmarking:
    startTime = time.time()
    startProc = time.clock()

# Paths
pathname=os.environ.get('CASAPATH').split()[0]

# This is where the NGC1333 UVFITS data will be
datapath=pathname+'/data/regression/smoothcal/'
msfile=datapath+'ngc1333.ms'
gtable=datapath+'ngc1333.gcal'

# The testdir where all output files will be kept
testdir='smoothcal_regression'

# The prefix to use for output files.
prefix=testdir+"/"+'ngc1333'

# Make new test directory
# (WARNING! Removes old test directory of the same name if one exists)
tstutl.maketestdir(testdir)


#=====================================================================
#
# Apply smoothcal on gain table
#
print '--Smoothcal--'
default('smoothcal')

smtable = prefix + '.smoothed'
smoothcal(vis=msfile,tablein=gtable,caltable=smtable,
     smoothtype='mean',smoothtime=7200.0)

# smoothcal calibration completion time
if benchmarking:
    endProc = time.clock()
    endTime = time.time()

#=====================================================================
# Do regression test
#

EPS  = 1e-5;  # Logical "zero"

# Reference statistics for gtable and smtable
# GAIN table
#gmax = 1.0
#gmean = 0.55299845106360213
#gmedabsdevmed = 0.1120542585849762
#gmedian = 0.46294707059860229
#gmin = 0.045811429619789124
#gnpts = 14500.0
#gquartile = 0.36033809185028076
#grms = 0.60084819793701172
#gstddev = 0.2349788425923284
#gsum = 8018.477540422231
#gsumsq = 5234.7687634427484
#gvar = 0.055215056466030248


# SMOOTHED table
smax = 1.0
smean = 0.53380899890332389
smedabsdevmed = 0.090539306402206421
smedian = 0.45989406108856201
smin = 0.25960710644721985
snpts = 14500.0
squartile = 0.26996403932571411
srms = 0.573993980884552
sstddev = 0.21099782732171318
ssum = 7740.230484098196
ssumsq = 4777.301371364324
svar = 0.044520083134483496

try:
    regstate = False;

#    tb.open(gtable)
#    gstats = tb.statistics(column='GAIN',complex_value='amp')
#    tb.close()
 
     # Get the difference between new and reference values 
    # Original Gain table
#    dif_max = gmax - gstats['GAIN']['max'] 
#    dif_mean = gmean - gstats['GAIN']['mean']
#    dif_medabsdevmed = gmedabsdevmed - gstats['GAIN']['medabsdevmed']
#    dif_median = gmedian - gstats['GAIN']['median']
#    dif_min = gmin - gstats['GAIN']['min']
#    dif_npts = gnpts - gstats['GAIN']['npts']
#    dif_quartile = gquartile - gstats['GAIN']['quartile']
#    dif_rms = grms - gstats['GAIN']['rms']
#    dif_stddev = gstddev - gstats['GAIN']['stddev']
#    dif_sum = gsum - gstats['GAIN']['sum']
#    dif_sumsq = gsumsq - gstats['GAIN']['sumsq']
#    dif_var = gvar - gstats['GAIN']['var']
       
    print '--Statistics of smoothed table--'
    tb.open(smtable)
    sstats = tb.statistics(column='GAIN',complex_value='amp')
    tb.close()
    
    
    # Get the difference between new and reference values 
    # Smoothed table
    dif_max = smax - sstats['GAIN']['max'] 
    dif_mean = smean - sstats['GAIN']['mean']
    dif_medabsdevmed = smedabsdevmed - sstats['GAIN']['medabsdevmed']
    dif_median = smedian - sstats['GAIN']['median']
    dif_min = smin - sstats['GAIN']['min']
    dif_npts = snpts - sstats['GAIN']['npts']
    dif_quartile = squartile - sstats['GAIN']['quartile']
    dif_rms = srms - sstats['GAIN']['rms']
    dif_stddev = sstddev - sstats['GAIN']['stddev']
    dif_sum = ssum - sstats['GAIN']['sum']
    dif_sumsq = ssumsq - sstats['GAIN']['sumsq']
    dif_var = svar - sstats['GAIN']['var']

#    print dif_max,dif_mean,dif_medabsdevmed,dif_median,dif_min,dif_npts, \
#              dif_quartile,dif_rms,dif_stddev,dif_sum,dif_sumsq,dif_var
    
    print ''
    
    if ((abs(dif_max) > EPS) or
        (abs(dif_mean) > EPS) or
        (abs(dif_medabsdevmed) > EPS) or
        (abs(dif_median) > EPS) or
        (abs(dif_min) > EPS) or
        (abs(dif_npts) > EPS) or
        (abs(dif_quartile) > EPS) or
        (abs(dif_rms) > EPS) or
        (abs(dif_stddev) > EPS) or
        (abs(dif_sum) > EPS) or
        (abs(dif_sumsq) > EPS) or
        (abs(dif_var)> EPS)
        ) :
        regstate = False
        print >> sys.stderr, "Smoothcal regression tests failed!"
        print ''
    else :
        regstate = True
        print "Smoothcal regression tests passed."
        print ''

    if benchmarking:
        print '********* Benchmarking *****************'
        print '*                                      *'
        print 'Total wall clock time was: ', endTime - startTime
        print 'Total CPU        time was: ', endProc - startProc


except Exception, instance:
    print >> sys.stderr, "Regression test failed for smoothcal instance = ", instance



