##########################################################################
#                                                                        #
# 2008 NRAO Synthesis Summer School                                      #
# VLA Continuum Polarimetry Tutorial for CASA                            #
# This script reads VLA archive files and stores them as a CASA          #
# measurement set, and then processes it.                                #
#                                                                        #
# Original version GMoellenbrok 2008-06-08  Patch 2 summer school        #
# Modified/merge   STM          2008-07-31  Clean up                     #
# Updated version  STM          2008-08-07  finishing touches            #
# Updated version  GMoellenbrock2009-11-20  Add evlabands=F              #
#                                                                        #
# Based on 2008 Synthesis Imaging Workshop script                        #
#                                                                        #
# Script Notes:                                                          #
#    o Uses the VLA export files AT166_1 to 3, which should be in the    #
#      working directory that casapy was started in                      #
#                                                                        #
#    o This script has some interactive commands, such as with           #
#      plotxy and the viewer.  If scriptmode=True, then this script      #
#      will stop and require a carriage-return to continue at these      #
#      points.                                                           #
#                                                                        #
#    o Sometimes cut-and-paste of a series of lines from this script     #
#      into the casapy terminal will get garbled (usually a single       #
#      dropped character). In this case, try fewer lines, like groups    #
#      of 4-6.                                                           #
#                                                                        #
#                                                                        #
##########################################################################

import time
import os
import pickle

# 
# This is a script that reduces B- and C-configuration VLA continuum 
#  polarimetry data at 5 GHz on 3C129 and calibrators, and images the 
#  dual-config 3C129 in full polarization. 
# 
# Observations:  AT166 
#                B-array: 1994Jul25 
#                C-array: 1994Nov03 
# 
# Sources:   Science Target: 3C129, a radio galaxy 
#            Calibrator:     0420+417, observed altenately with 3C129 
#            Calibrator:     0518+165 (3C138), for flux density/poln p.a. 
#            Calibrator:     0134+329 (3C48), for flux density/poln p.a. 
# 
# Obs Modes: Two 50 MHz continuum (single chan) sub-bands at 4585.1 & 4885.1 MHz 
#            Full polarization: RR,RL,LR,LL 
# 
 
# In the following script, the B array and C array data are separately 
#  filled and reduced.  The calibration parameters are fairly standard, 
#  and some variations may be suggested.  In general, after each calibration 
#  opearation, a data or calibration plotting command is included, again 
#  with suggestions on possible variations. 
# 
# This script is not written to be run automatically; doing so will likely 
#  yield a less-than-optimal result.  It is intended that students will 
#  cut-and-paste this script one task at a time to progress through the 
#  reduction. 
# The methods are run in the function-call style:  task(param1=x,param2=y) 
#  To see the range of parameters for a function, use 'inp <task>'.  After 
#  running a task (in function-call style), type 'tget <task>', and 
#  'inp <task>' to review the parameters that were used. 
 
# if you are running it and want it to stop during interactive parts.

scriptmode = F

# Enable benchmarking?
benchmarking = True

#=====================================================================
#
# Set up some useful variables

pathname=os.environ.get('CASAPATH').split()[0]

prefix='3c129.tutorial'

# Sets a shorthand for fixed input script/regression files
scriptprefix='3c129_tutorial_regression'

# A few parameters used repeatedly below, are defined here: 
#  (Be sure to reset these if you exit CASA and start it up again) 
msnameB='at166B.ms'; 
msnameC='at166C.ms'; 
 
#
#=====================================================================
# Clean up old versions of files to be created in this script
os.system('rm -rf '+prefix+'.* at166B.* at166C.* 3C129BC.*')

# Start timing
if benchmarking:
    startTime=time.time()
    startProc=time.clock()

#
#=====================================================================
# IMPORT, FLAG, CALIBRATE, IMAGE THE B-CONFIGURATION DATA
#=====================================================================
#
print ""
print "Loading B-config data..."
print ""

#=====================================================================
# Fill B-config data at C-band (5 GHz)
print "--Import (Bconfig)--"
importvla(archivefiles=['AT166_1', 'AT166_2'],vis=msnameB,bandname='C',evlabands=F); 
 
#=====================================================================
# List a summary of the dataset in the logger
print "--Listobs--"
listobs(vis=msnameB); 
 
#--> Note scan sequence, fields, and spectral window information, etc. 
 
