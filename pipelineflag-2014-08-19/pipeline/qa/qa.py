
# ------------------------------------------------------------------------------
# System module ICD
# ------------------------------------------------------------------------------

# qa.py

# Description:
# ------------
# This module runs the entire QA system.

# User functions (system level):
# ------------------------------
# qa        - This function runs the entire QA system.
#
# qa_flag   - This function runs the QA flagging statistics subsystem.
# qa_gcal   - This function runs the QA gain calibration statistics subsystem.
# qa_bpcal  - This function runs the QA bandpass calibration statistics
#              subsystem.
# qa_polcal - This function runs the QA polarization calibration statistics
#              subsystem.
# qa_image  - This function runs the QA imaging statistics subsystem.
# qa_psf    - This function runs the QA psf information subsystem.
# qa_vis    - This function runs the QA visibility statistics subsystem.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version created with function qa().
# 2011 Jun 08 - Nick Elias, NRAO
#               Removed extraneous code.
# 2011 Sep 01 - Nick Elias, NRAO
#               Functions qa_flag(), qa_gcal(), qa_bpcal(), qa_polcal(),
#               qa_image(), qa_log_start(), and qa_log_stop() added.
# 2011 Sep 10 - Nick Elias, NRAO
#               Functions qa_log_start() and qa_log_stop() removed in favor of
#               local logging classes.
# 2011 Sep 29 - Nick Elias, NRAO
#               Imported psf.py module and added qa_psf() function.
# 2011 Oct 19 - Nick Elias, NRAO
#               Imported vis.py module and added qa_vis() function.

# ------------------------------------------------------------------------------

# Imports
# -------

import utility.fd as fd
import utility.logs as logs

import flag
import gcal
import bpcal
import polcal
import image
import psf
import vis

# ------------------------------------------------------------------------------
# System user functions and ICDs
# ------------------------------------------------------------------------------

# qa

# Description:
# ------------
# This function runs the entire QA system.

# Inputs:
# -------
# in_dirs - This python list of strings contains the full path input
#           directories.
# out_dir - This python string contains the output directory.
# logobj  - This python variable contains the logging object information
#           ('PYTHON' creates a local python Logger class, 'CASA' creates a
#           local python logger, <python> and <casa> are actual instances of the
#           logger classes; default = 'PYTHON').

# Outputs:
# --------
# If the function finishes successfully, True is returned via the function
# value.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version.
# 2011 Jun 13 - Nick Elias, NRAO
#               Improved error handling.
# 2011 Sep 01 - Nick Elias, NRAO
#               Logging functionality incorporated.  Calls to functions replace
#               much of the code in this function.
# 2011 Sep 10 - Nick Elias, NRAO
#               log_level input parameter removed and replaced with logobj
#               parameter.
# 2011 Sep 29 - Nick Elias, NRAO
#               Incorporated a call to the qa_psf() function.
# 2011 Oct 19 - Nick Elias, NRAO
#               Incorporated a call to the qa_vis() function.
# 2012 Mar 16 - Nick Elias, NRAO
#               Changed function calls to qa_gcal() and qa_bpcal().

# ------------------------------------------------------------------------------

def qa( in_dirs, out_dir, logobj='PYTHON' ):

	# Initialize the logger

	root = 'qa.qa'
	level = logs.INFO

	log_local, logger = logs.init( out_dir, root, level, logobj )


	# Print the first log message

	msg = 'QA system started ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Calculate and write the flagging statistics

	try:
		status = qa_flag( in_dirs, out_dir, logobj=logger )
	except Exception, err:
		origin = root
		logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )


	# Calculate and write the gain calibration statistics

	try:
		status = qa_gcal( in_dirs, out_dir, logobj=logger )
	except Exception, err:
		origin = root
		logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )


	# Calculate and write the bandpass calibration statistics

	try:
		status = qa_bpcal( in_dirs, out_dir, logobj=logger )
	except Exception, err:
		origin = root
		logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )


	# Calculate and write the polarization calibration statistics

	try:
		status = qa_polcal( in_dirs, out_dir, logobj=logger )
	except Exception, err:
		origin = root
		logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )


	# Calculate and write the imaging statistics

	try:
		status = qa_image( in_dirs, out_dir, logobj=logger )
	except Exception, err:
		origin = root
		logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )


	# Calculate and write the psf information

	try:
		status = qa_psf( in_dirs, out_dir, logobj=logger )
	except Exception, err:
		origin = root
		logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )


	# Calculate and write the visibility statistics

	try:
		status = qa_vis( in_dirs, out_dir, logobj=logger )
	except Exception, err:
		origin = root
		logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )


	# Print the last log message

	msg = 'QA system finished ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Delete the logger, if it was created locally

	if log_local: del logger


	# Return True

	return True

