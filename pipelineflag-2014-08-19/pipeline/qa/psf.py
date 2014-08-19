
# ------------------------------------------------------------------------------
# Subsystem module ICD
# ------------------------------------------------------------------------------

# psf.py

# Description:
# ------------
# This module runs the psf information (histograms and statistics) subsystem of
# the QA system.

# User functions (subsystem and component level):
# -----------------------------------------------
# psf       - This function runs the psf information subsystem of the QA
#             system.
#
# psf_calc  - This function calculates the psf information.
# psf_write - This function writes the psf information to a CASA table.

# Non-user functions (unit level):
# --------------------------------
# psf_polar_hists      - This function calculates the polar psf histograms
#                        versus radius.
# psf_polar_stats      - This function calculates the polar psf statistics
#                        versus radius.
# psf_polar_interp     - This function calculates and returns the psf
#                        interpolated onto a polar grid.
#
# psf_all_fits         - This function fits 2D Gaussian to the psf cores of the
#                        psfs.
# psf_hwhm             - This function estimates the crude HWHMs of the psfs in
#                        pixel units.
# psf_all_hists        - This function calculates the histograms for the psfs.
# psf_all_stats        - This function calculates the statistics for the psfs.
# psf_mask             - This function creates a temporary masked psf less than
#                        the input threshold.
# psf_summary          - This function returns the summary of the summary
#                        returned by the image analysis and coordinate system
#                        tools.
#
# psf_desc             - This function returns the place holder data description
#                        dictionary.
# psf_desc_polar_hists - This function creates the polar histograms data
#                        description dictionary.
# psf_desc_polar_stats - This function creates the polar statistics data
#                        description dictionary.
# psf_desc_all_fits    - This function creates the all fits data description
#                        dictionary.
# psf_desc_all_hists   - This function creates the all histograms data
#                        description dictionary.
# psf_desc_all_stats   - This function creates the all statistics data
#                        description dictionary.

# Modification history:
# ---------------------
# 2011 Sep 19 - Nick Elias, NRAO
#               Initial version created with functions psf() and psf_calc(); and
#               stub function psf_write().
# 2011 Sep 23 - Nick Elias, NRAO
#               Functions psf_all_hists(), psf_polar_interp(), and
#               psf_polar_stats() added.
# 2011 Sep 29 - Nick Elias, NRAO
#               Function psf_write() populated.  Function psf_desc() added.
# 2011 Oct 12 - Nick Elias, NRAO
#               Function psf_polar_hists() added.
# 2011 Oct 13 - Nick Elias, NRAO
#               Functions psf_all_stats() and psf_mask() added.
# 2011 Oct 14 - Nick Elias, NRAO
#               Function psf_all_fits() added.
# 2011 Oct 17 - Nick Elias, NRAO
#               Function psf_hwhm() added.
# 2011 Oct 18 - Nick Elias, NRAO
#               The psf_write() function was refactored to make it more general
#               and easier to use.  The functions psf_desc_polar_hists(),
#               psf_desc_polar_stats(), psf_desc_all_fits(),
#               psf_desc_all_hists(), and psf_desc_all_stats(), and
#               psf_summary() were added.  The code within function psf_desc()
#               was replaced with a place holder.
# 2012 Aug 01 - Nick Elias, NRAO
#               Tool calls modified to new format.

# ------------------------------------------------------------------------------

# Imports
# -------

import math
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

# psf

# Description:
# ------------
# This function runs the psf information subsystem of the QA system.

# Inputs:
# -------
# in_psf       - This python string contains the name of the input psf.
# out_dir      - This python string contains the name of the output directory.
# fit_factor   - This python float contains the fudge factor for the size of the
#                fitting region (default=2.0, which is twice the estimated
#                HWHM).
# stats_thresh - This python float contains the maximum statistics threshold.
#                Its value must be between 0.0 and 1.0 (default = 0.3).
# logobj       - This python variable contains the logging object information
#                ('PYTHON' creates a local python Logger class, 'CASA' creates a
#                local python logger, <python> and <casa> are actual instances
#                of the logger classes; default = 'PYTHON').

# Outputs:
# --------
# If the function finishes successfully, True is returned via the function
# value.  If the function does not finish successfully, it throws an exception.

# Modification history:
# ---------------------
# 2011 Sep 19 - Nick Elias, NRAO
#               Initial version.
# 2011 Oct 13 - Nick Elias, NRAO
#               The stats_thresh input parameter has been added.
# 2011 Oct 17 - Nick Elias, NRAO
#               The fit_factor input parameter has been added.

# ------------------------------------------------------------------------------

def psf( in_psf, out_dir, fit_factor=2.0, stats_thresh=0.3, logobj='PYTHON' ):

	# Initialize the logger

	root = 'psf.psf'
	level = logs.INFO

	log_local, logger = logs.init( out_dir, root, level, logobj )


	# Print the first log message

	msg = 'PSF information started for ' + in_psf + ' ...'
	origin = root
	logger.info( msg, origin=origin )


	# Check to see if the output file already exists

	out_table_root = os.path.basename( os.path.splitext( in_psf )[0] )
	out_table = out_dir + '/' + out_table_root + '.psf.stats'

	if os.path.exists( out_table ):
		msg = 'Output table ' + out_table + ' already exists ...\n'
		origin = root
		logger.error( msg, origin=origin )
		raise IOError( msg )


	# Calculate the psf information

	try:
		psf_info = psf_calc( in_psf, stats_thresh=stats_thresh,
                                     logger=logger )
	except Exception, err:
		origin = root
		logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )

	msg = 'PSF information of ' + in_psf + ' calculated ...'
	origin = root
	logger.info( msg, origin=origin )


	# Write the psf information to the table

	status = psf_write( psf_info, out_table )

	msg = 'PSF information written in ' + out_table + ' ...'
	origin = root
	logger.info( msg, origin=origin )


	# Print the last log message

	msg = 'PSF information finished for ' + in_psf + ' ...\n'
	origin = root
	logger.info( msg, origin=origin )


	# Delete the logger, if it was created locally

	if log_local: del logger


	# Return True

	return True

# ------------------------------------------------------------------------------
# Component user functions and ICDs
# ------------------------------------------------------------------------------

# psf_calc

# Description:
# ------------
# This function calculates the psf information.

