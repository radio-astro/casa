import os
import sys
import collections
import numpy as np
from copy import deepcopy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from pipeline.infrastructure import casa_tasks

LOG = infrastructure.get_logger(__name__)

"""
The ALMA receiver band, nominal tsys, and sensitivity info.
    This information should go elsewhere in the next release
    The ALMA receiver bands are defined per pipeline convention
"""

ALMA_BANDS = ['ALMA Band 3', 'ALMA Band 4', 'ALMA Band 6', \
    'ALMA Band 7', 'ALMA Band 8', 'ALMA Band 9', 'ALMA Band 10']
ALMA_TSYS = [75.0, 86.0, 90.0, 150.0, 387.0, 1200.0, 1515.0]
ALMA_SENSITIVITIES = [0.20, 0.24, 0.27, 0.50, 1.29, 5.32, 8.85] \
    # mJy (for 16*12 m antennas, 1 minute, 8 GHz, 2pol)

"""
Estimate the optimal solint for the selected bandpass data and return
the solution in the form of a dictionary
"""

def estimate_bpsolint (ms, fieldlist, intent, spwidlist, compute_nantennas,
    max_fracflagged, phaseupsnr, minphaseupints, bpsnr, minbpnchan):

    """
    Input Parameters
                   ms: The pipeline context ms object
        fieldnamelist: The list of field names to be selected 
               intent: The intent of the fields to be selected 
            spwidlist: The list of spw ids to be selected
    compute_nantennas: The algorithm for computing the number of unflagged antennas ('all', 'flagged') 
      max_fracflagged: The maximum fraction of an antenna can be flagged, e.g. 0.90
           phaseupsnr: The desired phaseup gain solution SNR, e.g. 20.0
       minphaseupints: The minimum number of phaseup solution intervals, e.g. 2
                bpsnr: The desired bandpass solution SNR, e.g. 50.0
           minbpnchan: The minimum number of bandpass solution intervals, e.g. 8

    The output solution interval dictionary

    The bandpass preaveraging dictionary keys abd values
        key: the spw id     value: The science spw id as an integer

    The preaveraging parameter dictionary keys abd values
        key: 'band'               value: The ALMA receiver band
        key: 'frequency_Hz'       value: The frequency of the spw
        key: 'nchan_total'        value: The total number of channels
        key: 'chanwidth_Hz'       value: The median channel width in Hz

        key: 'tsys_spw'           value: The tsys spw id as an integer
        key: 'median_tsys'        value: The median tsys value

        key: 'flux_Jy'            value: The flux of the source in Jy
        key: 'exptime_minutes'    value: The exposure time in minutes
        key: 'snr_per_channel'    value: The signal to noise per channel
        key: 'sensitivity_per_channel_mJy'    value: The sensitivity in mJy per channel

        key: 'bpsolint'           value: The frequency solint in MHz
    """

    # Get the flux dictionary from the pipeline context
    flux_dict = get_fluxinfo(ms, fieldlist, intent, spwidlist)
    if not flux_dict:
        LOG.info('No flux values')
        return {}

    # Get the Tsys dictionary
    #    This dictionary defines the science to Tsys scan mapping and the
    #    science spw to Tsys spw mapping.
    #    Return if there are no Tsys spws for the bandpass calibrator.
    tsys_dict = get_tsysinfo(ms, fieldlist, intent, spwidlist)
    if not tsys_dict:
        LOG.info('No Tsys spws')
        return {}

    # Construct the Tsys spw list and the associated bandpass scan list.
    # from the Tsys dictionary
    tsys_spwlist, scan_list = make_tsyslists(spwidlist, tsys_dict)

    tsystemp_dict = get_mediantemp (ms, tsys_spwlist, scan_list,
        antenna='', temptype='tsys')
    if not tsystemp_dict:
        LOG.info('No Tsys estimates')
        return {}

    # Get the observing characteristics dictionary as a function of spw
    #    This includes the spw configuration, time on source and
    #    integration information
    obs_dict = get_obsinfo (ms, fieldlist, intent, spwidlist,
        compute_nantennas=compute_nantennas, max_fracflagged=max_fracflagged)
    if not obs_dict:
        LOG.info('No observation scans')
        return {}

    # Combine all the dictionariies
    spw_dict = join_dicts (spwidlist, tsys_dict, flux_dict,
         tsystemp_dict, obs_dict)

    # Compute the bandpass solint parameters and return a solution
    # dictionary
    solint_dict = compute_bpsolint(ms, spwidlist, spw_dict,
        phaseupsnr, minphaseupints, bpsnr, minbpnchan)

    return solint_dict

