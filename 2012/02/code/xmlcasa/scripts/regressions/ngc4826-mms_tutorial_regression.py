##########################################################################
#                                                                        #
# Demo Script for NGC 4826 (BIMA line data)                              #
# modified to test the usage of multi-MSs (MMSs)
#                                                                        #
# N4826 - BIMA SONG Data                                                 #
#                                                                        #
# This data is from the BIMA Survey of Nearby Galaxies (BIMA SONG)       #
# Helfer, Thornley, Regan, et al., 2003, ApJS, 145, 259                  #
# Many thanks to Michele Thornley for providing the data and description #
#                                                                        #
# Script Notes:                                                          #
#    o The results are written out as a dictionary in a pickle file      #
#         out.ngc4826-mms.tutorial.regression.<datestring>.pickle        #
#      as well as in a text file                                         #
#         out.ngc4826.tutorial.<datestring>.log                          #
#      (these are not auto-deleted at start of script)                   #
#    o This script keeps internal regression values, but you can provide #
#      a file ngc4826_tutorial_regression.pickle from a previous run     #
#                                                                        #
##########################################################################
import time
import os
import pickle

# Crate multi-MS at which step?
usemms_step    = -1 # not at all
#usemms_step   = 0 # at concat
#usemms_step   = 1 # before flagging
#usemms_step   = 2 # before setjy
#usemms_step   = 3 # before gaincal
#usemms_step   = 4 # before fluxscale
#usemms_step   = 5 # before applycal
#usemms_step   = 6 # before split
#usemms_step   = 7 # before clean


# Use field-wise or spw-wise splitting in multi-MS?
usefieldwise = True


def makefieldwisemms(msname):    
    casalog.post('******* Making field-wise MMS. *******', "INFO", 'makefieldwisemms')
    print '******* Making field-wise MMS. *******'
    os.system('rm -rf temp.ms; mv '+msname+' temp.ms')
    members = []
    tb.open('temp.ms/FIELD')
    nfield = tb.nrows()
    tb.close()
    for i in range(0,nfield):
        member = 'ngc4826.tutorial-field'+str(i)+'.ms'
        os.system('rm -rf '+member)
        print member
        split(vis='temp.ms', outputvis=member, field=i, datacolumn='all')
        members.append(member)
    concat(vis=members, concatvis=msname, createmms=True)

def makespwwisemms(msname):
    casalog.post('******* Making spw-wise MMS. *******', "INFO", 'makespwwisemms')
    print '******* Making spw-wise MMS. *******'
    os.system('rm -rf temp.ms; mv '+msname+' temp.ms')
    members = []
    tb.open('temp.ms/SPECTRAL_WINDOW')
    nspw = tb.nrows()
    tb.close()
    for i in range(0,nspw):
        member = 'ngc4826.tutorial-spw'+str(i)+'.ms'
        os.system('rm -rf '+member)
        print member
        split(vis='temp.ms', outputvis=member, spw=str(i), datacolumn='all')
        members.append(member)
    concat(vis=members, concatvis=msname, createmms=True)

# Sets a shorthand for fixed input script/regression files
scriptprefix='ngc4826-mms_tutorial_regression'

#
##########################################################################
#                                                                        
# Clear out previous run results
os.system('rm -rf ngc4826.tutorial.*')

# Sets a shorthand for the ms, not necessary
prefix='ngc4826-mms.tutorial'
msfile = prefix + '.16apr98.ms'

print 'Tutorial Regression Script for BIMASONG NGC4826 Mosaic'
print 'Version for Release 0 (3.0.0) 7-Dec-2009'
print 'Will do: import, flagging, calibration, imaging'
print ''
#
##########################################################################
#
# 
##########################################################################
#
# N4826 - BIMA SONG Data
# 16apr98
#	source=ngc4826
#	phasecal=1310+323
#	fluxcal=3c273, Flux = 23 Jy on 16apr98
#	passcal= none - data were observed with online bandpass correction.
#
# NOTE: This data has been filled into MIRIAD, line-length correction 
#	done, and then exported as separate files for each source.
#	3c273 was not line length corrected since it was observed
#	for such a short amount of time that it did not need it.  
#
# From miriad: source Vlsr = 408; delta V is 20 km/s 
#
#
##########################################################################
#

##########################################################################
# Import and concatenate sources
##########################################################################
#
# USB spectral windows written separately by miriad for 16apr98
# Assumes these are in sub-directory called "fitsfiles" of working directory
print '--Importuvfits (16apr98)--'
default('importuvfits')

print "Starting from the uvfits files exported by miriad"
print "The USB spectral windows were written separately by miriad for 16apr98"

importuvfits(fitsfile='fitsfiles/3c273.fits5', vis='ngc4826.tutorial.3c273.5.ms')

importuvfits(fitsfile='fitsfiles/3c273.fits6', vis='ngc4826.tutorial.3c273.6.ms')

importuvfits(fitsfile='fitsfiles/3c273.fits7', vis='ngc4826.tutorial.3c273.7.ms')

importuvfits(fitsfile='fitsfiles/3c273.fits8', vis='ngc4826.tutorial.3c273.8.ms')

importuvfits(fitsfile='fitsfiles/1310+323.ll.fits9', vis='ngc4826.tutorial.1310+323.ll.9.ms')

importuvfits(fitsfile='fitsfiles/1310+323.ll.fits10', vis='ngc4826.tutorial.1310+323.ll.10.ms')

