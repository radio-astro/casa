##########################################################################
#                                                                        #
# Use Case Script for POLCAL 6cm Data                                    #
# Using POLCA data 20080224 BnC-config C-band                            #
#                                                                        #
# Last Updated STM 2008-05-23 (Beta Patch 2)                             #
# Updated      STM 2008-06-11 (Beta Patch 2.0)                           #
#    Uses new clean task                                                 #
#                                                                        #
##########################################################################

import time
import os
import pickle

# 
#=====================================================================
#
# This script may have some interactive commands: scriptmode = True
# if you are running it and want it to stop during interactive parts.

scriptmode = True

#=====================================================================
# SET UP THE SCRIPT CONTROL PARAMETERS HERE
#=====================================================================
#
# Set up some useful variables to control subsequent actions:

pathname=os.environ.get('CASAPATH').split()[0]

# This name will prefix all output files
prefix = 'polcal_20080224.cband.all'

# Clean up old files
os.system('rm -rf '+prefix+'*')

#=====================================================================

# Import data from export or use already existing MS?  Or UVFITS?
importmode = 'vla'               # 'vla','fits','ms'
# This is the name of the datafile used in import
# or the name of a previously made ms that will be copied
# NOTE: if an ms name must be different than prefix + '.ms'
#datafile = 'polcal_20080224.cband.edited.ms'
#datafile = '20080224C.UVF'
#
# NOTE: This file may be obtained from the CASA repository:
# http://casa.nrao.edu/Data/VLA/Polcal/POLCA_20080224_1
datafile = ['POLCA_20080224_1']
#
# If from export set these:
exportproject = 'POLCA'
exportband = 'C'
#
# Spectral windows to use in ms (usually 0,1)
usespw = ''
usespwlist = ['0','1']

# The ms will have this name
msfile = prefix + '.ms'

# These are names of calibration tables
gtable = prefix + '.gcal'
ftable = prefix + '.fluxscale'
ptable = prefix + '.pcal'
xtable = prefix + '.polx'

# Flagging:
myquackinterval = 14.0        # if >0 then quack scan beginnings
# Flagging these antennas (if blank then no flagging)
# NOTE: This script uses NEW names, so VLA ants are VAxx
flagants = ''
#flagants = 'EA*'             # keep only VLA antennas
#flagants = 'VA*'             # keep only EVLA antennas

#
# List of sources in ms
#
#	  0    A    1924-292      19:24:51.06      -29.14.30.12  J2000   
#	  1    A    1743-038      17:43:58.86      -03.50.04.62  J2000   
#	  2    A    2202+422      22:02:43.29      +42.16.39.98  J2000   
#	  3    A    2253+161      22:53:57.75      +16.08.53.56  J2000   
#	  4    B    2136+006      21:36:38.59      +00.41.54.21  J2000   
#	  5    B    0137+331      01:37:41.30      +33.09.35.13  J2000   
#	  6    A    2355+498      23:55:09.46      +49.50.08.34  J2000   
#	  7    B    0319+415      03:19:48.16      +41.30.42.10  J2000   
#	  8    B    0359+509      03:59:29.75      +50.57.50.16  J2000   
#
# These sources are the gain calibrators
gaincalfield = ['0137+331','2202+422','1743-038','1924-292','2136+006','2253+161','2355+498','0319+415','0359+509']
#
# These sources will have calibration transferred from srclist
targets = []

# Assemble field strings from lists
fieldgain = ''
if ( len(gaincalfield) > 0 ):
    for fn in range(len(gaincalfield)):
        if ( fn > 0 ):
            fieldgain += ','
        fieldgain += gaincalfield[fn]

fieldtargets = ''
if ( len(targets) > 0 ):
    for fn in range(len(targets)):
        if ( fn > 0 ):
            fieldtargets += ','
        fieldtargets += targets[fn]

#
# This list is used for final clean and stats
srclist = gaincalfield + targets

# Location of Cal Models
# e.g. for MacOSX
#fluxcaldir = '/opt/casa/data/nrao/VLA/CalModels/'
# or standard distro
fluxcaldir = pathname + '/data/nrao/VLA/CalModels/'
# or in place
#fluxcaldir = './'

# Calibration parameters:
fluxcalfield = '0137+331'    # primary calibrator for setjy
fluxcalmodel = '3C48_C.im'   # if non-blank use this model image
gaincalfield = ''            # names of gain calibrators (''=all fields)
usegaincurve = False         # use a-priori antenna gain-elevation curve?
gainopacity = 0.0            # a-priori atmospheric optical depth (Tau)
calrefant = 'VA15'           # reference antenna name for calibration (VA15,EA19)
gainsolint = 20.0            # 20s for gaincal solutions
polcalfield = '2202+422'     # polarization (D-term) calibrator
polcalmode = 'D+QU'          # polarization (D-term) calibration mode
polduvrange = ''             # uvrange for polcal D
setpolmodel = True           # if true then use setjy to set pol model
polxfield = '0137+331'       # polarization angle (X) calibrator
polxuvrange = ''             # uvrange for polcal X
#
setjymode = 'set'            # mode for fluxcal setyjy: 'set', 'flux', 'ft'

# This is the name of the split file for corrected data
srcsplitms = prefix + '.split.ms'

#
# Set up general clean parameters

# This is BnC-config VLA 6cm (4.85GHz) obs
# Check the observational status summary
# Primary beam FWHM = 45'/f_GHz = 557"
# Synthesized beam for VLA/EVLA at C-Band:
#      A-config FWHM = 0.4"
#      B-config FWHM = 1.2"
#      C-config FWHM = 3.9"
#      D-config FWHM = 14.0"
# RMS in 10min (600s) = 0.06 mJy (thats now, but close enough)
#
# Set the output image size and cell size (arcsec)
# 0.4" will give 3x oversampling at least
# clean will say to use a composite integer (e.g.288) for efficiency
#clnalg = 'clark'
clnalg = 'hogbom'
usecsclean = False
clnimsize = 288
clncell = 0.4
# Fix maximum number of iterations
clniter = 200
# Also set flux residual threshold (0.04 mJy)
# Our scans are around 120s
# With rms of 0.06 mJy in 600s ==> rms = 0.13 mJy
# Set to 10x thermal rms
clthreshold = 1.3

