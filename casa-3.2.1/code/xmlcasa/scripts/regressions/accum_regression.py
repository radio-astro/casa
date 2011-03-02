#############################################################################
## $Id:$
# Test Name:                                                                #
#    Regression Test Script for accum ()                                    #
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
# ngc5921.fits    --->  importuvfits ----> ngc5921.ms                       #
#                            |                                              #
#                            v                                              #
#                            |                                              #
#                            v                                              #
#                          setjy     ----> ngc1333.ms                       #
#                            |                                              #
#                            v                                              #
#                         gaincal    ----> ngc1333.int.gcal                 #
#  apply the gain solution using solution interval per integration          #
#                            |                                              #
#                            v                                              #
#                         gaincal    ----> ngc1333.scan.gcal                #
#  apply the gain solution using solution interval infinite (up to the      #
#  boundaries                                                               #
#                            |                                              #
#                            v                                              #
#                         gaincal    ----> ngc1333.rint.gcal                #
#  apply the gain solution using the previous scan gain table, but using    #
#  solution interval per integration                                        #
#                            |                                              #
#                            v                                              #
#                        accum   ------>  ngc1333.acc1.gcal                 #
#  accumulate the first time, using the scan table as the gain table        #
#                            |                                              #
#                            v                                              #
#                        accum   ------>  ngc1333.acc2.gcal                 #
#  accumulate the second time, using the relative int table as the gain     #
#  table. This last table will be compared with the first gain table        #
#                                                                           #
#                                                                           #
# Input data:                                                               #
#    ngc5921.fits                                                           #
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
fitsdata='ngc5921.fits'

# The testdir where all output files will be kept
testdir='accum_regression'

# The prefix to use for output files.
prefix=testdir+"/"+'ngc5921'

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
    # Set the fluxes of the primary calibrator(s)
    #
    print '--Setjy--'
    default('setjy')
    
    setjy(vis=msfile,field='0') 
    
    # Record setjy completion time
    if benchmarking:
        setjytime = time.time()
    
    #
    #=====================================================================
    #
    # Gain calibration using integration time
    #
    print '--Gaincal using interval per integration--'
    default('gaincal')
    
    Ginttable = prefix + '.int.gcal'
    gaincal(vis=msfile,caltable=Ginttable,
            field='0', gaintype='G',solint='int',combine='',refant='VA02')
    
    
    # gaincal calibration completion time
    if benchmarking:
        gaintime1 = time.time()
    
    #
    #=====================================================================
    #
    # Gain calibration using scan
    #
    print '--Gaincal using interval infinite--'
    default('gaincal')
    
    Gscantable = prefix + '.scan.gcal'
    gaincal(vis=msfile,caltable=Gscantable,
            field='0', gaintype='G',solint='inf',combine='',refant='VA02')
    
    
    # gaincal calibration completion time
    if benchmarking:
        gaintime2 = time.time()
    
    #
    #=====================================================================
    #
    # Gain calibration using scan table to recalibrate
    #
    print '--Gaincal using previous solution--'
    default('gaincal')
    
    Grinttable = prefix + '.rint.gcal'
    gaincal(vis=msfile,caltable=Grinttable,field='0', gaintype='G',gaintable=Gscantable,
            solint='int',combine='',interp='nearest',refant='VA02')
    
    
    # gaincal calibration completion time
    if benchmarking:
        gaintime3 = time.time()
    
    #
    #=====================================================================
    #
    # Rum accum on tables
    #
    print '--Accum on initial data--'
    default('accum')
    
    Acc1table = prefix + '.acc1.gcal'
    accum(vis=msfile,tablein='',accumtime=1.0,caltable=Acc1table,incrtable=Gscantable,
          field='0', calfield='0',interp='nearest')
    
    
    # gaincal calibration completion time
    if benchmarking:
        accumtime1 = time.time()
    
    
    #
    #=====================================================================
    #
    # Rum accum again on tables
    #
    print '--Accum using previous solution--'
    default('accum')
    
    Acc2table = prefix + '.acc2.gcal'
    accum(vis=msfile,tablein=Acc1table,caltable=Acc2table,incrtable=Grinttable,
          field='0', calfield='0',interp='nearest')
    
    
    # gaincal calibration completion time
    if benchmarking:
        accumtime2 = time.time()
        
    endProc = time.clock()
    endTime = time.time()

    # Save plot with the two table
    print '--Saving Plots--'
    saveplot = prefix + '.pdf'
    default('plotcal')
    plotcal(caltable=Ginttable,plotsymbol='+',overplot=False,field='0',markersize=10.0,
            showgui=False,figfile=saveplot)
    plotcal(caltable=Acc2table,plotsymbol='.',overplot=True,field='0',markersize=8.0,
            showgui=False,figfile=saveplot)

        
    # Compare Acc2table with Ginttable
    EPS = 1e-5
    total = 0
    fail = 0
    
    tb.open(Ginttable)
    intcol = tb.getvarcol('GAIN')
    iflag = tb.getvarcol('FLAG')
    tb.close()
    
    tb.open(Acc2table)
    afield = tb.query('FIELD_ID == 0')
    acccol = afield.getvarcol('GAIN')
    aflag = afield.getvarcol('FLAG')
    afield.done()
    tb.close()
    
    n1 = len(intcol)
    n2 = len(acccol)
    if n1 != n2 :
        print >> sys.stderr, "The two tables have different lengths"
        
     # Loop over every row,pol and get the data
    for i in range(1,n1,1) :
      row = 'r%s'%i     
      # polarization is 0-1
      for pol in range(0,2) :     
        
        # do not take flagged values
        if (not (iflag[row][pol] and aflag[row][pol])) :
            total += 1
            intdata = intcol[row][pol]
            accdata = acccol[row][pol]
    #        print intdata,accdata
            if (abs(intdata - accdata) > EPS) :
                fail += 1
                print >>sys.stderr, row,pol,intdata,accdata
            
    if fail > 0 :
        perc = fail*100/total
        regstate = False
        print >> sys.stderr, "Regression test failed: %f %% of values are different "\
                        "by more than the allowed maximum %s" %(perc,EPS)
    else :
        regstate = True
        print >> sys.stdout, "Regression tests passed. %s rows were analysed" %total

    print >>sys.stdout,'********* Benchmarking *****************'
    print >>sys.stdout,'*                                      *'
    print >>sys.stdout,'Total wall clock time was: '+str(endTime - startTime)
    print >>sys.stdout,'Total CPU        time was: '+str(endProc - startProc)
    print >>sys.stdout,'* Breakdown:                           *'
    print >>sys.stdout,'*   import       time was: '+str(importtime-startTime)
    print >>sys.stdout,'*   setjy        time was: '+str(setjytime-importtime)
    print >>sys.stdout,'*   gaincal1     time was: '+str(gaintime1-setjytime)
    print >>sys.stdout,'*   gaincal2     time was: '+str(gaintime2-gaintime1)
    print >>sys.stdout,'*   gaincal3     time was: '+str(gaintime3-gaintime2)
    print >>sys.stdout,'*   accum1       time was: '+str(accumtime1-gaintime3)
    print >>sys.stdout,'*   accum2       time was: '+str(accumtime2-accumtime1)
    print >>sys.stdout,'*****************************************'

except Exception, instance:
    print >> sys.stderr, "Regression test failed for accum instance = ", instance

