##############################################################################
#                                                                            #
# Test Name:                                                                 #
#    Use Case/Regression/Benchmark Script for NGC 5921                       #
# Rationale for Inclusion:                                                   #
#    This script is used as a running example in the CASA cookbook           #
#    (circa 2007). It is included in the regression suite to ensure          #
#    that modifications to CASA do not invalidate the cookbook examples.     #
# Features Tested:                                                           #
#    The script illustrates end-to-end processing with CASA                  #
#    as depicted in the following flow-chart.                                #
#                                                                            #
#    Input Data           Process              Output Data                   #
#                                                                            #
#   NGC5921.fits ----> importuvfits  ------>  ngc5921.ms   +                 #
#   (1.4GHz,                 |                ngc5921.ms.flagversions        #
#    63 sp chan,             v                                               #
#    D-array)             listobs    ------>  casapy.log                     #
#                            |                                               #
#                            v                                               #
#                      flagautocorr                                          #
#                            |                                               #
#                            v                                               #
#                          setjy                                             #
#                            |                                               #
#                            v                                               #
#                         bandpass   ------>  ngc5921.bcal                   #
#                            |                                               #
#                            v                                               #
#                         gaincal    ------>  ngc5921.gcal                   #
#                            |                                               #
#                            v                                               #
#                         listcal    ------>  ngc5921.listcal.out            #
#                            |                                               #
#                            v                                               #
#                        fluxscale   ------>  ngc5921.fluxscale              #
#                            |                                               #
#                            v                                               #
#                        applycal    ------>  ngc5921.ms                     #
#                            |                                               #
#                            v                                               #
#                          split     ------>  ngc5921.cal.split.ms           #
#                            |                                               #
#                            v                                               #
#                          split     ------>  ngc5921.src.split.ms           #
#                            |                                               #
#                            v                                               #
#                      exportuvfits  ------>  ngc5921.split.uvfits           #
#                            |                                               #
#                            v                                               #
#                        uvcontsub   ------>  ngc5921.ms.cont +              #
#                            |                ngc5921.ms.contsub             #
#                            v                                               #
#                         listvis    ------>  ngc5921.listvis.out            #
#                            |                                               #
#                            v                                               #
#                          clean     ------>  ngc5921.clean.image +          #
#                            |                ngc5921.clean.model +          #
#                            |                ngc5921.clean.residual         #
#                            v                                               #
#                       exportfits   ------>  ngc5921.clean.fits             #
#                            |                                               #
#                            v                                               #
#                         imstat     ------>  casapy.log                     #
#                            |                                               #
#                            v                                               #
#                        immoments   ------>  ngc5921.moments.integrated +   #
#                            |                ngc5921.moments.weighted_coord #
#                            v                                               #
#                  Get various statistics                                    #
#               on ms, image, moment images   --> See benchmarking log       #
#               Existence tests for UVFITS                                   #
#                     and image FITS                                         #
# Success/failure criteria:                                                  #
#   Listcal and Listvis output files match output from past runs PASS/FAIL   #
#   Existence of exported UVFITS and FITS image files PASS/FAIL              #
#     UVFITS max value test PASS/FAIL                                        #
#     FITS image max and rms test PASS/FAIL                                  #
#   Cal max ampl, src max ampl, image max and rms tests PASS/FAIL            #
#   Benchmark results compared to past runs by external framework            #
#                                                                            #
##############################################################################
#                                                                            #
# Use Case/Regression/Benchmark Script for NGC 5921                          #
#                                                                            #
# Converted by RRusk 2007-10-31 from ngc5921_usecase.py                      #
# Updated      RRusk 2007-11-01 import regression_utility.py                 #
#                               Tests loading of uvfits file                 #
# Updated     SMyers 2007-11-08 better import/export testing                 #
#                               different pass/fail bookkeeping              #
# Updated      RRusk 2007-11-08 Added some test template info                #
# Updated      RRusk 2007-11-08 More teplate info                            #
# Updated  JCrossley 2008-09-17 Added listvis and listcal tests              #
# Converted by RReid 2010-01-31 from ngc5921_regression.py                   #
# Updated      RReid 2010-05-25 Made listvis use uvcontsub2 output           #
##############################################################################

import os
import time
import regression_utility as tstutl
import listing
import datetime
from callibrary import *

# Enable benchmarking?
benchmarking = True

# Run exportuvfits asynchronously (twice)?
# S.Rankin - Shinosuke Kawakami found
#   1. this test fails when "export_asynchronously = True"
#   2. this value is used to set "async".
#   3. no other test scripts set async = True.
# export_asynchronously = True
export_asynchronously = False

checklistvis=True
# 
# Set up some useful variables
#
# Get path to CASA home directory by stipping name from '$CASAPATH'
pathname=os.environ.get('CASAPATH').split()[0]

# This is where the NGC5921 UVFITS data will be
fitsdata=pathname+'/data/regression/ngc5921/ngc5921.fits'

# The testdir where all output files will be kept
testdir='ngc5921_regression'

# The prefix to use for all output files.
prefix=testdir+"/"+'ngc5921'

