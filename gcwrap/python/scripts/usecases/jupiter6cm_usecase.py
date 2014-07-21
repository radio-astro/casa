######################################################################
#                                                                    #
# Use Case Script for Jupiter 6cm VLA                                #
#                                                                    #
# Last Updated STM 2007-10-10 (Beta)                                 #
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

pathname=os.environ.get('CASAPATH').split()[0]
prefix='jupiter6cm.usecase'

msfile = prefix + '.ms'

gtable = prefix + '.gcal'
ftable = prefix + '.fluxscale'
atable = prefix + '.accum'

srcsplitms = prefix + '.split.ms'

clnimsize = [288,288]
clncell = [4.,4.]

imname1 = prefix + '.clean1'
clnimage1 = imname1+'.image'
clnmodel1 = imname1+'.model'
clnresid1 = imname1+'.residual'
clnmask1  = imname1+'.clean_interactive.mask'

selfcaltab1 = srcsplitms + '.selfcal1'

imname2 = prefix + '.clean2'
clnimage2 = imname2+'.image'
clnmodel2 = imname2+'.model'
clnresid2 = imname2+'.residual'
clnmask2  = imname2+'.clean_interactive.mask'

selfcaltab2 = srcsplitms + '.selfcal2'
smoothcaltab2 = srcsplitms + '.smoothcal2'

imname3 = prefix + '.clean3'
clnimage3 = imname3+'.image'
clnmodel3 = imname3+'.model'
clnresid3 = imname3+'.residual'
clnmask3  = imname3+'.clean_interactive.mask'

#
#=====================================================================
#
# Get to path to the CASA home and stip off the name
pathname=os.environ.get('CASAPATH').split()[0]

# This is where the UVFITS data will be
#fitsdata=pathname+'/data/demo/jupiter6cm.fits'
fitsdata='/home/sandrock2/smyers/NAUG2/Data/VLA_CONT/FLUX99-6CM.CBAND'

# The prefix to use for all output files
prefix='jupiter6cm.usecase'

# Clean up old files
os.system('rm -rf '+prefix+'*')

#
#=====================================================================
# Data Import and List
#=====================================================================
#
# Import the data from FITS to MS
#
print '--Import--'

# Safest to start from task defaults
default('importuvfits')

# Set up the MS filename and save as new global variable
msfile = prefix + '.ms'

# Use task importuvfits
fitsfile = fitsdata
vis = msfile
importuvfits()

#=====================================================================
#
# List a summary of the MS
#
print '--Listobs--'

# Don't default this one and make use of the previous setting of
# vis.  Remember, the variables are GLOBAL!

# You may wish to see more detailed information, in this case
# use the verbose = True option
verbose = True

listobs()