# Inputs:
# -------
# in_psf       - This python string contains the name of the psf.
# fit_factor   - This python float contains the fudge factor for the size of the
#                fitting region (default=2.0, which is twice the estimated
#                HWHM).
# stats_thresh - This python float contains the maximum statistics threshold.
#                Its value must be between 0.0 and 1.0 (default = 0.3).
# logger       - This python variable contains the logging object information
#                (<python> and <casa> are actual instances of the logger
#                classes).  The default is '', which means that the log
#                information is either sent to stdout or raised.

# Outputs:
# --------
# The dictionary containing the psf information, returned via the function
# value.  The keys:
# 'polar_hists' - The python dictionary containing the polar psf histograms (see
#                 the psf_polar_hists() function for its structure).
# 'polar_stats' - The python dictionary containing the polar psf statistics (see
#                 the psf_polar_stats() function for its structure).
# 'all_fits'    - The python dictionary containing the psf 2D Gauusian fits (see
#                 the psf_all_fits() function for its structure).
# 'all_hists'   - The python dictionary containing the psf histograms (see the
#                 psf_all_hists() function for its structure).
# 'all_stats'   - The python dictionary containing the psf statistics (see the
#                 psf_all_stats() function for its structure).

# Modification history:
# ---------------------
# 2011 Sep 19 - Nick Elias, NRAO
#               Initial version.
# 2011 Sep 30 - Nick Elias, NRAO
#               The logger input parameter can also take a '' value.
# 2011 Oct 12 - Nick Elias, NRAO
#               The polar psf histograms versus radius are separated from the
#               polar psf information versus radius.
# 2011 Oct 13 - Nick Elias, NRAO
#               The statistics are calculated using psf_all_stats().  The
#               stats_thresh input parameter has been added.
# 2011 Oct 17 - Nick Elias, NRAO
#               The 2D Gaussian fits are calculated using psf_all_fits().  The
#               fit_factor input parameter has been added.

# ------------------------------------------------------------------------------

def psf_calc( in_psf, fit_factor=2.0, stats_thresh=0.3, logger='' ):

	# Calculate the psf information

	psf_info = dict()

	try:

		ppi = psf_polar_interp( in_psf )
		psf_info['polar_hists'] = psf_polar_hists( in_psf, ppi )
		psf_info['polar_stats'] = psf_polar_stats( in_psf, ppi )

		psf_info['all_fits'] = psf_all_fits( in_psf,
                    psf_info['polar_stats'], fit_factor=fit_factor )
		psf_info['all_hists'] = psf_all_hists( in_psf, logger )
		psf_info['all_stats'] = psf_all_stats( in_psf, logger,
                    stats_thresh=stats_thresh )

	except Exception, err:

		origin = 'psf.psf_calc'
		if logger != '': logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )


	# Return the dictionary containing the psf information

	return psf_info

# ------------------------------------------------------------------------------

# psf_write

# Description:
# ------------
# This function writes the psf information to a CASA table.

# Inputs:
# -------
# psf_info  - The python dictionary that contains the psf information.
# out_table - The python string that contains the output table name.

# Outputs:
# --------
# The psf information table is created in the output directory. If the function
# finishes successfully, True is returned via the function value.  If the
# function does not finish successfully, it throws an exception.

# Modification history:
# ---------------------
# 2011 Sep 19 - Nick Elias, NRAO
#               Initial stub version.
# 2011 Sep 29 - Nick Elias, NRAO
#               Added code to write to the CASA table.
# 2011 Oct 12 - Nick Elias, NRAO
#               Added radial maximum and minimum statistics.  The degenerate
#               dimensions required for writing hypercolumns are added here
#               instead of when the python arrays are created.  Added the radii
#               for the radial histograms and statistics.
# 2011 Oct 18 - Nick Elias, NRAO
#               Rewrote the function in a more general fashion to make it
#               considerably shorter.  Keywords are now written as well.

# ------------------------------------------------------------------------------

def psf_write( psf_info, out_table ):

	# Create the local instance of the table tool, create the output main
	# table with a place holder data description, and close the table

        try:
	    tbLoc = casac.table()
        except:
            tbLoc = casa.__tablehome__.create()

	tbLoc.create( out_table, psf_desc() )

	tbLoc.addrows()
	tbLoc.putcol( 'place_holder', True, 0, 1, 1 )

	tbLoc.close()


	# Get the list of subtables and create them

	subtables = psf_info.keys()

	for st in subtables:
		out_subtable = out_table + '/' + st.upper()
		tbLoc.create( out_subtable, psf_info[st]['desc'] )
		tbLoc.close()


	# Put the keywords into the subtables

	for st in subtables:

		out_subtable = out_table + '/' + st.upper()
		tbLoc.open( out_subtable, nomodify=False )

		keywords = psf_info[st]['keyword']
		keys = keywords.keys()
		for k in keys: tbLoc.putkeyword( k, keywords[k] )

		tbLoc.close()


	# Put the data into the subtables

	for st in subtables:
		
		out_subtable = out_table + '/' + st.upper()
		tbLoc.open( out_subtable, nomodify=False )

		tbLoc.addrows()

		hypercolumns = psf_info[st].keys()
		hypercolumns.remove('keyword')
		hypercolumns.remove('desc')

		for hc in hypercolumns:
			temp = psf_info[st][hc]
			shape = list( temp.shape ); shape.append( 1 )
			temp = numpy.reshape( temp, shape )
			tbLoc.putcol( hc, temp, 0, 1, 1 )

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

# psf_polar_hists

# Description:
# ------------
# This function calculates the polar psf histogram versus radius.

# Inputs:
# -------
# in_psf         - This python string contains the name of the psf.
# psf_polar_dict - This python dictionary contains the radii, azimuths, and
#                  interpolated polar psfs. It comes from the psf_polar_interp()
#                  function.

# Outputs:
# --------
# The python dictionary containing the polar histogram information, returned via
# the function value.

# The keys:
# 'values'  - The numpy float array containing the histogram bin values.  The
#             indices represent Stokes parameter, frequency, radial pixel, and
#             bin.
# 'counts'  - The numpy float array containing the histogram numbers.  The
#             indices represent Stokes parameter, frequency, radial pixel, and
#             bin.
# 'desc'    - The python dictionary containing the data description.
# 'keyword' - The python dictionary containing the keywords.

