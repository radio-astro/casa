
##########################################################################
#                                                                        #
# Demo Script for NGC 4826 (BIMA line data)                              #
#                                                                        #
# Converted by  STM 2008-05-27 (Beta Patch 2.0) new tasking/clean/cal    #
# Updated by     CB 2008-05-30                  start from raw data      #
# Updated by    STM 2008-06-01                  scriptmode, plotting     #
# Updated by CB,STM 2008-06-02                  improvements             #
# Updated by CB,STM 2008-06-03                  bigger cube, pbcor       #
# Updated by CB,STM 2008-06-04                  pbcor stuff              #
# Updated by CB,STM 2008-06-04                  tutorial script          #
# Updated by CB     2008-06-05                  small tweaks             #
# Updated by CB,STM 2008-06-12                  final revisions (DS)     #
# Updated by STM    2008-06-14                  post-school fix          #
#                                                                        #
# N4826 - BIMA SONG Data                                                 #
#                                                                        #
# This data is from the BIMA Survey of Nearby Galaxies (BIMA SONG)       #
# Helfer, Thornley, Regan, et al., 2003, ApJS, 145, 259                  #
# Many thanks to Michele Thornley for providing the data and description #
#                                                                        #
# First day of observations only                                         #
# Trial for summer school                                                #
#                                                                        #
# Script Notes:                                                          #
#    o The "default" commands are not necessary, but are included        #
#      in case we want to change from function calls to globals          #
#    o This script has some interactive commands, such as with plotxy    #
#      and the viewer.  This script will stop and require a              #
#      carriage-return to continue at these points.                      #
#    o Sometimes cut-and-paste of a series of lines from this script     #
#      into the casapy terminal will get garbled (usually a single       #
#      dropped character). In this case, try fewer lines, like groups    #
#      of 4-6.                                                           #
#                                                                        #
##########################################################################
import os

# Some diagnostic stuff
pl.ion()
pl.clf()

#
##########################################################################
#                                                                        
# Clear out previous run results
os.system('rm -rf ngc4826.tutorial.*')

# Sets a shorthand for the ms, not necessary
prefix='ngc4826.tutorial'
msfile = prefix + '.16apr98.ms'

print 'Tutorial Script for BIMASONG NGC4826 Mosaic'
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
       freqtol="",dirtol="1arcsec",async=False)

#
##########################################################################
#
# TUTORIAL NOTES:
#
# You can invoke tasks in two ways:
#
# (1) As function calls with arguments as shown above for concat and used
#     extensively in this script, e.g.
#
#        task( par1=val1, par2=val2, ... )
#
#     with parameters set as arguments in the call.  Note that in this
#     case, the global parameter values are NOT used or changed, and any
#     task parameters that are not specified as arguments to the call
#     will be defaulted to the task-specific default values (see the
#     "help task" description).
#
# (2) By setting the values of the global parameters and then using the
#     "go" command (if taskname is set) or calling the task with no
#     arguments.  For example:
#
#        default task
#        par1 = val1
#        par2 = val2
#        ...
#        inp
#        task()
#
#     In this case, the "default" command sets the parmeters to their
#     task defaults, and sets the "taskname" paramter to the task to be
#     run.  The "inp" command displays the current values for the task
#     parameters.  Then the call with no arguments runs with the globals.
#
#     Warning: "go" does not work inside scripts. See Cookbook.
#
# Using the concat call above as an example, we would do:
#
#default('concat')
#
#vis = ['ngc4826.tutorial.3c273.5.ms',
#       'ngc4826.tutorial.3c273.6.ms',
#       'ngc4826.tutorial.3c273.7.ms',
#       'ngc4826.tutorial.3c273.8.ms',
#       'ngc4826.tutorial.1310+323.ll.9.ms',
#       'ngc4826.tutorial.1310+323.ll.10.ms',
#       'ngc4826.tutorial.1310+323.ll.11.ms',
#       'ngc4826.tutorial.1310+323.ll.12.ms',
#       'ngc4826.tutorial.1310+323.ll.13.ms',
#       'ngc4826.tutorial.1310+323.ll.14.ms',
#       'ngc4826.tutorial.1310+323.ll.15.ms',
#       'ngc4826.tutorial.1310+323.ll.16.ms',	    
#       'ngc4826.tutorial.ngc4826.ll.5.ms',
#       'ngc4826.tutorial.ngc4826.ll.6.ms',
#       'ngc4826.tutorial.ngc4826.ll.7.ms',
#       'ngc4826.tutorial.ngc4826.ll.8.ms']
#
#concatvis='ngc4826.tutorial.ms'
#freqtol = ""
#dirtol = "1arcsec"
#async=False
#
#concat()

