
# ------------------------------------------------------------------------------
# Subsystem module ICD
# ------------------------------------------------------------------------------

# flag.py

# Description:
# ------------
# This module runs the flagging statistics subsystem of the QA system.

# User functions (subsystem and component level):
# -----------------------------------------------
# flag       - This function runs the flagging statistics subsystem of the QA
#              system.
#
# flag_calc  - This function calculates the flagging statistics.
# flag_write - This function writes the flagging statistics to a CASA table.
# flag_plot  - This function calls functions to create flagging statistics
#              plots.

# Non-user functions (unit level):
# --------------------------------
# flag_desc                 - This function returns the description for the flag
#                             statistics table.
#
# flag_plot_antenna         - This function creates the plot for flagging
#                             statistics versus antenna.
#
# flag_plot_baseline        - This function creates the plot for flagging
#                             statistics versus baseline.
# flag_antenna_map          - This function returns the antenna names versus
#                             antenna numbers from the MS antenna table.
# flag_antenna_number       - This function returns the unique antenna 1s and
#                             antenna 2s from the MS main table.
# flag_antenna_position     - This function returns the nominal antenna position
#                             vectors from then antenna table of the MS.
#
# flag_plot_spw_channel     - This function creates the plot for flagging
#                             statistics versus channel per spectral window.
#
# flag_plot_spw_correlation - This function creates the plot for flagging
#                             statistics versus correlation per spectral window.
#
# flag_plot_field           - This function creates the plot for flagging
#                             statistics versus field.
#
# flag_plot_scan            - This function creates the plot for flagging
#                             statistics versus scan.
#
# flag_plot_spw             - This function creates the plot for flagging
#                             statistics versus spectral window.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version created with functions flag(), flag_calc(), and
#               flag_write().
# 2011 Jun 12 - Nick Elias, NRAO
#               Function flag_desc() added.
# 2011 Sep 01 - Nick Elias, NRAO
#               Logging functionality incorporated.
# 2011 Sep 10 - Nick Elias, NRAO
#               Local logging classes employed.
# 2012 Jun 11 - Nick Elias, NRAO
#               Functions flag_plot(), flag_plot_antenna(),
#               flag_plot_baseline(), flag_antenna_map(),
#               flag_baseline_length(), flag_plot_channel(),
#               flag_plot_correlation(), flag_plot_field(),
#               flag_plot_scan(), and flag_plot_spw() added.
# 2012 Jun 18 - Nick Elias, NRAO
#               Function flag_baseline_length() removed because it was too slow.
#               Function flag_antenna_number() added.
# 2012 Jun 19 - Nick Elias, NRAO
#               Function flag_plot_channel() renamed to flag_plot_spw_channel()
#               and function flag_plot_correlation() renamed to
#               flag_plot_spw_correlation().  These name changes were due to the
#               renamed dictionary elements of the test flag tool.  The function
#               flag_antenna_position() was added.
# 2012 Aug 01 - Nick Elias, NRAO
#               Tool calls modified to new format.

# ------------------------------------------------------------------------------

# Imports
# -------

import os
import math

import numpy
import matplotlib.pyplot as pl

try:
    from casac import casac
except:
    import casa

import utility.logs as logs

# ------------------------------------------------------------------------------
# Subsystem user function and ICD
# ------------------------------------------------------------------------------

# flag

# Description:
# ------------
# This function runs the flagging statistics subsystem of the QA system.

# Inputs:
# -------
# in_ms   - This python string contains the name of the input MS.
# out_dir - This python string contains the output directory.
# logobj  - This python variable contains the logging object information
#           ('PYTHON' creates a local python Logger class, 'CASA' creates a
#           local python logger, <python> and <casa> are actual instances of the
#           logger classes; default = 'PYTHON').

# Outputs:
# --------
# If the function finishes successfully, True is returned via the function
# value.  If the function does not finish successfully, it raises an exception.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version.
# 2011 Jun 13 - Nick Elias, NRAO
#               Improved error handling.
# 2011 Jun 20 - Nick Elias, NRAO
#               Raise an exception if the output table already exists.
# 2011 Sep 10 - Nick Elias, NRAO
#               Added logging capability.
# 2011 Jun 11 - Nick Elias, NRAO
#               Added call to flag_plot() function.