"""
Retrieve the fluxes of selected sources from the pipeline context
as a function of spw id and return the results in a dictinary indexed
by spw id.
"""

def get_fluxinfo(ms, fieldnamelist, intent, spwidlist):

    """
    The input parameters
               ms: The pipeline context ms object
    fieldnamelist: The list of field names to be selected 
           intent: The intent of the fields to be selected 
          spwlist: The list of spw ids to be selected

    The output flux dictionary fluxdict

    The flux dictionary key and value 
        key: The spw id      value: The source flux dictionary

    The source flux dictionary keys and values
        key: 'field_name'    value: The name of the source, e.g. '3C286'
        key: 'flux'          value: The flux of the source in Jy, e.g. 1.53
    """

    # Initialize the flux dictionary as an ordered dictionary
    fluxdict = collections.OrderedDict()
    LOG.info('Finding sources fluxes')

    # Loop over the science spectral windows
    for spwid in spwidlist:

        # Get the spectral window object.
        try:
            spw = ms.get_spectral_window(spwid)
        except:
            continue

        # Loop over field names. There is normally only one.
        for fieldname in fieldnamelist:

            # Get fields associated with the name and intent.
            #    There should be only one. If there is more
            #    than one pick the first field.
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

"""
Get the tsys information as functions of spw and return a dictionary
"""

def get_tsysinfo(ms, fieldnamelist, intent, spwidlist):

    """
    Input parameters
               ms: The pipeline context ms object
    fieldnamelist: The list of field names to be selected 
           intent: The intent of the fields to be selected 
       spwidlist: The list of spw ids to be selected

    The output dictionary

    The tsys dictionary tsysdict keys and values 
        key: The spw id       value: The Tsys source dictionary

    The tsys source dictionary keys and values
        key: 'tsys_field_name'  value: The name of the Tsys field source, e.g. '3C286' (was 'atm_field_name') 
        key: 'intent'           value: The intent of the selected source, e.g. 'BANDPASS'
        key: 'snr_scan'         value: The scan associated with Tsys used to compute the SNR, e.g. 4 (was 'bandpass_scan')
        key: 'tsys_scan'        value: The Tsys scan to be used for Tsys computation, e.g. 3
        key: 'tsys_spw'         value: The Tsys spw associated with the science spw id, e.g. 13
    """

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
    for spwid in spwidlist:

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
                #bestspwid = scanspw.id
                bestspwid = spw.id
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
            ftsysdict['tsys_field_name'] = fieldname
            ftsysdict['intent'] = intent

            # Pick the first obs scan following the Tsys scan
            #    This should deal with the shared phase / science target
            #    scans
            for obscan in obscans:
                if obscan.id > atmscan.id:
                    ftsysdict['snr_scan'] = obscan.id
                    break

            ftsysdict['tsys_scan'] = atmscan.id
            ftsysdict['tsys_spw'] = bestspwid
            break

        # Update the spw dictinary
        if ftsysdict:
            LOG.info('    Matched spw %d to a Tsys spw %d' % (spwid, bestspwid))
            tsysdict[spwid] = ftsysdict
        else:
            LOG.warn('    Cannot match spw %d to a Tsys spw in MS %s' % (spwid, ms.basename))

    return tsysdict

"""
Utility routine for constructing the tsys spw list and the observing
scan list from the tysdict produced by get_tsysinfo.
"""

def make_tsyslists (spwlist, tsysdict):

    """
    Input Parameters
         spwlist: The science spw list, e.g. [13, 15]
        tsysdict: The Tsys dictionary created by get_tsysinfo

    Returned values
        tsys_spwlist: The Tsys spw id list corresponding to spwlist
            scanlist: The list of snr scans for each Tsys window
    """

    tsys_spwlist = []; scan_list = []
    for spw in spwlist:
        if not tsysdict.has_key(spw):
            continue
        if not tsysdict[spw].has_key('tsys_spw'):
            continue
        tsys_spwlist.append(tsysdict[spw]['tsys_spw'])
        scan_list.append(tsysdict[spw]['snr_scan'])

    return tsys_spwlist, scan_list


