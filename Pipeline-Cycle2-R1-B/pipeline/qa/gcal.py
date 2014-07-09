
# ------------------------------------------------------------------------------
# Subsystem module ICD
# ------------------------------------------------------------------------------

# gcal.py

# Description:
# ------------
# This module runs the gain calibration statistics subsystem of the QA system.

# TBD:
# ----
# * Make cal_putFitRow() a more generic function.

# User Functions (subsystem and component level):
# -----------------------------------------------
# gcal       - This function runs the gain calibration statistics subsystem of
#              the QA system.
#
# gcal_calc  - This function calculates the gain calibration statistics.
# gcal_write - This function writes the gain calibration statistics to a CASA
#              table.

# Non-user functions (unit level):
# --------------------------------
# gcal_putFitRow - This function puts a row of data into the output table.
#
# gcal_desc      - This function returns the place holder data description
#                  dictionary.
# gcal_desc_st   - This function creates the gain calibration subtable data
#                  description dictionary.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version created with functions gcal(), gcal_calc(), and
#               gcal_write().
# 2012 Mar 09 - Nick Elias, NRAO
#               Functions gcal_desc() and gcal_desc_st() added.
# 2012 Mar 16 - Nick Elias, NRAO
#               Function gcalc_putFitRow() added.
# 2012 May 15 - Nick Elias, NRAO
#               Major revison using new version of calibration analysis tool.
# 2012 Aug 01 - Nick Elias, NRAO
#               Tool calls modified to new format.

# ------------------------------------------------------------------------------

# Imports
# -------

import os

import numpy

try:
    from casac import casac
except:
    import casa
    import casac

import utility.logs as logs

# ------------------------------------------------------------------------------
# Subsystem user function and ICD
# ------------------------------------------------------------------------------

# gcal

# Description:
# ------------
# This function runs the gain calibration statistics subsystem of the QA
# system.

# Inputs:
# -------
# in_table - This python string contains the name of the input gain calibration
#            table.
# out_dir  - This python string contains the output directory.
# logobj   - This python variable contains the logging object information
#            ('PYTHON' creates a local python Logger class, 'CASA' creates a
#            local python logger, <python> and <casa> are actual instances of
#            the logger classes; default = 'PYTHON').

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
# 2012 Mar 09 - Nick Elias, NRAO
#               The logobj input parameter and logging code has been added.
# 2012 Mar 15 - Nick Elias, NRAO
#               Added the spw_in input parameter.
# 2012 May 15 - Nick Elias, NRAO
#               Removed the spw_in input parameter.

# ------------------------------------------------------------------------------

def gcal( in_table, out_dir, logobj='PYTHON' ):

	# Initialize the logger

	root = 'gcal.gcal'
	level = logs.INFO

	log_local, logger = logs.init( out_dir, root, level, logobj )


	# Print the first log message

	msg = 'Gain statistics started for ' + in_table + ' ...'
	origin = root
	logger.info( msg, origin=origin )


	# Check to see if the output file already exists

	out_table_root = os.path.basename( os.path.splitext( in_table )[0] )
	out_table = out_dir + '/' + out_table_root + '.gcal.stats'

	if os.path.exists( out_table ):
		msg = 'Output table ' + out_table + ' already exists ...\n'
		origin = root
		logger.error( msg, origin=origin )
		raise IOError( msg )


	# Calculate the gain calibration statistics

	try:
		gcal_stats = gcal_calc( in_table, logger=logger )
	except Exception, err:
		origin = root
		logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )

	msg = 'Gain statistics of ' + in_table + ' calculated ...'
	origin = root
	logger.info( msg, origin=origin )


	# Write the gain calibration statistics to the table

	status = gcal_write( gcal_stats, out_table )

	msg = 'Gain statistics written in ' + out_table + ' ...'
	origin = root
	logger.info( msg, origin=origin )


	# Delete the logger, if it was created locally

	if log_local: del logger


	# Return True

	return True

# ------------------------------------------------------------------------------
# Component user functions and ICDs
# ------------------------------------------------------------------------------

# gcal_calc

