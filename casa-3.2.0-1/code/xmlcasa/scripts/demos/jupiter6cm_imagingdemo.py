######################################################################
#                                                                    #
# Use Case Demo Script for Jupiter 6cm VLA                           #
# Trimmed down from Use Case jupiter6cm_usecase.py                   #
#                                                                    #
# Assumes you have already flagged using jupiter6cm_flagdemo.py      #
# and calibratied using jupiter6cm_caldemo.py                        #
# Will do imaging                                                    #
#                                                                    #
# Updated STM 2008-05-26 (Beta Patch 2.0) use new clean task         #
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

prefix='jupiter6cm.demo'

msfile = prefix + '.ms'

#
# Special prefix for calibration demo output
#
calprefix = prefix + '.cal'

srcname = 'JUPITER'
srcsplitms = calprefix + '.' + srcname + '.split.ms'

#
#=====================================================================
#
# Special prefix for this imaging demo output
#
imprefix = prefix + '.img'

# Clean up old files
os.system('rm -rf '+imprefix+'*')

#
#=====================================================================
#
# Imaging parameters
#
# This is D-config VLA 6cm (4.85GHz) obs
# Check the observational status summary
# Primary beam FWHM = 45'/f_GHz = 557"
# Synthesized beam FWHM = 14"
# RMS in 10min (600s) = 0.06 mJy (thats now, but close enough)

# Set the output image size and cell size (arcsec)
# 4" will give 3.5x oversampling
clncell = [4.,4.]

# 280 pix will cover to 2xPrimaryBeam
# clean will say to use 288 (a composite integer) for efficiency
clnalg = 'clark'
clnmode = ''
# For Cotton-Schwab use
clnmode = 'csclean'
clnimsize = [288,288]

# iterations
clniter = 10000

# Also set flux residual threshold (0.04 mJy)
# From our listobs:
# Total integration time = 85133.2 seconds
# With rms of 0.06 mJy in 600s ==> rms = 0.005 mJy
# Set to 10x thermal rms
clnthreshold=0.05

#
# Filenames
#
imname1 = imprefix + '.clean1'
clnimage1 = imname1+'.image'
clnmodel1 = imname1+'.model'
clnresid1 = imname1+'.residual'
clnmask1  = imname1+'.clean_interactive.mask'

imname2 = imprefix + '.clean2'
clnimage2 = imname2+'.image'
clnmodel2 = imname2+'.model'
clnresid2 = imname2+'.residual'
clnmask2  = imname2+'.clean_interactive.mask'

imname3 = imprefix + '.clean3'
clnimage3 = imname3+'.image'
clnmodel3 = imname3+'.model'
clnresid3 = imname3+'.residual'
clnmask3  = imname3+'.clean_interactive.mask'

#
# Selfcal parameters
#
# reference antenna 11 (11=VLA:N1)
calrefant = '11'

#
# Filenames
#
selfcaltab1 = imprefix + '.selfcal1.gtable'

selfcaltab2 = imprefix + '.selfcal2.gtable'
smoothcaltab2 = imprefix + '.smoothcal2.gtable'

#
#=====================================================================
# Calibration Reset
#=====================================================================
#
# Reset the CORRECTED_DATA column to data
#
print '--Clearcal--'
default('clearcal')

vis = srcsplitms

clearcal()

print "Reset calibration for MS "+vis
print ""
#
#=====================================================================
# FIRST CLEAN / SELFCAL CYCLE
#=====================================================================
#
# Now clean an image of Jupiter
# NOTE: this uses the new combined invert/clean/mosaic task Patch 2
#
print '--Clean 1--'
default('clean')

# Pick up our split source data
vis = srcsplitms

# Make an image root file name
imagename = imname1

print "Output images will be prefixed with "+imname1

# Set up the output continuum image (single plane mfs)
mode = 'mfs'
stokes = 'I'

print "Will be a single MFS continuum image"

# NOTE: current version field='' doesnt work
field = '*'

# Combine all spw
spw = ''

# Imaging mode params
psfmode = clnalg
imagermode = clnmode

# Imsize and cell
imsize = clnimsize
cell = clncell

# NOTE: will eventually have an imadvise task to give you this
# information

# Standard gain factor 0.1
gain = 0.1

# Fix maximum number of iterations and threshold
niter = clniter
threshold = clnthreshold

# Note - we can change niter and threshold interactively
# during clean

# Set up the weighting
# Use Briggs weighting (a moderate value, on the uniform side)
weighting = 'briggs'
robust = 0.5

# No clean mask or box
mask = ''

# Use interactive clean mode
interactive = True

# Moderate number of iter per interactive cycle
npercycle = 100

saveinputs('clean',imagename+'.clean.saved')
clean()