# The 'keyword' dictionary mentioned above points to the following keys:
# 'Stokes'    - The numpy string array containing the Stokes parameters.
# 'frequency' - The numpy float array containing the frequencies.
# 'freq_unit' - The python string containing the frequency unit.
# 'radius'    - The numpy float array containing the radii (relative pixel
#               units).

# Modification history:
# ---------------------
# 2011 Oct 12 - Nick Elias, NRAO
#               Initial version created from code pilfered from
#               psf_polar_stats().
# 2011 Oct 18 - Nick Elias, NRAO
#               The function was modified to include the data description and
#               keywords.

# ------------------------------------------------------------------------------

def psf_polar_hists( in_psf, psf_polar_dict ):

	# Get the summary of the summary returned by the image analysis and
	# coordinate system tools

	summary = psf_summary( in_psf )

	Stokes = summary['Stokes']

	frequency = summary['frequency']
	freqUnit = summary['freq_unit']


	# Disassemble the input dictionary

	rho = psf_polar_dict['rho']
	phi = psf_polar_dict['phi']

	psf_polar = psf_polar_dict['psf_polar']


	# Get the polar psf shape parameters

	nS = psf_polar.shape[0]
	rS = range( nS )

	nF = psf_polar.shape[1]
	rF = range( nF )

	nR = psf_polar.shape[2]
	rR = range( nR )

	nP = psf_polar.shape[3]
	rP = range( nP )


	# Form the polar psf histogram versus radius

	nBin = int( nP / 20.0 )

	psf_counts = numpy.zeros( [nS,nF,nR,nBin], dtype='float' )
	psf_values = numpy.zeros( [nS,nF,nR,nBin], dtype='float' )

	for s in rS:
		for f in rF:
			for r in rR:

				hist_dict = numpy.histogram( psf_polar[s,f,r,:],
				    bins=nBin )

				psf_counts[s,f,r,:] = hist_dict[0]

				left = hist_dict[1][0:nBin]
				right  = hist_dict[1][1:nBin+1]
				psf_values[s,f,r,:] = 0.5 * ( left + right )


	# Set and return the output dictionary

	polar_hists = dict()

	polar_hists['keyword'] = dict()
	polar_hists['keyword']['Stokes'] = Stokes
	polar_hists['keyword']['frequency'] = frequency
	polar_hists['keyword']['freq_unit'] = freqUnit
	polar_hists['keyword']['radius'] = rho

	polar_hists['desc'] = psf_desc_polar_hists()

	polar_hists['counts'] = psf_counts
	polar_hists['values'] = psf_values

	return polar_hists

# ------------------------------------------------------------------------------

# psf_polar_stats

# Description:
# ------------
# This function calculates the polar psf statistics versus radius.

# Inputs:
# -------
# in_psf         - This python string contains the name of the psf.
# psf_polar_dict - This python dictionary contains the radii, azimuths, and
#                  interpolated polar psfs. It comes from the psf_polar_interp()
#                  function.

# Outputs:
# --------
# The dictionary containing the polar statistics information, returned via the
# function value.

# The keys:
# 'mean'    - The numpy float array containing the means.  The indices represent
#             the Stokes parameter, channel, and radial pixel.
# 'stdev'   - The numpy float array containing the standard deviations.  The
#             indices represent the Stokes parameter, channel, and radial pixel.
# 'max'     - The numpy float array containing the maxima.  The indices
#             represent the Stokes parameter, channel, and radial pixel.
# 'min'     - The numpy float array containing the minima.  The indices
#             represent the Stokes parameter, channel, and radial pixel.
# 'fft'     - The numpy complex array containing the ffts.  The frequencies are
#             arranged in "standard FFT order."
# 'desc'    - The python dictionary containing the data description.
# 'keyword' - The python dictionary containing the keywords.

# The 'keyword' dictionary mentioned above points to the following keys:
# 'Stokes'    - The numpy string array containing the Stokes parameters.
# 'frequency' - The numpy float array containing the frequencies.
# 'freq_unit' - The python string containing the frequency unit.
# 'radius'    - The numpy float array containing the radii (relative pixel
#               units).
# 'fftfreq'  - The numpy float array containing the fft frequencies in units of
#              cycles (2*pi*radian)^{-1}.  The frequencies are arranged in
#              "standard FFT order."

# Modification history:
# ---------------------
# 2011 Sep 23 - Nick Elias, NRAO
#               Initial version.
# 2011 Oct 12 - Nick Elias, NRAO
#               Added radial maximum and minimum statistics.  The degenerate
#               dimensions required for writing hypercolumns are no longer added
#               here.  The frequency units of the fft are now cycles
#               (2*pi*radian)^{-1}.  The histograms versus radius have been
#               moved to psf_polar_hists().
# 2011 Oct 18 - Nick Elias, NRAO
#               The function was modified to include the data description and
#               keywords.

# ------------------------------------------------------------------------------

def psf_polar_stats( in_psf, psf_polar_dict ):

	# Get the summary of the summary returned by the image analysis and
	# coordinate system tools

	summary = psf_summary( in_psf )

	Stokes = summary['Stokes']

	frequency = summary['frequency']
	freqUnit = summary['freq_unit']


	# Disassemble the input dictionary

	rho = psf_polar_dict['rho']
	phi = psf_polar_dict['phi']

	psf_polar = psf_polar_dict['psf_polar']


	# Get the polar psf shape parameters

	nS = psf_polar.shape[0]
	rS = range( nS )

	nF = psf_polar.shape[1]
	rF = range( nF )

	nR = psf_polar.shape[2]
	rR = range( nR )

	nP = psf_polar.shape[3]
	rP = range( nP )


	# Calculate the polar psf mean, standard deviation, maximum, and
	# minimum versus radius

	psf_mean = numpy.mean( psf_polar, axis=3 )
	psf_stdev = numpy.std( psf_polar, axis=3, ddof=1 )

	psf_max = numpy.max( psf_polar, axis=3 )
	psf_min = numpy.min( psf_polar, axis=3 )


	# Calculate the polar psf fft versus radius

	psf_fft = numpy.zeros( [nS,nF,nR,nP], dtype='complex' )

	for s in rS:
		for f in rF:
			for r in rR:
				fft_temp = numpy.fft.fft( psf_polar[s,f,r,:] )
				psf_fft[s,f,r,:] = fft_temp

	dPhi = phi[1] - phi[0]
	psf_fftfreq = numpy.fft.fftfreq( nP, d=dPhi ) / ( 2.0 * math.pi )


	# Set and return the output dictionary

	polar_stats = dict()

	polar_stats['keyword'] = dict()
	polar_stats['keyword']['Stokes'] = Stokes
	polar_stats['keyword']['frequency'] = frequency
	polar_stats['keyword']['freq_unit'] = freqUnit
	polar_stats['keyword']['radius'] = rho
	polar_stats['keyword']['fftfreq'] = psf_fftfreq

	polar_stats['desc'] = psf_desc_polar_stats()

	polar_stats['mean'] = psf_mean
	polar_stats['stdev'] = psf_stdev

	polar_stats['max'] = psf_max
	polar_stats['min'] = psf_min

	polar_stats['fft'] = psf_fft

	return polar_stats