importuvfits(fitsfile='fitsfiles/1310+323.ll.fits11', vis='ngc4826.tutorial.1310+323.ll.11.ms')

importuvfits(fitsfile='fitsfiles/1310+323.ll.fits12', vis='ngc4826.tutorial.1310+323.ll.12.ms')

importuvfits(fitsfile='fitsfiles/1310+323.ll.fits13', vis='ngc4826.tutorial.1310+323.ll.13.ms')

importuvfits(fitsfile='fitsfiles/1310+323.ll.fits14', vis='ngc4826.tutorial.1310+323.ll.14.ms')

importuvfits(fitsfile='fitsfiles/1310+323.ll.fits15', vis='ngc4826.tutorial.1310+323.ll.15.ms')

importuvfits(fitsfile='fitsfiles/1310+323.ll.fits16', vis='ngc4826.tutorial.1310+323.ll.16.ms')

importuvfits(fitsfile='fitsfiles/ngc4826.ll.fits5', vis='ngc4826.tutorial.ngc4826.ll.5.ms')

importuvfits(fitsfile='fitsfiles/ngc4826.ll.fits6', vis='ngc4826.tutorial.ngc4826.ll.6.ms')

importuvfits(fitsfile='fitsfiles/ngc4826.ll.fits7', vis='ngc4826.tutorial.ngc4826.ll.7.ms')

importuvfits(fitsfile='fitsfiles/ngc4826.ll.fits8', vis='ngc4826.tutorial.ngc4826.ll.8.ms')


#
##########################################################################
#
print '--Concat--'
default('concat')

concat(vis=['ngc4826.tutorial.3c273.5.ms',
	    'ngc4826.tutorial.3c273.6.ms',
	    'ngc4826.tutorial.3c273.7.ms',
	    'ngc4826.tutorial.3c273.8.ms',
	    'ngc4826.tutorial.1310+323.ll.9.ms',
	    'ngc4826.tutorial.1310+323.ll.10.ms',
	    'ngc4826.tutorial.1310+323.ll.11.ms',
	    'ngc4826.tutorial.1310+323.ll.12.ms',
	    'ngc4826.tutorial.1310+323.ll.13.ms',
	    'ngc4826.tutorial.1310+323.ll.14.ms',
	    'ngc4826.tutorial.1310+323.ll.15.ms',
	    'ngc4826.tutorial.1310+323.ll.16.ms',
	    'ngc4826.tutorial.ngc4826.ll.5.ms',
	    'ngc4826.tutorial.ngc4826.ll.6.ms',
	    'ngc4826.tutorial.ngc4826.ll.7.ms',
	    'ngc4826.tutorial.ngc4826.ll.8.ms'],
       concatvis='ngc4826.tutorial.ms',
       freqtol="",dirtol="1arcsec",async=False, createmms=(usemms_step==0))


#
##########################################################################
# 16 APR Calibration
##########################################################################
print '--Clearcal--'
print 'Create scratch columns and initialize in '+'ngc4826.tutorial.ms'

# Force create/initialize of scratch columns
# NOTE: plotxy will not run properly without this step.
#
clearcal(vis='ngc4826.tutorial.ms')


#
##########################################################################
#
# List contents of MS
#
print '--Listobs--'
listobs(vis='ngc4826.tutorial.ms')


if (usemms_step==1):
    if usefieldwise:
        makefieldwisemms('ngc4826.tutorial.ms')
    else:
        makespwwisemms('ngc4826.tutorial.ms')

#
##########################################################################
#
# Flag end channels
#
print '--Flagdata--'
default('flagdata')

print ""
print "Flagging edge channels in all spw"
print "  0~3:0~1;62~63 , 4~11:0~1;30~31, 12~15:0~1;62~63 "
print ""

flagdata(vis='ngc4826.tutorial.ms', mode='manualflag',
         spw='0~3:0;1;62;63,4~11:0;1;30;31,12~15:0;1;62;63')

#
# Flag correlator glitch
#
print ""
print "Flagging bad correlator field 8 antenna 3&9 spw 15 all channels"
print "  timerange 1998/04/16/06:19:00.0~1998/04/16/06:20:00.0"
print ""

flagdata(vis='ngc4826.tutorial.ms', mode='manualflag', field='8', spw='15', antenna='3&9', 
         timerange='1998/04/16/06:19:00.0~1998/04/16/06:20:00.0')

#
# Flag non-fringing antenna 6
#
# NOTE: this is already flagged in the data so do nothing more here
#flagdata(vis='ngc4826.tutorial.ms', mode='manualflag', antenna='6',
#	 timerange='1998/04/16/09:42:39.0~1998/04/16/10:24:46.0')

#
#
#
##########################################################################
#
# Use Flagmanager to save a copy of the flags so far
#
print '--Flagmanager--'
default('flagmanager')

print "Now will use flagmanager to save a copy of the flags we just made"
print "These are named myflags"

flagmanager(vis='ngc4826.tutorial.ms',mode='save',versionname='myflags',
            comment='My flags',merge='replace')

# Can also use Flagmanager to list all saved versions
#
flagmanager(vis='ngc4826.tutorial.ms',mode='list')


print "Completed pre-calibration flagging"

if (usemms_step==2):
    if usefieldwise:
        makefieldwisemms('ngc4826.tutorial.ms')
    else:
        makespwwisemms('ngc4826.tutorial.ms')