# Make new test directory
# (WARNING! Removes old test directory of the same name if one exists)
tstutl.maketestdir(testdir)

datestring = datetime.datetime.isoformat(datetime.datetime.today())
outfile = 'ngc5921.' + datestring + '.log'
n5921reglog = open(outfile, 'w')

def reportresults(redi):
    """
    Helper function to pretty print the results in redi and report whether
    they all passed.  redi is a dict with format
    {test_name1: (pass/fail,
                  (Optional) note,
                  (Even more optional) quantitative difference),
     test_name2: (pass/fail,
                  (Optional) note,
                  (Even more optional) quantitative difference)}
    """
    passfail = {True: '* Passed', False: '* FAILED'}
    normalsevere = {True: 'NORMAL', False: 'SEVERE'}
    ok = True
    
    for t in redi:
        tup = redi[t]
        msg = passfail[tup[0]] + ' ' + t + ' test'
        print >>n5921reglog, msg
        tstutl.note(msg, normalsevere[tup[0]])
        if len(tup) > 1:
            print >>n5921reglog, tup[1]
        #tstutl.note("\"tup[0]\": \"%s\"" % tup[0], "WARN")
        if not tup[0]:
            ok = False
            if len(tup) > 2:
                tstutl.note('  ' + t + " difference: " + str(tup[2]),
                            normalsevere[tup[0]])
    return ok

def listfailures(redi):
    """
    Helper function to summarize any failures in redi at the end.
    redi has the same format as in reportresults.
    """
    for t in redi:
        tup = redi[t]
        if not tup[0]:
            msg = t + "  FAILED"
            if len(tup) > 2:
                msg += ":\n  " + str(tup[1]) + "\n    difference: " + str(tup[2])
            tstutl.note(msg, "SEVERE")
    
# Start benchmarking
if benchmarking:
    startTime = time.time()
    startProc = time.clock()

passedall = True  # So far!

#
#=====================================================================
#
# Import the data from FITS to MS
#
print '--Import--'

# Safest to start from task defaults
default('importuvfits')

# Set up the MS filename and save as new global variable
msfile = prefix + '.ms'

# Use task importuvfits
fitsfile = fitsdata
vis = msfile
antnamescheme = 'new'
importuvfits()

# Note that there will be a ngc5921.ms.flagversions
# containing the initial flags as backup for the main ms flags.

# Record import time
if benchmarking:
    importtime = time.time()

#
#=====================================================================
#
# List a summary of the MS
#
print '--Listobs--'

# Don't default this one.  Make use of the previous setting of
# vis.  Remember, the variables are GLOBAL!

# You may wish to see more detailed information, like the scans.
# In this case use the verbose = True option
verbose = True

listobs()

