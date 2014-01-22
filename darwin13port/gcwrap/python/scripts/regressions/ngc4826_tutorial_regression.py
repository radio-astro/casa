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
# Updated by STM    2008-06-17                  scriptmode for regress   #
# Updated by STM    2008-06-19                  regression dictionaries  #
# Updated by STM    2008-06-30                  add channel check        #
# Updated by STM    2008-07-08                  bug fixes                #
# Updated by STM    2008-10-06                  Patch 3 regression       #
# Updated by STM    2008-10-16                  Patch 3 immoments        #
# Updated by STM    2008-10-22                  testing version          #
# Updated by STM    2008-11-03                  minor updates            #
# Updated by STM    2008-11-19                  Patch 3 400x400 image    #
# Updated by STM    2008-12-01                  Patch 3 release          #
# Updated by STM    2008-06-03                  Patch 4 vals, fix error  #
# Updated by STM    2009-06-19                  Patch 4 release          #
# Updated by STM    2009-12-07                  Release 3.0 final        #
# Updated by GAM    2013-05-15 Removed redundant gaincurve/opacity refs  #
#                                                                        #
# N4826 - BIMA SONG Data                                                 #
#                                                                        #
# This data is from the BIMA Survey of Nearby Galaxies (BIMA SONG)       #
# Helfer, Thornley, Regan, et al., 2003, ApJS, 145, 259                  #
# Many thanks to Michele Thornley for providing the data and description #
#                                                                        #
# First day of observations only                                         #
# Trial for summer school                                                #
# NOTE: REGRESSION VERSION FOR CASA TESTING                              #
#                                                                        #
# Script Notes:                                                          #
#    o This script has some interactive commands, such as with plotxy    #
#      and the viewer.  If scriptmode=True, then this script will stop   #
#      and require a carriage-return to continue at these points.        #
#    o Sometimes cut-and-paste of a series of lines from this script     #
#      into the casapy terminal will get garbled (usually a single       #
#      dropped character). In this case, try fewer lines, like groups    #
#      of 4-6.                                                           #
#    o The results are written out as a dictionary in a pickle file      #
#         out.ngc4826.tutorial.regression.<datestring>.pickle            #
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

#scriptmode = True

# If you want to run like a regression, including making PNG for plots,
# then set to False
scriptmode = False

# Enable benchmarking?
benchmarking = True

# Sets a shorthand for fixed input script/regression files
scriptprefix='ngc4826_tutorial_regression'

#
##########################################################################
#                                                                        
# Clear out previous run results
os.system('rm -rf ngc4826.tutorial.*')

# Sets a shorthand for the ms, not necessary
prefix='ngc4826.tutorial'
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
if benchmarking:
    startTime=time.time()
    startProc=time.clock()

_mydatapath = os.environ.get('CASAPATH').split()[0]+'/data/regression/ngc4826/'

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

importuvfits(fitsfile=_mydatapath+'fitsfiles/3c273.fits5', vis='ngc4826.tutorial.3c273.5.ms')

importuvfits(fitsfile=_mydatapath+'fitsfiles/3c273.fits6', vis='ngc4826.tutorial.3c273.6.ms')

importuvfits(fitsfile=_mydatapath+'fitsfiles/3c273.fits7', vis='ngc4826.tutorial.3c273.7.ms')

importuvfits(fitsfile=_mydatapath+'fitsfiles/3c273.fits8', vis='ngc4826.tutorial.3c273.8.ms')

importuvfits(fitsfile=_mydatapath+'fitsfiles/1310+323.ll.fits9', vis='ngc4826.tutorial.1310+323.ll.9.ms')

importuvfits(fitsfile=_mydatapath+'fitsfiles/1310+323.ll.fits10', vis='ngc4826.tutorial.1310+323.ll.10.ms')

importuvfits(fitsfile=_mydatapath+'fitsfiles/1310+323.ll.fits11', vis='ngc4826.tutorial.1310+323.ll.11.ms')

importuvfits(fitsfile=_mydatapath+'fitsfiles/1310+323.ll.fits12', vis='ngc4826.tutorial.1310+323.ll.12.ms')

importuvfits(fitsfile=_mydatapath+'fitsfiles/1310+323.ll.fits13', vis='ngc4826.tutorial.1310+323.ll.13.ms')

importuvfits(fitsfile=_mydatapath+'fitsfiles/1310+323.ll.fits14', vis='ngc4826.tutorial.1310+323.ll.14.ms')