"""
Get median Tsys, Trx, or Tsky temperatures as a function of spw and return
a dictionary
"""

def get_mediantemp (ms, tsys_spwlist, scan_list, antenna='',
    temptype='tsys'):

    """
    Input parameters
              ms: The pipeline measurement set object
    tsys_spwlist: The list of Tsys spw ids, e.g. [9,11,13,15]
       scan_list: The list of associated observation scan numbers, e.g. [4,8]
         antenna: The antenna selectionm '' for all antennas, or a single antenna id or name
        temptype: The temperature type 'tsys' (default), 'trx' or 'tsky'

    The output dictionary

    The median temperature dictionary keys and values 
        key: the spw id         value: The median Tsys temperature in degrees K
    """

    # Initialize
    medtempsdict = collections.OrderedDict()
    LOG.info('Estimating Tsys temperatures')

    # Temperature type must be one of 'tsys' or 'trx' or 'tsky'
    if (temptype != 'tsys' and temptype != 'trx' and temptype != 'tsky'):
        return medtempsdict

    # Get list of unique scan ids.
    uniqueScans = list(set(scan_list))

    # Determine the start and end times for each unique scan
    beginScanTimes = []; endScanTimes = []
    for scan in uniqueScans:
        reqscan = ms.get_scans(scan_id=scan)
        if not reqscan:
            LOG.warn ('Cannot find observation scan %d in MS %s' % (scan, ms.basename))
            return medtempsdict
        startTime = reqscan[0].start_time 
        endTime = reqscan[0].end_time 
        beginScanTimes.append(startTime)
        endScanTimes.append(endTime)
        LOG.debug ('scan %d start %s end %s' % (scan, startTime, endTime)) 

    # Get the syscal table meta data.
    with casatools.TableReader(os.path.join(ms.name,  'SYSCAL')) as table:

        # Get the antenna ids
        tsys_antennas = table.getcol('ANTENNA_ID')
        if (len(tsys_antennas) < 1):
            LOG.warn('The SYSCAL table is blank in MS %s' % ms.basename)
            return medtempsdict

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
            LOG.debug ('row %d start %s end %s' % (i, tstart, tend)) 

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
            'No SYSCAL table row matches for scans %s tsys spws %s in MS %s' % \
            (uniqueScans, tsys_spwlist, ms.basename))
            return medtempsdict
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

    # Loop over the spw and scan list which have the same length
    for spw, scan in zip (tsys_spwlist, scan_list):

        if (spw not in tsys_uniqueSpws):
            LOG.warn ('Tsys spw %d is not in the SYSCAL table for MS %s' % \
                (spw, ms.basename))
            return medtempsdict

        # Loop over the rows
        medians = []
        with casatools.TableReader(os.path.join(ms.name, 'SYSCAL')) as table:
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
            medtempsdict[spw] = np.median(medians)
            LOG.info ( \
                "    Median Tsys %s value for Tsys spw %2d = %.1f K" % \
                (temptype, spw, medtempsdict[spw]))
        else:
            LOG.warn ('    No Tsys data for spw %d scan %d in MS %s' % \
                (spw, scan, ms.basename))

    # Return median temperature per spw and scan.
    return medtempsdict 

"""
Internal method for determining the number of unflagged 12m and 7m
antennas.

Loop over the scans in scanlist. Compute the list of unflagged
and flagged 12m and 7m antennas for each scan. In most cases
there will be only one scan. Return the number of unflagged
12m and 7m antennas
"""