# You should get in your logger window and in the casapy.log file
# something like:
#
# MeasurementSet Name:  /home/sandrock2/smyers/Testing2/Sep07/ngc5921_regression/ngc5921.ms
# MS Version 2
# 
# Observer: TEST     Project:   
# Observation: VLA
# 
# Data records: 22653       Total integration time = 5280 seconds
#    Observed from   09:19:00   to   10:47:00
# 
#    ObservationID = 0         ArrayID = 0
#   Date        Timerange                Scan  FldId FieldName      SpwIds
#   13-Apr-1995/09:19:00.0 - 09:24:30.0     1      0 1331+30500002_0  [0]
#               09:27:30.0 - 09:29:30.0     2      1 1445+09900002_0  [0]
#               09:33:00.0 - 09:48:00.0     3      2 N5921_2        [0]
#               09:50:30.0 - 09:51:00.0     4      1 1445+09900002_0  [0]
#               10:22:00.0 - 10:23:00.0     5      1 1445+09900002_0  [0]
#               10:26:00.0 - 10:43:00.0     6      2 N5921_2        [0]
#               10:45:30.0 - 10:47:00.0     7      1 1445+09900002_0  [0]
# 
# Fields: 3
#   ID   Code Name          Right Ascension  Declination   Epoch   
#   0    C    1331+30500002_013:31:08.29      +30.30.32.96  J2000   
#   1    A    1445+09900002_014:45:16.47      +09.58.36.07  J2000   
#   2         N5921_2       15:22:00.00      +05.04.00.00  J2000   
# 
# Spectral Windows:  (1 unique spectral windows and 1 unique polarization setups)
#   SpwID  #Chans Frame Ch1(MHz)    Resoln(kHz) TotBW(kHz)  Ref(MHz)    Corrs   
#   0          63 LSRK  1412.68608  24.4140625  1550.19688  1413.44902  RR  LL  
# 
# Feeds: 28: printing first row only
#   Antenna   Spectral Window     # Receptors    Polarizations
#   1         -1                  2              [         R, L]
# 
# Antennas: 27:
#   ID   Name  Station   Diam.    Long.         Lat.         
#   0    1     VLA:N7    25.0 m   -107.37.07.2  +33.54.12.9  
#   1    2     VLA:W1    25.0 m   -107.37.05.9  +33.54.00.5  
#   2    3     VLA:W2    25.0 m   -107.37.07.4  +33.54.00.9  
#   3    4     VLA:E1    25.0 m   -107.37.05.7  +33.53.59.2  
#   4    5     VLA:E3    25.0 m   -107.37.02.8  +33.54.00.5  
#   5    6     VLA:E9    25.0 m   -107.36.45.1  +33.53.53.6  
#   6    7     VLA:E6    25.0 m   -107.36.55.6  +33.53.57.7  
#   7    8     VLA:W8    25.0 m   -107.37.21.6  +33.53.53.0  
#   8    9     VLA:N5    25.0 m   -107.37.06.7  +33.54.08.0  
#   9    10    VLA:W3    25.0 m   -107.37.08.9  +33.54.00.1  
#   10   11    VLA:N4    25.0 m   -107.37.06.5  +33.54.06.1  
#   11   12    VLA:W5    25.0 m   -107.37.13.0  +33.53.57.8  
#   12   13    VLA:N3    25.0 m   -107.37.06.3  +33.54.04.8  
#   13   14    VLA:N1    25.0 m   -107.37.06.0  +33.54.01.8  
#   14   15    VLA:N2    25.0 m   -107.37.06.2  +33.54.03.5  
#   15   16    VLA:E7    25.0 m   -107.36.52.4  +33.53.56.5  
#   16   17    VLA:E8    25.0 m   -107.36.48.9  +33.53.55.1  
#   17   18    VLA:W4    25.0 m   -107.37.10.8  +33.53.59.1  
#   18   19    VLA:E5    25.0 m   -107.36.58.4  +33.53.58.8  
#   19   20    VLA:W9    25.0 m   -107.37.25.1  +33.53.51.0  
#   20   21    VLA:W6    25.0 m   -107.37.15.6  +33.53.56.4  
#   21   22    VLA:E4    25.0 m   -107.37.00.8  +33.53.59.7  
#   23   24    VLA:E2    25.0 m   -107.37.04.4  +33.54.01.1  
#   24   25    VLA:N6    25.0 m   -107.37.06.9  +33.54.10.3  
#   25   26    VLA:N9    25.0 m   -107.37.07.8  +33.54.19.0  
#   26   27    VLA:N8    25.0 m   -107.37.07.5  +33.54.15.8  
#   27   28    VLA:W7    25.0 m   -107.37.18.4  +33.53.54.8  
# 
# Tables:
#    MAIN                   22653 rows     
#    ANTENNA                   28 rows     
#    DATA_DESCRIPTION           1 row      
#    DOPPLER             <absent>  
#    FEED                      28 rows     
#    FIELD                      3 rows     
#    FLAG_CMD             <empty>  
#    FREQ_OFFSET         <absent>  
#    HISTORY                  273 rows     
#    OBSERVATION                1 row      
#    POINTING                 168 rows     
#    POLARIZATION               1 row      
#    PROCESSOR            <empty>  
#    SOURCE                     3 rows     
#    SPECTRAL_WINDOW            1 row      
#    STATE                <empty>  
#    SYSCAL              <absent>  
#    WEATHER             <absent>  
# 

# Record listing completion time
if benchmarking:
    listtime = time.time()

#
#=====================================================================
#
# Get rid of the autocorrelations from the MS
#
#print '--Flagautocorr--'
print '--Flag auto-correlations--'

# Don't default this one either, there is only one parameter (vis)
default(flagdata)
vis = msfile
autocorr = True
flagbackup = False
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

vis = msfile

#
# 1331+305 = 3C286 is our primary calibrator
# Use the wildcard on the end of the source name
# since the field names in the MS have inherited the
# AIPS qualifiers
field = '1331+305*'

# This is 1.4GHz D-config and 1331+305 is sufficiently unresolved
# that we do not need a model image.  For higher frequencies
# (particularly in A and B config) you would want to use one.
modimage = ''

# Setjy knows about this source so we don't need anything more
standard='Perley-Taylor 99'  # enforce older standard

scalebychan=False

usescratch=False

setjy()

#
# You should see something like this in the logger and casapy.log file:
#
# 1331+30500002_0  spwid=  0  [I=14.76, Q=0, U=0, V=0] Jy, (Perley-Taylor 99)
#
# So its using 14.76Jy as the flux of 1331+305 in the single Spectral Window
# in this MS.

# Record setjy completion time
if benchmarking:
    setjytime = time.time()


# the callibrary into which we will deposit calibration results
c=callibrary()


#
#=====================================================================
#
# Bandpass calibration
#
print '--Bandpass--'
default('bandpass')

# We can first do the bandpass on the single 5min scan on 1331+305
# At 1.4GHz phase stablility should be sufficient to do this without
# a first (rough) gain calibration.  This will give us the relative
# antenna gain as a function of frequency.

vis = msfile

# set the name for the output bandpass caltable
btable = prefix + '.bcal'
caltable = btable

# Use flux calibrator 1331+305 = 3C286 (FIELD_ID 0) as bandpass calibrator
field = '0'
# all channels
spw = ''
# No other selection
selectdata = False

# Choose bandpass solution type
# Pick standard time-binned B (rather than BPOLY)
bandtype = 'B'

# set solution interval arbitrarily long (get single bpass)
solint = 'inf'
combine='scan'