importuvfits(fitsfile=_mydatapath+'fitsfiles/1310+323.ll.fits15', vis='ngc4826.tutorial.1310+323.ll.15.ms')

importuvfits(fitsfile=_mydatapath+'fitsfiles/1310+323.ll.fits16', vis='ngc4826.tutorial.1310+323.ll.16.ms')

importuvfits(fitsfile=_mydatapath+'fitsfiles/ngc4826.ll.fits5', vis='ngc4826.tutorial.ngc4826.ll.5.ms')

importuvfits(fitsfile=_mydatapath+'fitsfiles/ngc4826.ll.fits6', vis='ngc4826.tutorial.ngc4826.ll.6.ms')

importuvfits(fitsfile=_mydatapath+'fitsfiles/ngc4826.ll.fits7', vis='ngc4826.tutorial.ngc4826.ll.7.ms')

importuvfits(fitsfile=_mydatapath+'fitsfiles/ngc4826.ll.fits8', vis='ngc4826.tutorial.ngc4826.ll.8.ms')

if benchmarking:
    import2time=time.time()

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

if benchmarking:
    concat2time=time.time()

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
#spwid.setfield(-1,int)
# Had to do this for 64bit systems 08-Jul-2008
spwid.setfield(-1,'int32')
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
clearcal(vis='ngc4826.tutorial.ms', addmodel=False)

if benchmarking:
    clearcal2time=time.time()

#
##########################################################################
#
# List contents of MS
#
print '--Listobs--'
listobs(vis='ngc4826.tutorial.ms')

# Should see the listing in logger
# Some parts shown below
#

print "There are 3 fields observed in a total of 16 spectral windows"
print "   field=0    3c273    spwids 0,1,2,3               64 chans "
print "   field=1    1310+323 spwids 4,5,6,7,8,9,10,11     32 chans "
print "   field=2~8  NGC4826  spwids 12,13,14,15           64 chans "
print ""
print "See listobs summary in logger"

if benchmarking:
    list2time=time.time()

# Fields: 9
# ID   Code Name          Right Ascension  Declination   Epoch   
# 0         3C273         12:29:06.70      +02.03.08.60  J2000   
# 1         1310+323      13:10:28.66      +32.20.43.78  J2000   
# 2         NGC4826       12:56:44.24      +21.41.05.10  J2000   
# 3         NGC4826       12:56:41.08      +21.41.05.10  J2000   
# 4         NGC4826       12:56:42.66      +21.41.43.20  J2000   
# 5         NGC4826       12:56:45.82      +21.41.43.20  J2000   
# 6         NGC4826       12:56:47.39      +21.41.05.10  J2000   
# 7         NGC4826       12:56:45.82      +21.40.27.00  J2000   
# 8         NGC4826       12:56:42.66      +21.40.27.00  J2000   
# Spectral Windows:
# SpwID  #Chans Frame Ch1(MHz)    ChanWid(kHz)TotBW(kHz)  Ref(MHz)    Corrs
# 0          64 LSRK  115108.478  1562.5      100000      115108.478  YY  
# 1          64 LSRK  115198.615  1562.5      100000      115198.615  YY  
# 2          64 LSRK  115288.478  1562.5      100000      115288.478  YY  
# 3          64 LSRK  115378.615  1562.5      100000      115378.615  YY  
# 4          32 LSRK  114974.256  3125        100000      114974.256  YY  
# 5          32 LSRK  115074.393  3125        100000      115074.393  YY  
# 6          32 LSRK  115174.256  3125        100000      115174.256  YY  
# 7          32 LSRK  115274.393  3125        100000      115274.393  YY  
# 8          32 LSRK  115374.256  3125        100000      115374.256  YY  
# 9          32 LSRK  115474.392  3125        100000      115474.392  YY  
# 10         32 LSRK  115574.255  3125        100000      115574.255  YY  
# 11         32 LSRK  115674.392  3125        100000      115674.392  YY  
# 12         64 LSRK  114950.191  1562.5      100000      114950.191  YY  
# 13         64 LSRK  115040.205  1562.5      100000      115040.205  YY  
# 14         64 LSRK  115129.946  1562.5      100000      115129.946  YY  
# 15         64 LSRK  115219.96   1562.5      100000      115219.96   YY  

