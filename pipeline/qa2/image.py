
# ------------------------------------------------------------------------------
# Subsystem module ICD
# ------------------------------------------------------------------------------

# image.py

# Description:
# ------------
# This module runs the image statistics subsystem of the QA2 system.

# Notes:
# ------
# 1) To add statistics, the functions image_calc(), image_calc_stats(), and
#    image_desc() must be modified.

# User functions (subsystem and component level):
# -----------------------------------------------
# image       - This function runs the image statistics subsystem of the QA2
#               system.
#
# image_calc  - This function calculates the image statistics.
# image_write - This function writes the image statistics to a CASA table.

# Non-user functions (unit level):
# --------------------------------
# image_calc_stats - This function calculates the image statistics.
#
# image_desc       - This function creates the description of the image
#                    statistics table.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version created with functions image(), image_calc(),
#               and image_write().
# 2011 Jun 20 - Nick Elias, NRAO
#               Function image_desc() added.
# 2011 Jun 21 - Nick Elias, NRAO
#               Function image_record() added.
# 2011 Jun 22 - Nick Elias, NRAO
#               Dictionary at top-level of module scope added.  It contains the
#               statistics names (keys) and comments (elements).
# 2011 Aug 29 - Nick Elias, NRAO
#               Function image_calc_fix() added.
# 2011 Aug 30 - Nick Elias, NRAO
#               Function image_calc_stats() added.
# 2011 Sep 01 - Nick Elias, NRAO
#               Logging functionality incorporated.
# 2011 Sep 10 - Nick Elias, NRAO
#               Local logging classes employed.
# 2011 Oct 13 - Nick Elias, NRAO
#               Module cube.py imported.  The functions within it, cube_fix()
#               and cube_record() replace image_calc_fix() and image_record(),
#               respectively.
# 2012 Aug 01 - Nick Elias, NRAO
#               Tool calls modified to new format.

# ------------------------------------------------------------------------------

# Imports
# -------

import os
import shutil

import numpy

import casa
try:
    from casac import casac
except:
    pass

import utility.cube as cube
import utility.logs as logs

# ------------------------------------------------------------------------------
# Subsystem user function and ICD
# ------------------------------------------------------------------------------

# image

# Description:
# ------------
# This function runs the image statistics subsystem of the QA2 system.

# Inputs:
# -------
# in_image - This python string contains the name of the input image.
# out_dir  - This python string contains the name of the output directory.
# logobj   - This python variable contains the logging object information
#            ('PYTHON' creates a local python Logger class, 'CASA' creates a
#            local python logger, <python> and <casa> are actual instances of
#            the logger classes; default = 'PYTHON').

# Outputs:
# --------
# If the function finishes successfully, True is returned via the function
# value.  If the function does not finish successfully, it throws an exception.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version.
# 2011 Jun 13 - Nick Elias, NRAO
#               Improved error handling.
# 2011 Jun 20 - Nick Elias, NRAO
#               Raise an exception if the output table already exists.
# 2011 Sep 10 - Added logging capability.

# ------------------------------------------------------------------------------

def image( in_image, out_dir, logobj='PYTHON' ):

	# Initialize the logger

	root = 'image.image'
	level = logs.INFO

	log_local, logger = logs.init( out_dir, root, level, logobj )


	# Print the first log message

	msg = 'Imaging statistics started for ' + in_image + ' ...'
	origin = root
	logger.info( msg, origin=origin )


	# Check to see if the output file already exists.  The correct suffix is
	# appended non-MFS continuum, MFS continuum, and cube images.

	index = in_image.rfind( '.image' )

	out_table_root = os.path.basename( in_image[0:index] )
        out_table_suffix = in_image[index:] + '.stats'
	out_table = out_dir + '/' + out_table_root + out_table_suffix

	if os.path.exists( out_table ):
		msg = 'Output table ' + out_table + ' already exists ...\n'
		origin = root
		logger.error( msg, origin=origin )
		raise IOError( msg )


	# Calculate the imaging statistics

	try:
		image_stats = image_calc( in_image, logger=logger )
	except Exception, err:
		origin = root
		logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )

	msg = 'Imaging statistics of ' + in_image + ' calculated ...'
	origin = root
	logger.info( msg, origin=origin )


	# Write the imaging statistics to the table

	status = image_write( image_stats, out_table )

	msg = 'Imaging statistics written in ' + out_table + ' ...'
	origin = root
	logger.info( msg, origin=origin )


	# Print the last log message

	msg = 'Imaging statistics finished for ' + in_image + ' ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Delete the logger, if it was created locally

	if log_local: del logger


	# Return True

	return True

