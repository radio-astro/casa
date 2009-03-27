###################################################################################
##
##  Example script: multipleSpwPlot.py
##  Author:         Shannon Jaeger
##  Description:    Plot all visible channels for all spectral windows.  
##
##  Usage:         -Change the filename in the "mp.open" command to the filename
##                  of your measurment set.
##                 -Change the value of maxSPW to the index of the last spectral
##                  window in your measurement set.
##                 -run casapy
##                 -type 'run -i <myDirectory>/antIteration.py
##
## COMMENTS:      -Change the plot from 'vischannel' to one of the other plots, such
##                  as 'uvdist', See the msplot tool documentation for the full
##                  range of accepted plots
##                -Set a color in the plotoption call. The way this script works
##                 each SPW will be displayed in a different color. If a single
##                 color is desired then the color should be set in the
##                 mp.plotoption call
##                -The added ":30~100" on the spw specifiation, selects channels
##                 30 through to 100
##
###################################################################################

mp.open( "~/casa/testdata/ggtau_07feb97.ms" );
maxSpw=23
for spwId in range( 0,23 ):
    mp.plotoptions( title="Vischannel for all SPWs", overplot=True );
    spwString=str(spwId) + ":30~100";

    # Set data might fail if there are less then 30 channels for the
    # particular SPW we are ploting.
    if ( mp.setdata( spw=spwString ) ) :
        mp.plot( 'vischannel' )

mp.done()
