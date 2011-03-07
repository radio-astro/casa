###################################################################################
##
##  Example script: iterateOnAntAndCorr.py
##  Author:         Shannon Jaeger
##  Description:    Plot all uv dist for all each spectral window and
##                  each correlation separately.
##
##  Usage:         -Change the filename in the "mp.open" command to the filename
##                  of your measurment set.
##                 -Change the value of numAntennas to number of antennas 
##                  used to acquire the measurement set
##                 -Change the list of correlations to a list that corresponds
##                  to what's in your measurement set.
##                 -Change the numPanelRows 
##                 -run casapy
##                 -type 'run -i <myDirectory>/antIteration.py
##
## COMMENTS:      -Change the plot from 'uvdist' to one of the other plots.
##                
###################################################################################

mp.open( "~/casa/testdata/3C273XC1.ms" );
numAntennas=28;
correlations=['rr','rl','lr','ll'];
numPanelColumns=len(correlations);
numPanelRows=2;

antIndex=1;
while ( antIndex < numAntennas ):
    # Keep track of which panel we are plotting on with this variable.
    panelNum = 1;

    for rowNum in range ( 1, numPanelRows+1 ):
        for corrIndex in range ( 0, len(correlations) ):
            # Calculate the subplot value which is an integer value
            # 100's column is the number of rows of plot panels
            # 10's column is the number of columns of plot panels
            # 1's column is the panel we want to plot on
            subplotVal = (numPanelRows*100)+(numPanelColumns*10)+panelNum;
            titleStr = "Ant " + str(antIndex) \
            	+ "/ corr " + correlations[corrIndex];
            mp.plotoptions( subplot=subplotVal, title=titleStr, xlabel="AMP", ylabel="UVDist" );
		
            # Plot the current SPW
            mp.setdata( baseline=str(antIndex), correlation=correlations[corrIndex] );
            mp.plot( 'uvdist' )
            panelNum+=1
        antIndex+=1

    # Wait before plotting the next batch of data
    junk=raw_input('\nPress return to see the next set of plots >' )
	
mp.done()