# You should get in your logger window and in the casapy.log file
# something like:
#
#    Observer: FLUX99     Project:   
# Observation: VLA
# 
# Data records: 2021424       Total integration time = 85133.2 seconds
#    Observed from   23:15:27   to   22:54:20
# 
#    ObservationID = 0         ArrayID = 0
#   Date        Timerange                Scan  FldId FieldName      SpwIds
#   15-Apr-1999/23:15:26.7 - 23:16:10.0     1      0 0137+331       [0, 1]
#               23:38:40.0 - 23:48:00.0     2      1 0813+482       [0, 1]
#               23:53:40.0 - 23:55:20.0     3      2 0542+498       [0, 1]
#   16-Apr-1999/00:22:10.1 - 00:23:49.9     4      3 0437+296       [0, 1]
#               00:28:23.3 - 00:30:00.1     5      4 VENUS          [0, 1]
#               00:48:40.0 - 00:50:20.0     6      1 0813+482       [0, 1]
#               00:56:13.4 - 00:57:49.9     7      2 0542+498       [0, 1]
#               01:10:20.1 - 01:11:59.9     8      5 0521+166       [0, 1]
#               01:23:29.9 - 01:25:00.1     9      3 0437+296       [0, 1]
#               01:29:33.3 - 01:31:10.0    10      4 VENUS          [0, 1]
#               01:49:50.0 - 01:51:30.0    11      6 1411+522       [0, 1]
#               02:03:00.0 - 02:04:30.0    12      7 1331+305       [0, 1]
#               02:17:30.0 - 02:19:10.0    13      1 0813+482       [0, 1]
#               02:24:20.0 - 02:26:00.0    14      2 0542+498       [0, 1]
#               02:37:49.9 - 02:39:30.0    15      5 0521+166       [0, 1]
#               02:50:50.1 - 02:52:20.1    16      3 0437+296       [0, 1]
#               02:59:20.0 - 03:01:00.0    17      6 1411+522       [0, 1]
#               03:12:30.0 - 03:14:10.0    18      7 1331+305       [0, 1]
#               03:27:53.3 - 03:29:39.9    19      1 0813+482       [0, 1]
#               03:35:00.0 - 03:36:40.0    20      2 0542+498       [0, 1]
#               03:49:50.0 - 03:51:30.1    21      6 1411+522       [0, 1]
#               04:03:10.0 - 04:04:50.0    22      7 1331+305       [0, 1]
#               04:18:49.9 - 04:20:40.0    23      1 0813+482       [0, 1]
#               04:25:56.6 - 04:27:39.9    24      2 0542+498       [0, 1]
#               04:42:49.9 - 04:44:40.0    25      8 MARS           [0, 1]
#               04:56:50.0 - 04:58:30.1    26      6 1411+522       [0, 1]
#               05:24:03.3 - 05:33:39.9    27      7 1331+305       [0, 1]
#               05:48:00.0 - 05:49:49.9    28      1 0813+482       [0, 1]
#               05:58:36.6 - 06:00:30.0    29      8 MARS           [0, 1]
#               06:13:20.1 - 06:14:59.9    30      6 1411+522       [0, 1]
#               06:27:40.0 - 06:29:20.0    31      7 1331+305       [0, 1]
#               06:44:13.4 - 06:46:00.0    32      1 0813+482       [0, 1]
#               06:55:06.6 - 06:57:00.0    33      8 MARS           [0, 1]
#               07:10:40.0 - 07:12:20.0    34      6 1411+522       [0, 1]
#               07:28:20.0 - 07:30:10.1    35      7 1331+305       [0, 1]
#               07:42:49.9 - 07:44:30.0    36      8 MARS           [0, 1]
#               07:58:43.3 - 08:00:39.9    37      6 1411+522       [0, 1]
#               08:13:30.0 - 08:15:19.9    38      7 1331+305       [0, 1]
#               08:27:53.4 - 08:29:30.0    39      8 MARS           [0, 1]
#               08:42:59.9 - 08:44:50.0    40      6 1411+522       [0, 1]
#               08:57:09.9 - 08:58:50.0    41      7 1331+305       [0, 1]
#               09:13:03.3 - 09:14:50.1    42      9 NGC7027        [0, 1]
#               09:26:59.9 - 09:28:40.0    43      6 1411+522       [0, 1]
#               09:40:33.4 - 09:42:09.9    44      7 1331+305       [0, 1]
#               09:56:19.9 - 09:58:10.0    45      9 NGC7027        [0, 1]
#               10:12:59.9 - 10:14:50.0    46      8 MARS           [0, 1]
#               10:27:09.9 - 10:28:50.0    47      6 1411+522       [0, 1]
#               10:40:30.0 - 10:42:00.0    48      7 1331+305       [0, 1]
#               10:56:10.0 - 10:57:50.0    49      9 NGC7027        [0, 1]
#               11:28:30.0 - 11:35:30.0    50     10 NEPTUNE        [0, 1]
#               11:48:20.0 - 11:50:10.0    51      6 1411+522       [0, 1]
#               12:01:36.7 - 12:03:10.0    52      7 1331+305       [0, 1]
#               12:35:33.3 - 12:37:40.0    53     11 URANUS         [0, 1]
#               12:46:30.0 - 12:48:10.0    54     10 NEPTUNE        [0, 1]
#               13:00:29.9 - 13:02:10.0    55      6 1411+522       [0, 1]
#               13:15:23.3 - 13:17:10.1    56      9 NGC7027        [0, 1]
#               13:33:43.3 - 13:35:40.0    57     11 URANUS         [0, 1]
#               13:44:30.0 - 13:46:10.0    58     10 NEPTUNE        [0, 1]
#               14:00:46.7 - 14:01:39.9    59      0 0137+331       [0, 1]
#               14:10:40.0 - 14:12:09.9    60     12 JUPITER        [0, 1]
#               14:24:06.6 - 14:25:40.1    61     11 URANUS         [0, 1]
#               14:34:30.0 - 14:36:10.1    62     10 NEPTUNE        [0, 1]
#               14:59:13.4 - 15:00:00.0    63      0 0137+331       [0, 1]
#               15:09:03.3 - 15:10:40.1    64     12 JUPITER        [0, 1]
#               15:24:30.0 - 15:26:20.1    65      9 NGC7027        [0, 1]
#               15:40:10.0 - 15:45:00.0    66     11 URANUS         [0, 1]
#               15:53:50.0 - 15:55:20.0    67     10 NEPTUNE        [0, 1]
#               16:18:53.4 - 16:19:49.9    68      0 0137+331       [0, 1]
#               16:29:10.1 - 16:30:49.9    69     12 JUPITER        [0, 1]
#               16:42:53.4 - 16:44:30.0    70     11 URANUS         [0, 1]
#               16:54:53.4 - 16:56:40.0    71      9 NGC7027        [0, 1]
#               17:23:06.6 - 17:30:40.0    72      2 0542+498       [0, 1]
#               17:41:50.0 - 17:43:20.0    73      3 0437+296       [0, 1]
#               17:55:36.7 - 17:57:39.9    74      4 VENUS          [0, 1]
#               18:19:23.3 - 18:20:09.9    75      0 0137+331       [0, 1]
#               18:30:23.3 - 18:32:00.0    76     12 JUPITER        [0, 1]
#               18:44:49.9 - 18:46:30.0    77      9 NGC7027        [0, 1]
#               18:59:13.3 - 19:00:59.9    78      2 0542+498       [0, 1]
#               19:19:10.0 - 19:21:20.1    79      5 0521+166       [0, 1]
#               19:32:50.1 - 19:34:29.9    80      3 0437+296       [0, 1]
#               19:39:03.3 - 19:40:40.1    81      4 VENUS          [0, 1]
#               20:08:06.7 - 20:08:59.9    82      0 0137+331       [0, 1]
#               20:18:10.0 - 20:19:50.0    83     12 JUPITER        [0, 1]
#               20:33:53.3 - 20:35:40.1    84      1 0813+482       [0, 1]
#               20:40:59.9 - 20:42:40.0    85      2 0542+498       [0, 1]
#               21:00:16.6 - 21:02:20.1    86      5 0521+166       [0, 1]
#               21:13:53.4 - 21:15:29.9    87      3 0437+296       [0, 1]
#               21:20:43.4 - 21:22:30.0    88      4 VENUS          [0, 1]
#               21:47:26.7 - 21:48:20.1    89      0 0137+331       [0, 1]
#               21:57:30.0 - 21:59:10.0    90     12 JUPITER        [0, 1]
#               22:12:13.3 - 22:14:00.1    91      2 0542+498       [0, 1]
#               22:28:33.3 - 22:30:19.9    92      4 VENUS          [0, 1]
#               22:53:33.3 - 22:54:19.9    93      0 0137+331       [0, 1]
# 
# Fields: 13
#   ID   Name          Right Ascension  Declination   Epoch   
#   0    0137+331      01:37:41.30      +33.09.35.13  J2000   
#   1    0813+482      08:13:36.05      +48.13.02.26  J2000   
#   2    0542+498      05:42:36.14      +49.51.07.23  J2000   
#   3    0437+296      04:37:04.17      +29.40.15.14  J2000   
#   4    VENUS         04:06:54.11      +22.30.35.91  J2000   
#   5    0521+166      05:21:09.89      +16.38.22.05  J2000   
#   6    1411+522      14:11:20.65      +52.12.09.14  J2000   
#   7    1331+305      13:31:08.29      +30.30.32.96  J2000   
#   8    MARS          14:21:41.37      -12.21.49.45  J2000   
#   9    NGC7027       21:07:01.59      +42.14.10.19  J2000   
#   10   NEPTUNE       20:26:01.14      -18.54.54.21  J2000   
#   11   URANUS        21:15:42.83      -16.35.05.59  J2000   
#   12   JUPITER       00:55:34.04      +04.45.44.71  J2000   
# 
# Spectral Windows: (2 unique spectral windows and 1 unique polarization setups)
#   SpwID  #Chans Frame Ch1(MHz)    Resoln(kHz) TotBW(kHz)  Ref(MHz)    Corrs           
#   0           1 TOPO  4885.1      50000       50000       4885.1      RR  RL  LR  LL  
#   1           1 TOPO  4835.1      50000       50000       4835.1      RR  RL  LR  LL  
# 
# Feeds: 28: printing first row only
#   Antenna   Spectral Window     # Receptors    Polarizations
#   1         -1                  2              [         R, L]
# 
# Antennas: 27:
#   ID   Name  Station   Diam.    Long.         Lat.         
#   0    1     VLA:W9    25.0 m   -107.37.25.1  +33.53.51.0  
#   1    2     VLA:N9    25.0 m   -107.37.07.8  +33.54.19.0  
#   2    3     VLA:N3    25.0 m   -107.37.06.3  +33.54.04.8  
#   3    4     VLA:N5    25.0 m   -107.37.06.7  +33.54.08.0  
#   4    5     VLA:N2    25.0 m   -107.37.06.2  +33.54.03.5  
#   5    6     VLA:E1    25.0 m   -107.37.05.7  +33.53.59.2  
#   6    7     VLA:E2    25.0 m   -107.37.04.4  +33.54.01.1  
#   7    8     VLA:N8    25.0 m   -107.37.07.5  +33.54.15.8  
#   8    9     VLA:E8    25.0 m   -107.36.48.9  +33.53.55.1  
#   9    10    VLA:W3    25.0 m   -107.37.08.9  +33.54.00.1  
#   10   11    VLA:N1    25.0 m   -107.37.06.0  +33.54.01.8  
#   11   12    VLA:E6    25.0 m   -107.36.55.6  +33.53.57.7  
#   12   13    VLA:W7    25.0 m   -107.37.18.4  +33.53.54.8  
#   13   14    VLA:E4    25.0 m   -107.37.00.8  +33.53.59.7  
#   14   15    VLA:N7    25.0 m   -107.37.07.2  +33.54.12.9  
#   15   16    VLA:W4    25.0 m   -107.37.10.8  +33.53.59.1  
#   16   17    VLA:W5    25.0 m   -107.37.13.0  +33.53.57.8  
#   17   18    VLA:N6    25.0 m   -107.37.06.9  +33.54.10.3  
#   18   19    VLA:E7    25.0 m   -107.36.52.4  +33.53.56.5  
#   19   20    VLA:E9    25.0 m   -107.36.45.1  +33.53.53.6  
#   21   22    VLA:W8    25.0 m   -107.37.21.6  +33.53.53.0  
#   22   23    VLA:W6    25.0 m   -107.37.15.6  +33.53.56.4  
#   23   24    VLA:W1    25.0 m   -107.37.05.9  +33.54.00.5  
#   24   25    VLA:W2    25.0 m   -107.37.07.4  +33.54.00.9  
#   25   26    VLA:E5    25.0 m   -107.36.58.4  +33.53.58.8  
#   26   27    VLA:N4    25.0 m   -107.37.06.5  +33.54.06.1  
#   27   28    VLA:E3    25.0 m   -107.37.02.8  +33.54.00.5  
# 
# Tables:
#    MAIN                 2021424 rows     
#    ANTENNA                   28 rows     
#    DATA_DESCRIPTION           2 rows     
#    DOPPLER             <absent>  
#    FEED                      28 rows     
#    FIELD                     13 rows     
#    FLAG_CMD             <empty>  
#    FREQ_OFFSET         <absent>  
#    HISTORY                 7058 rows     
#    OBSERVATION                1 row      
#    POINTING                2604 rows     
#    POLARIZATION               1 row      
#    PROCESSOR            <empty>  
#    SOURCE               <empty> (see FIELD)
#    SPECTRAL_WINDOW            2 rows     
#    STATE                <empty>  
#    SYSCAL              <absent>  
#    WEATHER             <absent>  