#=====================================================================
# set flux density calibrator total intensity models 
#  NB: these sources are resolved, so we use model images provided 
#      by the VLA (copy them from the data repository: data/VLA/CalModels/
#      or point to the location on your system)
#
# Location of Cal Models
# e.g. for MacOSX
#fluxcaldir = '/opt/casa/data/nrao/VLA/CalModels/'
# or standard distro
fluxcaldir = pathname + '/data/nrao/VLA/CalModels/'
# or in place
#fluxcaldir = './'

#  NB: By default, the model for 0420+417 is a 1 Jy point source 
print "--Setjy--"
# Should say:
## 0518+165  spwid=  0  [I=3.688, Q=0, U=0, V=0] Jy, (Perley-Taylor 99)
## 0518+165  spwid=  1  [I=3.862, Q=0, U=0, V=0] Jy, (Perley-Taylor 99)
setjy(vis=msnameB,field='0518+165',modimage=fluxcaldir+'3C138_C.im',scalebychan=False,standard='Perley-Taylor 99')
## 0134+329  spwid=  0  [I=5.405, Q=0, U=0, V=0] Jy, (Perley-Taylor 99)
## 0134+329  spwid=  0  [I=5.739, Q=0, U=0, V=0] Jy, (Perley-Taylor 99)
setjy(vis=msnameB,field='0134+329',modimage=fluxcaldir+'3C48_C.im',scalebychan=False,standard='Perley-Taylor 99') 
 
#=====================================================================
# Plot data and interactively edit 
#  One spw at a time, calibrators only, RR, LL only 
print "--Plotxy--"
if scriptmode:
    plotxy(vis=msnameB,spw='0',field='0420+417,0518+165,0134+329',selectdata=T,correlation='RR,LL');
    print 'Plotting 0420+417,0518+165,0134+329 SPW 0'
    print "  Note VA01 seems to be bad (low) on one integration of one scan"
    print "  Mark, Locate, and Flag this"
    user_check=raw_input('hit Return to continue script\n')

    plotxy(vis=msnameB,spw='1',field='0420+417,0518+165,0134+329',selectdata=T,correlation='RR,LL'); 
    print 'Plotting 0420+417,0518+165,0134+329 SPW 1'
    print "  Mark, Locate, and Flag bad VA01 integration in this SPW 1 also"
    user_check=raw_input('hit Return to continue script\n')
else:
    plotxy(vis=msnameB,spw='0',field='0420+417,0518+165,0134+329',selectdata=T,correlation='RR,LL',interactive=F,figfile='at166B.plotxy.initial.spw0.png');
    plotxy(vis=msnameB,spw='1',field='0420+417,0518+165,0134+329',selectdata=T,correlation='RR,LL',interactive=F,figfile='at166B.plotxy.initial.spw1.png'); 
    print "--Flagdata--"
    tflagdata(vis=msnameB,antenna='VA01',timerange='1994/07/25/14:21:10.0~14:21:20.0',mode='manual')

#--> Variations: plot on different axes (e.g., phase vs. time, real vs imag, amp/phase vs. uvdist) 
#-->             plot model data for calibrators 
#-->             plot 3C129 data 
 
#=====================================================================
# Solve for gains on calibrators 
#  NB: reference phases to VA12; pre-apply parallactic angle correction 
print "--Gaincal--"
gaincal(vis=msnameB,caltable='at166B.gcal',field='0420+417,0518+165,0134+329',refant='VA12',parang=T); 
 
#--> (22/22 good solutions) 
 
#=====================================================================
# Examine solutions: 
print "--Plotcal--"
# Use subplot panels to stack amp, phase, snr above each other
if scriptmode:
    plotcal(caltable='at166B.gcal',yaxis='amp',subplot=311); 
    plotcal(caltable='at166B.gcal',yaxis='phase',subplot=312); 
    plotcal(caltable='at166B.gcal',yaxis='snr',subplot=313); 
    user_check=raw_input('hit Return to continue script\n')
else:
    plotcal(caltable='at166B.gcal',yaxis='amp',subplot=211,showgui=F,figfile=''); 
    plotcal(caltable='at166B.gcal',yaxis='phase',subplot=212,showgui=F,figfile='at166B.gcal.plotcal.png'); 
    
#--> Variations:  plot solutions per antenna using iteration='antenna'
 