#
##########################################################################
#
# CALIBRATION
#
##########################################################################
#
# Bandpasses are very flat because of observing mode used (online bandpass
# correction) so bandpass calibration is unnecessary for these data.
#
##########################################################################
#
# Derive gain calibration solutions.
# We will use VLA-like G (per-scan) calibration:
#
##########################################################################
#
# Set the flux density of 3C273 to 23 Jy
#
print '--Setjy (3C273)--'
default('setjy')

setjy(vis='ngc4826.tutorial.ms',field='0',fluxdensity=[23.0,0.,0.,0.],spw='0~3')
#
# Not really necessary to set spw but you get lots of warning messages if
# you don't


if (usemms_step==3):
    if usefieldwise:
        makefieldwisemms('ngc4826.tutorial.ms')
    else:
        makespwwisemms('ngc4826.tutorial.ms')


#
##########################################################################
#
# Gain calibration
#
print '--Gaincal--'
default('gaincal')

# This should be combining all spw for the two calibrators for single
# scan-based solutions

print 'Gain calibration for fields 0,1 and spw 0~11'
print 'Using solint=inf combining over spw'
print 'Output table ngc4826.tutorial.16apr98.gcal'

gaincal(vis='ngc4826.tutorial.ms', caltable='ngc4826.tutorial.16apr98.gcal',
	field='0,1', spw='0~11', gaintype='G', minsnr=2.0,
	refant='ANT5', gaincurve=False, opacity=0.0,
	solint='inf', combine='spw')

if (usemms_step==4):
    if usefieldwise:
        makefieldwisemms('ngc4826.tutorial.ms')
    else:
        makespwwisemms('ngc4826.tutorial.ms')


#
##########################################################################
#
# Transfer the flux density scale:
#
print '--Fluxscale--'
default('fluxscale')

print ''
print 'Transferring flux of 3C273 to sources: 1310+323'
print 'Output table ngc4826.tutorial.16apr98.fcal'

fluxscale(vis='ngc4826.tutorial.ms', caltable='ngc4826.tutorial.16apr98.gcal',
	  fluxtable='ngc4826.tutorial.16apr98.fcal',
	  reference='3C273', transfer=['1310+323'])

# Flux density for 1310+323 is: 1.48 +/- 0.016 (SNR = 90.6, nAnt= 8)

if (usemms_step==5):
    if usefieldwise:
        makefieldwisemms('ngc4826.tutorial.ms')
    else:
        makespwwisemms('ngc4826.tutorial.ms')


#
##########################################################################
#
# Correct the calibrater/target source data:
# Use new parm spwmap to apply gain solutions derived from spwid1
# to all other spwids... 
print '--Applycal--'
default('applycal')

print 'Applying calibration table ngc4826.tutorial.16apr98.fcal to data'

applycal(vis='ngc4826.tutorial.ms',
	 field='', spw='',
	 gaincurve=False, opacity=0.0, 
         gaintable='ngc4826.tutorial.16apr98.fcal',
	 spwmap=[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])


if (usemms_step==6):
    if usefieldwise:
        makefieldwisemms('ngc4826.tutorial.ms')
    else:
        makespwwisemms('ngc4826.tutorial.ms')


#
# Split out calibrated target source and calibrater data:
#
print '--Split--'
default('split')

print 'Splitting 3C273 data to ngc4826.tutorial.16apr98.3C273.split.ms'

split(vis='ngc4826.tutorial.ms',
      outputvis='ngc4826.tutorial.16apr98.3C273.split.ms',
      field='0',spw='0~3:0~63', datacolumn='corrected')

print 'Splitting 1310+323 data to ngc4826.tutorial.16apr98.1310+323.split.ms'

split(vis='ngc4826.tutorial.ms',
      outputvis='ngc4826.tutorial.16apr98.1310+323.split.ms',
      field='1', spw='4~11:0~31', datacolumn='corrected')

print 'Splitting NGC4826 data to ngc4826.tutorial.16apr98.src.split.ms'

split(vis='ngc4826.tutorial.ms',
      outputvis='ngc4826.tutorial.16apr98.src.split.ms',
      field='2~8', spw='12~15:0~63',
      datacolumn='corrected')


if (usemms_step==7):
    if usefieldwise:
        makefieldwisemms('ngc4826.tutorial.16apr98.3C273.split.ms')
        makefieldwisemms('ngc4826.tutorial.16apr98.1310+323.split.ms')
        makefieldwisemms('ngc4826.tutorial.16apr98.src.split.ms')
    else:
        makespwwisemms('ngc4826.tutorial.16apr98.3C273.split.ms')
        makespwwisemms('ngc4826.tutorial.16apr98.1310+323.split.ms')
        makespwwisemms('ngc4826.tutorial.16apr98.src.split.ms')


#
#
##########################################################################
#
# IMAGING OF NGC4826 MOSAIC
#
##########################################################################
#
#          Mosaic field spacing looks like:
#
#          F3 (field 3)         F2 (field 2)
#
#   F4 (field 4)      F0 (field 0)        F1 (field 1)
#
#          F5 (field 5)         F6 (field 6)
#
# 4x64 channels = 256 channels 
#
# Primary Beam should be about 1.6' FWHM (7m dishes, 2.7mm wavelength)
# Resolution should be about 5-8"
##########################################################################
#
# Image the target source mosaic:
#
print '--Clean (NGC4826)--'
default('clean')

# Make image big enough so mosaic is in inner quarter (400x400)
clnsize = 400
print " Creating CLEAN image of size "+str(clnsize)