# Set up a clean box in the center (1/8 of image)
clncenter = clnimsize/2
clnblc = clncenter - clnimsize/8
clntrc = clncenter + clnimsize/8
# For poor uv coverage, use tigher box (6 x SynthBeam = 18xcell)
clnblc = clncenter - 10
clntrc = clncenter + 10
centerbox = [clnblc,clnblc,clntrc,clntrc]

myclnbox = centerbox
# Can also force interactive cleaning
#myclnbox = 'interactive'    

aipsmodel = {}
#
#=====================================================================
#
# Polarization of X angle calibrator 0137+331
# If setpolmodel = True
#
# Set up fluxcalmodel
#
fcalmodel = {}
#
# The flux model for 0137+331 (C-band)
fcalfield = {}
# NOTE: you must have entries for all spw in usespwlist
# I,Q,U,V
fcalfield['0'] = [5.405,0,0,0]
fcalfield['1'] = [5.458,0,0,0]
fcalmodel['0137+331'] = fcalfield
# Put in 2202+422
# These values from AIPS (http://www.vla.nrao.edu/astro/calib/polar/2004/)
fcalfield = {}
fcalfield['0'] = [2.465,0,0,0]
fcalfield['1'] = [2.461,0,0,0]
fcalmodel['2202+422'] = fcalfield
#
# Set up pcalmodel
#
pcalmodel = {}
#
# The polarization model for 0137+331
pcalfield = {}
# NOTE: you must have entries for all spw in usespwlist
# From calibrator manual: C-band RLPD=-148deg P/I=0.041
# IPOL,FPOL,RLPHASE
pcalfield['0'] = [5.405,0.041,-148.0]
pcalfield['1'] = [5.458,0.041,-148.0]
pcalmodel['0137+331'] = pcalfield
# Put in 2202+422 (with effective flux of 1.0 before fluxscale)
# These values from AIPS (http://www.vla.nrao.edu/astro/calib/polar/2004/)
pcalfield = {}
pcalfield['0'] = [1.0,0.072,-55.00]
pcalfield['1'] = [1.0,0.072,-55.00]
pcalmodel['2202+422'] = pcalfield
#
# Set the polmodel from pcalmodel
#
print '--Setting up Polarization models--'

polmodel = {}
for field in pcalmodel.keys() :
    spwmodel = {}
    # the RLPD is atan2(U,Q) so Q=I*P/I*cos(RLPD)  U=I*P/I*sin(RLPD)
    for spw in usespwlist:
        ipol = pcalmodel[field][spw][0]
        fpol = pcalmodel[field][spw][1]
        rlpd_deg = pcalmodel[field][spw][2]
        rlpd = rlpd_deg*pl.pi/180.0
        ppol = ipol*fpol
        qpol = ppol*cos(rlpd)
        upol = ppol*sin(rlpd)
        fluxdensity=[ipol,qpol,upol,0.0]
    
        pmodel = {}
        pmodel['rlpd_deg'] = rlpd_deg
        pmodel['rlpd'] = rlpd
        pmodel['fpol'] = fpol
    
        fmodel = {}
        fmodel['flux'] = fluxdensity
        fmodel['poln'] = pmodel
        spwmodel[spw] = fmodel
    
    polmodel[field] = spwmodel

print "Created polmodel dictionary"
print polmodel
#
#=====================================================================
# Data Import and List
#=====================================================================
#
if ( importmode == 'vla' ):
    #
    # Import the data from VLA Export to MS
    #
    print '--ImportVLA--'
    default('importvla')
    
    print "Use importvla to read VLA Export and make an MS"
    
    archivefiles = datafile
    vis = msfile
    bandname = exportband
    autocorr = False
    antnamescheme = 'new'
    project = exportproject
    
    saveinputs('importvla',prefix+'.importvla.saved')
    importvla()
elif ( importmode == 'fits' ):
    #
    # Import the data from VLA Export to MS
    #
    print '--ImportUVFITS--'
    default('importuvfits')
    
    print "Use importuvfits to read UVFITS and make an MS"
    
    fitsfile = datafile
    vis = msfile
    async = False
    
    saveinputs('importuvfits',prefix+'.importuvfits.saved')
    importuvfits()
else:
    #
    # Copy from msfile
    #
    print '--MS Copy--'
    print "Copying "+datafile+" to "+msfile
    
    os.system('cp -r '+datafile+' '+msfile)
    vis = msfile

#
#=====================================================================
#
print '--Listobs--'

print "List summary of MS"

listobs()

