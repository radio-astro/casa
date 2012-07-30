
# ------------------------------------------------------------------------------

# caltab_convert2.py

# NB:
# 0. I threw this code together because it's temporary, so I'm not happy with
#    the design.
# 1. These functions are provided as a convenience to convert old-format
#    caltables to new-format caltables.
# 2. BPOLY and GSPLINE caltables are not supported.
# 3. The information transferred should be enough for most calibration purposes.
# 4. Simple bugs will be fixed.  If new features are required, however, you must
#    recalculate a new-format caltable.

# Modification history:
# ---------------------
# 2011 Jan 30 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

# Imports

import os

import numpy

from casac import casac

# ------------------------------------------------------------------------------

# caltab_convert2

# NB: I threw this code together because it's temporary, so I'm not happy with
# the design.

# NB: The type of the PARAM (and PARAMERR) columns in the new-format caltable
# cannot be determined from the old-format caltable, so it must be supplied by
# the user (see the inputs).  It is either 'float' or 'complex'.  It should be
# easy to choose the correct value.  Delays, WVR, etc. type calibration data
# correspond to 'float'.  Gains, bandpass, polarization, etc. type calibration
# data correspond to 'complex'.  If you are not sure what to use, ask George
# Moellenbrock or Nick Elias.

# NB: If all of the values in the NUM_CHAN column of the old-format caltable
# are 1, this function assumes that the calibration is averaged over frequency
# for each spectral window.  The average frequency is the median over all
# channels listed in the MS.  This scheme ignores user channel selection
# information, which is not available.

# NB: The REF_ANT column in the old-format caltable (analogous to the ANTENNA2
# column in the new-format caltable) is messed up, which means that I fill
# ANTENNA2 with -1.  Hopefully, that will not create a fatal problem.

# Inputs:
# -------
# caltabold - The name of the old-format caltable.
# ms        - The name of the MS associated with the old-format caltable.
# pType     - The type of data in the PARAM column of the new-format caltable.
#             The allowed types are 'float' and 'complex'.
# caltabnew - The name of the new-format caltable.  It is optional.  The
#             default is caltabold + '.new'.

# Modification history:
# ---------------------
# 2012 Jan 30 - Nick Elias, NRAO
#               Initial version.
# 2012 Mar 26 - gmoellen: fixed header information
# 2012 May 09 - gmoellen: added proper cal_desc_id->spw_id conversion
#
# ------------------------------------------------------------------------------

def caltab_convert2( caltabold, ms, pType, caltabnew='' ):

	# Check the inputs

	if not os.path.exists( caltabold ):
		raise IOError( 'Invalid old-format caltable.' )

	if not os.path.exists( ms ):
		raise IOError( 'Invalid visibility file (MS).' )

	pTypeTemp = pType.strip().lower()
	if ( pTypeTemp != 'complex' and pTypeTemp != 'float' ):
		raise Exception( 'Invalid parameter type ("complex" or "float").' )

  	if caltabnew == '': caltabnew = caltabold + '.new'
	if os.path.exists( caltabnew ):
		raise IOError( 'New-format caltable already exists.' )


	# Open the old-format caltable and get the number of rows

	tbOld = casac.table()
	tbOld.open( caltabold )
	nRow = tbOld.nrows()

	# Get spwid map from old CAL_DESC subtable
	tbCD = casac.table()
	tbCD.open( caltabold+'/CAL_DESC' )
	spwmap=tbCD.getcol('SPECTRAL_WINDOW_ID')[0,:]
	tbCD.close()

  	# Create the empty new-format caltable with the correct number of rows

  	tbNew = casac.table()
  	tbNew.create( caltabnew, desc_new( pTypeTemp ) )

  	tbNew.addrows( nRow )


	# Transfer most column information from the old-format caltable to the
	# new-format caltable.  NB: The REF_ANT column in the old-format
	# caltable (analogous to the ANTENNA2 column in the new-format caltable)
	# is messed up, which means that I fill ANTENNA2 with -1.

  	tbNew.putcol( 'ANTENNA1', tbOld.getcol( 'ANTENNA1' ) )
	tbNew.putcol( 'ANTENNA2', -1.0*numpy.ones( nRow, dtype='int' ) )
	tbNew.putcol( 'FIELD_ID', tbOld.getcol( 'FIELD_ID' ) )
	tbNew.putcol( 'FLAG', tbOld.getcol( 'FLAG' ) )
	tbNew.putcol( 'INTERVAL', tbOld.getcol( 'INTERVAL' ) )
	tbNew.putcol( 'SNR', tbOld.getcol( 'SNR' ) )
	# Map CAL_DESC_ID to SPECTRAL_WINDOW_ID:
	tbNew.putcol( 'SPECTRAL_WINDOW_ID', spwmap[tbOld.getcol( 'CAL_DESC_ID' )] )
	tbNew.putcol( 'TIME', tbOld.getcol( 'TIME' ) )
	tbNew.putcol( 'WEIGHT', tbOld.getcol( 'FIT_WEIGHT' ) )


	# Transfer the parameter column from the old-format caltable to the
	# new-format caltable, taking the type into account.  No parameter
	# errors are present in the old-format caltable, so they are set to
	# 1.0 (+j0.0).

	param = tbOld.getcol( 'GAIN' )
	if pTypeTemp == 'float':
		param = param.real
		tbNew.putcol( 'FPARAM', param )
	else:
		tbNew.putcol( 'CPARAM', param )

	tbNew.putcol( 'PARAMERR', -1.0*numpy.ones(param.shape,dtype='float') )


	# Determine the scans and put them into the new-format caltable

	scanTimes = get_scantimes( ms )
	keys = scanTimes.keys()

	times = tbOld.getcol( 'TIME' )
	scans = numpy.zeros( 0, dtype='int' )

	for t in times.tolist():
	
		flag = False

		for i in range( len(keys) ):
			k = keys[i]
			if t>=scanTimes[k]['min'] and t<scanTimes[k]['max']:
				flag = True
				scans = numpy.append( scans, k )
				break
			if i == len(keys)-1: break
			l = keys[i+1]
			if t>=scanTimes[k]['max'] and t<scanTimes[l]['min']:
				flag = True
				scans = numpy.append( scans, l )
				break

		if not flag: scans = numpy.append( scans, -1 )

	tbNew.putcol( 'SCAN_NUMBER', scans )


	# Copy the appropriate subcaltables from the MS to the new-format
	# caltable

	arg = 'cp -r ' + ms + '/ANTENNA ' + caltabnew
	os.system( arg )

	arg = 'cp -r ' + ms + '/FIELD ' + caltabnew
	os.system( arg )