# ------------------------------------------------------------------------------

# qa_flag

# Description:
# ------------
# This function runs the QA flagging statistics subsystem.

# Inputs:
# -------
# in_dirs - This python list of strings contains the full path input
#           directories.
# out_dir - This python string contains the output directory.
# logobj  - This python variable contains the logging object information
#           ('PYTHON' creates a local python Logger class, 'CASA' creates a
#           local python logger, <python> and <casa> are actual instances of the
#           logger classes; default = 'PYTHON').

# Outputs:
# --------
# If the function finishes successfully, True is returned via the function
# value.

# Modification history:
# ---------------------
# 2011 Sep 01 - Nick Elias, NRAO
#               Initial version.
# 2011 Sep 10 - Nick Elias, NRAO
#               logobj input parameter added.

# ------------------------------------------------------------------------------

def qa_flag( in_dirs, out_dir, logobj='PYTHON' ):

	# Initialize the logger

	root = 'qa.qa_flag'
	level = logs.INFO

	log_local, logger = logs.init( out_dir, root, level, logobj )


	# Print the first log message

	msg = 'Flagging statistics subsystem started ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Calculate and write the flagging statistics

	mses = fd.get_file_match( in_dirs, match='*.ms' )

	for ms in mses:
		try:
			status = flag.flag( ms, out_dir, logobj=logger )
		except Exception, err:
			origin = root
			logger.error( err.args[0], origin=origin )
			raise Exception( err.args[0] )


	# Print the last log message

	msg = 'Flagging statistics subsystem finished ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Delete the logger, if it was created locally

	if log_local: del logger


	# Return True

	return True

# ------------------------------------------------------------------------------

# qa_gcal

# Description:
# ------------
# This function runs the QA gain calibration statistics subsystem.

# Inputs:
# -------
# in_dirs - This python list of strings contains the full path input
#           directories.
# out_dir - This python string contains the output directory.
# logobj  - This python variable contains the logging object information
#           ('PYTHON' creates a local python Logger class, 'CASA' creates a
#           local python logger, <python> and <casa> are actual instances of the
#           logger classes; default = 'PYTHON').

# Outputs:
# --------
# If the function finishes successfully, True is returned via the function
# value.

# Modification history:
# ---------------------
# 2011 Sep 01 - Nick Elias, NRAO
#               Initial version.
# 2011 Sep 10 - Nick Elias, NRAO
#               logobj input parameter added.
# 2012 Mar 16 - Nick Elias, NRAO
#               Added the spw_in input parameter.
# 2012 May 15 - Nick Elias, NRAO
#               Removed the spw_in input parameter.

# ------------------------------------------------------------------------------

def qa_gcal( in_dirs, out_dir, logobj='PYTHON' ):

	# Initialize the logger

	root = 'qa.qa_gcal'
	level = logs.INFO

	log_local, logger = logs.init( out_dir, root, level, logobj )


	# Print the first log message

	msg = 'Gain calibration statistics subsystem started ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Calculate and write the gain calibration statistics

	tables = fd.get_file_match( in_dirs, match='*.gcal*' )

	for table in tables:
		try:
			status = gcal.gcal( table, out_dir )
		except Exception, err:
			origin = root
			logger.error( err.args[0], origin=origin )
			raise Exception( err.args[0] )


	# Print the last log message

	msg = 'Gain calibration statistics subsystem finished ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Delete the logger, if it was created locally

	if log_local: del logger


	# Return True

	return True

# ------------------------------------------------------------------------------

# qa_bpcal

# Description:
# ------------
# This function runs the QA bandpass calibration statistics subsystem.

