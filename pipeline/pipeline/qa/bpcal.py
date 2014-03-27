
# ------------------------------------------------------------------------------
# Subsystem module ICD
# ------------------------------------------------------------------------------

# bpcal.py

# Description:
# ------------
# This module runs the bandpass calibration statistics subsystem of the QA
# system.

# TBD:
# ----
# * Make bpcal_putFitRow() a more generic function.

# User Functions (subsystem and component level):
# -----------------------------------------------
# bpcal       - This function runs the bandpass calibration statistics subsystem
#               of the QA system.
#
# bpcal_calc  - This function calculates the bandpass calibration statistics.
# bpcal_write - This function writes the bandpass calibration statistics to a
#               CASA table.

# Non-user functions (unit level):
# --------------------------------
# bpcal_putFitRow     - This function puts a row of data into the output table.
#
# bpcal_desc          - This function returns the place holder data description
#                       dictionary.
# bpcal_desc_st       - This function creates the bandpass calibration subtable
#                       data description dictionary.
#
# bpcal_chanRangeList - This function returns the trimmed channel range.
# bpcal_spwChanString - This function converts spectral window and channel range
#                       lists to a CASA selection string.
#
# bpcal_score         - This function calculates the score to prioritize the
#                       plots on the HTML page.
# bpcal_score_flag    - This function calculates the flag score for a given
#                       iteration.
# bpcal_score_RMS     - This function calculates the RMS score for a given
#                       iteration.
# bpcal_score_delay   - This function calculates the delay score for a given
#                       iteration.
#
# bpcal_plot          - This function is the top-level interface to other
#                       functions that create bandpass calibration plots.
# bpcal_plot1         - This function creates a single bandpass statistics plot
#                       and saves it to disk.
# bpcal_plot_hist     - This function plots the histograms of either number of
#                       flagged data, RMSes, or absolute value of delays, color
#                       coded by spectral window.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version created with functions bpcal(), bpcal_calc(),
#               and bpcal_write().
# 2012 Mar 09 - Nick Elias, NRAO
#               Functions bpcal_desc() and bpcal_desc_st() added.
# 2012 Mar 16 - Nick Elias, NRAO
#               Function bpcal_putFitRow() added.
# 2012 May 11 - Nick Elias, NRAO
#               Major revison using new version of calibration analysis tool.
# 2012 May 15 - Nick Elias, NRAO
#               Functions bpcal_chanRangeList() and bpcal_spwChanString() added.
# 2012 Jun 20 - Nick Elias, NRAO
#               Functions bpcal_plot(), bpcal_plot1(), and bpcal_plot_hist()
#               added.
# 2012 Jun 23 - Nick Elias, NRAO
#               Functions bpcal_html() and bpcal_score() added.
# 2012 Aug 01 - Nick Elias, NRAO
#               Tool calls modified to new format.
# 2012 Aug 20 - Nick Elias, NRAO
#               Functions bpcal_score_flag(), bpcal_score_RMS(), and
#               bpcal_score_delay() added.
# 2012 Aug 21 - Nick Elias, NRAO
#               Function bpcal_html() removed.

# ------------------------------------------------------------------------------

# Imports
# -------

import math
import os
import string

import numpy
import scipy
import scipy.stats.mstats
import matplotlib.pyplot as pl

try:
    from casac import casac
except:
    import casa
    import casac

import utility.logs as logs

def rms(data):
    return numpy.sqrt(numpy.sum(data**2) / len(data))

# ------------------------------------------------------------------------------
# Subsystem user function and ICD
# ------------------------------------------------------------------------------

# bpcal

# Description:
# ------------
# This function runs the bandpass calibration statistics subsystem of the QA
# system.

# Inputs:
# -------
# in_table - This python string contains the name of the input bandpass
#            calibration table.
# out_dir  - This python string contains the output directory.
# logobj   - This python variable contains the logging object information
#            ('PYTHON' creates a local python Logger class, 'CASA' creates a
#            local python logger, <python> and <casa> are actual instances of
#            the logger classes; default = 'PYTHON').

# Outputs:
# --------
# If the function finishes successfully, a tuple containing the bandpass
# statistics, scores, and plots dictionaries is returned via the function value.
# If the function does not finish successfully, it throws an exception.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version.
# 2011 Jun 13 - Nick Elias, NRAO
#               Improved error handling.
# 2011 Jun 20 - Nick Elias, NRAO
#               Raise an exception if the output table already exists.
# 2012 Mar 09 - Nick Elias, NRAO
#               The logobj input parameter and logging code has been added.
# 2012 Mar 15 - Nick Elias, NRAO
#               Added the spw_in input parameter.
# 2012 May 09 - Nick Elias, NRAO
#               Removed the spw_in input parameter.
# 2012 Jun 22 - Nick Elias, NRAO
#               Added call to bpcal_plot() function.
# 2012 Aug 21 - Nick Elias, NRAO
#               Added call to bpcal_score() function.  Return value changed to
#               a three-element tuple containing the bandpass statistics,
#               scores, and plots dictionaries.
# 2013 Dec 17 - Dirk Muders, MPIfR
#               Made plotting optional.

# ------------------------------------------------------------------------------

def bpcal( in_table, out_dir, logobj='PYTHON', create_plots=False ):

	# Initialize the logger

	root = 'bpcal.bpcal'
	level = logs.INFO

	log_local, logger = logs.init( out_dir, root, level, logobj )


	# Print the first log message

	msg = 'Bandpass statistics started for ' + in_table + ' ...'
	origin = root
	logger.info( msg, origin=origin )


	# Check to see if the output file already exists

	out_table_root = os.path.basename( os.path.splitext( in_table )[0] )
	out_table = out_dir + '/' + out_table_root + '.bpcal.stats'

	if os.path.exists( out_table ):
		msg = 'Output table ' + out_table + ' already exists ...\n'
		origin = root
		logger.error( msg, origin=origin )
		raise IOError( msg )


	# Calculate the bandpass calibration statistics

	try:
		bpcal_stats = bpcal_calc( in_table, logger=logger )
	except Exception, err:
		origin = root
		logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )

	msg = 'Bandpass statistics of ' + in_table + ' calculated ...'
	origin = root
	logger.info( msg, origin=origin )


	# Write the bandpass calibration statistics to the table

	status = bpcal_write( bpcal_stats, out_table )

	msg = 'Bandpass statistics written in ' + out_table + ' ...'
	origin = root
	logger.info( msg, origin=origin )


	# Calculate the bandpass calibration scores

	bpcal_scores = bpcal_score( bpcal_stats )

	msg = 'Bandpass scores of ' + in_table + ' calculated ...'
	origin = root
	logger.info( msg, origin=origin )


	# Create the bandpass calibration statistics plots

        if (create_plots):
	    bpcal_plots = bpcal_plot( in_table, out_dir, bpcal_stats )

	    msg = 'Bandpass calibration statistics plots created in '
	    msg += out_dir + ' ...'
	    origin = root
	    logger.info( msg, origin=origin )
        else:
            bpcal_plots = {}


	# Print the last log message

	msg = 'Bandpass calibration statistics finished for '
	msg += in_table + ' ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Delete the logger, if it was created locally

	if log_local: del logger


	# Return the dictionary containing the bandpass statistics, scores, and
	# plots dictionaries
        bpcal_qa = {'QANUMBERS': bpcal_stats, \
                     'QASCORES': bpcal_scores, \
                     'QAPLOTS': bpcal_plots}

	return bpcal_qa

# ------------------------------------------------------------------------------
# Component user functions and ICDs
# ------------------------------------------------------------------------------

# bpcal_calc