#	arg = 'cp -r ' + caltabold + '/CAL_HISTORY ' + caltabnew + '/HISTORY'
#	os.system( arg )

  	tbHis = casac.table()
	tbHis.open(ms+'/HISTORY')
	tbHis.copy(newtablename=caltabnew+'/HISTORY',deep=True,valuecopy=True,norows=True)
	tbHis.close()
	del tbHis
	
	arg = 'cp -r ' + ms + '/SPECTRAL_WINDOW ' + caltabnew \
		+ '/SPECTRAL_WINDOW'
	os.system( arg )


	# Add the info and keywords to the main table of the new-format caltable
	tabinfo=tbOld.info()

	tbNew.putinfo(tabinfo)

	if (pTypeTemp=='complex'):
		tbNew.putkeyword( 'ParType', 'Complex' )
	if (pTypeTemp=='float'):
		tbNew.putkeyword( 'ParType', 'Float' )
	
	tbNew.putkeyword( 'MSName', ms )
	tbNew.putkeyword( 'VisCal', tabinfo['subType'] )
			  
	polBasis = get_polbasis( ms )
	tbNew.putkeyword( 'PolBasis', polBasis )

	tbNew.putkeyword( 'ANTENNA', 'Table: ' + caltabnew + '/ANTENNA' )
	tbNew.putkeyword( 'FIELD', 'Table: ' + caltabnew + '/FIELD' )
	tbNew.putkeyword( 'HISTORY', 'Table: ' + caltabnew + '/HISTORY' )
	tbNew.putkeyword( 'SPECTRAL_WINDOW',
		'Table: ' + caltabnew + '/SPECTRAL_WINDOW' )


	# Add the column keywords to the main table of the new-format caltable

	colList = ['INTERVAL', 'TIME']

	for col in colList:
		colKeys = tbOld.getcolkeywords( col )
		if colKeys != {}: tbNew.putcolkeywords( col, colKeys )


	# Close the old- and new- format caltables

	tbOld.close()
	del tbOld

	tbNew.close()
	del tbNew


	# Get the channel ranges from the CAL_DESC subtable of the old-format
	# caltable

	tbDesc = casac.table()
	tbDesc.open( caltabold + '/CAL_DESC' )

	nDesc = tbDesc.nrows()
	rDesc = range( nDesc )

	spwMap = tbDesc.getcol( 'SPECTRAL_WINDOW_ID' )[0]

	chanMin = numpy.zeros( nDesc, dtype='int' )
	chanMax = numpy.zeros( nDesc, dtype='int' )

	for d in rDesc:
		chanRange = tbDesc.getcol( 'CHAN_RANGE', startrow=d, nrow=1 )
		chanMin[d] = numpy.min( chanRange[0,:,:,:][0,:,0] )
		chanMax[d] = numpy.max( chanRange[1,:,:,:][0,:,0] )

	if numpy.all( chanMax - chanMin + 1 == 1 ):
		gain = True
	else:
		gain = False

	tbDesc.close()
	del tbDesc


	# Modify the columns of the new-format caltable according to the
	# channel ranges

	tbSPW = casac.table()
	tbSPW.open( caltabnew + '/SPECTRAL_WINDOW', nomodify=False )

	for d in rDesc:

		s = int( spwMap[d] )

		nChan = int( chanMax[d] - chanMin[d] + 1 )
		tbSPW.putcell( 'NUM_CHAN', s, nChan )

		chanFreq = tbSPW.getcell( 'CHAN_FREQ', s )
		if gain:
			chanFreq = numpy.median( chanFreq )
		else:
			chanFreq = chanFreq[chanMin[d]:chanMax[d]+1]
		tbSPW.putcell( 'CHAN_FREQ', s, chanFreq )

		chanWidth = tbSPW.getcell( 'CHAN_WIDTH', s )
		if gain:
			chanWidth = numpy.sum( chanWidth )
		else:
			chanWidth = chanWidth[chanMin[d]:chanMax[d]+1]
		tbSPW.putcell( 'CHAN_WIDTH', s, chanWidth )
		tbSPW.putcell( 'EFFECTIVE_BW', s, chanWidth )
		tbSPW.putcell( 'RESOLUTION', s, chanWidth )

		totalBandwidth = numpy.sum( chanWidth )
		tbSPW.putcell( 'TOTAL_BANDWIDTH', s, totalBandwidth )

	tbSPW.close()
	del tbSPW


	# Return True

	return True

