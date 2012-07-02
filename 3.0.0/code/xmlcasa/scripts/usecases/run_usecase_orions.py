#####################################
#
# ORION-S SDtasks Use Case
# Position-Switched data
# Version TT 2008-10-14 (updated)
# Version STM 2007-03-04
#
# This is a detailed walk-through
# for using the SDtasks on a
# test dataset.
#
#####################################
import time
import os

# NOTE: you should have already run
# asap_init()
# to import the ASAP tools as sd.<tool>
# and the SDtasks

#
# This is the environment variable
# pointing to the head of the CASA
# tree that you are running
casapath=os.environ['CASAPATH']

#
# This bit removes old versions of the output files
os.system('rm -rf sdusecase_orions* ')
#
# This is the path to the OrionS GBT ms in the data repository
datapath=casapath.split()[0]+'/data/regression/ATST5/OrionS/OrionS_rawACSmod'
#
# The follwing will remove old versions of the data and
# copy the data from the repository to your
# current directory.  Comment this out if you already have it
# and don't want to recopy
os.system('rm -rf OrionS_rawACSmod')
copystring='cp -r '+datapath+' .'
os.system(copystring)

# Now is the time to set some of the more useful
# ASAP environment parameters (the ones that the
# ASAP User Manual claims are in the .asaprc file).
# These are in the Python dictionary sd.rcParams
# You can see whats in it by typing:
#sd.rcParams
# One of them is the 'verbose' parameter which tells
# ASAP whether to spew lots of verbiage during processing
# or to keep quiet.  The default is
#sd.rcParams['verbose']=True
# You can make ASAP run quietly (with only task output) with
#sd.rcParams['verbose']=False

# Another key one is to tell ASAP to save memory by
# going off the disk instead.  The default is
#sd.rcParams['scantable.storage']='memory'
# but if you are on a machine with small memory, do
#sd.rcParams['scantable.storage']='disk'

# You can reset back to defaults with
#sd.rcdefaults

##########################
#
# ORION-S HC3N
# Position-Switched data
#
##########################
startTime=time.time()
startProc=time.clock()

##########################
# List data
##########################
# List the contents of the dataset
# First reset parameter defaults (safe)
default('sdlist')

# You can see its inputs with
#inp('sdlist')
# or just
#inp
# now that the defaults('sdlist') set the
# taskname='sdlist'
#
# Set the name of the GBT ms file
sdfile = 'OrionS_rawACSmod'

# Set an output file in case we want to
# refer back to it
listfile = 'sdusecase_orions_summary.txt'
sdlist()

# You could also just type
#go