# reference antenna Name 15 (15=VLA:N2) (Id 14)
refant = 'VA15'

bandpass()

# You can use plotcal to examine the solutions
#default('plotcal')
#caltable = btable
#yaxis = 'amp'
#field = '0'
#overplot = True
#plotcal()
#
#yaxis = 'phase'
#plotcal()
#
# Note the rolloff in the start and end channels.  Looks like
# channels 6-56 (out of 0-62) are the best

# add this to the callibrary
c.add(caltable=btable,tinterp='nearest',calwt=T)

# bandpass calibration completion time
if benchmarking:
    bptime = time.time()

#
#=====================================================================
#
# Gain calibration
#
print '--Gaincal--'
default('gaincal')

# Armed with the bandpass, we now solve for the
# time-dependent antenna gains

vis = msfile

# set the name for the output gain caltable
gtable = prefix + '.gcal'
caltable = gtable

# Gain calibrators are 1331+305 and 1445+099 (FIELD_ID 0 and 1)
field = '0,1'

# We have only a single spectral window (SPW 0)
# Choose 51 channels 6-56 out of the 63
# to avoid end effects.
# Channel selection is done inside spw
spw = '0:6~56'

# No other selection
selectdata = False

# scan-based G solutions for both amplitude and phase
gaintype = 'G'
solint = 'inf'
combine=''
calmode = 'ap'

# minimum SNR allowed
minsnr = 1.0

# reference antenna 15 (15=VLA:N2)
refant = '15'

# Turn on cal library apply
docallib=T
callib=c.cld

gaincal()

# You can use plotcal to examine the gain solutions
#default('plotcal')
#caltable = gtable
#yaxis = 'amp'
#field = '0,1'
#overplot = True
#plotcal()
#
#yaxis = 'phase'
#plotcal()
#
# The amp and phase coherence looks good


# add this gain result to the callibrary
c.add(caltable=gtable,field='0',fldmap=[0],tinterp='nearest',calwt=T)
c.add(caltable=gtable,field='1,2',fldmap='1',tinterp='linear',calwt=T)


# gaincal calibration completion time
if benchmarking:
    gaintime = time.time()

#
#=====================================================================
# List calibration solutions
#
print '--Listcal--'
listcalOut = prefix + '.listcal.out'

default('listcal')
vis = msfile
caltable = gtable
listfile = listcalOut
listcal()

# Record calibration listing time
if benchmarking:
    listcaltime = time.time()

# Before testing listcal output, remove first line of file
# (First line contains hard-coded path to input files)
os.system('mv ' + listcalOut + ' ' + listcalOut + '.tmp')
os.system('tail -n +2 ' + listcalOut + '.tmp > ' + listcalOut)
os.system('rm -f ' + listcalOut + '.tmp')

# Test the listcal output
print "Comparing listcal output with standard..."
standardOut = pathname+'/data/regression/ngc5921/listcal.default.out'
listcalresults = {}
try:
    print "  1. Checking that metadata agree..."
    listcalresults['listcal metadata'] = (listing.diffMetadata(listcalOut,
                                                               standardOut,
                                                               prefix=prefix + ".listcal"),)
    # Test data (floats)
    print "  2. Checking that data agree to within allowed imprecision..."
    precision = '0.003'
    print "     Allowed visibility imprecision is " + precision
    listcalresults['listcal data'] = (listing.diffAmpPhsFloat(listcalOut,
                                                              standardOut,
                                                              prefix = prefix+".listcal",
                                                              precision = precision),)
    passedall = reportresults(listcalresults) and passedall
except Exception, e:
    print "Error", e, "checking listcal."
    raise e

#
#=====================================================================
#
# Bootstrap flux scale
#
print '--Fluxscale--'
default('fluxscale')

vis = msfile

# set the name for the output rescaled caltable
ftable = prefix + '.fluxscale'
fluxtable = ftable

# point to our first gain cal table
caltable = gtable

# we will be using 1331+305 (the source we did setjy on) as
# our flux standard reference - note its extended name as in
# the FIELD table summary above (it has a VLA seq number appended)
reference = '1331*'

# we want to transfer the flux to our other gain cal source 1445+099
transfer = '1445*'

# testing new option
incremental=True

fluxscale()

# In the logger you should see something like:
# Flux density for 1445+09900002_0 in SpW=0 is:
#     2.48576 +/- 0.00123122 (SNR = 2018.94, nAnt= 27)

# If you run plotcal() on the caltable = 'ngc5921.fluxscale'
# you will see now it has brought the amplitudes in line between
# the first scan on 1331+305 and the others on 1445+099

# Add this to the cal library

c.add(caltable=ftable,fldmap='nearest',tinterp='nearest',calwt=T)

# Record fluxscale completion time
if benchmarking:
    fstime = time.time()

#
#=====================================================================
#
# Apply our calibration solutions to the data
# (This will put calibrated data into the CORRECTED_DATA column)
#
print '--ApplyCal--'
default('applycal')

vis = msfile

# We want to correct the calibrators using themselves
# and transfer from 1445+099 to itself and the target N5921

# all channels
spw = ''
selectdata = False

