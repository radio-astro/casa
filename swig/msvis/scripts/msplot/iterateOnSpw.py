###################################################################################
##
##  Example script: iterateOnSpw.py
##  Author:         Shannon Jaeger
##  Description:    Plot all uv dist for all spectral window separately.  
##
##  Usage:         -Change the filename in the "mp.open" command to the filename
##                  of your measurment set.
##                 -Change the value of maxSPW to the index of the last spectral
##                  window in your measurement set.
##                 -Change the numPanelRows and numPanelColumns to the desired
##                  number of rows and columns.
##                 -run casapy
##                 -type 'run -i <myDirectory>/antIteration.py
##
## COMMENTS:      -Change the plot from 'uvdist' to one of the other plots.
##                
###################################################################################

mp.open( "~/casa/testdata/ggtau_07feb97.ms" );
maxSpw=23;
numPanelRows=3;
numPanelColumns=2
spwId=0;

while ( spwId < maxSpw ):
	for panelNum in range ( 1, (numPanelRows*numPanelColumns)+1 ):
		# Calculate the subplot value which is an integer value
		# 100's column is the number of rows of plot panels
		# 10's column is the number of columns of plot panels
		# 1's column is the panel we want to plot on
	 	subplotVal = (numPanelRows*100)+(numPanelColumns*10)+panelNum
		titleStr = "uvDist for SPW:" + str(spwId)
		mp.plotoptions( subplot=subplotVal, title=titleStr );
		
		# Plot the current SPW
		mp.setdata( spw=str(spwId) );
		mp.plot( 'uvdist' )
		spwId+=1

	# Wait before plotting the next batch of data
	junk=raw_input('\nPress return to see the next set of plots >' )
	
mp.done()