# 
#=====================================================================
# Data Examination and Flagging
#=====================================================================
# 
# Get rid of the autocorrelations from the MS
#
print '--Flag auto-correlations--'

default(flagdata)
vis = msfile
mode = 'manual'
autocorr = True
flagdata()

#
#=====================================================================
#
# Use Flagmanager to save a copy of the flags
#
print '--Flagmanager--'
default('flagmanager')

vis = msfile

# Save a copy of the MAIN table flags

mode = 'save'
versionname = 'flagautocorr'
comment = 'flagged autocorr'
merge = 'replace'

flagmanager()

# If you look in the 'jupiter6cm.usecase.ms.flagversions/
# you'll see flags.flagautocorr there along with the
# flags.Original that importuvfits made for you
# Or use

mode = 'list'

flagmanager()

# In the logger you will see something like:
#
# MS : /home/sandrock2/smyers/Testing2/Aug07/jupiter6cm.usecase.ms
# 
# main : working copy in main table
# Original : Original flags at import into CASA
# flagautocorr : flagged autocorr
# See logger for flag versions for this file

#
#=====================================================================
#
# Use Plotxy to interactively flag the data
#
print '--Plotxy--'
default('plotxy')

vis = msfile

# The fields we are interested in: 1331+305,JUPITER,0137+331
selectdata = True