# ------------------------------------------------------------------------------

def flag( in_ms, out_dir, logobj='PYTHON' ):

	# Initialize the logger

	root = 'flag.flag'
	level = logs.INFO

	log_local, logger = logs.init( out_dir, root, level, logobj )


	# Print the first log message

	msg = 'Flagging statistics started for ' + in_ms + ' ...'
	origin = root
	logger.info( msg, origin=origin )


	# Check to see if the output file already exists

	out_table_root = os.path.basename( os.path.splitext( in_ms )[0] )
	out_table = out_dir + '/' + out_table_root + '.flag.stats'

	if os.path.exists( out_table ):
		msg = 'Output table ' + out_table + ' already exists ...\n'
		origin = root
		logger.error( msg, origin=origin )
		raise IOError( msg )


	# Calculate the flagging statistics

	flag_stats = flag_calc( in_ms )

	msg = 'Flagging statistics of ' + in_ms + ' calculated ...'
	origin = root
	logger.info( msg, origin=origin )


	# Write the flagging statistics to the table

	status = flag_write( flag_stats, out_table )

	msg = 'Flagging statistics written in ' + out_table + ' ...'
	origin = root
	logger.info( msg, origin=origin )


	# Create the flagging statistics plots

	flag_plots = flag_plot( in_ms, out_dir, flag_stats )

	msg = 'Flagging statistics plots created in ' + out_dir + ' ...'
	origin = root
	logger.info( msg, origin=origin )


        # Create the flagging scores

        flag_scores = flag_score( flag_stats )

        msg = 'Flagging statistics scores calculated ...'
	origin = root
	logger.info( msg, origin=origin )


	# Print the last log message

	msg = 'Flagging statistics finished for ' + in_ms + ' ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Delete the logger, if it was created locally

	if log_local: del logger


        # Return the dictionary containing the flagging statistics, scores, and
        # plots dictionaries
        flag_qa = {'QANUMBERS': flag_stats, \
                    'QASCORES': flag_scores, \
                    'QAPLOTS': flag_plots}

	return flag_qa

# ------------------------------------------------------------------------------
# Component user functions and ICDs
# ------------------------------------------------------------------------------

# flag_calc

# Description:
# ------------
# This function calculates the flagging statistics.

# Inputs:
# -------
# in_ms - This python string contains the name of the input MS.

# Outputs:
# --------
# The dictionary containing the flagging statistics for the MS, returned via
# the function value.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version.
# 2012 Jun 18 - Nick Elias, NRAO
#               Found 'array', 'observation', and 'type' keys in the flag
#               statistics dictionary.  They may be new.  I insured that they
#               are not written to the output file.  I also saved the data from
#               the 'total' and 'flagged' keys in a new 'global' key and then
#               deleted them.
# 2012 Jun 19 - Nick Elias, NRAO
#               The old flag tool has been replaced by the test flag tool.  The
#               'antenna:scan', 'correlation', 'name' and 'type' keys in the
#               flag statistics dictionary were removed.  The test flag tool
#               does not presently calculate statistics versus baseline or
#               channel.

# ------------------------------------------------------------------------------

def flag_calc( in_ms ):

	# Create the local instance of the test flag tool and open it

	try:
            tfLoc = casac.agentflagger()
        except:
	    # This won't work anymore
            tfLoc = casa.__agentflaggerhome__.create()

	tfLoc.open( in_ms )


	# Get the flagging statistics

	tfLoc.selectdata()
	tfLoc.parsesummaryparameters( spwchan=True, spwcorr=True, basecnt=True )
	tfLoc.init()

	flag_stats = tfLoc.run()['report0']

	# Fix the dictionary

	total = flag_stats['total']
	flagged = flag_stats['flagged']

	del flag_stats['total']
	del flag_stats['flagged']

	flag_stats['global'] = dict()
	flag_stats['global']['total'] = total
	flag_stats['global']['flagged'] = flagged

	del flag_stats['antenna:scan']
	del flag_stats['correlation']
	del flag_stats['array']
	del flag_stats['name']
	del flag_stats['observation']
	del flag_stats['type']


	# Close and delete the local test flag tool

	tfLoc.done()

	del tfLoc


	# Return the dictionary containing the flagging statistics

	return flag_stats

# ------------------------------------------------------------------------------

# flag_write