#=====================================================================
# Scale gain solution from 0420+417 according to f.d. calibrators 
print "--Fluxscale--"
fluxscale(vis=msnameB,caltable='at166B.gcal',fluxtable='at166B.fcal',reference='0518+165,0134+329'); 
 
# -->  0420: 1.441/1.443 Jy (broadly consistent with VLA Cal man values at L & X) 
 
#=====================================================================
# Examine solutions: 
print "--Plotcal--"
if scriptmode:
    plotcal(caltable='at166B.fcal',yaxis='amp'); 
    user_check=raw_input('hit Return to continue script\n')
else:
    plotcal(caltable='at166B.fcal',yaxis='amp',showgui=F,figfile='at166B.fcal.plotcal.png'); 

#--> Note that gain amps are ~constant now  
 
#=====================================================================
# Solve for instrumental polarization on 0420+417 (and also for source poln) 
# NB: IMPORTANT: use gcal---not fcal---here because model is _still_ 1.0 Jy 
print "--Polcal (D)--"
polcal(vis=msnameB,caltable='at166B.dcal',field='0420+417',refant='VA12',gaintable='at166B.gcal',gainfield='0420+417'); 
 
# --> 2/2 good solutions; 
# --> 0420: 0.027Jy @ 61.2deg / 0.024Jy @ -83.0 Jy 
 
#=====================================================================
# Examine solutions: 
print "--Plotcal--"
if scriptmode:
    plotcal(caltable='at166B.dcal',xaxis='antenna',yaxis='amp'); 
    user_check=raw_input('hit Return to continue script\n')
else:
    plotcal(caltable='at166B.dcal',xaxis='antenna',yaxis='amp',showgui=F,figfile='at166B.dcal.plotcal.png'); 

# You can plot imag vs. real also
# plotcal(caltable='at166B.dcal',xaxis='real',yaxis='imag',plotrange=[-0.05,0.05,-0.05,0.05]); 
 
#=====================================================================
# Set full polarization model for 0518+165 (pol is 11.1% @ -11 deg  [RL = -22]) 
#  NB: neglecting source structure here) 
print "--Setjy (X)--"
setjy(vis=msnameB,field='0518+165',spw='0',scalebychan=False,fluxdensity=[3.688, 0.380, -0.153, 0.0]); 
setjy(vis=msnameB,field='0518+165',spw='1',scalebychan=False,fluxdensity=[3.862, 0.397, -0.161, 0.0]); 
 
#=====================================================================
# Solve for polarization position angle on 0518+165 
print "--Polcal (X)--"
polcal(vis='at166B.ms',caltable='at166B.xcal',field='0518+165',refant='VA12',poltype='X',gaintable=['at166B.fcal', 'at166B.dcal'],gainfield=['0518+165','0420+417']); 
 
# --> 2/2 good solutions 
# --> 0518:  2.7deg  / 41.8deg  (I need to check these numbers) 

#=====================================================================
# apply all calibration... 
#  (NB: different fields are selected from each caltable, depending on selected data fields) 
#  (NB: parang=T is set to rotate polarization p.a. frame from antennas to sky 
print "--Applycal--"
print "  apply calibration to 0420+417,3C129"
applycal(vis=msnameB,field='0420+417,3C129',gaintable=['at166B.fcal','at166B.dcal','at166B.xcal'],gainfield=['0420+417','0420+417','0518+165'],parang=T); 

print "  apply calibration to 0518+165"
applycal(vis=msnameB,field='0518+165',gaintable=['at166B.fcal','at166B.dcal','at166B.xcal'],gainfield=['0518+165','0420+417','0518+165'],parang=T); 

print "  apply calibration to 0134+329"
applycal(vis=msnameB,field='0134+329',gaintable=['at166B.fcal','at166B.dcal','at166B.xcal'],gainfield=['0134+329','0420+417','0518+165'],parang=T); 
 
#=====================================================================
# Examine (edit?) calibrated data (calibrators) 
print "--Plotxy (corrected)--"
if scriptmode:
    plotxy(vis=msnameB,datacolumn='corrected',field='0420+417,0518+165,0134+329',selectdata=T,correlation='RR,LL'); 
    user_check=raw_input('hit Return to continue script\n')
else:
    plotxy(vis=msnameB,datacolumn='corrected',field='0420+417,0518+165,0134+329',selectdata=T,correlation='RR,LL',interactive=F,figfile='at166B.plotxy.final.png'); 

