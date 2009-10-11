##########################################################################
#                                                                        #
# Use Case Script for CLASS Graviational Lens B1608+656                  #
#                                                                        #
# Created      STM 2008-08-05 (Beta Patch 2.0) regression version        #
# Updated      STM 2008-08-21 (Beta Patch 3.0) imfit                     #
# Revised      STM 2009-07-06 (Beta Patch 4, v2.4) final                 #
# Revised      STM 2009-07-13 (Release R0, v3.0) initial                 #
# Revised      STM 2009-10-07 (Release R0, v3.0) final status            #
#                                                                        #
# Features Tested:                                                       #
#    The script illustrates end-to-end processing with CASA              #
#    as depicted in the following flow-chart.                            #
#                                                                        #
#    Filenames will have the <prefix> = 'b1608.demo'                     #
#                                                                        #
#    Input Data           Process          Output Data                   #
#                                                                        #
# AM484_B950818.xp1 --> importvla  -->  <prefix>.ms   +                  #
#   (8.4GHz,               |            <prefix>.ms.flagversions         #
#    2x50MHz IF,           v                                             #
#    A-config)          listobs    -->  casapy.log                       #
#                          |                                             #
#                          v                                             #
#                        setjy                                           #
#                          |                                             #
#                          v                                             #
#                       gaincal    -->  <prefix>.gcal                    #
#                          |                                             #
#                          v                                             #
#                      applycal    -->  <prefix>.ms                      #
#                          |                                             #
#                          v                                             #
#                        split     -->  <prefix>.<src>.split.ms          #
#                          |                                             #
#                          v                                             #
#                   (exportuvfits) -->  <prefix>.<src>.split.uvfits      #
#                          |                                             #
#                          v                                             #
#                        clean     -->  <prefix>.clean1.image +          #
#                          |            <prefix>.clean1.model +          #
#                          |            <prefix>.clean1.residual         #
#                          v                                             #
#                       gaincal    -->  <prefix>.selfcal                 #
#                          |                                             #
#                          v                                             #
#                      applycal    -->  <prefix>.<src>.split.ms          #
#                          |                                             #
#                          v                                             #
#                        clean     -->  <prefix>.clean2.image +          #
#                          |            <prefix>.clean2.model +          #
#                          |            <prefix>.clean2.residual         #
#                          v                                             #
#                    (exportfits)  -->  <prefix>.clean.fits              #
#                          |                                             #
#                          v                                             #
#                       imhead     -->  casapy.log                       #
#                          |                                             #
#                          v                                             #
#                       imstat     -->  xstat (return value)             #
#                          |                                             #
#                          v                                             #
#                       imfit      -->  xfit (return value)              #
#                          |                                             #
#                          v                                             #
##########################################################################

import time
import os

scriptmode = F

# Enable benchmarking?
benchmarking = True

# 
# Set up some useful variables
#
# Get to path to the CASA home and stip off the name
pathname=os.environ.get('CASAPATH').split()[0]

# This is where the data will be
datapath='./'

# The prefix to use for all output files
prefix='b1608.demo'

# Clean up old files
os.system('rm -rf '+prefix+'*')

# Start timing
if benchmarking:
    startTime=time.time()
    startProc=time.clock()

#
#=====================================================================
#
# Import the data from FITS to MS
#
print '--ImportVLA--'

# Safest to start from task defaults
default('importvla')

# Set up the MS filename and save as new global variable
msfile = prefix + '.ms'

# Use task importuvfits
archivefiles = [datapath+'AM484_B950818.xp1']
vis = msfile

# The X-band only
bandname = 'X'

saveinputs('importvla',prefix+'.saved.importvla')

# Pause script if you are running in scriptmode
if scriptmode:
    inp()
    user_check=raw_input('Return to continue script\n')

importvla()

#
# Note that there will be a b1950.demo.ms.flagversions
# there containing the initial flags as backup for the main ms
# flags.
#
#=====================================================================
#
# List a summary of the MS
#
print '--Listobs--'

# Don't default this one and make use of the previous setting of
# vis.  Remember, the variables are GLOBAL!

# You may wish to see more detailed information, like the scans.
# In this case use the verbose = True option
verbose = True

listobs()