# Description:
# ------------
# This function calculates the gain calibration statistics versus time for each
# spectral window.

# TB: If the calibration analysis tool eventually has a mode where the spectral
# window can be an iteration axis, the spw and channel range elements of the
# output dictionary can be removed.

# NB: This function calculates amplitude and phase linear unweighted
# least-squares fits along the time axis for all spectral windows (each has one
# frequency) for each unique (field,antenna1,antenna2).
# NB: The amplitudes are normalized to the maximum channel separately for each
# spectral window (frequency).
# NB: The phases are unwrapped across the times separately for each spectral
# window (frequency).

# Inputs:
# -------
# in_table - This python string contains the name of the input gain calibration
#            table.
# logger   - This python variable contains the logging object information
#            (<python> and <casa> are actual instances of the logger classes).
#            The default is '', which means that the log information is either
#            sent to stdout or raised.

# Outputs:
# --------
# The dictionary containing the gain calibration statistics, returned via the
# function value.  If the function does not finish successfully, an exception is
# raised.

# The keys:
# 'amplitude_fit' - The python dictionary containing the amplitude fit
#                   statistics.
# 'phase_fit'     - The python dictionary containing the phase fit statistics.

# The 'amplitude_fit' and 'phase_fit' python dictionaries mentioned above have
# the same format and point to the following key:
# '<spw index #>' - The python dictionary containing the spectral window index.

# The '<spw index#>' python dictionaries mentioned above have the same format
# and point to the following key:
# '<iter#>'    - The python string containing the iteration number.  Each
#                iteration contains a fit for a specific field, antenna 1,
#                antenna 2, feed, and time.
# '<spw#>'     - The spectral window number for all corresponding iterations.

# The <'iter#'> python dictionary mentioned above have the same format and point
# to the following keys:
# 'abscissa'  - The python string containing the type of abscissa data (here, it
#               is always 'time').
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
# 'frequency' - The numpy array of doubles (one element only) containing the
#               frequency used in the fit.
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
# 'time'      - The numpy array of doubles containing the time stamp abscissae
#               used in the fit.
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
# 2012 May 15 - Nick Elias, NRAO
#               spw_in input parameter removed.  The ca tool now has the
#               capability of parsing inputs, so that code has been removed
#               from this function.

# ------------------------------------------------------------------------------

def gcal_calc( in_table, logger='' ):

	# Create the local instance of the calibration analysis tool and open
	# the gain caltable

        try:
	    caLoc = casac.calanalysis()
        except:
            caLoc = casac.homefinder.find_home_by_name( 'calanalysisHome' ).create()

	caLoc.open( in_table )


	# Get the spectral window list

	spwList = caLoc.spw( name=False )


	# Initialize the gain calibration statistics dictionary

	gcal_stats = dict()


	# Get the amplitude fit statistics for each spectral window (frequency).
	# The spectral window element is appended to the result from ca.fit().
	# Effectively, the spectral window is another iteration axis.

	gcal_stats['amplitude_fit'] = dict()

	try:
		for s in range( len(spwList) ):
			f = caLoc.fit( spw=spwList[s], axis='FREQUENCY',
			    ap='AMPLITUDE', norm=True, order='LINEAR',
			    type='LSQ', weight=False )
			f['spw'] = int( spwList[s] )
			gcal_stats['amplitude_fit'][str(s)] = f

	except Exception, err:
		origin = 'gcal.gcal_calc'
		if logger != '': logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )


	# Get the phase fit statistics for each spectral window (frequency).
	# The spectral window element is appended to the result from ca.fit().
	# Effectively, the spectral window is another iteration axis.

	gcal_stats['phase_fit'] = dict()

	try:
		for s in range( len(spwList) ):
			f = caLoc.fit( spw=spwList[s], axis='FREQUENCY',
			    ap='PHASE', unwrap=True, jumpmax=0.2,
                            order='LINEAR', type='LSQ', weight=False )
			f['spw'] = int( spwList[s] )
			gcal_stats['phase_fit'][str(s)] = f

	except Exception, err:
		origin = 'gcal.gcal_calc'
		if logger != '': logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )


	# Close the calibration analysis tool and delete the local instance

	caLoc.close()

	del caLoc


	# Return the dictionary containing the gain calibration statistics

	return gcal_stats