#
##########################################################################
# Plotting and Flagging
##########################################################################
#
# The plotxy task is the interactive x-y display and flagging GUI
#
#print '--Plotxy--'
#default(plotxy)

# First look at amplitude as a funciton of uv-distance using an
# average over all times and channels for each source.
#if scriptmode:
#    plotxy(vis='ngc4826.tutorial.ms',xaxis='uvdist',yaxis='amp',
#           field='0',spw='0~3',
#           averagemode='vector',timebin='1e7',width='1000',crossscans=True,
#           selectplot=True,title='Field 0 SPW 0~3')

#    print "Looking at 3C273 versus uvdist with time and chan average"
    # Pause script if you are running in scriptmode
#    user_check=raw_input('Return to continue script\n')
#else:
#    plotxy(vis='ngc4826.tutorial.ms',xaxis='uvdist',yaxis='amp',
#           field='0',spw='0~3',
#           averagemode='vector',timebin='1e7',width='1000',crossscans=True,
#           selectplot=True,title='Field 0 SPW 0~3',
#           interactive=False,
#           figfile='ngc4826.tutorial.ms.plotxy.field0.ampuv.allavg.png')

# NOTE: width here needs to be larger than combination of all channels
# selected with spw and/or field. Since field and spw are unique in this
# case, both don't need to be specified, however plotting is much faster
# if you "help it" by selecting both.

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
#if scriptmode:
#    plotxy(vis='ngc4826.tutorial.ms',xaxis='velocity',yaxis='amp',
#           field='2',spw='12~15',
#           averagemode='vector',timebin='1e7',crossscans=True,
#           selectplot=True,newplot=False,title='Field 2 SPW 12~15')

#    print "You could Mark Region around outliers and Flag"
    # Pause script if you are running in scriptmode
#    user_check=raw_input('Return to continue script\n')
#else:
    # Output to file
    # Set up a Python loop to do all the N4826 fields:
#    for fld in range(2,9):
#        field = str(fld)
#        plotxy(vis='ngc4826.tutorial.ms',xaxis='velocity',yaxis='amp',
#               field=field,spw='12~15',
#               averagemode='vector',timebin='1e7',crossscans=True,
#               selectplot=True,newplot=False,title='Field 2 SPW 12~15',
#               interactive=False,
#               figfile='ngc4826.tutorial.ms.plotxy.field2.ampvel.tavg.png')

    # Now the 1310+323 field
#    plotxy(vis='ngc4826.tutorial.ms',xaxis='velocity',yaxis='amp',
#           field='1',spw='4~11',
#           averagemode='vector',timebin='1e7',crossscans=True,
#           selectplot=True,newplot=False,title='Field 1 SPW 4~11',
#           interactive=False,
#           figfile='ngc4826.tutorial.ms.plotxy.field1.ampvel.tavg.png')

    # Now the 3C273 field
    # This one should be time and channel averaged to test this
#   plotxy(vis='ngc4826.tutorial.ms',xaxis='velocity',yaxis='amp',
#         field='0',spw='0~3',
#         averagemode='vector',timebin='1e7',crossscans=True,
#           selectplot=True,newplot=False,title='Field 0 SPW 0~3',
#           interactive=False,
#           figfile='ngc4826.tutorial.ms.plotxy.field0.ampvel.tavg.png')

# You can also have it iterate over baselines, using Next to advance
# Set to NOT plot autocorrelations
# Example using 3C273: (interactive only)
#if scriptmode:
#    plotxy(vis='ngc4826.tutorial.ms',xaxis='channel',yaxis='amp',
#           field='0',spw='0~3',
#           selectdata=True,antenna='*&*',
#           averagemode='vector',timebin='1e7',crossscans=True,
#           iteration='baseline',
#           selectplot=True,newplot=False,title='Field 0 SPW 0~3')
	
    # Pause script if you are running in scriptmode
#    user_check=raw_input('Return to continue script\n')

#
# Finally, look for bad data. Here we look at field 8 w/o averaging
#if scriptmode:
#    plotxy(vis='ngc4826.tutorial.ms',xaxis='time',yaxis='amp',field='8',spw='12~15',
#           selectplot=True,newplot=False,title='Field 8 SPW 12~15')

#    print "You can see some bad data here"
#    print "Mark Region and Locate, look in logger"
#    print "This is a correlator glitch in baseline 3-9 at 06:19:30"
#    print "PLEASE DON\'T FLAG ANYTHING HERE. THE SCRIPT WILL DO IT!"
#    print "In a normal session you could Mark Region and Flag."
#    print "Here we will use flagdata instead."
    # Pause script if you are running in scriptmode