# ------------------------------------------------------------------------------

# psf_polar_interp

# Description:
# ------------
# This function calculates and returns the psf interpolated onto a polar grid.

# Notes:
# ------
# 1) Each radial coordinate has the same number of azimuthal coordinates.  This
#    scheme may not be optimum and can be changed easily, if desired.
# 2) Psfs with different numbers of pixels in x and y and/or with pixels with
#    different x and y sizes are handled in this manner:
#    a) Determine the range in x and y and choose the MINIMUM value.
#    b) Determine the number of pixels in x and y and choose the MAXIMUM value.
#    c) With these choices, you get a slightly overresolved set of rho and phi
#       coordinates such that the psf will always be defined.

# Inputs:
# -------
# in_psf - This python string contains the name of the psf.

# Outputs:
# --------
# The python dictionary containing the interpolated polar psf, returned via the
# function value.  The keys:
# 'rho'       - This numpy float array contains the radii (relative pixel
#               units).
# 'phi'       - This numpy float array contains the azimuths (radians).
# 'psf_polar' - This numpy float array contains the psf interpolated on to the
#               polar grid.  The four indices represent Stokes parameter,
#               frequency, radius, and azimuth.

# Modification history:
# ---------------------
# 2011 Sep 23 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def psf_polar_interp( in_psf ):

	# Create and open the local instance of the image analysis tool

        try:
	    iaLoc = casac.image()
        except:
            iaLoc = casa.__imagehome__.create()

	iaLoc.open( in_psf )


	# Get the psf shape parameters

	psfShape = iaLoc.shape()

	nX = psfShape[0]
	nY = psfShape[1]
	nS = psfShape[2]
	nF = psfShape[3]


	# Form the radius array (relative pixel coordinates) and the azimuth
	# array (radians)

	nR = int( max( [nX-1,nY-1] ) / 2.0 )
	rho = numpy.linspace( 0, nR-1, nR )

	nP = int( round( 2.0 * math.pi * nR ) )
        dPhi = 2.0 * math.pi / float( nP )
	phi = dPhi * numpy.linspace( 0, nP-1, nP )


	# Get the reference pixel from the coordinate system tool

	csysLoc = iaLoc.coordsys()

	refPixel = csysLoc.referencepixel()['numeric'][0:2]

	csysLoc.done()
	del csysLoc


	# Form the vertex python lists in absolute pixel coordinates

	xVertex = numpy.zeros( [nP,2], dtype='float' )
	xVertex[:,1] = nR * numpy.cos( phi )
	xVertex += refPixel[0]
	xVertex = xVertex.tolist()

	yVertex = numpy.zeros( [nP,2], dtype='float' )
	yVertex[:,1] = nR * numpy.sin( phi )
	yVertex += refPixel[1]
	yVertex = yVertex.tolist()


	# Get the interpolated one-dimensional psf slices as a function of
	# azimuth

	rP = range( nP )
	rS = range( nS )
	rF = range( nF )

	psf_polar = numpy.zeros( [nS,nF,nR,nP], dtype='float' )

	for s in rS:
		for f in rF:
			for p in rP:
				psf_polar[s,f,:,p] = iaLoc.getslice( xVertex[p],
				    yVertex[p], axes=[0,1], coord=[-1,-1,s,f],
				    npts=nR, method='linear' )['pixel']


	# Close and delete the local instance of the image analysis tool

	iaLoc.close()
	del iaLoc


	# Return the dictionary containing the interpolated one-dimensional
	# psf slice information

	psf_polar_dict = dict()

	psf_polar_dict['rho'] = rho
	psf_polar_dict['phi'] = phi

	psf_polar_dict['psf_polar'] = psf_polar

	return psf_polar_dict

# ------------------------------------------------------------------------------

# psf_all_fits

# Description:
# ------------
# This function fits 2D Gaussian to the psf cores of the psfs.

# Inputs:
# -------
# in_psf      - This python string contains the name of the psf.
# polar_stats - This python dictionary contains the polar statistics (the output
#               from the psf_polar_stats() function).
# fit_factor  - This python float contains the fudge factor for the size of the
#               fitting region (default=2.0, which is twice the estimated HWHM).

# Outputs:
# --------
# The python dictionary containing the fit information, returned via the
# function value.

# The keys:
# 'converged'           - The numpy boolean array containing convergence flags.
#                         The indices represent the Stokes parameter and
#                         frequency.
# 'major_axis'          - The numpy float array containing the major axes.  The
#                         indices represent the Stokes parameter and frequency.
# 'major_axis_err'      - The numpy float array containing the major axis
#                         errors.  The indices represent the Stokes parameter
#                         and frequency.
# 'minor_axis'          - The numpy float array containing the minor axes.  The
#                         indices represent the Stokes parameter and frequency.
# 'minor_axis_err'      - The numpy float array containing the minor axis
#                         errors.  The indices represent the Stokes parameter
#                         and frequency.
# 'position_angle'      - The numpy float array containing the position angles.
#                         The indices represent the Stokes parameter and
#                         frequency.
# 'position_angle_err'  - The numpy float array containing the position angle
#                         errors.  The indices represent the Stokes parameter
#                         and frequency.
# 'desc'                - The python dictionary containing the data description.
# 'keyword'             - The python dictionary containing the keywords.