# You should get in your logger window and in the casapy.log file
# something like:
#
# MeasurementSet Name:  /home/sandrock/smyers/Testing/Patch2/B1608/am484.ms      MS Version 2
#	
#	   Observer: unavailable     Project: AM484  
#	Observation: VLA
#   Data records: 103194       Total integration time = 1680 seconds
#	   Observed from   23:56:05   to   00:24:05
#   
#	   ObservationID = 0         ArrayID = 0
#	  Date        Timerange                Scan  FldId FieldName      SpwIds
#	  18-Aug-1995/23:56:05.0 - 23:58:55.0     1      0 1642+689       [0, 1]
#	  19-Aug-1995/00:00:05.0 - 00:04:15.0     2      1 1600+434       [0, 1]
#	              00:05:45.0 - 00:07:45.0     3      0 1642+689       [0, 1]
#	              00:08:15.0 - 00:14:25.0     4      2 1608+656       [0, 1]
#	              00:14:55.0 - 00:17:35.0     5      0 1642+689       [0, 1]
#	              00:18:05.0 - 00:20:45.0     6      3 1638+625       [0, 1]
#	              00:21:15.0 - 00:24:05.0     7      0 1642+689       [0, 1]
#   Fields: 4
#	  ID   Code Name          Right Ascension  Declination   Epoch   
#	  0    A    1642+689      16:42:07.85      +68.56.39.76  J2000   
#	  1         1600+434      16:01:40.50      +43.16.47.00  J2000   
#	  2         1608+656      16:09:13.96      +65.32.28.97  J2000   
#	  3         1638+625      16:38:28.21      +62.34.44.32  J2000   
#   Spectral Windows:  (2 unique spectral windows and 1 unique polarization setups)
#	  SpwID  #Chans Frame Ch1(MHz)    ChanWid(kHz)TotBW(kHz)  Ref(MHz)    Corrs           
#	  0           1 TOPO  8414.9      50000       50000       8414.9      RR  RL  LR  LL  
#	  1           1 TOPO  8464.9      50000       50000       8464.9      RR  RL  LR  LL  
#   Feeds: 27: printing first row only
#	  Antenna   Spectral Window     # Receptors    Polarizations
#	  1         -1                  2              [         R, L]
#   Antennas: 27:
#	  ID   Name  Station   Diam.    Long.         Lat.         
#	  0    VA18  VLA:W48   25.0 m   -107.42.44.3  +33.50.52.1  
#	  1    VA20  VLA:W40   25.0 m   -107.41.13.5  +33.51.43.1  
#	  2    VA08  VLA:W8    25.0 m   -107.37.21.6  +33.53.53.0  
#	  3    VA03  VLA:W24   25.0 m   -107.38.49.0  +33.53.04.0  
#	  4    VA12  VLA:W16   25.0 m   -107.37.57.4  +33.53.33.0  
#	  5    VA02  VLA:W32   25.0 m   -107.39.54.8  +33.52.27.2  
#	  6    VA28  VLA:W64   25.0 m   -107.46.20.1  +33.48.50.9  
#	  7    VA21  VLA:W72   25.0 m   -107.48.24.0  +33.47.41.2  
#	  8    VA10  VLA:W56   25.0 m   -107.44.26.7  +33.49.54.6  
#	  9    VA17  VLA:E48   25.0 m   -107.30.56.1  +33.51.38.4  
#	  10   VA07  VLA:E40   25.0 m   -107.32.35.4  +33.52.16.9  
#	  11   VA04  VLA:E8    25.0 m   -107.36.48.9  +33.53.55.1  
#	  12   VA16  VLA:E24   25.0 m   -107.35.13.4  +33.53.18.1  
#	  13   VA22  VLA:E16   25.0 m   -107.36.09.8  +33.53.40.0  
#	  14   VA19  VLA:E32   25.0 m   -107.34.01.5  +33.52.50.3  
#	  15   VA06  VLA:E64   25.0 m   -107.27.00.1  +33.50.06.7  
#	  16   VA24  VLA:E72   25.0 m   -107.24.42.3  +33.49.18.0  
#	  17   VA05  VLA:E56   25.0 m   -107.29.04.1  +33.50.54.9  
#	  18   VA11  VLA:N48   25.0 m   -107.37.38.1  +33.59.06.2  
#	  19   VA23  VLA:N40   25.0 m   -107.37.29.5  +33.57.44.4  
#	  20   VA27  VLA:N8    25.0 m   -107.37.07.5  +33.54.15.8  
#	  21   VA15  VLA:N32   25.0 m   -107.37.22.0  +33.56.33.6  
#	  22   VA25  VLA:N24   25.0 m   -107.37.16.1  +33.55.37.7  
#	  23   VA14  VLA:N16   25.0 m   -107.37.10.9  +33.54.48.0  
#	  24   VA01  VLA:N64   25.0 m   -107.37.58.7  +34.02.20.5  
#	  25   VA26  VLA:N72   25.0 m   -107.38.10.5  +34.04.12.2  
#	  26   VA09  VLA:N56   25.0 m   -107.37.47.9  +34.00.38.4  
#   
#	
#	Tables:
#	   MAIN                  103194 rows     
#	   ANTENNA                   27 rows     
#	   DATA_DESCRIPTION           2 rows     
#	   DOPPLER                    2 rows     
#	   FEED                      27 rows     
#	   FIELD                      4 rows     
#	   FLAG_CMD             <empty>  
#	   FREQ_OFFSET         <absent>  
#	   HISTORY                    6 rows     
#	   OBSERVATION                1 row      
#	   POINTING             <empty>  
#	   POLARIZATION               1 row      
#	   PROCESSOR            <empty>  
#	   SOURCE                     4 rows     
#	   SPECTRAL_WINDOW            2 rows     
#	   STATE                <empty>  
#	   SYSCAL              <absent>  
#	   WEATHER             <absent>  
#
#=====================================================================
#
# Now use plotxy to plot the data and do any preliminary editing
#
print '--Plotxy--'
default('plotxy')