#    user_check=raw_input('Return to continue script\n')

    # If you change xaxis='channel' you see its all channels
#else:
    # Plot to file
#    plotxy(vis='ngc4826.tutorial.ms',xaxis='time',yaxis='amp',
#           field='8',spw='12~15',
#           selectplot=True,newplot=False,title='Field 8 SPW 12~15',
#           interactive=False,
#           figfile='ngc4826.tutorial.ms.plotxy.field2.amptime.noavg.png')
    
if benchmarking:
    plotxy2time=time.time()

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

flagdata(vis='ngc4826.tutorial.ms', mode='manual',
         spw='0~3:0;1;62;63,4~11:0;1;30;31,12~15:0;1;62;63')

#
# Flag correlator glitch
#
print ""
print "Flagging bad correlator field 8 antenna 3&9 spw 15 all channels"
print "  timerange 1998/04/16/06:19:00.0~1998/04/16/06:20:00.0"
print ""

flagdata(vis='ngc4826.tutorial.ms', mode='manual', field='8', spw='15', antenna='3&9', 
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

if benchmarking:
    flag2time=time.time()

print "Completed pre-calibration flagging"

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

setjy(vis='ngc4826.tutorial.ms',field='0',standard='manual',fluxdensity=[23.0,0.,0.,0.],spw='0~3', scalebychan=False, usescratch=False)
#
# Not really necessary to set spw but you get lots of warning messages if
# you don't

if benchmarking:
    setjy2time=time.time()

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
	refant='ANT5', 
	solint='inf', combine='spw')

if benchmarking:
    gaincal2time=time.time()

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

if benchmarking:
    fluxscale2time=time.time()

#
##########################################################################
#
# Plot calibration
print '--Plotcal (fluxscale)--'
default(plotcal)

if scriptmode:
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
    plotcal(caltable='ngc4826.tutorial.16apr98.fcal', yaxis='snr', field='')
else:
    # You can also plotcal to file
    plotcal(caltable='ngc4826.tutorial.16apr98.fcal',yaxis='amp',field='',
            showgui=False,figfile='ngc4826.tutorial.16apr98.fcal.plotcal.amp.png')
    plotcal(caltable='ngc4826.tutorial.16apr98.fcal',yaxis='phase',field='',
            showgui=False,figfile='ngc4826.tutorial.16apr98.fcal.plotcal.phase.png')
    plotcal(caltable='ngc4826.tutorial.16apr98.fcal',yaxis='snr',field='',
            showgui=False,figfile='ngc4826.tutorial.16apr98.fcal.plotcal.snr.png')

if benchmarking:
    plotcal2time=time.time()

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
         gaintable='ngc4826.tutorial.16apr98.fcal',
	 spwmap=[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])

if benchmarking:
    correct2time=time.time()

#
##########################################################################
#
# Check calibrated data
#print '--Plotxy--'
#default(plotxy)

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

#if benchmarking:
#    plotfinal2time=time.time()

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

if benchmarking:
    split2time=time.time()

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

#if benchmarking:
#    clearfinal2time=time.time()

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

#if benchmarking:
#    cleancal2time=time.time()

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
      multiscale=[],
      cyclefactor=2, niter=10000, threshold='140mJy',
      minpb=0.3,pbcor=False, usescratch=False)

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

if benchmarking:
    clean2time=time.time()

#
##########################################################################
#
# Do interactive viewing of clean image
print '--Viewer--'
if scriptmode:
    viewer('ngc4826.tutorial.16apr98.src.clean.image')

    print ""
    print "This is the non-pbcorrected cube of NGC4826"
    print "Use tape deck to move through channels"
    print "Close the viewer when done"
    print ""

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

if benchmarking:
    math2time=time.time()

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

# Now view the resulting images
if scriptmode:
    viewer('ngc4826.tutorial.16apr98.moments.integrated')
    #
    print "Now viewing Moment-0 ngc4826.tutorial.16apr98.moments.integrated"
    print "Note PBCOR effects at field edge"
    print "Change the colorscale to get better image"
    print "You can also Open and overlay Contours of Moment-1 ngc4826.tutorial.16apr98.moments.mom1"
    print "Close the viewer when done"
    
    # Pause script if you are running in scriptmode
    user_check=raw_input('Return to continue script\n')