clean(vis='ngc4826.tutorial.16apr98.src.split.ms',
      imagename='ngc4826.tutorial.16apr98.src.clean',
      field='0~6',spw='0~3',
      cell=[1.,1.],imsize=[clnsize,clnsize],
      stokes='I',
      mode='channel',nchan=36,start=34,width=4,
      interpolation='nearest',
      psfmode='clark',imagermode='mosaic',ftmachine='mosaic',
      scaletype='SAULT',
### As we moved to clean by default in flat sigma rather than
### flat snr it converges less well
 ###     cyclefactor=1.5,niter=10000,threshold='45mJy',
      cyclefactor=4,niter=10000,threshold='45mJy',
      minpb=0.3,pbcor=False)

### NOTE: mosaic data ...Sault weighting implies a noise unform image

### Using ftmachine='mosaic', can also use ftmachine='ft' for more
### traditional image plane mosaicing

### NOTE: that niter is set to large number so that stopping point is
### controlled by threshold.

### NOTE: with pbcor=False, the final image is not "flux correct",
### instead the image has constant noise despite roll off in power as
### you move out from the phase center(s). Though this format makes it
### "look nicest", for all flux density measurements, and to get an
### accurate integrated intensity image, one needs to divide the
### srcimage.image/srcimage.flux in order to correct for the mosaic
### response pattern. One could also achieve this by setting pbcor=True
### in clean.

# Try running clean adding the parameter interactive=True.
# This parameter will periodically bring up the viewer to allow
# interactive clean boxing. For poor uv-coverage, deep negative bowls
# from missing short spacings, this can be very important to get correct
# integrated flux densities.

#
##########################################################################
#
# Statistics on clean image cube
#
print '--ImStat (Clean cube)--'

srcstat = imstat('ngc4826.tutorial.16apr98.src.clean.image')

print "Found image max = "+str(srcstat['max'][0])

# 256x256: refpix = '128,128,128,128'
# 400x400: refpix = '200,200,200,200'
# 512x512: refpix = '256,256,256,256'
# 800x800: refpix = '400,400,400,400'
refpix = int(clnsize/2)
refbox = str(refpix)+','+str(refpix)+','+str(refpix)+','+str(refpix)
print "  Using Reference Pixel "+refbox

# 256x256: offbox = '106,161,153,200'
# 400x400: offbox = '178,233,225,272'
# 512x512: offbox = '234,289,281,328'
# 800x800: offbox = '378,433,425,472'
blcx = refpix - 22
blcy = refpix + 33
trcx = refpix + 25
trcy = refpix + 72
offbox = str(blcx)+','+str(blcy)+','+str(trcx)+','+str(trcy)
print "  Using Off-Source Box "+offbox

offstat = imstat('ngc4826.tutorial.16apr98.src.clean.image',
                 box=offbox)

print "Found off-source image rms = "+str(offstat['sigma'][0])

# 256x256: cenbox = '108,108,148,148'
# 400x400: cenbox = '180,180,320,320'
# 512x512: cenbox = '236,236,276,276'
# 800x800: cenbox = '380,380,420,420'
blcx = refpix - 20
blcy = refpix - 20
trcx = refpix + 20
trcy = refpix + 20
cenbox = str(blcx)+','+str(blcy)+','+str(trcx)+','+str(trcy)
print "  Using On-Source Box "+cenbox

# offlinechan = '0,1,2,3,4,5,30,31,32,33,34,35'

offlinestat = imstat('ngc4826.tutorial.16apr98.src.clean.image',
                     box=cenbox,
                     chans='0,1,2,3,4,5,30,31,32,33,34,35')

print "Found off-line image rms = "+str(offlinestat['sigma'][0])

#
##########################################################################
#
# Statistics on clean model
#
print '--ImStat (Clean model)--'

modstat = imstat('ngc4826.tutorial.16apr98.src.clean.model')

print "Found total model flux = "+str(modstat['sum'][0])

#
##########################################################################
#
# Manually correct for mosaic response pattern using .image/.flux images
#
print '--ImMath (PBcor)--'

immath(outfile='ngc4826.tutorial.16apr98.src.clean.pbcor',
       mode='evalexpr',
       expr="'ngc4826.tutorial.16apr98.src.clean.image'/'ngc4826.tutorial.16apr98.src.clean.flux'")

# now pbcor the model, be careful to mask zeros

immath(outfile='ngc4826.tutorial.16apr98.src.clean.pbcormod',
       mode='evalexpr',
       expr="'ngc4826.tutorial.16apr98.src.clean.model'['ngc4826.tutorial.16apr98.src.clean.model'!=0.0]/'ngc4826.tutorial.16apr98.src.clean.flux'")


#
##########################################################################
#
# Statistics on PBcor image cube
#
print '--ImStat (PBcor cube)--'

pbcorstat = imstat('ngc4826.tutorial.16apr98.src.clean.pbcor')

print "Found image max = "+str(pbcorstat['max'][0])

pbcoroffstat = imstat('ngc4826.tutorial.16apr98.src.clean.pbcor',
                      box=offbox)

print "Found off-source image rms = "+str(pbcoroffstat['sigma'][0])

pbcorofflinestat = imstat('ngc4826.tutorial.16apr98.src.clean.pbcor',
                          box=cenbox,
                          chans='0,1,2,3,4,5,30,31,32,33,34,35')

print "Found off-line image rms = "+str(pbcorofflinestat['sigma'][0])

#
# Statistics on PBcor image cube
#
print '--ImStat (PSF)--'