# select all fields
field = ''

# Turn on the cal library
docallib=T
callib=c.cld

applycal()

# Record applycal completion time
if benchmarking:
    correcttime = time.time()

#
#=====================================================================
#
# Split the gain calibrater data, then the target
#
print '--Split 1445+099 Data--'
default('split')

vis = msfile

# We first want to write out the corrected data for the calibrator

# Make an output vis file
calsplitms = prefix + '.cal.split.ms'
outputvis = calsplitms

# Select the 1445+099 field, all chans
field = '1445*'
spw = ''

# pick off the CORRECTED_DATA column
datacolumn = 'corrected'

split()

# Record split cal completion time
if benchmarking:
    splitcaltime = time.time()

#
# Now split NGC5921 data (before continuum subtraction)
#
print '--Split NGC5921 Data--'

splitms = prefix + '.src.split.ms'
outputvis = splitms

# Pick off N5921 
field = 'N5921*'

split()

# Record split src data completion time
if benchmarking:
    splitsrctime = time.time()

#
#=====================================================================
#
# Export the NGC5921 data as UVFITS
# Start with the split file.
#
print '--Export UVFITS--'
default('exportuvfits')

srcuvfits = prefix + '.split.uvfits'

vis = splitms
fitsfile = srcuvfits

# Since this is a split dataset, the calibrated data is
# in the DATA column already.
datacolumn = 'data'

# Write as a multisource UVFITS (with SU table)
# even though it will have only one field in it
multisource = True

# Run asynchronously so as not to interfere with other tasks
# (BETA: also avoids crash on next importuvfits)
#async = True
async = export_asynchronously

async_exportuvfits_id = exportuvfits()
print "async_exportuvfits_id =", async_exportuvfits_id

# Record exportuvfits completion time
# NOTE: If async=true this can't be used to time exportuvfits
#       but it is still needed as a start time for uvcontsub.
if benchmarking:
    exportuvfitstime = time.time()

#
#=====================================================================
#
# UV-plane continuum subtraction on the target
# (this will update the CORRECTED_DATA column)
#
print '--UV Continuum Subtract--'
default('uvcontsub')

vis = msfile

# Pick off N5921 
field = 'N5921*'

# Use channels 4-6 and 50-59 for continuum fit
fitspw='0:4~6;50~59'

# Subtr and write out all of spw=0
spw = '0'

# Averaging time (per integration)
solint = 'int'

# Fit only a mean level
fitorder = 0

# Let it split out the data automatically for us
want_cont = True

uvcontsub()

# You will see it made two new MS:
# ngc5921.ms.cont         (Continuum estimate)
# ngc5921.ms.contsub      (Continuum subtracted)

srcsplitms = msfile + '.contsub'

# Record continuum subtraction time
if benchmarking:
    contsubtime = time.time()

if checklistvis:
    #=====================================================================
    # List corrected data in MS
    #
    print '--Listvis--'
    listvisOut = prefix + '.listvis.out'

    default('listvis')
#    vis = srcsplitms
#    datacolumn = 'data'
#    selectdata=True
#    antenna='VA03&VA04'
#    listfile=listvisOut
    print "Listing corrected data."
    print "Reducing output by selecting only baseline 3&4."
#    listvis()
    listvis(vis=srcsplitms,datacolumn='data',selectdata=True,antenna='VA03&VA04',listfile=listvisOut)

    # Record visibility listing time
    if benchmarking:
        listvistime = time.time()

    # Test the listvis output
    print "Comparing continuum subtracted listvis output with repository standard..."
    standardOut = pathname+'/data/regression/ngc5921/listvis.ant34.contsub.out'

    # Test metadata
    print "  Checking that metadata agree and that data agree to within allowed imprecision..."
    precision = '0.200'
    print "     Allowed visibility imprecision is ", precision
    listvisresults = {}
    try:
        listvisresults['listvis metadata'] = (listing.diffMetadata(listvisOut,
                                                                   standardOut,
                                                                   prefix=prefix + ".listvis"),)
        listvisresults['listvis data'] = (listing.diffAmpPhsFloat(listvisOut,
                                                                  standardOut,
                                                                  prefix=prefix + ".listvis",
                                                                  precision=precision),)
        passedall = reportresults(listvisresults) and passedall
    except Exception, e:
        print "Error", e, "checking listvis."
        raise e               

#=====================================================================
#
# Done with calibration
# Now clean an image cube of N5921
#
print '--Clean--'
default('clean')

# Pick up our split source data
vis = srcsplitms

# Make an image root file name
imname = prefix + '.clean'
imagename = imname

# Set up the output image cube
mode = 'channel'
nchan = 46
start = 5
width = 1

# This is a single-source MS with one spw
field = '0'
spw = ''

# Standard gain factor 0.1
gain = 0.1

# Set the output image size and cell size (arcsec)
#imsize = [256,256]

# Do a simple Clark clean
psfmode = 'clark'
imagermode = ''
# If desired, you can do a Cotton-Schwab clean
# but will have only marginal improvement for this data
#alg = 'csclean'
# Twice as big for Cotton-Schwab (cleans inner quarter)
#imsize = [512,512]