# First we do the primary calibrator
field = '1331+305'

# Plot only the RR and LL for now
correlation = 'RR LL'

# Plot amplitude vs. uvdist
xaxis = 'uvdist'
yaxis = 'amp'
multicolor = 'both'

# The easiest thing is to iterate over antennas
iteration = 'antenna'

plotxy()

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

# You'll see lots of low points as you step through RR LL RL LR
# A basic clip at 0.75 for RR LL and 0.055 for RL LR will work
# If you want to do this interactively, set
iteration = ''

plotxy()

# You can also use flagdata to do this non-interactively
# (see below)

# Now look at the cross-polar products
correlation = 'RL LR'

plotxy()

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

#---------------------------------------------------------------------
# Now do calibrater 0137+331
field = '0137+331'
correlation = 'RR LL'
xaxis = 'uvdist'
spw = ''
iteration = ''
antenna = ''

plotxy()

# You'll see a bunch of bad data along the bottom near zero amp
# Draw a box around some of it and use Locate
# Looks like much of it is Antenna 9 (ID=8) in spw=1

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

xaxis = 'time'
spw = '1'
correlation = ''

# Note that the strings like antenna='9' first try to match the 
# NAME which we see in listobs was the number '9' for ID=8.
# So be careful here (why naming antennas as numbers is bad).
antenna = '9'

