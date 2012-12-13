######################################################################
#                                                                    #
# Use Case Script for Jupiter 6cm VLA                                #
#                                                                    #
# Updated STM 2008-05-26 (Beta Patch 2.0)                            #
# Updated STM 2008-06-12 (Beta Patch 2.0) for summer school demo     #
#                                                                    #
# IMPORT AND FLAGGING ONLY SET UP AS DEMO                            #
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
prefix='jupiter6cm.demo'

# Clean up old files
os.system('rm -rf '+prefix+'*')

msfile = prefix + '.ms'

#
#=====================================================================
#
# Get to path to the CASA home and stip off the name
pathname=os.environ.get('CASAPATH').split()[0]

# This is where the UVFITS data should be
#fitsdata=pathname+'/data/demo/jupiter6cm.fits'
# Or
#fitsdata=pathname+'/data/nrao/VLA/planets_6cm.fits'
#
# Can also be found online at
#http://casa.nrao.edu/Data/VLA/Planets6cm/planets_6cm.fits

# Use version in current directory
fitsdata='planets_6cm.fits'

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

print "Use importuvfits to read UVFITS and make an MS"

# Set up the MS filename and save as new global variable
msfile = prefix + '.ms'

print "MS will be called "+msfile

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

print "Use listobs to print verbose summary to logger"

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
# Use Plotxy to interactively flag the data
#
print '--Plotxy--'
default('plotxy')

print "Now we use plotxy to examine and interactively flag data"

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

# Use the field name as the title
selectplot = True
title = field+"  "

iteration = ''

plotxy()

print ""
print "-----------------------------------------------------"
print "Plotxy"
print "Showing 1331+305 RR LL for all antennas"
print "Use MarkRegion then draw boxes around points to flag"
print "You can use ESC to drop last drawn box"
print "When happy with boxes, hit Flag to flag"
print "You can repeat as necessary"

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

# You can also use flagdata to do this non-interactively
# (see below)

# Now look at the cross-polar products
correlation = 'RL LR'

plotxy()

print ""
print "-----------------------------------------------------"
print "Looking at RL LR"
print "Now flag the bad data here"

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

title = field+"  "

plotxy()

# You'll see a bunch of bad data along the bottom near zero amp
# Draw a box around some of it and use Locate
# Looks like much of it is Antenna 9 (ID=8) in spw=1

print ""
print "-----------------------------------------------------"
print "Plotting 0137+331 RR LL all antennas"
print "You see bad data along bottom"
print "Mark a box around a bit of it and hit Locate"
print "Look in logger to see what it is"
print "You see much is Antenna 9 (ID=8) in spw 1"

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

print ""
print "-----------------------------------------------------"
print "Plotting vs. time antenna='9' and spw='1' "
print "Box up last 4 scans which are bad and Flag"

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

print ""
print "-----------------------------------------------------"
print "Back to all data"
print "Clean up remaining bad points"

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

#---------------------------------------------------------------------
# Finally, do JUPITER
field = 'JUPITER'
correlation = 'RR LL'
iteration = ''
xaxis = 'uvdist'

title = field+"  "

plotxy()

# Here you will see that the final scan at 22:00:00 UT is bad
# Draw a box around it and flag it!

print ""
print "-----------------------------------------------------"
print "Now plot JUPITER versus uvdist"
print "Lots of bad stuff near bottom"
print "Lets go and find it - try Locate"
print "Looks like lots of different antennas but at same time"

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

correlation = ''
xaxis = 'time'

plotxy()

# Here you will see that the final scan at 22:00:00 UT is bad
# Draw a box around it and flag it!

print ""
print "-----------------------------------------------------"
print "Now plotting vs. time"
print "See bad scan at end - flag it!"

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

print ""
print "-----------------------------------------------------"
print "Looking now at SPW 1"
print "Now we set iteration to Antenna"
print "Step through antennas with Next"
print "See bad Antenna 9 (ID 8) as in 0137+331"

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

print ""
print "-----------------------------------------------------"
print "Now plotting vs. time antenna 9 spw 1"
print "Box up the bad scans and Flag"

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

print ""
print "-----------------------------------------------------"
print "Final cleanup of JUPITER data"
print "Back to uvdist plot, see remaining bad data"
print "You can draw little boxes around the outliers and Flag"
print "Depends how patient you are in drawing boxes!"
print "Could also use Locate to find where they come from"

# Pause script if you are running in scriptmode
if scriptmode:
    user_check=raw_input('Return to continue script\n')

print "Done with plotxy!"

#
#=====================================================================
#
# Use Flagmanager to save a copy of the flags so far
#
print '--Flagmanager--'
default('flagmanager')

print "Now will use flagmanager to save a copy of the flags we just made"
print "These are named xyflags"

vis = msfile
mode = 'save'
versionname = 'xyflags'
comment = 'Plotxy flags'
merge = 'replace'

flagmanager()

#=====================================================================
#
# Use Flagmanager to list all saved versions
#
print '--Flagmanager--'
default('flagmanager')

print "Now will use flagmanager to list all the versions we saved"

vis = msfile
mode = 'list'

flagmanager()

#
# Done Flagging
print '--Done with flagging--'

#
#=====================================================================