# ------------------------------------------------------------------------------
# Component user functions and ICDs
# ------------------------------------------------------------------------------

# image_calc

# Description:
# ------------
# This function calculates the image statistics.

# Inputs:
# -------
# in_image - This python string contains the name of the image.
# logger   - This python variable contains the logging object information
#            (<python> and <casa> are actual instances of the logger classes).
#            The default is '', which means that the log information is either
#            sent to stdout or raised.

# Outputs:
# --------
# The dictionary containing the image statistics, returned via the function
# value.  If there are no mask boxes, the in and out values are identical.  The
# keys:
# 'Stokes'        - The numpy string array containing the Stokes parameters.
# 'frequency'     - The numpy float array containing the frequencies.
# 'sum_in'        - The numpy float array containing the sum values inside the
#                   mask boxes.  The indices represent the Stokes parameters and
#                   channels.
# 'min_out'       - The numpy float array containing the minimum values outside
#                   the mask boxes.  The indices represent the Stokes parameters
#                   and channels.
# 'max_in'        - The numpy float array containing the maximum values inside
#                   the mask boxes.  The indices represent the Stokes parameters
#                   and channels.
# 'max_out'       - The numpy float array containing the maximum values outside
#                   the mask boxes.  The indices represent the Stokes parameters
#                   and channels.
# 'mean_out'      - The numpy float array containing the mean values outside the
#                   mask boxes.  The indices represent the Stokes parameters and
#                   channels.
# 'sigma_out'     - The numpy float array containing the standard deviation
#                   values outside the mask boxes.  The indices represent the
#                   Stokes parameters and channels.
# 'rms_out'       - The numpy float array containing the RMS values outside the
#                   mask boxes.  The indices represent the Stokes parameters and
#                   channels.
# 'median_out'    - The numpy float array containing the median values outside
#                   the mask boxes.  The indices represent the Stokes parameters
#                   and channels.
# 'medabsdev_out' - The numpy float array containing the absolute median
#                   deviation values outside the mask boxes.  The indices
#                   represent the Stokes parameters and channels.
# 'mask'          - The numpy boolean array containing the mask booleans.  The
#                   indices represent the Stokes parameters and channels.
# 'dynrange'      - The numpy float array containing the dynamic ranges (max_in
#                   divided by rms_out).  the indices represent the Stokes
#                   parameters and channels.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version (stub).
# 2011 Jun 17 - Nick Elias, NRAO
#               Image analysis performed over entire image cube.  Calculating
#               statistics only in selected region(s) not yet included.
# 2011 Aug 29 - Nick Elias, NRAO
#               Function modified to handle masks.  Moved much of the code to
#               other functions, for better design.
# 2011 Aug 31 - Nick Elias, NRAO
#               Employing casa.immath() task instead of the image analysis tool
#               to create masked images.
# 2011 Sep 10 - Nick Elias, NRAO
#               Logging capability added.
# 2011 Sep 30 - Nick Elias, NRAO
#               The logger input parameter can also take a '' value.

# ------------------------------------------------------------------------------