# Description:
# ------------
# This function writes the flagging statistics to a CASA table.

# TBD:
# ---
# A simple-minded approach to write information using the table tool was used.
# There may be a more elegant way to do it.

# Inputs:
# -------
# flag_stats - The python dictionary that contains the flagging statistics.
# out_table  - The python string that contains the output table name.

# Outputs:
# --------
# The flagging statistics table is created in the output directory.  If the
# function finishes successfully, True is returned via the function value.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version (stub).
# 2011 Jun 12 - Nick Elias, NRAO
#               First working version.

# ------------------------------------------------------------------------------

def flag_write( flag_stats, out_table ):

	# Create the local instance of the table tool and create the output
	# table according to the description

        try:
	    tbLoc = casac.table()
        except:
            tbLoc = casa.__tablehome__.create()

	tbLoc.create( out_table, flag_desc() )


	# Put the global flagging statistics into the output table

	row = 0

	total = flag_stats['global']['total']
	flagged = flag_stats['global']['flagged']
	fraction = float( flagged ) / float( total )

	tbLoc.addrows()
	tbLoc.putcol( 'type', 'global', row, 1, 1 )
	tbLoc.putcol( 'id', '', row, 1, 1 )
	tbLoc.putcol( 'total', total, row, 1, 1 )
	tbLoc.putcol( 'flagged', flagged, row, 1, 1 )
	tbLoc.putcol( 'fraction', fraction, row, 1, 1 )


	# Put the rest of the flagging statistics into the output table

	row = 0

	keys1 = sorted( flag_stats )
	keys1.remove( 'global' )

	for k1 in keys1:

		keys2 = sorted( flag_stats[k1] )

		for k2 in keys2:

			total = flag_stats[k1][k2]['total']
			flagged = flag_stats[k1][k2]['flagged']
			fraction = float( flagged ) / float( total )

			row += 1

			tbLoc.addrows()
			tbLoc.putcol( 'type', k1, row, 1, 1 )
			tbLoc.putcol( 'id', k2, row, 1, 1 )
			tbLoc.putcol( 'total', total, row, 1, 1 )
			tbLoc.putcol( 'flagged', flagged, row, 1, 1 )
			tbLoc.putcol( 'fraction', fraction, row, 1, 1 )


	# Close and delete the local table tool

	tbLoc.done()

	del tbLoc


	# Return True

	return True

# ------------------------------------------------------------------------------

# flag_plot

# Description:
# ------------
# This function calls functions to create flagging statistics plots.

# NB: The present flagging statistics are calculated versus antenna, baseline
# length (in meters), channel (per spectral window), correlation (per spectral
# window), field, scan, and spectral window.

# Inputs:
# -------
# in_ms      - This python string contains the name of the input MS.
# out_dir    - This python string contains the output directory.
# flag_stats - The python dictionary that contains the flagging statistics.

# Outputs:
# --------
# The python True value, returned via the function value.

# Modification history:
# ---------------------
# 2012 Jun 11 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def flag_plot( in_ms, out_dir, flag_stats ):

        flag_plots = dict()

	# Create the different types of flag statistics plots

	flag_plots['ANTENNA_PLOT'] = flag_plot_antenna( in_ms, out_dir, flag_stats['antenna'] )

	flag_plots['BASELINE_PLOT'] = flag_plot_baseline( in_ms, out_dir, flag_stats['baseline'] )

	flag_plots['SPW_CHANNEL_PLOT'] = flag_plot_spw_channel( in_ms, out_dir, flag_stats['spw:channel'] )

	flag_plots['SPW_CORRELATION_PLOT'] = flag_plot_spw_correlation( in_ms, out_dir,
	    flag_stats['spw:correlation'] )

	flag_plots['FIELD_PLOT'] = flag_plot_field( in_ms, out_dir, flag_stats['field'] )

	flag_plots['SCAN_PLOT'] = flag_plot_scan( in_ms, out_dir, flag_stats['scan'] )

	flag_plots['SPW_PLOT'] = flag_plot_spw( in_ms, out_dir, flag_stats['spw'] )


        # Return dictionary with plot names
	return flag_plots

# ------------------------------------------------------------------------------
# Unit functions and ICDs
# ------------------------------------------------------------------------------

# flag_desc