# Do a moment one on channel 0 to check that the indexing is right
# NOTE: THIS STILL CRASHES
try:
    immoments(imagename='ngc4826.tutorial.16apr98.src.clean.image',
              moments=[1],includepix=[],
              chans='0',
              outfile='ngc4826.tutorial.16apr98.moments.plane0.mom1') 
except:
    pass

# Do a moment one on channel 35 to check that the indexing is right
try:
    immoments(imagename='ngc4826.tutorial.16apr98.src.clean.image',
	  moments=[1],includepix=[],
	  chans='35',
          outfile='ngc4826.tutorial.16apr98.moments.plane35.mom1')
except:
    pass

if benchmarking:
    moments2time=time.time()

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

try:
    momoneplane0=imstat('ngc4826.tutorial.16apr98.moments.plane0.mom1')
    print "Found plane 0 moment 1 value = "+str(momoneplane0['median'][0])
except:
    pass


try:
    momoneplane35=imstat('ngc4826.tutorial.16apr98.moments.plane35.mom1')
    print "Found plane 35 moment 1 value = "+str(momoneplane35['median'][0])
except:
    pass

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
#	   planes='6~27',
#	   outfile='ngc4826.tutorial.16apr98.moments.integratedmasked') 
#
#print 'Creating masked moment 1 image ngc4826.tutorial.16apr98.moments.mom1masked'
#
#immoments(imagename='ngc4826.tutorial.16apr98.src.clean.pbcor.masked',
#          moments=1,axis=3,
#	   includepix=[0.2,1000.0],
#	   planes='6~27',
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

if benchmarking:
    endProc=time.clock()
    endTime=time.time()

#
##########################################################################
# Previous results to be used for regression

# Canonical regression values (using this script STM 2008-06-04) were:
#vis_mean_cal=4.3269
#vis_mean_src=156.992
#clean_image_max=1.45868253708
#clean_offsrc_rms=0.0438643493782
#clean_offline_rms=0.0544108718199
#clean_momentzero_max=169.420959473
#clean_momentzero_rms=14.3375244141
# And from STM 2008-06-18
#clean_momentone_median=428.43
# And from STM 2008-06-30
#clean_momentone_planezero = 688.575012
#clean_momentone_planelast = 119.659264
# MS mean STM 2008-07-02
#vis_mean_cal = 194.613642
#vis_mean_src = 54.583024

#New values STM 2008-10-06 Patch3 (gaincal fix)
#clean_image_max = 1.466937
#clean_offsrc_rms = 0.043549
#clean_offline_rms = 0.054727
#clean_momentzero_max = 174.315887
#clean_momentzero_rms = 14.601299
#clean_momentone_median = 427.726257
#clean_momentone_planezero = 688.575012
#clean_momentone_planelast = 119.659264
#vis_mean_cal = 194.914215
#vis_mean_src = 54.626986

#New model fluxes STM 2008-10-23 Patch3
#  from run w/o clean mosaic pb change using 256x256 image
#model_sum = 73.183142
#model_pbcor_sum = 76.960971

#New values STM 2008-12-01 Patch3.0 (released version)
#for 400x400 clean 
#testdate = '2008-12-01 (STM)'
#testvers = 'CASA Version 2.3.0 Rev 6654'
#clean_image_max = 1.481322
#clean_offsrc_rms = 0.043665
#clean_offline_rms = 0.055379
#clean_momentzero_max = 174.731827
#clean_momentzero_rms = 14.858011
#clean_momentone_median = 428.499237
#clean_momentone_planezero = 688.575012
#clean_momentone_planelast = 119.659264
#vis_mean_cal = 194.915497
#vis_mean_src = 54.627127
#model_sum = 72.437971
#model_pbcor_sum = 70.417830

#New values STM 2009-02-25 Patch3.1
#for 400x400 clean 
#testdate = '2009-02-25 (STM)'
#testvers = 'CASA Version 2.3.1 Rev 6826'
#clean_image_max = 1.481322
#clean_offsrc_rms = 0.043665
#clean_offline_rms = 0.055379
#clean_momentzero_max = 174.731842
#clean_momentzero_rms = 14.858011
#clean_momentone_median = 428.499237
#clean_momentone_planezero = 688.575012
#clean_momentone_planelast = 119.659264
#vis_mean_cal = 194.915497
#vis_mean_src = 54.627127
#model_sum = 72.437973
#model_pbcor_sum = 70.417831

