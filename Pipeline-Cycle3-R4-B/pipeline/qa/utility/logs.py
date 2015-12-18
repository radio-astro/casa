
# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

# logs.py

# Description:
# ------------
# This file contains functions and classes that create and configure loggers.
# These comments represent the module ICD.  The module functions and classes
# have their own ICDs (located after the import statements).  The member
# functions of the classes also have their own ICD.

# Functions:
# ----------
# init - This module function creates an instance of either a logsCASA() or
#        logsPYTHON() class.

# Classes:
# --------
# logsCASA   - This class wraps the CASA logger.
# logsPYTHON - This class wraps the python Logger class.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version created with the function init() and the classes
#               logsCASA() and logsPYTHON().

# ------------------------------------------------------------------------------

# Imports
# -------

import logging
import time

import casa

# ------------------------------------------------------------------------------

# Module variables
# ----------------

DEBUG    = logging.DEBUG
INFO     = logging.INFO
WARNING  = logging.WARNING
ERROR    = logging.ERROR
CRITICAL = logging.CRITICAL

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

# init

# Description:
# ------------
# This module function creates an instance of either a logsCASA() or
# logsPYTHON() class.

# Inputs:
# -------
# out_dir - This python string contains the output directory.
# root    - This python string contains the root of the output file name.
# level   - This python integer contains the logging level.
# logobj  - This python variable contains the logging object information
#           ('PYTHON' creates a local python Logger class, 'CASA' creates a
#           local python logger, <python> and <casa> are actual instances of the
#           logger classes).

# Outputs:
# --------
# The local log boolean and logger class instance tuple, returned via the
# function value.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def init( out_dir, root, level, logobj ):

	# Create the logger instance

	if logobj == 'CASA':

		log_local = True
		logger = logsCASA()

	elif logobj == 'PYTHON':

		log_local = True
		logger = logsPYTHON( out_dir, root=root, level=level )

	else:

		log_local = False
		logger = logobj


	# Return the local log flag and the logger instance as a tuple

	return log_local, logger

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

# logsCASA

# Description:
# ------------
# This class wraps the CASA logger.  NB: The CASA logger has limited log levels.
# Member functions that correspond the python logger log levels map messages to
# the CASA logger log messages.

# Member functions:
# -----------------
# __init__ - This member function constructs an instance of this class.
# __del__  - This member function destructs an instance of this class.
# critical - This member function prints a critical message to the CASA logger.
# debug    - This member function prints a debug message to the CASA logger.
# error    - This member function prints an error message to the CASA logger.
# info     - This member function prints an info message to the CASA logger.
# warning  - This member function prints a warning message to the CASA logger.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version created with member functions __init__(),
#               __del__(), critical(), debug(), error(), info(), and warning().

# ------------------------------------------------------------------------------

class logsCASA:

# ------------------------------------------------------------------------------

# logsCASA.__init__

# Description:
# ------------
# This member function constructs an instance of this class.

# Inputs:
# -------
# self - The built-in python variable.

# Outputs:
# -------
# The python value None, returned via the function value.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def __init__( self ):

		# Create the instance of the CASA logger

		myLog = casa.casac.homefinder.find_home_by_name( 'logsinkHome' )

		self._casaLog = myLog.create()


		# Return None

		return None

# ------------------------------------------------------------------------------

# logsCASA.__del__

# Description:
# ------------
# This member function destructs an instance of this class.

# Inputs:
# -------
# self - The built-in python variable.

# Outputs:
# --------
# The python value None, returned via the function value.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def __del__( self ):

		# Destruct the CASA logger instance

		del self._casaLog


		# Return None

		return None

# ------------------------------------------------------------------------------

# logsCASA.critical

# Description:
# ------------
# This member function posts a critical message to the CASA logger.  NB: The
# CASA logger has no CRITICAL level, so the SEVERE level is used.

# Inputs:
# ------
# self   - The built-in python variable.
# msg    - This python string contains the message.
# origin - This python string contains the message origin (default = '' ).

# Outputs:
# --------
# The python value None, returned via the function value.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def critical( self, msg, origin='' ):

		# Post the message to the CASA logger

		self._casaLog.post( msg, priority='SEVERE', origin=origin )


		# Return None

		return None

# ------------------------------------------------------------------------------

# logsCASA.debug