# Description:
# ------------
# This function creates the description of the flag statistics table.  The
# description is a python dictionary whose elements correspond to table
# columns.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The python dictionary containing the description of the flag statistics table,
# returned via the function value.

# Modification history:
# ---------------------
# 2011 Jun 12 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def flag_desc():

	# Create the description of the flag statistics table

	desc = dict()

	desc['type'] = {
		'comment': 'Type of flag statistics',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'string'
	}

	desc['id'] = {
		'comment': 'ID for flag statistics',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'string'
	}

	desc['total'] = {
		'comment': 'Total number of data',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'integer'
	}

	desc['flagged'] = {
		'comment': 'Number of flagged data',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'integer'
	}

	desc['fraction'] = {
		'comment': 'Fraction of flagged data',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'float'
	}


	# Return the description of the flag statistics table

	return desc

# ------------------------------------------------------------------------------

# flag_plot_antenna

# Description:
# ------------
# This function creates the plot for flagging statistics versus antenna.

# Inputs:
# -------
# in_ms      - This python string contains the name of the input MS.
# out_dir    - This python string contains the output directory.
# stats_dict - The python dictionary that contains the flagging statistics
#              versus antenna (it is the 'antenna' element of the dictionary
#              returned by the flag_calc() function).

# Outputs:
# --------
# The plot name, returned via the function value.  The plot containing
# the flagging statistics versus antenna; the file name is
# <ms root>.flag.stats.antenna.png.

# Modification history:
# ---------------------
# 2012 Jun 11 - Nick Elias, NRAO
#               Initial version.
# 2013 Jan 22 - Dirk Muders, MPIfR
#               Return plot name

# ------------------------------------------------------------------------------

def flag_plot_antenna( in_ms, out_dir, stats_dict ):

	# Get the antenna names from the antenna table of the MS

	names = flag_antenna_map( in_ms )

	nName = len( names )
	rName = range( nName )


	# Calculate the fraction of flagged data corresponding to each antenna
	# name

	fracs = list()

	for n in names:
		if not stats_dict.has_key( n ): continue
		flagged = stats_dict[n]['flagged']
		total = stats_dict[n]['total']
		fracs.append( flagged / total )


	# Plot the data and set the axes

	pl.plot( rName, fracs, 'bo' )

	pl.axis( [-2, nName+1, 0.0, 1.0] )


	# Add the x and y labels

	pl.xlabel( 'Antenna ID' )
	pl.ylabel( 'Fraction of Flagged Data' )


	# Save the plot to disk

	out_plot_root = os.path.basename( os.path.splitext( in_ms )[0] )
	out_plot = out_dir + '/' + out_plot_root + '.flag.stats.antenna.png'

	pl.savefig( out_plot )
	pl.clf()


	# Return plot name
	return out_plot

# ------------------------------------------------------------------------------

# flag_plot_baseline

# Description:
# ------------
# This function creates the plot for flagging statistics versus baseline.

# Inputs:
# -------
# in_ms      - This python string contains the name of the input MS.
# out_dir    - This python string contains the output directory.
# stats_dict - The python dictionary that contains the flagging statistics
#              versus baseline (it is the 'baseline' element of the dictionary
#              returned by the flag_calc() function).

# Outputs:
# --------
# The plot name, returned via the function value.  The plot containing
# the flagging statistics versus baseline; the file name is
# <ms root>.flag.stats.baseline.png.

# Modification history:
# ---------------------
# 2012 Jun 11 - Nick Elias, NRAO
#               Initial version.
# 2012 Jun 18 - Nick Elias, NRAO
#               Call to function flag_baseline_length() removed because it is
#               too slow.
# 2013 Jan 22 - Dirk Muders, MPIfR
#               Return plot name

# ------------------------------------------------------------------------------