vis = msfile

field = '1642+689'
spw = '0,1'

selectdata=T
correlation='RR,LL'

xaxis = 'uvdist'
yaxis = 'amp'
datacolumn = 'data'

saveinputs('plotxy',prefix+'.saved.plotxy.initial.amp')

print "  1642+689 amplitude vs. uv distance"

if scriptmode:
    interactive = True
    figfile = ''
    plotxy()
    print "  Looks good, no editing required"
    user_check=raw_input('Return to continue script\n')

else:
    interactive = False
    figfile = prefix+'.'+field+'.plotxy.initial.png'
    plotxy()

#
#=====================================================================
#
# Set the fluxes of the primary calibrator(s)
#
print '--Setjy--'
default('setjy')

vis = msfile

#
# Unfortunately there were no flux standards observed this run
# We will assume that 1642+689 has a X-band flux of 0.82 Jy
# Guesstimated from the flux calibration database when it started
# in 1996.
# 
field = '1642+689'

# This is 8.4GHz A-config in 2 x 50MHz IFs
spw = '0,1'

# 1642+689 is sufficiently unresolved that we dont need a model image.
modimage = ''

# We have to set the flux density manually
print "  Set the flux density of 1642+689 to 0.82 Jy"
fluxdensity = [0.82,0.0,0.0,0.0]

saveinputs('setjy',prefix+'.saved.setjy')

# Pause script if you are running in scriptmode
if scriptmode:
    inp()
    user_check=raw_input('Return to continue script\n')

setjy()

#
#=====================================================================
#
# Gain calibration
#
print '--Gaincal--'
default('gaincal')

# solve for the time-dependent antenna gains

vis = msfile

# set the name for the output gain caltable
gtable = prefix + '.gcal'
caltable = gtable

# Gain calibrator is 1642+689 (FIELD_ID 0)
field = '1642+689'

# We have two spectral windows (SPW 0,1)
spw = '0,1'

# Exclude longest baselines (possible droop beyond 800klambda)
selectdata = T
uvrange = '0~600klambda'

# In this band we do not need a-priori corrections for
# antenna gain-elevation curve or atmospheric opacity
# especially given that the sources are all together on sky
# (above 8.4GHz you would want these)
gaincurve = False
opacity = 0.0

# scan-based G solutions for both amplitude and phase
gaintype = 'G'
solint = 'inf'
combine = ''
calmode = 'ap'

# minimum SNR allowed
minsnr = 3.0

print "  scan-based amplitude and phase cal"

# reference antenna VA08 (VLA:W8)
refant = 'VA08'

saveinputs('gaincal',prefix+'.saved.gaincal')

if scriptmode:
    inp()
    user_check=raw_input('Return to continue script\n')

gaincal()

#
#=====================================================================
#
# Now use plotcal to examine the gain solutions
#
print '--Plotcal--'
default('plotcal')

caltable = gtable

# Set up 2x1 panels - upper panel amp vs. time
subplot = 211
yaxis = 'amp'
# No output file yet (wait to plot next panel)