# The 'keyword' dictionary mentioned above points to the following keys:
# 'Stokes'              - The numpy string array containing the Stokes
#                         parameters.
# 'frequency'           - The numpy float array containing the frequencies.
# 'freq_unit'           - The python string containing the frequency unit.
# 'major_axis_unit'     - The python string containing the major axis unit.
# 'minor_axis_unit'     - The python string containing the minor axis unit.
# 'position_angle_unit' - The python string containing the position angle unit.

# Modification history:
# ---------------------
# 2011 Oct 14 - Nick Elias, NRAO
#               Initial version.
# 2011 Oct 18 - Nick Elias, NRAO
#               The function was modified to include the data description and
#               keywords.
# 2011 Oct 28 - Nick Elias, NRAO
#               Brightness units issue fixed.

# ------------------------------------------------------------------------------

def psf_all_fits( in_psf, polar_stats, fit_factor=2.0 ):

	# Get the summary of the summary returned by the image analysis and
	# coordinate system tools

	summary = psf_summary( in_psf )

	Stokes = summary['Stokes']
	nS = Stokes.shape[0]
	rS = range( nS )

	frequency = summary['frequency']
	freqUnit = summary['freq_unit']
	nF = frequency.shape[0]
	rF = range( nF )

	refPixel = summary['refpixel']


	# Form the box limits for the 2D Gaussian fit

	hwhm = numpy.max( psf_hwhm( polar_stats ) )

	xMin = str( refPixel[0] - int(fit_factor*hwhm+0.5) )
	xMax = str( refPixel[0] + int(fit_factor*hwhm+0.5) )

	yMin = str( refPixel[1] - int(fit_factor*hwhm+0.5) )
	yMax = str( refPixel[1] + int(fit_factor*hwhm+0.5) )

	box = xMin + ',' + yMin + ',' + xMax + ',' + yMax


	# Initialize the keyword dictionary

	kd = dict()

	kd['Stokes'] = Stokes

	kd['frequency'] = frequency
	kd['freq_unit'] = freqUnit

	kd['major_axis_unit'] = ''
	kd['minor_axis_unit'] = ''
	kd['position_angle_unit'] = ''


	# Initialize the fit parameter dictionary

	all_fits = dict()

	all_fits['converged'] = numpy.zeros( [nS,nF], dtype='bool' )

	all_fits['major_axis'] = numpy.zeros( [nS,nF], dtype='float' )
	all_fits['major_axis_err'] = numpy.zeros( [nS,nF], dtype='float' )

	all_fits['minor_axis'] = numpy.zeros( [nS,nF], dtype='float' )
	all_fits['minor_axis_err'] = numpy.zeros( [nS,nF], dtype='float' )

	all_fits['position_angle'] = numpy.zeros( [nS,nF], dtype='float' )
	all_fits['position_angle_err'] = numpy.zeros( [nS,nF], dtype='float' )


	# Create and open the local instance of the image analysis tool.  The
	# brightness unit is changed to keep the fitcomponents() method from
	# spewing many warning messages (normally, psfs have no units).

        try:
	    iaLoc = casac.image()
        except:
            iaLoc = casa.__imagehome__.create()
	iaLoc.open( in_psf )

	buOld = iaLoc.brightnessunit()
	iaLoc.setbrightnessunit( 'Jy/pixel' )


	# Perform the 2D Gaussian fits.  A loop over Stokes parameters is a
	# required kludge because the fitcomponents() method of the image
	# analysis tool throws an exception when more than one Stokes parameter
	# is fed into it.

	for s in rS:

		fit_comp = iaLoc.fitcomponents( box=box, stokes=Stokes[s] )

		all_fits['converged'][s,:] = fit_comp['converged']

		# For brevity and convenience, eliminate the extra key
		fit_comp = fit_comp['results']

		for f in rF:

			c = 'component' + str(f)

			fit_temp = fit_comp[c]['shape']['majoraxis']
			all_fits['major_axis'][s,f] = fit_temp['value']

			fit_temp = fit_comp[c]['shape']['majoraxiserror']
			all_fits['major_axis_err'][s,f] = fit_temp['value']
			if kd['major_axis_unit'] == '':
				kd['major_axis_unit'] = fit_temp['unit']

			fit_temp = fit_comp[c]['shape']['minoraxis']
			all_fits['minor_axis'][s,f] = fit_temp['value']

			fit_temp = fit_comp[c]['shape']['minoraxiserror']
			all_fits['minor_axis_err'][s,f] = fit_temp['value']
			if kd['minor_axis_unit'] == '':
				kd['minor_axis_unit'] = fit_temp['unit']

			fit_temp = fit_comp[c]['shape']['positionangle']
			all_fits['position_angle'][s,f] = fit_temp['value']

			fit_temp = fit_comp[c]['shape']['positionangleerror']
			all_fits['position_angle_err'][s,f] = fit_temp['value']
			if kd['position_angle_unit'] == '':
				kd['position_angle_unit'] = fit_temp['unit']


	# Close and delete the local instance of the image analysis tool.  Make
	# sure to reset the brightness unit first.

	iaLoc.setbrightnessunit( buOld )

	iaLoc.close()
	del iaLoc


	# Add the keywords and data description to the output dictionary

	all_fits['keyword'] = kd
	all_fits['desc'] = psf_desc_all_fits()


	# Return the output dictionary

	return all_fits

# ------------------------------------------------------------------------------

# psf_hwhm

# Description:
# ------------
# This function estimates the crude HWHMs of the psfs in pixel units.  The
# azimuthally averaged psfs are used.

# Inputs:
# -------
# polar_stats - This numpy dictionary contains the psf polar statistics
#               (calculated from the psf_polar_stats() function).

# Outputs:
# --------
# The numpy int array containing the crude HWHMs of the psfs (pixel units),
# returned via the function value.  The dimensions are Stokes x frequency.

# Modification history:
# ---------------------
# 2011 Oct 17 - Nick Elias, NRAO
#               Initial version.
# 2012 Jan 26 - Nick Elias, NRAO
#               Added a check to insure that blank images have a zero HWHM.

# ------------------------------------------------------------------------------