def flag_plot_baseline( in_ms, out_dir, stats_dict ):

	# Get the antenna names versus antenna numbers (a python list used as a
	# map) from the MS antenna table, the lists of unique antenna numbers
	# from the MS main table (just in case antennas as split out), and the
	# antenna positions

	antennaMap = flag_antenna_map( in_ms )
	antennaNum = flag_antenna_number( in_ms )

	antennaPos = flag_antenna_position( in_ms )


	# Create the baseline length and flagging fraction python lists

	bLen = list()
	frac = list()

	for a1 in antennaNum['antenna1']:
		for a2 in antennaNum['antenna2']:

			if a1 >= a2: continue

			bVec = antennaPos[:,a1] - antennaPos[:,a2]
			bLen.append( numpy.sqrt( numpy.sum( bVec**2 ) ) )

			bName = str(antennaMap[a1]) + '&&' + str(antennaMap[a2])
			flagged = stats_dict[bName]['flagged']
			total = stats_dict[bName]['total']
			frac.append( flagged / total )


	# Plot the data and set the axes

	pl.plot( bLen, frac, 'bo' )

	bMin = min( bLen )
	bMax = max( bLen )
	bDelta = 0.05 * ( bMax - bMin )
	pl.axis( [bMin-bDelta, bMax+bDelta, 0.0, 1.0] )


	# Add the x and y labels

	pl.xlabel( 'Baseline Length (meters)' )
	pl.ylabel( 'Fraction of Flagged Data' )


	# Save the plot to disk

	out_plot_root = os.path.basename( os.path.splitext( in_ms )[0] )
	out_plot = out_dir + '/' + out_plot_root + '.flag.stats.baseline.png'

	pl.savefig( out_plot )
	pl.clf()


	# Return plot name
	return out_plot	

# ------------------------------------------------------------------------------

# flag_antenna_map

# Description:
# ------------
# This function returns the antenna names versus antenna numbers from the MS
# antenna table.

# Inputs:
# -------
# in_ms - This python string contains the name of the input MS.

# Outputs:
# --------
# The python list of string containing the antenna names, returned via the
# function value.

# Modification history:
# ---------------------
# 2012 Jun 12 - Nick Elias, NRAO
#               Initial version.
# 2012 Jun 19 - Nick Elias, NRAO
#               The test flag tool does not capitalize, so the corresponding
#               code has been eliminated.

# ------------------------------------------------------------------------------

def flag_antenna_map( in_ms ):

	# Create the local instance of the table tool and open it

        try:
	    tbLoc = casac.table()
        except:
            tbLoc = casa.__tablehome__.create()

	tbLoc.open( in_ms + '/ANTENNA' )


	# Get the antenna names versus the antenna numbers

	names = tbLoc.getcol( 'NAME' ).tolist()


	# Close and delete the local table tool

	tbLoc.close()
	del tbLoc


	# Return the antenna names

	return( names )

# ------------------------------------------------------------------------------

# flag_antenna_number

# Description:
# ------------
# This function returns the unique antenna 1s and antenna 2s from the MS main
# table.

# Inputs:
# -------
# in_ms - This python string contains the name of the input MS.

# Outputs:
# --------
# The python dictionary containing the unique antenna numbers, returned via the
# function value.  The keys:
# 'antenna1' - The python int list containing the unique antenna 1s.
# 'antenna2' - The python int list containing the unique antenna 2s.

# Modification history:
# ---------------------
# 2012 Jun 18 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def flag_antenna_number( in_ms ):

	# Create the local instance of the table tool and open it

        try:
	    tbLoc = casac.table()
        except:
            tbLoc = casa.__tablehome__.create()

	tbLoc.open( in_ms )


	# Get the unique antenna 1s and antenna2 from the main table

	d = dict()

	d['antenna1'] = numpy.unique( tbLoc.getcol( 'ANTENNA1' ) ).tolist()
	d['antenna2'] = numpy.unique( tbLoc.getcol( 'ANTENNA2' ) ).tolist()


	# Close and delete the local table tool

	tbLoc.close()
	del tbLoc


	# Return the dictionary containing the antenna numbers

	return d

# ------------------------------------------------------------------------------

# flag_antenna_position

# Description:
# ------------
# This function returns the nominal antenna position vectors from then antenna
# table of the MS.

# NB: The antenna position vectors are measured with respect to the center of
# the earth, but this extra distance cancels when baseline vectors are
# calculated.

# Inputs:
# -------
# in_ms - This python string contains the name of the input MS.

# Outputs:
# --------
# The numpy array of floats containing the antenna positions with dimensions
# 3 x numAntenna, returned via the function value.