plotxy()

# YES! the last 4 scans are bad.  Box 'em and flag.

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

# Go back and clean up
xaxis = 'uvdist'
spw = ''
antenna = ''
correlation = 'RR LL'

plotxy()

# Box up the bad low points (basically a clip below 0.52) and flag

# Note that RL,LR are too weak to clip on.

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

#---------------------------------------------------------------------
# Finally, do JUPITER
field = 'JUPITER'
correlation = ''
iteration = ''
xaxis = 'time'

plotxy()

# Here you will see that the final scan at 22:00:00 UT is bad
# Draw a box around it and flag it!

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

# Now look at whats left
correlation = 'RR LL'
xaxis = 'uvdist'
spw = '1'
antenna = ''
iteration = 'antenna'

plotxy()

# As you step through, you will see that Antenna 9 (ID=8) is often 
# bad in this spw. If you box and do Locate (or remember from
# 0137+331) its probably a bad time.

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

# The easiset way to kill it:

antenna = '9'
iteration = ''
xaxis = 'time'
correlation = ''

plotxy()

# Draw a box around all points in the last bad scans and flag 'em!

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

# Now clean up the rest
xaxis = 'uvdist'
correlation = 'RR LL'
antenna = ''
spw = ''

# You will be drawing many tiny boxes, so remember you can
# use the ESC key to get rid of the most recent box if you
# make a mistake.

plotxy()

# Note that the end result is we've flagged lots of points
# in RR and LL.  We will rely upon imager to ignore the
# RL LR for points with RR LL flagged!

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

#
#=====================================================================
#
# Use Flagmanager to save a copy of the flags so far
#
print '--Flagmanager--'
default('flagmanager')

vis = msfile
mode = 'save'
versionname = 'xyflags'
comment = 'Plotxy flags'
merge = 'replace'

flagmanager()

#
#=====================================================================
#
# You can use Flagdata to explicitly clip the data also
#
print '--Flagdata--'
default('flagdata')

vis = msfile


# Set some clipping regions
mode = 'clip'
clipcolumn = 'DATA'
clipoutside = False 

# Clip calibraters
field = '1331+305'
correlation = 'ABS_RR,LL'
clipminmax = [0.0,0.75]
flagdata()

correlation = 'ABS_RL,LR'
clipminmax = [0.0,0.055]
flagdata()

field = '0137+331'
correlation = 'ABS_RR,LL'
clipminmax = [0.0,0.55]
flagdata()

# You can also do the antenna edits on 0137+331 and JUPITER
# with flagdata

#
#=====================================================================
# Calibration
#=====================================================================
#
# Set the fluxes of the primary calibrator(s)
#
print '--Setjy--'
default('setjy')

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

#
#=====================================================================
#
# Initial gain calibration
#
print '--Gaincal--'
default('gaincal')

vis = msfile

# set the name for the output gain caltable
gtable = prefix + '.gcal'
caltable = gtable

# Gain calibrators are 1331+305 and 0137+331 (FIELD_ID 7 and 0)
# We have 2 IFs (SPW 0,1) with one channel each

# selection is via the field and spw strings
field = '1331+305,0137+331'
spw = ''

# a-priori calibration application
# atmospheric optical depth (turn off)
gaincurve = True
opacity = 0.0

# scan-based G solutions for both amplitude and phase
gaintype = 'G'
solint = 0.
calmode = 'ap'