saveinputs('plotcal',prefix+'.saved.plotcal.gcal.amp')

if scriptmode:
    showgui = True
else:
    showgui = False

plotcal()
#
# Set up 2x1 panels - lower panel phase vs. time
subplot = 212
yaxis = 'phase'

saveinputs('plotcal',prefix+'.saved.plotcal.gcal.phase')

#
# The amp and phase coherence looks good

# Pause script if you are running in scriptmode
if scriptmode:
    # If you want to do this interactively and iterate over antenna, set
    #iteration = 'antenna'
    showgui = True
    plotcal()
    print " You can see an antenna that is off, use Mark and Locate to ID"
    print " This is VA24"
    user_check=raw_input('Return to continue script\n')
else:
    # No GUI for this script
    showgui = False
    # Now send final plot to file in PNG format (via .png suffix)
    figfile = caltable + '.plotcal.png'
    plotcal()

#
#=====================================================================
#
print "--Flagdata--"
default('flagdata')

vis = msfile
spw='0,1'
mode='manualflag'

# VA24 is off in calibration - flag it

print "  flag VA24 for all times"

timerange=''
correlation=''
antenna = 'VA24'

saveinputs('flagdata',prefix+'.saved.flagdata.VA24')

flagdata()

if scriptmode:
    pass
else:
    # If not interactive, then flag bad stuff for VA02 in one timeslot
    print "  flag VA02 for one time at 00:05:53"
    
    timerange='1995/08/19/00:05:54.0~00:05:56.0'
    correlation=''
    antenna = 'VA02'
    
    saveinputs('flagdata',prefix+'.saved.flagdata.VA02')
    
    flagdata()

#
#=====================================================================
#
# Apply our calibration solutions to the data
# (This will put calibrated data into the CORRECTED_DATA column)
#
print '--ApplyCal--'
default('applycal')

vis = msfile

# We want to transfer from 1642+689 to itself and the targets

# Start with the gain table
gaintable = [gtable]

# pick the 1642+689 out of the gain table for transfer
gainfield = ['1642+689']

# interpolation using linear for gain
interp = ['linear']

# both spw, default mapping
spw = ''
selectdata = False

# as before
gaincurve = False
opacity = 0.0

# select all of the fields
field = ''

saveinputs('applycal',prefix+'.saved.applycal')

# Pause script if you are running in scriptmode
if scriptmode:
    inp()
    user_check=raw_input('Return to continue script\n')

applycal()

#
#=====================================================================
#
# Now use plotxy to plot the calibrated target data
#
print '--Plotxy--'
default('plotxy')

vis = msfile

field = '1642+689'
spw = '0,1'

selectdata=T
correlation='RR,LL'

# Put any time average here
#averagemode = 'vector'
#timebin = '0'

xaxis = 'time'
yaxis = 'amp'
datacolumn = 'corrected'

print "  Final calibrated data amplitudes"

figfile = ''
if scriptmode:
    interactive = True
    figfile = ''
    saveinputs('plotxy',prefix+'.saved.plotxy.final.amp')
    plotxy()
    print "  Amplitude vs. time"
    print "  You can Mark, Locate and Flag the bad time in second scan"
    user_check=raw_input('Return to continue script\n')
else:
    interactive = False
    figfile = vis + '.plotxy.final.amp.png'
    saveinputs('plotxy',prefix+'.saved.plotxy.final.amp')
    print "  Amplitude vs. time to "+figfile
    plotxy()

plotxy()

print "  Final calibrated data phases"

yaxis = 'phase'

# Pause script if you are running in scriptmode
if scriptmode:
    interactive = True
    figfile = ''
    saveinputs('plotxy',prefix+'.saved.plotxy.final.phase')
    plotxy()
    print "  Now phase vs. time"
    print "  Notice the effect of the interpolation - will need to selfcal"
    user_check=raw_input('Return to continue script\n')
else:
    interactive = False
    # Now send final plot to file in PNG format (via .png suffix)
    figfile = vis + '.plotxy.final.phase.png'
    saveinputs('plotxy',prefix+'.saved.plotxy.final.phase')
    print "  Phase vs. time to "+figfile
    plotxy()

#=====================================================================
#
# Split the sources out, pick off the CORRECTED_DATA column
#
print '--Split--'
default('split')

vis = msfile
spw = ''
datacolumn = 'corrected'