###############################################
###  Begin Task: listobs  ###
#   
# MeasurementSet Name:
#     /home/sandrock/smyers/Testing/2008-03/polcal_20080224/polcal_20080224.cband.raw.ms
# MS Version 2
#	
#	   Observer: unavailable     Project: POLCA  
#	Observation: VLA
#   Data records: 318708       Total integration time = 9836.67 seconds
#	   Observed from   17:10:52   to   19:54:48
#   
#	   ObservationID = 0         ArrayID = 0
#	  Date        Timerange                Scan  FldId FieldName      SpwIds
#	  24-Feb-2008/17:10:51.7 - 17:12:08.3     1      0 1924-292       [0, 1]
#	              17:21:01.7 - 17:22:18.3     2      1 1743-038       [0, 1]
#	              17:34:31.7 - 17:35:48.3     3      2 2202+422       [0, 1]
#	              17:45:01.7 - 17:46:18.3     4      3 2253+161       [0, 1]
#	              17:55:11.7 - 17:56:28.3     5      4 2136+006       [0, 1]
#	              18:08:01.7 - 18:09:18.3     6      5 0137+331       [0, 1]
#	              18:22:11.7 - 18:23:58.3     7      6 2355+498       [0, 1]
#	              18:32:51.7 - 19:07:58.3     8      2 2202+422       [0, 1]
#	              19:20:51.7 - 19:22:18.3     9      5 0137+331       [0, 1]
#	              19:32:11.7 - 19:33:48.3    10      7 0319+415       [0, 1]
#	              19:42:01.7 - 19:43:18.3    11      8 0359+509       [0, 1]
#	              19:53:31.7 - 19:54:48.3    12      2 2202+422       [0, 1]
#   Fields: 9
#	  ID   Code Name          Right Ascension  Declination   Epoch   
#	  0    A    1924-292      19:24:51.06      -29.14.30.12  J2000   
#	  1    A    1743-038      17:43:58.86      -03.50.04.62  J2000   
#	  2    A    2202+422      22:02:43.29      +42.16.39.98  J2000   
#	  3    A    2253+161      22:53:57.75      +16.08.53.56  J2000   
#	  4    B    2136+006      21:36:38.59      +00.41.54.21  J2000   
#	  5    B    0137+331      01:37:41.30      +33.09.35.13  J2000   
#	  6    A    2355+498      23:55:09.46      +49.50.08.34  J2000   
#	  7    B    0319+415      03:19:48.16      +41.30.42.10  J2000   
#	  8    B    0359+509      03:59:29.75      +50.57.50.16  J2000   
#   Spectral Windows:  (2 unique spectral windows and 1 unique polarization setups)
#	  SpwID  #Chans Frame Ch1(MHz)    ChanWid(kHz)TotBW(kHz)  Ref(MHz)    Corrs
#	  0    1 TOPO  4885.1      50000       50000       4885.1      RR  RL  LR  LL  
#	  1    1 TOPO  4835.1      50000       50000       4835.1      RR  RL  LR  LL  
#   Feeds: 27: printing first row only
#	  Antenna   Spectral Window     # Receptors    Polarizations
#	  1         -1                  2              [         R, L]
#   Antennas: 27:
#	  ID   Name  Station   Diam.    Long.         Lat.         
#	  0    EA24  VLA:W12   25.0 m   -107.37.37.4  +33.53.44.2  
#	  1    EA16  VLA:W6    25.0 m   -107.37.15.6  +33.53.56.4  
#	  2    EA01  VLA:W10   25.0 m   -107.37.28.9  +33.53.48.9  
#	  3    EA19  VLA:W4    25.0 m   -107.37.10.8  +33.53.59.1  
#	  4    VA08  VLA:W16   25.0 m   -107.37.57.4  +33.53.33.0  
#	  5    EA17  VLA:W14   25.0 m   -107.37.46.9  +33.53.38.9  
#	  6    VA06  VLA:W8    25.0 m   -107.37.21.6  +33.53.53.0  
#	  7    VA22  VLA:W2    25.0 m   -107.37.07.4  +33.54.00.9  
#	  8    EA04  UNKNOWN   25.0 m   -107.37.41.3  +33.53.42.0  
#	  9    VA20  VLA:E12   25.0 m   -107.36.31.7  +33.53.48.5  
#	  10   VA15  VLA:E4    25.0 m   -107.37.00.8  +33.53.59.7  
#	  11   VA28  VLA:E6    25.0 m   -107.36.55.6  +33.53.57.7  
#	  12   VA10  VLA:E8    25.0 m   -107.36.48.9  +33.53.55.1  
#	  13   EA14  VLA:E16   25.0 m   -107.36.09.8  +33.53.40.0  
#	  14   EA11  VLA:E10   25.0 m   -107.36.40.9  +33.53.52.0  
#	  15   VA03  VLA:E14   25.0 m   -107.36.21.3  +33.53.44.5  
#	  16   EA23  VLA:E18   25.0 m   -107.35.57.2  +33.53.35.1  
#	  17   EA21  VLA:E2    25.0 m   -107.37.04.4  +33.54.01.1  
#	  18   VA12  VLA:N4    25.0 m   -107.37.06.5  +33.54.06.1  
#	  19   VA02  VLA:N20   25.0 m   -107.37.13.2  +33.55.09.5  
#	  20   EA13  VLA:N16   25.0 m   -107.37.10.9  +33.54.48.0  
#	  21   EA26  VLA:N32   25.0 m   -107.37.22.0  +33.56.33.6  
#	  22   EA25  VLA:N24   25.0 m   -107.37.16.1  +33.55.37.7  
#	  23   VA09  VLA:N8    25.0 m   -107.37.07.5  +33.54.15.8  
#	  24   EA18  VLA:N12   25.0 m   -107.37.09.0  +33.54.30.0  
#	  25   VA07  VLA:N36   25.0 m   -107.37.25.6  +33.57.07.6  
#	  26   VA27  VLA:N28   25.0 m   -107.37.18.7  +33.56.02.5  
#   
#	
#	Tables:
#	   MAIN                  318708 rows     
#	   ANTENNA                   27 rows     
#	   DATA_DESCRIPTION           2 rows     
#	   DOPPLER                    2 rows     
#	   FEED                      27 rows     
#	   FIELD                      9 rows     
#	   FLAG_CMD             <empty>  
#	   FREQ_OFFSET         <absent>  
#	   HISTORY                    6 rows     
#	   OBSERVATION                1 row      
#	   POINTING             <empty>  
#	   POLARIZATION               1 row      
#	   PROCESSOR            <empty>  
#	   SOURCE                     9 rows     
#	   SPECTRAL_WINDOW            2 rows     
#	   STATE                <empty>  
#	   SYSCAL              <absent>  
#	   WEATHER             <absent>  
#	
###  End Task: listobs  ###
###############################################

# Note that the antennas are out of order as loaded by importvla

