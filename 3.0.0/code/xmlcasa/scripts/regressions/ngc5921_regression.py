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
##############################################################################

import os
import time
import regression_utility as tstutl
import listing

# Enable benchmarking?
benchmarking = True

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

# Start benchmarking
if benchmarking:
    startTime = time.time()
    startProc = time.clock()

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
print '--Flagautocorr--'

# Don't default this one either, there is only one parameter (vis)

flagautocorr()

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

# No gain tables yet
gaintable = ''
gainfield = ''
interp = ''

# Use flux calibrator 1331+305 = 3C286 (FIELD_ID 0) as bandpass calibrator
field = '0'
# all channels
spw = ''
# No other selection
selectdata = False

# In this band we do not need a-priori corrections for
# antenna gain-elevation curve or atmospheric opacity
# (at 8GHz and above you would want these)
gaincurve = False
opacity = 0.0

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

# Use our previously determined bandpass
# Note this will automatically be applied to all sources
# not just the one used to determine the bandpass
gaintable = btable
gainfield = ''

# Use nearest (there is only one bandpass entry)
interp = 'nearest'

# Gain calibrators are 1331+305 and 1445+099 (FIELD_ID 0 and 1)
field = '0,1'

# We have only a single spectral window (SPW 0)
# Choose 51 channels 6-56 out of the 63
# to avoid end effects.
# Channel selection is done inside spw
spw = '0:6~56'

# No other selection
selectdata = False

# In this band we do not need a-priori corrections for
# antenna gain-elevation curve or atmospheric opacity
# (at 8GHz and above you would want these)
gaincurve = False
opacity = 0.0

# scan-based G solutions for both amplitude and phase
gaintype = 'G'
solint = 'inf'
combine=''
calmode = 'ap'

# minimum SNR allowed
minsnr = 1.0

# reference antenna 15 (15=VLA:N2)
refant = '15'

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

# gaincal calibration completion time
if benchmarking:
    gaintime = time.time()

#
#=====================================================================
# List calibration solutions
#
print '--Listcal--'
listcalOut = prefix + '.listcal.out'

import listing 

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
passlistcal = True

# Test metadata
print "  1. Checking that metadata agree."
if (listing.diffMetadata(listcalOut,standardOut,prefix=prefix+".listcal")):
    print "  Metadata agree"
else:
    print "  Metadata do not agree!"
    passlistcal = False

# Test data (floats)
print "  2. Checking that data agree to within allowed imprecision..."
precision = '0.003'
print "     Allowed visibility imprecision is " + precision
if ( listing.diffAmpPhsFloat(listcalOut,standardOut,prefix=prefix+".listcal",
                             precision=precision) ):
    print "  Data agree"
else:
    print "  Data do not agree!"
    passlistcal = False

if (passlistcal): print "Passed listcal output test"
else:             print "FAILED listcal output test"
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

fluxscale()

# In the logger you should see something like:
# Flux density for 1445+09900002_0 in SpW=0 is:
#     2.48576 +/- 0.00123122 (SNR = 2018.94, nAnt= 27)

# If you run plotcal() on the caltable = 'ngc5921.fluxscale'
# you will see now it has brought the amplitudes in line between
# the first scan on 1331+305 and the others on 1445+099

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

# Start with the fluxscale/gain and bandpass tables
gaintable = [ftable,btable]

# pick the 1445+099 out of the gain table for transfer
# use all of the bandpass table
gainfield = ['1','*']

# interpolation using linear for gain, nearest for bandpass
interp = ['linear','nearest']

# only one spw, do not need mapping
spwmap = []

# all channels
spw = ''
selectdata = False

# as before
gaincurve = False
opacity = 0.0

# select the fields for 1445+099 and N5921
field = '1,2'

applycal()

# Now for completeness apply 1331+305 to itself

field = '0'
gainfield = ['0','*']

# The CORRECTED_DATA column now contains the calibrated visibilities

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
async = True

async_split_id = exportuvfits()

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

# Do the uv-plane subtraction
fitmode = 'subtract'

# Let it split out the data automatically for us
splitdata = True

uvcontsub()

# You will see it made two new MS:
# ngc5921.ms.cont
# ngc5921.ms.contsub

srcsplitms = msfile + '.contsub'

# Note that ngc5921.ms.contsub contains the uv-subtracted
# visibilities (in its DATA column), and ngc5921.ms.cont
# the pseudo-continuum visibilities (as fit).

# The original ngc5921.ms now contains the uv-continuum
# subtracted vis in its CORRECTED_DATA column and the continuum
# in its MODEL_DATA column as per the fitmode='subtract'

# Record continuum subtraction time
if benchmarking:
    contsubtime = time.time()