field = '1642+689'
calsplitms = prefix + '.'+field+'.split.ms'
outputvis = calsplitms

print "  splitting source "+field

saveinputs('split',prefix+'.saved.split.'+field)

# Pause script if you are running in scriptmode
if scriptmode:
    inp()
    user_check=raw_input('Return to continue script\n')

split()

print "  Created "+outputvis

field = '1608+656'
srcsplitms = prefix + '.'+field+'.split.ms'
outputvis = srcsplitms

print ""
print "  splitting source "+field

saveinputs('split',prefix+'.saved.split.'+field)

# Pause script if you are running in scriptmode
if scriptmode:
    inp()
    user_check=raw_input('Return to continue script\n')

split()

print "  Created "+outputvis

#=====================================================================
#
# Here is how to export the data as UVFITS
# Start with the split file.
# Since this is a split dataset, the calibrated data is
# in the DATA column already.
# Write as a multisource UVFITS (with SU table)
# even though it will have only one field in it
# Run asynchronously so as not to interfere with other tasks
# (BETA: also avoids crash on next importuvfits)
#
#print '--Export UVFITS--'
#default('exportuvfits')
#
#srcuvfits = prefix + '.1608+656.split.uvfits'
#
#vis = srcsplitms
#fitsfile = srcuvfits
#datacolumn = 'data'
#multisource = True
#async = True
#
#saveinputs('exportuvfits',prefix+'.saved.1608+656.exportuvfits')
#
#myhandle = exportuvfits()
#
#print "The return value for this exportuvfits async task for tm is "+str(myhandle)

#
#=====================================================================
# Done with calibration
#=====================================================================
#
# Here is how to make a dirty image
#
#print '--Clean (dirty image)--'
#default('clean')

#vis = srcsplitms
#imname = prefix + '.dirty'
#imagename = imname
#
#mode = 'mfs'
#
#imsize = [256,256]
#cell = [0.05.,0.05]
#weighting = 'briggs'
#robust = 0.5

# No cleaning
#niter = 0

#saveinputs('clean',prefix+'.saved.invert')

# Pause script if you are running in scriptmode
#if scriptmode:
#    inp()
#    user_check=raw_input('Return to continue script\n')
#
#clean()

#dirtyimage = imname+'.image'
 
# Get the dirty image cube statistics
#dirtystats = imstat(dirtyimage)

# Could also image the calibrator using vis=calsplitms

#=====================================================================
#
# Now clean an initial image of the lens 1608+656
#
print '--Clean (cycle1)--'
default('clean')

# Pick up our split source continuum-subtracted data
vis = srcsplitms

# Make an image root file name
imname = prefix + '.clean1'
imagename = imname

# This is a single-source MS with 2 spw
# Set up the output image cube
mode = 'mfs'

# Set the cell size (arcsec) for A-config X-band
cell = [0.05,0.05]

# Set the output image size
imsize = [256,256]

# Do a Hogbom style clean as the PSF is poor
psfmode = 'hogbom'
# Cotton-Schwab iterations
imagermode='csclean'

# Fix maximum number of iterations
niter = 1000

# Also set flux residual threshold (in mJy)
threshold=0.5

# Set up the weighting
# Use Briggs weighting (a moderate value, on the uniform side)
weighting = 'briggs'
robust = 0.5

# Set a cleanbox around the center
#mask = [102,78,157,139]

# But if you had a cleanbox saved in a file, e.g. "regionfile.txt"
# you could use it:
#mask='regionfile.txt'
#
# If you don't want any clean boxes or masks, then
#mask = ''

# Create scratch columns for selfcal (NEW RELEASE R0 v3.0.0)
calready = True

# If you want interactive clean set to True
if scriptmode:
    interactive=True
    npercycle=25
else:
    interactive=False
    # In regression mode box around the 4 components
    mask = [[124,125,132,133],[110,116,117,124],[109,85,117,94],[147,100,155,108]]
    niter = 100

saveinputs('clean',prefix+'.saved.clean1')

# Pause script if you are running in scriptmode
if scriptmode:
    inp()
    print "  First time you will see only 1 component, box and iterate"
    print "  You should eventually be able to box/clean the 3 brightest components"
    print "  Use Stop Cleaning button when you have cleaned out obvious emission"
    print "  There will still be artifacts - will have to selfcal"
    print ""
    user_check=raw_input('Return to continue script\n')

clean()