def psf_hwhm( polar_stats ):

	# Initialize the radii and means

	radius = polar_stats['keyword']['radius']
	mean = polar_stats['mean']


	# Get the shape

	shape = mean.shape

	nS = shape[0]
	rS = range( nS )

	nF = shape[1]
	rF = range( nF )

	nR = shape[2]
	rR = range( nR )


	# Estimate the crude HWHMs

	hwhm = numpy.zeros( [nS,nF], dtype='int' )

	for s in rS:
		for f in rF:

			check = ( numpy.abs( mean[s,f,:] ) <= 1.0E-8 )
			if ( numpy.all( check ) ):
				hwhm[s,f] = 0.0
				continue

			mean[s,f,:] /= mean[s,f,0]

			r0 = nR - 1

			for r in rR[1:]:
				if mean[s,f,r] < 0.5:
					r0 = r
					break

			hwhm_float = 0.5 * ( radius[r0-1] + radius[r0] )
			hwhm[s,f] = int( hwhm_float + 0.5 )


	# Return the crude HWHMs

	return hwhm

# ------------------------------------------------------------------------------

# psf_all_hists

# Description:
# ------------
# This function calculates the histograms for the psfs.

# Inputs:
# -------
# in_psf - This python string contains the name of the psf.
# logger - This python variable contains the logging object information
#          (<python> and <casa> are actual instances of the logger classes).

# Outputs:
# --------
# The python dictionary containing the histogram information, returned via the
# function value.

# The keys:
# 'values'  - The numpy float array containing the histogram bin values.  The
#             indices represent Stokes parameter, frequency, and bin.
# 'counts'  - The numpy float array containing the histogram numbers.  The
#             indices represent Stokes parameter, frequency, and bin.
# 'desc'    - The python dictionary containing the data description.
# 'keyword' - The python dictionary containing the keywords.

# The 'keyword' dictionary mentioned above points to the following keys:
# 'Stokes'    - The numpy string array containing the Stokes parameters.
# 'frequency' - The numpy float array containing the frequencies.
# 'freq_unit' - The python string containing the frequency unit.

# Modification history:
# ---------------------
# 2011 Sep 23 - Nick Elias, NRAO
#               Initial version.
# 2011 Oct 12 - Nick Elias, NRAO
#               The degenerate dimensions required for writing hypercolumns are
#               no longer added here.
# 2011 Oct 18 - Nick Elias, NRAO
#               The function was modified to include the data description and
#               keywords.  The axis order returned by the histogram method of
#               the image analysis tool is not consistent with other axis order
#               returned by other functions, so it is rolled.

# ------------------------------------------------------------------------------

def psf_all_hists( in_psf, logger ):

	# Get the summary of the summary returned by the image analysis and
	# coordinate system tools

	summary = psf_summary( in_psf )

	Stokes = summary['Stokes']

	frequency = summary['frequency']
	freqUnit = summary['freq_unit']


	# Create the local instance of the image analysis tool and open it

        try:
	    iaLoc = casac.image()
        except:
            iaLoc = casa.__imagehome__.create()

	iaLoc.open( in_psf )


	# Calculate the histograms for the psfs.  Roll the axis order to be
	# consistent with other function results.

	h = iaLoc.histograms( axes=[0,1], nbins=100 )

	h['values'] = numpy.rollaxis( h['values'], 0, 3 )
	h['counts'] = numpy.rollaxis( h['counts'], 0, 3 )


	# Close and delete the local image analysis tool

	iaLoc.done()

	del iaLoc


	# Populate the output dictionary

	all_hists = dict()

	all_hists['keyword'] = dict()
	all_hists['keyword']['Stokes'] = Stokes
	all_hists['keyword']['frequency'] = frequency
	all_hists['keyword']['freq_unit'] = freqUnit

	all_hists['desc'] = psf_desc_all_hists()

	all_hists['values'] = h['values']
	all_hists['counts'] = h['counts']


	# Return the output dictionary

	return all_hists

# ------------------------------------------------------------------------------

# psf_all_stats

# Description:
# ------------
# This function calculates the psf statistics.

# Inputs:
# -------
# in_psf       - This python string contains the name of the psf.
# logger       - This python variable contains the logging object information
#                (<python> and <casa> are actual instances of the logger
#                classes).
# stats_thresh - This python float contains the maximum statistics threshold.
#                Its value must be between 0.0 and 1.0 (default = 0.3).

# Outputs:
# --------
# The python dictionary containing the statistics information, returned via the
# function value.

# The keys:
# 'min'       - The numpy float array containing the minimum values.  The
#               indices represent the Stokes parameter and frequency.
# 'max'       - The numpy float array containing the maximum values.  The
#               indices represent the Stokes parameter and frequency.  NB: These
#               values may be close the maximum threshold if it is low.
# 'mean'      - The numpy float array containing the mean values.  The indices
#               represent the Stokes parameter and frequency.
# 'stdev'     - The numpy float array containing the standard deviation values.
#               The indices represent the Stokes parameter and frequency.
# 'rms'       - The numpy float array containing the RMS values.  The indices
#               represent the Stokes parameter and frequency.
# 'median'    - The numpy float array containing the median values.  The indices
#               represent the Stokes parameter and frequency.
# 'medabsdev' - The numpy float array containing the absolute median deviation
#               values.  The indices represent the Stokes parameter and
#               frequency.
# 'desc'      - The python dictionary containing the data description.
# 'keyword'   - The python dictionary containing the keywords.

# The 'keyword' dictionary mentioned above points to the following keys:
# 'Stokes'    - The numpy string array containing the Stokes parameters.
# 'frequency' - The numpy float array containing the frequencies.
# 'freq_unit' - The python string containing the frequency unit.

# Modification history:
# ---------------------
# 2011 Oct 13 - Nick Elias, NRAO
#               Initial version.
# 2011 Oct 18 - Nick Elias, NRAO
#               The function was modified to include the data description and
#               keywords.

# ------------------------------------------------------------------------------

