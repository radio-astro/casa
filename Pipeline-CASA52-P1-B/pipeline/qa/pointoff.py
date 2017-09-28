
# ------------------------------------------------------------------------------
# Subsystem module ICD
# ------------------------------------------------------------------------------

# pointoff.py

# Description:
# ------------
# This module runs the pointing offset subsystem of the QA system.

# Subsystem user functions:
# -------------------------
# pointoff - This function determines the pointing offset between two CASA
#            images, most probably an interferometer image and a single-dish
#            image.

# Modification history:
# ---------------------
# 2012 Jul 24 - Nick Elias, NRAO
#               Function pointoff() added.
# 2012 Aug 01 - Nick Elias, NRAO
#               Tool calls modified to new format.

# ------------------------------------------------------------------------------

# Imports
# -------

import os

import numpy
import scipy.fftpack
import scipy.signal  # Hopefully, this will be removed (see TBD in pointoff())

try:
    from casac import casac
except:
    import casa

import utility.logs as logs

# ------------------------------------------------------------------------------
# Subsystem user functions and ICDs
# ------------------------------------------------------------------------------

# pointoff

# Description:
# ------------
# This function determines the pointing offset between two CASA images, most 
# probably an interferometer image and a single-dish image.

# NB: The offsets are in the sense of single-dish minus interferometer.

# NB: The input images must have only a single frequency and Stokes I.  If not,
# the results will be unpredictable.  The number of frequencies and the Stokes
# parameters are not checked.

# NB: This function will fail if the interferometer and single-dish images show
# different structure, i.e., if the field contains extended structure and the
# resolutions are very different.

# NB: I repeat creating/deleting iaLoc and tbLoc several times.  I could do it 
# once, but this way the code is easier to read.  There are no related
# performance issues.

# TBD: I am presently using the scipy.signal.correlate2d() function to calculate
# the correlation.  It is a little slow because it does a brute-force
# calculation instead of a Fourier-based calculation.  I tried the latter (the
# corresponding code is commented out), but I keep getting the wrong offsets.
# I will investigate this issue in the future.

# TBD: The offsets and their errors python dictionaries are returned via the
# function value and they are also saved directly as keywords in the correlation
# image.  They can be retrieved using the CASA table tool, but there appears to
# be an issue with the table browser.  I will investigate.

# TBD: With experience, we may decide to change the shape and/or size of the
# Gaussian fit region.

# Algorithm:
# ----------
# * Get the single-dish coordinate system.
# * Regrid the interferometer image to the single-dish image.
# * Calculate the cross correlation of the single-dish image and regridded
#   interferometer image by brute force (eventually, use FFTs, which are
#   faster).  Normalize the cross correlation.
# * Find the peak in the correlation image and perform the Gaussian fit.
# * Save the fit offsets and errors to the return variable and the header
#   keywords.

# Inputs:
# -------
# imageInt  - This python string contains the name of the first image, most
#             likely an interferometer CASA image.
# imageSD   - This python string contains the name of the second image, most
#             likely a single-dish CASA image.
# out_dir   - This python string contains the output directory.
# imageCorr - This python string contains the name of the output correlation
#             image.
# size      - This python integer contains the half-size of the square Gaussian
#             fit region in the output correlation image (default = 10).
# logobj    - This python variable contains the logging object information
#             ('PYTHON' creates a local python Logger class, 'CASA' creates a
#             local python logger, <python> and <casa> are actual instances of
#             the logger classes; default = 'PYTHON').

# Outputs:
# --------
# The regridded interferometer image.
# The correlation image.
# The python dictionary (actually, a CASA quanta) containing the offsets and
# their errors, returned via a function value.  If the function does not finish
# successfully, an exception is raised.

# The keys:
# 'dRA'     - The python dictionary containing the right ascension offset.
# 'dRAErr'  - The python dictionary containing the right ascension offset error.
# 'dDEC'    - The python dictionary containing the declination offset.
# 'dDECErr' - The python dictionary containing the declinarion offset error.

# Each of these dictionaries contains the following elements:
# 'value' - This python double contains the value.
# 'unit'  - This python string contains the value units.

# Modification history:
# ---------------------
# 2012 Jul 24 - Nick Elias, NRAO
#               Initial production version created.

# ------------------------------------------------------------------------------