# Should find stuff in the logger like:
# Beam used in restoration: 0.291649 by 0.249021 (arcsec) at pa 22.9926 (deg) 

# It will have made the images:
# -----------------------------
# b1608.demo.clean1.image
# b1608.demo.clean1.model
# b1608.demo.clean1.residual

clnimage = imname+'.image'

#=====================================================================
#
# Now view the image
#
if scriptmode:
    print '--View image--'
    viewer(clnimage,'image')
    print "  Quit viewer when done"
    user_check=raw_input('Return to continue script\n')

#=====================================================================
#
# Get the image statistics
#
print '--Imstat--'
default('imstat')

imagename = clnimage

# Do whole image
box = ''
# or you could stick to the cleanbox
#box = '102,78,157,139'

stats1 = imstat()

# Do off-source box
box = '18,148,99,229'

stats1_offsrc = imstat()

# Statistics will printed to logfile, and the return
# value will contain a dictionary of the statistics

print ""
print "  First clean cycle:"
print "  ------------------"
print "     On-source max = "+str(stats1['max'][0])
print "    Off-source rms = "+str(stats1_offsrc['sigma'][0])
print "         S/N ratio = "+str(stats1['max'][0]/stats1_offsrc['sigma'][0] )
print ""

#
# NOTE: should now have a rms of around 0.0005 Jy and S/N ratio around 36
# We need some self-calibration to improve this.

#
#=====================================================================
#
# Gain self-calibration
#
print '--Gaincal (selfcal)--'
default('gaincal')

# the model should be in the MODEL_DATA column after clean
vis = srcsplitms

# set the name for the output gain caltable
selfcaltable = prefix + '.selfcal'
caltable = selfcaltable

# G solutions for phase only
gaintype = 'G'
solint = '30.0'
combine = ''
calmode = 'ap'

# minimum SNR allowed
minsnr = 3.0

print "  amp and phase self-cal with solint = "+str(solint)

# reference antenna VA08 (VLA:W8)
refant = 'VA08'

saveinputs('gaincal',prefix+'.saved.selfcal')

if scriptmode:
    inp()
    user_check=raw_input('Return to continue script\n')

gaincal()

#
#=====================================================================
#
print '--Plotcal--'
default('plotcal')

caltable = selfcaltable

subplot = 211
yaxis = 'amp'

if scriptmode:
    showgui = T
    saveinputs('plotcal',prefix+'.saved.plotcal.selfcal.amp')
    plotcal()
else:
    # No GUI for this script
    showgui = F
    figfile = ''
    saveinputs('plotcal',prefix+'.saved.plotcal.selfcal.amp')
    plotcal()

subplot = 212
yaxis = 'phase'

# Pause script if you are running in scriptmode
if scriptmode:
    # If you want to do this interactively and iterate over antenna, set
    #iteration = 'antenna'
    showgui = True
    saveinputs('plotcal',prefix+'.saved.plotcal.selfcal.phase')
    plotcal()
    print "  the phase coherence looks good"
    user_check=raw_input('Return to continue script\n')
else:
    # No GUI for this script
    showgui = False
    # Now send final plot to file in PNG format (via .png suffix)
    figfile = caltable + '.plotcal.png'
    saveinputs('plotcal',prefix+'.saved.plotcal.selfcal.phase')
    plotcal()

#
#=====================================================================
#
print '--ApplyCal (selfcal)--'
default('applycal')

vis = srcsplitms

gaintable = [selfcaltable]

# interpolation using linear for gain
interp = ['linear']

saveinputs('applycal',prefix+'.saved.applycal.selfcal')

# Pause script if you are running in scriptmode
if scriptmode:
    inp()
    user_check=raw_input('Return to continue script\n')

applycal()

#=====================================================================
# SECOND CLEAN CYCLE
#=====================================================================
#
print '--Clean (cycle2)--'
default('clean')

# Pick up our split source continuum-subtracted data
vis = srcsplitms

# Make an image root file name
imname = prefix + '.clean2'
imagename = imname

# This is a single-source MS with 2 spw
# Set up the output image cube
mode = 'mfs'

# Set the cell size (arcsec) for A-config X-band
cell = [0.05,0.05]

# Set the output image size
imsize = [256,256]

# Do a Hogbom style clean as the PSF is poor
psfmode = 'hogbom'
# Cotton-Schwab iterations
imagermode='csclean'

# Fix maximum number of iterations
niter = 1000

