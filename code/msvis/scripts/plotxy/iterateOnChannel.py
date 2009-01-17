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
## COMMENTS:      -Change the plot from 'channel' to one of the other plots.
##                -Channel selection is done via the "spw" parameter.
##                
###################################################################################

vis             = "/u/jaeger/casa/testdata/MSs/ggtau_07feb97.ms";
numPanelRows    = 3;
numPanelColumns = 2;

# This is the number of channels for each SPW.
numChannels     = [1,1,64,64,1,1,256,256,1,1,64,64,1,1,64,64,1,1,64,64,1,1,256,256]


chanIndex = 0;
panelNum  = 1;
while( chanIndex < numChannels ):
	for rowNum in range ( 1, numPanelRows+1 ):
		for colNum in range( 1, numPanelColumns+1 ):
			if ( chanIndex > numChannels ) :
				break;
		
			# Calculate the subplot value which is an integer value
			# 100's column is the number of rows of plot panels
			# 10's column is the number of columns of plot panels
			# 1's column is the panel we want to plot on
			subplot     = (numPanelRows*100)+(numPanelColumns*10)+panelNum;
			title       = "UV dist, chan: " + str(chanIndex);
			xlabel      = "UV distance";
			ylabel      = "amp";

			# Construct the SPW selection string.  We want only
			# SPWs with more channels then the current channel
			# number we are plotting.
			spw = ""
			for spwIndex in range( 0, len(numChannels) ):
				if ( numChannels[spwIndex] > chanIndex ) :
					if ( len(spw) < 1 ):
						spw = str(spwIndex);
					else:
						spw = spw + "," + str(spwIndex);
			spw	    = spw + ":" + str(chanIndex);
			xaxis       = 'uvdist'
        
			# Plot the data
			plotxy();

			panelNum += 1;
			chanIndex += 1;

	panelNum        = 1;
        
        # Wait before plotting the next batch of data
        junk=raw_input('\nPress return to see the next set of plots >' )
	
mp.done()