#New values STM 2009-06-19 Patch4 (released version)
#for 400x400 clean
#testdate = '2009-06-19 (STM)'
#testvers = 'CASA Version 2.4.0 Rev 8115'
#clean_image_max = 1.418478
#clean_offsrc_rms = 0.043584
#clean_offline_rms = 0.056824
#clean_momentzero_max = 171.601685
#clean_momentzero_rms = 15.532441
#clean_momentone_median = 428.499237
#clean_momentone_planezero = 688.575012
#clean_momentone_planelast = 119.659264
#vis_mean_cal = 194.915497
#vis_mean_src = 54.627127
#model_sum = 70.707405
#model_pbcor_sum = 63.006854

#New values KG 2009-11-01 Release 0 (prerelease version)
#for 400x400 clean
#new values for flat noise clean
#testdate = '2009-12-02 (KG)'
#testvers = 'CASA Version 3.0.0 Rev 9692'
#clean_image_max = 1.46
#clean_offsrc_rms = 0.0573
#clean_offline_rms = 0.05429
#clean_momentzero_max = 165.7
#clean_momentzero_rms = 15.1
#clean_momentone_median = 422.84
#clean_momentone_planezero = 688.575012
#clean_momentone_planelast = 119.659264
#vis_mean_cal = 194.915497
#vis_mean_src = 54.627127
#model_sum = 74.374
#model_pbcor_sum = 65.319

## #New values STM 2009-12-02 Release 0 (prerelease version)
## #for 400x400 clean
## #new values for flat noise clean
## testdate = '2009-12-02 (STM)'
## testvers = 'CASA Version 3.0.1 Rev 10130'
## clean_image_max = 1.465047 
## clean_offsrc_rms = 0.058497
## clean_offline_rms = 0.055416
## clean_momentzero_max = 163.726852
## clean_momentzero_rms = 15.206372
## clean_momentone_median = 428.326385
## clean_momentone_planezero = 696.702393
## clean_momentone_planelast = 127.786629
## vis_mean_cal = 194.915085
## vis_mean_src = 54.627020
## model_sum = 71.171693
## model_pbcor_sum = 61.853749

## #New values RR 2010-03-30 3.0.1 prerelease
## #for 400x400 clean
## #new values after start channel change.  I did not update passing values.
## testdate = '2010-03-30 (RR)'
## testvers = 'CASA Version 3.0.1 (build #10841)'
## clean_image_max = 1.615747 # was 1.465047 Peak hits a channel better?
## clean_offsrc_rms = 0.058497
## clean_offline_rms = 0.055416
## clean_momentzero_max = 163.726852
## clean_momentzero_rms = 15.206372
## clean_momentone_median = 429.658844 # was 428.326385; change << 1 chanwidth.
## clean_momentone_planezero = 696.702393
## clean_momentone_planelast = 127.786629
## vis_mean_cal = 194.915085
## vis_mean_src = 54.627020
## model_sum = 71.171693
## model_pbcor_sum = 66.882499 # was 61.853749 Peak hits a channel better?

## # slight change in regression values - reason not known yet.
## # Remy's changes to cleanhelper, or something from Sanjay or Kumar?
## testdate = '2010-04-24 (RR)'
## testvers = 'CASA Version 3.0.2 (build #11181)'
## clean_image_max = 1.615747
## clean_offsrc_rms = 0.058497
## clean_offline_rms = 0.055416
## clean_momentzero_max = 163.726852
## clean_momentzero_rms = 15.206372
## clean_momentone_median = 423.6954 # was 429.6588; change << 1 chanwidth.
## clean_momentone_planezero = 696.702393
## clean_momentone_planelast = 127.786629
## vis_mean_cal = 194.915085
## vis_mean_src = 54.627020
## model_sum = 71.171693
## model_pbcor_sum = 75.92 # was 66.88 Peak hits a channel better?

# slight change in 1 regression value - Kumar fixed a bug in setjy with
# multiple spws, which could affect this script.
testdate = '2010-04-29 (RR)'
testvers = 'CASA Version 3.0.2 (build #11306)'
clean_image_max = 1.615747
clean_offsrc_rms = 0.058497
clean_offline_rms = 0.0599
clean_momentzero_max = 159.22
clean_momentzero_rms = 14.31
#
#  32 bits gets 423.6954 and 64 bits gets 422.142792
#  diff << 1 chanwidth.
clean_momentone_median = 422.92
clean_momentone_planezero = 696.702393
clean_momentone_planelast = 127.786629
vis_mean_cal = 195.0509
vis_mean_src = 54.665
model_sum = 71.349
model_pbcor_sum = 75.92 # was 66.88 Peak hits a channel better?