# You should see something like:
#
#--------------------------------------------------------------------------------
# Scan Table Summary
#--------------------------------------------------------------------------------
#Beams:         1   
#IFs:           26  
#Polarisations: 2   (linear)
#Channels:      8192
#
#Observer:      Joseph McMullin
#Obs Date:      2006/01/19/01:45:58
#Project:       AGBT06A_018_01
#Obs. Type:     OffOn:PSWITCHOFF:TPWCAL
#Antenna Name:  GBT
#Flux Unit:     Jy
#Rest Freqs:    [4.5490258e+10] [Hz]
#Abcissa:       Channel
#Selection:     none
#
#Scan Source         Time      Integration       
#     Beam    Position (J2000)
#          IF       Frame   RefVal          RefPix    Increment   
#--------------------------------------------------------------------------------
#  20 OrionS_psr     01:45:58    4 x       30.0s
#        0    05:15:13.5 -05.24.08.2
#            0      LSRK   4.5489354e+10   4096    6104.233
#            1      LSRK   4.5300785e+10   4096    6104.233
#            2      LSRK   4.4074929e+10   4096    6104.233
#            3      LSRK   4.4166215e+10   4096    6104.233
#  21 OrionS_ps      01:48:38    4 x       30.0s
#        0    05:35:13.5 -05.24.08.2
#            0      LSRK   4.5489354e+10   4096    6104.233
#            1      LSRK   4.5300785e+10   4096    6104.233
#            2      LSRK   4.4074929e+10   4096    6104.233
#            3      LSRK   4.4166215e+10   4096    6104.233
#  22 OrionS_psr     01:51:21    4 x       30.0s
#        0    05:15:13.5 -05.24.08.2
#            0      LSRK   4.5489354e+10   4096    6104.233
#            1      LSRK   4.5300785e+10   4096    6104.233
#            2      LSRK   4.4074929e+10   4096    6104.233
#            3      LSRK   4.4166215e+10   4096    6104.233
#  23 OrionS_ps      01:54:01    4 x       30.0s
#        0    05:35:13.5 -05.24.08.2
#            0      LSRK   4.5489354e+10   4096    6104.233
#            1      LSRK   4.5300785e+10   4096    6104.233
#            2      LSRK   4.4074929e+10   4096    6104.233
#            3      LSRK   4.4166215e+10   4096    6104.233
#  24 OrionS_psr     02:01:47    4 x       30.0s
#        0    05:15:13.5 -05.24.08.2
#           12      LSRK   4.3962126e+10   4096   6104.2336
#           13      LSRK    4.264542e+10   4096   6104.2336
#           14      LSRK    4.159498e+10   4096   6104.2336
#           15      LSRK   4.3422823e+10   4096   6104.2336
#  25 OrionS_ps      02:04:27    4 x       30.0s
#        0    05:35:13.5 -05.24.08.2
#           12      LSRK   4.3962126e+10   4096   6104.2336
#           13      LSRK    4.264542e+10   4096   6104.2336
#           14      LSRK    4.159498e+10   4096   6104.2336
#           15      LSRK   4.3422823e+10   4096   6104.2336
#  26 OrionS_psr     02:07:10    4 x       30.0s
#        0    05:15:13.5 -05.24.08.2
#           12      LSRK   4.3962126e+10   4096   6104.2336
#           13      LSRK    4.264542e+10   4096   6104.2336
#           14      LSRK    4.159498e+10   4096   6104.2336
#           15      LSRK   4.3422823e+10   4096   6104.2336
#  27 OrionS_ps      02:09:51    4 x       30.0s
#        0    05:35:13.5 -05.24.08.2
#           12      LSRK   4.3962126e+10   4096   6104.2336
#           13      LSRK    4.264542e+10   4096   6104.2336
#           14      LSRK    4.159498e+10   4096   6104.2336
#           15      LSRK   4.3422823e+10   4096   6104.2336

# The HC3N and CH3OH lines are in IFs 0 and 2 respectively
# of scans 20,21,22,23.  We will pull these out in our
# calibration.

##########################
# Calibrate data
##########################
# We will use the sdcal task to calibrate the data.
# Set the defaults
default('sdcal')

# You can see the inputs with
#inp

# Set our sdfile (which would have been set from our run of
# sdlist if we were not cautious and reset defaults).
sdfile = 'OrionS_rawACSmod'

# Lets leave the spectral axis in channels for now
specunit = 'channel'

# This is position-switched data so we tell sdcal this
calmode = 'ps'

# For GBT data, it is safest to not have scantable pre-average
# integrations within scans.
average = True
scanaverage = False

# We do want sdcal to average up scans and polarization after
# calibration however.
timeaverage = True
tweight='tintsys'
polaverage = True
pweight='tsys'

# Do an atmospheric optical depth (attenuation) correction
# Input the zenith optical depth at 43 GHz
tau = 0.09

# Select our scans and IFs (for HC3N)
scanlist = [20,21,22,23]
iflist = [0]

# We do not require selection by field name (they are all
# the same except for on and off)
field = ''

# We will do some spectral smoothing
# For this demo we will use boxcar smoothing rather than
# the default
#kernel='hanning'
# We will set the width of the kernel to 5 channels
kernel = 'boxcar'
kwidth = 5

# We wish to fit out a baseline from the spectrum
# The GBT has particularly nasty baselines :(
# We will let ASAP use auto_poly_baseline mode
# but tell it to drop the 1000 edge channels from
# the beginning and end of the spectrum.
# A 2nd-order polynomial will suffice for this test.
# You might try higher orders for fun.
blmode = 'auto'
blpoly = 2
edge = [1000]

# We will not give it regions as an input mask
# though you could, with something like
#masklist=[[1000,3000],[5000,7000]]
masklist = []

# By default, we will not get plots in sdcal (but
# can make them using sdplot).
plotlevel = 0
# But if you wish to see a final spectrum, set
#plotlevel = 1
# or even
#plotlevel = 2
# to see intermediate plots and baselining output.

# Now we give the name for the output file
outfile = 'sdusecase_orions_hc3n.asap'

# We will write it out in ASAP scantable format
outform = 'asap'