psfstat = imstat('ngc4826.tutorial.16apr98.src.clean.psf',
                 box=refbox,chans='27')

print "Found PSF value at refpix = "+str(psfstat['mean'][0])+" (should be 1.0)"


#
# Statistics on PBcor model cube
#
print '--ImStat (PBcor model)--'

pbcormodstat = imstat('ngc4826.tutorial.16apr98.src.clean.pbcormod')

print "Found total model flux = "+str(pbcormodstat['sum'][0])

#
##########################################################################
#
# Do zeroth and first moments
#
# NGC4826 LSR velocity is 408 km/s; delta is 20 km/s

# NOTE: before 02-Jul-2008 (5631) the planes were 1-based, are now 0-based
# was planes 7~28, now 6~27

print '--ImMoments--'
default('immoments')

momfile = 'ngc4826.tutorial.16apr98.moments'
momzeroimage = 'ngc4826.tutorial.16apr98.moments.integrated'
momoneimage = 'ngc4826.tutorial.16apr98.moments.mom1'

print "Calculating Moments 0,1 for PBcor image"

# In the following we will let immoments figure out which axis
# to collapse along, the spectral axis=3

immoments(imagename='ngc4826.tutorial.16apr98.src.clean.pbcor',
	  moments=[0],
	  chans='6~27',
          outfile='ngc4826.tutorial.16apr98.moments.integrated') 

# TUTORIAL NOTES: For moment 0 we use the image corrected for the
# mosaic response to get correct integrated flux densities. However,
# in *real signal* regions, the value of moment 1 is not dependent on
# the flux being correct so the non-pb corrected SAULT image can be
# used, this avoids having lots of junk show up at the edges of your
# moment 1 image due to the primary beam correction. Try it both ways
# and see for yourself.

# TUTORIAL NOTES:
#
# Moments greater than zero need to have a conservative lower
# flux cutoff to produce sensible results.

immoments(imagename='ngc4826.tutorial.16apr98.src.clean.image',
	  moments=[1],includepix=[0.2,1000.0],
	  chans='6~27',
          outfile='ngc4826.tutorial.16apr98.moments.mom1') 

immoments(imagename='ngc4826.tutorial.16apr98.src.clean.image',
          moments=[1],includepix=[],
          chans='0',
          outfile='ngc4826.tutorial.16apr98.moments.plane0.mom1') 

immoments(imagename='ngc4826.tutorial.16apr98.src.clean.image',
	  moments=[1],includepix=[],
	  chans='35',
          outfile='ngc4826.tutorial.16apr98.moments.plane35.mom1')


#
##########################################################################
#
# Statistics on moment images
#
print '--ImStat (Moment images)--'

momzerostat=imstat('ngc4826.tutorial.16apr98.moments.integrated')

try:
    print "Found moment 0 max = "+str(momzerostat['max'][0])
    print "Found moment 0 rms = "+str(momzerostat['rms'][0])
except:
    pass

momonestat = imstat('ngc4826.tutorial.16apr98.moments.mom1')

try:
    print "Found moment 1 median = "+str(momonestat['median'][0])
except:
    pass



ia.open('ngc4826.tutorial.16apr98.src.clean.image')
csys=ia.coordsys()
vel0=0.0
vel35=0.0

momoneplane0=imstat('ngc4826.tutorial.16apr98.moments.plane0.mom1')
print "Found plane 0 moment 1 value = "+str(momoneplane0['median'][0])

momoneplane35=imstat('ngc4826.tutorial.16apr98.moments.plane35.mom1')
print "Found plane 35 moment 1 value = "+str(momoneplane35['median'][0])

if(type(momoneplane0)==bool):
    vel0=csys.frequencytovelocity(ia.toworld([0,0,0,0])['numeric'][3])
if(type(momoneplane35)==bool):
    vel35=csys.frequencytovelocity(ia.toworld([0,0,0,35])['numeric'][3])


#
##########################################################################
#
# Get MS stats
#
ms.open('ngc4826.tutorial.16apr98.1310+323.split.ms')
vismean_cal=pl.mean(ms.range(["amplitude"]).get("amplitude"))
ms.close()
ms.open('ngc4826.tutorial.16apr98.src.split.ms')
vismean_src=pl.mean(ms.range(["amplitude"]).get("amplitude"))
ms.close()

#


#
##########################################################################
# Previous results to be used for regression


# slight change in 1 regression value - Kumar fixed a bug in setjy with
# multiple spws, which could affect this script.
testdate = '2010-04-29 (RR)'
testvers = 'CASA Version 3.0.2 (build #11306)'
clean_image_max = 1.615747
clean_offsrc_rms = 0.058497
clean_offline_rms = 0.055416
clean_momentzero_max = 163.726852
clean_momentzero_rms = 15.206372
#
#  32 bits gets 423.6954 and 64 bits gets 422.142792
#  diff << 1 chanwidth.
clean_momentone_median = 422.92
clean_momentone_planezero = 696.702393
clean_momentone_planelast = 127.786629
vis_mean_cal = 195.0509
vis_mean_src = 54.665
model_sum = 75.7516
model_pbcor_sum = 75.92 # was 66.88 Peak hits a channel better?