# RR, 1/19/2011 - The rmses went down, just like in orionmos4sim.  This is
# good, so I won't complain too loudly.  The moment 1 median and pbcor_sum have
# jiggled around a fair bit.  The median is _not_ affected by the two spurious
# blobs at 501.64 km/s, though.  (Verified by doing imstat with a tight polygon
# region.)
clean_offsrc_rms = 0.0557
clean_offline_rms = 0.0550
clean_momentzero_rms = 14.57
# The chanwidth is ~16 km/s.
clean_momentone_median = 435.368103

model_pbcor_sum = 72.72

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
clean_image_max = 1.4647
clean_momentone_median = 415.9
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
if benchmarking:
    # Save timing to regression dictionary
    new_regression['timing'] = {}

    total = {}
    total['wall'] = (endTime - startTime)
    total['cpu'] = (endProc - startProc)
    total['rate_raw'] = (datasize_raw/(endTime - startTime))
    total['rate_ms'] = (datasize_ms/(endTime - startTime))

    new_regression['timing']['total'] = total

    nstages = 15
    new_regression['timing']['nstages'] = nstages

    stages = {}
    stages[0] = ['import',(import2time-startTime)]
    stages[1] = ['concat',(concat2time-import2time)]
    stages[2] = ['clearcal',(clearcal2time-concat2time)]
    stages[3] = ['listobs',(list2time-clearcal2time)]
    stages[4] = ['plotxy',(plotxy2time-list2time)]
    stages[5] = ['flagdata',(flag2time-plotxy2time)]
    stages[6] = ['setjy',(setjy2time-flag2time)]
    stages[7] = ['gaincal',(gaincal2time-setjy2time)]
    stages[8] = ['fluxscale',(fluxscale2time-gaincal2time)]
    stages[9] = ['plotcal',(plotcal2time-fluxscale2time)]
    stages[10] = ['applycal',(correct2time-plotcal2time)]
    stages[11] = ['split',(split2time-correct2time)]
    stages[12] = ['clean',(clean2time-split2time)]
    stages[13] = ['math/stat',(math2time-clean2time)]
    stages[14] = ['moments',(moments2time-math2time)]
    
    new_regression['timing']['stages'] = stages

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
    print ''
    print 'Regression PASSED'
    print ''
else:
    regstate=False
    print >>logfile,'----FAILED Regression test for NGC 4826 Mosaic'
    print '----FAILED Regression test for NGC 4826 Mosaic'
    print ''
    print 'Regression FAILED'
    print ''
    
#
##########################################################################
# Print benchmarking etc.

if benchmarking:
    print ''
    print 'Total wall clock time was: %10.3f ' % total['wall']
    print 'Total CPU        time was: %10.3f ' % total['cpu']
    print 'Raw processing rate MB/s was: %8.1f ' % total['rate_raw']
    print 'MS  processing rate MB/s was: %8.1f ' % total['rate_ms']
    print ''
    print '* Breakdown:                              *'

    print >>logfile,''
    print >>logfile,'********* Benchmarking *************************'
    print >>logfile,'*                                              *'
    print >>logfile,'Total wall clock time was: %10.3f ' % total['wall']
    print >>logfile,'Total CPU        time was: %10.3f ' % total['cpu']
    print >>logfile,'Raw processing rate MB/s was: %8.1f ' % total['rate_raw']
    print >>logfile,'MS  processing rate MB/s was: %8.1f ' % total['rate_ms']
    print >>logfile,'* Breakdown:                                   *'

    for i in range(nstages):
        print '* %16s * time was: %10.3f ' % tuple(stages[i])
        print >>logfile,'* %16s * time was: %10.3f ' % tuple(stages[i])
    
    print >>logfile,'************************************************'
    print >>logfile,'sandrock (2008-06-17) wall time was: 377 seconds'
    print >>logfile,'sandrock (2008-06-17) CPU  time was: 312 seconds'

logfile.close()

print "Done with NGC4826 Tutorial Regression"
#
##########################################################################