# Inputs:
# -------
# in_dirs - This python list of strings contains the full path input
#           directories.
# out_dir - This python string contains the output directory.
# logobj  - This python variable contains the logging object information
#           ('PYTHON' creates a local python Logger class, 'CASA' creates a
#           local python logger, <python> and <casa> are actual instances of the
#           logger classes; default = 'PYTHON').

# Outputs:
# --------
# If the function finishes successfully, True is returned via the function
# value.

# Modification history:
# ---------------------
# 2011 Sep 01 - Nick Elias, NRAO
#               Initial version.
# 2011 Sep 10 - Nick Elias, NRAO
#               logobj input parameter added.
# 2012 Mar 16 - Nick Elias, NRAO
#               Added the spw_in input parameter.
# 2012 May 15 - Nick Elias, NRAO
#               Removed the spw_in input parameter.

# ------------------------------------------------------------------------------

def qa_bpcal( in_dirs, out_dir, logobj='PYTHON' ):

	# Initialize the logger

	root = 'qa.qa_bpcal'
	level = logs.INFO

	log_local, logger = logs.init( out_dir, root, level, logobj )


	# Print the first log message

	msg = 'Bandpass calibration statistics subsystem started ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Calculate and write the bandpass calibration statistics

	tables = fd.get_file_match( in_dirs, match='*.bpcal*' )

	for table in tables:
		try:
			status = bpcal.bpcal( table, out_dir )
		except Exception, err:
			origin = root
			logger.error( err.args[0], origin=origin )
			raise Exception( err.args[0] )


	# Print the last log message

	msg = 'Bandpass calibration statistics subsystem finished ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Delete the logger, if it was created locally

	if log_local: del logger


	# Return True

	return True

# ------------------------------------------------------------------------------

# qa_polcal

# Description:
# ------------
# This function runs the QA polarization calibration statistics subsystem.

# Inputs:
# -------
# in_dirs - This python list of strings contains the full path input
#           directories.
# out_dir - This python string contains the output directory.
# logobj  - This python variable contains the logging object information
#           ('PYTHON' creates a local python Logger class, 'CASA' creates a
#           local python logger, <python> and <casa> are actual instances of the
#           logger classes; default = 'PYTHON').

# Outputs:
# --------
# If the function finishes successfully, True is returned via the function
# value.

# Modification history:
# ---------------------
# 2011 Sep 01 - Nick Elias, NRAO
#               Initial version.
# 2011 Sep 10 - Nick Elias, NRAO
#               logobj input parameter added.

# ------------------------------------------------------------------------------

def qa_polcal( in_dirs, out_dir, logobj='PYTHON' ):

	# Initialize the logger

	root = 'qa.qa_polcal'
	level = logs.INFO

	log_local, logger = logs.init( out_dir, root, level, logobj )


	# Print the first log message

	msg = 'Polarization calibration statistics subsystem started ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Calculate and write the polarization calibration statistics

	tables = fd.get_file_match( in_dirs, match='*.polcal*' )

	for table in tables:
		try:
			status = polcal.polcal( table, out_dir )
		except Exception, err:
			origin = root
			logger.error( err.args[0], origin=origin )
			raise Exception( err.args[0] )


	# Print the last log message

	msg = 'Polarization calibration statistics subsystem finished ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Delete the logger, if it was created locally

	if log_local: del logger


	# Return True

	return True

# ------------------------------------------------------------------------------

# qa_image

# Description:
# ------------
# This function runs the QA imaging statistics subsystem.

# Inputs:
# -------
# in_dirs - This python list of strings contains the full path input
#           directories.
# out_dir - This python string contains the output directory.
# logobj  - This python variable contains the logging object information
#           ('PYTHON' creates a local python Logger class, 'CASA' creates a
#           local python logger, <python> and <casa> are actual instances of the
#           logger classes; default = 'PYTHON').

# Outputs:
# --------
# If the function finishes successfully, True is returned via the function
# value.

# Modification history:
# ---------------------
# 2011 Sep 01 - Nick Elias, NRAO
#               Initial version.
# 2011 Sep 10 - Nick Elias, NRAO
#               logobj input parameter added.

# ------------------------------------------------------------------------------