def _get_unflagged_antennas(vis, scanidlist, ants12m, ants7m,
    max_fracflagged = 0.90):

    """
    Input Parameters
                vis: The name of the MS
         scanidlist: The input scan id list, e.g. [3,4,5]
            ants12m: The list of 12m antennas
             ants7m: The list of 7m antennas
    max_fracflagged:

    Return values
        nunflagged_12mantennas: number of unflagged 12m antennas
        nunflagged_7mantennas: number of unflagged 7m antennas
    """

    # Loop over the bandpass scans
    nunflagged_12mantennas = 0; nflagged_12mantennas = 0
    nunflagged_7mantennas = 0; nflagged_7mantennas = 0

    # Execute the CASA flagdata task for the specified bandpass scans
    #     Format the id list for CASA
    #     Execute task
    scanidstr = ','.join([str(scanid) for scanid in scanidlist])
    flagdata_task = casa_tasks.flagdata(vis=vis, scan=scanidstr,
        mode='summary')
    flagdata_result = flagdata_task.execute(dry_run=False)

    # Initialize the statistics per scan
    unflagged_12mantennas = []; flagged_12mantennas = []
    unflagged_7mantennas = []; flagged_7mantennas = []

    # Add up the antennas
    antennas = flagdata_result['antenna'].keys()
    for antenna in sorted(antennas):
        points = flagdata_result['antenna'][antenna]
        fraction = points['flagged']/points['total']
        if antenna in ants12m:
            if (fraction < max_fracflagged):
                unflagged_12mantennas.append(antenna)
            else:
                flagged_12mantennas.append(antenna)
        elif antenna in ants7m:
            if (fraction < max_fracflagged):
                unflagged_7mantennas.append(antenna)
            else:
                flagged_7mantennas.append(antenna)

    # Compute the number of unflagged antennas per scan
    nunflagged_12mantennas = len(unflagged_12mantennas)
    nunflagged_7mantennas = len(unflagged_7mantennas)

    #nflagged_12mantennas = len(flagged_12mantennas)
    #nflagged_7mantennas = len(flagged_7mantennas)

    # Return the number of unflagged antennas
    return nunflagged_12mantennas, nunflagged_7mantennas


"""
Get the observing information as a function of spw id  and return a dictionary
"""

def get_obsinfo (ms, fieldnamelist, intent, spwidlist, compute_nantennas='all',
    max_fracflagged=0.90):

    """
    Input parameters
                   ms: The pipeline context ms object
        fieldnamelist: The list of field names to be selected 
               intent: The intent of the fields to be selected 
            spwidlist: The list of spw ids to be selected
    compute_nantennas: The algorithm for computing the number of unflagged antennas ('all', 'flagged') (was 'hm_nantennas')
      max_fracflagged: The maximum fraction of an antenna can be flagged

    The output observing dictionary obsdict

    The observing dictionary key and value 
        key: the spw id         value: The observing scans dictionary

    The observing scans dictionary keys and values
        key: 'snr_scans'        value: The list of snr source scans, e.g. [4,8] (was 'bandpass_scans')
        key: 'num_12mantenna'   value: The max number of 12m antennas, e.g. 32
        key: 'num_7mantenna'    value: The max number of 7m antennas, e.g. 7
        key: 'exptime'          value: The exposure time in minutes, e.g. 6.32
        key: 'integrationtime'  value: The mean integration time in minutes, e.g. 0.016
        key: 'band'             value: The ALMA receiver band, e.g. 'ALMA Band 3'
        key: 'bandcenter'       value: The receiver band center frequency in Hz, e.g. 9.6e9
        key: 'bandwidth'        value: The band width in Hz, e.g. 2.0e9 
        key: 'nchan'            value: The number of channels, e.g. 28
        key: 'chanwidths'       value: The median channel width in Hz, e.g. 7.3e7
    """

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
    prev_spwid = None; prev_scanids = []
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
        obsdict[spwid]['snr_scans'] = scanids


        # Figure out the number of 7m and 12 m antennas
        #   Note comparison of floating point numbers is tricky ...
        #   
        if compute_nantennas == 'all':
            # Use numbers from the scan with the minimum number of 
            # antennas
            n7mant = np.iinfo('i').max; n12mant = np.iinfo('i').max
            for scan in fscans:
                n7mant = min (n7mant, len ([a for a in scan.antennas \
                    if a.diameter == 7.0]))
                n12mant = min (n12mant, len ([a for a in scan.antennas \
                    if a.diameter == 12.0]))
        elif len(set(scanids).difference(set(prev_scanids))) > 0:
            # Get the lists of unique 7m and 12m antennas
            ant7m = []; ant12m = []
            for scan in fscans:
                ant7m.extend ([a.name for a in scan.antennas if a.diameter == 7.0])
                ant12m.extend([a.name for a in scan.antennas if a.diameter == 12.0])
            ant12m = list(set(ant12m))
            ant7m = list(set(ant7m))
            # Get the number of unflagged antennas
            n12mant, n7mant = _get_unflagged_antennas(ms.name, scanids,
                ant12m, ant7m, max_fracflagged=max_fracflagged)
        else:
            # Use values from previous spw
            nant7m = obsdict[prev_spwid]['num_7mantenna']
            nant12m = obsdict[prev_spwid]['num_12mantenna']

        obsdict[spwid]['num_12mantenna'] = n12mant
        obsdict[spwid]['num_7mantenna'] = n7mant

        # Retrieve total exposure time and mean integration time in minutes
        #    Add to dictionary
        exposureTime = 0.0
        meanInterval = 0.0
        for scan in fscans:
            #scanTime = float (scan.time_on_source.total_seconds()) / 60.0
            scanTime = scan.exposure_time(spw.id).total_seconds() / 60.0
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

        prev_spwid = spwid
        prev_scanids = scanids

    return obsdict