#--> Variations: plot on different axes (e.g., phase vs. time, real vs imag) 
#-->             overlay model data for calibrators 
#-->             plot 3C129 data 
# For example: 
# Examine cross-hand data (real vs. imag) 
# plotxy(vis=msnameB,xaxis='real',yaxis='imag',datacolumn='corrected',selectdata=True,correlation='RL,LR',plotrange=[-0.5,0.5,-0.5,0.5],field = '0518+165'); 
# plotxy(vis=msnameB,xaxis='real',yaxis='imag',datacolumn='corrected',selectdata=True,correlation='RL,LR',plotrange=[-0.5,0.5,-0.5,0.5],field = '0134+329'); 
# plotxy(vis=msnameB,xaxis='real',yaxis='imag',datacolumn='corrected',selectdata=True,correlation='RL,LR',plotrange=[-0.5,0.5,-0.5,0.5],field = '0420+417'); 
 
#--> NB: RL and LR signal are complex conjugates of each other (Q+iU & Q-iU) 
 
#=====================================================================
# do some simple imaging of each source 
print "--Clean--"
# 3C129: 
# We will do a simple image-plane Hogbom clean with psfmode='hogbom' and imagermode=''
# This will clean the IQUV planes consecutively
#
# Non-interactive (no clean boxes) for now
#
clean(vis=msnameB,imagename='at166B.3c129',field='3C129',psfmode='hogbom',imagermode='',niter=2500,imsize=[2048,2048],cell=['0.3arcsec','0.3arcsec'],stokes='IQUV',weighting='briggs',robust=0.5,interactive=F);

print "  Clean image is at166B.3c129.image"

# You can do a a Cotton-Schwab clean with psfmode='clark' and imagermode='csclean'
# You can try a threshold also.

# You can clean the calibrators also:
# 0518: 
# clean(vis=msnameB,imagename='at166B.0518',field='0518+165',psfmode='hogbom',niter=500,imsize=[512,512],cell=['0.3arcsec','0.3arcsec'],stokes='IQUV',weighting='briggs',robust=0.5,interactive=F); 
 
# 0134: 
# clean(vis=msnameB,imagename='at166B.0134',field='0134+329',psfmode='hogbom',niter=500,imsize=[512,512],cell=['0.3arcsec','0.3arcsec'],stokes='IQUV',weighting='briggs',robust=0.5,interactive=F); 
 
# 0420: 
# clean(vis=msnameB,imagename='at166B.0420',field='0420+417',psfmode='hogbom',niter=500,imsize=[512,512],cell=['0.3arcsec','0.3arcsec'],stokes='IQUV',weighting='briggs',robust=0.5,interactive=F); 

#=====================================================================
# Examine images in viewer... 
if scriptmode:
    print "--Viewer--"
    viewer('at166B.3c129.image')
    user_check=raw_input('When done, close viewer and hit Return to continue script\n')
    
#  Questions:  Do calibrator polarizations come out right? 
#              Is calibrator structure as expected? 
#              Is 3C129 image any good?  (cf C-config imaging below) 
 
# Variations:  clean interactively (clean boxes) 
#              try different weighting  
 
#=====================================================================
# IMPORT, FLAG, CALIBRATE, IMAGE THE C-CONFIGURATION DATA
#=====================================================================
# NOW, reduce C-config data in much the same way...
print ""
print "Now reducing C-config data..."
print ""
 
# Fill C-config data at C-band (5 GHz)
print "--Import (Cconfig)--"
importvla(archivefiles='AT166_3',vis=msnameC,bandname='C',evlabands=F); 
 
#=====================================================================
# List a summary of the dataset in the logger
print "--Listobs--"
listobs(vis=msnameC); 
 
#=====================================================================
# set flux density calibrator total intensity models 
#  NB: these sources are resolved, so we use model images provided 
#      by the VLA, with location set in B-config part of script
#  NB: By default, the model for 0420+417 is a 1 Jy point source 
print "--Setjy--"
setjy(vis=msnameC,field='0518+165',modimage=fluxcaldir+'3C138_C.im',scalebychan=False,standard='Perley-Taylor 99'); 
setjy(vis=msnameC,field='0134+329',modimage=fluxcaldir+'3C48_C.im',scalebychan=False,standard='Perley-Taylor 99');
 
