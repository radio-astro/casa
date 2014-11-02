from __future__ import absolute_import

import os
import sys
import string
import types
import collections
import numpy as np
from copy import deepcopy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools

from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.utils as utils

LOG = infrastructure.get_logger(__name__)

class BpSolintInputs(basetask.StandardInputs):

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, field=None,
         intent=None, spw=None, hm_nantennas=None, maxfracflagged=None,
	 bpsnr=None): 

	 # set the properties to the values given as input arguments
	 self._init_properties(vars())

    @property
    def field(self):
        # If field was explicitly set, return that value
        if self._field is not None:
            return self._field

        # If invoked with multiple mses, return a list of fields
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('field')

        # Otherwise return each field name in the current ms that has been
	# observed with the desired intent
        fields = self.ms.get_fields(intent=self.intent)
	fieldids = set(sorted([f.id for f in fields])) 
	fieldnames = []
	for fieldid in fieldids: 
	    field = self.ms.get_fields(field_id=fieldid)
	    fieldnames.append(field[0].name)
        field_names = set(fieldnames)
        return ','.join(field_names)

    @field.setter
    def field(self, value):
        self._field = value

    @property
    def intent(self):
        if self._intent is not None:
            return self._intent.replace('*', '')
        return None

    @intent.setter
    def intent(self, value):
        if value is None:
            value = 'BANDPASS'
        self._intent = string.replace(value, '*', '')

    @property
    def spw(self):
        # If spw was explicitly set, return that value
        if self._spw is not None:
            return self._spw

        # If invoked with multiple mses, return a list of fields
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('spw')

        # Get the science spw ids
        sci_spws = set([spw.id for spw in \
            self.ms.get_spectral_windows(science_windows_only=True)])

        # Get the bandpass spw ids
        bandpass_spws = []
        for scan in self.ms.get_scans(scan_intent=self.intent):
            bandpass_spws.extend(spw.id for spw in scan.spws)
        bandpass_spws = set (bandpass_spws).intersection(sci_spws)

        # Get science target spw ids
        target_spws = []
        for scan in self.ms.get_scans(scan_intent='TARGET'):
            target_spws.extend([spw.id for spw in scan.spws])
        target_spws = set(target_spws).intersection(sci_spws)
        
        # Compute the intersection of the bandpass and science target spw
        # ids
        spws = list(bandpass_spws.intersection(target_spws))
	spws = [str(spw) for spw in sorted(spws)]
        return ','.join(spws)

    @spw.setter
    def spw(self, value):
        self._spw = value

    @property
    def hm_nantennas (self):
        if self._hm_nantennas is not None:
	    return self._hm_nantennas
	return None

    # Options are 'all' and 'unflagged'
    @hm_nantennas.setter
    def hm_nantennas (self, value):
        if value is None:
	    value = 'unflagged'
	self._hm_nantennas = value

    @property
    def maxfracflagged(self):
        if self._maxfracflagged is not None:
            return self._maxfracflagged
        return None

    @maxfracflagged.setter
    def maxfracflagged(self, value):
        if value is None:
            value = 0.90
        self._maxfracflagged = value

    @property
    def bpsnr(self):
        if self._bpsnr is not None:
            return self._bpsnr
        return None

    @bpsnr.setter
    def bpsnr(self, value):
        if value is None:
            value = 50.0
        self._bpsnr = value