#
##########################################################################
#
# Fix up the MS (temporary, changes to importfits underway)
#
print '--Fixing up spw rest frequencies in MS--'
vis='ngc4826.tutorial.ms'
tb.open(vis+'/SOURCE',nomodify=false)
spwid=tb.getcol('SPECTRAL_WINDOW_ID')
spwid.setfield(-1,int)
tb.putcol('SPECTRAL_WINDOW_ID',spwid)
tb.close()

# This ensures that the rest freq will be found for all spws. 

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

# Should see the listing included at the end of this script
#

print "There are 3 fields observed in a total of 16 spectral windows"
print "   field=0    3c273    spwids 0,1,2,3               64 chans "
print "   field=1    1310+323 spwids 4,5,6,7,8,9,10,11     32 chans "
print "   field=2~8  NGC4826  spwids 12,13,14,15           64 chans "
print ""
print "See listobs summary in logger"
#
##########################################################################
# Plotting and Flagging
##########################################################################
#
# The plotxy task is the interactive x-y display and flagging GUI
#
print '--Plotxy--'
default(plotxy)

# Here we will suggest things to plot, and actually only do a few
# (near the end of this task block).  If you like you can
# uncomment these when you run this script
#
# First look at amplitude as a funciton of uv-distance using an
# average over all channels for each source.
#
# Interactive plotxy
#plotxy(vis='ngc4826.tutorial.ms',xaxis='uvdist',yaxis='amp',field='0',spw='0~3',
#       averagemode='vector',width='1000',
#       selectplot=True,title='Field 0 SPW 0~3')
#	
# Pause script if you are running in scriptmode
#user_check=raw_input('Return to continue script\n')

# NOTE: width here needs to be larger than combination of all channels
# selected with spw and/or field. Since field and spw are unique in this
# case, both don't need to be specified, however plotting is much faster
# if you "help it" by selecting both.
#
# Now average over all times across scans but not over channel and
# plot versus channel. There are four 64-channel spws set end-to-end
# by plotxy You should see bad edge channels in each spw segment We
# will flag these (non-interactively) later
#
# Interactive plotxy
#plotxy(vis='ngc4826.tutorial.ms',xaxis='channel',yaxis='amp',field='0',spw='0~3',
#       averagemode='vector',timebin='1e7',crossscans=True,
#       selectplot=True,newplot=False,title='Field 0 SPW 0~3')
#
# Pause script if you are running in scriptmode
#user_check=raw_input('Return to continue script\n')

# You can also plot versus velocity by setting xaxis='velocity'
#
# You might do this for all the other spw/field combos
#

# You can also non-interactively plotxy to a file
# (note this works only if iteration is not set)
# Also, see how we use Python variables to make this easier
#
#field = '0'
#spw = '0~3'
#figfile = 'ngc4826.tutorial.ms' + '.plotxy.'+field+'.spectrum.raw.png'
#plotxy(vis='ngc4826.tutorial.ms',xaxis='velocity',yaxis='amp',field=field,spw=spw,
#       averagemode='vector',timebin='1e7',crossscans=True,
#       selectplot=True,newplot=False,title='Field '+field+' SPW '+spw,
#       interactive=False,figfile=figfile)