# Modification history:
# ---------------------
# 2012 Jun 19 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def flag_antenna_position( in_ms ):

	# Create the local instance of the table tool and open it

        try:
	    tbLoc = casac.table()
        except:
            tbLoc = casa.__tablehome__.create()

	tbLoc.open( in_ms + '/ANTENNA' )


	# Get the antenna position vectors

	position = tbLoc.getcol( 'POSITION' )


	# Close and delete the local table tool

	tbLoc.close()
	del tbLoc


	# Return the antenna position vectors

	return position

# ------------------------------------------------------------------------------

# flag_plot_spw_channel

# Description:
# ------------
# This function creates the plot for flagging statistics versus channel per
# spectral window.

# Inputs:
# -------
# in_ms      - This python string contains the name of the input MS.
# out_dir    - This python string contains the output directory.
# stats_dict - The python dictionary that contains the flagging statistics
#              versus channel per spectral window (it is the 'channel' element
#              of the dictionary returned by the flag_calc() function).

# Outputs:
# --------
# The plot names, returned via the function value.  The plot containing
# the flagging statistics versus antenna; the file name is
# <ms root>.flag.stats.channel.spw.<#>.png.

# Modification history:
# ---------------------
# 2012 Jun 11 - Nick Elias, NRAO
#               Initial version.
# 2013 Jan 22 - Dirk Muders, MPIfR
#               Return plot names.

# ------------------------------------------------------------------------------

def flag_plot_spw_channel( in_ms, out_dir, stats_dict ):

	# Group the channels according to spectral window and calculate the
	# corresponding fractions of flagged data

        plot_names = dict()

	names = stats_dict.keys()

	channels = dict()
	fracs = dict()

	for n in names:

		s,c = n.split( ':' )

		if not channels.has_key(s):
			channels[s] = list()
			fracs[s] = list()

		channels[s].append( int(c) )

		flagged = stats_dict[n]['flagged']
		total = stats_dict[n]['total']
		fracs[s].append( flagged / total )


	# For each spectral window: plot the data, set the axes, add the x and
	# y lables, and save the plots to disk

	spws = channels.keys()

	for s in spws:

		pl.plot( channels[s], fracs[s], 'bo' )

		pl.axis( [-2, len(channels[s])+1, 0.0, 1.0] )

		pl.xlabel( 'Channel Number (Spectral Window ' + s + ')' )
		pl.ylabel( 'Fraction of Flagged Data' )

		out_plot_root = os.path.basename( os.path.splitext( in_ms )[0] )

		out_plot = out_dir + '/' + out_plot_root
		out_plot += '.flag.stats.channel.spw.' + s + '.png'

		pl.savefig( out_plot )
		pl.clf()

                plot_names[s] = out_plot

	# Return plot names
	return plot_names

# ------------------------------------------------------------------------------

# flag_plot_spw_correlation

# Description:
# ------------
# This function creates the plot for flagging statistics versus correlation.

# Inputs:
# -------
# in_ms      - This python string contains the name of the input MS.
# out_dir    - This python string contains the output directory.
# stats_dict - The python dictionary that contains the flagging statistics
#              versus correlation (it is the 'correlation' element of the
#              dictionary returned by the flag_calc() function).

# Outputs:
# --------
# The plot names , returned via the function value.  The plot containing
# the flagging statistics versus correlation; the file name is
# <ms root>.flag.stats.correlation.spw.<#>.png.

# Modification history:
# ---------------------
# 2012 Jun 11 - Nick Elias, NRAO
#               Initial version.
# 2013 Jan 22 - Dirk Muders, MPIfR
#               Return plot names.

# ------------------------------------------------------------------------------

def flag_plot_spw_correlation( in_ms, out_dir, stats_dict ):

	# Group the correlations according to spectral window and calculate the
	# corresponding fractions of flagged data

        plot_names = dict()

	names = stats_dict.keys()

	correlations = dict()
	fracs = dict()

	for n in names:

		s,c = n.split( ':' )

		if not correlations.has_key(s):
			correlations[s] = list()
			fracs[s] = list()

		correlations[s].append( c )

		flagged = stats_dict[n]['flagged']
		total = stats_dict[n]['total']
		fracs[s].append( flagged / total )


	# For each spectral window: plot the data, set the axes, add the x and
	# y lables, and save the plots to disk

	spws = correlations.keys()

	for s in spws:

		corrNums = range( len(correlations[s]) )
		pl.plot( corrNums, fracs[s], 'bo' )
		pl.xticks( corrNums, correlations[s], rotation=45 )

		pl.axis( [-2, len(corrNums)+1, 0.0, 1.0] )

		pl.xlabel( 'Correlation Number (Spectral Window ' + s + ')' )
		pl.ylabel( 'Fraction of Flagged Data' )

		out_plot_root = os.path.basename( os.path.splitext( in_ms )[0] )

		out_plot = out_dir + '/' + out_plot_root
		out_plot += '.flag.stats.correlation.spw.' + s + '.png'

		pl.savefig( out_plot )
		pl.clf()

                plot_names[s] = out_plot

	# Return plot names
	return plot_names

