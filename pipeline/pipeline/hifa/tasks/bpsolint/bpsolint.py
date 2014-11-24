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
         intent=None, spw=None, phaseupsnr=None, minphaseupints=None,
	 bpsnr=None, minbpnchan=None, hm_nantennas=None, maxfracflagged=None): 

	 # set the properties to the values given as input arguments
	 self._init_properties(vars())

    @property
    def field(self):
        # If field was explicitly set, return that value
        if self._field is not None:
            return self._field

        # If invoked with multiple ms's, return a list of fields
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

        # If invoked with multiple mses, return a list of spws
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
    def phaseupsnr(self):
        if self._phaseupsnr is not None:
            return self._phaseupsnr
        return None

    @phaseupsnr.setter
    def phaseupsnr(self, value):
        if value is None:
            value = 20.0
        self._phaseupsnr = value

    @property
    def minphaseupints(self):
        if self._minphaseupints is not None:
            return self._minphaseupints
        return None

    @minphaseupints.setter
    def minphaseupints(self, value):
        if value is None:
            value = 2
        self._minphaseupints = value

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

    @property
    def minbpnchan(self):
        if self._minbpnchan is not None:
            return self._minbpnchan
        return None

    @minbpnchan.setter
    def minbpnchan(self, value):
        if value is None:
            value = 8
        self._minbpnchan = value

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