# Description:
# ------------
# This function calculates the bandpass calibration statistics versus frequency
# for each time stamp and spectral window.

# TB: If the calibration analysis tool eventually has a mode where the spectral
# window can be an iteration axis, the spw and channel range elements of the
# output dictionary can be removed.

# NB: This function calculates amplitude and phase linear unweighted
# least-squares fits along the frequency axis for all spectral windows for each
# unique (field,antenna1,antenna2) for selected times.
# NB: About 10% of the edge channels are trimmed first.
# NB: The amplitudes are normalized to the maximum channel separately for each
# time stamp.
# NB: The phases are unwrapped across the channels separately for each time
# stamp.

# Inputs:
# -------
# in_table - This python string contains the name of the input bandpass
#            calibration table.
# logger   - This python variable contains the logging object information
#            (<python> and <casa> are actual instances of the logger classes).
#            The default is '', which means that the log information is either
#            sent to stdout or raised.

# Outputs:
# --------
# The dictionary containing the bandpass calibration statistics, returned via
# the function value.  If the function does not finish successfully, an
# exception is raised.

# The keys:
# 'AMPLITUDE_FIT' - The python dictionary containing the amplitude fit
#                   statistics.
# 'PHASE_FIT'     - The python dictionary containing the phase fit statistics.

# The 'AMPLITUDE_FIT' and 'PHASE_FIT' python dictionaries mentioned above have
# the same format and point to the following key:
# '<spw index #>' - The python dictionary containing the spectral window index.

# The '<spw index#>' python dictionaries mentioned above have the same format
# and point to the following keys:
# '<iter#>'     - The python string containing the iteration number.  Each
#                 iteration contains a fit for a specific field, antenna 1,
#                 antenna 2, feed, and time.
# '<spw#>'      - The spectral window number for all corresponding iterations.
# '<chanRange>' - The channel range [start,stop] for all corresponding
#                 iterations.

# The <'iter#'> python dictionary mentioned above have the same format and point
# to the following keys:
# 'abscissa'  - The python string containing the type of abscissa data (here, it
#               is always 'frequency').
# 'antenna1'  - The python string containing the antenna 1 number.
# 'antenna2'  - The python string containing the antenna 2 number.
# 'covars'    - The numpy array of doubles containing the fit covariances (empty
#               for an average fit; covar(0,1) for a linear fit; and covar(0,1),
#               covar(0,2), covar(1,2) for a quadratic fit).
# 'field'     - The python string containing the field number.
# 'feed'      - The python string containing the feed ID ('R' or 'L' for
#               circular feeds, 'X' or 'Y' for linear feeds).
# 'flag'      - The numpy array of booleans containing the value flags (True =
#               flagged, False = unflagged).
# 'frequency' - The numpy array of doubles containing the frequency abscissae
#               used in the fit.
# 'model'     - The numpy array of doubles containing the fit model calculated
#               from the abscissae and fit parameters. It includes flagged data.
# 'order'     - The python string containing the fit order (always 'LINEAR').
# 'pars'      - The numpy array of doubles containing the fit parameters (y
#               intercept and slope).
# 'redChi2'   - The python double containing the reduced chi2 (it is equal to
#               1.0 when the fit is unweighted).
# 'res'       - The numpy array of doubles containing the fit residuals
#               calculated from the values minus the model.  It includes flagged
#               data.
# 'resMean'   - The python double containing the mean of the residuals.
# 'resVar'    - The python double containing the variance of the residuals.
# 'spw'       - The numpy array of strings containing the spectral window
#               numbers.
# 'startChan' - The numpy array of integers containing the start channels for
#               each spectral window.
# 'stopChan'  - The numpy array of integers containing the stop channels for
#               each spectral window.
# 'time'      - The numpy array of doubles (one element only) containing the
#               time stamp used in the fit.
# 'type'      - The python string containing the fit type (always 'LSQ').
# 'validFit'  - The python boolean containing the fit success flag (True = OK,
#               False = not OK).
# 'value'     - The numpy array of doubles containing the values used in the
#               fit.
# 'valueErr'  - The numpy array of doubles containing the value errors used in
#               the fit (unused here since the fit is unweighted).
# 'vars'      - The numpy array of doubles containing the fit variances (the
#               y intercept and slope).
# 'weight'    - The python boolean containing the weight flag (always False =
#               unweighted).

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version (stub).
# 2012 Mar 09 - Nick Elias, NRAO
#               First working version.  Some of the code in the function will be
#               eliminated when calibration selection and defaults are
#               implemented.  The logger input parameter and logging code has
#               been added.
# 2012 Mar 15 - Nick Elias, NRAO
#               Added the spw_in input parameter.
# 2012 May 09 - Nick Elias, NRAO
#               spw_in input parameter removed.  The ca tool now has the
#               capability of parsing inputs, so that code has been removed
#               from this function.
# 2013        - Dirk Muders, MPIfR
#               Added AMPLITUDE_SN.
# 2013 Jul 23 - Dirk Muders, MPIfR
#               Added AMPLITUDE.

# ------------------------------------------------------------------------------

def bpcal_calc( in_table, logger='' ):

	# Create the local instance of the calibration analysis tool and open
	# the bandpass caltable

        try:
	    caLoc = casac.calanalysis()
        except:
            caLoc = casac.homefinder.find_home_by_name( 'calanalysisHome' ).create()

	caLoc.open( in_table )


	# Get the spectral window and channel lists

	spwList = caLoc.spw( name=False )

	numchanList = caLoc.numchannel()


	# Initialize the bandpass calibration statistics dictionary

	bpcal_stats = dict()


	# Get the amplitude fit statistics for each spectral window and time.
	# The spectral window and channel range elements are appended to the
	# result from ca.fit().  Effectively, the spectral window is another
	# iteration axis.

	bpcal_stats['AMPLITUDE_FIT'] = dict()

	try:
		for s in range( len(spwList) ):
			chanRange = bpcal_chanRangeList( numchanList[s] )
			if chanRange == []: continue
			spw = bpcal_spwChanString( spwList[s], chanRange )
			f = caLoc.fit( spw=spw, axis='TIME', ap='AMPLITUDE',
			    norm=True, order='LINEAR', type='LSQ',
			    weight=False )
			f['spw'] = int( spwList[s] )
			f['chanRange'] = chanRange
			bpcal_stats['AMPLITUDE_FIT'][str(s)] = f

	except Exception, err:
		origin = 'bpcal.bpcal_calc'
		if logger != '': logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )


	# Get the phase fit statistics for each spectral window and time.  The
	# spectral window and channel range elements are appended to the result
	# from ca.fit().

	bpcal_stats['PHASE_FIT'] = dict()

	try:
                # TODO: Use spwList directly, using "range" seems wrong.
		for s in range( len(spwList) ):
			chanRange = bpcal_chanRangeList( numchanList[s] )
			if chanRange == []: continue
			spw = bpcal_spwChanString( spwList[s], chanRange )
			f = caLoc.fit( spw=spw, axis='TIME', ap='PHASE',
			    unwrap=True, jumpmax=0.1, order='LINEAR',
			    type='LSQ', weight=False )
			f['spw'] = int( spwList[s] )
			f['chanRange'] = chanRange
			bpcal_stats['PHASE_FIT'][str(s)] = f

	except Exception, err:
		origin = 'bpcal.bpcal_calc'
		if logger != '': logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )


        # Get the amplitudes and phases and calculate signal-to-noise ratios
        bpcal_stats['AMPLITUDE'] = dict()
	bpcal_stats['AMPLITUDE_SNR'] = dict()
        bpcal_stats['PHASE'] = dict()

	try:
            for s in range( len(spwList) ):
                chanRange = bpcal_chanRangeList( numchanList[s] )
                if chanRange == []: continue
                spw = bpcal_spwChanString( spwList[s], chanRange )
                bpcal_stats['AMPLITUDE'][str(s)] = dict()
                bpcal_stats['AMPLITUDE'][str(s)]['spw'] = int( spwList[s] )
                bpcal_stats['AMPLITUDE'][str(s)]['chanRange'] = chanRange
                bpcal_stats['PHASE'][str(s)] = dict()
                bpcal_stats['PHASE'][str(s)]['spw'] = int( spwList[s] )
                bpcal_stats['PHASE'][str(s)]['chanRange'] = chanRange
                bpcal_stats['AMPLITUDE_SNR'][str(s)] = dict()
                bpcal_stats['AMPLITUDE_SNR'][str(s)]['spw'] = int( spwList[s] )
                bpcal_stats['AMPLITUDE_SNR'][str(s)]['chanRange'] = chanRange
                # Amplitude
                bp_data = caLoc.get(spw = spw)
                for pol in bp_data.iterkeys():
                    bpcal_stats['AMPLITUDE'][str(s)][pol] = bp_data[pol]['value'][chanRange[0]:chanRange[1]]
                    amp_mean = numpy.average(bpcal_stats['AMPLITUDE'][str(s)][pol])
                    amp_rms = rms(bp_data[pol]['value'][chanRange[0]:chanRange[1]]-amp_mean)
                    bpcal_stats['AMPLITUDE_SNR'][str(s)][pol] = amp_mean / amp_rms
                # Phase
                bp_data = caLoc.get(spw = spw, ap='PHASE')
                for pol in bp_data.iterkeys():
                    bpcal_stats['PHASE'][str(s)][pol] = bp_data[pol]['value'][chanRange[0]:chanRange[1]]
	except Exception, err:
		origin = 'bpcal.bpcal_calc'
		if logger != '': logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )


	# Close the calibration analysis tool and delete the local instance

	caLoc.close()

	del caLoc


	# Return the dictionary containing the bandpass calibration statistics

	return bpcal_stats

