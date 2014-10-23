from __future__ import absolute_import

import os
import sys
import string
import types
import collections
import numpy as np

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
	fluxdict = self._get_fluxdict(ms=inputs.ms, fieldnamelist=fieldlist,
	    intent=inputs.intent, spwlist=spwlist)
	if not fluxdict:
	    LOG.info('No flux values for MS %s' % inputs.ms.basename)
	    return result

	# Get the tsys as function of spw and field
	#    Return if there are no flux values for the bandpass calibrator.
	tsysdict = self._get_tsysdict(ms=inputs.ms, fieldnamelist=fieldlist,
	    intent=inputs.intent, spwlist=spwlist)
	if not tsysdict:
	    LOG.info('No tsys spw for MS %s' % inputs.ms.basename)
	    return result
	print 'Tsys Dictionary'
	print tsysdict

	# Compute the median temperatures
	for spw in spwlist:
	    if not tsysdict.has_key(spw):
	        continue
            tsystempdict = self._get_mediantemp (inputs.ms,
	        tsysdict[spw]['tsysspw'], tsysdict[spw]['scan'],
		antenna='', temptype='tsys')
	    if not tsystempdict:
	        continue
	    tsysdict[spw]['mediantsys'] = tsystempdict[spw]
	print 'Tsys Dictionary'
	print tsysdict

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

    # Get the fluxes as a function of field and spw
    def _get_fluxdict(self, ms, fieldnamelist=[], intent='', spwlist=[]):

	fluxdict = {}

	for spwid in spwlist:

	    # Get the spectral window object.
	    try:
	        spw = ms.get_spectral_window(spwid)
	    except:
	        continue

	    # Loop over field names. There is normally only one.
	    for fieldname in fieldnamelist:

		# Get fields associated with that name and intent.
		#    There should be only one. Pick the first.
	        fields = ms.get_fields (name=fieldname, intent=intent)
		if len(fields) <= 0:
		    continue
		field = fields[0]

	        # Check for flux densities
	        if len(field.flux_densities) <= 0:
	            continue

		# Find the flux for the spw
		#    Quit after the first successful selection.
		for flux in field.flux_densities:
		    if flux.spw_id != spw.id:
		        continue
		    fluxdict[spw.id] = collections.OrderedDict()
		    (I, Q, U, V) = flux.casa_flux_density
		    fluxdict[spw.id]['fieldname'] = fieldname
		    fluxdict[spw.id]['flux'] = I
	            LOG.info('Setting flux for spw %s  field  %s to %0.2f Jy '  \
		        % (spw.id, fieldname, I))

        return fluxdict

    # Get the fluxes as a function of field and spw
    def _get_tsysdict(self, ms, fieldnamelist=[], intent='', spwlist=[]):

	# Initialize
	tsysdict = {}
	fieldset = set(fieldnamelist)

	# Get atmospheric scans associated with the field list
	#    Ccheck that they come bacj in order
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

	# Loop over the science spws
	for spwid in spwlist:

	    # Get spectral window
	    try:
	        spw = ms.get_spectral_window(spwid)
	    except:
	        continue

	    # Find best atmospheric window
	    ftsysdict = collections.OrderedDict()
	    for scan in atmscans:

		# Get field name
	        scanfieldlist = [field.name for field in scan.fields]
		fieldname = scanfieldlist[0]

		# Get tsys spws and spw ids
	        scanspwlist = [scanspw for scanspw in list(scan.spws) \
		    if scanspw.num_channels not in (1,4)]
	        scanspwidlist = [scanspw.id for scanspw in list(scan.spws) \
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
		if bestspwid is not None:
		    ftsysdict['field'] = fieldname
		    ftsysdict['scan'] = scan.id
		    ftsysdict['tsysspw'] = bestspwid
		    break

	    # Update the spw dictinary
	    if ftsysdict:
	        tsysdict[spw.id] = ftsysdict

	return tsysdict

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
    def _get_mediantemp (self, ms, spwlist, scan, antenna='', temptype='tsys'):
    
        """
        Compute median temperatures for either the Tsys, Trx or Tsky from the
        SYSCAL table of the specified ms for a specified spw list and scan
        combination.
    
        Inputs:
           ms: pipeline measurement set object
           spw: the list of spw ids as integers
           scan: the scan number as a single integer greater than 0
           antenna: '' for all antennas or a single antenna id or name
           temptype: The temperature type 'tsys' (default), 'trx' or 'tsky'
    
        Returns:
           The median temperature for the specified spwlist / scan combinations

        """
    
        # Initialize
        mediantemps = {}
    
        # Temperature type must be one of 'tsys' or 'trx' or 'tsky'
        temptype = temptype.lower()
        if (temptype != 'tsys' and temptype != 'trx' and temptype != 'tsky'):
            return mediantemps
    
        # Get a list of unique antenna ids.
        if antenna == '':
            uniqueAntennaIds = [a.id for a in ms.get_antenna()]
        else:
            uniqueAntennaIds = [ms.get_antenna(search_term=antenna)[0].id]
    
        # Get the requested scan and returns its id and begin and end times.
        reqscan = ms.get_scans(scan_id=scan)
        if not reqscan:
            LOG.info ("Requested scan %d not found" % scan)
            return mediantemps
        begintime = reqscan[0].start_time
        endtime = reqscan[0].end_time
        reqscan = reqscan[0].id
    
        # Get the syscal table meta data.
        #    Create a dummy scans column and populate it with zero.
        with utils.open_table (os.path.join (ms.name,  'SYSCAL')) as table:

            tsys_antennas = table.getcol('ANTENNA_ID')
            if (len(tsys_antennas) < 1):
                LOG.info("The SYSCAL table is blank")
                return mediantemps

            # get columns and tools needed to create tsys times
            time_colkeywords = table.getcolkeywords('TIME')
            time_unit = time_colkeywords['QuantumUnits'][0]
            time_ref = time_colkeywords['MEASINFO']['Ref']
            mt = casatools.measures
            qt = casatools.quanta

            tsys_times = table.getcol('TIME')
            tsys_scans = np.zeros(len(tsys_times), dtype=np.int32)
            nmatch = 0
            for i in range(len(tsys_times)):
                #tsys_times[i] = mt.epoch(time_ref,
		    #qt.quantity(tsys_times[i], time_unit))
                tt = mt.epoch(time_ref, qt.quantity(tsys_times[i], time_unit))

	        print 'row time %s %s %s' % (tt, begintime, endtime)
                if tt >= begintime and tt <= endtime:
    	            tsys_scans[i] = scan
    	            nmatch = nmatch + 1
            if nmatch <= 0:
	        LOG.info ('No time matches for the scan %d' % scan)
                return mediantemps
            #tsys_intervals = table.getcol('INTERVAL')
            #tsys_times -= 0.5*tsys_intervals
            tsys_spws = table.getcol('SPECTRAL_WINDOW_ID')
    
        tsys_uniqueAntennas = np.unique(tsys_antennas)
        tsys_uniqueSpws = np.unique(tsys_spws)
    
    
        # Initialize
        for spw in spwlist:
    
            medians = []
            if (spw not in tsys_uniqueSpws):
                LOG.info ("spw %d is not in the SYSCAL table: " % (spw, tsys_uniqueSpws))
                return mediantemps
    
            # Loop over the rows
            with utils.open_table (os.path.join (ms.name,  'SYSCAL')) as table:
                for i in range(len(tsys_antennas)):
                    if (spws[i] != spw):
                        continue
                    if (antennas[i] not in uniqueAntennaIds):
                        continue
                    if (scan != scans[i]):
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
                LOG.info ("Median %s value for spw %2d = %.1f K" % (temptype, spw, 
    	            mediantemps[spw]))
            else:
                LOG.info ("No data for spw %d scan %d" % (spw, scan))
    	    pass
    
        # Return median temperature per spw and scan.
        return mediantemps 

    # Loop over the scans in scanlist. Compute the list and number of unflagged and
    # flagged antennas for each scan. In most cases there will be only one scan
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