# Now lets look at the target source, the first of the NGC4826 mosaic fields
# which are 2~8 in this MS.
#
# Since we are plotting versus velocity we can clearly see the bad edge
# channels and the overlap between spw
#
# There is nothing terribly wrong with this data and again we will flag the
# edge channels non-interactively later for consistency.
#
# Normally, if there were obviously bad data, you would flag it here
# before calibration.  To do this, hit the Mark Region button, then draw a box around
# some of the moderately high outliers, and then Flag.
#
# But this data is relatively clean, and flagging will not improve results.
#
# Interactive plotxy
plotxy(vis='ngc4826.tutorial.ms',xaxis='velocity',yaxis='amp',field='2',spw='12~15',
       averagemode='vector',timebin='1e7',crossscans=True,
       selectplot=True,newplot=False,title='Field 2 SPW 12~15')

print "You could Mark Region around outliers and Flag"
# Pause script if you are running in scriptmode
user_check=raw_input('Return to continue script\n')

#
# You could set up a Python loop to do all the N4826 fields, like this:
#
#for fld in range(2,9):
#	field = str(fld)
#	plotxy(vis='ngc4826.tutorial.ms',xaxis='velocity',yaxis='amp',field=field,spw=spw,
#	       averagemode='vector',timebin='1e7',crossscans=True,
#	       selectplot=True,newplot=False,title='Field '+field+' SPW '+spw)
#
#	print "Nominally, Mark Region around outliers and Flag"
#	# Pause script if you are running in scriptmode
#	user_check=raw_input('Return to continue script\n')

# Back to first field.
# You can also have it iterate over baselines, using Next to advance
# Ignore baselines 1:1, 2:2 etc. as they would correspond to autocorrelations
# if they were present (they are not in this dataset)
#
# Interactive plotxy
#plotxy(vis='ngc4826.tutorial.ms',xaxis='channel',yaxis='amp',field='0',spw='0~3',
#       averagemode='vector',timebin='1e7',crossscans=True,
#       iteration='baseline',
#       selectplot=True,newplot=False,title='Field 0 SPW 0~3')
#	
# Pause script if you are running in scriptmode
#user_check=raw_input('Return to continue script\n')

#
# Finally, look for bad data. Here we look at field 8 w/o averaging
plotxy(vis='ngc4826.tutorial.ms',xaxis='time',yaxis='amp',field='8',spw='12~15',
       selectplot=True,newplot=False,title='Field 8 SPW 12~15')

print "You can see some bad data here"
print "Mark Region and Locate, look in logger"
print "This is a correlator glitch in baseline 3-9 at 06:19:30"
print "You could Mark Region and Flag but we will use flagdata instead"
# Pause script if you are running in scriptmode
user_check=raw_input('Return to continue script\n')

# If you change xaxis='channel' you see its all channels
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
##########################################################################
#
# Some example clean-up editing
# Slightly high almost-edge channel in field='1', spw='4' (channel 2)
# can be flagged interactively with plotxy.
#
#plotxy(vis='ngc4826.tutorial.ms',
#       xaxis='channel',yaxis='amp',field='1',spw='4',
#       averagemode='vector',timebin='1e7',crossscans=True,
#       selectplot=True,newplot=False,title='Field 1 SPW 4')

print "Completed pre-calibration flagging"

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
#flagmanager(vis='ngc4826.tutorial.ms',mode='list')

#
##########################################################################
#
# CALIBRATION
#
##########################################################################
#
# Bandpasses are very flat because of observing mode used (online bandpass
# correction) so bandpass calibration is unecessary for these data.
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
#
##########################################################################
#
# Plot calibration
print '--Plotcal (fluxscale)--'
default(plotcal)