# ------------------------------------------------------------------------------

# bpcal_write

# Description:
# ------------
# This function writes the bandpass calibration statistics to a CASA table.

# NB: Because of the constant changes between integer and string variables, it
# was difficult to implement a generic field loop when writing the data.  For
# now, I'm use a brute-force kludge when I write each column manually.  Also, I
# may use the same routine(s) for other types of caltables.

# Inputs:
# -------
# bpcal_stats - This python dictionary contains the bandpass calibation
#               statistics.
# out_table   - This python string contains the output table name.

# Outputs:
# --------
# The bandpass calibration statistics table is created in the output directory.
# If the function finishes successfully, True is returned via the function
# value.  If the function does not finish successfully, it throws an exception.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version (stub).
# 2012 Mar 09 - Nick Elias, NRAO
#               First working version.
# 2012 Mar 14 - Nick Elias, NRAO
#               Used brute-force kludge to write columns, may change later.
# 2012 Mar 16 - Nick Elias, NRAO
#               Modified to handle different spectral window configurations.
# 2012 May 09 - Nick Elias, NRAO
#               Reverted to writing one spectral window at a time.

# ------------------------------------------------------------------------------

def bpcal_write( bpcal_stats, out_table ):

	# Create the local instance of the table tool, create the output main
	# table with a place holder data description, and close the table

        try:
	    tbLoc = casac.table()
        except:
            tbLoc = casa.__tablehome__.create()

	tbLoc.create( out_table, bpcal_desc() )

	tbLoc.addrows()
	tbLoc.putcol( 'place_holder', True, 0, 1, 1 )

	tbLoc.close()


	# Get the list of subtables and create them (AMPLITUDE and PHASE)

	subtables = bpcal_stats.keys(); subtables.sort()

	for st in subtables:
		out_subtable = out_table + '/' + st.upper()
		tbLoc.create( out_subtable, bpcal_desc_st() )
		tbLoc.close()


	# Put the data into the subtables

	for st in subtables:

            # Cannot write AMPLITUDE_SNR data yet.
            if (st.find('FIT') != -1):
		out_subtable = out_table + '/' + st.upper()
		tbLoc.open( out_subtable, nomodify=False )

		dictST = bpcal_stats[st]

		spwGroup = dictST.keys(); spwGroup.sort()

		for s in spwGroup:
			dictSPW = bpcal_stats[st][s]
			spw = dictSPW['spw']
			chanRange = dictSPW['chanRange']
			row = tbLoc.nrows()
			for r in range( len(dictSPW)-2 ): # -2: spw & chanRange
				dictRow = dictSPW[str(r)]
				tbLoc.addrows( 1 )
				bpcal_putFitRow( tbLoc, spw, chanRange, dictRow,
				    row+r )

		tbLoc.close()


	# Link the output subtables to the output main table using keywords,
	# close the main table, and delete the local table tool

	tbLoc.open( out_table, nomodify=False )

	for st in subtables:
		keyword = st.upper()
		value = 'Table: ' + out_table + '/' + keyword
		tbLoc.putkeyword( keyword, value )

	tbLoc.close()

	del tbLoc


	# Return True

	return True

# ------------------------------------------------------------------------------
# Unit functions and ICDs
# ------------------------------------------------------------------------------

# bpcal_putFitRow

# Description:
# ------------
# This function puts a row of data into the output table.

# NB: An instance of the table tool must already be created, the output table
# must be open, the output table must have a sufficient number of rows, and the
# output table must be closed elsewhere.

# Inputs:
# -------
# tbLoc     - This python instance contains the table tool which is opened to
#             the output table.
# spw       - This python integer contains the spectral window number.
# chanRange - This two-element python list of integers contains the start and
#             stop channels.
# dictRow   - This python dictionary contains the fit information for a row.
# row       - This python int contains the row number in the output table.

# Outputs:
# --------
# The python boolean true, returned via the function value.

# Modification history:
# ---------------------
# 2012 Mar 16 - Nick Elias, NRAO
#               Initial version.
# 2012 May 11 - Nick Elias, NRAO
#               Input parameters spw and chanRange added.

# ------------------------------------------------------------------------------

def bpcal_putFitRow( tbLoc, spw, chanRange, dictRow, row ):

	# Put the columns in the desired row using the putcell method of the
	# table tool

	tbLoc.putcell( 'field', row, int(dictRow['field']) )
	tbLoc.putcell( 'antenna1', row, int(dictRow['antenna1']) )
	tbLoc.putcell( 'antenna2', row, int(dictRow['antenna2']) )

	tbLoc.putcell( 'spw', row, spw )
	tbLoc.putcell( 'chanRange', row, chanRange )

	tbLoc.putcell( 'feed', row, dictRow['feed'] )

	tbLoc.putcell( 'abscissa', row, dictRow['abscissa'] )
	tbLoc.putcell( 'time', row, dictRow['time'] )
	tbLoc.putcell( 'frequency', row, dictRow['frequency'] )

	tbLoc.putcell( 'value', row, dictRow['value'] )
	tbLoc.putcell( 'valueErr', row, dictRow['valueErr'] )
	tbLoc.putcell( 'flag', row, dictRow['flag'] )

	tbLoc.putcell( 'order', row, dictRow['order'] )
	tbLoc.putcell( 'type', row, dictRow['type'] )
	tbLoc.putcell( 'weight', row, dictRow['weight'] )

	tbLoc.putcell( 'validFit', row, dictRow['validFit'] )

	tbLoc.putcell( 'pars', row, dictRow['pars'] )
	tbLoc.putcell( 'vars', row, dictRow['vars'] )
	tbLoc.putcell( 'covars', row, dictRow['covars'] )
	tbLoc.putcell( 'redChi2', row, dictRow['redChi2'] )
	tbLoc.putcell( 'res', row, dictRow['res'] )
	tbLoc.putcell( 'resVar', row, dictRow['resVar'] )
	tbLoc.putcell( 'resMean', row, dictRow['resMean'] )
	tbLoc.putcell( 'model', row, dictRow['model'] )

	return True