# You can look at the inputs with
#inp

# Before running, lets save the inputs in case we want
# to come back and re-run the calibration.
saveinputs('sdcal','sdcal.orions.save')
# These can be recovered by
#execfile 'sdcal.orions.save'

# We are ready to calibrate
sdcal()

# Note that after the task ran, it produced a file
# sdcal.last which contains the inputs from the last
# run of the task (all tasks do this). You can recover
# this (anytime before sdcal is run again) with
#execfile 'sdcal.last'

##########################
# List data
##########################
# List the contents of the calibrated dataset
# Set the input to the just created file
sdfile = outfile
listfile = ''
sdlist()

# You should see:
#
#--------------------------------------------------------------------------------
# Scan Table Summary
#--------------------------------------------------------------------------------
#Beams:         1   
#IFs:           26  
#Polarisations: 1   (linear)
#Channels:      8192
#
#Observer:      Joseph McMullin
#Obs Date:      2006/01/19/01:45:58
#Project:       AGBT06A_018_01
#Obs. Type:     OffOn:PSWITCHOFF:TPWCAL
#Antenna Name:  GBT
#Flux Unit:     K
#Rest Freqs:    [4.5490258e+10] [Hz]
#Abcissa:       Channel
#Selection:     none
#
#Scan Source         Time      Integration       
#     Beam    Position (J2000)
#          IF       Frame   RefVal          RefPix    Increment   
#--------------------------------------------------------------------------------
#   0 OrionS_ps      01:52:05    1 x    08:00.5 
#        0    05:35:13.5 -05.24.08.2
#            0      LSRK   4.5489354e+10   4096    6104.233
#
# Note that our scans are now collapsed (timeaverage=True) but we still have
# our IF 0

##########################
# Plot data
##########################
default('sdplot')

# The file we produced after calibration
# (if we hadn't reset defaults it would have
# been set - note that sdplot,sdfit,sdstat use
# sdfile as the input file, which is the output
# file of sdcal).
sdfile = 'sdusecase_orions_hc3n.asap'

# Lets just go ahead and plot it up as-is
sdplot()

# Looks ok.  Plot with x-axis in GHz
specunit='GHz'
sdplot()

# Note that the rest frequency in the scantable
# is set correctly to the HCCCN line at 45.490 GHz.
# So you can plot the spectrum in km/s
specunit='km/s'
sdplot()

# Zoom in
sprange=[-100,50]
sdplot()

# Lets plot up the lines to be sure
# We have to go back to GHz for this
# (known deficiency in ASAP)
specunit='GHz'
sprange=[45.48,45.51]
linecat='all'
sdplot()

# Too many lines! Focus on the HC3N ones
linecat='HCCCN'
sdplot()

# Finally, we can convert from K to Jy
# using the aperture efficiencies we have
# coded into the sdtasks
# For GBT data, do not set telescopeparm
fluxunit='Jy'
telescopeparm=''
sdplot()

# Lets save this plot
plotfile='sdusecase_orions_hc3n.eps'
sdplot()

##########################
# Off-line Statistics
##########################
# Now do some region statistics
# First the line-free region
# Set parameters
default('sdstat')
sdfile = 'sdusecase_orions_hc3n.asap'

# Keep the default spectrum and flux units
# K and channel
fluxunit = ''
specunit = ''

# Pick out a line-free region
# You can bring up a default sdplot again
# to check this
masklist = [[5000,7000]]

# This is a line-free region so we don't need
# to invert the mask
invertmask = False

# You can check with
#inp

# sdstat returns some results in
# the Python dictionary.  You can assign
# this to a variable
off_stat=sdstat()

# and look at it
off_stat
# which should give
# {'eqw': 38.563105620704945,
#  'max': 0.15543246269226074,
#  'mean': -0.0030361821409314871,
#  'median': -0.0032975673675537109,
#  'min': -0.15754437446594238,
#  'rms': 0.047580458223819733,
#  'stddev': 0.047495327889919281,
#  'sum': -6.0754003524780273}


#You see it has some keywords for the various
#stats.  We want the standard deviation about
#the mean, or 'stddev'
print "The off-line std. deviation = ",off_stat['stddev']
# which should give
# The off-line std. deviation =  0.0474953278899

# or better formatted (using Python I/O formatting)
print "The off-line std. deviation = %5.3f K" % (off_stat['stddev'])
# which should give
# The off-line std. deviation = 0.047 K