# When the interactive clean window comes up, use the right-mouse
# to draw rectangles around obvious emission double-right-clicking
# inside them to add to the flag region.  You can also assign the
# right-mouse to polygon region drawing by right-clicking on the
# polygon drawing icon in the toolbar.  When you are happy with
# the region, click 'Done Flagging' and it will go and clean another
# 100 iterations.  When done, click 'Stop'.

print ""
print "----------------------------------------------------"
print "Clean"
print "Final clean model is "+clnmodel1
print "Final restored clean image is "+clnimage1
print "The clean residual image is "+clnresid1
print "Your final clean mask is "+clnmask1

print ""
print "This is the final restored clean image in the viewer"
print "Zoom in and set levels to see faint emission"
print "Use rectangle drawing tool to box off source"
print "Double-click inside to print statistics"
print "Move box on-source and get the max"
print "Calcualte DynRange = MAXon/RMSoff"
print "I got 1.060/0.004 = 270"
print "Still not as good as it can be - lets selfcal"
print "Close viewer panel when done"

#
#---------------------------------------------------------------------
#
# If you did not do interactive clean, bring up viewer manually
viewer(clnimage1,'image')

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

# You can use the right-mouse to draw a box in the lower right
# corner of the image away from emission, the double-click inside
# to bring up statistics.  Use the right-mouse to grab this box
# and move it up over Jupiter and double-click again.  You should
# see stuff like this in the terminal:
#
# jupiter6cm.usecase.clean1.image     (Jy/beam)
# 
# n           Std Dev     RMS         Mean        Variance    Sum
# 4712        0.003914    0.003927    0.0003205   1.532e-05   1.510     
# 
# Flux        Med |Dev|   IntQtlRng   Median      Min         Max
# 0.09417     0.002646    0.005294    0.0001885   -0.01125    0.01503   
#
#
# On Jupiter:
#
# n           Std Dev     RMS         Mean        Variance    Sum
# 3640        0.1007      0.1027      0.02023     0.01015     73.63     
# 
# Flux        Med |Dev|   IntQtlRng   Median      Min         Max
# 4.592       0.003239    0.007120    0.0001329   -0.01396    1.060     
#
# Estimated dynamic range = 1.060 / 0.003927 = 270 (poor)
#
# Note that the exact numbers you get will depend on how deep you
# take the interactive clean and how you draw the box for the stats.

#=====================================================================
#
# Do some non-interactive image statistics
print '--Imstat--'
default('imstat')

imagename = clnimage1
on_statistics1 = imstat()

# Now do stats in the lower right corner of the image
# remember clnimsize = [288,288]
box = '216,1,287,72'
off_statistics1 = imstat()

# Pull the max and rms from the clean image
thistest_immax=on_statistics1['max'][0]
print ' Found : Max in image = ',thistest_immax
thistest_imrms=off_statistics1['rms'][0]
print ' Found : rms in image = ',thistest_imrms
print ' Clean image Dynamic Range = ',thistest_immax/thistest_imrms
print ''
#
#---------------------------------------------------------------------
#
# Self-cal using clean model
#
# Note: clean will have left FT of model in the MODEL_DATA column
# If you've done something in between, can use the ft task to
# do this manually.
#
print '--SelfCal 1--'
default('gaincal')

vis = srcsplitms

print "Will self-cal using MODEL_DATA left in MS by clean"

# New gain table
caltable = selfcaltab1

print "Will write gain table "+selfcaltab1

# Don't need a-priori cals
selectdata = False
gaincurve = False
opacity = 0.0

# This choice seemed to work
refant = calrefant

# Do amp and phase
gaintype = 'G'
calmode = 'ap'

# Do 30s solutions with SNR>1
solint = 30.0
minsnr = 1.0
print "Calibrating amplitudes and phases on 30s timescale"

# Do not need to normalize (let gains float)
solnorm = False

gaincal()

#
#---------------------------------------------------------------------
# It is useful to put this up in plotcal
#
#
print '--PlotCal--'
default('plotcal')

caltable = selfcaltab1
multiplot = True
yaxis = 'amp'

plotcal()

print ""
print "-------------------------------------------------"
print "Plotcal"
print "Looking at amplitude in self-cal table "+caltable

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

yaxis = 'phase'

plotcal()

print ""
print "-------------------------------------------------"
print "Plotcal"
print "Looking at phases in self-cal table "+caltable

#
# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

#
#---------------------------------------------------------------------
#
# Correct the data (no need for interpolation this stage)
#
print '--ApplyCal--'
default('applycal')

vis = srcsplitms

print "Will apply self-cal table to over-write CORRECTED_DATA in MS"

gaintable = selfcaltab1

gaincurve = False
opacity = 0.0
field = ''
spw = ''
selectdata = False

calwt = True

applycal()

# Self-cal is now in CORRECTED_DATA column of split ms
#=====================================================================
# Use Plotxy to look at the self-calibrated data
#
print '--Plotxy--'
default('plotxy')