# Interactive plotcal
plotcal(caltable='ngc4826.tutorial.16apr98.fcal', yaxis='amp', field='')
print ''
print 'Plotting final scaled gain calibration table'
print 'First amp vs. time for all fields '
	
# Pause script if you are running in scriptmode
user_check=raw_input('Return to continue script\n')

plotcal(caltable='ngc4826.tutorial.16apr98.fcal', yaxis='phase', field='')
print ''
print 'and phase vs. time '

# Pause script if you are running in scriptmode
user_check=raw_input('Return to continue script\n')

# And you can plot the SNR of the solution
#plotcal(caltable='ngc4826.tutorial.16apr98.fcal', yaxis='snr', field='')

# You can also plotcal to file
#figfile = 'ngc4826.tutorial.16apr98.fcal.plotcal.amp.png'
#plotcal(caltable='ngc4826.tutorial.16apr98.fcal',yaxis='amp',field='',showgui=False,figfile=figfile)
#figfile = 'ngc4826.tutorial.16apr98.fcal.plotcal.phase.png'
#plotcal(caltable='ngc4826.tutorial.16apr98.fcal',yaxis='phase',field='',showgui=False,figfile=figfile)

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

#
##########################################################################
#
# Check calibrated data
print '--Plotxy--'
default(plotxy)

#
# Here we plot the first of the NGC4826 fields unaveraged versus velocity
# Notice how the spw fit together

# Interactive plotxy
#print "Will plot all the NGC4826 calibrated data unaveraged - this will take a while"
#plotxy(vis='ngc4826.tutorial.ms',xaxis='velocity',yaxis='amp',field='2~8',spw='12~15',
#       averagemode='vector',datacolumn='corrected',
#       selectplot=True,newplot=False,title='Field 2~8 SPW 12~15')

#print ""
#print "Look for outliers, flag them if there are any bad ones"
#print ""
	
# Pause script if you are running in scriptmode
#user_check=raw_input('Return to continue script\n')

# You can also plot all the N4826 fields 2 through 8, for example using a loop:

#for fld in range(2,9):
#	field = str(fld)
#	plotxy(vis='ngc4826.tutorial.ms',xaxis='velocity',yaxis='amp',
#              field=field,spw='11~15',
#	       averagemode='vector',datacolumn='corrected',
#	       selectplot=True,newplot=False,title='Field '+field+' SPW 11~15')
#	
#	user_check=raw_input('Return to continue script\n')

# Now here we time-average the data, plotting versus velocity

#plotxy(vis='ngc4826.tutorial.ms',xaxis='velocity',yaxis='amp',field=field,spw=spw,
#       averagemode='vector',datacolumn='corrected',
#       timebin='1e7',crossscans=True,plotcolor='blue',
#       selectplot=True,newplot=False,title='Field '+field+' SPW '+spw)
#print ""
#print 'Final Spectrum field '+field+' spw '+spw+' TimeAverage Corrected Data'
	
# Pause script if you are running in scriptmode
#user_check=raw_input('Return to continue script\n')

# Here we overplot 3C273 the Time+Chan averaged calibrated and uncalibrated data

#
# First the corrected column in blue
#field = '0'
#spw = '0~3'
#plotxy(vis='ngc4826.tutorial.ms',xaxis='uvdist',yaxis='amp',field=field,spw=spw,
#       averagemode='vector',width='1000',datacolumn='corrected',
#       timebin='1e7',crossscans=True,plotcolor='blue',
#       selectplot=True,newplot=False,title='Field '+field+' SPW '+spw)
#print ""
#print 'Plotting field '+field+' spw '+spw+' TimeChanAverage Corrected Data in blue'
#
# Now the original data column in red
#plotxy(vis='ngc4826.tutorial.ms',xaxis='uvdist',yaxis='amp',field=field,spw=spw,
#       averagemode='vector',width='1000',datacolumn='data',
#       timebin='1e7',crossscans=True,plotcolor='red',overplot=True,
#       selectplot=True,newplot=False,title='Field '+field+' SPW '+spw)
#
#print 'OverPlotting field '+field+' spw '+spw+' TimeChanAverage Original Data in red'
	