"""
Combine all the input dictionaries and output the spw dictionary
This dictionary contains all the information needed to compute the SNR
estimates.
"""

def join_dicts (spwlist, tsys_dict, flux_dict, tsystemp_dict, obs_dict):

    """
    The input parameters

    The output dictionary spw_dict

    The spw dictionary spw_dict  key and value 
        key: The spw id       value: The spw source dictionary

    The spw source dictionary keys and values
        key: 'tsys_field_name'  value: The name of the Tsys field source, e.g. '3C286'
        key: 'intent'           value: The intent of the field source, e.g. 'BANDPASS'
        key: 'snr_scan'         value: The scan associated with Tsys used to compute the SNR, e.g. 4 
        key: 'tsys_scan'        value: The Tsys scan to be used for Tsys computation, e.g. 3
        key: 'tsys_spw'         value: The Tsys spw associated with the science spw id, e.g. 13

        key: 'field_name'       value: The name of the field source, e.g. '3C286'
        key: 'flux'             value: The flux of the field source in Jy, e.g. 5.305

        key: 'median_tsys'      value: The median Tsys value in degrees K, e.g. 45.5

        key: 'snr_scans'        value: The list of snr source scans, e.g. [4,8] (was 'bandpass_scans')
        key: 'num_12mantenna'   value: The max number of 12m antennas, e.g. 32
        key: 'num_7mantenna'    value: The max number of 7m antennas, e.g. 7
        key: 'exptime'          value: The exposure time in minutes, e.g. 6.32
        key: 'integrationtime'  value: The mean integration time in minutes, e.g. 0.016
        key: 'band'             value: The ALMA receiver band, e.g. 'ALMA Band 3'
        key: 'bandcenter'       value: The receiver band center frequency in Hz, e.g. 9.6e9
        key: 'bandwidth'        value: The band width in Hz, e.g. 2.0e9 
        key: 'nchan'            value: The number of channels, e.g. 28
        key: 'chanwidths'       value: The median channel width in Hz, e.g. 7.3e7
    """

    # Initialize the spw dictionary from the Tsys dictionary
    #    Make a deep copy of this dictionary
    spw_dict = deepcopy(tsys_dict)

    # Transfer flux information to the spw dictionary.
    _transfer_fluxes (spwlist, spw_dict, flux_dict)

    # Transfer the tsys temperature information to the spw dictionary
    _transfer_temps (spwlist, spw_dict, tsystemp_dict)

    # Transfer the observing information to the spw dictionary
    _transfer_obsinfo (spwlist, spw_dict, obs_dict)

    return spw_dict

"""
Transfer flux information from the flux dictionary to the spw dictionary.
"""

def _transfer_fluxes (spwlist, spw_dict, flux_dict):
    for spw in spwlist:
        if not flux_dict.has_key(spw):
            continue
        if not spw_dict.has_key(spw):
            continue
        #if spw_dict[spw]['tsys_field_name'] != flux_dict[spw]['field_name']:
            #continue
        spw_dict[spw]['field_name'] = flux_dict[spw]['field_name']
        spw_dict[spw]['flux'] = flux_dict[spw]['flux']


"""
Transfer the tsys temp information to the spw dictionary
"""

def _transfer_temps (spwlist, spw_dict, tsystemp_dict):
    for spw in spwlist:
        if not spw_dict.has_key(spw):
            continue
        if not tsystemp_dict.has_key(spw_dict[spw]['tsys_spw']):
            continue
        spw_dict[spw]['median_tsys'] = \
            tsystemp_dict[spw_dict[spw]['tsys_spw']]