#
#=====================================================================
# Data Flagging if needed
#=====================================================================
#
if ( myquackinterval > 0.0 ):
    #
    # First quack the data
    #
    print '--Flagdata--'
    default('flagdata')
    
    print "Quacking scan beginnings using interval "+str(myquackinterval)
    
    vis = msfile
    correlation = ''
    field = ''
    antenna = ''
    spw = usespw
    mode = 'quack'
    quackinterval = myquackinterval
    
    saveinputs('flagdata',prefix+'.flagdata.quack.saved')
    flagdata()
    
    #
    # Use Flagmanager to save a copy of the flags so far
    #
    default('flagmanager')
    
    print "Now will use flagmanager to save the flags"
    
    vis = msfile
    mode = 'save'
    versionname = 'quack'
    comment = 'Quack '+str(myquackinterval)
    merge = 'replace'
    
    saveinputs('flagmanager',prefix+'.flagmanager.quack.saved')
    flagmanager()


#
if (flagants != '' and not flagants.isspace() ):
    print '--Flagdata--'
    default('flagdata')
    
    print "Flag all data to AN "+flagants
    
    vis = msfile
    correlation = ''
    field = ''
    spw = usespw
    mode = 'manualflag'
    antenna = flagants
    
    saveinputs('flagdata',prefix+'.flagdata.ants.saved')
    flagdata()
    
    #
    # Use Flagmanager to save a copy of the flags so far
    #
    default('flagmanager')
    
    print "Now will use flagmanager to save the flags"
    
    vis = msfile
    mode = 'save'
    versionname = 'antflags'
    comment = 'flag AN '+flagants
    merge = 'replace'
    
    saveinputs('flagmanager',prefix+'.flagmanager.ants.saved')
    flagmanager()


#
#=====================================================================
# Calibration
#=====================================================================
#
# Set the fluxes of the primary calibrator(s)
#
if ( setjymode == 'flux' ):
    print '--Setjy--'
    default('setjy')
    
    vis = msfile
    
    print "Use setjy to set flux of "+fluxcalfield+" to point model"
    field = fluxcalfield
    spw = usespw

    # If we need a model for flux calibrator then put this here
    modimage = fluxcaldir + fluxcalmodel
    
    # Loop over spw
    for spw in usespwlist:
        fluxdensity = fcalmodel[fluxcalfield][spw]
        print "Setting SPW "+spw+" to "+str(fluxdensity)
        saveinputs('setjy',prefix+'.setjy.'+spw+'.saved')
        setjy()

elif ( setjymode == 'ft' ):
    print '--FT--'

    default('ft')
    vis = msfile
    field = fluxcalfield

    for spw in usespwlist:
        model = fluxcaldir + fluxcalmodel+'_'+spw+'_IQUV.model'
        print "Use FT to set model"+model
        saveinputs('ft',prefix+'.ft.0.saved')
        ft()
    
else:
    print '--Setjy--'
    default('setjy')
    
    vis = msfile
    
    print "Use setjy to set flux of "+fluxcalfield
    field = fluxcalfield
    spw = usespw

    # If we need a model or fluxdensities then put those here
    modimage = fluxcaldir + fluxcalmodel
    
    saveinputs('setjy',prefix+'.setjy.saved')
    setjy()
    #
    # You should see something like this in the logger and casapy.log file:
    #
    # 0137+331  spwid=  0  [I=5.405, Q=0, U=0, V=0] Jy, (Perley-Taylor 99)
    # 0137+331  spwid=  1  [I=5.458, Q=0, U=0, V=0] Jy, (Perley-Taylor 99)
    
    # cf. AIPS
    #  SETJY     '0137+331        ' IF =  1 FLUX = 5.4054 (Jy calcd)
    #  SETJY     '0137+331        ' IF =  2 FLUX = 5.4585 (Jy calcd)
    
    print "Look in logger for the fluxes (should be 5.405 and 5.458 Jy)"

#=====================================================================
#
# Initial gain calibration
#
print '--Gaincal--'
default('gaincal')

print "Solve for antenna gains on sources "+gaincalfield
print "We have 2 single-channel continuum spw"

vis = msfile

# set the name for the output gain caltable
print "Output gain table name is "+gtable
caltable = gtable

# All fields are calibrators
# We have 2 IFs (SPW 0,1) with one channel each

# Assemble field string from gaincalfield list
field = fieldgain
print "Calibrating using fields "+field

# Calibrate these spw
spw = usespw

# a-priori calibration application
gaincurve = usegaincurve
opacity = gainopacity

# do not apply parallactic angle correction
parang = False

# G solutions for both amplitude and phase using gainsolint
gaintype = 'G'
solint = gainsolint
calmode = 'ap'

# reference antenna
refant = calrefant

# minimum SNR 3
minsnr = 3

saveinputs('gaincal',prefix+'.gaincal.saved')
gaincal()

# use plotcal to view or listcal to list

#=====================================================================
#
# List gain calibration
#
print '--Listcal--'

listfile = caltable + '.list'

print "Listing calibration to file "+listfile

listcal()

#
#=====================================================================
#
# Bootstrap flux scale
#
print '--Fluxscale--'
default('fluxscale')

print "Use fluxscale to rescale gain table to make new one"

vis = msfile

# set the name for the output rescaled caltable
ftable = prefix + '.fluxscale'
fluxtable = ftable

print "Output scaled gain cal table is "+ftable

# point to our first gain cal table
caltable = gtable

# use the source we did setjy on as our flux standard reference
reference = fluxcalfield

# transfer the flux to all our other sources
# to bring amplitues in line with the absolute scale
transfer = fieldgain

saveinputs('fluxscale',prefix+'.fluxscale.saved')
fluxscale()

