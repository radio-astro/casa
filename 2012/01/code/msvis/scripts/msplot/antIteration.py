###################################################################################
##
##  Example script: antIteration.py
##  Author:         Urvashi Venkata
##  Description:    overplot the uvdist plot for each antenna over the uvdist
##                  plot for all antenna's combined on the first field and the
##                  channels 4 to 50 only.
##
##  Usage:         -Change the filename in the "mp.open" command to the filename
##                  of your measurment set.
##                 -run casapy
##                 -type 'run -i <myDirectory>/antIteration.py
##
## COMMENTS:       -Comment out the "mp.setdata" and "mp.setspectral()" calls if
##                  you'd prefer not to make any data selection, or change the
##                  values.  See the msplot tool description for more information
##                  on the usage of "mp.setdata"
##                 -Change the "range(1,10)" to the antenna range you wish to
##                  iterate over
##                 -Change the plot from 'uvdist' to one of the other plots, such
##                  as 'vischannel', See the msplot tool documentation for the full
##                  range of accepted plots
##                 -Change the colors of the plot.  This example plot the full
##                  'uvdist' plot in darksalmon, and plots the individual antennas
##                  to blue.
##
###################################################################################

mp.open('~/casa/testdata/ngc5921.ms');
mp.setdata( field='0',spw='>0:4~40' );
mp.plotoptions( overplot=False, plotsymbol=',', plotcolor='darksalmon' );
mp.plot( 'uvdist' )
mp.plotoptions( overplot=True, plotsymbol='b,');
for ant in range(0,27):
    mp.setdata( field='0', spw='>0:4~40', baseline=str(ant) );
    mp.plot( 'uvdist' );
    mp.plotoptions( overplot=True, replacetopplot=True, plotsymbol='b,' );