def pointoff( imageInt, imageSD, out_dir, imageCorr, size=10, logobj='PYTHON' ):

	# Initialize the logger

	root = 'pointoff.pointoff'
	level = logs.INFO

	log_local, logger = logs.init( out_dir, root, level, logobj )


	# Print the first log message

	msg = 'Pointing offsets statistics started for ' + imageInt + ' and '
	msg += imageSD + ' ...'
	origin = root
	logger.info( msg, origin=origin )


	# Check to see if the output file already exists

	out_imageCorr = out_dir + '/' + imageCorr

	if os.path.exists( out_imageCorr ):
		msg = 'Output image ' + out_imageCorr + ' already exists ...\n'
		origin = root
		logger.error( msg, origin=origin )
		raise IOError( msg )


	# Get the coordinate system from the single-dish image

        try:
	    iaLoc = casac.image()
        except:
            iaLoc = casa.__imagehome__.create()

	iaLoc.open( imageSD )

	csysSD = iaLoc.coordsys().torecord()

	iaLoc.close()
	del iaLoc

	msg = 'Single-dish image coordinates obtained ...'
	origin = root
	logger.info( msg, origin=origin )


	# Regrid the interferometric image to the single-dish image coordinates.
	# A regridded image is saved to disk.

        try:
	    iaLoc = casac.image()
        except:
            iaLoc = casa.__imagehome__.create()
	iaLoc.open( imageInt )

	imageIntRegrid = out_dir + '/' + imageInt + '.regrid.tmp'
	iaLoc.regrid( outfile=imageIntRegrid, csys=csysSD )

	iaLoc.close()
	del iaLoc

	msg = 'Interferometer image regridder to single-dish image '
	msg += 'coordinates ...'
	origin = root
	logger.info( msg, origin=origin )


	# Correlate the single-dish and regridded interferometric image

        try:
	    iaLoc = casac.image()
        except:
            iaLoc = casa.__imagehome__.create()

	iaLoc.open( imageSD )
	sdMap = iaLoc.getchunk()[:,:,0,0]
	iaLoc.close()

	iaLoc.open( imageIntRegrid )
	intMap = iaLoc.getchunk()[:,:,0,0]
	iaLoc.subimage( imageCorr )
	iaLoc.close()

#	sdFFT = scipy.fftpack.fft2( sdMap )
#	intFFT = scipy.fftpack.fft2( intMap )
#	corrMap = scipy.fftpack.ifft2( numpy.conj(intFFT)*sdFFT )
	corrMap = scipy.signal.correlate2d( sdMap, intMap, mode='same' )
	corrMap = corrMap.reshape( (corrMap.shape[0],corrMap.shape[1],1,1) )

	iaLoc.open( imageCorr )
	iaLoc.putchunk( pixels=corrMap )
	iaLoc.close()

	del iaLoc

	msg = 'Correlation image calculated and saved to disk ...'
	origin = root
	logger.info( msg, origin=origin )


	# Normalize the correlated image and make its coordinate system relative

        try:
	    tbLoc = casac.table()
        except:
            tbLoc = casa.__tablehome__.create()

	tbLoc.open( out_imageCorr, nomodify=False )

	map = tbLoc.getcol( 'map' )
	tbLoc.putcol( 'map', map/map[:,:,0,0,0].max() )

	coords = tbLoc.getkeyword( 'coords' )
	coords['direction0']['crval'] = numpy.array( [0.0,0.0] )
	tbLoc.putkeyword( 'coords', coords )

	tbLoc.close()
	del tbLoc

	msg = 'Correlation image normalized and coordinates made relative ...'
	origin = root
	logger.info( msg, origin=origin )


	# Create the fitting box string and perform a Gaussian fit to determine
	# the pointing offset

	maxLoc = numpy.where( map[:,:,0,0,0] == map[:,:,0,0,0].max() )

	box = str( maxLoc[0][0] - size )
	box += ',' + str( maxLoc[1][0] - size )
	box += ',' + str( maxLoc[0][0] + size )
	box += ',' + str( maxLoc[1][0] + size )

        try:
	    iaLoc = casac.image()
        except:
            iaLoc = casa.__imagehome__.create()
	iaLoc.open( imageCorr )

	result = iaLoc.fitcomponents( box=box, chans='0', stokes='I',
	    residual=imageCorr+'.residual', model=imageCorr+'.model' )

	iaLoc.close()
	del iaLoc

	msg = 'Peak of the image found and Gaussian fit performed ...'
	origin = root
	logger.info( msg, origin=origin )


	# Save the results as keywords in the correlated image

        try:
	    tbLoc = casac.table()
        except:
            tbLoc = casa.__tablehome__.create()
	tbLoc.open( out_imageCorr, nomodify=False )

	d = result['results']['component0']['shape']['direction']

        try:
	    qaLoc = casac.quanta()
        except:
            qaLoc = casa.__quantahome__.create()

	dRA = qaLoc.convert( qaLoc.quantity(d['m0'],'rad'), 'arcsec' )
	tbLoc.putkeyword( 'RA_SHIFT', dRA )

	dRAErr = qaLoc.quantity( d['error']['latitude'], 'arcsec' )
	tbLoc.putkeyword( 'RA_SHIFT_ERR', dRAErr )

	dDEC = qaLoc.convert( qaLoc.quantity(d['m1'],'rad'), 'arcsec' )
	tbLoc.putkeyword( 'DEC_SHIFT', dDEC )

	dDECErr = qaLoc.quantity( d['error']['longitude'], 'arcsec' )
	tbLoc.putkeyword( 'DEC_SHIFT_ERR', dDECErr )

	del qaLoc

	tbLoc.close()
	del tbLoc

	msg = 'Keywords saved ...'
	origin = root
	logger.info( msg, origin=origin )


	# Create the results dictionary

	dPos = dict()

	dPos['dRA'] = dRA
	dPos['dRAErr'] = dRAErr
	dPos['dDEC'] = dDEC
	dPos['dDECErr'] = dDECErr

	msg = 'Dictionary populated ...'
	origin = root
	logger.info( msg, origin=origin )


	# Print the last log message

	msg = 'Pointing offset statistics finished for ' + imageInt + ' and '
	msg += imageSD + ' ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Return the results dictionary

	return dPos
