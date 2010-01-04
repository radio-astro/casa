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
# NGC1333_1.UVFITS--->  importuvfits ----> ngc1333.ms                       #
#                            |                                              #
#                            v                                              #
#                        flagdata    ----> ngc1333.ms                       #
#                            |                                              #
#                            v                                              #
#                          setjy     ----> ngc1333.ms                       #
#                            |                                              #
#                            v                                              #
#                         gaincal    ----> ngc1333.gcal                     #
#                            |                                              #
#                            v                                              #
#                            |                                              #
#                            v                                              #
#                        smoothcal   ------>  ngc1333.smoothed              #
#                                                                           #
# Input data:                                                               #
#    N1333_1.UVFITS and ngc1333.ref.smoothed                                #
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
# Get path to CASA home directory by stipping name from '$CASAPATH'
#pathname=os.environ.get('CASAPATH').split()[0]

# This is where the NGC1333 UVFITS data will be
#datapath=pathname+'/data/regression/ATST2/NGC1333/'
#fitsdata=datapath+'N1333_1.UVFITS'
fitsdata='N1333_1.UVFITS'
reffile='ngc1333.ref.smoothed'

# 3C147 model image for setjy
#modelim=pathname+'/data/nrao/VLA/CalModels/3C147_Q.im'
modelim='3C147_Q.im'

# The testdir where all output files will be kept
testdir='ngc1333_regression'

# The prefix to use for output files.
prefix=testdir+"/"+'ngc1333'

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
print '*** 02 MAY ***'
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
#  Flagging
#
print '--Flagdata--'

#
# The following information on bad data comes from a test report
# created by Debra Shepherd.  It is currently available at
# http://aips2.nrao.edu/projectoffice/almatst1.1/TST1.1.data.description.pdf
#
# NGC 1333: bad data on 2 May:
# o Correlator glitches
#   - Spwid 1, field 1: 02-May-2003/21:44:00 to 21:50:00
#   - Spwid 1, fields 2,3,4,5,6: 02-May-2003/21:40:00 to 21:51:00
#   - Spwid 1, field 7: 02-May-2003/21:56:00 to 21:58:00
#   - Spwid 2, fields 8,9,10,11,12: 02-May-2003/21:55:00 to 22:20:00
# o Non-fringing antennas:
#   - Ant 9 - everything: all spwids, all fields, all times
#   - Ant 14 - spwid 1, 02-May-2003/18:36:00 to 18:53:00
#   - Ant 14 - spwid 2, 02-May-2003/18:52:00 to 19:10:00
# o Bad bandpass solutions:
#   - Ant 22 - spwid 2 (pathological bandpass solution,
#     don't trust this antenna).
# o End channels:
#   - Flag channels 1,2,3 and 61, 62, 63 (very noisy)
#     Flag them to prevent higher noise in some image planes.
# o Even after all this, the calibrated source data still has amplitudes
# that vary from a maximum of 10Jy/channel to 30Jy/channel.
# The uv weighting will properly downweight the poor data.
#

# Use flagdata() in vector mode
default('flagdata')
vis = msfile
mode = 'manualflag'


##################################################
#
# Get rid of the autocorrelations from the MS
#
# Flag antenna with antennaid 8
#
# Flag all data whose amplitude  are not in range [0.0,2.0] on the
# parallel hands       ( not done )
#
# Flag data (which is bad) in a time range
#
# Flag all antenna 14, 15 data in the time ranges stated
#
# Sequential flagdata execution:
#
# flagdata(vis=msfile1, autocorr=True, mode='manualflag')
# flagdata(vis=msfile1,antenna='9', mode='manualflag')
# flagdata(vis=msfile1,mode='manualflag',clipexpr='ABS RR',
#          clipminmax=[0.0,2.0],clipoutside=True)
#
# flagdata(vis=msfile1,mode='manualflag',clipexpr='ABS LL',
#          clipminmax=[0.0,2.0],clipoutside=True)
# flagdata(vis=msfile1,mode='manualflag',timerange='2003/05/02/21:40:58~2003/05/02/22:01:30')
#
# flagdata(vis=msfile1,mode='manualflag', antenna='14',
#          timerange='2003/05/02/18:50:50~2003/05/02/19:13:30')
# flagdata(vis=msfile1,mode='manualflag', antenna='15', spw='0',
#          timerange='2003/05/02/22:38:49~2003/05/02/22:39:11')

# Parallel flagdata execution:
autocorr  = [true, false , false, false , false ]
antenna   = [''  , 'VA09', ''   , 'VA14', 'VA15']
timerange = [''  , ''    , '2003/05/02/21:40:58~2003/05/02/22:01:30', \
                           '2003/05/02/18:50:50~2003/05/02/19:13:30', \
                           '2003/05/02/22:38:49~2003/05/02/22:39:11'] 
spw       = [''  , ''    , ''   , ''    , '0'   ]


###################################################
# Finally, apply all the flag specifications
#

flagdata()


# Record flagging completion time
if benchmarking:
    flagtime = time.time()


#
#=====================================================================
#
# Set the fluxes of the primary calibrator(s)
#
print '--Setjy--'
default('setjy')

setjy(vis=msfile,field='0542+498_1',modimage=modelim) 
setjy(vis=msfile,field='0542+498_2',modimage=modelim)

# Record setjy completion time
if benchmarking:
    setjytime = time.time()

#
#=====================================================================
#
# Gain calibration
#
print '--Gaincal--'
default('gaincal')

gtable = prefix + '.gcal'
gaincal(vis=msfile,caltable=gtable,
    field='0,12,14',spw='0:4~58', gaintype='G',
    opacity=0.06,solint='int',combine='',refant='VA27',minsnr=2.,gaincurve=True)


# gaincal calibration completion time
if benchmarking:
    gaintime = time.time()

#
#=====================================================================
#
# Smoothcal the gain table
#
try:
    
    regstate = True
    ret_status = None
    print '--Smoothcal--'
    default('smoothcal')

    smtable = prefix + '.smoothed'
    ret_status = smoothcal(vis=msfile,tablein=gtable,caltable=smtable,smoothtype='mean',smoothtime=7200.0)
    if ret_status == False :
        regstate = False
        print >> sys.stderr, "Smoothcal failed to execute!"  
    
    # gaincal calibration completion time
    if benchmarking:
        smtime = time.time()
 
    endProc = time.clock()
    endTime = time.time()
 
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

    print >>sys.stdout,'********* Benchmarking *****************'
    print >>sys.stdout,'*                                      *'
    print >>sys.stdout,'Total wall clock time was: '+str(endTime - startTime)
    print >>sys.stdout,'Total CPU        time was: '+str(endProc - startProc)
    print >>sys.stdout,'* Breakdown:                           *'
    print >>sys.stdout,'*   import       time was: '+str(importtime-startTime)
    print >>sys.stdout,'*   flagdata     time was: '+str(flagtime-importtime)
    print >>sys.stdout,'*   setjy        time was: '+str(setjytime-flagtime)
    print >>sys.stdout,'*   gaincal      time was: '+str(gaintime-setjytime)
    print >>sys.stdout,'*   smoothcal    time was: '+str(smtime-gaintime)
    print >>sys.stdout,'*****************************************'

 
except Exception, instance:
    print >> sys.stderr, "Regression test failed for smoothcal instance = ", instance