# RR, 1/19/2011 - The rmses went down, just like in orionmos4sim.  This is
# good, so I won't complain too loudly.  The moment 1 median and pbcor_sum have
# jiggled around a fair bit.  The median is _not_ affected by the two spurious
# blobs at 501.64 km/s, though.  (Verified by doing imstat with a tight polygon
# region.)
clean_offsrc_rms = 0.04736
clean_offline_rms = 0.049904
clean_momentzero_rms = 13.737679
# The chanwidth is ~16 km/s.
clean_momentone_median = 435.368103

model_pbcor_sum = 77.7232

## # RR, 3/11/2011 - The rmses went up, but not to their historical maxima.  The
## # model_pbcor_sum went down, but not to its historical minimum.  Nobody seems
## # to know why.
## clean_offsrc_rms = 0.0535
## clean_offline_rms = 0.0563
## model_pbcor_sum = 62.5022
## # RR, 3/12/2011 - And now, mysteriously, they're back to their 1/19/2011 values.

## RR, 3/25 - 4/3/2011, after clean was changed to used the center of output
## channel frequencies, instead of center of the first input channel in each
## output channel.
clean_image_max = 1.4637
clean_momentone_median = 424.3254
clean_momentone_planezero = 690.6068
clean_momentone_planelast = 121.6911

canonical = {}
canonical['exist'] = True

canonical['date'] = testdate
canonical['version'] = testvers
canonical['user'] = 'smyers'
canonical['host'] = 'sandrock'
canonical['cwd'] = '/home/sandrock/smyers/Testing/Patch4/N4826'
print "Using internal regression from "+canonical['version']+" on "+canonical['date']

canonical_results = {}
canonical_results['clean_image_max'] = {}
canonical_results['clean_image_max']['value'] = clean_image_max
canonical_results['clean_image_offsrc_max'] = {}
canonical_results['clean_image_offsrc_max']['value'] = clean_offsrc_rms
canonical_results['clean_image_offline_max'] = {}
canonical_results['clean_image_offline_max']['value'] = clean_offline_rms
canonical_results['clean_momentzero_max'] = {}
canonical_results['clean_momentzero_max']['value'] = clean_momentzero_max
canonical_results['clean_momentzero_rms'] = {}
canonical_results['clean_momentzero_rms']['value'] = clean_momentzero_rms
# And from STM 2008-06-18
canonical_results['clean_momentone_median'] = {}
canonical_results['clean_momentone_median']['value'] = clean_momentone_median
# And from STM 2008-06-30
canonical_results['clean_momentone_planezero'] = {}
canonical_results['clean_momentone_planezero']['value'] = clean_momentone_planezero
canonical_results['clean_momentone_planelast'] = {}
canonical_results['clean_momentone_planelast']['value'] = clean_momentone_planelast
canonical_results['clean_psfcenter'] = {}
canonical_results['clean_psfcenter']['value'] = 1.0

# MS mean STM 2008-07-02
canonical_results['vis_mean_cal'] = {}
canonical_results['vis_mean_cal']['value'] = vis_mean_cal
canonical_results['vis_mean_src'] = {}
canonical_results['vis_mean_src']['value'] = vis_mean_src

# Model fluxes STM 2008-10-22
canonical_results['model_sum'] = {}
canonical_results['model_sum']['value'] = model_sum
canonical_results['model_pbcor_sum'] = {}
canonical_results['model_pbcor_sum']['value'] = model_pbcor_sum

canonical['results'] = canonical_results

print "Canonical Regression (default) from "+canonical['date']

#
# Try and load previous results from regression file
#
regression = {}
regressfile = scriptprefix + '.pickle'
prev_results = {}

try:
    fr = open(regressfile,'r')
except:
    print "No previous regression results file "+regressfile
    regression['exist'] = False
else:
    u = pickle.Unpickler(fr)
    regression = u.load()
    fr.close()
    print "Regression results filled from "+regressfile
    print "Regression from version "+regression['version']+" on "+regression['date']
    regression['exist'] = True

    prev_results = regression['results']
    
#
##########################################################################
# Calculate regression values
##########################################################################
#
print '--Calculate Results--'
print ''
#
# Currently using non-PBcor values
#
try:
    srcmax = srcstat['max'][0]
except:
    srcmax = 0.0

try:
    offrms = offstat['sigma'][0]
except:
    offrms = 0.0

try:
    offlinerms = offlinestat['sigma'][0]
except:
    offlinerms = 0.0
    
try:
    momzero_max = momzerostat['max'][0]
except:
    momzero_max = 0.0

try:
    momzero_rms = momzerostat['rms'][0]
except:
    momzero_rms = 0.0

try:
    momone_median = momonestat['median'][0]
except:
    momone_median = 0.0

#
# Added these sanity checks STM 2008-06-30
#
try:
    momone_plane0 = momoneplane0['median'][0]
except:
    momone_plane0 = vel0

try:
    momone_plane35 = momoneplane35['median'][0]
except:
    momone_plane35 = vel35

try:
    psfcenter = psfstat['mean'][0]
except:
    psfcenter = 0.0

#
# Added model image fluxes STM 2008-10-22
#
try:
    modflux = modstat['sum'][0]
except:
    modflux = 0.0

try:
    pbcormodflux = pbcormodstat['sum'][0]
except:
    pbcormodflux = 0.0

#
# Store results in dictionary
#
new_regression = {}

# Some date and version info
import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())

# System info
myvers = casalog.version()
try:
    myuser = os.getlogin()
except:
    myuser = os.getenv('USER')
#myhost = str( os.getenv('HOST') )
myuname = os.uname()
myhost = myuname[1]
myos = myuname[0]+' '+myuname[2]+' '+myuname[4]
mycwd = os.getcwd()
mypath = os.environ.get('CASAPATH')

