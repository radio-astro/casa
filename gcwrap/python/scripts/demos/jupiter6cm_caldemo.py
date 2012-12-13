######################################################################
#                                                                    #
# Use Case Demo Script for Jupiter 6cm VLA                           #
# Trimmed down from Use Case jupiter6cm_usecase.py                   #
#                                                                    #
# Assumes you have already flagged using jupiter6cm_flagdemo.py      #
# Will do calibration steps                                          #
#                                                                    #
# Updated STM 2008-05-26 (Beta Patch 2.0) new cal parameters         #
# Updated STM 2008-06-12 (Beta Patch 2.0) for summer school demo     #
#                                                                    #
######################################################################

import time
import os

# 
#=====================================================================
#
# This script has some interactive commands: scriptmode = True
# if you are running it and want it to stop during interactive parts.

scriptmode = True

#=====================================================================
#
# Set up some useful variables - these will be set during the script
# also, but if you want to restart the script in the middle here
# they are in one place:

prefix = 'jupiter6cm.demo'

msfile = prefix + '.ms'

#
#=====================================================================
#
# Special prefix for this calibration demo output
#
calprefix = prefix + '.cal'

# Clean up old files
os.system('rm -rf '+calprefix+'*')

#
#=====================================================================
# Calibration variables
#
# spectral windows to process
usespw = ''
usespwlist = ['0','1']

# prior calibration to apply
usegaincurve = True
gainopacity = 0.0

# reference antenna 11 (11=VLA:N1)
calrefant = '11'

gtable = calprefix + '.gcal'
ftable = calprefix + '.fluxscale'
atable = calprefix + '.accum'

#
#=====================================================================
# Polarization calibration setup
#
dopolcal = True

ptable = calprefix + '.pcal'
xtable = calprefix + '.polx'

# Pol leakage calibrator
poldfield = '0137+331'

# Pol angle calibrator
polxfield = '1331+305'
# At Cband the fractional polarization of this source is 0.112 and
# the R-L PhaseDiff = 66deg (EVPA = 33deg)
polxfpol = 0.112
polxrlpd_deg = 66.0
# Dictionary of IPOL in the spw
polxipol = {'0' : 7.462,
            '1' : 7.510}

# Make Stokes lists for setjy
polxiquv = {}
for spw in ['0','1']:
    ipol = polxipol[spw]
    fpol = polxfpol
    ppol = ipol*fpol
    rlpd = polxrlpd_deg*pi/180.0
    qpol = ppol*cos(rlpd)
    upol = ppol*sin(rlpd)
    polxiquv[spw] = [ipol,qpol,upol,0.0]

#
# Split output setup
#
srcname = 'JUPITER'
srcsplitms = calprefix + '.' + srcname + '.split.ms'
calname = '0137+331'
calsplitms = calprefix + '.' + calname + '.split.ms'

#
#=====================================================================
# Calibration Reset
#=====================================================================
#
# Reset the CORRECTED_DATA column to data
#
print '--Clearcal--'
default('clearcal')

vis = msfile

clearcal()

print "Reset calibration for MS "+vis
print ""
#
#=====================================================================
# Calibration
#=====================================================================
#
# Set the fluxes of the primary calibrator(s)
#
print '--Setjy--'
default('setjy')

print "Use setjy to set flux of 1331+305 (3C286)"

vis = msfile

#
# 1331+305 = 3C286 is our primary calibrator
field = '1331+305'     

# Setjy knows about this source so we dont need anything more

setjy()

#
# You should see something like this in the logger and casapy.log file:
#
# 1331+305  spwid=  0  [I=7.462, Q=0, U=0, V=0] Jy, (Perley-Taylor 99)
# 1331+305  spwid=  1  [I=7.51, Q=0, U=0, V=0] Jy, (Perley-Taylor 99)
# 

print "Look in logger for the fluxes (should be 7.462 and 7.510 Jy)"

#
#=====================================================================
#
# Initial gain calibration
#
print '--Gaincal--'
default('gaincal')

print "Solve for antenna gains on 1331+305 and 0137+331"
print "We have 2 single-channel continuum spw"
print "Do not want bandpass calibration"

vis = msfile

# set the name for the output gain caltable
caltable = gtable

print "Output gain cal table will be "+gtable

# Gain calibrators are 1331+305 and 0137+331 (FIELD_ID 7 and 0)
# We have 2 IFs (SPW 0,1) with one channel each

# selection is via the field and spw strings
field = '1331+305,0137+331'
spw = ''

# a-priori calibration application
gaincurve = usegaincurve
opacity = gainopacity

# scan-based G solutions for both amplitude and phase
gaintype = 'G'
calmode = 'ap'