##########################
# On-line Statistics
##########################
# Now do the line region
# Continue setting or resetting parameters
masklist = [[3900,4200]]

line_stat=sdstat()

# look at these
line_stat
# which gives
# {'eqw': 73.335154614280981,
#  'max': 0.92909121513366699,
#  'mean': 0.22636228799819946,
#  'median': 0.10317134857177734,
#  'min': -0.13283586502075195,
#  'rms': 0.35585442185401917,
#  'stddev': 0.27503398060798645,
#  'sum': 68.135047912597656}

# of particular interest are the max value
print "The on-line maximum = %5.3f K" % (line_stat['max'])
# which gives
# The on-line maximum = 0.929 K

# and the estimated equivalent width (in channels)
# which is the sum/max
print "The estimated equivalent width = %5.1f channels" % (line_stat['eqw'])
# which gives
# The estimated equivalent width =  73.3 channels

##########################
# Line Fitting
##########################
# Now we are ready to do some line fitting
# Default the parameters
default('sdfit')

# Set our input file
sdfile = 'sdusecase_orions_hc3n.asap'

# Stick to defaults
# fluxunit = 'K', specunit = 'channel'
fluxunit = ''
specunit = ''

# We will try auto-fitting first
fitmode = 'auto'
# A single Gaussian
nfit = [1]
# Leave the auto-parameters to their defaults for
# now, except ignore the edge channels
edge = [1000]

# Lets see a plot while doing this
plotlevel = 1

# Save the fit output in a file
fitfile = 'sdusecase_orions_hc3n.fit'

# Go ahead and do the fit
fit_stat=sdfit()

# If you had verbose mode on, you probably saw something
# like:
#
# 0: peak = 0.811 K , centre = 4091.041 channel, FWHM = 72.900 channel
#    area = 62.918 K channel
#

# returned dictionary is stored in a variable, fit_stat
fit_stat
#
# {'cent': [[4091.04052734375, 0.72398632764816284]],
#  'fwhm': [[72.899894714355469, 1.7048574686050415]],
#  'nfit': 1,
#  'peak': [[0.81080442667007446, 0.016420882195234299]]}
#
# So you can write them out or test them:
print "The line-fit parameters were:"
print "      maximum = %6.3f +/- %6.3f K" % (fit_stat['peak'][0][0],fit_stat['peak'][0][1])
print "       center = %6.1f +/- %6.1f channels" % (fit_stat['cent'][0][0],fit_stat['cent'][0][1])
print "         FWHM = %6.2f +/- %6.2f channels" % (fit_stat['fwhm'][0][0],fit_stat['fwhm'][0][1])
#
# Which gives:
# The line-fit parameters were:
#       maximum =  0.811 +/-  0.016 K
#        center = 4091.0 +/-    0.7 channels
#          FWHM =  72.90 +/-   1.70 channels

# We can do the fit in km/s also
specunit = 'km/s'
# For some reason we need to help it along with a mask
maskline = [-50,0]

fitfile = 'sdusecase_orions_hc3n_kms.fit'
fit_stat_kms=sdfit()
# Should give (if in verbose mode)
#   0: peak = 0.811 K , centre = -27.134 km/s, FWHM = 2.933 km/s
#      area = 2.531 K km/s
#

# with
fit_stat_kms
# giving
# {'cent': [[-27.133651733398438, 0.016480101272463799]],
#  'fwhm': [[2.93294358253479, 0.038807671517133713]],
#  'nfit': 1,
#  'peak': [[0.81080895662307739, 0.0092909494414925575]]}


print "The line-fit parameters were:"
print "      maximum = %6.3f +/- %6.3f K" % (fit_stat_kms['peak'][0][0],fit_stat_kms['peak'][0][1])
print "       center = %6.2f +/- %6.2f km/s" % (fit_stat_kms['cent'][0][0],fit_stat_kms['cent'][0][1])
print "         FWHM = %6.4f +/- %6.4f km/s" % (fit_stat_kms['fwhm'][0][0],fit_stat_kms['fwhm'][0][1])

# The line-fit parameters were:
#       maximum =  0.811 +/-  0.009 K
#        center = -27.13 +/-   0.02 km/s
#          FWHM = 2.9329 +/- 0.0388 km/s

##########################
#
# End ORION-S Use Case
#
##########################