vis = srcsplitms
selectdata = True
field = 'JUPITER'

correlation = 'RR LL'
xaxis = 'uvdist'
yaxis = 'amp'
datacolumn = 'corrected'
multicolor = 'both'

# Use the field name as the title
selectplot = True
title = field+"  "

iteration = ''

plotxy()

print ""
print "-----------------------------------------------------"
print "Plotting JUPITER self-corrected visibilities"
print "Look for outliers, and you can flag them"

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

#
#=====================================================================
# SECOND CLEAN / SELFCAL CYCLE
#=====================================================================
#
print '--Clean 2--'
default('clean')

print "Now clean on self-calibrated data"

vis = srcsplitms

imagename = imname2

field = '*'
spw = ''
mode = 'mfs'
gain = 0.1

# Imaging mode params
psfmode = clnalg
imagermode = clnmode
imsize = clnimsize
cell = clncell
niter = clniter
threshold = clnthreshold

weighting = 'briggs'
robust = 0.5

mask = ''
interactive = True
npercycle = 100

saveinputs('clean',imagename+'.clean.saved')
clean()

print ""
print "----------------------------------------------------"
print "Clean"
print "Final clean model is "+clnmodel2
print "Final restored clean image is "+clnimage2
print "The clean residual image is "+clnresid2
print "Your final clean mask is "+clnmask2

print ""
print "This is the final restored clean image in the viewer"
print "Zoom in and set levels to see faint emission"
print "Use rectangle drawing tool to box off source"
print "Double-click inside to print statistics"
print "Move box on-source and get the max"
print "Calcualte DynRange = MAXon/RMSoff"
print "This time I got 1.076 / 0.001389 = 775 (better)"
print "Still not as good as it can be - lets selfcal again"
print "Close viewer panel when done"

#
#---------------------------------------------------------------------
#
# If you did not do interactive clean, bring up viewer manually
viewer(clnimage2,'image')

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

# jupiter6cm.usecase.clean2.image     (Jy/beam)
# 
# n           Std Dev     RMS         Mean        Variance    Sum
# 5236        0.001389    0.001390    3.244e-05   1.930e-06   0.1699    
# 
# Flux        Med |Dev|   IntQtlRng   Median      Min         Max
# 0.01060     0.0009064   0.001823    -1.884e-05  -0.004015   0.004892  
# 
# 
# On Jupiter:
# 
# n           Std Dev     RMS         Mean        Variance    Sum
# 5304        0.08512     0.08629     0.01418     0.007245    75.21     
# 
# Flux        Med |Dev|   IntQtlRng   Median      Min         Max
# 4.695       0.0008142   0.001657    0.0001557   -0.004526   1.076     
#
# Estimated dynamic range = 1.076 / 0.001389 = 775 (better)
#
# Note that the exact numbers you get will depend on how deep you
# take the interactive clean and how you draw the box for the stats.
#
print ""
print "--------------------------------------------------"
print "After this script is done you can continue on with"
print "more self-cal, or try different cleaning options"

#
#=====================================================================
# Image Analysis
#=====================================================================
#
# Can do some image statistics if you wish
print '--Imstat (Cycle 2)--'
default('imstat')

imagename = clnimage2
on_statistics2 = imstat()

# Now do stats in the lower right corner of the image
# remember clnimsize = [288,288]
box = '216,1,287,72'
off_statistics2 = imstat()

# Pull the max and rms from the clean image
thistest_immax=on_statistics2['max'][0]
print ' Found : Max in image = ',thistest_immax
thistest_imrms=off_statistics2['rms'][0]
print ' Found : rms in image = ',thistest_imrms
print ' Clean image Dynamic Range = ',thistest_immax/thistest_imrms
print ''

#=====================================================================
#
# Print results and regression versus previous runs
#
print ""
print ' Final Jupiter results '
print ' ===================== '
print ''
# Pull the max and rms from the clean image
thistest_immax=on_statistics2['max'][0]
oldtest_immax = 1.07732224464
print '   Clean image  ON-SRC max = ',thistest_immax
print '   Previously found to be  = ',oldtest_immax
diff_immax = abs((oldtest_immax-thistest_immax)/oldtest_immax)
print '   Difference (fractional) = ',diff_immax

print ''
thistest_imrms=off_statistics2['rms'][0]
oldtest_imrms = 0.0010449
print '   Clean image OFF-SRC rms = ',thistest_imrms
print '   Previously found to be  = ',oldtest_imrms
diff_imrms = abs((oldtest_imrms-thistest_imrms)/oldtest_imrms)
print '   Difference (fractional) = ',diff_imrms

print ''
print ' Final Clean image Dynamic Range = ',thistest_immax/thistest_imrms
print ''
print '--- Done ---'

#
#=====================================================================