# Also set flux residual threshold (in mJy)
threshold=0.07

# Set up the weighting
# Use Briggs weighting (a moderate value, on the uniform side)
weighting = 'briggs'
robust = 0.5

# Set a cleanbox around the center
#mask = [102,78,157,139]
# or around the individual components
#mask = [[124,125,132,133],[110,116,117,124],[109,85,117,94],[147,100,155,108]]
# But if you had a cleanbox saved in a file, e.g. "regionfile.txt"
# you could use it:
#mask='regionfile.txt'
#
# If you don't want any clean boxes or masks, then
#mask = ''

# Create scratch columns for selfcal (NEW RELEASE R0 v3.0.0)
calready = True

# If you want interactive clean set to True
if scriptmode:
    interactive=True
    npercycle=25
else:
    interactive=False
    # In regression mode box around the 4 components
    mask = [[124,125,132,133],[110,116,117,124],[109,85,117,94],[147,100,155,108]]
    niter = 500

saveinputs('clean',prefix+'.saved.clean2')

# Pause script if you are running in scriptmode
if scriptmode:
    inp()
    print "You should now see all 4 quad-lens components clearly"
    user_check=raw_input('Return to continue script\n')

clean()

clnimage = imname+'.image'

#=====================================================================
#
# Now view the image
#
if scriptmode:
    print '--View image--'
    viewer(clnimage,'image')
    user_check=raw_input('Return to continue script\n')

#=====================================================================
# Final Analysis
#=====================================================================
#
# Here is how to export the Final CLEAN Image as FITS
# Run asynchronously so as not to interfere with other tasks
# (BETA: also avoids crash on next importfits)
#
#print '--Final Export CLEAN FITS--'
#default('exportfits')
#
#clnfits = prefix + '.clean.fits'
#
#imagename = clnimage
#fitsimage = clnfits
#async = True
#
#saveinputs('exportfits',prefix+'.saved.exportfits')
#
#myhandle2 = exportfits()
#
#print "The return value for this exportfits async task for tm is "+str(myhandle2)

#=====================================================================
#
# Print the image header
#
print '--Imhead--'
default('imhead')

imagename = clnimage

mode = 'summary'

imhead()

# A summary of the header will be seen in the logger

#=====================================================================
#
# Get the final image statistics
#
print '--Imstat (final)--'
default('imstat')

imagename = clnimage

# Do whole image
box = ''
# or you could stick to the cleanbox
#box = '102,78,157,139'

stats = imstat()

# Do off-source box
box = '18,148,99,229'

stats_offsrc = imstat()

print ""
print "  Final clean cycle:"
print "  ------------------"
print "     On-source max = "+str(stats['max'][0])
print "    Off-source rms = "+str(stats_offsrc['sigma'][0])
print "         S/N ratio = "+str(stats['max'][0]/stats_offsrc['sigma'][0] )
print ""

#
# NOTE: should now have a rms of around 0.00006 Jy and S/N ratio around 290
# You can still see artifacts in the image so another round of selfcal will
# certainly help.  We leave that to you!

if benchmarking:
    endProc=time.clock()
    endTime=time.time()

#=====================================================================
#
# Image-plane Gaussian fitting
#
print '--Imfit--'
default('imfit')

imagename = clnimage

# Let it "deconvolve" from clean beam
# NOTE: THIS CURRENTLY DOESNT WORK IN THE OUTPUT CL
#usecleanbeam = True
usecleanbeam = False

# This will currently fit only a single component at a time
# This is OK since the components are well-separated and not blended

# Box around component A
box = '121,121,136,136'

# Will return a component list
# Look in the logger for better formatted output
xfit_A_cl = imfit()

# Use toolkit to convert componentlist to dictionary
xfit_A = xfit_A_cl.getcomponent(0)

# Now component B
box = '108,114,120,126'
xfit_B_cl = imfit()
xfit_B = xfit_B_cl.getcomponent(0)

# Now component C
box = '108,84,120,96'
xfit_C_cl = imfit()
xfit_C = xfit_C_cl.getcomponent(0)

# Finally component D
box = '144,98,157,110'
xfit_D_cl = imfit()
xfit_D = xfit_D_cl.getcomponent(0)