class BpSolint(basetask.StandardTaskTemplate):
    Inputs = BpSolintInputs

    def prepare(self, **parameters):

	# Simplify the inputs
	inputs = self.inputs

	# Turn the CASA field name and spw id lists into Python lists
	fieldlist = inputs.field.split(',')
	spwlist = [int(spw) for spw in inputs.spw.split(',')]

	# Log the data selection choices
	LOG.info('Estimating bandpass solution intervals')
	LOG.info('    Setting bandpass intent to %s ' % inputs.intent)
	LOG.info('    Selecting bandpass fields %s ' % fieldlist)
	LOG.info('    Selecting bandpass spws %s ' % spwlist)
	LOG.info('    Setting requested phaseup snr to %0.1f ' % \
	    (inputs.phaseupsnr))
	LOG.info('    Setting requested bandpass snr to %0.1f ' % \
	    (inputs.bpsnr))
	if len(fieldlist) <= 0 or len(spwlist) <= 0:
	    LOG.info('    No bandpass data for MS %s' % inputs.ms.basename)
	    return BpSolintResults(vis=inputs.vis)

	# Get the flux dictionary from the pipeline context
	#    Return if there are no flux values for the bandpass calibrator.
	flux_dict = self._get_fluxinfo(inputs.ms, fieldlist, inputs.intent,
	    spwlist)
	if not flux_dict:
	    LOG.info('No flux values for MS %s' % inputs.ms.basename)
	    return BpSolintResults(vis=inputs.vis)

	# Get the Tsys dictionary
	#    This dictionary defines the science to Tsys scan mapping and the
	#    science spw to Tsys spw mapping.
	#    Return if there are no Tsys spws for the bandpass calibrator.
	tsys_dict = self._get_tsysinfo(inputs.ms, fieldlist, inputs.intent,
	    spwlist)
	if not tsys_dict:
	    LOG.info('No Tsys spw for MS %s' % inputs.ms.basename)
	    return BpSolintResults(vis=inputs.vis)

	# Construct the Tsys spw list and the associated bandpass scan list.
	# from the Tsys dictionary
	tsys_spwlist, scan_list = self._make_tsyslists(spwlist, tsys_dict)

	# Get the median Tsys dictionary as a function of Tsys spw
        tsystemp_dict = self._get_mediantemp (inputs.ms, tsys_spwlist,
	    scan_list, antenna='', temptype='tsys')

	# Get the observing characteristics dictionary as a function of spw
	#    This includes the spw configuration, time on source and
	#    integration information
	obs_dict = self._get_obsinfo (inputs.ms, fieldlist, inputs.intent,
	    spwlist)

	# Combine all the dictionariies
	spw_dict = self._join_dicts (spwlist, tsys_dict, flux_dict,
	     tsystemp_dict, obs_dict)

	# Compute the bandpass solint parameters and return a solution
	# dictionary
	solint_dict = self._compute_bpsolint(spwlist, spw_dict,
	    inputs.phaseupsnr, inputs.minphaseupints, inputs.bpsnr,
	    inputs.minbpnchan)

	# Construct the results object
	#   Should I just pass the dictionary ?
	result = self._get_results (inputs.vis, spwlist, solint_dict)

	# Return the results
	return result

    def analyse(self, result):
        return result

    # Get the fluxes as a function of field and spw from the pipeline
    # context and return a dictionary.
    #
    # The flux dictionary keys abd values 
    #        key: the spw id         value: The source dictionary
    # The source dictionary keys and values
    #        key: field_name         value: The name of the source
    #        key: flux               value: The flux of the source in Jy
    #
    #
    def _get_fluxinfo(self, ms, fieldnamelist, intent, spwlist):

	# Initialize as an ordered dictionary
	fluxdict = collections.OrderedDict()
	LOG.info('Finding sources fluxes')

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
		#   Take the last selection in the list.
		for flux in field.flux_densities:
		    if flux.spw_id != spw.id:
		        continue
		    fluxdict[spw.id] = collections.OrderedDict()
		    (I, Q, U, V) = flux.casa_flux_density
		    fluxdict[spw.id]['field_name'] = fieldname
		    fluxdict[spw.id]['flux'] = I
	            LOG.info( \
		        '    Setting flux for field %s spw %s to %0.2f Jy' \
		        % (fieldname, spw.id, I))

        return fluxdict

    # Get the tsys information as functions of field and spw and
    # return a dictionary
    #
    # The tsys dictionary keys abd values 
    #        key: the spw id       value: The source dictionary
    # The source dictionary keys and values
    #        key: atm_field_name   value: The name of the source
    #        key: intent           value: The intent of the bandpass source
    #        key: bandpass_scan    value: The bandpass scan associated with Tsys
    #        key: tsys_scan        value: The Tsys scan for Tsys computation
    #        key: tsys_spw         value: The Tsys spw
    #
    #
    def _get_tsysinfo(self, ms, fieldnamelist, intent, spwlist):

	# Initialize
	tsysdict = collections.OrderedDict()
	LOG.info('Matching spws')

	# Get the list of unique field names
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
	#    If phase calibrator examine the TARGET atmospheric scans
	if not atmscans and intent == 'PHASE':

	    # Get science target names
	    scifields = ms.get_fields(intent='TARGET')
	    if len(scifields) <= 0:
	       return tsysdict
	    scifieldset = set([scifield.name for scifield in scifields])

	    # Find atmospheric scans associated with the science target
            for scan in ms.get_scans(scan_intent='ATMOSPHERE'):
	        # Remove scans not associated with the input field names
	        scanfieldset = set([field.name for field in scan.fields])
	        if len(scifieldset.intersection(scanfieldset)) == 0:
	            continue
	        atmscans.append(scan)

	# Still no atmospheric scans found
	#    Return
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
	if not obscans:
	    return tsysdict

	# Loop over the science spws
	for spwid in spwlist:

	    # Get spectral window
	    try:
	        spw = ms.get_spectral_window(spwid)
	    except:
	        continue

	    # Find best atmospheric spw
	    #    This dictionary is created only if the spw id is valid
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

		# Match the Tsys spw to the science spw
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

		# No spw match found
		if bestspwid is None:
		    continue

		# Create dictionary entry based on first scan matched.
		ftsysdict['atm_field_name'] = fieldname
		ftsysdict['intent'] = intent

		# Pick the first obs scan following the Tsys scan
		#    This should deal with the shared phase / science target
		#    scans
		for obscan in obscans:
		    if obscan.id > atmscan.id:
		        ftsysdict['bandpass_scan'] = obscan.id
			break

		ftsysdict['tsys_scan'] = atmscan.id
		ftsysdict['tsys_spw'] = bestspwid
		break

	    # Update the spw dictinary
	    if ftsysdict:
		LOG.info('    Matched spw %d to a Tsys spw %d' % (spwid, bestspwid))
	        tsysdict[spwid] = ftsysdict
	    else:
		LOG.warn('    Cannot match spw %d to a Tsys spw' % spwid)

	return tsysdict

    # Get the Tsys spw list and the tsys bandpass scan list.
    # from the spw dictionary
    def _make_tsyslists (self, spwlist, tsys_dict):
	tsys_spwlist = []; scan_list = []
	for spw in spwlist:
	    if not tsys_dict.has_key(spw):
	        continue
	    if not tsys_dict[spw].has_key('tsys_spw'):
	        continue
	    tsys_spwlist.append(tsys_dict[spw]['tsys_spw'])
	    scan_list.append(tsys_dict[spw]['bandpass_scan'])

	return tsys_spwlist, scan_list

    # Get the observing information and return a dictionary
    #
    # The observing dictionary keys abd values 
    #        key: the spw id         value: The source observing dictionary
    # The observation dictionary keys and values
    #        key: bandpass_scans     value: The lists of bandpass source scans
    #        key: num_12mantenna     value: The max number of 12 m antennas
    #        key: num_7mantenna      value: The max number of 7 m antennas
    #        key: exptime            value: The exposure time in minutes
    #        key: integrationtime    value: The mean integration time in minutes
    #        key: band               value: The ALMA receiver band
    #        key: bandcenter         value: The center frequency in Hz
    #        key: bandwidth          value: The band width in Hz 
    #        key: nchan              value: The number of channels
    #        key: chanwidths         value: The median channel width in Hz
    #
    def _get_obsinfo (self, ms, fieldnamelist, intent, spwidlist):

        obsdict = collections.OrderedDict()
	LOG.info ('Observation summary')
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

	    # Find scans associated with the spw. They may be different from
	    # one spw to the next
	    spwscans = []
	    for obscan in obscans:
	        scanspwset = set ([scanspw.id for scanspw in list(obscan.spws) \
		    if scanspw.num_channels not in (1,4)])
	        if len(set([spwid]).intersection(scanspwset)) == 0:
	            continue
		spwscans.append(obscan)
	    if not spwscans:
	        continue


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
	    if not fscans:
	        continue

	    obsdict[spwid] = collections.OrderedDict()
	    scanids = [scan.id for scan in fscans]
	    obsdict[spwid]['bandpass_scans'] = scanids

	    # Figure out the number of 7m and 12 m antennas
	    #   Note comparison of floating point numbers is tricky ...
	    #   Flagging not taken into account here
	    n7mant = 0; n12mant = 0
	    for scan in fscans:
	        n7mant = max (n7mant, len ([a for a in scan.antennas \
		    if a.diameter == 7.0]))
	        n12mant = max (n12mant, len ([a for a in scan.antennas \
		    if a.diameter == 12.0]))
	    obsdict[spwid]['num_12mantenna'] = n12mant
	    obsdict[spwid]['num_7mantenna'] = n7mant

	    # Retrieve total exposure time and mean integration time in minutes
	    #    Add to dictionary
	    exposureTime = 0.0
	    meanInterval = 0.0
	    for scan in fscans:
		#scanTime = float (scan.time_on_source.total_seconds()) / 60.0
		scanTime = scan.exposure_time(spw.id) / 60.0
	        exposureTime = exposureTime + scanTime
		#intTime = scan.mean_interval(spw.id).total_seconds() / 60.0
		intTime = scan.mean_interval(spw.id)
		intTime = (intTime.seconds + intTime.microseconds * 1.0e-6) / 60.0
		meanInterval = meanInterval + intTime
	    obsdict[spw.id]['exptime'] = exposureTime
	    obsdict[spw.id]['integrationtime'] = meanInterval / len(fscans)

	    # Retrieve spw characteristics
	    #    Receiver band, center frequency, bandwidth, number of
	    #    channels, and median channel width
	    #    Add to dictionary
	    obsdict[spwid]['band'] = spw.band 
	    obsdict[spwid]['bandcenter'] = float(spw.centre_frequency.value) 
	    obsdict[spwid]['bandwidth'] = float(spw.bandwidth.value) 
	    obsdict[spwid]['nchan'] = spw.num_channels 
	    channels = spw.channels
	    chanwidths = np.zeros(spw.num_channels)
	    for i in range(spw.num_channels):
	        chanwidths[i] = (channels[i].high - channels[i].low).value 
	    obsdict[spwid]['chanwidths'] = np.median(chanwidths) 

	    LOG.info('For field %s spw %2d scans %s' % (fieldname, spwid, scanids))
	    LOG.info('    %2d 12m antennas  %2d 7m antennas  exposure %0.3f minutes  interval %0.3f minutes' % \
	        (obsdict[spwid]['num_12mantenna'], obsdict[spwid]['num_7mantenna'], exposureTime, meanInterval))

	return obsdict
    
    # Get Tsys median temperatures and return as a dictionary
    #
    # The observing dictionary keys abd values 
    #        key: the spw id         value: The median Tsys value

    def _get_mediantemp (self, ms, tsys_spwlist, scan_list, antenna='',
        temptype='tsys'):
    
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
           The median temperature for the specified tsys_spwlist / scan
	   combinations

        """
    
        # Initialize
        mediantemps = collections.OrderedDict()
	LOG.info('Estimating Tsys temperatures')
    
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
                LOG.warn ('Cannot find observation scan %d' % scan)
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
                LOG.warn("The SYSCAL table is blank")
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
                tstart = mt.epoch(time_ref, qt.quantity(tsys_start_times[i],
		    time_unit))
                tend = mt.epoch(time_ref, qt.quantity(tsys_end_times[i],
		    time_unit))

		# Scan starts after end of validity interval or ends before
		# the beginning of the validity interval
		for j in range(len(uniqueScans)):
                    if beginScanTimes[j] > tend or endScanTimes[j] < tstart:
		        continue
		    if scanids[i] <= 0:
    	                scanids[i] = uniqueScans[j]
    	                nmatch = nmatch + 1

            if nmatch <= 0:
	        LOG.warn ( \
		'No SYSCAL table row matches for scans %s tsys spws %s' % \
		(uniqueScans, tsys_spwlist))
                return mediantemps
	    else:
	        LOG.info ( \
		    '    SYSCAL table row matches for scans %s Tsys spws %s %d / %d' % \
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
                LOG.warn ("Tsys spw %d is not in the SYSCAL table: " % spw)
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
                LOG.info ( \
		    "    Median Tsys %s value for Tsys spw %2d = %.1f K" % \
		    (temptype, spw, mediantemps[spw]))
            else:
                LOG.warn ("    No Tsys data for spw %d scan %d" % (spw, scan))
    
        # Return median temperature per spw and scan.
        return mediantemps 

    # Combine all the dictionaries
    #
    def _join_dicts (self, spwlist, tsys_dict, flux_dict, tsystemp_dict, obs_dict):

	# Initialize the spw dictionary from the Tsys dictionary
	#    Make a deep copy of this dictionary
	spw_dict = deepcopy(tsys_dict)

	# Transfer flux information to the spw dictionary.
	self._transfer_fluxes (spwlist, spw_dict, flux_dict)

	# Transfer the tsys temperature information to the spw dictionary
	self._transfer_temps (spwlist, spw_dict, tsystemp_dict)

	# Transfer the observing information to the spw dictionary
	self._transfer_obsinfo (spwlist, spw_dict, obs_dict)

	return spw_dict

    # Transfer flux information from the flux dictionary to the spw dictionary.
    #
    def _transfer_fluxes (self, spwlist, spw_dict, flux_dict):
	for spw in spwlist:
	    if not flux_dict.has_key(spw):
	        continue
	    if not spw_dict.has_key(spw):
	        continue
	    #if spw_dict[spw]['atm_field_name'] != flux_dict[spw]['field_name']: 
	        #continue
	    spw_dict[spw]['field_name'] = flux_dict[spw]['field_name']
	    spw_dict[spw]['flux'] = flux_dict[spw]['flux']


    # Transfer the tsys temp information to the spw dictionary
    def _transfer_temps (self, spwlist, spw_dict, tsystemp_dict):
	for spw in spwlist:
	    if not spw_dict.has_key(spw):
	        continue
	    if not tsystemp_dict.has_key(spw_dict[spw]['tsys_spw']):
	        continue
	    spw_dict[spw]['median_tsys'] = \
	        tsystemp_dict[spw_dict[spw]['tsys_spw']]

    # Transfer the observing information to the spw dictionary
    def _transfer_obsinfo (self, spwlist, spw_dict, obs_dict):
	for spw in spwlist:
	    if not spw_dict.has_key(spw):
	        continue
	    if not obs_dict.has_key(spw):
	        continue
	    spw_dict[spw]['bandpass_scans'] = obs_dict[spw]['bandpass_scans']
	    spw_dict[spw]['exptime'] = obs_dict[spw]['exptime']
	    spw_dict[spw]['integrationtime'] = obs_dict[spw]['integrationtime']
	    spw_dict[spw]['num_7mantenna'] = obs_dict[spw]['num_7mantenna']
	    spw_dict[spw]['num_12mantenna'] = obs_dict[spw]['num_12mantenna']
	    spw_dict[spw]['band'] = obs_dict[spw]['band']
	    spw_dict[spw]['bandcenter'] = obs_dict[spw]['bandcenter']
	    spw_dict[spw]['bandwidth'] = obs_dict[spw]['bandwidth']
	    spw_dict[spw]['nchan'] = obs_dict[spw]['nchan']
	    spw_dict[spw]['chanwidths'] = obs_dict[spw]['chanwidths']


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
    #
    # The bandpass preaveraging dictionary keys abd values 
    #        key: the spw id     value: The science spw id as an integer
    # The preaveraging parameter dictionary keys abd values 
    #        key: band               value: The ALMA receiver band
    #        key: frequency_Hz       value: The frequency of the spw
    #        key: nchan_total        value: The total number of channels
    #        key: chanwidth_Hz       value: The median channel width in Hz
    #
    #        key: tsys_spw           value: The tsys spw id as an integer
    #        key: median_tsys        value: The median tsys value
    #
    #        key: flux_Jy            value: The flux of the source in Jy
    #        key: exptime_minutes    value: The exposure time in minutes
    #        key: snr_per_channel    value: The signal to noise per channel
    #        key: sensitivity_per_channel_mJy    value: The sensitivity in mJy per channel

    #        key: bpsolint           value: The frequency solint in MHz
    #        key: nchan_bpsolint       value: The total number of solint channels

    def _compute_bpsolint(self, spwlist, spw_dict, reqPhaseupSnr,
        minBpNintervals, reqBpSnr, minBpNchan):

	# The ALMA receiver band, nominal tsys, and sensitivity info.
	#    This information should go elsewhere in the next release
	#    The ALMA receiver bands are defined per pipeline convention

	ALMA_BANDS = ['ALMA Band 3', 'ALMA Band 4', 'ALMA Band 6', \
	    'ALMA Band 7', 'ALMA Band 8', 'ALMA Band 9', 'ALMA Band 10']
	ALMA_TSYS = [75.0, 86.0, 90.0, 150.0, 387.0, 1200.0, 1515.0]
	ALMA_SENSITIVITIES = [0.20, 0.24, 0.27, 0.50, 1.29, 5.32, 8.85] \
	    # mJy (for 16*12 m antennas, 1 minute, 8 GHz, 2pol)

	# Initialize
        solint_dict = collections.OrderedDict()

	for spwid in spwlist:

	    # Determine the receiver band
	    bandidx = ALMA_BANDS.index(spw_dict[spwid]['band'])

	    # Compute the various SNR factors
	    #    The following are shared between the phaseup time solint and
	    #    the bandpass frequency solint
	    relativeTsys = spw_dict[spwid]['median_tsys'] / ALMA_TSYS[bandidx]
	    nbaselines = spw_dict[spwid]['num_7mantenna'] + \
	        spw_dict[spwid]['num_12mantenna'] - 1
	    arraySizeFactor = np.sqrt(16 * 15 / 2.0) / np.sqrt(nbaselines)
	    if spw_dict[spwid]['num_7mantenna'] == 0:
	        areaFactor = 1.0
	    elif spw_dict[spwid]['num_12mantenna'] == 0:
	        areaFactor = (12.0 / 7.0) ** 2
	    else:
		# Not sure this is correct
		ntotant = spw_dict[spwid]['num_7mantenna'] + \
		    spw_dict[spwid]['num_12mantenna']
	        areaFactor = (spw_dict[spwid]['num_12mantenna'] + \
		    (12.0 / 7.0)**2 * spw_dict[spwid]['num_7mantenna']) / \
		    ntotant 
	    polarizationFactor = np.sqrt(2.0)

	    # Phaseup bandpass time solint
	    putimeFactor = 1.0 / np.sqrt(spw_dict[spwid]['integrationtime'])
	    pubandwidthFactor = np.sqrt(8.0e9 / spw_dict[spwid]['bandwidth'])
	    pufactor = relativeTsys * putimeFactor * arraySizeFactor * \
	        areaFactor * pubandwidthFactor * polarizationFactor
	    pusensitivity = ALMA_SENSITIVITIES[bandidx] * pufactor
	    snrPerIntegration = spw_dict[spwid]['flux'] * 1000.0 / pusensitivity
	    requiredIntegrations = (reqPhaseupSnr / snrPerIntegration) ** 2

	    # Bandpass frequency solint
	    bptimeFactor = 1.0 / np.sqrt(spw_dict[spwid]['exptime'])
	    bpbandwidthFactor = np.sqrt(8.0e9 / spw_dict[spwid]['chanwidths'])
	    bpfactor = relativeTsys * bptimeFactor * arraySizeFactor * \
	        areaFactor * bpbandwidthFactor * polarizationFactor
	    bpsensitivity = ALMA_SENSITIVITIES[bandidx] * bpfactor
	    snrPerChannel = spw_dict[spwid]['flux'] * 1000.0 / bpsensitivity
	    requiredChannels = ( reqBpSnr / snrPerChannel ) ** 2

	    # Fill in the dictionary
	    solint_dict[spwid] = collections.OrderedDict()

	    # Science spw info
	    solint_dict[spwid]['band'] = spw_dict[spwid]['band']
	    solint_dict[spwid]['frequency_Hz'] = spw_dict[spwid]['bandcenter']
	    solint_dict[spwid]['bandwidth'] = spw_dict[spwid]['bandwidth']
	    solint_dict[spwid]['nchan_total'] = spw_dict[spwid]['nchan']
	    solint_dict[spwid]['chanwidth_Hz'] = spw_dict[spwid]['chanwidths']

	    # Tsys spw info
	    solint_dict[spwid]['tsys_spw'] = spw_dict[spwid]['tsys_spw']
	    solint_dict[spwid]['median_tsys'] = spw_dict[spwid]['median_tsys']

	    # Sensitivity info
	    solint_dict[spwid]['flux_Jy'] = spw_dict[spwid]['flux']
	    solint_dict[spwid]['integration_minutes'] = \
	        spw_dict[spwid]['integrationtime']
	    solint_dict[spwid]['sensitivity_per_integration_mJy'] = \
	       pusensitivity
	    solint_dict[spwid]['snr_per_integration'] = snrPerIntegration
	    solint_dict[spwid]['exptime_minutes'] = spw_dict[spwid]['exptime']
	    solint_dict[spwid]['snr_per_channel'] = snrPerChannel
	    solint_dict[spwid]['sensitivity_per_channel_mJy'] = bpsensitivity

	    # Phaseup bandpass solution info
	    if requiredIntegrations <= 1.0:
	        solint_dict[spwid]['phaseup_solint'] = 'int'
	        solint_dict[spwid]['nint_phaseup_solint'] = 1
	    else:
	        solint_dict[spwid]['phaseup_solint'] = '%fs' % \
	            (solint_dict[spwid]['integrationtime'] * \
		    requiredIntegrations * 60.0)
	        solint_dict[spwid]['nint_phaseup_solint'] = \
		    int(np.ceil(requiredIntegrations))
	    solInts = int (np.ceil(solint_dict[spwid]['exptime_minutes'] / \
	        solint_dict[spwid]['integration_minutes'])) / \
	        int(np.ceil(requiredIntegrations))
	    if solInts < minBpNintervals:
	        tooFewIntervals = True
		asterisks = '***'
	    else:
	        tooFewIntervals = False
	        asterisks = ''
	    LOG.info("%sspw %2d (%6.3fmin) requires phaseup solint='%6.3fsec' (%d time intervals in solution) to reach S/N=%.0f" % \
	        (asterisks,
		spwid,
		solint_dict[spwid]['exptime_minutes'],
		60.0 * requiredIntegrations * solint_dict[spwid]['integration_minutes'],
		solInts,
		reqPhaseupSnr))
	    solint_dict[spwid]['nphaseup_solutions'] = solInts
	    if tooFewIntervals:
	        LOG.warn( \
		"%s This spw would have less than %d time intervals in its solution" % \
		(asterisks, minBpNintervals))

	    # Bandpass solution info
	    solint_dict[spwid]['bpsolint'] = '%fMHz' % \
	        (requiredChannels * solint_dict[spwid]['chanwidth_Hz'] * 1.0e-6)
	    solint_dict[spwid]['nchan_bpsolint'] = \
	        int(np.ceil(requiredChannels))
	    solChannels = solint_dict[spwid]['nchan_total'] / \
	        int(np.ceil(requiredChannels)) 
	    if solChannels  < minBpNchan:
	        tooFewChannels = True
		asterisks = '***'
	    else:
	        tooFewChannels = False
	        asterisks = ''
	    LOG.info("%sspw %2d (%4.0fMHz) requires solint='%.2fMHz' (%d channels intervals in solution) to reach S/N=%.0f" % \
	        (asterisks,
		spwid,
		solint_dict[spwid]['bandwidth']*1.0e-6,
		requiredChannels * solint_dict[spwid]['chanwidth_Hz'] * 1.0e-6,
		solChannels,
		reqBpSnr))
	    solint_dict[spwid]['nbandpass_solutions'] = solChannels
	    if tooFewChannels:
	        LOG.warn("%s This spw would have less than %d channels in its solution" % (asterisks, minBpNchan))

	return solint_dict

    # Get final results from the spw dictionary
    def _get_results (self, vis, spwidlist, solint_dict):

	# Initialize result structure.
	result = BpSolintResults(vis=vis, spwids=spwidlist)

	# Initialize the lists
        phsolints = []
        phintsolints = []
	nphsolutions = []
	phsensitivities = []
	phintsnrs = []

        bpsolints = []
        bpchansolints = []
	nbpsolutions = []
        bpsensitivities = []
        bpchansnrs = []

	# Loop over the spws. Values for spws with
	# not dictionary entries are set to None
        for spwid in spwidlist:

	    if not solint_dict.has_key(spwid):

		phsolints.append(None)
		phintsolints.append(None)
		nphsolutions.append(None)
		phsensitivities.append(None)
		phintsnrs.append(None)

	        bpsolints.append(None)
	        bpchansolints.append(None)
		nbpsolutions.append(None)
	        bpsensitivities.append(None)
	        bpchansnrs.append(None)

	    else:

	        phsolints.append(solint_dict[spwid]['phaseup_solint'])
	        phintsolints.append(solint_dict[spwid]['nint_phaseup_solint'])
		nphsolutions.append(solint_dict[spwid]['nphaseup_solutions'])
	        phsensitivities.append( \
		    '%fmJy' % solint_dict[spwid]['sensitivity_per_integration_mJy'])
	        phintsnrs.append(solint_dict[spwid]['snr_per_integration'])

	        bpsolints.append(solint_dict[spwid]['bpsolint'])
	        bpchansolints.append(solint_dict[spwid]['nchan_bpsolint'])
		nbpsolutions.append(solint_dict[spwid]['nbandpass_solutions'])
	        bpsensitivities.append( \
		    '%fmJy' % solint_dict[spwid]['sensitivity_per_channel_mJy'])
	        bpchansnrs.append(solint_dict[spwid]['snr_per_channel'])

	# Populate the result.
        result.phsolints = phsolints
        result.phintsolints = phintsolints
	result.nphsolutions = nphsolutions
	result.phsensitivities = phsensitivities
	result.phintsnrs = phintsnrs

        result.bpsolints = bpsolints
        result.bpchansolints = bpchansolints
	result.nbpsolutions = nbpsolutions
        result.bpchansensitivities = bpsensitivities
        result.bpchansnrs = bpchansnrs

        return  result

	    
# The results class

class BpSolintResults(basetask.Results):
    def __init__(self, vis=None, spwids=[],
        phsolints=[], phintsolints=[], nphsolutions=[],
	phsensitivities=[], phintsnrs=[],
        bpsolints=[], bpchansolints=[], nbpsolutions=[],
	bpsensitivities=[], bpchansnrs=[]):

        """
        Initialise the results object.
        """
        super(BpSolintResults, self).__init__()

        self.vis=vis

	# Spw list
        self.spwids = spwids

	# Phaseup solutions
	self.phsolints = phsolints
	self.phintsolints = phintsolints
	self.nphsolutions = nphsolutions
	self.phsensitivities = phsensitivities
	self.phintsnrs = phintsnrs

	# Bandpass solutions
	self.bpsolints = bpsolints
	self.bpchansolints = bpchansolints
	self.nbpsolutions = nbpsolutions
	self.bpchansensitivities = bpsensitivities
	self.bpchansnrs = bpchansnrs

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
        if self.vis is None or not self.spwids:
            return('BpSolintResults:\n'
            '\tNo bandpass solution intervals computed')
        else:
            line = 'BpSolintResults:\nvis %s\n' % (self.vis)
	    line = line + 'Phaseup solution time intervals\n'
	    for i in range(len(self.spwids)):
	        line = line + \
		    "    spwid %2d solint '%s' intsolint %2d sensitivity %s intsnr %0.1f\n" % \
		    (self.spwids[i], self.phsolints[i], self.phintsolints[i], \
		    self.phsensitivities[i], self.phintsnrs[i])
	    line = line + 'Bandpass frequency solution intervals\n'
	    for i in range(len(self.spwids)):
	        line = line + \
		    "    spwid %2d solint '%s' channels %2d sensitivity %s chansnr %0.1f\n" % \
		    (self.spwids[i], self.bpsolints[i], self.bpchansolints[i], \
		    self.bpchansensitivities[i], self.bpchansnrs[i])
            return line