## Pause script if you are running in scriptmode
#user_check=raw_input('Return to continue script\n')

# Can repeat for
#field = '1'
#spw = '4~11'

print "Done calibration and plotting"
#
##########################################################################
#
# SPLIT THE DATA INTO SINGLE-SOURCE MS
# AND THEN IMAGE THE CALIBRATOR
#
##########################################################################
#
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

#
##########################################################################
#
#print '--Clearcal (split data)--'

# If you want to use plotxy before cleaning to look at the split ms
# then you will need to force the creation of the scratch columns
# (clean will also do this)
#clearcal('ngc4826.tutorial.16apr98.1310+323.split.ms')

#clearcal('ngc4826.tutorial.16apr98.src.split.ms')

#Then you might look at the data
#clearplot()
#plotxy(vis='ngc4826.tutorial.16apr98.src.split.ms',xaxis='time',yaxis='amp')

#
##########################################################################
#
# You might image the calibrater data:
#
#print '--Clean (1310+323)--'
#default('clean')
#
#
#clean(vis='ngc4826.tutorial.16apr98.1310+323.split.ms', 
#      imagename='ngc4826.tutorial.16apr98.cal.clean',
#      cell=[1.,1.],imsize=[256,256],
#      field='0',spw='0~7',threshold=10.,
#      mode='mfs',psfmode='clark',niter=100,stokes='I')

# You can look at this in the viewer
#viewer('ngc4826.tutorial.16apr98.cal.clean.image')

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

clean(vis='ngc4826.tutorial.16apr98.src.split.ms',
      imagename='ngc4826.tutorial.16apr98.src.clean',
      field='0~6',spw='0~3',
      cell=[1.,1.],imsize=[256,256],stokes='I',
      mode='channel',nchan=36,start=35,width=4,
      psfmode='clark',imagermode='mosaic',scaletype='SAULT',
      cyclefactor=1.5,niter=10000,threshold='45mJy',
      minpb=0.3,pbcor=False)

### NOTE: mosaic data ...Sault weighting implies a noise unform image

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
# Do interactive viewing of clean image
print '--Viewer--'
viewer('ngc4826.tutorial.16apr98.src.clean.image')

print ""
print "This is the non-pbcorrected cube of NGC4826"
print "Use tape deck to move through channels"
print "Close the viewer when done"
print ""
#
# Pause script if you are running in scriptmode
user_check=raw_input('Return to continue script\n')

#
##########################################################################
#
# Statistics on clean image cube
#
print '--ImStat (Clean cube)--'

srcstat = imstat('ngc4826.tutorial.16apr98.src.clean.image')

print "Found image max = "+str(srcstat['max'][0])

# offbox = '106,161,153,200'

offstat = imstat('ngc4826.tutorial.16apr98.src.clean.image',
                 box='106,161,153,200')

print "Found off-source image rms = "+str(offstat['sigma'][0])

# cenbox = '108,108,148,148'
# offlinechan = '0,1,2,3,4,5,30,31,32,33,34,35'

offlinestat = imstat('ngc4826.tutorial.16apr98.src.clean.image',
                     box='108,108,148,148',
                     chans='0,1,2,3,4,5,30,31,32,33,34,35')

print "Found off-line image rms = "+str(offlinestat['sigma'][0])

#
##########################################################################
#
# Manually correct for mosaic response pattern using .image/.flux images
#
print '--ImMath (PBcor)--'

immath(outfile='ngc4826.tutorial.16apr98.src.clean.pbcor',
       mode='evalexpr',
       expr="'ngc4826.tutorial.16apr98.src.clean.image'/'ngc4826.tutorial.16apr98.src.clean.flux'")

