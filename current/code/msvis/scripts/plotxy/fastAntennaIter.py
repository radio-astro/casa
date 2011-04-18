###################################################################################
##
##  Example script: antIterAvecPlotxyFast.py
##  Author:         Shannon Jaeger
##  Description:    overplot the channel plot for each antenna over the channel
##                  plot for all antenna's.  It uses the removetopplot parameter
##                  so we don't need to replot the bottom plot.
##
##  Usage:         -Change the filename (myMS) to  your measurment set.
##                 -run casapy
##                 -type 'run -i <myDirectory>/antIterAvecPlotxy.py
##
## COMMENTS:       -Comment change the field,spw, and other data selection
##                  parameters to refine what is plotted.
##                 -This example plots the corrected data, but instead of
##                  corrected the residual, data, imaginary, or reall data
##                  can be plotted
##                  iterate over
##                 -Change the plot from 'channel' to one of the other plots, such
##                  as 'uvdist', See the plotxy documentation for the full
##                  range of accepted plots
##                 -Change the colors of the plot.  This example plot the full
##                  'channel' plot in forestgreen, and plots the individual
##                  attena's to coral.
##
###################################################################################

myMS='/Users/shannonjaeger/casa/testdata/measurementSets/ngc5921.ms';

# Plot channels for all antennas
plotxy( vis=myMS,xaxis='channel',yaxis='amp', datacolumn='corrected', field='0', spw='0', plotcolor='forestgreen');

for ant in range(0,28):
    # Plot channels for the current antenna
    # If it is the first antenna, then we just plot it, otherwise we want to
    # replace the last plot we made with the current plot.  We do this by
    # setting replacetopplot to true.
    myTitle = 'Channels, Antenna Index: ' + str(ant);
    if ( ant > 0 ):
        plotxy( vis=myMS, xaxis='channel', yaxis='amp', datacolumn='corrected', field='0', spw='0', plotcolor='coral', overplot = true, title = myTitle, antenna=str(ant), replacetopplot=true );
    else :
        plotxy( vis=myMS, xaxis='channel', yaxis='amp', datacolumn='corrected', field='0', spw='0', plotcolor='coral', overplot = true, title = myTitle, antenna=str(ant) );
    
    # Wait before plotting the next batch of data
    junk=raw_input('\nPress return to see the next set of plots >' )

mp.done()