# reference antenna 11 (11=VLA:N1)
refant = '11'

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

vis = msfile

# set the name for the output rescaled caltable
ftable = prefix + '.fluxscale'
fluxtable = ftable

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

#=====================================================================
#
# Interpolate the gains onto Jupiter (and others)
#
print '--Accum--'
default('accum')

vis = msfile

tablein = ''
incrtable = ftable
calfield = '1331+305, 0137+331'

# set the name for the output interpolated caltable
atable = prefix + '.accum'
caltable = atable

# linear interpolation
interp = 'linear'

# make 10s entries
accumtime = 10.0

accum()

#=====================================================================
#
# Correct the data
# (This will put calibrated data into the CORRECTED_DATA column)
#
print '--ApplyCal--'
default('applycal')

vis = msfile

# Start with the interpolated fluxscale/gain table
bptable = ''
gaintable = atable

# Since we did gaincurve=True in gaincal, we need it here also
gaincurve = True
opacity=0.0

# select the fields
field = '1331+305,0137+331,JUPITER'
spw = ''
selectdata = False

# do not need to select subset since we did accum
# (note that correct only does 'nearest' interp)
gainselect = ''

applycal()

#
#=====================================================================
#
# Now split the Jupiter target data
#
print '--Split Jupiter--'
default('split')

vis = msfile

# Now we write out the corrected data for the calibrator

# Make an output vis file
srcsplitms = prefix + '.split.ms'
outputvis = srcsplitms

# Select the Jupiter field
field = 'JUPITER'
spw = ''

# pick off the CORRECTED_DATA column
datacolumn = 'corrected'

split()

#=====================================================================
#
# Export the Jupiter data as UVFITS
# Start with the split file.
#
print '--Export UVFITS--'
default('exportuvfits')

srcuvfits = prefix + '.split.uvfits'

vis = srcsplitms
fitsfile = srcuvfits

# Since this is a split dataset, the calibrated data is
# in the DATA column already.
datacolumn = 'data'

# Write as a multisource UVFITS (with SU table)
# even though it will have only one field in it
multisource = True

# Run asynchronously so as not to interfere with other tasks
# (BETA: also avoids crash on next importuvfits)
async = True

exportuvfits()

#
#=====================================================================
# FIRST CLEAN / SELFCAL CYCLE
#=====================================================================
#
# Now clean an image of Jupiter
#
print '--Clean 1--'
default('clean')

# Pick up our split source data
vis = srcsplitms

# Make an image root file name
imname1 = prefix + '.clean1'
imagename = imname1

# Set up the output continuum image (single plane mfs)
mode = 'mfs'
stokes = 'I'

# NOTE: current version field='' doesnt work
field = '*'

# Combine all spw
spw = ''

# This is D-config VLA 6cm (4.85GHz) obs
# Check the observational status summary
# Primary beam FWHM = 45'/f_GHz = 557"
# Synthesized beam FWHM = 14"
# RMS in 10min (600s) = 0.06 mJy (thats now, but close enough)

# Set the output image size and cell size (arcsec)
# 4" will give 3.5x oversampling
# 280 pix will cover to 2xPrimaryBeam
# clean will say to use 288 (a composite integer) for efficiency
clnalg = 'clark'
clnimsize = [288,288]

# double for CS Clean
#clnalg = 'csclean'
#clnimsize = [576,576]

clncell = [4.,4.]

alg = clnalg
imsize = clnimsize
cell = clncell

# NOTE: will eventually have an imadvise task to give you this
# information

# Standard gain factor 0.1
gain = 0.1

# Fix maximum number of iterations
niter = 10000

# Also set flux residual threshold (0.04 mJy)
# From our listobs:
# Total integration time = 85133.2 seconds
# With rms of 0.06 mJy in 600s ==> rms = 0.005 mJy
# Set to 10x thermal rms
threshold=0.05

# Note - we can change niter and threshold interactively
# during clean

# Set up the weighting
# Use Briggs weighting (a moderate value, on the uniform side)
weighting = 'briggs'
rmode = 'norm'
robust = 0.5

# No clean mask
mask = ''

# Use interactive clean mode
cleanbox = 'interactive'

# Moderate number of iter per interactive cycle
npercycle = 100

clean()

# When the interactive clean window comes up, use the right-mouse
# to draw rectangles around obvious emission double-right-clicking
# inside them to add to the flag region.  You can also assign the
# right-mouse to polygon region drawing by right-clicking on the
# polygon drawing icon in the toolbar.  When you are happy with
# the region, click 'Done Flagging' and it will go and clean another
# 100 iterations.  When done, click 'Stop'.