# Pixel size 15 arcsec for this data (1/3 of 45" beam)
# VLA D-config L-band
cell = [15.,15.]

# Fix maximum number of iterations
niter = 6000

# Also set flux residual threshold (in mJy)
threshold=8.0

# Set up the weighting
# Use Briggs weighting (a moderate value, on the uniform side)
weighting = 'briggs'
#rmode = 'norm'
robust = 0.5

# No clean mask or cleanbox for now
mask = ''
#cleanbox = []

# But if you had a cleanbox saved in a file, e.g. "regionfile.txt"
# you could use it:
#cleanbox='regionfile.txt'
#
# and if you wanted to use interactive clean
#cleanbox='interactive'

usescratch=False

clean()

# Should find stuff in the logger like:
#
# Fitted beam used in restoration: 51.5643 by 45.6021 (arcsec)
#     at pa 14.5411 (deg)
#
# It will have made the images:
# -----------------------------
# ngc5921.clean.image
# ngc5921.clean.model
# ngc5921.clean.residual

clnimage = imname+'.image'

# Record clean completion time
if benchmarking:
    cleantime = time.time()

#
#=====================================================================
#
# Done with imaging
# Now view the image cube of N5921
#
#print '--View image--'
#viewer(clnimage,'image')

#=====================================================================
#
# Export the Final CLEAN Image as FITS
#
print '--Final Export CLEAN FITS--'
default('exportfits')

clnfits = prefix + '.clean.fits'

imagename = clnimage
fitsimage = clnfits

# Run asynchronously so as not to interfere with other tasks
# (BETA: also avoids crash on next importfits)
#async = True
async = export_asynchronously

async_clean_id = exportfits()

# Record export FITS image completion time
# NOTE: this is currently irrelevant because its async
if benchmarking:
    exportimtime = time.time()

#
#=====================================================================
#
# Get some image statistics
#
print '--Imstat--'
default('imstat')

imagename = clnimage

cubestats =imstat()

# A summary of the cube will be seen in the logger
# cubestats will contain a dictionary of the statistics

#
#=====================================================================
#
# Get some image moments
#
print '--ImMoments--'
default('immoments')

imagename = clnimage

# Do first and second moments
moments = [0,1]

# Need to mask out noisy pixels, currently done
# using hard global limits
excludepix = [-100,0.009]

# Include all planes
chans = ''

# Output root name
momfile = prefix + '.moments'
outfile = momfile

# execute, expecting success
if not immoments():
    raise Exception, 'Execution of immoments() returned False.'

momzeroimage = momfile + '.integrated'
momoneimage = momfile + '.weighted_coord'

# It will have made the images:
# --------------------------------------
# ngc5921.moments.integrated
# ngc5921.moments.weighted_coord

#
#=====================================================================
#
# Get some statistics of the moment images
#
print '--Imstat--'
default('imstat')

imagename = momzeroimage
momzerostats = imstat()

imagename = momoneimage
momonestats = imstat()

#
#=====================================================================
#
# Can do some image statistics if you wish
# Treat this like a regression script
# WARNING: currently requires toolkit
#
print ""
print ' NGC5921 results '
print ' =============== '

print ''
print ' --Regression Tests--'
print ''
#
# Use the ms tool to get max of the MSs
# Eventually should be available from a task
#
# Pull the max cal amp value out of the MS
ms.open(calsplitms)
thistest_cal = max(ms.range(["amplitude"]).get('amplitude'))
ms.close()
oldtest_cal = 34.0338668823
print ' Cal Max amplitude should be ',oldtest_cal
print ' Found : Max = ',thistest_cal
diff_cal = abs((oldtest_cal-thistest_cal)/oldtest_cal)
print ' Difference (fractional) = ',diff_cal

print ''
# Pull the max src amp value out of the MS
ms.open(srcsplitms)
thistest_src = max(ms.range(["amplitude"]).get('amplitude'))
ms.close()
#oldtest_src =  1.37963354588 # This was in chans 5-50
oldtest_src =  46.2060050964 # now in all chans
print ' Src Max amplitude should be ',oldtest_src
print ' Found : Max = ',thistest_src
diff_src = abs((oldtest_src-thistest_src)/oldtest_src)
print ' Difference (fractional) = ',diff_src

#
# Now use the stats produced by imstat above
#
print ''
# Pull the max from the cubestats dictionary
# created above using imstat
thistest_immax=cubestats['max'][0]
oldtest_immax = 0.052414759993553162
print ' Clean image max should be ',oldtest_immax
print ' Found : Image Max = ',thistest_immax
diff_immax = abs((oldtest_immax-thistest_immax)/oldtest_immax)
print ' Difference (fractional) = ',diff_immax

print ''
# Pull the rms from the cubestats dictionary
thistest_imrms=cubestats['rms'][0]
oldtest_imrms = 0.0020064469
print ' Clean image rms should be ',oldtest_imrms
print ' Found : Image rms = ',thistest_imrms
diff_imrms = abs((oldtest_imrms-thistest_imrms)/oldtest_imrms)
print ' Difference (fractional) = ',diff_imrms