# Description:
# ------------
# This member function posts a debug message to the CASA logger.  NB: The CASA
# logger has no DEBUG level, so the INFO level is used.

# Inputs:
# ------
# self   - The built-in python variable.
# msg    - This python string contains the message.
# origin - This python string contains the message origin (default = '' ).

# Outputs:
# --------
# The python value None, returned via the function value.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def debug( self, msg, origin='' ):

		# Post the message to the CASA logger

		self._casaLog.post( msg, priority='INFO', origin=origin )


		# Return None

		return None

# ------------------------------------------------------------------------------

# logsCASA.error

# Description:
# ------------
# This member function posts an error message to the CASA logger.  NB: The CASA
# logger has no ERROR level, so the SEVERE level is used.

# Inputs:
# ------
# self   - The built-in python variable.
# msg    - This python string contains the message.
# origin - This python string contains the message origin (default = '' ).

# Outputs:
# --------
# The python value None, returned via the function value.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def error( self, msg, origin='' ):

		# Post the message to the CASA logger

		self._casaLog.post( msg, priority='SEVERE', origin=origin )


		# Return None

		return None

# ------------------------------------------------------------------------------

# logsCASA.info

# Description:
# ------------
# This member function posts an info message to the CASA logger.

# Inputs:
# ------
# self   - The built-in python variable.
# msg    - This python string contains the message.
# origin - This python string contains the message origin (default = '' ).

# Outputs:
# --------
# The python value None, returned via the function value.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def info( self, msg, origin='' ):

		# Post the message to the CASA logger

		self._casaLog.post( msg, priority='INFO', origin=origin )


		# Return None

		return None

# ------------------------------------------------------------------------------

# logsCASA.warning

# Description:
# ------------
# This member function posts a warning message to the CASA logger.  NB: The CASA
# logger has no WARNING level, so the INFO level is used.

# Inputs:
# ------
# self   - The built-in python variable.
# msg    - This python string contains the message.
# origin - This python string contains the message origin (default = '' ).

# Outputs:
# --------
# The python value None, returned via the function value.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def warning( self, msg, origin='' ):

		# Post the message to the CASA logger

		self._casaLog.post( msg, priority='INFO', origin=origin )


		# Return None

		return None

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

# logsPYTHON

# Description:
# ------------
# This class wraps the python Logger class.  

# Member functions:
# -----------------
# __init__  - This member function constructs an instance of this class.
# __del__   - This member function destructs an instance of this class.
# critical  - This member function prints a critical message to the python
#             logger.
# debug     - This member function prints a debug message to the python logger.
# dir_name  - This member function returns the output directory name.
# error     - This member function prints an error message to the python logger.
# file_name - This member function returns the log file name.
# info      - This member function prints an info message to the python logger.
# level     - This member function returns the minimum log level.
# root      - This member function returns the root of the output file name.
# warning   - This member function prints a warning message to the python
#             logger.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version created with member functions __init__(),
#               __del__(), dir_name(), file_name(), level(), and root().
# 2011 Sep 10 - Nick Elias, NRAO
#               Member functions critical(), debug(), error(), info(), and
#               warning().

# ------------------------------------------------------------------------------

class logsPYTHON:

# ------------------------------------------------------------------------------

# logsPYTHON.__init__

# Description:
# ------------
# This member function constructs an instance of this class.

# Inputs:
# -------
# self     - The built-in python variable.
# dir_name - This python string contains the output directory.
# root     - This python string contains the root of the output file name.
# level    - This python int contains the level for the logging module.  The
#            allowed values are DEBUG, INFO, WARNING, ERROR, and CRITICAL
#            (module variables; default = INFO). 

# Outputs:
# --------
# The python None value, returned via the function value.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def __init__( self, dir_name, root, level=INFO ):

		# Initialize the "private" variables

		self._dir_name = dir_name
		self._root = root
		self._level = level

		dt = time.asctime( time.gmtime() ).replace( ' ', '-' )
		self._file_name = dir_name + '/' + root + '-' + dt + '.log'


		# Start up and configure the file handler

		self._fh = logging.FileHandler( self._file_name )

		self._fh.setLevel( self._level )

		format = '%(levelname)s %(message)s'
		self._fh.setFormatter( logging.Formatter( format ) )


		# Start up and configure the Logger.  This pointer is used from
		# outside the class to write to the log.

		self._Logger = logging.getLogger()

		self._Logger.setLevel( self._level )
		self._Logger.addHandler( self._fh )


		# Return None

		return None