def qa_image( in_dirs, out_dir, logobj='PYTHON' ):

	# Initialize the logger

	root = 'qa.qa_image'
	level = logs.INFO

	log_local, logger = logs.init( out_dir, root, level, logobj )


	# Print the first log message

	msg = 'Imaging statistics subsystem started ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Calculate and write the imaging statistics

	ims = fd.get_file_match( in_dirs, match='*.image' )

	ims_tt0 = fd.get_file_match( in_dirs, match='*.image.tt0' )
	for im in ims_tt0: ims.append( im )
	
	for im in ims:
		try:
			status = image.image( im, out_dir, logobj=logger )
		except Exception, err:
			origin = root
			logger.error( err.args[0], origin=origin )
			raise Exception( err.args[0] )


	# Print the last log message

	msg = 'Imaging statistics subsystem finished ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Delete the logger, if it was created locally

	if log_local: del logger


	# Return True

	return True

# ------------------------------------------------------------------------------

# qa_psf

# Description:
# ------------
# This function runs the QA psf information subsystem.

# Inputs:
# -------
# in_dirs - This python list of strings contains the full path input
#           directories.
# out_dir - This python string contains the output directory.
# logobj  - This python variable contains the logging object information
#           ('PYTHON' creates a local python Logger class, 'CASA' creates a
#           local python logger, <python> and <casa> are actual instances of the
#           logger classes; default = 'PYTHON').

# Outputs:
# --------
# If the function finishes successfully, True is returned via the function
# value.

# Modification history:
# ---------------------
# 2011 Sep 29 - Nick Elias, NRAO
#               Initial version.
# 2011 Oct 17 - Nick Elias, NRAO
#               The fit_factor and stats_thresh parameters are fed to the
#               psf.psf() function, but they cannot be changed from the
#               qa.qa_psf() input parameters (QA is automated).

# ------------------------------------------------------------------------------

def qa_psf( in_dirs, out_dir, logobj='PYTHON' ):

	# Initialize the logger

	root = 'qa.qa_psf'
	level = logs.INFO

	log_local, logger = logs.init( out_dir, root, level, logobj )


	# Print the first log message

	msg = 'PSF information subsystem started ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Calculate and write the psf information

	ps = fd.get_file_match( in_dirs, match='*.psf' )
	
	for p in ps:
		try:
			status = psf.psf( p, out_dir, fit_factor=2.0,
                            stats_thresh=0.3, logobj=logger )
		except Exception, err:
			origin = root
			logger.error( err.args[0], origin=origin )
			raise Exception( err.args[0] )


	# Print the last log message

	msg = 'PSF information subsystem finished ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Delete the logger, if it was created locally

	if log_local: del logger


	# Return True

	return True

# ------------------------------------------------------------------------------

# qa_vis

# Description:
# ------------
# This function runs the QA visibility statistics subsystem.

# Inputs:
# -------
# in_dirs - This python list of strings contains the full path input
#           directories.
# out_dir - This python string contains the output directory.
# logobj  - This python variable contains the logging object information
#           ('PYTHON' creates a local python Logger class, 'CASA' creates a
#           local python logger, <python> and <casa> are actual instances of the
#           logger classes; default = 'PYTHON').

# Outputs:
# --------
# If the function finishes successfully, True is returned via the function
# value.

# Modification history:
# ---------------------
# 2011 Oct 19 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

# vis module is missing

#def qa_vis( in_dirs, out_dir, logobj='PYTHON' ):
#
#	# Initialize the logger
#
#	root = 'qa.qa_vis'
#	level = logs.INFO
#
#	log_local, logger = logs.init( out_dir, root, level, logobj )
#
#
#	# Print the first log message
#
#	msg = 'Visibility statistics subsystem started ...\n'
#	origin = root
#	logger.info( msg, origin=origin )
#
#
#	# Calculate and write the visibility statistics
#
#	mses = fd.get_file_match( in_dirs, match='*.ms' )
#
#	for ms in mses:
#		try:
#			status = vis.vis( ms, out_dir, logobj=logger )
#		except Exception, err:
#			origin = root
#			logger.error( err.args[0], origin=origin )
#			raise Exception( err.args[0] )
#
#
#	# Print the last log message
#
#	msg = 'Visibility statistics subsystem finished ...\n'
#	origin = root
#	logger.info( msg, origin=origin )
#
#
#	# Delete the logger, if it was created locally
#
#	if log_local: del logger
#
#
#	# Return True
#
#	return True