# You should see in the logger something like:
# Found reference field(s): 0137+331
# Found transfer field(s): 1924-292 1743-038 2202+422 2253+161 2136+006 2355+498 0319+415 0359+509
# Flux density for 1924-292 in SpW=0 is: 8.25145 +/- 0.00988121 (SNR = 835.065, nAnt= 13)
# Flux density for 1924-292 in SpW=1 is: 8.22457 +/- 0.0140951 (SNR = 583.505, nAnt= 13)
# Flux density for 1743-038 in SpW=0 is: 5.31336 +/- 0.00603626 (SNR = 880.239, nAnt= 13)
# Flux density for 1743-038 in SpW=1 is: 5.3184 +/- 0.00480634 (SNR = 1106.54, nAnt= 13)
# Flux density for 2202+422 in SpW=0 is: 2.46545 +/- 0.00335055 (SNR = 735.833, nAnt= 13)
# Flux density for 2202+422 in SpW=1 is: 2.46072 +/- 0.00353799 (SNR = 695.512, nAnt= 13)
# Flux density for 2253+161 in SpW=0 is: 8.74607 +/- 0.0142334 (SNR = 614.474, nAnt= 13)
# Flux density for 2253+161 in SpW=1 is: 8.77219 +/- 0.0102289 (SNR = 857.587, nAnt= 13)
# Flux density for 2136+006 in SpW=0 is: 9.97863 +/- 0.013815 (SNR = 722.303, nAnt= 13)
# Flux density for 2136+006 in SpW=1 is: 9.99001 +/- 0.0170089 (SNR = 587.339, nAnt= 13)
# Flux density for 2355+498 in SpW=0 is: 1.29395 +/- 0.00181169 (SNR = 714.221, nAnt= 13)
# Flux density for 2355+498 in SpW=1 is: 1.29893 +/- 0.00217214 (SNR = 597.995, nAnt= 13)
# Flux density for 0319+415 in SpW=0 is: 13.5742 +/- 0.0221722 (SNR = 612.218, nAnt= 13)
# Flux density for 0319+415 in SpW=1 is: 13.5481 +/- 0.0230828 (SNR = 586.932, nAnt= 13)
# Flux density for 0359+509 in SpW=0 is: 5.13982 +/- 0.00906505 (SNR = 566.993, nAnt= 13)
# Flux density for 0359+509 in SpW=1 is: 5.10322 +/- 0.00990264 (SNR = 515.339, nAnt= 13)
# Storing result in polcal_20080224.cband.vla_3c84.fluxscale
# Writing solutions to table: polcal_20080224.cband.vla_3c84.fluxscale

#=====================================================================
#
# List fluxscale table
#
print '--Listcal--'

caltable = ftable
listfile = caltable + '.list'

print "Listing calibration to file "+listfile

listcal()

#=====================================================================
#
# Plot final gain calibration
#
print '--Plotcal--'

iteration = ''
showgui = False

xaxis = 'time'
yaxis = 'amp'
figfile = caltable + '.plot.amp.png'
print "Plotting calibration to file "+figfile
saveinputs('plotcal',prefix+'.plotcal.fluxscale.amp.saved')
plotcal()

xaxis = 'time'
yaxis = 'phase'
figfile = caltable + '.plot.phase.png'
print "Plotting calibration to file "+figfile
saveinputs('plotcal',prefix+'.plotcal.fluxscale.phase.saved')
plotcal()

xaxis = 'antenna'
yaxis = 'amp'
figfile = caltable + '.plot.antamp.png'
print "Plotting calibration to file "+figfile
saveinputs('plotcal',prefix+'.plotcal.fluxscale.antamp.saved')
plotcal()

if ( setpolmodel and polcalmode.count('X') > 0 ):
    #
    # =====================================================================
    #
    # Now run setjy to (re)set model for polxfield
    #
    print '--Setjy--'
    default('setjy')

    vis = msfile
    
    print "Use setjy to set IQU fluxes of "+polxfield
    field = polxfield

    for spw in usespwlist:
        fluxdensity = polmodel[field][spw]['flux']
    
        saveinputs('setjy',prefix+'.setjy.polspw.'+spw+'.saved')
        setjy()
    

#=====================================================================
#
# Polarization (D-term) calibration
#
print '--PolCal--'
default('polcal')

print "Polarization D-term Calibration (linear approx) on "+polcalfield

vis = msfile

# Start with the un-fluxscaled gain table
gaintable = gtable

# use settings from gaincal
gaincurve = usegaincurve
opacity = gainopacity

# Output table
ptable = prefix + '.pcal'
caltable = ptable

# Use an unpolarized source or a source tracked through a range of PA
field = polcalfield
spw = usespw

selectdata=True
uvrange = polduvrange

# Polcal mode
poltype = polcalmode

# Currently 1-day timescale is hardwired
solint = 86400.

# reference antenna
refant = calrefant

# minimum SNR 3
minsnr = 3

saveinputs('polcal',prefix+'.polcal.saved')
polcal()

# You should see something like:
# Fractional polarization solution for 2202+422 (spw = 0):
# : Q = 0.00356182, U = 0.0717148  (P = 0.0718032, X = 43.5783 deg)
# Fractional polarization solution for 2202+422 (spw = 1):
# : Q = -0.00561314, U = -0.0720833  (P = 0.0723015, X = -47.2263 deg)

#=====================================================================
#
# List polcal solutions
#
print '--Listcal--'

listfile = caltable + '.list'

print "Listing calibration to file "+listfile

listcal()

#=====================================================================
#
# Plot polcal solutions
#
print '--Plotcal--'

iteration = ''
showgui = False

xaxis = 'real'
yaxis = 'imag'
figfile = caltable + '.plot.reim.png'
print "Plotting calibration to file "+figfile
saveinputs('plotcal',prefix+'.plotcal.polcal.d.reim.saved')
plotcal()

xaxis = 'antenna'
yaxis = 'amp'
figfile = caltable + '.plot.antamp.png'
print "Plotting calibration to file "+figfile
saveinputs('plotcal',prefix+'.plotcal.polcal.d.antamp.saved')
plotcal()

xaxis = 'antenna'
yaxis = 'phase'
figfile = caltable + '.plot.antphase.png'
print "Plotting calibration to file "+figfile
saveinputs('plotcal',prefix+'.plotcal.polcal.d.antphase.saved')
plotcal()

xaxis = 'antenna'
yaxis = 'snr'
figfile = caltable + '.plot.antsnr.png'
print "Plotting calibration to file "+figfile
saveinputs('plotcal',prefix+'.plotcal.polcal.d.antsnr.saved')
plotcal()