# Set up variables
clnimage1 = imname1+'.image'
clnmodel1 = imname1+'.model'
clnresid1 = imname1+'.residual'
clnmask1  = imname1+'.clean_interactive.mask'

#
#---------------------------------------------------------------------
#
# Look at this in viewer
viewer(clnimage1,'image')

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

# New gain table
selfcaltab1 = srcsplitms + '.selfcal1'
caltable = selfcaltab1

# Don't need a-priori cals
selectdata = False
gaincurve = False
opacity = 0.0

# This choice seemed to work
refant = '11'

# Lets do phase-only first time around
gaintype = 'G'
calmode = 'p'

# Do scan-based solutions with SNR>3
solint = 0.0
minsnr = 3.0

# Do not need to normalize (let gains float)
solnorm = False

gaincal()

#
#---------------------------------------------------------------------
#
# Correct the data (no need for interpolation this stage)
#
print '--ApplyCal--'
default('applycal')

vis = srcsplitms

gaintable = selfcaltab1

gaincurve = False
opacity = 0.0
field = ''
spw = ''
selectdata = False

calwt = True

applycal()

# Self-cal is now in CORRECTED_DATA column of split ms
#
#=====================================================================
# SECOND CLEAN / SELFCAL CYCLE
#=====================================================================
#
print '--Clean 2--'
default('clean')

vis = srcsplitms

imname2 = prefix + '.clean2'
imagename = imname2

field = '*'
spw = ''
mode = 'mfs'
gain = 0.1
niter = 10000
threshold=0.04

alg = clnalg
imsize = clnimsize
cell = clncell

weighting = 'briggs'
rmode = 'norm'
robust = 0.5

cleanbox = 'interactive'
npercycle = 100

clean()

# Set up variables
clnimage2 = imname2+'.image'
clnmodel2 = imname2+'.model'
clnresid2 = imname2+'.residual'
clnmask2  = imname2+'.clean_interactive.mask'

#
#---------------------------------------------------------------------
#
# Look at this in viewer
viewer(clnimage2,'image')

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
#---------------------------------------------------------------------
#
# Next self-cal cycle
#
print '--SelfCal 2--'
default('gaincal')

vis = srcsplitms

selfcaltab2 = srcsplitms + '.selfcal2'
caltable = selfcaltab2

selectdata = False
gaincurve = False
opacity = 0.0
refant = '11'

# This time amp+phase on 10s timescales SNR>1
gaintype = 'G'
calmode = 'ap'
solint = 10.0
minsnr = 1.0
solnorm = False

gaincal()

#
# It is useful to put this up in plotcal
#
#---------------------------------------------------------------------
#
print '--PlotCal--'
default('plotcal')

tablein = selfcaltab2
multiplot = True
yaxis = 'amp'

plotcal()

# Use the Next button to iterate over antennas

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

yaxis = 'phase'

plotcal()

#
# You can see it is not too noisy.
#
# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

# Lets do some smoothing anyway.
#
#---------------------------------------------------------------------
#
# Smooth calibration solutions
#
print '--Smooth--'
default('smoothcal')

vis = srcsplitms

tablein = selfcaltab2

smoothcaltab2 = srcsplitms + '.smoothcal2'
caltable = smoothcaltab2

# Do a 30s boxcar average
smoothtype = 'mean'
smoothtime = 30.0

smoothcal()

# If you put into plotcal you'll see the results
# For example, you can grap the inputs from the last
# time you ran plotcal, set the new tablename, and plot!
#run plotcal.last
#tablein = smoothcaltab2
#plotcal()

#
#---------------------------------------------------------------------
#
# Correct the data
#
print '--ApplyCal--'
default('applycal')

vis = srcsplitms

gaintable = smoothcaltab2

gaincurve = False
opacity = 0.0
field = ''
spw = ''
selectdata = False
calwt = True

applycal()

#
#=====================================================================
# THIRD CLEAN / SELFCAL CYCLE
#=====================================================================
#
print '--Clean 3--'
default('clean')

vis = srcsplitms

imname3 = prefix + '.clean3'
imagename = imname3

field = '*'
spw = ''
mode = 'mfs'
gain = 0.1
niter = 10000
threshold=0.04

alg = clnalg
imsize = clnimsize
cell = clncell

weighting = 'briggs'
rmode = 'norm'
robust = 0.5

cleanbox = 'interactive'
npercycle = 100

clean()

