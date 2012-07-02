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
#                            |                                              #
#                            v                                              #
#                        smoothcal   ------>  ngc1333.smoothed              #
#                                                                           #
# Input data:                                                               #
#    ngc1333.ms, ngc1333.gcal and ngc1333.ref.smoothed                      #
#                                                                           #
# Note: all input data have relative paths to the local directory           #
#############################################################################


import os
import time
import regression_utility as tstutl
from __main__ import default
from tasks import *
from taskinit import *


# Set up some useful variables
#
# Get path to CASA home directory by stripping name from '$CASAPATH'
#pathname=os.environ.get('CASAPATH').split()[0]

# Define the input data
msfile='ngc1333.ms'
gtable='ngc1333.gcal'
reffile='ngc1333.ref.smoothed'

# The testdir where all output files will be kept
testdir='ngc1333_regression'

# The prefix to use for output files.
prefix=testdir+"/"+'ngc1333'

# Make new test directory
# (WARNING! Removes old test directory of the same name if one exists)
tstutl.maketestdir(testdir)


#
#=====================================================================
#
# Smoothcal the gain table
#
try:
    
    regstate = True
    ret_status = None
    print '-- Smoothcal --'
    default('smoothcal')

    smtable = prefix + '.smoothed'
    ret_status = smoothcal(vis=msfile,tablein=gtable,caltable=smtable,smoothtype='mean',smoothtime=7200.0)
    if ret_status == False :
        regstate = False
        print >> sys.stderr, "Smoothcal failed to execute!"  
     
    #=====================================================================
    # Do regression test
    #   
    EPS  = 1e-5;  # Logical "zero"
    total = 0
    fail = 0

   # Compare the values of both tables
    tb.open(reffile)
    refcol = tb.getvarcol('GAIN')
    tb.close()
   
    tb.open(smtable)
    smcol = tb.getvarcol('GAIN')
    tb.close()
   
    # get the length of rows
    nrows = len(refcol)
    print "-- Regression tests --"
    print "-- Comparing the values of %s rows --" %nrows

     # Loop over every row,pol and get the data
    for i in range(1,nrows,1) :
      row = 'r%s'%i     
      # polarization is 0-1
      for pol in range(0,2) :     
        total += 1
        refdata = refcol[row][pol]
        smdata = smcol[row][pol]
#        print refdata,smdata
        if (abs(refdata - smdata) > EPS) :
            fail += 1
            
    # raise an exception if values are not within a maximum   
    if fail > 0 :
        perc = fail*100/total
        regstate = False
        print >> sys.stderr, "Regression test failed: %s %% of values are different "\
                        "by more than the allowed maximum %s" %(perc,EPS)
    else :
        regstate = True
        print >> sys.stdout, "Regression tests passed"

 
except Exception, instance:
    print >> sys.stderr, "Regression test failed for smoothcal instance = ", instance