#=====================================================================
# Plot data and interactively edit 
#  One spw at a time, calibrators only, RR, LL only 
print "--Plotxy--"
if scriptmode:
    plotxy(vis=msnameC,spw='0',field='0420+417,0518+165,0134+329',selectdata=T,correlation='RR,LL'); 
    print 'Plotting 0420+417,0518+165,0134+329 SPW 0'
    user_check=raw_input('hit Return to continue script\n')

    plotxy(vis=msnameC,spw='1',field='0420+417,0518+165,0134+329',selectdata=T,correlation='RR,LL'); 
    print 'Plotting 0420+417,0518+165,0134+329 SPW 1'
    user_check=raw_input('hit Return to continue script\n')
else:
    plotxy(vis=msnameC,spw='0',field='0420+417,0518+165,0134+329',selectdata=T,correlation='RR,LL',interactive=F,figfile='at166C.plotxy.initial.spw0.png');
    plotxy(vis=msnameC,spw='1',field='0420+417,0518+165,0134+329',selectdata=T,correlation='RR,LL',interactive=F,figfile='at166C.plotxy.initial.spw1.png'); 
 
#--> Variations: plot on different axes (e.g., phase vs. time, real vs imag, amp/phase vs. uvdist) 
#-->             plot model data for calibrators (datacolumn='model') 
#-->             plot 3C129 data 
#-->             plot individual baselines (iteration='baseline') 
 
# Solve for gains on calibrators 
#  NB: reference phases to VA12; pre-apply parallactic angle correction 
print "--Gaincal--"
gaincal(vis=msnameC,caltable='at166C.gcal',field='0420+417,0518+165,0134+329',refant='VA12',parang=T,solint=500); 
 
#--> (12/14 good solutions)  NB: no solutions for 0134+329 (too little data?) 
 
#=====================================================================
# Examine solutions: 
print "--Plotcal--"
if scriptmode:
    plotcal(caltable='at166C.gcal',yaxis='amp'); 
    user_check=raw_input('hit Return to continue script\n')

    plotcal(caltable='at166C.gcal',yaxis='phase'); 
    user_check=raw_input('hit Return to continue script\n')
else:
    plotcal(caltable='at166C.gcal',yaxis='amp',subplot=211,showgui=F,figfile=''); 
    plotcal(caltable='at166C.gcal',yaxis='phase',subplot=212,showgui=F,figfile='at166C.gcal.plotcal.png'); 

#--> Variations:  plot solutions per antenna, spw, etc. 
 
#=====================================================================
# Scale gain solution from 0420+417 according to f.d. calibrators 
print "--Fluxscale--"
fluxscale(vis=msnameC,caltable='at166C.gcal',fluxtable='at166C.fcal',reference='0518+165'); 
# -->  0420:  1.282/1.292 Jy (broadly consistent with VLA Cal man values at L & X) 
 
#=====================================================================
# Examine solutions: 
print "--Plotcal--"
if scriptmode:
    plotcal(caltable='at166C.fcal',yaxis='amp'); 
    user_check=raw_input('hit Return to continue script\n')
else:
    plotcal(caltable='at166C.fcal',yaxis='amp',showgui=F,figfile='at166C.fcal.plotcal.png'); 
 
#--> gain amplitudes now ~constant 
 
# Solve for instrumental polarization on 0420+417 (and also for source poln) 
# NB: IMPORTANT: use gcal---not fcal---here because model is _still_ 1.0 Jy 
print "--Polcal (D)--"
polcal(vis=msnameC,caltable='at166C.dcal',field='0420+417',refant='VA12',gaintable='at166C.gcal',gainfield='0420+417'); 
# --> 2/2 good solutions; 
# --> 0420: 0.035Jy @ -41.4deg / 0.033Jy @ 18.1deg 
 
#=====================================================================
# Examine solutions: 
print "--Plotcal--"
if scriptmode:
    plotcal(caltable='at166C.dcal',xaxis='antenna',yaxis='amp'); 
    user_check=raw_input('hit Return to continue script\n')
else:
    plotcal(caltable='at166C.dcal',xaxis='antenna',yaxis='amp',showgui=F,figfile='at166C.dcal.plotcal.png'); 

# You can plot imag vs. real also
# plotcal(caltable='at166C.dcal',xaxis='real',yaxis='imag',plotrange=[-0.05,0.05,-0.05,0.05]); 
 