# Cleans alot deeper
# You can change the npercycle to larger numbers
# (like 250 or so) as you get deeper also.

# Set up variables
clnimage3 = imname3+'.image'
clnmodel3 = imname3+'.model'
clnresid3 = imname3+'.residual'
clnmask3  = imname3+'.clean_interactive.mask'

#
#---------------------------------------------------------------------
#
# Look at this in viewer
viewer(clnimage3,'image')

# jupiter6cm.usecase.clean3.image     (Jy/beam)
# 
# n           Std Dev     RMS         Mean        Variance    Sum
# 5848        0.001015    0.001015    -4.036e-06  1.029e-06   -0.02360  
# 
# Flux        Med |Dev|   IntQtlRng   Median      Min         Max
# -0.001470   0.0006728   0.001347    8.245e-06   -0.003260   0.003542  
# 
# 
# On Jupiter:
# 
# n           Std Dev     RMS         Mean        Variance    Sum
# 6003        0.08012     0.08107     0.01245     0.006419    74.72     
# 
# Flux        Med |Dev|   IntQtlRng   Median      Min         Max
# 4.653       0.0006676   0.001383    -1.892e-06  -0.002842   1.076     
# 
# Estimated dynamic range = 1.076 / 0.001015 = 1060 (even better!)
#
# Note that the exact numbers you get will depend on how deep you
# take the interactive clean and how you draw the box for the stats.
#
# Greg Taylor got 1600:1 so we still have some ways to go
# This will probably take several more careful self-cal cycles.

# Set up final variables
clnimage = clnimage3
clnmodel = clnmodel3
clnresid = clnresid3
clnmask  = clnmask3

#=====================================================================
#
# Export the Final CLEAN Image as FITS
#
print '--Final Export CLEAN FITS--'
default('exportfits')

clnfits = prefix + '.clean.fits'

imagename = clnimage
fitsimage = clnfits

# Run asynchronously so as not to interfere with other tasks
# (BETA: also avoids crash on next importfits)
async = True

exportfits()

#=====================================================================
#
# Export the Final Self-Calibrated Jupiter data as UVFITS
#
print '--Final Export UVFITS--'
default('exportuvfits')

caluvfits = prefix + '.selfcal.uvfits'

vis = srcsplitms
fitsfile = caluvfits

# The self-calibrated data is in the CORRECTED_DATA column
datacolumn = 'corrected'

# Write as a multisource UVFITS (with SU table)
# even though it will have only one field in it
multisource = True

# Run asynchronously so as not to interfere with other tasks
# (BETA: also avoids crash on next importuvfits)
async = True

exportuvfits()

#
#=====================================================================
# Image Analysis
#=====================================================================
#
# Can do some image statistics if you wish
# Treat this like a regression script
# WARNING: currently requires toolkit
#
print ' Jupiter results '
print ' =============== '

print ''
# Pull the max src amp value out of the MS
ms.open(srcsplitms)
thistest_src = max(ms.range(["amplitude"]).get('amplitude'))
oldtest_src =  4.92000198364
print ' MS max amplitude should be ',oldtest_src
print ' Found : Max in MS = ',thistest_src
diff_src = abs((oldtest_src-thistest_src)/oldtest_src)
print ' Difference (fractional) = ',diff_src

ms.close()

print ''
# Pull the max and rms from the clean image
ia.open(clnimage)
on_statistics=ia.statistics(list=True, verbose=True)
thistest_immax=on_statistics['max'][0]
oldtest_immax = 1.07732224464
print ' Clean image ON-SRC max should be ',oldtest_immax
print ' Found : Max in image = ',thistest_immax
diff_immax = abs((oldtest_immax-thistest_immax)/oldtest_immax)
print ' Difference (fractional) = ',diff_immax

print ''
# Now do stats in the lower right corner of the image
#box = ia.setboxregion([0.75,0.00],[1.00,0.25],frac=true)
box = rg.box([0.75,0.00],[1.00,0.25],frac=true)
off_statistics=ia.statistics(region=box, list=True, verbose=True)
thistest_imrms=off_statistics['rms'][0]
oldtest_imrms = 0.0010449
print ' Clean image OFF-SRC rms should be ',oldtest_imrms
print ' Found : rms in image = ',thistest_imrms
diff_imrms = abs((oldtest_imrms-thistest_imrms)/oldtest_imrms)
print ' Difference (fractional) = ',diff_imrms

print ''
print ' Final Clean image Dynamic Range = ',thistest_immax/thistest_imrms
print ''
print ' =============== '

ia.close()

print ''
print '--- Done ---'

#
#=====================================================================