#
#=====================================================================
# List corrected data in MS
#
print '--Listvis--'
listvisOut = prefix + '.listvis.out'

default('listvis')
vis = msfile
datacolumn = 'corrected'
selectdata=True
antenna='VA03&VA04'
listfile = listvisOut
print "Listing corrected data."
print "Reducing output by selecting only baseline 3&4."
listvis()

# Record visibility listing time
if benchmarking:
    listvistime = time.time()

# Test the listvis output
print "Comparing listvis corrected data output with repository standard..."
standardOut = pathname+'/data/regression/ngc5921/listvis.ant34.out'
passlistvis = True

# Test metadata
print "  1. Checking that metadata agree."
if (listing.diffMetadata(listvisOut,standardOut,prefix=prefix+".listvis")):
    print "  Metadata agree"
else:
    print "  Metadata do not agree!"
    passlistvis = False

# Test data (floats)
print "  2. Checking that data agree to within allowed imprecision..."
precision = '0.200'
print "     Allowed visibility imprecision is ", precision
if ( listing.diffAmpPhsFloat(listvisOut,standardOut,prefix=prefix+".listvis",
                             precision=precision) ):
    print "  Data agree"
else:
    print "  Data do not agree!"
    passlistvis = False

if (passlistvis): print "Passed listvis output test"
else:             print "FAILED listvis output test"
#
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
async = True

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

immoments()

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
oldtest_imrms = 0.0020218724384903908
print ' Clean image rms should be ',oldtest_imrms
print ' Found : Image rms = ',thistest_imrms
diff_imrms = abs((oldtest_imrms-thistest_imrms)/oldtest_imrms)
print ' Difference (fractional) = ',diff_imrms

print ''
# Pull the max from the momzerostats dictionary
thistest_momzeromax=momzerostats['max'][0]
oldtest_momzeromax = 1.40223777294
print ' Moment 0 image max should be ',oldtest_momzeromax
print ' Found : Moment 0 Max = ',thistest_momzeromax
diff_momzeromax = abs((oldtest_momzeromax-thistest_momzeromax)/oldtest_momzeromax)
print ' Difference (fractional) = ',diff_momzeromax

print ''
# Pull the mean from the momonestats dictionary
thistest_momoneavg=momonestats['mean'][0]
oldtest_momoneavg = 1479.77119646
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

while True:
    if tm.retrieve(async_split_id):
        break
    else:
        time.sleep(1)

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
        print ' UVFITS Src Max amplitude should be ',thistest_src
        print ' Found : UVFITS Max = ',fitstest_src
        diff_uvfits = abs((fitstest_src-thistest_src)/thistest_src)
        print ' Difference (fractional) = ',diff_uvfits
    except Exception, e:
        tstutl.note("Failed to open UVFITS file because: "+e,"SEVERE")
else:
    tstutl.note("Could not open the UVFITS file", "SEVERE")