print ""
print "  Imfit Results:"
print "  --------------"
print "  Component A  Flux = %6.4f %s   Bmaj = %6.4f %s " % ( xfit_A['flux']['value'][0], xfit_A['flux']['unit'], xfit_A['shape']['majoraxis']['value'], xfit_A['shape']['majoraxis']['unit'] )
print "  Component B  Flux = %6.4f %s   Bmaj = %6.4f %s " % ( xfit_B['flux']['value'][0], xfit_B['flux']['unit'], xfit_B['shape']['majoraxis']['value'], xfit_B['shape']['majoraxis']['unit'] )
print "  Component C  Flux = %6.4f %s   Bmaj = %6.4f %s " % ( xfit_C['flux']['value'][0], xfit_C['flux']['unit'], xfit_C['shape']['majoraxis']['value'], xfit_C['shape']['majoraxis']['unit'] )
print "  Component D  Flux = %6.4f %s   Bmaj = %6.4f %s " % ( xfit_D['flux']['value'][0], xfit_D['flux']['unit'], xfit_D['shape']['majoraxis']['value'], xfit_D['shape']['majoraxis']['unit'] )
print ""

#=====================================================================
# Done with processing
#=====================================================================
# Set up an output logfile
import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())

outfile = 'out.'+prefix+'.'+datestring+'.log'
logfile=open(outfile,'w')
print >>logfile,'Results for '+prefix+' :'
print >>logfile,""

#=====================================================================
#
# Can do some image statistics if you wish
# Treat this like a regression script
# WARNING: currently requires toolkit
#
print ' B1608+656 results '
print ' ================= '

print >>logfile,' B1608+656 results '
print >>logfile,' ================= '

#
# Now use the stats produced by imstat above
#
thistest_immax = stats['max'][0]
oldtest_immax = 0.0170652698725
diff_immax = abs((oldtest_immax-thistest_immax)/oldtest_immax)

print ' Clean image  on-src max = ',thistest_immax
print '   Previous:  on-src max = ',oldtest_immax
print '   Difference (fractional) = ',diff_immax
print ''

print >>logfile,' Clean Image  on-src max = ',thistest_immax
print >>logfile,'   Previous:  on-src max = ',oldtest_immax
print >>logfile,'   Difference (fractional) = ',diff_immax
print >>logfile,''

thistest_imrms = stats_offsrc['rms'][0]
oldtest_imrms = 5.87700160395e-05
diff_imrms = abs((oldtest_imrms-thistest_imrms)/oldtest_imrms)

print ' Clean image off-src rms = ',thistest_imrms
print '   Previous: off-src rms = ',oldtest_imrms
print '   Difference (fractional) = ',diff_imrms
print ''

print >>logfile,' Clean image off-src rms = ',thistest_imrms
print >>logfile,'   Previous: off-src rms = ',oldtest_imrms
print >>logfile,'   Difference (fractional) = ',diff_imrms
print >>logfile,''

thistest_Aflux = xfit_A['flux']['value'][0]
oldtest_Aflux = 0.01803075
diff_Aflux = abs((oldtest_Aflux-thistest_Aflux)/oldtest_Aflux)

print ' Component A  Total Flux = ',thistest_Aflux
print '   Previous: Comp A Flux = ',oldtest_Aflux
print '   Difference (fractional) = ',diff_Aflux
print ''

print >>logfile,' Component A  Total Flux = ',thistest_Aflux
print >>logfile,'   Previous: Comp A Flux = ',oldtest_Aflux
print >>logfile,'   Difference (fractional) = ',diff_Aflux
print >>logfile,''

final_status = (diff_immax<0.01)&(diff_imrms<0.01)&(diff_Aflux<0.01)
if final_status:
    regstate=True
    print >>logfile,'---'
    print >>logfile,'Passed Regression test for B1608'
    print >>logfile,'---'
    print 'Passed Regression test for B1608'
else:
    regstate=False
    print >>logfile,'----FAILED Regression test for B1608'
    print '----FAILED Regression test for B1608'

if benchmarking:
    walltime = (endTime - startTime)
    cputime = (endProc - startProc)
    
    print ''
    print '  Total wall clock time was: %10.3f ' % walltime
    print '  Total CPU        time was: %10.3f ' % cputime
    print ''

    print >>logfile,''
    print >>logfile,'  Total wall clock time was: %10.3f ' % walltime
    print >>logfile,'  Total CPU        time was: %10.3f ' % cputime
    print >>logfile,''

#
#=====================================================================
# Done
#
logfile.close()
print "Results are in "+outfile

print "Done with B1608+656 Tutorial"
#
##########################################################################