# ------------------------------------------------------------------------------

# bpcal_desc

# Description:
# ------------
# This function returns the place holder data description dictionary.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The python dictionary containing the place holder data description dictionary,
# returned via the function value.

# Modification history:
# ---------------------
# 2012 Mar 09 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def bpcal_desc():

	# Create the data description dictionary

	desc = dict()

	desc['place_holder'] = {
		'comment': 'Place holder',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'boolean'
	}


	# Return the data description dictionary

	return desc

# ------------------------------------------------------------------------------

# bpcal_desc_st

# Description:
# ------------
# This function creates the bandpass calibration subtable data description
# dictionary.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The python dictionary containing the bandpass calibration subtable data
# description dictionary, returned via the function value.

# Modification history:
# ---------------------
# 2012 Mar 09 - Nick Elias, NRAO
#               Initial version.
# 2012 May 09 - Nick Elias, NRAO
#               Made the spectral window cells scalar and removed the start and
#               stop channel columns.

# ------------------------------------------------------------------------------

def bpcal_desc_st():

	# Create the bandpass calibration subtable data description dictionary

	desc = dict()

	desc['field'] = {
		'comment': 'Field number',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'integer'
	}

	desc['antenna1'] = {
		'comment': 'Antenna 1 number',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'integer'
	}

	desc['antenna2'] = {
		'comment': 'Antenna 2 number',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'integer'
	}

	desc['spw'] = {
		'comment': 'Spectral window',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'integer'
	}

	desc['chanRange'] = {
		'comment': 'Start and stop channels',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 1,
		'option': 0,
		'valueType': 'integer'
	}

	desc['feed'] = {
		'comment': 'Feed',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'string'
	}

	desc['abscissa'] = {
		'comment': 'Abscissa',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'string'
	}

	desc['time'] = {
		'comment': 'Times',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 1,
		'option': 0,
		'valueType': 'double'
	}

	desc['frequency'] = {
		'comment': 'Frequencies',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 1,
		'option': 0,
		'valueType': 'double'
	}

	desc['value'] = {
		'comment': 'Values',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 1,
		'option': 0,
		'valueType': 'double'
	}

	desc['valueErr'] = {
		'comment': 'Value errors',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 1,
		'option': 0,
		'valueType': 'double'
	}

	desc['flag'] = {
		'comment': 'Flags',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 1,
		'option': 0,
		'valueType': 'boolean'
	}

	desc['order'] = {
		'comment': 'Fit order',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'string'
	}

	desc['type'] = {
		'comment': 'Fit type',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'string'
	}

	desc['weight'] = {
		'comment': 'Fit Weight',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'string'
	}

	desc['validFit'] = {
		'comment': 'Fit valid boolean',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'boolean'
	}

	desc['pars'] = {
		'comment': 'Fit parameters',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 1,
		'option': 0,
		'valueType': 'double'
	}

	desc['vars'] = {
		'comment': 'Fit variances',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 1,
		'option': 0,
		'valueType': 'double'
	}

	desc['covars'] = {
		'comment': 'Fit covariances',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 1,
		'option': 0,
		'valueType': 'double'
	}

	desc['redChi2'] = {
		'comment': 'Fit chi-squared',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'double'
	}

	desc['res'] = {
		'comment': 'Fit residuals',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 1,
		'option': 0,
		'valueType': 'double'
	}

	desc['resVar'] = {
		'comment': 'Variance of residuals',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'double'
	}

	desc['resMean'] = {
		'comment': 'Mean of residuals',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'double'
	}

	desc['model'] = {
		'comment': 'Fit model',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 1,
		'option': 0,
		'valueType': 'double'
	}


	# Return the data description dictionary

	return desc

# ------------------------------------------------------------------------------

# bpcal_chanRangeList

# Description:
# ------------
# This function returns the trimmed channel range.

# Algorithm:
# ----------
# 1. Start channel:
#    a. Multiply the number of channels by the trim parameter.
#    b. Subtract 0.5.
#    c. Round and int the result.
# 2. Stop channel:
#    a. Multiply the number of channels by 1.0 minus the trim parameter.
#    b. Subtract 0.5.
#    c. Round and int the result.
# 3. These values maximize the number of kept channels.

# Inputs:
# -------
# numchan - This python integer contains the number of channels.
# trim    - This python float contains the fraction of trimmed channels from
#           each side of the spectral window.  The default is 0.1.

# Outputs:
# --------
# The python list of integers (two elements) containing the start and stop
# channels, returned via the function value.

# Modification history:
# ---------------------
# 2012 May 15 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def bpcal_chanRangeList( numchan, trim=0.1 ):

	# If the spectral window has no channels, return []

	if numchan == 0: return []


	# Calculate the start and stop channels and return them

	startChan = int(round( trim*numchan - 0.5 ))
	stopChan = int(round( (1.0-trim)*numchan - 0.5 ))

	return [startChan,stopChan]

# ------------------------------------------------------------------------------

# bpcal_spwChanString

# Description:
# ------------
# This function converts spectral window and channel range lists to a CASA
# selection string.

# Inputs:
# -------
# spw    - This python integer contains the spectral window number.
# sschan - This python list of integers (two elements) contains the start and
#          stop channels.

# Outputs:
# --------
# The string containing the CASA spectral window selection string, returned via
# the function value.

# Modification history:
# ---------------------
# 2012 May 15 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def bpcal_spwChanString( spw, sschan ):

	# Form the spectral window string and return it

	spwS = str(spw) + ':' + str(sschan[0]) + '~' + str(sschan[1])

	return spwS

# ------------------------------------------------------------------------------

# bpcal_score

# Description:
# ------------
# This function calculates the score to prioritize the plots on the HTML page.

# NB: I make the distinction between amplitude and phase flags because the
# underlying C++ code of the calibration analysis tool performs various checks.

# NB: The maximum cutoff in the bpcal_score_RMS() call (0.1 for normalized
# amplitude, 0.2 radians for phase) can be changed here, if desired.

# Inputs:
# -------
# bpcal_stats - This python dictionary contains the bandpass calibation
#               statistics.

# Outputs:
# --------
# The dictionary containing the bandpass scores dictionary, returned via the
# function value.

# The keys:
# 'AMPLITUDE_SCORE_FLAG'  - The python dictionary containing the amplitude flag
#                           scores.
# 'AMPLITUDE_SCORE_RMS'   - The python dictionary containing the amplitude RMS
#                           scores.
# 'AMPLITUDE_SCORE_SNR'   - The python dictionary containing the amplitude signal
#                           to noise scores.
# 'AMPLITUDE_SCORE_FN'    - The python dictionary containing the amplitude flatness
#                           scores.
# 'AMPLITUDE_SCORE_DD'    - The python dictionary containing the amplitude derivative
#                           deviation scores.
# 'AMPLITUDE_SCORE_TOTAL' - The python dictionary containing the amplitude total
#                           (flag plus RMS) scores.
# 'PHASE_SCORE_FLAG'      - The python dictionary containing the phase flag
#                           scores.
# 'PHASE_SCORE_RMS'       - The python dictionary containing the phase RMS
#                           scores.
# 'PHASE_SCORE_FN'        - The python dictionary containing the phase flatness
#                           scores.
# 'PHASE_SCORE_DD'        - The python dictionary containing the phase derivative
#                           deviation scores.
# 'PHASE_SCORE_DELAY'     - The python dictionary containing the phase delay
#                           scores.
# 'PHASE_SCORE_TOTAL'     - The python dictionary containing the phase total
#                           (flag plus RMS plus delay) scores.