# one solution per scan
solint = 'inf'
combine = ''

# do not apply parallactic angle correction (yet)
parang = False

# reference antenna
refant = calrefant

# minimum SNR 3
minsnr = 3

gaincal()

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
fluxtable = ftable

print "Output scaled gain cal table is "+ftable

# point to our first gain cal table
caltable = gtable

# we will be using 1331+305 (the source we did setjy on) as
# our flux standard reference
reference = '1331+305'

# we want to transfer the flux to our other gain cal source 0137+331
# to bring its gain amplitues in line with the absolute scale
transfer = '0137+331'

fluxscale()

# You should see in the logger something like:
#Flux density for 0137+331 in SpW=0 is:
#   5.42575 +/- 0.00285011 (SNR = 1903.7, nAnt= 27)
#Flux density for 0137+331 in SpW=1 is:
#   5.46569 +/- 0.00301326 (SNR = 1813.88, nAnt= 27)

#
#---------------------------------------------------------------------
# Plot calibration
#
print '--PlotCal--'
default('plotcal')

showgui = True
    
caltable = ftable
multiplot = True
yaxis = 'amp'

showgui = True
    
plotcal()

print ""
print "-------------------------------------------------"
print "Plotcal"
print "Looking at amplitude in cal-table "+caltable

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

#
# Now go back and plot to file
#
showgui = False

yaxis = 'amp'

figfile = caltable + '.plotcal.amp.png'
print "Plotting calibration to file "+figfile
#saveinputs('plotcal',caltable.plotcal.amp.saved')
plotcal()

yaxis = 'phase'

figfile = caltable + '.plotcal.phase.png'
print "Plotting calibration to file "+figfile
#saveinputs('plotcal',caltable.plotcal.phase.saved')
plotcal()

#
#=====================================================================
# Polarization Calibration
#=====================================================================
#
if (dopolcal):
    print '--Polcal (D)--'
    default('polcal')
    
    print "Solve for polarization leakage on 0137+331"
    print "Pretend it has unknown polarization"

    vis = msfile

    # Start with the un-fluxscaled gain table
    gaintable = gtable

    # use settings from gaincal
    gaincurve = usegaincurve
    opacity = gainopacity
    
    # Output table
    caltable = ptable

    # Use a 3C48 tracked through a range of PA
    field = '0137+331'
    spw = ''

    # No need for further selection
    selectdata=False

    # Polcal mode (D+QU = unknown pol for D)
    poltype = 'D+QU'

    # One solution for entire dataset
    solint = 'inf'
    combine = 'scan'

    # reference antenna
    refant = calrefant

    # minimum SNR 3
    minsnr = 3

    #saveinputs('polcal',calprefix+'.polcal.saved')
    polcal()
    
    #=====================================================================
    #
    # List polcal solutions
    #
    print '--Listcal (PolD)--'

    listfile = caltable + '.list'

    print "Listing calibration to file "+listfile

    listcal()
    
    #=====================================================================
    #
    # Plot polcal solutions
    #
    print '--Plotcal (PolD)--'
    
    iteration = ''
    showgui = False
    
    xaxis = 'real'
    yaxis = 'imag'
    figfile = caltable + '.plotcal.reim.png'
    print "Plotting calibration to file "+figfile
    #saveinputs('plotcal',caltable+'.plotcal.reim.saved')
    plotcal()

    xaxis = 'antenna'
    yaxis = 'amp'
    figfile = caltable + '.plotcal.antamp.png'
    print "Plotting calibration to file "+figfile
    #saveinputs('plotcal',caltable+'.plotcal.antamp.saved')
    plotcal()

    xaxis = 'antenna'
    yaxis = 'phase'
    figfile = caltable + '.plotcal.antphase.png'
    print "Plotting calibration to file "+figfile
    #saveinputs('plotcal',caltable+'.plotcal.antphase.saved')
    plotcal()

    xaxis = 'antenna'
    yaxis = 'snr'
    figfile = caltable + '.plotcal.antsnr.png'
    print "Plotting calibration to file "+figfile
    #saveinputs('plotcal',caltable+'.plotcal.antsnr.saved')
    plotcal()

    #=====================================================================
    # Do Chi (X) pol angle calibration
    #=====================================================================
    # First set the model
    print '--Setjy--'
    default('setjy')
        
    vis = msfile
        
    print "Use setjy to set IQU fluxes of "+polxfield
    field = polxfield
    
    for spw in usespwlist:
        fluxdensity = polxiquv[spw]
        
        #saveinputs('setjy',calprefix+'.setjy.polspw.'+spw+'.saved')
        setjy()
    
    #
    # Polarization (X-term) calibration
    #
    print '--PolCal (X)--'
    default('polcal')
    
    print "Polarization R-L Phase Calibration (linear approx)"
    
    vis = msfile
    
    # Start with the G and D tables
    gaintable = [gtable,ptable]
    
    # use settings from gaincal
    gaincurve = usegaincurve
    opacity = gainopacity
    
    # Output table
    caltable = xtable

    # previously set with setjy
    field = polxfield
    spw = ''
    
    selectdata=False
    
    # Solve for Chi
    poltype = 'X'
    solint = 'inf'
    combine = 'scan'
    
    # reference antenna
    refant = calrefant
    
    # minimum SNR 3
    minsnr = 3
    
    #saveinputs('polcal',calprefix+'.polcal.X.saved')
    polcal()
    