#=====================================================================
# Do Chi (X) pol angle calibration if possible
#=====================================================================
#
dopolx = False
if ( pcalmodel.has_key(polxfield) ):
    dopolx = True

    if ( setpolmodel and not polcalmode.count('X') > 0 ):
        #
        # =============================================================
        #
        # Now run setjy if we havent already
        #
    
        print '--Setjy--'
        default('setjy')
        
        vis = msfile
        
        print "Use setjy to set IQU fluxes of "+polxfield
        field = polxfield
        
        for spw in usespwlist:
            fluxdensity = polmodel[field][spw]['flux']
            
            saveinputs('setjy',prefix+'.setjy.polspw.'+spw+'.saved')
            setjy()
        
    
    #
    # =====================================================================
    #
    # Polarization (X-term) calibration
    #
    print '--PolCal--'
    default('polcal')
    
    print "Polarization R-L Phase Calibration (linear approx)"
    
    vis = msfile
    
    # Start with the G and D tables
    gaintable = [gtable,ptable]
    
    # use settings from gaincal
    gaincurve = usegaincurve
    opacity = gainopacity
    
    # Output table
    xtable = prefix + '.polx'
    caltable = xtable

    # previously set with setjy
    field = polxfield
    spw = usespw
    
    selectdata=True
    uvrange = polxuvrange
    
    # Solve for Chi
    poltype = 'X'
    solint = 86400.
    
    # reference antenna
    refant = calrefant
    
    # minimum SNR 3
    minsnr = 3
    
    saveinputs('polcal',prefix+'.polcal.X.saved')
    polcal()
    
    # You should get something like:
    # Position angle offset solution for 0137+331 (spw = 0) = 72.437 deg.
    # Position angle offset solution for 0137+331 (spw = 1) = -21.0703 deg.

    #
    # =====================================================================
    #
    # List polcal solutions
    #
    #print '--Listcal--'
    
    #listfile = caltable + '.list'
    
    #print "Listing calibration to file "+listfile
    
    #listcal()

    #
    # =====================================================================
    #
    # Plot polcal solutions
    #
    print '--Plotcal--'
    
    xaxis = 'antenna'
    yaxis = 'phase'
    iteration = ''
    
    showgui = False
    figfile = caltable + '.plot.png'
    
    print "Plotting calibration to file "+figfile
    saveinputs('plotcal',prefix+'.plotcal.polcal.x.antphase.saved')
    plotcal()

else:
    if (polxfield != '' and not polxfield.isspace() ):
        print "DO NOT HAVE PCALMODEL FOR "+polxfield
        print "PCALMODEL = ",pcalmodel


#=====================================================================
#
# Correct the data
# (This will put calibrated data into the CORRECTED_DATA column)
#
# First using gaincalfield
#
print '--ApplyCal--'
default('applycal')

print "This will apply the calibration to the DATA"
print "Fills CORRECTED_DATA"

vis = msfile

# Start with the fluxscaled G table, the D table, and the X table
if (dopolx):
    gaintable = [ftable,ptable,xtable]
else:
    gaintable = [ftable,ptable]

# use settings from gaincal
gaincurve = usegaincurve
opacity = gainopacity

# select all the data
spw = usespw
selectdata = False

# IMPORTANT set parang=True for polarization
parang = True

# use the list of gain calibrators, apply to themselves
field = fieldgain
gainselect = field
print "Applying calibration to gain calibrators "+field

saveinputs('applycal',prefix+'.applycal.saved')
applycal()

if ( len(targets) > 0 ):
    #
    # Now with targets if any (transfer from gaincalfield)
    #
    # Assemble field string from target list
    field = fieldtargets
    print "Applying calibration to targets "+field
    
    saveinputs('applycal',prefix+'.applycal.targets.saved')
    applycal()

#
#=====================================================================
#
# Now write out the corrected data
#
print '--Split--'
default('split')

vis = msfile

# Now we write out the corrected data to a new MS

# Make an output vis file
srcsplitms = prefix + '.split.ms'
outputvis = srcsplitms

# Select all data
field = ''

# Have to split all spw to preserve numbering
spw = ''

# pick off the CORRECTED_DATA column
datacolumn = 'corrected'

print "Split CORRECTED_DATA into DATA in new ms "+srcsplitms

saveinputs('split',prefix+'.split.saved')
split()

#
#=====================================================================
#
# Plot up the visibilities for the main calibrators
#
print '--Plotxy--'
default('plotxy')

vis = srcsplitms

field = fluxcalfield
spw = ''

selectdata=True

xaxis = 'uvdist'

interactive=False

correlation='RR LL'
yaxis = 'amp'
figfile = prefix+'.split.'+field+'.uvplot.amp.png'
saveinputs('plotxy',prefix+'.plotxy.'+field+'.amp.saved')
plotxy()

correlation='RL LR'
yaxis = 'phase'
figfile = prefix+'.split.'+field+'.uvplot.rlphase.png'
saveinputs('plotxy',prefix+'.plotxy.'+field+'.rlphase.saved')
plotxy()

if ( polcalfield != fluxcalfield ):
    # Now the poln calibrator
    field = polcalfield

    correlation='RR LL'
    yaxis = 'amp'
    figfile = prefix+'.split.'+field+'.uvplot.amp.png'
    saveinputs('plotxy',prefix+'.plotxy.'+field+'.amp.saved')
    plotxy()
    
    correlation='RL LR'
    yaxis = 'phase'
    figfile = prefix+'.split.'+field+'.uvplot.rlphase.png'
    saveinputs('plotxy',prefix+'.plotxy.'+field+'.rlphase.saved')
    plotxy()

#
#=====================================================================
# CLEAN the sources
#=====================================================================