def image_calc( in_image, logger='' ):

	# Initialize the variables

	image_stats = dict()

	mask = os.path.splitext( in_image )[0] + '.mask'
	mask_image = os.path.splitext( in_image )[0] + '.masked_temp'

	mask_flag = os.path.exists( mask )


	# Calculate the statistics based on whether a mask exists or not

	if mask_flag:

		casa.immath( imagename=[in_image,mask], mode='evalexpr',
                             expr='IM0*IM1', varnames='', outfile=mask_image,
                             mask='', region='', box='', chans='', stokes='' )

		ib = image_calc_stats( mask_image )
		shutil.rmtree( mask_image )

		casa.immath( imagename=[in_image,mask], mode='evalexpr',
                             expr='IM0*(1.0-IM1)', varnames='',
                             outfile=mask_image, mask='', region='', box='',
                             chans='', stokes='' )

		ob = image_calc_stats( mask_image )
		shutil.rmtree( mask_image )

		image_stats['sum_in'] = numpy.copy( ib['sum'] )
		image_stats['min_out'] = numpy.copy( ob['min'] )
		image_stats['max_in'] = numpy.copy( ib['max'] )
		image_stats['max_out'] = numpy.copy( ob['max'] )
		image_stats['mean_out'] = numpy.copy( ob['mean'] )
		image_stats['sigma_out'] = numpy.copy( ob['sigma'] )
		image_stats['rms_out'] = numpy.copy( ob['rms'] )
		image_stats['median_out'] = numpy.copy( ob['median'] )
		image_stats['medabsdev_out'] = numpy.copy( ob['medabsdev'] )

	else:

		nb = image_calc_stats( in_image )

		image_stats['sum_in'] = numpy.copy( nb['sum'] )
		image_stats['min_out'] = numpy.copy( nb['min'] )
		image_stats['max_in'] = numpy.copy( nb['max'] )
		image_stats['max_out'] = numpy.copy( nb['max'] )
		image_stats['mean_out'] = numpy.copy( nb['mean'] )
		image_stats['sigma_out'] = numpy.copy( nb['sigma'] )
		image_stats['rms_out'] = numpy.copy( nb['rms'] )
		image_stats['median_out'] = numpy.copy( nb['median'] )
		image_stats['medabsdev_out'] = numpy.copy( nb['medabsdev'] )


	# Add the mask boolean to the image statistics dictionary

        index_shape = numpy.shape( image_stats['sum_in'] )

	image_stats['mask'] = numpy.zeros( index_shape, dtype='bool' )
	image_stats['mask'][:] = mask_flag


	# Add the dynamic range to the image statistics dictionary.  If the
	# dynamic range is very large (>=1.0E+07), it means that there is
	# something wrong with the image and the dynamic range is set to zero.

	image_stats['dynrange'] = numpy.zeros( index_shape, dtype='float' )

	for index in numpy.ndindex(index_shape):
		rms_out = image_stats['rms_out'][index]
		max_in = image_stats['max_in'][index]
		dynrange_inv = rms_out / max_in
		if dynrange_inv >= 1.0E-07:
			image_stats['dynrange'][index] = 1.0 / dynrange_inv
		else:
			image_stats['dynrange'][index] = 0.0


	# Fix the dimensions of the numpy arrays in the image statistics
	# dictionary for specific cases

	try:
		image_stats = cube.cube_fix( in_image, image_stats, logger )
	except Exception, err:
		origin = 'image.image_calc'
		if logger != '': logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )


	# Return the dictionary containing the image statistics

	return image_stats

# ------------------------------------------------------------------------------

# image_write

# Description:
# ------------
# This function writes the image statistics to a CASA table.

# NB: The tablebrowser() tool calls may be optimum but they work well enough,
# especially considering the numpy array accessing issues.

# Inputs:
# -------
# image_stats - The python dictionary that contains the image statistics.
# out_table   - The python string that contains the output table name.

# Outputs:
# --------
# The image statistics table is created in the output directory. If the
# function finishes successfully, True is returned via the function value.  If
# the function does not finish successfully, it throws an exception.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version (stub).
# 2011 Jun 20 - Nick Elias, NRAO
#               Wrote preliminary version.  Writing frequencies and Stokes
#               parameters to output table not yet implemented.
# 2011 Jun 22 - Nick Elias, NRAO
#               Added capability to write frequencies and Stokes parameters.

# ------------------------------------------------------------------------------

def image_write( image_stats, out_table ):

	# Create the local instance of the table tool and create the output
	# table according to the description

        try:
	    tbLoc = casac.table()
        except:
            tbLoc = casa.__tablehome__.create()

	tbLoc.create( out_table, image_desc() )


	# Put the image statistics into the output table

	rS = range( numpy.shape( image_stats['Stokes'][:,0] )[0] )
	rf = range( numpy.shape( image_stats['Stokes'][0,:] )[0] )

	keys = image_stats.keys()

	row = 0

	for s in rS:
		for f in rf:

			tbLoc.addrows()

			for k in keys:
				cell = image_stats[k][s][f]
				tbLoc.putcol( k, cell, row, 1, 1 )

			row += 1


	# Close and delete the local table tool

	tbLoc.done()

	del tbLoc


	# Return True

	return True

# ------------------------------------------------------------------------------
# Unit functions and ICDs
# ------------------------------------------------------------------------------

# image_calc_stats

# Description:
# ------------
# This function calculates the image statistics.