# All of the python dictionaries mentioned above have the same format and point
# to the same key:
# '<spw index #>' - The python dictionary containing the spectral window index.

# The '<spw index#>' python dictionaries mentioned above have the same format
# and point to the following element:
# '<iter#>'     - The python string containing the iteration number.  It points
#                 to a python float containing the score for a field, antenna
#                 1, antenna 2, feed, and time.

# Modification history:
# ---------------------
# 2012 Jun 23 - Nick Elias, NRAO
#               Initial stub version created.
# 2012 Jul 13 - Nick Elias, NRAO
#               Calls to score functions added and results saved to score
#               dictionary.
# 2013        - Dirk Muders, MPIfR
#               Added amplitude signal to noise scoring
# 2013 Jul 22 - Dirk Muders, MPIfR
#               Added amplitude shape scoring
# 2013 Aug 06 - Dirk Muders, MPIfR
#               Renamed SHAPE to FLATNESS (FN)
#               Added derivative deviation scoring

# ------------------------------------------------------------------------------

def bpcal_score( bpcal_stats ):

	# Initialize the score dictionary

	bpcal_scores = dict()

	bpcal_scores['AMPLITUDE_SCORE_FLAG'] = dict()
	bpcal_scores['AMPLITUDE_SCORE_RMS'] = dict()
	bpcal_scores['AMPLITUDE_SCORE_SNR'] = dict()
	bpcal_scores['AMPLITUDE_SCORE_FN'] = dict()
	bpcal_scores['AMPLITUDE_SCORE_DD'] = dict()
	bpcal_scores['AMPLITUDE_SCORE_TOTAL'] = dict()

	bpcal_scores['PHASE_SCORE_FLAG'] = dict()
	bpcal_scores['PHASE_SCORE_RMS'] = dict()
	bpcal_scores['PHASE_SCORE_FN'] = dict()
	bpcal_scores['PHASE_SCORE_DD'] = dict()
	bpcal_scores['PHASE_SCORE_DELAY'] = dict()
	bpcal_scores['PHASE_SCORE_TOTAL'] = dict()


	# Calculate all of the amplitude (flag, RMS, total) scores for each
	# spectral window and save them to the score dictionary.  The keys are
	# antenna1, antenna2, and feed (hopefully, field and time have only one
	# value).

	amp = bpcal_stats['AMPLITUDE_FIT']
	spw = amp.keys()

	for s in spw:

		keys = amp[s].keys()

		spwNum = amp[s]['spw']; keys.remove( 'spw' )
		chanRange = amp[s]['chanRange']; keys.remove( 'chanRange' )

		if len(keys) == 0: continue

		bpcal_scores['AMPLITUDE_SCORE_FLAG'][s] = dict()
		bpcal_scores['AMPLITUDE_SCORE_RMS'][s] = dict()
		bpcal_scores['AMPLITUDE_SCORE_SNR'][s] = dict()
		bpcal_scores['AMPLITUDE_SCORE_FN'][s] = dict()
		bpcal_scores['AMPLITUDE_SCORE_DD'][s] = dict()
		bpcal_scores['AMPLITUDE_SCORE_TOTAL'][s] = dict()

		for k in keys:

			if len( amp[s][k]['pars'] ) == 0: continue

			bpcal_scores['AMPLITUDE_SCORE_FLAG'][s][k] = \
			    bpcal_score_flag( amp[s][k]['flag'], chanRange )

			bpcal_scores['AMPLITUDE_SCORE_RMS'][s][k] = \
			    bpcal_score_RMS( math.sqrt(amp[s][k]['resVar']),
			    0.1 )

                        bpcal_scores['AMPLITUDE_SCORE_SNR'][s][k] = \
                            bpcal_score_SNR( bpcal_stats['AMPLITUDE_SNR'][s][k] )

                        bpcal_scores['AMPLITUDE_SCORE_FN'][s][k] = \
                            bpcal_score_flatness( bpcal_stats['AMPLITUDE'][s][k] )

                        bpcal_scores['AMPLITUDE_SCORE_DD'][s][k] = \
                            bpcal_score_derivative_deviation( bpcal_stats['AMPLITUDE'][s][k] )

			bpcal_scores['AMPLITUDE_SCORE_TOTAL'][s][k] = \
			    bpcal_scores['AMPLITUDE_SCORE_FLAG'][s][k] \
			    * bpcal_scores['AMPLITUDE_SCORE_RMS'][s][k]


	# Calculate all of the phase scores (flag, RMS, delay, total) for each
	# spectral window and save them to the score dictionary.  The keys are
	# antenna1, antenna2, and feed (hopefully, field and time have only one
	# value).

	phase = bpcal_stats['PHASE_FIT']
	spw = phase.keys()

	for s in spw:

		keys = phase[s].keys()

		spwNum = phase[s]['spw']; keys.remove( 'spw' )
		chanRange = phase[s]['chanRange']; keys.remove( 'chanRange' )

		if len(keys) == 0: continue

		bpcal_scores['PHASE_SCORE_FLAG'][s] = dict()
		bpcal_scores['PHASE_SCORE_RMS'][s] = dict()
		bpcal_scores['PHASE_SCORE_FN'][s] = dict()
		bpcal_scores['PHASE_SCORE_DD'][s] = dict()
		bpcal_scores['PHASE_SCORE_DELAY'][s] = dict()
		bpcal_scores['PHASE_SCORE_TOTAL'][s] = dict()

		for k in keys:

			if len( phase[s][k]['pars'] ) == 0: continue

			bpcal_scores['PHASE_SCORE_FLAG'][s][k] = \
			    bpcal_score_flag( phase[s][k]['flag'], chanRange )

			bpcal_scores['PHASE_SCORE_RMS'][s][k] = \
			    bpcal_score_RMS( math.sqrt(phase[s][k]['resVar']),
			    0.05 )

                        bpcal_scores['PHASE_SCORE_FN'][s][k] = \
                            bpcal_score_flatness( bpcal_stats['PHASE'][s][k]+45./180.*math.pi )

                        bpcal_scores['PHASE_SCORE_DD'][s][k] = \
                            bpcal_score_derivative_deviation( bpcal_stats['PHASE'][s][k] )

			bpcal_scores['PHASE_SCORE_DELAY'][s][k] = \
			    bpcal_score_delay(
			    phase[s][k]['pars'][1]/(2.0*math.pi),
			    phase[s][k]['vars'][1]/(2.0*math.pi),
			    phase[s][k]['frequency'], phase[s][k]['value'],
			    phase[s][k]['flag'], chanRange )

			bpcal_scores['PHASE_SCORE_TOTAL'][s][k] = \
			    bpcal_scores['PHASE_SCORE_FLAG'][s][k] \
			    * bpcal_scores['PHASE_SCORE_RMS'][s][k] \
			    * bpcal_scores['PHASE_SCORE_DELAY'][s][k]

        # Calculate grand total score
        bpcal_scores['TOTAL'] = \
            numpy.median([numpy.median(bpcal_scores['AMPLITUDE_SCORE_SNR'][s].values()) for s in bpcal_scores['AMPLITUDE_SCORE_SNR'].iterkeys()])

	# Return the bandpass score dictionary

	return bpcal_scores