def psf_all_stats( in_psf, logger, stats_thresh=0.3 ):

	# Get the summary of the summary returned by the image analysis and
	# coordinate system tools

	summary = psf_summary( in_psf )

	Stokes = summary['Stokes']

	frequency = summary['frequency']
	freqUnit = summary['freq_unit']


	# Create the temporary masked psf less than the input threshold

	mask_psf = psf_mask( in_psf, thresh=stats_thresh )


	# Create the local instance of the image analysis tool and open it

        try:
	    iaLoc = casac.image()
        except:
            iaLoc = casa.__imagehome__.create()

	iaLoc.open( mask_psf )


	# Create the psf statistics

	plotstats = list()
	plotstats.append( 'max' ) # Likely close to the threshold, if low
	plotstats.append( 'min' )
	plotstats.append( 'mean' )
	plotstats.append( 'sigma' )
	plotstats.append( 'rms' )
	plotstats.append( 'median' )
	plotstats.append( 'medabsdevmed' )

	all_stats = iaLoc.statistics( axes=[0,1], plotstats=plotstats )

	# A kludge, since the image analysis tool erroneously returns all
	# statistics
	keys = all_stats.keys()
	for k in keys:
		if plotstats.count( k ) == 0: del all_stats[k]

	# I want to change this name
	all_stats['stdev'] = all_stats['sigma']
	del all_stats['sigma']

	# I like the shorter name
	all_stats['medabsdev'] = all_stats['medabsdevmed']
	del all_stats['medabsdevmed']


	# Close and delete the local image analysis tool

	iaLoc.done()

	del iaLoc


	# Delete the temporary masked psf

	shutil.rmtree( mask_psf )


	# Fix the dimensions of the numpy arrays in the output dictionary

	try:
		all_stats = cube.cube_fix( in_psf, all_stats, logger )
		del all_stats['Stokes']
		del all_stats['frequency']
	except Exception, err:
		origin = 'psf.psf_all_stats'
		if logger != '': logger.error( err.args[0], origin=origin )
		raise Exception( err.args[0] )


	# Add the keywords and data description to the output dictionary

	all_stats['keyword'] = dict()
	all_stats['keyword']['Stokes'] = Stokes
	all_stats['keyword']['frequency'] = frequency
	all_stats['keyword']['freq_unit'] = freqUnit

	all_stats['desc'] = psf_desc_all_stats()


	# Return the output dictionary

	return all_stats

# ------------------------------------------------------------------------------

# psf_mask

# Description:
# ------------
# This function creates a temporary masked psf less than the input threshold.

# Inputs:
# -------
# in_psf - This python string contains the name of the psf.
# thresh - This python float contains the maximum threshold.  Its value must be
#          between 0.0 and 1.0 (default = 0.3).

# Outputs:
# --------
# The python string containing the temporary masked psf less than the input
# threshold, returned via the function value.

# Modification history:
# ---------------------
# 2011 Oct 13 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

def psf_mask( in_psf, thresh=0.3 ):

	# Create a temporary masked psf less than the maximum threshold

	mask_psf = os.path.splitext( in_psf )[0] + '.mask_psf_temp'

	casa.immath( imagename=[in_psf], mode='evalexpr',
                     expr='IM0[IM0<'+str(thresh)+'*MAX(IM0)]', varnames='',
                     outfile=mask_psf, mask='', region='', box='', chans='',
                     stokes='' )


	# Return the name of the temporary masked psf

	return mask_psf

# ------------------------------------------------------------------------------

# psf_summary

# Description:
# ------------
# This function returns the summary of the summary returned by the image
# analysis and coordinate system tools.

# Inputs:
# -------
# in_psf - This python string contains the name of the psf.

# Outputs:
# --------
# The python dictionary containing the Stokes parameters and frequencies (and
# units), returned via the function value.  The keys:
# 'Stokes'    - The numpy string array containing the Stokes parameters (length
#               1 to 4).
# 'frequency' - The numpy float array containing the frequencies.
# 'freq_unit' - The python string containing the frequency unit.
# 'refpixel'  - The numpy int array containing the reference pixel (length 2).

# Modification history:
# ---------------------
# 2011 Oct 18 - Nick Elias, NRAO
#               Initial version.
# 2011 Oct 28 - Nick Elias, NRAO
#               Fixed string and list of strings issue for the Stokes parameter
#               numpy array.

# ------------------------------------------------------------------------------

def psf_summary( in_psf ):

	# Create and open the local instance of the image analysis tool

        try:
	    iaLoc = casac.image()
        except:
            iaLoc = casa.__imagehome__.create()

	iaLoc.open( in_psf )


	# Get the Stokes parameters and reference pixel from the local
	# coordinate system tool

	csysLoc = iaLoc.coordsys()

	Stokes = numpy.array( list(csysLoc.stokes()), dtype='|S8' )

	refPixel = csysLoc.referencepixel()['numeric'][0:2]
	refPixel = numpy.array( refPixel, dtype='int64' )

	csysLoc.done()
	del csysLoc


	# Get the frequency reference, increment, number, and unit.  Create the
	# frequency array.

	summary = iaLoc.summary()

	fRef = summary['refval'][3]
	dF = summary['incr'][3]
	nF = summary['shape'][3]

	frequency = fRef + ( dF * numpy.linspace( 0, nF-1, num=nF ) )

	fUnit = summary['axisunits'][3]


	# Close the local instance of the image analysis tool and delete it

	iaLoc.close()

	del iaLoc


	# Create the summary of summary dictionary and return it

	d = dict()

	d['Stokes'] = Stokes

	d['frequency'] = frequency
	d['freq_unit'] = fUnit

	d['refpixel'] = refPixel

	return d

# ------------------------------------------------------------------------------

# psf_desc

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
# 2011 Sep 29 - Nick Elias, NRAO
#               Initial version.
# 2011 Oct 12 - Nick Elias, NRAO
#               Added radial maximum and minimum statistics.  Added the radii
#               for the radial histograms and statistics.
# 2011 Oct 13 - Nick Elias, NRAO
#               Added all statistics.
# 2011 Oct 17 - Nick Elias, NRAO
#               Added all 2D Gaussian fits.
# 2011 Oct 18 - Nick Elias, NRAO
#               All dictionary keys were moved to other functions and replaced
#               with a single place holder.

# ------------------------------------------------------------------------------

def psf_desc():

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

# psf_desc_polar_hists

# Description:
# ------------
# This function creates the polar histograms data description dictionary.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The python dictionary containing the polar histograms data description
# dictionary, returned via the function value.

# Modification history:
# ---------------------
# 2011 Oct 18 - Nick Elias, NRAO
#               Initial version

# ------------------------------------------------------------------------------