#=====================================================================
# Apply the Calibration
#=====================================================================
#
# Interpolate the gains onto Jupiter (and others)
#
# print '--Accum--'
# default('accum')
# 
# print "This will interpolate the gains onto Jupiter"
# 
# vis = msfile
# 
# tablein = ''
# incrtable = ftable
# calfield = '1331+305, 0137+331'
# 
# # set the name for the output interpolated caltable
# caltable = atable
# 
# print "Output cumulative gain table will be "+atable
# 
# # linear interpolation
# interp = 'linear'
# 
# # make 10s entries
# accumtime = 10.0
# 
# accum()
#
# NOTE: bypassing this during testing
atable = ftable

# #=====================================================================
#
# Correct the data
# (This will put calibrated data into the CORRECTED_DATA column)
#
print '--ApplyCal--'
default('applycal')

print "This will apply the calibration to the DATA"
print "Fills CORRECTED_DATA"

vis = msfile

# Start with the interpolated fluxscale/gain table
gaintable = [atable,ptable,xtable]

# use settings from gaincal
gaincurve = usegaincurve
opacity = gainopacity

# select the fields
field = '1331+305,0137+331,JUPITER'
spw = ''
selectdata = False

# IMPORTANT set parang=True for polarization
parang = True

# do not need to select subset since we did accum
# (note that correct only does 'nearest' interp)
gainfield = ''

applycal()

#
#=====================================================================
#
# Now split the Jupiter target data
#
print '--Split Jupiter--'
default('split')

vis = msfile

# Now we write out the corrected data to a new MS

# Select the Jupiter field
field = srcname
spw = ''

# pick off the CORRECTED_DATA column
datacolumn = 'corrected'

# Make an output vis file
outputvis = srcsplitms

print "Split "+field+" data into new ms "+srcsplitms

split()

# Also split out 0137+331 as a check
field = calname

outputvis = calsplitms

print "Split "+field+" data into new ms "+calsplitms

split()

#=====================================================================
# Force scratch column creation so plotxy will work
#
vis = srcsplitms
clearcal()

vis = calsplitms
clearcal()

#=====================================================================
# Use Plotxy to look at the split calibrated data
#
print '--Plotxy--'
default('plotxy')

vis = srcsplitms
selectdata = True

# Plot only the RR and LL for now
correlation = 'RR LL'

# Plot amplitude vs. uvdist
xaxis = 'uvdist'
datacolumn = 'data'
multicolor = 'both'

iteration = ''
selectplot = True
interactive = True

field = 'JUPITER'
yaxis = 'amp'
# Use the field name as the title
title = field+"  "

plotxy()

print ""
print "-----------------------------------------------------"
print "Plotting JUPITER corrected visibilities"
print "Look for outliers"

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

# Now go back and plot to files
interactive = False

#
# First the target
#
vis = srcsplitms
field = srcname
yaxis = 'amp'
# Use the field name as the title
title = field+"  "
figfile = vis + '.plotxy.amp.png'
print "Plotting to file "+figfile
#saveinputs('plotxy',vis+'.plotxy.amp.saved')

plotxy()

yaxis = 'phase'
# Use the field name as the title
figfile = vis + '.plotxy.phase.png'
print "Plotting to file "+figfile
#saveinputs('plotxy',vis+'.plotxy.phase.saved')

plotxy()

#
# Now the calibrator
#
vis = calsplitms
field = calname
yaxis = 'amp'
# Use the field name as the title
title = field+"  "
figfile = vis + '.plotxy.amp.png'
print "Plotting to file "+figfile
#saveinputs('plotxy',vis+'.plotxy.amp.saved')

plotxy()

yaxis = 'phase'
# Use the field name as the title
figfile = vis + '.plotxy.phase.png'
print "Plotting to file "+figfile
#saveinputs('plotxy',vis+'.plotxy.phase.saved')

plotxy()

#
#=====================================================================
# Done

print 'Calibration completed for '+calprefix