# ------------------------------------------------------------------------------

# bpcal_score_flag

# Description:
# ------------
# This function calculates the flag score for a given iteration.

# Algorithm:
# ----------
# * Calculate the percentage of flagged data.
# * Calculate 1.0 minus percentage of flagged data, which is the percentage of
#   good data and the score.
# * Data not included in the channel range, e.g. edge channels, are not included
#   in the calculation.
# * This algorithm is very simple, but it can be refined, if necessary.

# Inputs:
# -------
# flags     - This numpy array of booleans contains the flags for a given
#             iteration.
# chanRange - This python list of two integers contains the start and stop
#             channels (in order to exclude the edge channels).

# Outputs:
# --------
# The python float containing the flag score, returned via the function value.

# Modification history:
# ---------------------
# 2012 Aug 20 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def bpcal_score_flag( flags, chanRange ):

	# Calculate and return the flag score for this iteration

	flagsTemp = flags[chanRange[0]:chanRange[1]+1]

	nData = len( flagsTemp )
	nFlag = len( numpy.where( flagsTemp == True ) )

	score = 1.0 - (float(nFlag)/float(nData))

	return score

# ------------------------------------------------------------------------------

# bpcal_score_RMS

# Description:
# ------------
# This function calculates the RMS score for a given iteration.

# Algorithm:
# ----------
# * Determine if the RMS is larger than the cutoff maximum RMS.  If so, set the
#   RMS to the maximum value.
# * Divide the RMS by the cutoff maximum RMS.
# * Calculate 1.0 minus the previous fraction, which is the score.
# * This algorithm is very simple, but it can be refined, if necessary.

# Inputs:
# -------
# RMS    - This python float contains the RMS.
# RMSMax - This python float contains the cutoff maximum RMS.  If the RMS is
#          larger than this value, the RMS is set to this value.

# Outputs:
# --------
# The python float containing the RMS score, returned via the function value.

# Modification history:
# ---------------------
# 2012 Aug 20 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def bpcal_score_RMS( RMS, RMSMax ):

	# Calculate and return the RMS score for this iteration

	if RMS <= RMSMax:
		RMSTemp = RMS
	else:
		RMSTemp = RMSMax

        if (RMSTemp == 0.0):
            score = 1.0
        else:
            try:
	        score = scipy.special.erf( RMSMax / RMSTemp / math.sqrt(2.0) )
            except FloatingPointError as e:
                # work around scipy bug triggered with certain values, such as when
                # SNR=37.5922006575. The bug is supposed to be fixed in scipy 0.12.0,
                # so detect which version of scipy we're running under and try the
                # operation again if this version is known to be affected.
                #
                # We can safely ignore the exception as it is only thrown when the
                # error function return value is so close to -1 or 1 that the lack of
                # precision makes no practical difference.
                (_, minor_version, _) = string.split(scipy.version.short_version, '.')
                if int(minor_version) < 12:
                        under_orig = scipy.geterr()['under']
                        try:
                                scipy.seterr(under='warn')
                                score = scipy.special.erf( RMSMax / RMSTemp / math.sqrt(2.0) )
                        except FloatingPointError as e:
                                msg = 'Error calling scipy.special.erf(%s/math.sqrt(2.0))' % (RMSMax / RMSTemp)
                                raise FloatingPointError(msg)
                        finally:
                                scipy.seterr(under=under_orig)
                else:
                        msg = 'Error calling scipy.special.erf(%s/math.sqrt(2.0))' % (RMSMax / RMSTemp)
                        raise FloatingPointError(msg)

	return score

# ------------------------------------------------------------------------------

# bpcal_score_SNR

# Description:
# ------------
# This function calculates the signal-to-noise score.

# Algorithm:
# ----------
# Use error function to evaluate validity of a given sigma.

# Modification history:
# ---------------------
# 2013 Jan 22 - Dirk Muders, MPIfR
#               Initial version.

# ------------------------------------------------------------------------------

def bpcal_score_SNR( SNR ):
	try:
		score = scipy.special.erf( SNR / math.sqrt(2.0) )
	except FloatingPointError as e:
		# work around scipy bug triggered with certain values, such as when
		# SNR=37.5922006575. The bug is supposed to be fixed in scipy 0.12.0,
		# so detect which version of scipy we're running under and try the
		# operation again if this version is known to be affected.
		#
		# We can safely ignore the exception as it is only thrown when the
		# error function return value is so close to -1 or 1 that the lack of
		# precision makes no practical difference.
		(_, minor_version, _) = string.split(scipy.version.short_version, '.')
		if int(minor_version) < 12:
			under_orig = scipy.geterr()['under']
			try:
				scipy.seterr(under='warn')
				score = scipy.special.erf( SNR / math.sqrt(2.0) )
			except FloatingPointError as e:
				msg = 'Error calling scipy.special.erf(%s/math.sqrt(2.0))' % SNR
				raise FloatingPointError(msg)
			finally:
				scipy.seterr(under=under_orig)				
		else:
			msg = 'Error calling scipy.special.erf(%s/math.sqrt(2.0))' % SNR
			raise FloatingPointError(msg)

	return score

# ------------------------------------------------------------------------------

# bpcal_score_flatness

# Description:
# ------------
# This function calculates the flatness score.

# Algorithm:
# ----------
# * Calculate the Wiener Entropy of amplitude values vs. frequency
# * A result of 1.0 means the shape is perfectly flat
# * Determine score by evaluating the deviation of the Wiener
#   Entropy from 1.0

# Inputs:
# -------
# values    - Values

# Outputs:
# --------
# The python float containing the flatness score.

# Modification history:
# ---------------------
# 2013 Jul 22 - Dirk Muders, MPIfR
#               Initial version.
# 2013 Aug 05 - Dirk Muders, MPIfR
#               Weight Wiener entropy with error function
#               to create sharper fall-off.

def bpcal_score_flatness( values ):

    # Need to avoid zero mean
    if (numpy.mean(values) == 0.0):
        if ((values == 0.0).all()):
            wEntropy = 1.0
        else:
            wEntropy = 1.0e10
    else:
        # Geometrical mean can not be calculated for vectors <= 0.0 for all
        # elements.
        if ((values <= 0.0).all()):
            wEntropy = 1.0e10
        else:
            wEntropy = scipy.stats.mstats.gmean(values)/numpy.mean(values)

    if (wEntropy == 1.0):
        flatnessScore = 1.0
    else:
        try:
            flatnessScore = scipy.special.erf(0.001 / abs(1.0 - wEntropy) / math.sqrt(2.0))
        except FloatingPointError as e:
            # work around scipy bug triggered with certain values, such as when
            # SNR=37.5922006575. The bug is supposed to be fixed in scipy 0.12.0,
            # so detect which version of scipy we're running under and try the
            # operation again if this version is known to be affected.
            #
            # We can safely ignore the exception as it is only thrown when the
            # error function return value is so close to -1 or 1 that the lack of
            # precision makes no practical difference.
            (_, minor_version, _) = string.split(scipy.version.short_version, '.')
            if int(minor_version) < 12:
                under_orig = scipy.geterr()['under']
                try:
                    scipy.seterr(under='warn')
                    flatnessScore = scipy.special.erf(0.001 / abs(1.0 - wEntropy) / math.sqrt(2.0))
                except FloatingPointError as e:
                    msg = 'Error calling scipy.special.erf(%s/math.sqrt(2.0))' % (0.001 / abs(1.0 - wEntropy))
                    raise FloatingPointError(msg)
                finally:
                    scipy.seterr(under=under_orig)
            else:
                msg = 'Error calling scipy.special.erf(%s/math.sqrt(2.0))' % (0.001 / abs(1.0 - wEntropy))
                raise FloatingPointError(msg)

    return flatnessScore