def psf_desc_polar_hists():

	# Create the polar histograms data description dictionary

	desc = dict()

	desc['counts'] = {
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 4,
		'option': 0,
		'valueType': 'float'
	}
	desc['counts']['comment'] = 'Polar histogram counts (Stokes x frequency'
	desc['counts']['comment'] += ' x radius x bin)'

	desc['values'] = {
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 4,
		'option': 0,
		'valueType': 'float'
	}
	desc['values']['comment'] = 'Polar histogram values (Stokes x frequency'
	desc['values']['comment'] += ' x radius x bin)'


	# Return the data description dictionary

	return desc

# ------------------------------------------------------------------------------

# psf_desc_polar_stats

# Description:
# ------------
# This function creates the polar statistics data description dictionary.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The python dictionary containing the polar statistics data description
# dictionary, returned via the function value.

# Modification history:
# ---------------------
# 2011 Oct 18 - Nick Elias, NRAO
#               Initial version

# ------------------------------------------------------------------------------

def psf_desc_polar_stats():

	# Create the polar statistics data description dictionary

	desc = dict()

	desc['mean'] = {
		'comment': 'Mean versus radius (Stokes x frequency x pixel)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 3,
		'option': 0,
		'valueType': 'float'
	}

	desc['stdev'] = {
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 3,
		'option': 0,
		'valueType': 'float'
	}
	desc['stdev']['comment'] = 'Standard deviation versus radius (Stokes x '
	desc['stdev']['comment'] += 'frequency x pixel)'

	desc['max'] = {
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 3,
		'option': 0,
		'valueType': 'float'
	}
	desc['max']['comment'] = 'Maximum versus radius (Stokes x frequency x '
	desc['max']['comment'] += 'pixel)'

	desc['min'] = {
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 3,
		'option': 0,
		'valueType': 'float'
	}
	desc['min']['comment'] = 'Minimum versus radius (Stokes x frequency x '
	desc['min']['comment'] += 'pixel)'

	desc['fft'] = {
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 4,
		'option': 0,
		'valueType': 'complex'
	}
	desc['fft']['comment'] = 'FFT versus radius (Stokes x frequency x '
	desc['fft']['comment'] += 'radius x fftfreq), standard FFT ordering'


	# Return the data description dictionary

	return desc

# ------------------------------------------------------------------------------

# psf_desc_all_fits

# Description:
# ------------
# This function creates the all fits data description dictionary.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The python dictionary containing the all fits data description dictionary,
# returned via the function value.

# Modification history:
# ---------------------
# 2011 Oct 18 - Nick Elias, NRAO
#               Initial version

# ------------------------------------------------------------------------------

def psf_desc_all_fits():

	# Create the all fits data description dictionary

	desc = dict()

	desc['converged'] = {
		'comment': 'All fits converged booleans (Stokes x frequency)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 2,
		'option': 0,
		'valueType': 'boolean'
	}

	desc['major_axis'] = {
		'comment': 'All fits major axes (Stokes x frequency)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 2,
		'option': 0,
		'valueType': 'float'
	}

	desc['major_axis_err'] = {
		'comment': 'All fits major axis error (Stokes x frequency)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 2,
		'option': 0,
		'valueType': 'float'
	}


	desc['minor_axis'] = {
		'comment': 'All fits minor axes (Stokes x frequency)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 2,
		'option': 0,
		'valueType': 'float'
	}

	desc['minor_axis_err'] = {
		'comment': 'All fits minor axis error (Stokes x frequency)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 2,
		'option': 0,
		'valueType': 'float'
	}

	desc['position_angle'] = {
		'comment': 'All fits position angles (Stokes x frequency)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 2,
		'option': 0,
		'valueType': 'float'
	}

	desc['position_angle_err'] = {
		'comment': 'All fits position angle errors (Stokes x freq)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 2,
		'option': 0,
		'valueType': 'float'
	}


	# Return the data description dictionary

	return desc

# ------------------------------------------------------------------------------

# psf_desc_all_hists

# Description:
# ------------
# This function creates the all histograms data description dictionary.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The python dictionary containing the all histograms data description
# dictionary, returned via the function value.

# Modification history:
# ---------------------
# 2011 Oct 18 - Nick Elias, NRAO
#               Initial version

# ------------------------------------------------------------------------------

def psf_desc_all_hists():

	# Create the all histograms data description dictionary

	desc = dict()

	desc['values'] = {
		'comment': 'All histogram values (bin x Stokes x frequency)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 3,
		'option': 0,
		'valueType': 'float'
	}

	desc['counts'] = {
		'comment': 'All histogram counts (bin x Stokes x frequency)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 3,
		'option': 0,
		'valueType': 'float'
	}


	# Return the data description dictionary

	return desc

# ------------------------------------------------------------------------------

# psf_desc_all_stats

# Description:
# ------------
# This function creates the all statistics data description dictionary.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The python dictionary containing the all statistics data description
# dictionary, returned via the function value.

# Modification history:
# ---------------------
# 2011 Oct 18 - Nick Elias, NRAO
#               Initial version

# ------------------------------------------------------------------------------

def psf_desc_all_stats():

	# Create the all statistics data description dictionary

	desc = dict()

	desc['max'] = {
		'comment': 'Statistics maximum (Stokes x frequency)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 2,
		'option': 0,
		'valueType': 'float'
	}

	desc['min'] = {
		'comment': 'Statistics minimum (Stokes x frequency)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 2,
		'option': 0,
		'valueType': 'float'
	}

	desc['mean'] = {
		'comment': 'Statistics mean (Stokes x frequency)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 2,
		'option': 0,
		'valueType': 'float'
	}

	desc['stdev'] = {
		'comment': 'Statistics std dev (Stokes x frequency)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 2,
		'option': 0,
		'valueType': 'float'
	}

	desc['rms'] = {
		'comment': 'Statistics RMS (Stokes x frequency)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 2,
		'option': 0,
		'valueType': 'float'
	}

	desc['median'] = {
		'comment': 'Statistics median (Stokes x frequency)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 2,
		'option': 0,
		'valueType': 'float'
	}

	desc['medabsdev'] = {
		'comment': 'Statistics median abs dev (Stokes x frequency)',
		'dataManagerGroup': 'SSM',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': 2,
		'option': 0,
		'valueType': 'float'
	}


	# Return the data description dictionary

	return desc