#
##########################################################################
#
# Statistics on PBcor image cube
#
print '--ImStat (PBcor cube)--'

pbcorstat = imstat('ngc4826.tutorial.16apr98.src.clean.pbcor')

print "Found image max = "+str(pbcorstat['max'][0])

pbcoroffstat = imstat('ngc4826.tutorial.16apr98.src.clean.pbcor',
                      box='106,161,153,200')

print "Found off-source image rms = "+str(pbcoroffstat['sigma'][0])

pbcorofflinestat = imstat('ngc4826.tutorial.16apr98.src.clean.pbcor',
                          box='108,108,148,148',
                          chans='0,1,2,3,4,5,30,31,32,33,34,35')

print "Found off-line image rms = "+str(pbcorofflinestat['sigma'][0])

#
##########################################################################
#
# Do zeroth and first moments
#
# NGC4826 LSR velocity is 408 km/s; delta is 20 km/s
#
print '--ImMoments--'
default('immoments')

momfile = 'ngc4826.tutorial.16apr98.moments'
momzeroimage = 'ngc4826.tutorial.16apr98.moments.integrated'
momoneimage = 'ngc4826.tutorial.16apr98.moments.mom1'

print "Calculating Moments 0,1 for PBcor image"

immoments(imagename='ngc4826.tutorial.16apr98.src.clean.pbcor',
	  moments=0,axis=3,
	  planes='7~28',
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
	  moments=1,axis=3,includepix=[0.2,1000.0],
	  planes='7~28',
          outfile='ngc4826.tutorial.16apr98.moments.mom1') 

# Now view the resulting images
viewer('ngc4826.tutorial.16apr98.moments.integrated')
#
print "Now viewing Moment-0 ngc4826.tutorial.16apr98.moments.integrated"
print "Note PBCOR effects at field edge"
print "Change the colorscale to get better image"
print "You can also Open and overlay Contours of Moment-1 ngc4826.tutorial.16apr98.moments.mom1"
print "Close the viewer when done"
#
# Pause script if you are running in scriptmode
user_check=raw_input('Return to continue script\n')

#
##########################################################################
#
# Statistics on moment images
#
print '--ImStat (Moment images)--'

momzerostat=imstat('ngc4826.tutorial.16apr98.moments.integrated')

print "Found moment 0 max = "+str(momzerostat['max'][0])

print "Found moment 0 rms = "+str(momzerostat['rms'][0])

momonestat=imstat('ngc4826.tutorial.16apr98.moments.mom1')

print "Found moment 1 median = "+str(momonestat['median'][0])

#
##########################################################################
#
# An alternative is to mask the pbcor image before calculating
# moments.  The following block shows how to do this.

# To do this, open the clean pbcor file in the viewer and use the
# Region Manager to create a region file

#print '--Viewer--'
#viewer(ngc4826.tutorial.16apr98.src.clean.pbcor)

# TUTORIAL NOTES: After loading ngc4826.tutorial.16apr98.src.clean.pbcor
# in the viewer as a raster, click on the file icon in top left
# corner, and select the momzero image but open as a Contour map
# instead of Raster. Then decrease "contour scale factor" in "Data
# Display Options" gui to something like 10. select "region manager
# tool" from "tool" drop down menu In region manager tool select "all
# axes". Then assign the sqiggly Polygon button to a mouse button by
# clicking on it with a mouse button. Then draw a polygon region
# around galaxy emission, avoiding edge regions. Then in "region
# manager tool" select "save last region".

# Pause script if you are running in scriptmode
#user_check=raw_input('Return to continue script\n')

# You should have created region file ngc4826.tutorial.16apr98.src.clean.pbcor.rgn

# Now use immath to use the region file to mask the cube
#
#print '--ImMath (masking)--'
#immath(outfile='ngc4826.tutorial.16apr98.src.clean.pbcor.masked',
#       mode='evalexpr',
#       expr="'ngc4826.tutorial.16apr98.src.clean.pbcor'",
#       region='ngc4826.tutorial.16apr98.src.clean.pbcor.rgn')