# ------------------------------------------------------------------------------
# TODO: Use usual MAD function from pipeline.

import numpy.ma as ma

def MAD(a, c=0.6745, axis=None):
    """
    Median Absolute Deviation along given axis of an array:

    median(abs(a - median(a))) / c

    c = 0.6745 is the constant to convert from MAD to std; it is used by
    default

    """

    # Avoid underflow exceptions
    under_orig = scipy.geterr()['under']
    scipy.seterr(under='warn')

    a = ma.masked_where(a!=a, a)
    if a.ndim == 1:
        d = ma.median(a)
        m = ma.median(ma.fabs(a - d) / c)
    else:
        d = ma.median(a, axis=axis)
        # I don't want the array to change so I have to copy it?
        if axis > 0:
            aswp = ma.swapaxes(a,0,axis)
        else:
            aswp = a
        m = ma.median(ma.fabs(aswp - d) / c, axis=0)

    scipy.seterr(under=under_orig)

    return m

def nanmedian(arr, **kwargs):
    """
    Returns median ignoring NAN
    """
    return ma.median( ma.masked_where(arr!=arr, arr), **kwargs )

# ------------------------------------------------------------------------------

# bpcal_score_derivative_deviation

# Description:
# ------------
# This function calculates the derivative deviation score.

# Algorithm:
# ----------
# * Calculate the 1-channel derivative of the values and determine
#   the fraction of channels whose absolute value deviates by more
#   than 5*MAD. Score this fraction against the tolerated fraction
#   using the error function.

# Inputs:
# -------
# values    - Values

# Outputs:
# --------
# The python float containing the derivative deviation score.

# Modification history:
# ---------------------
# 2013 Aug 06 - Dirk Muders, MPIfR
#               Initial version.

def bpcal_score_derivative_deviation( values ):

    # Avoid scoring numerical inaccuracies for the reference antenna phase
    if (numpy.sum(numpy.abs(values)) < 1e-4):
        ddScore = 1.0
    else:
        derivative = values[:-1]-values[1:]
        derivativeMAD = MAD(derivative)
        numOutliers = len(numpy.where(derivative > 5.0 * derivativeMAD)[0])

        if (numOutliers == 0):
            ddScore = 1.0
        else:
            outliersFraction = float(numOutliers) / float(len(values))
            toleratedFraction = 0.01
            fractionRatio = 3.0 * toleratedFraction / outliersFraction
            try:
                ddScore = scipy.special.erf(fractionRatio / math.sqrt(2.0))
            except FloatingPointError as e:
                (_, minor_version, _) = string.split(scipy.version.short_version, '.')
                if int(minor_version) < 12:
                    under_orig = scipy.geterr()['under']
                    try:
                        scipy.seterr(under='warn')
                        ddScore = scipy.special.erf(fractionRatio / math.sqrt(2.0))
                    except FloatingPointError as e:
                        msg = 'Error calling scipy.special.erf(%s/math.sqrt(2.0))' % (fractionRatio)
                        raise FloatingPointError(msg)
                    finally:
                        scipy.seterr(under=under_orig)
                else:
                    msg = 'Error calling scipy.special.erf(%s/math.sqrt(2.0))' % (fractionRatio)
                    raise FloatingPointError(msg)

    return ddScore


# ------------------------------------------------------------------------------

# bpcal_score_delay

# Description:
# ------------
# This function calculates the delay score for a given iteration.

# Algorithm:
# ----------
# * Determine is the delay is less than three times the delay error.  If so,
#   return a score of 1.0.
# * Determine which two near central channels to use.  If there are issues,
#   return a score of 0.0.
# * Calculate the frequency and phase difference between the two near central
#   channels.
# * Calculate the cutoff maximum delay.
# * Determine if the delay is larger than the cutoff maximum delay.  If so, set
#   the delay to the maximum value.
# * Divide the delay by the cutoff maximum delay.
# * Calculate 1.0 minus the previous fraction, which is the score.
# * This algorithm is very simple, but it can be refined, if necessary.

# Inputs:
# -------
# delay     - This python float contains the delay.
# delayErr  - This python float contains the delay error.
# freqs     - This numpy float array contains the frequencies.
# phases    - This numpy float array contains the phases.
# flags     - This numpy array of booleans contains the flags for a given
#             iteration.
# chanRange - This python list of two integers contains the start and stop
#             channels (in order to exclude the edge channels).

# Outputs:
# --------
# The python float containing the delay score, returned via the function value.

# Modification history:
# ---------------------
# 2012 Aug 20 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def bpcal_score_delay( delay, delayErr, freqs, phases, flags, chanRange ):

	# Calculate and return the delay score for this iteration

	if delay <= 3.0*delayErr: return 1.0

	chanMiddle = int( 0.5*(chanRange[0]+chanRange[1]) )

	if chanRange[1]-chanRange[0] < 2: return 0.0
	if chanMiddle <= chanRange[0] or chanMiddle >= chanRange[1]: return 0.0
	if flags[chanMiddle] or flags[chanMiddle+1]: return 0.0

	dFreq = abs( freqs[chanMiddle+1]-freqs[chanMiddle] )
	dPhase = abs( phases[chanMiddle+1]-phases[chanMiddle] )

	delayMax = dPhase / ( (2.0*math.pi) * dFreq )

	if delay <= delayMax:
		delayTemp = delay
	else:
		delayTemp = delayMax

	score = 1.0 - (delayTemp/delayMax)

	return score

# ------------------------------------------------------------------------------

# bpcal_plot

# Directory:
# ----------
# This function is the top-level interface to other functions that create
# bandpass calibration plots.

# NB: The input caltable is used only for its name, which is used as the prefix
# for plot names.

# NB: This function assumes that only a single field will be used for a bandpass
# calibration.  If multiple fields appear in the bandpass statistics dictionary,
# the plot file names will have to take it into account.

# Inputs:
# -------
# in_table    - This python string contains the name of the input bandpass
#               calibration table.
# out_dir     - This python string contains the output directory.
# bpcal_stats - This python dictionary contains the bandpass calibation
#               statistics.

# Outputs:
# --------
# The dictionary containing the bandpass plot names, returned via the function
# value.  If the function does not finish successfully, an exception is raised.

# The keys:
# 'AMPLITUDE_PLOT' - The python dictionary containing the amplitude plots.
# 'PHASE_PLOT'     - The python dictionary containing the phase plots.

# The 'AMPLITUDE_PLOT' and 'PHASE_PLOT' python dictionaries mentioned above have
# the same format and point to the following key:
# '<spw index #>' - The python dictionary containing the spectral window index.

# The '<spw index#>' python dictionaries mentioned above have the same format
# and point to the following dictionary:
# '<iter#>'     - The python string containing the iteration number.  It points
#                 to a python float containing the score for a field, antenna
#                 1, antenna2, feed, and time.

# Modification history:
# ---------------------
# 2012 Jun 20 - Nick Elias, NRAO
#               Initial version.
# 2012 Jun 20 - Nick Elias, NRAO
#               Added calls to bpcal_plot_hist() for histograms of number of
#               flagged data, RMSes, and absolute value of delays.

# ------------------------------------------------------------------------------

