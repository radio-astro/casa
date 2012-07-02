###################################################################################
##
##  Example script: iterateOnCorr.py
##  Author:         Shannon Jaeger
##  Description:    Plot all uv dist for all each correlation separately.
##
##  Usage:         -Change the filename in the "mp.open" command to the filename
##                  of your measurment set.
##                 -Change the list of correlations to a list that corresponds
##                  to what's in your measurement set.
##                 -Change the numPanelRows and numPanelCols to your desired #
##                 -run casapy
##                 -type 'run -i <myDirectory>/iterateOnCorr.py'
##
## COMMENTS:      -Change the plot from 'uvdist' to one of the other plots.
##                
###################################################################################

vis             = "/u/jaeger/casa/testdata/MSs/3C273XC1_tmp.ms";
correlations    = ['rr','rl','lr','ll'];
numPanelRows    = len(correlations);
numPanelColumns = 1;
panelNum        = 1;

corrIndex = 0;
while ( corrIndex < len(correlations ) ):
        for rowNum in range ( 1, numPanelRows+1 ):
            if ( corrIndex > len(correlations) ) :
                 break;

            # Calculate the subplot value which is an integer value
            # 100's column is the number of rows of plot panels
            # 10's column is the number of columns of plot panels
            # 1's column is the panel we want to plot on
            subplot     = (numPanelRows*100)+(numPanelColumns*10)+panelNum;
            title       = "UVDIST --  corr: " + correlations[corrIndex].upper();
            xlabels      = "AMPLITUDE"
            ylabels      = "UVDIST"
            correlation = correlations[corrIndex].upper();
            xaxis       = 'uvdist'

        
            # Plot the data
            plotxy();

            panelNum += 1;
            corrIndex += 1;

        panelNum        = 1;
        
        # Wait before plotting the next batch of data
        junk=raw_input('\nPress return to see the next set of plots >' )
	
mp.done()

