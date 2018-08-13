
# ------------------------------------------------------------------------------
# Module ICD
# ------------------------------------------------------------------------------

# cube.py

# Description:
# ------------
# This module regularizes the elements of the cube statistics dictionary.

# Functions:
# ----------
# cube_fix    - This function fixes the dimensions of the numpy arrays in the
#               cube statistics dictionary when the number of Stokes parameters
#               and/or the number of frequencies is one.  This function also
#               adds the Stokes and frequency numpy arrays to the cube
#               statistics dictionary.
# cube_record - This function gets the Stokes parameters and frequencies from a
#               header record of a CASA cube.

# Modification history:
# ---------------------
# 2011 Oct 13 - Nick Elias, NRAO
#               Initial version created with functions cube_fix() and
#               cube_record().  These functions used to be image_calc_fix() and
#               image_record() in image.py, respectively.  They were moved here
#               and renamed so that psf.py can use them as well.

# ------------------------------------------------------------------------------

# Imports
# -------

import numpy

try:
    from casac import casac
except:
    pass
import casa

import logs

# ------------------------------------------------------------------------------
# Functions and ICDs
# ------------------------------------------------------------------------------

# cube_fix

# Description:
# ------------
# This function fixes the dimensions of the numpy arrays in the cube statistics
# dictionary when the number of Stokes parameters and/or the number of
# frequencies is one.  This function also adds the Stokes and frequency numpy
# arrays to the cube statistics dictionary.

# Inputs:
# -------
# in_cube    - This python string contains the name of the cube.
# cube_stats - This python dictionary contains the cube statistics.
# logger     - This python variable contains the logging object information
#              (<python> and <casa> are actual instances of the logger classes).

# Outputs:
# --------
# The python dictionary containing the fixed cube statistics, returned via the
# function value.

# Modification history:
# ---------------------
# 2011 Oct 13 - Nick Elias, NRAO
#               Initial version.  It used to be image_calc_fix() in image.py,
#               but it has been renamed and moved here so that psf.py can use it
#               as well.

# ------------------------------------------------------------------------------

def cube_fix( in_cube, cube_stats, logger ):

	# Get the Stokes parameters and frequencies from the record keywords and
	# get the statistics keys

	try:
		Stokes, frequency = cube_record( in_cube, logger )
	except Exception, err:
		origin = 'cube.cube_fix'
		if logger != '': logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )

	nS = len( Stokes )
	rS = range( nS )

	nf = len( frequency )
	rf = range( nf )

	keys = cube_stats.keys()


	# If the number of Stokes parameters and/or the number of frequencies
	# is one, convert the one-dimensional arrays in the cube statistics
	# dictionary to two-dimensional arrays

	if nS == 1 and nf <> 1:

		z = numpy.zeros( [nS,nf], dtype='float' )

		for k in keys:
			temp = cube_stats[k]
			cube_stats[k] = numpy.copy( z )
			for f in rf: cube_stats[k][0][f] = numpy.copy(temp[f])

	elif nS <> 1 and nf == 1:

		z = numpy.zeros( [nS,nf], dtype='float' )

		for k in numpy.sort(keys):
			temp = cube_stats[k]
			cube_stats[k] = numpy.copy( z )
			for s in rS: cube_stats[k][s][0] = numpy.copy(temp[s])

	elif nS == 1 and nf == 1:

		z = numpy.zeros( [nS,nf], dtype='float' )

		for k in keys:
			temp = cube_stats[k]
			cube_stats[k] = numpy.copy( z )
			cube_stats[k][0][0] = numpy.copy( temp )


	# Add the Stokes parameters and frequencies to the cube statistics
	# dictionary

	cube_stats['Stokes'] = numpy.zeros( [nS,nf], dtype='|S1' )
	cube_stats['frequency'] = numpy.zeros( [nS,nf], dtype='float' )

	for s in rS:
		for f in rf:

			temp = Stokes[s]
			cube_stats['Stokes'][s][f] = numpy.copy( temp )

			temp = frequency[f]
			cube_stats['frequency'][s][f] = numpy.copy( temp )


	# Return the fixed dictionary containing the cube statistics and the
	# Stokes and frequency numpy arrays

	return cube_stats

# ------------------------------------------------------------------------------

# cube_record

# Description:
# ------------
# This function gets the Stokes parameters and frequencies from a header record
# of a CASA cube.

# NB: The structure of this information stored in different ways, depending on
# whether the cube has contiguous frequencies (including continuum) or non-
# contiguous frequencies.

# Inputs:
# -------
# in_cube - This python string contains the name of the cube.
# logger  - This python variable contains the logging object information
#           (<python> and <casa> are actual instances of the logger classes).

# Outputs:
# --------
# The Stokes parameter list and frequency list which are elements of a tuple,
# returned via the function value.

# Modification history:
# ---------------------
# 2011 Oct 13 - Nick Elias, NRAO
#               Initial version.  It used to be image_record() in image.py, but
#               it has been renamed and moved here so that psf.py can use it as
#               well.

# ------------------------------------------------------------------------------

def cube_record( in_cube, logger ):

	# Create the local instance of the image analysis tool and open it

        try:
	    iaLoc = casac.image()
        except:
            iaLoc = casa.__imagehome__.create()

	iaLoc.open( in_cube )


	# Create the local coordinate system tool from the image analysis tool

	csLoc = iaLoc.coordsys()


	# Get the axis types

	cs_types = csLoc.axiscoordinatetypes()


	# Get the frequencies.  They are stored in two different ways: 1)
	# 'Spectral' is used for cubes with contiguous frequencies (including
	# continuum); and 2) 'Tabular' is used for cubes with non-contiguous
	# frequencies.

	if cs_types.count( 'Spectral' ) > 0:

		index = cs_types.index( 'Spectral' )
		f0 = csLoc.referencevalue()['numeric'][index]
		df = csLoc.increment()['numeric'][index]
		rf = range( iaLoc.shape()[index] ) # IA tool, go figure

		frequency = list()
		for f in rf: frequency.append( f0 + f*df )

	elif cs_types.count( 'Tabular' ) > 0:

		frequency = csLoc.torecord()['tabular2']['worldvalues'].tolist()

	else:

		msg = 'No valid frequencies ...'
		origin = 'cube.cube_record'
		if logger != '': logger.error( msg, origin=origin )
		raise Exception( msg )


	# Get the Stokes parameters

	if cs_types.count( 'Stokes' ) > 0:
		Stokes = csLoc.stokes()
		if isinstance( Stokes, str ): Stokes = [Stokes]
	else:
		msg = 'No Stokes parameters ...'
		origin = 'cube.cube_record'
		if logger != '': logger.error( msg, origin=origin )
		raise Exception( msg )


	# Close and delete the local coordinate system tool and the image
	# analysis tool

	csLoc.done()
	del csLoc

	iaLoc.done()
	del iaLoc


	# Return the Stokes parameter list and frequency list as a tuple

	return Stokes, frequency