mydataset = 'NGC4826 16apr98 BIMA'

# Save info in regression dictionary
new_regression['date'] = datestring
new_regression['version'] = myvers
new_regression['user'] = myuser
new_regression['host'] = myhost
new_regression['cwd'] = mycwd
new_regression['os'] = myos
new_regression['uname'] = myuname
new_regression['aipspath'] = mypath

new_regression['dataset'] = mydataset

# Fill results
# Note that 'op' tells what to do for the diff :
#    'divf' = abs( new - prev )/prev
#    'diff' = new - prev

results = {}

op = 'divf'
tol = 0.08
results['clean_image_max'] = {}
results['clean_image_max']['name'] = 'Clean image max'
results['clean_image_max']['value'] = srcmax
results['clean_image_max']['op'] = op
results['clean_image_max']['tol'] = tol

results['clean_image_offsrc_max'] = {}
results['clean_image_offsrc_max']['name'] = 'Clean image off-src rms'
results['clean_image_offsrc_max']['value'] = offrms
results['clean_image_offsrc_max']['op'] = op
results['clean_image_offsrc_max']['tol'] = tol

results['clean_image_offline_max'] = {}
results['clean_image_offline_max']['name'] = 'Clean image off-line rms'
results['clean_image_offline_max']['value'] = offlinerms
results['clean_image_offline_max']['op'] = op
results['clean_image_offline_max']['tol'] = tol

results['clean_momentzero_max'] = {}
results['clean_momentzero_max']['name'] = 'Moment 0 image max'
results['clean_momentzero_max']['value'] = momzero_max
results['clean_momentzero_max']['op'] = op
results['clean_momentzero_max']['tol'] = tol

results['clean_momentzero_rms'] = {}
results['clean_momentzero_rms']['name'] = 'Moment 0 image rms'
results['clean_momentzero_rms']['value'] = momzero_rms
results['clean_momentzero_rms']['op'] = op
results['clean_momentzero_rms']['tol'] = tol

op = 'diff'
tol = 0.1
results['clean_momentone_median'] = {}
results['clean_momentone_median']['name'] = 'Moment 1 image median'
results['clean_momentone_median']['value'] = momone_median
results['clean_momentone_median']['op'] = op
results['clean_momentone_median']['tol'] = 3.0 # km/s.  Was 0.1 before CAS-2163.

#
# Added these sanity checks STM 2008-06-30
#
results['clean_momentone_planezero'] = {}
results['clean_momentone_planezero']['name'] = 'Moment 1 plane 0'
results['clean_momentone_planezero']['value'] = momone_plane0
results['clean_momentone_planezero']['op'] = op
results['clean_momentone_planezero']['tol'] = tol

results['clean_momentone_planelast'] = {}
results['clean_momentone_planelast']['name'] = 'Moment 1 plane 35'
results['clean_momentone_planelast']['value'] = momone_plane35
results['clean_momentone_planelast']['op'] = op
results['clean_momentone_planelast']['tol'] = tol

tol = 0.01
results['clean_psfcenter'] = {}
results['clean_psfcenter']['name'] = 'PSF CH27 at RefPix'
results['clean_psfcenter']['value'] = psfcenter
results['clean_psfcenter']['op'] = op
results['clean_psfcenter']['tol'] = tol

# And return the ms mean 2008-07-02 STM
op = 'divf'
tol = 0.08
results['vis_mean_cal'] = {}
results['vis_mean_cal']['name'] = 'Vis mean of cal'
results['vis_mean_cal']['value'] = vismean_cal
results['vis_mean_cal']['op'] = op
results['vis_mean_cal']['tol'] = tol

results['vis_mean_src'] = {}
results['vis_mean_src']['name'] = 'Vis mean of src'
results['vis_mean_src']['value'] = vismean_src
results['vis_mean_src']['op'] = op
results['vis_mean_src']['tol'] = tol

# model total fluxes 2008-10-22 STM

results['model_sum'] = {}
results['model_sum']['name'] = 'Model image sum'
results['model_sum']['value'] = modflux
results['model_sum']['op'] = op
results['model_sum']['tol'] = tol

results['model_pbcor_sum'] = {}
results['model_pbcor_sum']['name'] = 'PBcor Model image sum'
results['model_pbcor_sum']['value'] = pbcormodflux
results['model_pbcor_sum']['op'] = op
results['model_pbcor_sum']['tol'] = tol

# Now go through and regress
resultlist = ['clean_image_max','clean_image_offsrc_max','clean_image_offline_max',
              'clean_momentzero_max','clean_momentzero_rms','clean_momentone_median',
              'clean_momentone_planezero','clean_momentone_planelast','clean_psfcenter',
              'vis_mean_cal','vis_mean_src','model_sum','model_pbcor_sum']

for keys in resultlist:
    res = results[keys]
    prev = None
    if prev_results.has_key(keys):
        # This is a known regression
        prev = prev_results[keys]
        results[keys]['test'] = 'Last'
    elif canonical_results.has_key(keys):
        # Go back to canonical values
        prev = canonical_results[keys]
        results[keys]['test'] = 'Canon'
    if prev:
        new_val = res['value']
        prev_val = prev['value']
        new_diff = new_val - prev_val
        if res['op'] == 'divf':
            new_diff /= prev_val

        if abs(new_diff) > res['tol']:
            new_status = 'Failed'
        else:
            new_status = 'Passed'
        
        results[keys]['prev'] = prev_val
        results[keys]['diff'] = new_diff
        results[keys]['status'] = new_status
    else:
        # Unknown regression key
        results[keys]['prev'] = 0.0
        results[keys]['diff'] = 1.0
        results[keys]['status'] = 'Missed'
        results[keys]['test'] = 'none'