clnmodel = {}
#
#=====================================================================
# Loop over sources and spw
# Set up for new clean in patch 2
#
for src in srclist:
    
    srcmodel = {}
    
    for spwid in usespwlist:

        print '-- Clean '+src+' spw '+spwid+' --'
        default('clean')
    
        field = src
        spw = spwid
    
        # Pick up our split source data
        vis = srcsplitms
        
        # Make an image root file name
        imname1 = prefix + '.' + src + '.' + spwid + '.clean'
        imagename = imname1
        
        print "  Output images will be prefixed with "+imname1
        
        # Set up the output continuum image (single plane mfs)
        mode = 'mfs'
        
        # All polarizations
        stokes = 'IQUV'

        # Use chose clean style
        psfmode = clnalg
        csclean = usecsclean
        
        imsize = [clnimsize,clnimsize]
        cell = [clncell,clncell]
    
        # Standard gain factor 0.1
        gain = 0.1
        
        niter = clniter
        
        threshold = clthreshold
        
        # Set up the weighting
        # Use Briggs weighting (a moderate value, on the uniform side)
        weighting = 'briggs'
        robust = 0.5
        # Use natural weighting
        weighting = 'natural'
        
        # Use the cleanbox
        mask = myclnbox
    
        saveinputs('clean',prefix+'.clean.'+src+'.'+spwid+'.saved')
        clean()
        
        # Set up variables
        clnimage1 = imname1+'.image'
        clnmodel1 = imname1+'.model'
        clnresid1 = imname1+'.residual'
        clnmask1  = imname1+'.mask'
        clnpsf1   = imname1+'.psf'
        clnflux1  = imname1+'.flux'
        
        #
        # =====================================================================
        #
        # Get some statistics of the clean image
        #
        default('imstat')

        field = src
        spw = spwid
        
        # Use the clean box
        mybox = str(clnblc)+','+str(clnblc)+','+str(clntrc)+','+str(clntrc)
        
        spwmodel = {}

        spwstats = {}
        spwfluxes = {}
        spwsum = {}
        spwmod = {}

        for stokes in ['I','Q','U','V']:

            # Use the clean image
            imagename = clnimage1
            box = mybox
            
            saveinputs('imstat',prefix+'.imstat.'+src+'.'+spwid+'.'+stokes+'.saved')
            xstat = imstat()

            spwstats[stokes] = xstat

            # Peak (max or min) in box
            xmax = xstat['max'][0]
            xmin = xstat['min'][0]
            if( abs(xmin) > abs(xmax) ):
                xpol = xmin
            else:
                xpol = xmax
            
            spwfluxes[stokes]= xpol

            # Integrated flux in box
            xsum = xstat['flux'][0]
            spwsum[stokes]= xsum
        
            # Use the clean model and no box
            imagename = clnmodel1
            box = ''

            saveinputs('imstat',prefix+'.imstat.'+src+'.'+spwid+'.'+stokes+'.model.saved')
            xstat = imstat()
            # Integrated flux in image
            xmod = xstat['sum'][0]
            spwmod[stokes]= xmod

        # Done with stokes
        
        spwmodel['stat'] = spwstats
        spwmodel['flux'] = spwfluxes
        spwmodel['integ'] = spwsum
        spwmodel['model'] = spwmod

        # Use ia tool for pixel values in the restored image
        imagename = clnimage1
        # Get image values at the reference pixel
        spwref = {}
        ia.open(imagename)
        #
        # Stokes I
        ipix = ia.pixelvalue()
        # Get reference pixel
        xref = ipix['pixel'][0]
        yref = ipix['pixel'][1]
        iflx = ipix['value']['value']
        spwref['I'] = iflx
        #
        # Stokes Q
        qpix = ia.pixelvalue([xref,yref,1,0])
        qflx = qpix['value']['value']
        spwref['Q'] = qflx
        #
        # Stokes U
        upix = ia.pixelvalue([xref,yref,2,0])
        uflx = upix['value']['value']
        spwref['U'] = uflx
        #
        # Stokes V
        vpix = ia.pixelvalue([xref,yref,3,0])
        vflx = vpix['value']['value']
        spwref['V'] = vflx
        #
        # Polarization quantities
        pflx = sqrt( qflx**2 + uflx**2 )
        fflx = pflx/iflx
        xflx = atan2(uflx,qflx)*180.0/pi
        spwref['P'] = pflx
        spwref['F'] = fflx
        spwref['X'] = xflx
        spwref['xref'] = xref
        spwref['yref'] = yref
        #

        # Now the values at the maximum of I
        spwmax = {}
        #
        # Pull the maxpos of I
        xref = spwstats['I']['maxpos'][0]
        yref = spwstats['I']['maxpos'][1]
        #
        # Stokes I
        iflx = spwstats['I']['max'][0]
        spwmax['I'] = iflx
        #
        # Stokes Q
        qpix = ia.pixelvalue([xref,yref,1,0])
        qflx = qpix['value']['value']
        spwmax['Q'] = qflx
        #
        # Stokes U
        upix = ia.pixelvalue([xref,yref,2,0])
        uflx = upix['value']['value']
        spwmax['U'] = uflx
        #
        # Stokes V
        vpix = ia.pixelvalue([xref,yref,3,0])
        vflx = vpix['value']['value']
        spwmax['V'] = vflx
        
        spwmax['xref'] = xref
        spwmax['yref'] = yref
        # Done with ia tool
        ia.close()
        
        spwmodel['refval'] = spwref
        spwmodel['maxval'] = spwmax
        
        srcmodel[spwid] = spwmodel
    
    # Done with spw

    clnmodel[src] = srcmodel
    
# Done with srcs
#
#=====================================================================
# Report Final Stats
#=====================================================================
#
print 'Results for '+prefix+' :'
print ""

import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())

outfile = 'out.'+prefix+'.'+datestring+'.log'
logfile=open(outfile,'w')
print >>logfile,'Results for '+prefix+' :'
print >>logfile,""