# ------------------------------------------------------------------------------

# gcal_write

# Description:
# ------------
# This function writes the gain calibration statistics to a CASA table.

# NB: Because of the constant changes between integer and string variables, it
# was difficult to implement a generic field loop when writing the data.  For
# now, I'm use a brute-force kludge when I write each column manually.  Also, I
# may use the same routine(s) for other types of caltables.

# Inputs:
# -------
# gcal_stats - This python dictionary contains the gain calibation statistics.
# out_table  - This python string contains the output table name.

# Outputs:
# --------
# The gain calibration statistics table is created in the output directory.  If
# the function finishes successfully, True is returned via the function value.
# If the function does not finish successfully, it raises an exception.

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
# 2012 May 15 - Nick Elias, NRAO
#               Modified to write one spectral window at a time.

# ------------------------------------------------------------------------------

def gcal_write( gcal_stats, out_table ):

	# Create the local instance of the table tool, create the output main
	# table with a place holder data description, and close the table

        try:
	    tbLoc = casac.table()
        except:
            tbLoc = casa.__tablehome__.create()

	tbLoc.create( out_table, gcal_desc() )

	tbLoc.addrows()
	tbLoc.putcol( 'place_holder', True, 0, 1, 1 )

	tbLoc.close()


	# Get the list of subtables and create them (AMPLITUDE and PHASE)

	subtables = gcal_stats.keys(); subtables.sort()

	for st in subtables:
		out_subtable = out_table + '/' + st.upper()
		tbLoc.create( out_subtable, gcal_desc_st() )
		tbLoc.close()


	# Put the data into the subtables

	for st in subtables:
		
		out_subtable = out_table + '/' + st.upper()
		tbLoc.open( out_subtable, nomodify=False )

		dictST = gcal_stats[st]

		spwGroup = dictST.keys(); spwGroup.sort()

		for s in spwGroup:
			dictSPW = gcal_stats[st][s]
			spw = dictSPW['spw']
			row = tbLoc.nrows()
			for r in range( len(dictSPW)-1 ): # -1: spw
				dictRow = dictSPW[str(r)]
				tbLoc.addrows( 1 )
				gcal_putFitRow( tbLoc, spw, dictRow, row+r )

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

# gcal_putFitRow

# Description:
# ------------
# This function puts a row of data into the output table.

# NB: An instance of the table tool must already be created, the output table
# must be open, and the output table must have a sufficient number of rows.

# Inputs:
# -------
# tbLoc   - This python instance contains the table tool which is opened to the
#           output table.
# spw     - This python integer contains the spectral window number.
# dictRow - This python dictionary contains the fit information for a row.
# row     - This python int contains the row number in the output table.

# Outputs:
# --------
# The python boolean true, returned via the function value.

# Modification history:
# ---------------------
# 2012 Mar 16 - Nick Elias, NRAO
#               Initial version.
# 2012 May 11 - Nick Elias, NRAO
#               Input parameter spw added.

# ------------------------------------------------------------------------------

def gcal_putFitRow( tbLoc, spw, dictRow, row ):

	# Put the columns in the desired row using the putcell method of the
	# table tool

	tbLoc.putcell( 'field', row, int(dictRow['field']) )
	tbLoc.putcell( 'antenna1', row, int(dictRow['antenna1']) )
	tbLoc.putcell( 'antenna2', row, int(dictRow['antenna2']) )

	tbLoc.putcell( 'spw', row, spw )

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

# gcal_desc

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

def gcal_desc():

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

# gcal_desc_st

# Description:
# ------------
# This function creates the gain calibration subtable data description
# dictionary.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The python dictionary containing the gain calibration subtable data
# description dictionary, returned via the function value.

# Modification history:
# ---------------------
# 2012 Mar 09 - Nick Elias, NRAO
#               Initial version

# ------------------------------------------------------------------------------

def gcal_desc_st():

	# Create the gain calibration subtable data description dictionary

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