#
# And then make masked moment images

#print '--ImMoments (masked)--'
#print 'Creating masked moment 0 image ngc4826.tutorial.16apr98.moments.integratedmasked'
#	
#immoments(imagename='ngc4826.tutorial.16apr98.src.clean.pbcor.masked',
#          moments=0,axis=3,
#	   planes='7~28',
#	   outfile='ngc4826.tutorial.16apr98.moments.integratedmasked') 
#
#print 'Creating masked moment 1 image ngc4826.tutorial.16apr98.moments.mom1masked'
#
#immoments(imagename='ngc4826.tutorial.16apr98.src.clean.pbcor.masked',
#          moments=1,axis=3,
#	   includepix=[0.2,1000.0],
#	   planes='7~28',
#	   outfile='ngc4826.tutorial.16apr98.moments.mom1masked') 

# Now view the resulting images
#viewer('ngc4826.tutorial.16apr98.moments.integratedmasked')
#
#print "Now viewing masked Moment-0 ngc4826.tutorial.16apr98.moments.integratedmasked"
#print "You can Open and overlay Contours of Moment-1 ngc4826.tutorial.16apr98.moments.mom1masked"
#
# Pause script if you are running in scriptmode
#user_check=raw_input('Return to continue script\n')

# Finally, can compute and print statistics
#print '--ImStat (masked moments)--'
#
#maskedmomzerostat = imstat('ngc4826.tutorial.16apr98.moments.integratedmasked')
#print "Found masked moment 0 max = "+str(maskedmomzerostat['max'][0])
#print "Found masked moment 0 rms = "+str(maskedmomzerostat['rms'][0])
#
#maskedmomonestat=imstat('ngc4826.tutorial.16apr98.moments.mom1masked')
#print "Found masked moment 1 median = "+str(maskedmomonestat['median'][0])

#
##########################################################################
#
# Now show how to print out results
#
print '--Results (16apr98)--'
print ''
#
# Currently using non-PBcor values
#
im_srcmax16 = srcstat['max'][0]
im_offrms16 = offstat['sigma'][0]
im_offlinerms16 = offlinestat['sigma'][0]
thistest_immax = momzerostat['max'][0]
thistest_imrms = momzerostat['rms'][0]

#
# Report a few key stats
#
print '  NGC4826 Image Cube Max = '+str(im_srcmax16)
print "          At ("+str(srcstat['maxpos'][0])+","+str(srcstat['maxpos'][1])+") Channel "+str(srcstat['maxpos'][3])
print '          '+srcstat['maxposf']
print ''
print '          Off-Source Rms = '+str(im_offrms16)
print '          Signal-to-Noise ratio = '+str(im_srcmax16/im_offrms16)
print ''
print '          Off-Line   Rms = '+str(im_offlinerms16)
print '          Signal-to-Noise ratio = '+str(im_srcmax16/im_offlinerms16)

# Note previous regression values (using this script STM 2008-06-04) were:
#srcmax16=1.45868253708
#immax=169.420959473
#imrms=14.3375244141
#offrms16=0.0438643493782
#offlinerms16=0.0544108718199

# Could print out comparison:
#print ''
#print '--Src image max (16apr98): '+str(im_srcmax16)+' was '+str(srcmax16)
#print '--Off-src   rms (16apr98): '+str(im_offrms16)+' was '+str(offrms16)
#print '--Off-line  rms (16apr98): '+str(im_offlinerms16)+' was '+str(offlinerms16)
#print '--Moment  0 max (16apr98): '+str(thistest_immax)+' was '+str(immax)
#print '--Moment  0 rms (16apr98): '+str(thistest_imrms)+' was '+str(imrms)
#print ''

print "Done with NGC4826 Tutorial"
#
##########################################################################