#=====================================================================
# Set full polarization model for 0518+165 (pol is 11.1% @ -11 deg  [RL = -22]) 
#  NB: neglecting source structure here) 
print "--Setjy (X)--"
setjy(vis=msnameC,field='0518+165',spw='0',scalebychan=False,fluxdensity=[3.688, 0.380, -0.153, 0.0]); 
setjy(vis=msnameC,field='0518+165',spw='1',scalebychan=False,fluxdensity=[3.862, 0.397, -0.161, 0.0]); 
 
#=====================================================================
# Solve for polarization position angle on 0518+165 
print "--Polcal (X)--"
polcal(vis='at166C.ms',caltable='at166C.xcal',field='0518+165',refant='VA12',poltype='X',gaintable=['at166C.fcal', 'at166C.dcal'],gainfield=['0518+165','0420+417']); 
# --> 2/2 good solutions 
# --> 0518:  77.0deg  / -42.0deg  
 
#=====================================================================
# apply all calibration... 
#  (NB: different fields are selected from each caltable, depending on selected data fields) 
#  (NB: parang=T is set to rotate polarization p.a. frame from antennas to sky 
#...to 0420 & 3C129 
print "--Applycal--"
print "  apply calibration to 0420+417,3C129"
applycal(vis=msnameC,field='0420+417,3C129',gaintable=['at166C.fcal','at166C.dcal','at166C.xcal'],gainfield=['0420+417','0420+417','0518+165'],parang=T); 

print "  apply calibration to 0518+165"
applycal(vis=msnameC,field='0518+165',gaintable=['at166C.fcal','at166C.dcal','at166C.xcal'],gainfield=['0518+165','0420+417','0518+165'],parang=T); 
 
#=====================================================================
# Examine (edit?) calibrated data (calibrators) 
print "--Plotxy (corrected)--"
if scriptmode:
    plotxy(vis=msnameC,datacolumn='corrected',field='0420+417,0518+165',selectdata=T,correlation='RR,LL'); 
    user_check=raw_input('hit Return to continue script\n')
else:
    plotxy(vis=msnameC,datacolumn='corrected',field='0420+417,0518+165',selectdata=T,correlation='RR,LL',interactive=F,figfile='at166C.plotxy.final.png'); 

#--> Variations: plot on different axes (e.g., phase vs. time, real vs imag) 
#-->             overlay model data for calibrators 
#-->             plot 3C129 data 
# For example: 
# Examine cross-hand data (real vs. imag) 
# plotxy(vis=msnameC,xaxis='real',yaxis='imag',datacolumn='corrected',selectdata=True,correlation='RL,LR',plotrange=[-0.5,0.5,-0.5,0.5],field = '0518+165'); 
# plotxy(vis=msnameC,xaxis='real',yaxis='imag',datacolumn='corrected',selectdata=True,correlation='RL,LR',plotrange=[-0.5,0.5,-0.5,0.5],field = '0420+417'); 
 
#--> NB: RL and LR signal are complex conjugates of each other (Q+iU & Q-iU) 
 
#=====================================================================
# do some simple imaging of each source 
print "--Clean--"
# 3C129: 
# We will do a simple image-plane Hogbom clean with psfmode='hogbom' and imagermode=''
# This will clean the IQUV planes consecutively
#
# Non-interactive (no clean boxes) for now
#
clean(vis=msnameC,imagename='at166C.3c129',field='3C129',psfmode='hogbom',imagermode='',niter=2500,imsize=[2048,2048],cell=['0.3arcsec','0.3arcsec'],stokes='IQUV',weighting='briggs',robust=0.5,interactive=F); 

print "  Clean image is at166C.3c129.image"

# You can do a a Cotton-Schwab clean with psfmode='clark' and imagermode='csclean'
# You can try a threshold also.

# You can also clean the calibrators:
# 0518: 
# clean(vis=msnameC,imagename='at166C.0518',field='0518+165',psfmode='hogbom',niter=500,imsize=[512,512],cell=['0.3arcsec','0.3arcsec'],stokes='IQUV',weighting='briggs',robust=0.5,interactive=F); 
 
# 0420: 
# clean(vis=msnameC,imagename='at166C.0420',field='0420+417',psfmode='hogbom',niter=500,imsize=[512,512],cell=['0.3arcsec','0.3arcsec'],stokes='IQUV',weighting='briggs',robust=0.5,interactive=F); 
 