# Inputs:
# -------
# in_image - This python string contains the name of the image.

# Outputs:
# --------
# The dictionary containing the image statistics, returned via the function
# value.  The keys:
# 'sum'       - The numpy float array containing the sum values.  The indices
#               represent the Stokes parameters and channels.
# 'min'       - The numpy float array containing the minimum values.  The
#               indices represent the Stokes parameters and channels.
# 'max'       - The numpy float array containing the maximum values.  The
#               indices represent the Stokes parameters and channels.
# 'mean'      - The numpy float array containing the mean values.  The indices
#               represent the Stokes parameters and channels.
# 'sigma'     - The numpy float array containing the standard deviation values.
#               The indices represent the Stokes parameters and channels.
# 'rms'       - The numpy float array containing the RMS values.  The indices
#               represent the Stokes parameters and channels.
# 'median'    - The numpy float array containing the median values.  The indices
#               represent the Stokes parameters and channels.
# 'medabsdev' - The numpy float array containing the absolute median deviation
#               values.  The indices represent the Stokes parameters and
#               channels.

# Modification history:
# ---------------------
# 2011 Aug 30 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def image_calc_stats( in_image ):

	# Create the local instance of the image analysis tool and open it

        try:
	    iaLoc = casac.image()
        except:
            iaLoc = casa.__imagehome__.create()

	iaLoc.open( in_image )


	# Calculate the image statistics

	plotstats = list()
	plotstats.append( 'sum' )
	plotstats.append( 'min' )
	plotstats.append( 'max' )
	plotstats.append( 'mean' )
	plotstats.append( 'sigma' )
	plotstats.append( 'rms' )
	plotstats.append( 'median' )
	plotstats.append( 'medabsdevmed' )

	stats = iaLoc.statistics( axes=[0,1], plotstats=plotstats )

	# A kludge, since the image analysis tool erroneously returns all
	# statistics
	keys = stats.keys()
	for k in keys:
		if plotstats.count( k ) == 0: del stats[k]

	# I like the shorter name
	stats['medabsdev'] = stats['medabsdevmed']
	del stats['medabsdevmed']


	# Close and delete the local image analysis tool

	iaLoc.done()

	del iaLoc


	# Return the dictionary containing the image statistics

	return stats

# ------------------------------------------------------------------------------

# image_desc

# Description:
# ------------
# This function creates the description of the image statistics table.  The
# description is a python dictionary whose elements correspond to table
# columns.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The python dictionary containing the description of the image statistics
# table, returned via the function value.

# Modification history:
# ---------------------
# 2011 Jun 20 - Nick Elias, NRAO
#               Initial version.
# 2011 Jun 23 - Nick Elias, NRAO
#               Rewrote the function to automatically populate the dictionary
#               the elements of the image statistics table.  It uses the
#               dictionary which is at the top-level scope of this module.
# 2011 Aug 30 - Nick Elias, NRAO
#               I reverted back to manually populating the dictionary because
#               its role changed slightly in the other functions of this module.

# ------------------------------------------------------------------------------

def image_desc():

	# Create the data description dictionary

	desc = dict()

	desc['Stokes'] = {
		'comment': 'Stokes parameter',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'string'
	}

	desc['frequency'] = {
		'comment': 'Frequency',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'float'
	}

	desc['mask'] = {
		'comment': 'mask',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'bool'
	}

	desc['sum_in'] = {
		'comment': 'sum inside boxes',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'float'
	}

	desc['min_out'] = {
		'comment': 'minimum outside boxes',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'float'
	}

	desc['max_in'] = {
		'comment': 'maximum inside boxes',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'float'
	}

	desc['max_out'] = {
		'comment': 'maximum outside boxes',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'float'
	}

	desc['mean_out'] = {
		'comment': 'mean outside boxes',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'float'
	}

	desc['sigma_out'] = {
		'comment': 'standard deviation outside boxes',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'float'
	}

	desc['rms_out'] = {
		'comment': 'RMS outside boxes',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'float'
	}

	desc['median_out'] = {
		'comment': 'median outside boxes',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'float'
	}

	desc['medabsdev_out'] = {
		'comment': 'median absolute deviation outside boxes',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'float'
	}

	desc['dynrange'] = {
		'comment': 'dynamic range',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 0,
		'valueType': 'float'
	}


	# Return the dictionary

	return desc