class BpSolint(basetask.StandardTaskTemplate):
    Inputs = BpSolintInputs

    def prepare(self, **parameters):

	# Simplify the inputs
	inputs = self.inputs

	# Create the results structure
	result = BpSolintResults(vis=inputs.vis)

	# Turn the CASA field name and spw id lists into Python lists
	fieldlist = inputs.field.split(',')
	#spwlist = inputs.spw.split(',')
	spwlist = [int(spw) for spw in inputs.spw.split(',')]
	LOG.info('Setting bandpass intent to %s ' % inputs.intent)
	LOG.info('Selecting bandpass fields %s ' % fieldlist)
	LOG.info('Selecting bandpass spws %s ' % spwlist)
	if len(fieldlist) <= 0 or len(spwlist) <= 0:
	    LOG.info('No bandpass data for MS %s' % inputs.ms.basename)
	    return result

	# Get the fluxes as a function of spw and field
	#    Return if there are no flux values for the bandpass calibrator.
	flux_dict = self._get_fluxinfo(inputs.ms, fieldlist, inputs.intent,
	    spwlist)
	if not flux_dict:
	    LOG.info('No flux values for MS %s' % inputs.ms.basename)
	    return result
	print
	print 'Flux dictionary'
	print flux_dict
	print

	# Get the tys dictionary
	#    Populate it initially with the tsys observation information
	#    Return if there are no tsys spws for the bandpass calibrator.
	tsys_dict = self._get_tsysinfo(inputs.ms, fieldlist, inputs.intent,
	    spwlist)
	if not tsys_dict:
	    LOG.info('No tsys spw for MS %s' % inputs.ms.basename)
	    return result
	print 'Tsys dictionary'
	print tsys_dict
	print

	# Initialize the spw dictionary from the tsys dictionary
	spw_dict = deepcopy(tsys_dict)
	print 'Spw dictionary'
	print spw_dict
	print

	# Transfer flux information to the spw dictionary.
	for spw in spwlist:
	    if not flux_dict.has_key(spw):
	        continue
	    if not spw_dict.has_key(spw):
	        continue
	    if spw_dict[spw]['fieldname'] != flux_dict[spw]['fieldname']: 
	        continue
	    spw_dict[spw]['flux'] = flux_dict[spw]['flux']
	print 'Spw dictionary'
	print spw_dict
	print

	# Get the tsys spw list and the tsys associated observing scan list.
	# from the spw dictionary
	tsys_spwlist = []; scan_list = []
	for spw in spwlist:
	    if not spw_dict.has_key(spw):
	        continue
	    if not spw_dict[spw].has_key('tsys_spw'):
	        continue
	    tsys_spwlist.append(spw_dict[spw]['tsys_spw'])
	    scan_list.append(spw_dict[spw]['tsys_obsscan'])

	# Get the median tsys as a function of spw
        tsystemp_dict = self._get_mediantemp (inputs.ms, tsys_spwlist,
	    scan_list, antenna='', temptype='tsys')
	print 'Tsys temp dictionary'
	print tsystemp_dict
	print

	# Transfer the tsys temp information to the spw dictionary
	for spw in spwlist:
	    if not spw_dict.has_key(spw):
	        continue
	    if not tsystemp_dict.has_key(spw_dict[spw]['tsys_spw']):
	        continue
	    spw_dict[spw]['median_tsys'] = tsystemp_dict[spw_dict[spw]['tsys_spw']]
	print 'Spw dictionary'
	print spw_dict
	print

	# Get the observing characteristics dictionary
	obs_dict = self._get_obsinfo (inputs.ms, fieldlist, inputs.intent,
	    spwlist)
	print 'Obs Dictionary'
	print obs_dict
	print

	# Transfer the observing information to the spw dictionary
	for spw in spwlist:
	    if not spw_dict.has_key(spw):
	        continue
	    if not obs_dict.has_key(spw):
	        continue
	    spw_dict[spw]['obs_scans'] = obs_dict[spw_dict[spw]['obs_scans']]
	    spw_dict[spw]['exptime'] = obs_dict[spw_dict[spw]['exptime']]
	    spw_dict[spw]['n7mant'] = obs_dict[spw_dict[spw]['n7mant']]
	    spw_dict[spw]['n12mant'] = obs_dict[spw_dict[spw]['n12mant']]
	    spw_dict[spw]['band'] = obs_dict[spw_dict[spw]['band']]
	    spw_dict[spw]['centerfreq'] = obs_dict[spw_dict[spw]['centerfreq']]
	    spw_dict[spw]['bandwidth'] = obs_dict[spw_dict[spw]['bandwidth']]
	    spw_dict[spw]['nchan'] = obs_dict[spw_dict[spw]['nchan']]
	    spw_dict[spw]['chanwidths'] = obs_dict[spw_dict[spw]['chanwidths']]
	print 'Spw dictionary'
	print spw_dict
	print

	# Compute the bandpass solint parameters
	solint_dict = self._compute_bpsolint(spwlist, spw_dict, inputs.bpsnr)

	# Get the list of bandpass scans and determine the number of
	# antennas in each scan.
	#scanlist, nunflagged_antennas, nflagged_antennas = \
	    #self._get_scanlist(ms=inputs.ms, fieldlist=fieldlist, 
	    #intent=intent, spwlist=spwlist)
	#if not scanlist:
	    #LOG.info('No bandpass scans for MS %s' % inputs.ms.name)
	    #return result

	# Compute the number of unflagged antennas if request.
	#if hm_nantennas == 'unflagged'
	    #nunflagged_antennas, nflagged_antennas = \
	        #self._get_unflagged_antennas(vis=inputs.vis, scanlist=scanlist,
		#maxfracflagged=inputs.maxfracflagged)

	#for scan in scanlist:
	    #LOG.info('Bandpass scan %s nantennas unflagged %d  flagged %d' \
	    #% (scan.id, nunflagged_antennas, nflagged_antennas))

	return result

    def analyse(self, result):
        return result

    # Get the fluxes as a function of field and spw from the pipeline
    # context
    #
    # The flux dictionary keys abd values 
    #        key: the spw id         value: The source dictionary
    # The source dictionary keys and values
    #        key: fieldname          value: The name of the source
    #        key: flux               value: The flux of the source in Jy
    #
    def _get_fluxinfo(self, ms, fieldnamelist, intent, spwlist):

	# Initialize as an ordered dictionary
	fluxdict = collections.OrderedDict()

	# Loop over the science windows
	for spwid in spwlist:

	    # Get the spectral window object.
	    try:
	        spw = ms.get_spectral_window(spwid)
	    except:
	        continue

	    # Loop over field names. There is normally only one.
	    for fieldname in fieldnamelist:

		# Get fields associated with the name and intent.
		#    There should be only one. If there are more
		# than one pick the first field.
	        fields = ms.get_fields (name=fieldname, intent=intent)
		if len(fields) <= 0:
		    continue
		field = fields[0]

	        # Check for flux densities
	        if len(field.flux_densities) <= 0:
	            continue

		# Find the flux for the spw
		#   Take the last section in the list. Check to
		#   see if this is necessary.
		for flux in field.flux_densities:
		    if flux.spw_id != spw.id:
		        continue
		    fluxdict[spw.id] = collections.OrderedDict()
		    (I, Q, U, V) = flux.casa_flux_density
		    fluxdict[spw.id]['fieldname'] = fieldname
		    fluxdict[spw.id]['flux'] = I
	            LOG.info('Setting flux for field %s  spw %s to %0.2f Jy' \
		        % (fieldname, spw.id, I))

        return fluxdict

    # Get the tsys information
    def _get_tsysinfo(self, ms, fieldnamelist, intent, spwlist):

	# Initialize
	tsysdict = collections.OrderedDict()
	fieldset = set(fieldnamelist)

	# Get atmospheric scans associated with the field name list
	atmscans = []
        for scan in ms.get_scans(scan_intent='ATMOSPHERE'):
	    # Remove scans not associated with the input field names
	    scanfieldset = set([field.name for field in scan.fields])
	    if len(fieldset.intersection(scanfieldset)) == 0:
	        continue
	    atmscans.append(scan)

	# No atmospheric scans found
	if not atmscans:
	    return tsysdict

	# Get the scans associated with the field name list and intent
	obscans = []
        for scan in ms.get_scans(scan_intent=intent):
	    # Remove scans not associated with the input field names
	    scanfieldset = set([field.name for field in scan.fields])
	    if len(fieldset.intersection(scanfieldset)) == 0:
	        continue
	    obscans.append(scan)

	# No data scans found
	#    Note possible issue with PHASE, OBSERVE_TARGET scans 
	if not obscans:
	    return tsysdict

	# Loop over the science spws
	for spwid in spwlist:

	    # Get spectral window
	    try:
	        spw = ms.get_spectral_window(spwid)
	    except:
	        continue

	    # Find best atmospheric window
	    ftsysdict = collections.OrderedDict()
	    for atmscan in atmscans:

		# Get field name
	        scanfieldlist = [field.name for field in atmscan.fields]
		fieldname = scanfieldlist[0]

		# Get tsys spws and spw ids
	        scanspwlist = [scanspw for scanspw in list(atmscan.spws) \
		    if scanspw.num_channels not in (1,4)]
	        scanspwidlist = [scanspw.id for scanspw in list(atmscan.spws) \
		    if scanspw.num_channels not in (1,4)]

		# Match the tsys spw to the science spw
		#   Match first by id then by frequency   
		bestspwid = None
		if spw.id in scanspwidlist: 
		    bestspwid = scanspw.id
		else:
		    mindiff = sys.float_info.max
		    for scanspw in scanspwlist:
		        if spw.band != scanspw.band:
			    continue
		        if spw.baseband != scanspw.baseband:
			    continue
			diff = abs(spw.centre_frequency.value - \
			    scanspw.centre_frequency.value)
			if diff < mindiff:
			    bestspwid = scanspw.id
			    mindiff = diff

		# Create dictionary entry based on first scan matched.
		if bestspwid is None:
		    continue
		ftsysdict['fieldname'] = fieldname
		ftsysdict['intent'] = intent
		ftsysdict['tsys_obsscan'] = obscans[0].id
		ftsysdict['tsys_scan'] = atmscan.id
		ftsysdict['tsys_spw'] = bestspwid
		break

	    # Update the spw dictinary
	    if ftsysdict:
	        tsysdict[spw.id] = ftsysdict

	return tsysdict

    # Retrieve observing parameter information
    def _get_obsinfo (self, ms, fieldnamelist, intent, spwidlist):

        obsdict = collections.OrderedDict()
	fieldset = set(fieldnamelist)
	spwset = set(spwidlist)

	# Get the scans associated with the field name list and intent
	obscans = []
        for scan in ms.get_scans(scan_intent=intent):
	    # Remove scans not associated with the input field names
	    scanfieldset = set([field.name for field in scan.fields])
	    if len(fieldset.intersection(scanfieldset)) == 0:
	        continue
	    obscans.append(scan)

	# No data scans found
	if not obscans:
	    return obsdict

        mt = casatools.measures
        qt = casatools.quanta

	# Loop over the spws
	for spwid in spwidlist:

	    # Get spectral window
	    try:
	        spw = ms.get_spectral_window(spwid)
	    except:
	        continue
	    print 'get_obsinfo check spw ids ', spw.id, spwid

	    # Find scans associated with the spw. They may be different from one spw to the next
	    spwscans = []
	    for obscan in obscans:
	        scanspwset = set ([scanspw.id for scanspw in list(obscan.spws) \
		    if scanspw.num_channels not in (1,4)])
	        if len(set([spwid]).intersection(scanspwset)) == 0:
	            continue
		spwscans.append(obscan)
	    if not spwscans:
	        continue

	    obsdict[spwid] = collections.OrderedDict()

	    # Limit the scans per spw to those for the first field
	    #    in the scan sequence.
	    fieldnames = [field.name for field in spwscans[0].fields]
	    fieldname = fieldnames[0]
	    fscans = []
	    for scan in spwscans:
		fnames = [field.name for field in scan.fields]
	        if fieldname != fnames[0]:
		    continue
		fscans.append(scan)
	    scanids = [scan.id for scan in fscans]
	    obsdict[spwid]['obs_scans'] = scanids

	    # Figure out the number of antennas
	    #   Flagging not taken into account here
	    n7mant = 0; n12mant = 0
	    for scan in fscans:
	        n7mant = max (n7mant, len ([a for a in scan.antenna if a.diameter == 7.0]))
	        n12mant = max (n12mant, len ([a for a in scan.antenna if a.diameter == 12.0]))
	    obsdict[spwid]['n12mant'] = n12mant
	    obsdict[spwid]['n7mant'] = n12mant

	    # Retrieve total exposure time in minutes
	    #    Add to dictionary
	    exposureTime = 0.0
	    for scan in fscans:
		scanTime = float (scan.time_on_source.total_seconds()) / 60.0
	        exposureTime = exposureTime + scanTime
	    obsdict[spw.id]['exptime'] = exposureTime

	    # Retrieve spw characteristics
	    #    Receiver band, center frequency, bandwidth, number of
	    #        channels, and median channel width
	    #    Add to dictionary
	    obsdict[spwid]['band'] = spw.band 
	    obsdict[spwid]['centerfreq'] = float(spw.centre_frequency.value) 
	    obsdict[spwid]['bandwidth'] = float(spw.bandwidth.value) 
	    obsdict[spwid]['nchan'] = spw.num_channels 
	    channels = spw.channels
	    chanwidths = np.zeros(spw.num_channels)
	    for i in range(spw.num_channels):
	        chanwidths[i] = (channels[i].high - channels[i].low).value 
	    obsdict[spwid]['chanwidths'] = np.median(chanwidths) 

	return obsdict

    # Create the bandpass scan list. Initialize the number of unflagged
    # and flagged antennas for each scan.
    def _get_scanlist (self, ms, fieldlist=[], intent='', spwlist=[]):

	fieldset = set(fieldlist)
	spwset = set(spwlist)

	# Loop over the scans with the correct intent.
	scanlist = []; nunflagged_antennas = []; nflagged_antennas = []
        for scan in ms.get_scans(scan_intent=intent):

	    # Skip scans with the wrong field name
	    scanfieldset = set([field.name for field in scan.fields])
	    if len(fieldset.intersection(scanfieldset)) == 0:
	        continue

	    # Skip scans with an incorrect spw
	    scanspwset = set ([spw.id for spw in scan.spws()])
	    if len(spwset.intersection(scanspwset)) == 0:
	        continue

	    # Update the lists
	    scanlist.append(scan)
	    nunflagged_antennas.append(len(scan.antennas))
	    nflagged_antennas.append(0)

	return scanlist, nunflagged_antennas, nflagged_antennas 

    
    # Get median temperatures
    def _get_mediantemp (self, ms, tsys_spwlist, scan_list, antenna='', temptype='tsys'):
    
        """
        Compute median temperatures for either the Tsys, Trx or Tsky from the
        SYSCAL table of the specified ms for a specified spw list and scan
        combination.
    
        Inputs:
           ms: pipeline measurement set object
           tsys_spwlist: the list of tsys spw ids as integers
           scan_list: the list of corresponding observation scan numbers
           antenna: '' for all antennas or a single antenna id or name
           temptype: The temperature type 'tsys' (default), 'trx' or 'tsky'
    
        Returns:
           The median temperature for the specified tsys_spwlist / scan combinations

        """
    
        # Initialize
        mediantemps = {}
    
        # Temperature type must be one of 'tsys' or 'trx' or 'tsky'
        if (temptype != 'tsys' and temptype != 'trx' and temptype != 'tsky'):
            return mediantemps

	# Get list of unique scan ids.
	uniqueScans = list(set(scan_list))

	# Determine the start and end times for each unique scan
	beginScanTimes = []; endScanTimes = []
	for scan in uniqueScans:
            reqscan = ms.get_scans(scan_id=scan)
	    if not reqscan:
                LOG.info ('Observation scan %d found' % scan)
                return mediantemps
	    startTime = reqscan[0].start_time 
	    endTime = reqscan[0].end_time 
	    beginScanTimes.append(startTime)
	    endScanTimes.append(endTime)
    
        # Get the syscal table meta data.
        with utils.open_table (os.path.join (ms.name,  'SYSCAL')) as table:

	    # Get the antenna ids
            tsys_antennas = table.getcol('ANTENNA_ID')
            if (len(tsys_antennas) < 1):
                LOG.info("The SYSCAL table is blank")
                return mediantemps

            # Get columns and tools needed to understand the tsys times
            time_colkeywords = table.getcolkeywords('TIME')
            time_unit = time_colkeywords['QuantumUnits'][0]
            time_ref = time_colkeywords['MEASINFO']['Ref']
            mt = casatools.measures
            qt = casatools.quanta

	    # Get time and intervals
            tsys_start_times = table.getcol('TIME')
            tsys_intervals = table.getcol('INTERVAL')

	    # Compute the time range of validity for each tsys measurement 
	    #    Worry about memory efficiency later
	    tsys_start_times = tsys_start_times - 0.5 * tsys_intervals
            tsys_end_times = np.zeros(len(tsys_start_times))
            tsys_end_times = tsys_start_times + tsys_intervals

	    # Create a scan id array and populate it with zeros
            scanids = np.zeros(len(tsys_start_times), dtype=np.int32)

	    # Determine if a tsys measurement matches the scan interval 
	    #    If it does  set the scan to the scan id
            nmatch = 0
            for i in range(len(tsys_start_times)):

		# Time conversions
		#    Not necessary if scan begin and end times are not converted
                tstart = mt.epoch(time_ref, qt.quantity(tsys_start_times[i], time_unit))
                tend = mt.epoch(time_ref, qt.quantity(tsys_end_times[i], time_unit))

		# Scan starts after end of validity interval or ends before
		# the beginning of tje validity interval
		for j in range(len(uniqueScans)):
                    if beginScanTimes[j] > tend or endScanTimes[j] < tstart:
		        continue
	            #print 'Row %d  tsys times %s %s  scan times %s %s' % \
		        #(i, tstart, tend, beginScanTimes[j], endScanTimes[j])
		    if scanids[i] <= 0:
    	                scanids[i] = uniqueScans[j]
    	                nmatch = nmatch + 1

            if nmatch <= 0:
	        LOG.info ('No time matches for scans %s tsys spws %s' % (uniqueScans,
		    tsys_spwlist))
                return mediantemps
	    else:
	        LOG.info ('Number of matching rows for scans %s tsys spws %s %d / %d' %
		    (uniqueScans, tsys_spwlist, nmatch, len(tsys_start_times)))

	    # Get the spw ids
            tsys_spws = table.getcol('SPECTRAL_WINDOW_ID')
            tsys_uniqueSpws = np.unique(tsys_spws)
    
        # Get a list of unique antenna ids.
        if antenna == '':
            uniqueAntennaIds = [a.id for a in ms.get_antenna()]
        else:
            uniqueAntennaIds = [ms.get_antenna(search_term=antenna)[0].id]
    
        # Lopp over the spw and scan list which have the same length
        for spw, scan in zip (tsys_spwlist, scan_list):
    
            if (spw not in tsys_uniqueSpws):
                LOG.info ("tsys spw %d is not in the SYSCAL table: " % spw)
                return mediantemps
    
            # Loop over the rows
            medians = []
            with utils.open_table (os.path.join (ms.name,  'SYSCAL')) as table:
                for i in range(len(tsys_antennas)):
                    if (tsys_spws[i] != spw):
                        continue
                    if (tsys_antennas[i] not in uniqueAntennaIds):
                        continue
                    if (scan != scanids[i]):
                        continue
                    if (temptype == 'tsys'):
                        tsys = table.getcell('TSYS_SPECTRUM',i)
                    elif (temptype == 'trx'):
                        tsys = table.getcell('TRX_SPECTRUM',i)
                    elif (temptype == 'tsky'):
                        tsys = table.getcell('TSKY_SPECTRUM',i)
                    medians.append(np.median(tsys))
    
            if (len(medians) > 0):
                mediantemps[spw] = np.median(medians)
                LOG.info ("Median %s value for spw %2d = %.1f K" % \
		    (temptype, spw, mediantemps[spw]))
            else:
                LOG.info ("No data for spw %d scan %d" % (spw, scan))
    
        # Return median temperature per spw and scan.
        return mediantemps 

    # Loop over the scans in scanlist. Compute the list and number of unflagged
    # and flagged antennas for each scan. In most cases there will be only one
    # scan.
    def _get_unflagged_antennas(self, vis, scanlist=[], maxfracflagged = 0.90):

	# Loop over the bandpass scans
	nunflagged_antennas = [], nflagged_antennas = []
        for scan in scanlist:

	    # Execute the CASA flagdata task for each bandpass scan
	    flagdata_task = casa_tasks.flagdata(vis=vis, scan=str(scan.id),
	        mode='summary')
	    flagdata_result = flagdata_task.execute(dry_run=False)

	    # Add up the antennas
	    unflagged_antennas = []; flagged_antennas = []
	    antennas = flagdata_result['report0']['antenna'].keys()
	    for antenna in sorted(antennas):
	        points = flagdata_result['antenna'][antenna]
		fraction = points['flagged']/points['total']
		if (fraction < maxfracflagged):
		    unflagged_antennas.append(antenna)
		else:
		    flagged_antennas.append(antenna)

	    # Compute the lengtjs
	    nunflagged_antennas.append(len(unflagged_antennas))
	    nflagged_antennas.append(len(flagged_antennas))

        return nunflagged_antennas, nflagged_antennas

    # Compute the bandpass frequency solution interval given the
    # spw list and the spw dictionary 
    def _compute_bpsolint(self, spwlist, spw_dict, reqSnr):

	# The ALMA receiver bands defined per pipeine convention
	#    There is pipeline interface for this at the moment
	ALMA_BANDS = ['ALMA Band 3', 'ALMA Band 4', 'ALMA Band 6', \
	    'ALMA Band 7', 'ALMA Band 8', 'ALMA Band 9', 'ALMA Band 10']
	ALMA_TSYS = [75.0, 86.0, 90.0, 150.0, 387.0, 1200.0, 1515.0]
	ALMA_SENSITIVITIES = [0.20, 0.24, 0.27, 0.50, 1.29, 5.32, 8.85]  # mJy (for 16*12 m antennas, 1 minute, 8 GHz, 2pol)


        solint_dict = collections.OrderedDict()

	for spwid in spwlist:
	    bandidx = ALMA_BANDS.index(spw_dict[spwid]['band'])
	    relativeTsys = spw_dict[swpid]['median_tsys'] / ALMA_TSYS[bandidx]
	    timeFactor = 1.0 / np.sqrt(spw_dict[spwid]['exposure'])
	    nbaselines = spw_dict[spwid]['n7mant'] + spw_dict[spwid]['n12mant'] - 1
	    arraySizeFactor = np.sqrt(16 * 15 / 2.0) / np.sqrt(nbaselines)
	    if spw_dict[spwid]['n7mant'] == 0:
	        areaFactor = 1.0
	    elif spw_dict[spwid]['n12mant'] == 0:
	        areaFactor = (12.0 / 7.0) ** 2
	    else:
		ntotant = spw_dict[spwid]['n7mant'] + spw_dict[spwid]['n12mant']
	        areaFactor = (spw_dict[spwid]['n12mant'] + (12.0 / 7.0)**2 * spw_dict[spwid]['n7mant']) / ntotant 
	    bandwidthFactor = np.sqrt(8.0e9 / spw_dict[spwid]['chanwidths'])
	    polarizationFactor = np.sqrt(2.0)
	    factor = relativeTsys * timeFactor * arraySizeFactor * areaFactor * \
	        bandwidthFactor * PolarizationFactor
	    sensitivity = ALMA_SENSITIVITIES[bandidx] * factor
	    snrPerChannel = spw_dict[spw]['flux'] * 1000.0 / sensitivity
	    requiredChannels = ( reqSnr / snrPerChannel ) ** 2

	    # Fill in the dictionary
	    solint_dict[spwid] = collection.OrderedDict()
	    solint_dict[spwid]['solint'] = '%fMHz' % (requiredChannels * spw_dict[spwid]['chanwidths'] * 1.0e-6)
	    solint_dict[spwid]['nchan_total'] = spw_dict[spwid]['nchan']
	    solint_dict[spwid]['nchan_solint'] = '%d' % (np.ceil(requireChannels))
	    solint_dict[spwid]['median_tsys'] = spw_dict[spwid]['median_tsys']
	    solint_dict[spwid]['tsys_spw'] = spw_dict[spwid]['tsys_spw']
	    solint_dict[spwid]['band'] = spw_dict[spwid]['band']
	    solint_dict[spwid]['minutes'] = spw_dict[spwid]['exposure']
	    solint_dict[spwid]['snr_per_channel'] = snrPerChannel
	    solint_dict[spwid]['sensitivity_mJy'] = sensitivity
	    solint_dict[spwid]['chanwidth_Hz'] = spw_dict[spwid]['chanwidths']
	    solint_dict[spwid]['flux_Jy'] = spw_dict[spwid]['flux']
	    solint_dict[spwid]['frequency_Hz'] = spw_dict[spwid]['centerfreq']

	return solint_dict
	    

class BpSolintResults(basetask.Results):
    def __init__(self, vis=None, solintdict={}):
        """
        Initialise the results object.
        """
        super(BpSolintResults, self).__init__()
        self.vis=vis
        self.solintdict = solintdict

#    def merge_with_context(self, context):
#
#        if self.vis is None:
#            LOG.error ( ' No results to merge ')
#            return
#
#        if not self.phaseup_result.final:
#            LOG.error ( ' No results to merge ')
#            return
#
#        # Merge the spw phaseup offset table
#        self.phaseup_result.merge_with_context(context)
#
#        # Merge the phaseup spwmap
#        ms = context.observing_run.get_ms( name = self.vis)
#        if ms:
#            ms.phaseup_spwmap = self.phaseup_spwmap

    def __repr__(self):
        if self.vis is None or not self.solintdict:
            return('BpSolintResults:\n'
            '\tNo bandpass solution intervals computed')
        else:
            spwmap = 'BpSolintResults:\n\dictionary = %s\n' % \
		(self.solintdict)
            return spwmap