#
# Now the Clean FITS
#
print ''
tstutl.note("Opening FITS image file " + fitsimage +
                " to verify its existence")

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
    import datetime
    datestring=datetime.datetime.isoformat(datetime.datetime.today())
    outfile='ngc5921.'+datestring+'.log'
    logfile=open(outfile,'w')

    print >>logfile,''
    print >>logfile,''
    print >>logfile,'********** Data Summary *********'
    print >>logfile,'*   Observer: TEST     Project:                                             *'
    print >>logfile,'* Observation: VLA(28 antennas)                                             *'
    print >>logfile,'* Data records: 22653  Total integration time = 5280 seconds                *'
    print >>logfile,'* Observed from   09:19:00   to   10:47:00                                  *'
    print >>logfile,'* Fields: 3                                                                 *'
    print >>logfile,'*  ID   Name          Right Ascension  Declination   Epoch                  *'
    print >>logfile,'*  0    1331+30500002_013:31:08.29      +30.30.32.96  J2000                 *'
    print >>logfile,'*  1    1445+09900002_014:45:16.47      +09.58.36.07  J2000                 *'
    print >>logfile,'*  2    N5921_2       15:22:00.00      +05.04.00.00  J2000                  *'
    print >>logfile,'* Data descriptions: 1 (1 spectral windows and 1 polarization setups)       *'
    print >>logfile,'*  ID  #Chans Frame Ch1(MHz)    Resoln(kHz) TotBW(kHz)  Ref(MHz)    Corrs   *'  
    print >>logfile,'*  0       63 LSRK  1412.68608  24.4140625  1550.19688  1413.44902  RR  LL  *'
    print >>logfile,'*********************************'
    print ""
    print >>logfile,''
    print >>logfile,'********* Export Tests***********'
    print >>logfile,'*                               *'

    passuvfits = False
    if (uvfitsexists):
        print >>logfile,'*                               *'
        print >>logfile,'* Passed UVFITS existence test'
        if (diff_uvfits < 0.05):
            passuvfits = True
            print >>logfile,'* Passed UVFITS max test'
        else:
            print >>logfile,'* FAILED UVFITS max test'
        print >>logfile,'*  UVFITS MS max '+str(fitstest_immax)
    else:
        print >>logfile,'*                               *'
        print >>logfile,'* FAILED UVFITS existence test'

    passfits = False
    if (fitsimageexists):
        print >>logfile,'*                               *'
        print >>logfile,'* Passed FITS image existence test'
        if (diff_fitsmax < 0.05):
            passfits = True
            print >>logfile,'* Passed FITS image max test'
        else:
            print >>logfile,'* FAILED FITS image max test'
        print >>logfile,'*  FITS Image max '+str(fitstest_immax)
        if (diff_fitsrms < 0.05):
            print >>logfile,'* Passed FITS image rms test'
        else:
            passfits = False
            print >>logfile,'* FAILED FITS image rms test'
        print >>logfile,'*  FITS Image rms '+str(fitstest_imrms)
    else:
        print >>logfile,'*                               *'
        print >>logfile,'* FAILED FITS image existence test'

    print >>logfile,''
    print >>logfile,'********** Regression ***********'
    print >>logfile,'*                               *'
    if (diff_cal < 0.05):
        passcal = True
        print >>logfile,'* Passed cal max amplitude test '
    else:
        passcal = False
        print >>logfile,'* FAILED cal max amplitude test '
    print >>logfile,'*  Cal max amp '+str(thistest_cal)

    if (diff_src < 0.05):
        passsrc = True
        print >>logfile,'* Passed src max amplitude test '
    else:
        passsrc = False
        print >>logfile,'* FAILED src max amplitude test '
    print >>logfile,'*  Src max amp '+str(thistest_src)

    if (diff_immax < 0.05):
        passimmax = True
        print >>logfile,'* Passed image max test'
    else:
        passimmax = False
        print >>logfile,'* FAILED image max test'
    print >>logfile,'*  Image max '+str(thistest_immax)

    if (diff_imrms < 0.05):
        passimrms = True
        print >>logfile,'* Passed image rms test'
    else:
        passimrms = False
        print >>logfile,'* FAILED image rms test'
    print >>logfile,'*  Image rms '+str(thistest_imrms)

    if ( passcal & passsrc & passimmax & passimrms & passuvfits & passfits &
         passlistvis & passlistcal ): 
	regstate=True
	print >>logfile,'---'
	print >>logfile,'Passed Regression test for NGC5921'
	print >>logfile,'---'
        tstutl.note("Passed Regression test for NGC5921","NORMAL")
    else: 
	regstate=False
	print >>logfile,'---'
	print >>logfile,'FAILED Regression test for NGC5921'
	print >>logfile,'---'
        tstutl.note("FAILED Regression test for NGC5921","SEVERE")
    print >>logfile,'*********************************'

    print >>logfile,''
    print >>logfile,''
    print >>logfile,'********* Benchmarking *****************'
    print >>logfile,'*                                      *'
    print >>logfile,'Total wall clock time was: ', endTime - startTime
    print >>logfile,'Total CPU        time was: ', endProc - startProc
    print >>logfile,'Processing rate MB/s  was: ', 35.1/(endTime - startTime)
    print >>logfile,'* Breakdown:                           *'
    print >>logfile,'*   import       time was: '+str(importtime-startTime)
    print >>logfile,'*   flagautocorr time was: '+str(flagtime-listtime)
    print >>logfile,'*   setjy        time was: '+str(setjytime-flagtime)
    print >>logfile,'*   bandpass     time was: '+str(bptime-setjytime)
    print >>logfile,'*   gaincal      time was: '+str(gaintime-bptime)
    print >>logfile,'*   listcal      time was: '+str(listcaltime-gaintime)    
    print >>logfile,'*   fluxscale    time was: '+str(fstime-listcaltime)
    print >>logfile,'*   applycal     time was: '+str(correcttime-fstime)
    print >>logfile,'*   split-cal    time was: '+str(splitcaltime-correcttime)
    print >>logfile,'*   split-src    time was: '+str(splitsrctime-splitcaltime)
    print >>logfile,'*   contsub      time was: '+str(contsubtime-exportuvfitstime)
    print >>logfile,'*   listvis      time was: '+str(listvistime-contsubtime)
    print >>logfile,'*   clean        time was: '+str(cleantime-listvistime)
    print >>logfile,'*****************************************'
    print >>logfile,'basho (test cpu) time was: ?? seconds'

    logfile.close()

    print ""
    print "Done!"
        
#exit()
#
#=====================================================================