#=====================================================================
# Examine images in viewer... 
if scriptmode:
    print "--Viewer--"
    viewer('at166C.3c129.image')
    print "  Notice better representation of large-scale emission than in B-config"
    user_check=raw_input('hit Return to continue script\n')
    
#  Questions:  Do calibrator polarizations come out right? 
#              Is calibrator structure as expected? 
#              Is 3C129 image good?  (cf B-config imaging above) 
 
# Variations:  clean interactively (clean boxes) 
#              try different weighting  
#              in viewer, blink between B and C 3C129 images (in all polarizations) 
 
 
#=====================================================================
# IMAGING OF COMBINED B+C CONFIGURATIONS
#=====================================================================
print ""
print "Combining B and C config data..."
print ""

# The next steps extract the 3C129 data from the above datasets, and combine 
# them to permit a dual-config imaging run 
 
#=====================================================================
# split out 3C129
print "--Split (3C129)--"
split(vis=msnameB,outputvis='at166B.3C129.ms',field='3C129'); 
split(vis=msnameC,outputvis='at166C.3C129.ms',field='3C129'); 
 
#=====================================================================
# make one MS so we can image the combined config
print "--Concat (B+C config)--"
concat(vis=['at166B.3C129.ms','at166C.3C129.ms'],concatvis='3C129BC.ms'); 

#=====================================================================
print "--Listobs--"
listobs(vis='3C129BC.ms'); 
 
#=====================================================================
# Clean the image
print "--Clean--"
#
# You can do a simple image-plane Hogbom clean with psfmode='hogbom' and imagermode=''
# This will clean the IQUV planes consecutively
#
# We will do a Cotton-Schwab clean with psfmode='clark' and imagermode='csclean'
# This will do a joint IQUV deconvolution
# We will also set a threshold

if scriptmode:
    print "Use interactive clean to draw clean boxes or polygons on image"
    print "Increase npercycle as you clean deeper"
    clean(vis='3C129BC.ms',imagename='3C129BC.clean',psfmode='clark',imagermode='csclean',niter=50000,threshold='0.08mJy',imsize=[2048,2048],cell=['0.3arcsec','0.3arcsec'],stokes='IQUV',weighting='briggs',robust=0.5,interactive=T,npercycle=1000)
else:
    clean(vis='3C129BC.ms',imagename='3C129BC.clean',psfmode='clark',imagermode='csclean',niter=50000,threshold='0.16mJy',imsize=[2048,2048],cell=['0.3arcsec','0.3arcsec'],stokes='IQUV',weighting='briggs',robust=0.5,interactive=F);

# The non-interactive clean gives a S/N ratio around 850.
# If you do custom clean boxes and clean more deeply you should
# be able to reach a dynamic range over 1000.

print "  Clean image is 3C129BC.clean.image"

# Variation: you could set up clean boxes around emission, e.g.
# mask = [[992,988,1172,1139],[1029,1053,1212,1222],[1029,1081,1299,1222],[1065,1158,1397,1409],[1237,1153,1612,1583]]

#=====================================================================
# Examine image in viewer... 
if scriptmode:
    print "--Viewer--"
    viewer('3C129BC.clean.image')
 
#  Questions:  How has the 3C129 image changed?  Is it better? 
 
# Variations:  clean interactively (clean boxes) 
#              try different weighting  
#              split/concat/image dual-config calibrator data 
#              self-cal? 
 
#=====================================================================
# Extract I, Q, U, V images 
print "--Immath--"
immath(stokes='I', outfile='3C129BC.I', mode='evalexpr', expr='\'3C129BC.clean.image\''); 
immath(stokes='Q', outfile='3C129BC.Q', mode='evalexpr', expr='\'3C129BC.clean.image\''); 
immath(stokes='U', outfile='3C129BC.U', mode='evalexpr', expr='\'3C129BC.clean.image\''); 
immath(stokes='V', outfile='3C129BC.V', mode='evalexpr', expr='\'3C129BC.clean.image\''); 

# Form poln intensity and pos ang 
immath(stokes='', outfile='3C129BC.P', mode='poli', imagename=['3C129BC.Q','3C129BC.U'], sigma='0.0mJy/beam'); 
immath(stokes='', outfile='3C129BC.X', mode='pola', imagename=['3C129BC.Q','3C129BC.U'], sigma='0.0mJy/beam'); 