# Done filling results
new_regression['results'] = results

# Dataset size info
datasize_raw = 96.0 # MB
datasize_ms = 279.0 # MB
new_regression['datasize'] = {}
new_regression['datasize']['raw'] = datasize_raw
new_regression['datasize']['ms'] = datasize_ms

#
# Timing
#

#
##########################################################################
#
# Save regression results as dictionary using Pickle
#
pickfile = 'out.'+prefix + '.regression.'+datestring+'.pickle'
f = open(pickfile,'w')
p = pickle.Pickler(f)
p.dump(new_regression)
f.close()

print ""
print "Regression result dictionary saved in "+pickfile
print ""
print "Use Pickle to retrieve these"
print ""

# e.g.
# f = open(pickfile)
# u = pickle.Unpickler(f)
# clnmodel = u.load()
# polmodel = u.load()
# f.close()

#
##########################################################################
#
# Now print out results
# The following writes a logfile for posterity
#
##########################################################################
#
#outfile='n4826.'+datestring+'.log'
outfile='out.'+prefix+'.'+datestring+'.log'
logfile=open(outfile,'w')

# Print version info to outfile
print >>logfile,'Running '+myvers+' on host '+myhost
print >>logfile,'at '+datestring
print >>logfile,''

#
# Report a few key stats
#
print '  NGC4826 Image Cube Max = '+str(srcstat['max'][0])
print "          At ("+str(srcstat['maxpos'][0])+","+str(srcstat['maxpos'][1])+") Channel "+str(srcstat['maxpos'][3])
print '          '+srcstat['maxposf']
print ''
print '          Off-Source Rms = '+str(offstat['sigma'][0])
print '          Signal-to-Noise ratio = '+str(srcstat['max'][0]/offstat['sigma'][0])
print ''
print '          Off-Line   Rms = '+str(offlinestat['sigma'][0])
print '          Signal-to-Noise ratio = '+str(srcstat['max'][0]/offlinestat['sigma'][0])
print ''

print >>logfile,'  NGC4826 Image Cube Max = '+str(srcstat['max'][0])
print >>logfile,"          At ("+str(srcstat['maxpos'][0])+","+str(srcstat['maxpos'][1])+") Channel "+str(srcstat['maxpos'][3])
print >>logfile,'          '+srcstat['maxposf']
print >>logfile,''
print >>logfile,'          Off-Source Rms = '+str(offstat['sigma'][0])
print >>logfile,'          Signal-to-Noise ratio = '+str(srcstat['max'][0]/offstat['sigma'][0])
print >>logfile,''
print >>logfile,'          Off-Line   Rms = '+str(offlinestat['sigma'][0])
print >>logfile,'          Signal-to-Noise ratio = '+str(srcstat['max'][0]/offlinestat['sigma'][0])
print >>logfile,''

# Print out comparison:
res = {}
resultlist = ['clean_image_max','clean_image_offsrc_max','clean_image_offline_max',
               'clean_momentzero_max','clean_momentzero_rms','clean_momentone_median',
              'clean_momentone_planezero','clean_momentone_planelast','clean_psfcenter',
              'vis_mean_cal','vis_mean_src','model_sum','model_pbcor_sum']

# First versus canonical values
print >>logfile,'---'
print >>logfile,'Regression versus previous values:'
print >>logfile,'---'
print '---'
print 'Regression versus previous values:'
print '---'

if regression['exist']:
    print >>logfile,"  Regression results filled from "+regressfile
    print >>logfile,"  Regression from version "+regression['version']+" on "+regression['date']
    print >>logfile,"  Regression platform "+regression['host']
    
    print "  Regression results filled from "+regressfile
    print "  Regression from version "+regression['version']+" on "+regression['date']
    print "  Regression platform "+regression['host']
    if regression.has_key('aipspath'):
        print >>logfile,"  Regression casapath "+regression['aipspath']
        print "  Regression casapath "+regression['aipspath']
    
else:
    print >>logfile,"  No previous regression file"

print ""
print >>logfile,""

final_status = 'Passed'
for keys in resultlist:
    res = results[keys]
    print '--%30s : %12.6f was %12.6f %4s %12.6f (%6s) %s ' % ( res['name'], res['value'], res['prev'], res['op'], res['diff'], res['status'], res['test'] )
    print >>logfile,'--%30s : %12.6f was %12.6f %4s %12.6f (%6s) %s ' % ( res['name'], res['value'], res['prev'], res['op'], res['diff'], res['status'], res['test'] )
    if res['status']=='Failed':
        final_status = 'Failed'

if (final_status == 'Passed'):
    regstate=True
    print >>logfile,'---'
    print >>logfile,'Passed Regression test for NGC 4826 Mosaic'
    print >>logfile,'---'
    print 'Passed Regression test for NGC 4826 Mosaic'
else:
    regstate=False
    print >>logfile,'----FAILED Regression test for NGC 4826 Mosaic'
    print '----FAILED Regression test for NGC 4826 Mosaic'
    
#
##########################################################################

logfile.close()

print "Done with NGC4826 Tutorial Regression"
#
##########################################################################
