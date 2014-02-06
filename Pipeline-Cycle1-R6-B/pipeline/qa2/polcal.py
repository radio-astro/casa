
# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

# polcal.py

# Description:
# ------------
# This module runs the polarization calibration statistics subsystem of the QA2
# system.

# User functions:
# ---------------
# polcal       - This function runs the polarization calibration statistics
#                subsystem of the QA2 system.
# polcal_calc  - This function calculates the polarization calibration
#                statistics.
# polcal_write - This function writes the polarization calibration statistics to
#                a CASA table.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version created with functions polcal(), polcal_calc(),
#               and polcal_write().

# ------------------------------------------------------------------------------

# Imports
# -------

import os

import casa

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

# polcal

# Description:
# ------------
# This function runs the polarization calibration statistics subsystem of the
# QA2 system.

# Inputs:
# -------
# in_table - This python string contains the name of the input polarization
#            calibration table.
# out_dir  - This python string contains the output directory.

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

# ------------------------------------------------------------------------------

def polcal( in_table, out_dir ):

	# Calculate the polarization calibration statistics

	try:
		polcal_stats = polcal_calc( in_table )
	except:
		msg = 'polcal.polcal: Calculating polarization calibration '
		msg += 'statistics failed.\n'
		print msg
		raise


	# Write the polarization calibration statistics to the table

	out_table_root = os.path.basename( os.path.splitext( in_table )[0] )
	out_table = out_dir + '/' + out_table_root + '.polcal.stats'

	if os.path.exists( out_table ):
		msg = 'polcal.polcal: Output file already exists.\n'
		print msg
		raise

	try:
		status = polcal_write( polcal_stats, out_table )
	except:
		msg = 'polcal.polcal: Writing polarization calibration '
		msg += 'statistics failed.\n'
		print msg
		raise


	# Return True

	return True

# ------------------------------------------------------------------------------

# polcal_calc

# Description:
# ------------
# This function calculates the polarization calibration statistics.

# Inputs:
# -------
# in_table - This python string contains the name of the input polarization
#            calibration table.

# Outputs:
# --------
# The dictionary containing the polarization calibration statistics, returned
# via the function value.  If the function does not finish successfully, an
# exception is raised.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version (stub).

# ------------------------------------------------------------------------------

def polcal_calc( in_table ):

	polcal_stats = {}

	# Return the dictionary containing the polarization calibration
	# statistics

	return polcal_stats

# ------------------------------------------------------------------------------

# polcal_write

# Description:
# ------------
# This function writes the polarization calibration statistics to a CASA table.

# Inputs:
# -------
# polcal_stats - The python dictionary that contains the polarization
#                calibation statistics.
# out_table    - The python string that contains the output table name.

# Outputs:
# --------
# The polarization calibration statistics table is created in the output
# directory. If the function finishes successfully, True is returned via the
# function value.  If the function does not finish successfully, it throws an
# exception.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version (stub).

# ------------------------------------------------------------------------------

def polcal_write( polcal_stats, out_table ):

	# Return True

	return True

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------