# ------------------------------------------------------------------------------

# flag_plot_field

# Description:
# ------------
# This function creates the plot for flagging statistics versus field.

# Inputs:
# -------
# in_ms      - This python string contains the name of the input MS.
# out_dir    - This python string contains the output directory.
# stats_dict - The python dictionary that contains the flagging statistics
#              versus field (it is the 'field' element of the dictionary
#              returned by the flag_calc() function).

# Outputs:
# --------
# The plot name, returned via the function value.  The plot containing
# the flagging statistics versus field; the file name is
# <ms root>.flag.stats.field.png.

# Modification history:
# ---------------------
# 2012 Jun 11 - Nick Elias, NRAO
#               Initial version.
# 2013 Jan 22 - Dirk Muders, MPIfR
#               Return plot name

# ------------------------------------------------------------------------------

def flag_plot_field( in_ms, out_dir, stats_dict ):

	# Get the field names

	names = stats_dict.keys()

	nName = len( names )
	rName = range( nName )


	# Calculate the fraction of flagged data corresponding to each field
	# name

	fracs = list()

	for n in names:
		flagged = stats_dict[n]['flagged']
		total = stats_dict[n]['total']
		fracs.append( flagged / total )


	# Plot the data and set the axes

	pl.plot( rName, fracs, 'bo' )

        pl.axis( [-2, nName+1, 0.0, 1.0] )


	# Add the x and y labels

	pl.xlabel( 'Field Number' )
	pl.ylabel( 'Fraction of Flagged Data' )


	# Save the plot to disk

	out_plot_root = os.path.basename( os.path.splitext( in_ms )[0] )
	out_plot = out_dir + '/' + out_plot_root + '.flag.stats.field.png'

	pl.savefig( out_plot )
	pl.clf()


	# Return plot name
	return out_plot

# ------------------------------------------------------------------------------

# flag_plot_scan

# Description:
# ------------
# This function creates the plot for flagging statistics versus scan.

# Inputs:
# -------
# in_ms      - This python string contains the name of the input MS.
# out_dir    - This python string contains the output directory.
# stats_dict - The python dictionary that contains the flagging statistics
#              versus scan (it is the 'scan' element of the dictionary returned
#              by the flag_calc() function).

# Outputs:
# --------
# The plot name, returned via the function value.  The plot containing
# the flagging statistics versus scan; the file name is
# <ms root>.flag.stats.scan.png.

# Modification history:
# ---------------------
# 2012 Jun 11 - Nick Elias, NRAO
#               Initial version.
# 2013 Jan 22 - Dirk Muders, MPIfR
#               Return plot name

# ------------------------------------------------------------------------------

def flag_plot_scan( in_ms, out_dir, stats_dict ):

	# Get and sort the scan number

	numbers = stats_dict.keys()

	nNumber = len( numbers )
	rNumber = range( nNumber )


	# Calculate the fraction of flagged data corresponding to each scan
	# number

	fracs = list()

	for n in numbers:
		flagged = stats_dict[n]['flagged']
		total = stats_dict[n]['total']
		fracs.append( flagged / total )


	# Plot the data and set the axes

	pl.plot( rNumber, fracs, 'bo' )

	pl.axis( [-2, nNumber+1, 0.0, 1.0] )


	# Add the x and y labels

	pl.xlabel( 'Scan Number' )
	pl.ylabel( 'Fraction of Flagged Data' )


	# Save the plot to disk

	out_plot_root = os.path.basename( os.path.splitext( in_ms )[0] )
	out_plot = out_dir + '/' + out_plot_root + '.flag.stats.scan.png'

	pl.savefig( out_plot )
	pl.clf()


	# Return plot name
	return out_plot