def bpcal_plot( in_table, out_dir, bpcal_stats ):

	# Initialize the plot names dictionary

	bpcal_plots = dict()


	# Create all amplitude plots for all keys for each spectral window.  The
	# keys are antenna1, antenna2, and feed (hopefully, field and time are
	# have only one value).

	amp = bpcal_stats['AMPLITUDE_FIT']
	spw = amp.keys()

	bpcal_plots['AMPLITUDE_PLOT'] = dict()

	for s in spw:

		keys = amp[s].keys()
		keys.remove( 'spw' )
		chanRange = amp[s]['chanRange']; keys.remove( 'chanRange' )

		bpcal_plots['AMPLITUDE_PLOT'][s] = dict()

		if len(keys) == 0: continue

		for k in keys:
			bpcal_plots['AMPLITUDE_PLOT'][s][k] = bpcal_plot1(
			    in_table, out_dir, amp[s][k], s, chanRange, k,
			    'AMPLITUDE' )


	# Create the amplitude histograms for the number of flagged data and the
	# absolute value of the delay

	bpcal_plot_hist( in_table, out_dir, amp, ap='AMPLITUDE', hist='FLAG' )
	bpcal_plot_hist( in_table, out_dir, amp, ap='AMPLITUDE', hist='RMS' )


	# Create all phase plots for all keys for each spectral window.  The
	# keys are antenna1, antenna2, and feed (hopefully, field and time are
	# have only one value).

	phase = bpcal_stats['PHASE_FIT']
	spw = phase.keys()

	bpcal_plots['PHASE_PLOT'] = dict()

	for s in spw:

		keys = phase[s].keys()
		keys.remove( 'spw' )
		chanRange = phase[s]['chanRange']; keys.remove( 'chanRange' )

		bpcal_plots['PHASE_PLOT'][s] = dict()

		if len(keys) == 0: continue

		for k in keys:
			bpcal_plots['PHASE_PLOT'][s][k] = bpcal_plot1( in_table,
			    out_dir, phase[s][k], s, chanRange, k, 'PHASE' )


	# Create the phase histograms for the number of flagged data, the
	# RMS, and the absolute value of the delay

	bpcal_plot_hist( in_table, out_dir, amp, ap='PHASE', hist='FLAG' )
	bpcal_plot_hist( in_table, out_dir, amp, ap='PHASE', hist='RMS' )
	bpcal_plot_hist( in_table, out_dir, amp, ap='PHASE', hist='DELAY' )


	# Return the plot names dictionary

	return bpcal_plots

# ------------------------------------------------------------------------------

# bpcal_plot1

# Description:
# ------------
# This function creates a single bandpass statistics plot and saves it to disk.

# NB: The plot file name is the prefix of the input table name plus
# '.bpcal.stats' plus 'amp' or 'phase' plus the spectral window number plus
# the antenna1 number plus the antenna2 number plus '.png'.

# Inputs:
# -------
# in_table   - This python string contains the name of the input bandpass
#              calibration table.
# out_dir    - This python string contains the output directory.
# stats_dict - The python dictionary that contains the bandpass calibration
#              statistics versus baseline (it is the 'iter#' element of the
#              'AMPLITUDE_FIT/PHASE_FIT' element of the dictionary returned by
#              the bpcal_calc() function).
# spw        - The python string containing the spectral window number.
# chanRange  - The python list of integers containing the start and stop channel
#              used in the bandpass fit.  NB: This list should have only two
#              elements.
# iteration  - The python string containing the iteration number.
# ap         - The python string containing either 'AMPLITUDE' or 'PHASE'.  Only
#              the first letter is required, and it does not need to be
#              capitalized.

# Outputs:
# --------
# The python string containing the bandpass plot name, returned via the function
# value.
# The bandpass statistics plot, saved to disk.

# Modification history:
# ---------------------
# 2012 Jun 20 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def bpcal_plot1( in_table, out_dir, stats_dict, spw, chanRange, iteration, ap ):

	# Determine and check amplitude 'A' or phase 'P'

	apTemp = ap[0].upper()

	if apTemp != 'A' and apTemp != 'P': return False


	# Get and check the frequencies

	frequency = stats_dict['frequency']

	if len(frequency) == 0: return False


	# Get the bandpass calibration values and their errors

	value = stats_dict['value']
	valueErr = stats_dict['valueErr']


	# Plot the bandpass calibration values and their errors with green
	# symbols.  Overwrite flagged data with red symbols.

	pl.errorbar( frequency, value, yerr=valueErr, fmt='go', label='good' )

	flag = stats_dict['flag']
	index = numpy.where( flag==True )[0]
	if len(index) > 0:
		pl.errorbar( frequency[index], value[index],
		    yerr=valueErr[index], fmt='ro', label='flagged' )


	# Add the title to the plot, which contains the spectral window, feed,
	# antenna1, antenna2, and time

	feed = stats_dict['feed']
	ant1 = str( stats_dict['antenna1'] )
	ant2 = str( stats_dict['antenna2'] )
	time = str( stats_dict['time'] )

	title = 'SPW = ' + spw + ':'
	title += str(chanRange[0]) + '~' + str(chanRange[1]) + ', '
	title += 'Feed = ' + feed + ', '
	title += 'Baseline = ' + ant1 + '&' + ant2 + ', '
	title += 'Time = ' + time
	pl.title( title )


	# Add the x and y labels

	pl.xlabel( 'Frequency (GHz)' )

	if apTemp == 'A':
		pl.ylabel( 'Normalized Bandpass Amplitude' )
	else:
		pl.ylabel( 'Unwrapped Bandpass Phase' )


	# If there is a valid model, add it as a line

	if stats_dict['validFit']:
		model = stats_dict['model']
		pl.plot( frequency, model, 'b-', label='model' )


	# Add the legen for all points plotted

	pl.legend()


	# Add annotations (RMS for amplitude and phase plots, delay for phase
	# plots)

	RMS = math.sqrt( stats_dict['resVar'] )
	RMSS = 'Fit RMS = %.3e' % RMS
	pl.annotate( RMSS, xycoords='figure fraction', xy=(0.15,0.125) )

	if apTemp == 'P' and stats_dict['validFit']:
		delay = stats_dict['pars'][1] / (2.0*math.pi)
		delayErr = math.sqrt(stats_dict['vars'][1]) / (2.0*math.pi)
		delayS = 'Delay = %.3e' % delay
		delayS += ' +/- %.3e ns' % delayErr
		pl.annotate( delayS, xycoords='figure fraction',
		    xy=(0.15,0.15) )


	# Create the bandpass plot name based on amp/phase, spectral window
	# number, feed, antenna1, and antenna2

	noExt = os.path.splitext( in_table )[0]
	out_plot_root = os.path.basename( noExt )

	if apTemp == 'A':
		ext = '.amp'
	else:
		ext = '.phase'

	out_plot = out_dir + '/' + out_plot_root
	out_plot += '.bpcal.stats' + ext
	out_plot += '.' + spw
	out_plot += '.' + feed
	out_plot += '.' + ant1
	out_plot += '.' + ant2
	out_plot += '.png'


	# Save the plot to disk and clear

	pl.savefig( out_plot )

	pl.clf()


	# Return the bandpass plot name

	return out_plot

# ------------------------------------------------------------------------------

# bpcal_plot_hist

# Description:
# ------------
# This function plots the histograms of either number of flagged data, RMSes,
# or absolute value of delays, color coded by spectral window.

# Inputs:
# -------

# Outputs:
# --------

# Modification history:
# ---------------------
# 2012 Jun 22 - Nick Elias, NRAO
#               Initial stub version created.

# ------------------------------------------------------------------------------

def bpcal_plot_hist( in_table, out_dir, stats, ap='AMPLITUDE', hist='FLAG' ):
	return None