print ''
# Pull the max from the momzerostats dictionary
thistest_momzeromax=momzerostats['max'][0]
oldtest_momzeromax = 1.4868
print ' Moment 0 image max should be ',oldtest_momzeromax
print ' Found : Moment 0 Max = ',thistest_momzeromax
diff_momzeromax = abs(1.0 - thistest_momzeromax / oldtest_momzeromax)
print ' Difference (fractional) = ',diff_momzeromax

print ''
# Pull the mean from the momonestats dictionary
thistest_momoneavg=momonestats['mean'][0]
oldtest_momoneavg = 1486.8473
print ' Moment 1 image mean should be ',oldtest_momoneavg
print ' Found : Moment 1 Mean = ',thistest_momoneavg
diff_momoneavg = abs((oldtest_momoneavg-thistest_momoneavg)/oldtest_momoneavg)
print ' Difference (fractional) = ',diff_momoneavg


# Record processing completion time
if benchmarking:
    endProc = time.clock()
    endTime = time.time()

#
#=====================================================================
# Export tests
#
print ''
print ' --Export Tests--'
print ''
#
# First the UVFITS
#
tstutl.note("Opening UVFITS file " + srcuvfits +
                " to verify its existence")

if export_asynchronously:
    while True:
        try:
            if tm.retrieve(async_exportuvfits_id):
                break
            else:
                time.sleep(1)
        except Exception, e:
            tstutl.note("Error checking whether exportuvfits finished.",
                        "SEVERE")
            tstutl.note("async_exportuvfits_id was " + str(async_exportuvfits_id), "SEVERE")
            raise e

uvfitsexists=False
if os.path.isfile(srcuvfits):
    uvfitsexists=True

diff_uvfits = 1.0
if uvfitsexists:
    tstutl.note("Opening the UVFITS file", "NORMAL")
    try:
        ms.fromfits(prefix+".fromuvfits.ms", srcuvfits)
        ms.summary()
        # Pull the max src amp value out of the MS
        fitstest_src = max(ms.range(["amplitude"]).get('amplitude'))
        ms.close()
        tstutl.note("Verified that a valid UVFITS file was written")
        print ''
        # Test the max in MS
        # oldtest_src =  46.2060050964
        print ' UVFITS Src Max amplitude should be ', thistest_src
        print ' Found : UVFITS Max = ', fitstest_src
        diff_uvfits = abs((fitstest_src - thistest_src) / thistest_src)
        print ' Difference (fractional) = ', diff_uvfits
    except Exception, e:
        tstutl.note("Failed to open UVFITS file because: "+e, "SEVERE")
else:
    tstutl.note("Could not open the UVFITS file", "SEVERE")

#
# Now the Clean FITS
#
print ''
tstutl.note("Opening FITS image file " + fitsimage +
                " to verify its existence")

if export_asynchronously:
    while True:
        if tm.retrieve(async_clean_id):
            break
        else:
            time.sleep(1)

fitsimageexists=False
if os.path.isfile(fitsimage):
    fitsimageexists=True

diff_fitsmax = 1.0
diff_fitsrms = 1.0
if fitsimageexists:
    tstutl.note("Opening the FITS image", "NORMAL")
    try:
        default('imstat')
        imagename = fitsimage
        fitstats = imstat()
        tstutl.note("Verified that a valid FITS image file was written")
        print ''
        # Pull the max from the fitstats dictionary
        fitstest_immax=fitstats['max'][0]
        print ' FITS image max should be ',thistest_immax
        print ' Found : FITS Image Max = ',fitstest_immax
        diff_fitsmax = abs((fitstest_immax-thistest_immax)/thistest_immax)
        print ' Difference (fractional) = ',diff_fitsmax
        print ''
        # Pull the rms from the fitstats dictionary
        fitstest_imrms=fitstats['rms'][0]
        print ' FITS image rms should be ',thistest_imrms
        print ' Found : FITS Image rms = ',fitstest_imrms
        diff_fitsrms = abs((fitstest_imrms-thistest_imrms)/thistest_imrms)
        print ' Difference (fractional) = ',diff_fitsrms
    except Exception, e:
        tstutl.note("Failed to open FITS image file because: "+e,"SEVERE")
else:
    tstutl.note("Could not open the FITS image", "SEVERE")

#
#=====================================================================
#
if not benchmarking:
    print ''
    print '--- Done ---'