# ------------------------------------------------------------------------------

# logsPYTHON.__del__

# Description:
# ------------
# This member function destructs an instance of this class.

# Inputs:
# -------
# self - The built-in python variable.

# Outputs:
# --------
# The python None value, returned via the function value.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def __del__( self ):

		# Remove the file handler from the logger, close the file
		# handler, and delete file handler and logger

		self._Logger.removeHandler( self._fh )
		self._fh.close()

		del self._fh
		del self._Logger


		# Return None

		return None

# ------------------------------------------------------------------------------

# logsPYTHON.critical

# Description:
# ------------
# This member function posts a critical message to the python logger.

# Inputs:
# ------
# self   - The built-in python variable.
# msg    - This python string contains the message.
# origin - This python string contains the message origin (default = '' ).

# Outputs:
# --------
# The python value None, returned via the function value.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def critical( self, msg, origin='' ):

		# Print the message to the python logger

		self._Logger.critical( origin + ': ' + msg )


		# Return None

		return None

# ------------------------------------------------------------------------------

# logsPYTHON.debug

# Description:
# ------------
# This member function posts a debug message to the python logger.

# Inputs:
# ------
# self   - The built-in python variable.
# msg    - This python string contains the message.
# origin - This python string contains the message origin (default = '' ).

# Outputs:
# --------
# The python value None, returned via the function value.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def debug( self, msg, origin='' ):

		# Print the message to the python logger

		self._Logger.debug( origin + ': ' + msg )


		# Return None

		return None

# ------------------------------------------------------------------------------

# logsPYTHON.dir_name

# Description:
# ------------
# This member function returns the output directory name.

# Inputs:
# -------
# self - The built-in python variable.

# Outputs:
# --------
# The python string containing the output directory name.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def dir_name( self ):

		# Return the output directory

		return self._dir_name

# ------------------------------------------------------------------------------

# logsPYTHON.error

# Description:
# ------------
# This member function posts an error message to the python logger.

# Inputs:
# ------
# self   - The built-in python variable.
# msg    - This python string contains the message.
# origin - This python string contains the message origin (default = '' ).

# Outputs:
# --------
# The python value None, returned via the function value.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def error( self, msg, origin='' ):

		# Print the message to the python logger

		self._Logger.error( origin + ': ' + msg )


		# Return None

		return None

# ------------------------------------------------------------------------------

# logsPYTHON.file_name

# Description:
# ------------
# This member function returns the log file name.

# Inputs:
# -------
# self - The built-in python variable.

# Outputs:
# --------
# The python string containing the log file name.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def file_name( self ):

		# Return the log file name

		return self._file_name

# ------------------------------------------------------------------------------

# logsPYTHON.info

# Description:
# ------------
# This member function posts an info message to the python logger.

# Inputs:
# ------
# self   - The built-in python variable.
# msg    - This python string contains the message.
# origin - This python string contains the message origin (default = '' ).

# Outputs:
# --------
# The python value None, returned via the function value.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def info( self, msg, origin='' ):

		# Print the message to the python logger

		self._Logger.info( origin + ': ' + msg )


		# Return None

		return None

# ------------------------------------------------------------------------------

# logsPYTHON.level

# Description:
# ------------
# This member function returns the minimum log level.

# Inputs:
# -------
# self - The built-in python variable.

# Outputs:
# --------
# The python integer containing the minimum log level.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def level( self ):

		# Return the log level

		return self._level

# ------------------------------------------------------------------------------

# logsPYTHON.root

# Description:
# ------------
# This member function returns the root of the output file name.

# Inputs:
# -------
# self - The built-in python variable.

# Outputs:
# --------
# The python string containing the root of the output file name.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def root( self ):

		# Return the root of the output file name

		return self._root

# ------------------------------------------------------------------------------

# logsPYTHON.warning

# Description:
# ------------
# This member function posts a warning message to the python logger.

# Inputs:
# ------
# self   - The built-in python variable.
# msg    - This python string contains the message.
# origin - This python string contains the message origin (default = '' ).

# Outputs:
# --------
# The python value None, returned via the function value.

# Modification history:
# ---------------------
# 2011 Sep 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def warning( self, msg, origin='' ):

		# Print the message to the python logger

		self._Logger.warning( origin + ': ' + msg )


		# Return None

		return None