# ------------------------------------------------------------------------------

# flag_plot_spw

# Description:
# ------------
# This function creates the plot for flagging statistics versus spectral window.

# Inputs:
# -------
# in_ms      - This python string contains the name of the input MS.
# out_dir    - This python string contains the output directory.
# stats_dict - The python dictionary that contains the flagging statistics
#              versus spectral window (it is the 'spw' element of the dictionary
#              returned by the flag_calc() function).

# Outputs:
# --------
# The plot name, returned via the function value.  The plot containing
# the flagging statistics versus spectral window; the file name is
# <ms root>.flag.stats.spw.png.

# Modification history:
# ---------------------
# 2012 Jun 11 - Nick Elias, NRAO
#               Initial version.
# 2013 Jan 22 - Dirk Muders, MPIfR
#               Return plot name

# ------------------------------------------------------------------------------

def flag_plot_spw( in_ms, out_dir, stats_dict ):

	# Get and sort the spectral window number

	numbers = stats_dict.keys()

	nNumber = len( numbers )
	rNumber = range( nNumber )


	# Calculate the fraction of flagged data corresponding to each spectral
	# window number

	fracs = list()

	for n in numbers:
		flagged = stats_dict[n]['flagged']
		total = stats_dict[n]['total']
		fracs.append( flagged / total )


	# Plot the data and set the axes

	pl.plot( rNumber, fracs, 'bo' )

	pl.axis( [-2, nNumber+1, 0.0, 1.0] )


	# Add the x and y labels

	pl.xlabel( 'Spectral Window Number' )
	pl.ylabel( 'Fraction of Flagged Data' )


	# Save the plot to disk

	out_plot_root = os.path.basename( os.path.splitext( in_ms )[0] )
	out_plot = out_dir + '/' + out_plot_root + '.flag.stats.spw.png'

	pl.savefig( out_plot )
	pl.clf()


	# Return plot name
	return out_plot

# ------------------------------------------------------------------------------

# flag_score

# Description:
# ------------
# This function calculates the flagging statistics scores

# Inputs:
# -------
# flag_stats - The python dictionary that contains the flagging statistics.

# Outputs:
# --------
# A python dictionary with flagging scores.

# Modification history:
# ---------------------
# 2012 Nov 06 - Dirk Muders, MPIfR
#               Initial version

# ------------------------------------------------------------------------------

def flag_score( flag_stats ):

    flag_scores = dict()

    # Calculate grand total score
    globalScore = 1.0

    globalFraction = 0.0
    numPoints = 0
    antennaPairs = [item.split('&&') for item in flag_stats['baseline'].iterkeys()]
    for antenna1, antenna2 in antennaPairs:
        # Count only real flags, no auto-correlations
        if (antenna1 != antenna2):
            globalFraction += flag_stats['baseline']['%s&&%s' % (antenna1, antenna2)]['flagged'] / flag_stats['baseline']['%s&&%s' % (antenna1, antenna2)]['total']
            numPoints += 1
    globalFraction /= numPoints
    globalScore -= globalFraction
    flag_scores['TOTAL'] = globalScore


    # Calculate individual scores

    flag_scores['AVERAGE'] = dict()
    flag_scores['AVERAGE']['antenna'] = 1.0 - numpy.average(numpy.array([fs_antenna['flagged'] / fs_antenna['total'] for fs_antenna in flag_stats['antenna'].itervalues()]))
    flag_scores['AVERAGE']['baseline'] = 1.0 - numpy.average(numpy.array([fs_baseline['flagged'] / fs_baseline['total'] for fs_baseline in flag_stats['baseline'].itervalues()]))

    flag_scores['ANTENNA'] = dict()
    for antennaKey in flag_stats['antenna'].iterkeys():
        flag_scores['ANTENNA'][antennaKey] = 1.0 - flag_stats['antenna'][antennaKey]['flagged'] / flag_stats['antenna'][antennaKey]['total']


    flag_scores['BASELINE'] = dict()
    for baselineKey in flag_stats['baseline'].iterkeys():
        flag_scores['BASELINE'][baselineKey] = 1.0 - flag_stats['baseline'][baselineKey]['flagged'] / flag_stats['baseline'][baselineKey]['total']

    return flag_scores