#=====================================================================
# Extract center of I image for testing

ia.open('3C129BC.I')
ia.subimage(outfile='3C129BC.core.I',region=rg.box([1010,1040,0,0],[1025,1055,0,0]))
ia.close()
 
#=====================================================================
# Complex Linear Polarization 
print "--ComplexLinPol (toolkit)--"

# NOTE: The viewer can take complex images to make Vector plots, although
# the image analysis tasks (and ia tool) cannot yet handle these.  But we
# can use the imagepol tool (which is not imported by default) to make
# a complex image of the linear polarized intensity for display.
# See CASA User Reference Manual:
# http://casa.nrao.edu/docs/casaref/imagepol-Tool.html
#
# Make an imagepol tool and open the clean image 
po = casac.imagepol()
po.open('3C129BC.clean.image')
# Use complexlinpol to make a Q+iU image
po.complexlinpol('3C129BC.cmplxlinpol')
po.close()

# You can now display this in the viewer, in particular overlay this
# over the intensity raster with the poli contours.  The vector lengths
# will be proportional to the polarized intensity.  You can play with
# the Data Display Options panel for vector spacing and length.
# You will want to have this masked, like the pola image above, on
# the polarized intensity.  When you load the image, use the LEL:
# '3C129BC.cmplxlinpol'['3C129BC.P'>0.0001]

#=====================================================================
# View results
# In viewer 'Load Data' window, use the following LEL expression to load the p.a. image: 
print "--Viewer--"
if scriptmode:
    viewer('3C129BC.I')
else:
    print "  For viewer: "

print "  In Load Data' window LEL box, use the following: "
print "    '3C129BC.X'['3C129BC.P'>0.00015] "
print "  or" 
print "    '3C129BC.cmplxlinpol'['3C129BC.P'>0.0001] "

if scriptmode:
    user_check=raw_input('hit Return to continue script\n')
  
#=====================================================================
# RESULTS
#=====================================================================
# Calculate image statistics
# Outer source-free region 1100,100,1950,750
outerbox = '1100,100,1950,750'
# Inner source-free region 1315,940,1532,1143  (in clean bowl area)
innerbox = '1315,940,1532,1143'
print "--Imstat--"

# Stats on IPOL
ipolstat = imstat('3C129BC.clean.image',stokes='I')
ipolstat_offsrc = imstat('3C129BC.clean.image',stokes='I',box=outerbox)

print '  %40s : %12.7f ' % ('3C129 Combined  I maximum (Jy)', ipolstat['max'][0])
print '  %40s : %12.7f ' % ('3C129 Combined  I off-source rms (Jy)', ipolstat_offsrc['sigma'][0])
print '  %40s : %12.3f ' % ('3C129 Combined  I dynamic range',
                            ipolstat['max'][0]/ipolstat_offsrc['sigma'][0])

# Stats on QPOL,UPOL
qupolstat = imstat('3C129BC.clean.image',stokes='QU')
qupolstat_offsrc = imstat('3C129BC.clean.image',stokes='QU',box=outerbox)
quabsmax = max( qupolstat['max'][0], -qupolstat['min'][0] )

print ''
print '  %40s : %12.7f ' % ('3C129 Combined QU maximum (Jy)', quabsmax)
print '  %40s : %12.7f ' % ('3C129 Combined QU off-source rms (Jy)', qupolstat_offsrc['sigma'][0])
print '  %40s : %12.3f ' % ('3C129 Combined QU dynamic range',
                            quabsmax/qupolstat_offsrc['sigma'][0])

# Stats on entire V image
vpolstat = imstat('3C129BC.clean.image',stokes='V')

print ''
print '  %40s : %12.7f ' % ('3C129 Combined VPOL image rms (Jy)', vpolstat['sigma'][0])

#=====================================================================
# DONE
#=====================================================================
if benchmarking:
    endProc=time.clock()
    endTime=time.time()

print ""
print "Done with 3C129 Tutorial"

if benchmarking:
    walltime = (endTime - startTime)
    cputime = (endProc - startProc)
    
    print ''
    print 'Regression PASSED'
    print ''
    print 'Total wall clock time was: %10.3f ' % walltime
    print 'Total CPU        time was: %10.3f ' % cputime
    print ''

#
##########################################################################