"""
Transfer the observing information to the spw dictionary
"""

def _transfer_obsinfo (spwlist, spw_dict, obs_dict):
    for spw in spwlist:
        if not spw_dict.has_key(spw):
            continue
        if not obs_dict.has_key(spw):
            continue
        spw_dict[spw]['snr_scans'] = obs_dict[spw]['snr_scans']
        spw_dict[spw]['exptime'] = obs_dict[spw]['exptime']
        spw_dict[spw]['integrationtime'] = obs_dict[spw]['integrationtime']
        spw_dict[spw]['num_7mantenna'] = obs_dict[spw]['num_7mantenna']
        spw_dict[spw]['num_12mantenna'] = obs_dict[spw]['num_12mantenna']
        spw_dict[spw]['band'] = obs_dict[spw]['band']
        spw_dict[spw]['bandcenter'] = obs_dict[spw]['bandcenter']
        spw_dict[spw]['bandwidth'] = obs_dict[spw]['bandwidth']
        spw_dict[spw]['nchan'] = obs_dict[spw]['nchan']
        spw_dict[spw]['chanwidths'] = obs_dict[spw]['chanwidths']


"""
Compute the optimal bandpass frequency solution intervals given the spw list
and the spw dictionary
"""

def compute_bpsolint(ms, spwlist, spw_dict, reqPhaseupSnr,
    minBpNintervals, reqBpSnr, minBpNchan):

    """
    The input parameters

    The output solution interval dictionary.

    The bandpass preaveraging dictionary keys abd values
        key: the spw id     value: The science spw id as an integer

    The preaveraging parameter dictionary keys abd values
        key: 'band'               value: The ALMA receiver band
        key: 'frequency_Hz'       value: The frequency of the spw
        key: 'nchan_total'        value: The total number of channels
        key: 'chanwidth_Hz'       value: The median channel width in Hz

        key: 'tsys_spw'           value: The tsys spw id as an integer
        key: 'median_tsys'        value: The median tsys value

        key: 'flux_Jy'            value: The flux of the source in Jy
        key: 'exptime_minutes'    value: The exposure time in minutes
        key: 'snr_per_channel'    value: The signal to noise per channel
        key: 'sensitivity_per_channel_mJy'    value: The sensitivity in mJy per channel

        key: 'bpsolint'           value: The frequency solint in MHz
        key: 'nchan_bpsolint'     value: The total number of solint channels
    """


    # Initialize the output solution interval dictionary
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

        # Phaseup bandpasstime solint
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
                (solint_dict[spwid]['integration_minutes'] * \
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
        LOG.info("%sspw %2d (%6.3fmin) requires phaseup solint='%0.3gsec' (%d time intervals in solution) to reach S/N=%.0f" % \
            (asterisks,
            spwid,
            solint_dict[spwid]['exptime_minutes'],
            60.0 * requiredIntegrations * solint_dict[spwid]['integration_minutes'],
            solInts,
            reqPhaseupSnr))
        solint_dict[spwid]['nphaseup_solutions'] = solInts
        if tooFewIntervals:
            LOG.warn( \
            '%s Spw %d would have less than %d time intervals in its solution in MS %s' % \
            (asterisks, spwid, minBpNintervals, ms.basename))

        # Bandpass solution info
        if requiredChannels > 1.0:
            solint_dict[spwid]['bpsolint'] = '%fMHz' % \
                (requiredChannels * solint_dict[spwid]['chanwidth_Hz'] * 1.0e-6)
        else:
            solint_dict[spwid]['bpsolint'] = '1ch'
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
        LOG.info("%sspw %2d (%4.0fMHz) requires solint='%0.3gMHz' (%d channels intervals in solution) to reach S/N=%.0f" % \
            (asterisks,
            spwid,
            solint_dict[spwid]['bandwidth']*1.0e-6,
            requiredChannels * solint_dict[spwid]['chanwidth_Hz'] * 1.0e-6,
            solChannels,
            reqBpSnr))
        solint_dict[spwid]['nbandpass_solutions'] = solChannels
        if tooFewChannels:
            LOG.warn('%s Spw %d would have less than %d channels in its solution in MS %s' % \
                (asterisks, spwid, minBpNchan, ms.basename))

    return solint_dict