if ( polmodel.has_key(polxfield) ):
    # Check RL phase offset on X calibrator
    print "R-L phase residual from image of "+polxfield
    print ""
    print >>logfile,"R-L phase residual from image of "+polxfield+" :"
    print >>logfile,""
    
    src = polxfield
    rlcor = {}

    for spwid in usespwlist:
        ipol = clnmodel[src][spwid]['flux']['I']
        qpol = clnmodel[src][spwid]['flux']['Q']
        upol = clnmodel[src][spwid]['flux']['U']
        vpol = clnmodel[src][spwid]['flux']['V']
        rlpd = atan2(upol,qpol)
        rlpdcal = polmodel[src][spwid]['poln']['rlpd']
        rlpcor = rlpdcal - rlpd
        scor = sin(rlpcor); ccor = cos(rlpcor); rlpcor = atan2(scor,ccor)
        rlcor[spwid] = rlpcor
        rlpcor_deg = rlpcor*180.0/pl.pi
        
        print "R-L Phase Correction SPW "+spwid+" = %7.2f deg" % rlpcor_deg
        print >>logfile,"R-L Phase Correction SPW "+spwid+" = %7.2f deg" % rlpcor_deg
    

#
#=====================================================================
#
# Loop over sources and spw
#
print ""
print "Final Stats:"
print ""

print >>logfile,""
print >>logfile,"Final Stats:"
print >>logfile,""

for src in srclist:

    print "Source "+src+" :"
    print >>logfile,"Source "+src+" :"

    for spwid in usespwlist:

        field = src
        spw = spwid

        # Get fluxes from images

        ipol = clnmodel[src][spwid]['flux']['I']
        qpol = clnmodel[src][spwid]['flux']['Q']
        upol = clnmodel[src][spwid]['flux']['U']
        vpol = clnmodel[src][spwid]['flux']['V']
        
        # Now get polarization results
        
        ppol = sqrt(qpol**2 + upol**2)
        fpol = ppol/ipol
        rlpd = atan2(upol,qpol)
        rlpd_deg = rlpd*180.0/pl.pi

        #print '  spw %s CASA I = %7.3f Q = %7.3f U = %7.3f V = %7.3f ' % (spwid,ipol,qpol,upol,vpol)
        print '  spw %s CASA I = %7.3f P = %7.3f F = %7.4f X = %7.2f deg' % (spwid,ipol,ppol,fpol,rlpd_deg)
        print >>logfile,'  spw %s CASA I = %7.3f P = %7.3f F = %7.4f X = %7.2f deg' % (spwid,ipol,ppol,fpol,rlpd_deg)

        if (aipsmodel.has_key(src)):
            iflx = aipsmodel[src][spwid][0]/1000.0
            fflx = aipsmodel[src][spwid][1]
            rlaips_deg = aipsmodel[src][spwid][2]
            rlaips = rlaips_deg*pl.pi/180.0
            
            pflx = iflx*fflx
            qflx = pflx*cos(rlaips)
            uflx = pflx*sin(rlaips)
            vflx = 0.0
            
            print '  spw %s AIPS I = %7.3f P = %7.3f F = %7.4f X = %7.2f deg' % (spwid,iflx,pflx,fflx,rlaips_deg)
            print >>logfile,'  spw %s AIPS I = %7.3f P = %7.3f F = %7.4f X = %7.2f deg' % (spwid,iflx,pflx,fflx,rlaips_deg)
    
    
    # Done with spw
    print ""
    print >>logfile,""

# Should see something like:
#
# R-L phase residual from image of 0137+331 :
# 
# R-L Phase Correction SPW 0 =    0.50 deg
# R-L Phase Correction SPW 1 =    0.17 deg
# 
# Final Stats:
# 
# Source 0137+331 :
#   spw 0 CASA I =   5.320 P =   0.229 F =  0.0430 X = -148.50 deg
#   spw 1 CASA I =   5.362 P =   0.236 F =  0.0439 X = -148.17 deg
# 
# Source 2202+422 :
#   spw 0 CASA I =   2.570 P =   0.185 F =  0.0719 X =  -57.71 deg
#   spw 1 CASA I =   2.571 P =   0.188 F =  0.0733 X =  -53.43 deg
# 
# Source 1743-038 :
#   spw 0 CASA I =   5.545 P =   0.073 F =  0.0131 X =   -1.69 deg
#   spw 1 CASA I =   5.543 P =   0.078 F =  0.0141 X =    4.38 deg
# 
# Source 1924-292 :
#   spw 0 CASA I =   8.243 P =   0.058 F =  0.0070 X =   12.19 deg
#   spw 1 CASA I =   8.184 P =   0.073 F =  0.0089 X =    7.27 deg
# 
# Source 2136+006 :
#   spw 0 CASA I =  10.491 P =   0.153 F =  0.0146 X = -171.61 deg
#   spw 1 CASA I =  10.521 P =   0.145 F =  0.0138 X = -160.12 deg
# 
# Source 2253+161 :
#   spw 0 CASA I =   9.113 P =   0.527 F =  0.0578 X =    4.42 deg
#   spw 1 CASA I =   9.096 P =   0.524 F =  0.0576 X =    4.72 deg
# 
# Source 2355+498 :
#   spw 0 CASA I =   1.340 P =   0.003 F =  0.0022 X = -162.58 deg
#   spw 1 CASA I =   1.355 P =   0.003 F =  0.0023 X =  151.08 deg
# 
# Source 0319+415 :
#   spw 0 CASA I =  14.147 P =   0.041 F =  0.0029 X =  -79.67 deg
#   spw 1 CASA I =  14.229 P =   0.043 F =  0.0030 X = -130.02 deg
# 
# Source 0359+509 :
#   spw 0 CASA I =   5.351 P =   0.090 F =  0.0168 X = -124.92 deg
#   spw 1 CASA I =   5.333 P =   0.094 F =  0.0177 X = -128.78 deg
# 
#=====================================================================
#
# Done
#
logfile.close()
print "Results are in "+outfile

#
#=====================================================================
#
# Now save stat dictionaries using Pickle
pickfile = prefix + '.pickle'
f = open(pickfile,'w')
p = pickle.Pickler(f)
p.dump(clnmodel)
p.dump(polmodel)
f.close()

print ""
print "Dictionaries clnmodel,polmodel saved in "+pickfile
print "Use Pickle to retrieve"
print ""

# e.g.
# f = open(pickfile)
# u = Unpickler(f)
# clnmodel = u.load()
# polmodel = u.load()
# f.close()

print ""
print "Completed Processing"