# ------------------------------------------------------------------------------

def get_scantimes( ms ):

	# Open the MS

	tbHandle = casac.table()
	tbHandle.open( ms )


	# Get the time and scan columns

	times = tbHandle.getcol( 'TIME' )
	scans = tbHandle.getcol( 'SCAN_NUMBER' )


	# Assign each time to the proper scan

	d = dict()

	for r in range( tbHandle.nrows() ):
		if not d.has_key( scans[r] ): d[scans[r]] = list()
		d[scans[r]].append( times[r] )

	tbHandle.close()
	del tbHandle


	# Get the minimum and maximum time for each scan

	scanTimes = dict()

	for k in d.keys():
		scanTimes[k] = dict()
		scanTimes[k]['min'] = numpy.min( d[k] )
		scanTimes[k]['max'] = numpy.max( d[k] )


	# Return the minimum and maximum time for each scan

	return scanTimes

# ------------------------------------------------------------------------------

def get_polbasis( ms ):

	# Open the MS

	tbHandle = casac.table()
	tbHandle.open( ms + '/FEED' )


	# Get the POLARIZATION_TYPE column

	pt = tbHandle.getcol( 'POLARIZATION_TYPE' )


	# Determine the polarization basis

	if pt[0,:][0] == 'R' or pt[1,:][0] == 'R':
		polBasis = 'CIRCULAR'
	else:
		polBasis = 'LINEAR'


	# Return the polarization basis

	return polBasis

# ------------------------------------------------------------------------------

def desc_new( pType ):

	desc = dict()

	desc['ANTENNA1'] = {'comment': '',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 5,
		'valueType': 'int'}

	desc['ANTENNA2'] = {'comment': '',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 5,
		'valueType': 'int'}

	desc['FIELD_ID'] = {'comment': '',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 5,
		'valueType': 'int'}

	desc['FLAG'] = {'comment': '',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': -1,
		'option': 0,
		'valueType': 'boolean'}

	desc['INTERVAL'] = {'comment': '',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 5,
		'valueType': 'double'}

	if pType.lower() == 'float':
		desc['FPARAM'] = {'comment': '',
			'dataManagerGroup': 'StandardStMan',
			'dataManagerType': 'StandardStMan',
			'maxlen': 0,
			'ndim': -1,
			'option': 0,
			'valueType': 'float'}
	else:
		desc['CPARAM'] = {'comment': '',
			'dataManagerGroup': 'StandardStMan',
			'dataManagerType': 'StandardStMan',
			'maxlen': 0,
			'ndim': -1,
			'option': 0,
			'valueType': 'complex'}

	desc['PARAMERR'] = {'comment': '',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': -1,
		'option': 0,
		'valueType': 'float'}

	desc['SCAN_NUMBER'] = {'comment': '',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 5,
		'valueType': 'int'}

	desc['SNR'] = {'comment': '',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': -1,
		'option': 0,
		'valueType': 'float'}

	desc['SPECTRAL_WINDOW_ID'] = {'comment': '',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 5,
		'valueType': 'int'}

	desc['TIME'] = {'comment': '',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'option': 5,
		'valueType': 'double'}

	desc['WEIGHT'] = {'comment': '',
		'dataManagerGroup': 'StandardStMan',
		'dataManagerType': 'StandardStMan',
		'maxlen': 0,
		'ndim': -1,
		'option': 0,
		'valueType': 'float'}

  	desc['_define_hypercolumn_'] = {}

	return desc

