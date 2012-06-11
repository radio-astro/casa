######################################################################
#                                                                    #
# jupiter6cm_poldemo.py                                              #
#                                                                    #
# Use Case Demo Script for Jupiter 6cm VLA                           #
# Trimmed down from Use Case jupiter6cm_usecase.py                   #
#                                                                    #
# Assumes:                                                           #
#    you have already flagged using jupiter6cm_flagdemo.py           #
#    calibrated using jupiter6cm_caldemo.py                          #
#    imaged/self-calibratied jupiter6cm_imagingdemo.py               #
# Will do polarization imaging                                       #
#                                                                    #
# Updated STM 2008-05-26 (Beta Patch 2.0) new clean task             #
# Updated STM 2008-06-05 (Beta Patch 2.0) complex pol example        #
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
# Special prefix for this polarization imaging demo output
#
polprefix = prefix + '.polimg'

# Clean up old files
os.system('rm -rf '+polprefix+'*')

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
#clnalg = 'clark'
clnalg = 'hogbom'
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

polimname = polprefix + '.clean'
polimage  = polimname+'.image'
polmodel  = polimname+'.model'
polresid  = polimname+'.residual'
polmask   = polimname+'.clean_interactive.mask'

#
# Other files
#
ipolimage = polimage+'.I'
qpolimage = polimage+'.Q'
upolimage = polimage+'.U'

poliimage = polimage+'.poli'
polaimage = polimage+'.pola'

#
#=====================================================================
# Polarization Imaging
#=====================================================================
#
print '--Clean (Polarization)--'
default('clean')

print "Now clean polarized data"

vis = srcsplitms

imagename = polimname

field = '*'
spw = ''
mode = 'mfs'
gain = 0.1

# Polarization
stokes = 'IQUV'

psfmode = clnalg
imagermode = clnmode

imsize = clnimsize
cell = clncell
niter = clniter
threshold = clnthreshold

weighting = 'briggs'
robust = 0.5

interactive = True
npercycle = 100

saveinputs('clean',imagename+'.clean.saved')
clean()

print ""
print "----------------------------------------------------"
print "Clean"
print "Final restored clean image is "+polimage
print "Final clean model is "+polmodel
print "The clean residual image is "+polresid
print "Your final clean mask is "+polmask

#
#=====================================================================
# Image Analysis
#=====================================================================
#
# Polarization statistics
print '--Final Pol Imstat--'
default('imstat')

imagename = polimage

on_statistics = {}
off_statistics = {}

# lower right corner of the image (clnimsize = [288,288])
onbox = ''
# lower right corner of the image (clnimsize = [288,288])
offbox = '216,1,287,72'

for stokes in ['I','Q','U','V']:
    box = onbox
    on_statistics[stokes] = imstat()
    box = offbox
    off_statistics[stokes] = imstat()

#
# Peel off some Q and U planes
#
print '--Immath--'
default('immath')

mode = 'evalexpr'

stokes = 'I'
outfile = ipolimage
expr = '\"'+polimage+'\"'

immath()
print "Created I image "+outfile

stokes = 'Q'
outfile = qpolimage
expr = '\"'+polimage+'\"'

immath()
print "Created Q image "+outfile

stokes = 'U'
outfile = upolimage
expr = '\"'+polimage+'\"'

immath()
print "Created U image "+outfile

#
#---------------------------------------------------------------------
# Now make POLI and POLA images
#
stokes = ''
outfile = poliimage
mode = 'poli'
imagename = [qpolimage,upolimage]
# Use our rms above for debiasing
mysigma = 0.5*( off_statistics['Q']['rms'][0] + off_statistics['U']['rms'][0] )
#sigma = str(mysigma)+'Jy/beam'
# This does not work well yet
sigma = '0.0Jy/beam'

immath()
print "Created POLI image "+outfile

outfile = polaimage
mode = 'pola'

immath()
print "Created POLA image "+outfile

#
#---------------------------------------------------------------------
# Save statistics of these images
default('imstat')

imagename = poliimage
stokes = ''
box = onbox
on_statistics['POLI'] = imstat()
box = offbox
off_statistics['POLI'] = imstat()

#
#
#---------------------------------------------------------------------
# Display clean I image in viewer but with polarization vectors
#
# If you did not do interactive clean, bring up viewer manually
viewer(polimage,'image')

print "Displaying pol I now.  You should overlay pola vectors"
print "Bring up the Load Data panel:"
print ""
print "Use LEL for POLA VECTOR with cut above 6*mysigma in POLI = "+str(6*mysigma)
print "For example:"
print "\'"+polaimage+"\'[\'"+poliimage+"\'>0.0048]"
print ""
print "In the Data Display Options for the vector plot:"
print "  Set the x,y increments to 2 (default is 3)"
print "  Use an extra rotation this 90deg to get B field"
print "Note the lengths are all equal. You can fiddle these."
print ""
print "You can also load the poli image as contours"

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

# NOTE: the LEL will be something like
# 'jupiter6cm.usecase.polimg.clean.image.pola'['jupiter6cm.usecase.polimg.clean.image.poli'>0.005]

#
# NOTE: The viewer can take complex images to make Vector plots, although
# the image analysis tasks (and ia tool) cannot yet handle these.  But we
# can use the imagepol tool (which is not imported by default) to make
# a complex image of the linear polarized intensity for display.
# See CASA User Reference Manual:
# http://casa.nrao.edu/docs/casaref/imagepol-Tool.html
#
# Make an imagepol tool and open the clean image 
potool = casac.homefinder.find_home_by_name('imagepolHome')
po = potool.create()
po.open(polimage)
# Use complexlinpol to make a Q+iU image
complexlinpolimage = polimname + '.cmplxlinpol'
po.complexlinpol(complexlinpolimage)
po.close()

# You can now display this in the viewer, in particular overlay this
# over the intensity raster with the poli contours.  The vector lengths
# will be proportional to the polarized intensity.  You can play with
# the Data Display Options panel for vector spacing and length.
# You will want to have this masked, like the pola image above, on
# the polarized intensity.  When you load the image, use the LEL:
# 'jupiter6cm.usecase.polimg.clean.cmplxlinpol'['jupiter6cm.usecase.polimg.clean.image.poli'>0.005]

#=====================================================================
#
# Print results
#
print ""
print ' Jupiter polarization results '
print ' ============================ '
print ''
for stokes in ['I','Q','U','V','POLI']:
    print ''
    print ' =============== '
    print ''
    print ' Polarization (Stokes '+stokes+'):'
    mymax = on_statistics[stokes]['max'][0]
    mymin = on_statistics[stokes]['min'][0]
    myrms = off_statistics[stokes]['rms'][0]
    absmax = max(mymax,mymin)
    mydra = absmax/myrms
    print '   Clean image  ON-SRC max = ',mymax
    print '   Clean image  ON-SRC min = ',mymin
    print '   Clean image OFF-SRC rms = ',myrms
    print '   Clean image dynamic rng = ',mydra


print '--- Done ---'

#
#=====================================================================