else:
    print >>n5921reglog,''
    print >>n5921reglog,''
    print >>n5921reglog,'********** Data Summary *********'
    print >>n5921reglog,'*   Observer: TEST     Project:                                             *'
    print >>n5921reglog,'* Observation: VLA(28 antennas)                                             *'
    print >>n5921reglog,'* Data records: 22653  Total integration time = 5280 seconds                *'
    print >>n5921reglog,'* Observed from   09:19:00   to   10:47:00                                  *'
    print >>n5921reglog,'* Fields: 3                                                                 *'
    print >>n5921reglog,'*  ID   Name          Right Ascension  Declination   Epoch                  *'
    print >>n5921reglog,'*  0    1331+30500002_013:31:08.29      +30.30.32.96  J2000                 *'
    print >>n5921reglog,'*  1    1445+09900002_014:45:16.47      +09.58.36.07  J2000                 *'
    print >>n5921reglog,'*  2    N5921_2       15:22:00.00      +05.04.00.00  J2000                  *'
    print >>n5921reglog,'* Data descriptions: 1 (1 spectral windows and 1 polarization setups)       *'
    print >>n5921reglog,'*  ID  #Chans Frame Ch1(MHz)    Resoln(kHz) TotBW(kHz)  Ref(MHz)    Corrs   *'  
    print >>n5921reglog,'*  0       63 LSRK  1412.68608  24.4140625  1550.19688  1413.44902  RR  LL  *'
    print >>n5921reglog,'*********************************'
    print ""
    print >>n5921reglog,''
    print >>n5921reglog,'********* Export Tests***********'
    print >>n5921reglog,'*                               *'

    exportresults = {'UVFITS existence': (uvfitsexists,),
                     'FITS image existence': (fitsimageexists,)}
    if uvfitsexists:
        exportresults['UVFITS max'] = (diff_uvfits < 0.05,
                                       '*  UVFITS MS max ' + str(fitstest_src), diff_uvfits)

    if (fitsimageexists):
        exportresults['FITS image max'] = (diff_fitsmax < 0.05,
                                           '*  FITS Image max ' + str(fitstest_immax),
                                           diff_fitsmax)
        exportresults['FITS image rms'] = (diff_fitsrms < 0.05,
                                           '*  FITS Image rms ' + str(fitstest_imrms),
                                           diff_fitsrms)
    passedall = reportresults(exportresults) and passedall

    print >>n5921reglog,''
    print >>n5921reglog,'********** Regression ***********'
    print >>n5921reglog,'*                               *'
    quantresults = {}
    quantresults['cal max amplitude'] = (diff_cal < 0.05,
                                         '*  Cal max amp ' + str(thistest_cal), diff_cal)
    quantresults['src max amplitude'] = (diff_src < 0.05,
                                         '*  Src max amp ' + str(thistest_src), diff_src)
    quantresults['image max'] = (diff_immax < 0.05,
                                 '*  Image max amp ' + str(thistest_immax), diff_immax)
    quantresults['image rms'] = (diff_imrms < 0.05,
                                 '*  Image rms ' + str(thistest_imrms), diff_imrms)
    passedall = reportresults(quantresults) and passedall

    if passedall: 
	regstate=True
	print >>n5921reglog,'---'
	print >>n5921reglog,'Passed Regression test for NGC5921'
	print >>n5921reglog,'---'
        print ''
        print 'Regression PASSED'
        print ''
        tstutl.note("Passed Regression test for NGC5921","NORMAL")
    else: 
	regstate=False
	print >>n5921reglog,'---'
	print >>n5921reglog,'FAILED Regression test for NGC5921'
	print >>n5921reglog,'---'
        print ''
        print 'Regression FAILED'
        print ''
        tstutl.note("FAILED Regression test for NGC5921","SEVERE")
        # Specify what failed here...just saying "FAILED" is aggravating.
        for d in (listvisresults, listcalresults, exportresults, quantresults):
            listfailures(d)
        
    print >>n5921reglog,'*********************************'

    print >>n5921reglog,''
    print >>n5921reglog,''
    print >>n5921reglog,'********* Benchmarking *****************'
    print >>n5921reglog,'*                                      *'
    print >>n5921reglog,'Total wall clock time was: ', endTime - startTime
    print >>n5921reglog,'Total CPU        time was: ', endProc - startProc
    print >>n5921reglog,'Processing rate MB/s  was: ', 35.1/(endTime - startTime)
    print >>n5921reglog,'* Breakdown:                           *'
    print >>n5921reglog,'*   import       time was: '+str(importtime-startTime)
    print >>n5921reglog,'*   flagautocorr time was: '+str(flagtime-listtime)
    print >>n5921reglog,'*   setjy        time was: '+str(setjytime-flagtime)
    print >>n5921reglog,'*   bandpass     time was: '+str(bptime-setjytime)
    print >>n5921reglog,'*   gaincal      time was: '+str(gaintime-bptime)
    print >>n5921reglog,'*   listcal      time was: '+str(listcaltime-gaintime)    
    print >>n5921reglog,'*   fluxscale    time was: '+str(fstime-listcaltime)
    print >>n5921reglog,'*   applycal     time was: '+str(correcttime-fstime)
    print >>n5921reglog,'*   split-cal    time was: '+str(splitcaltime-correcttime)
    print >>n5921reglog,'*   split-src    time was: '+str(splitsrctime-splitcaltime)
    print >>n5921reglog,'*   contsub      time was: '+str(contsubtime-exportuvfitstime)
    print >>n5921reglog,'*   listvis      time was: '+str(listvistime-contsubtime)
    print >>n5921reglog,'*   clean        time was: '+str(cleantime-listvistime)
    print >>n5921reglog,'*****************************************'
    #print >>n5921reglog,'basho (test cpu) time was: ?? seconds'

    print ""
    print "Done!"

n5921reglog.close()

#exit()
#
#=====================================================================
