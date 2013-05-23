
# ------------------------------------------------------------------------------

# fd.py

# Description:
# ------------
# This module contains file and directory utility functions.

# Functions:
# ----------
# change_file_ext - This function changes the extension of a file name.
# get_fd_expand   - This function expands '.' and '..' into full paths.
# get_fd_rstrip   - This function strips white space and '/' from the right of
#                   files and/or directories.
# get_file_match  - This function gets a list of file names from a list of input
#                   directories.  The file/table names match file name criteria.
#                   Wildcards are acceptable.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version created with function change_file_ext().
# 2011 Jun 07 - Nick Elias, NRAO
#               Function get_file_match() added.
# 2011 Jun 09 - Nick Elias, NRAO
#               Functions get_fd_expand() and get_fd_rstrip() added.

# ------------------------------------------------------------------------------

# Imports
# -------

import glob
import os
import types

# ------------------------------------------------------------------------------

# change_file_ext

# Description:
# ------------
# This function changes the extension of a file name.  The extension is defined
# as part of the file name after the final period.  It can add an additional
# suffix before the new extension, if desired.

# Inputs:
# -------
# file   - This python string contains the file name.
# suffix - This python string contains the additional suffix (default = '').
# ext    - This python string contains the new file extension (default = '').

# Outputs:
# --------
# The python string containing the file name with the new suffix and extension,
# returned via the function value.

# Modification history:
# ---------------------
# 2011 May 20 - Nicholas Elias, NRAO
#               Initial version.
# 2011 Jun 08 - Nicholas Elias, NRAO
#               Simplified using os.path.splitext()

# ------------------------------------------------------------------------------

def change_file_ext( file, suffix = '', ext = '' ):

	# Form and return the new file name

	return os.path.splitext( file )[0] + suffix + '.' + ext

# ------------------------------------------------------------------------------

# get_fd_expand

# Description:
# ------------
# This function expands '.' and '..' into full paths then returns.

# Inputs:
# -------
# fds - This python string or list of strings contains file and/or directory
#       names.  NB: Right side white spaces and '/' are stripped.  If the input
#       is a python string, it is converted to a python list with one python
#       string element.

# Outputs:
# --------
# The python list of strings containing the expanded paths, returned via the
# function value.

# Modification history:
# ---------------------
# 2011 Jun 09 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def get_fd_expand( fds ):

	# Fix the inputs

	fds = get_fd_rstrip( fds )


	# Expand '.' and '..' into full paths then return

	owd = os.getcwd()
	rfd = range( len( fds ) )

	for fd in rfd:
		if fds[fd] == '.':
			fds[fd] = owd
		elif fds[fd] == '..':
			os.chdir( '..' )
			fds[fd] = os.getcwd()
			os.chdir( owd )
		else:
			pass

	return fds

# ------------------------------------------------------------------------------

# get_fd_rstrip

# Description:
# ------------
# This function strips white space and '/' from the right of files and/or
# directories then returns.

# Inputs:
# -------
# fds - This python string or list of strings contains file and/or directory
#       names.  NB: If the input is a python string, it is converted to a python
#       list with one python string element.

# Outputs:
# --------
# The python list of strings containing the stripped files and/or directories,
# returned via the function value.

# Modification history:
# ---------------------
# 2011 Jun 09 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def get_fd_rstrip( fds ):

	# Fix the inputs

	if not isinstance( fds, types.ListType ): fds = [ fds ]


	# Strip white space and '/' from the right of the files and/or
	# directories and return

	rfd = range( len( fds ) )

	for fd in rfd: fds[fd] = fds[fd].rstrip( ' /' )

	return fds

# ------------------------------------------------------------------------------

# get_file_match

# Description:
# ------------
# This function gets a list of file names from a list of input directories.  The
# file names match criteria (wildcards are acceptable).  NB: 

# Inputs:
# -------
# in_dirs - This python list of strings contains the input directories to be
#           searched (default = ['.']).  NB: The full paths are required.  Right
#           side white spaces and '/' are stripped.  If the input is a python
#           string, it is converted to a python list with one python string
#           element.
# match   - This python string contains the glob match criteria (default = '',
#           don't match anything).  match = '*' matches everything.

# Outputs:
# --------
# The python list of strings containing the file names (full path).

# Modification history:
# ---------------------
# 2011 Jun 07 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def get_file_match( in_dirs = ['.'] , match = '' ):

	# Fix the inputs

	in_dirs = get_fd_expand( in_dirs )


	# Get the original working directory

	owd = os.getcwd()


	# Get the matching files and put them into the output list

	files = list()

	for in_dir in in_dirs:

		try:
			os.chdir( in_dir )
		except:
			msg = 'get_file_match: Invalid directory '
			msg += in_dir + '.\n'
			raise Exception( msg )

		tempfiles = glob.glob( match )
		for tempfile in tempfiles:
			file = os.path.join( in_dir + '/' + tempfile )
			files.append( file )


	# Go back to the original working directory

	os.chdir( owd )


	# Return the python list of strings containing the matching file names
	# (full path)

	return files
