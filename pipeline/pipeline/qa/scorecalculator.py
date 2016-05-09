'''
Created on 9 Jan 2014

@author: sjw
'''
import os
import collections
import datetime
import operator

import pipeline.domain.measures as measures
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa

__all__ = ['score_polintents',                                # ALMA specific
           'score_bands',                                     # ALMA specific
           'score_bwswitching',                               # ALMA specific
           'score_tsysspwmap',                                # ALMA specific
           'score_number_antenna_offsets',                    # ALMA specific
           '_score_missing_derived_fluxes',                    # ALMA specific
           'score_derived_fluxes_snr',                        # ALMA specific
           'score_phaseup_mapping_fraction',                  # ALMA specific
           'score_missing_phaseup_snrs',                      # ALMA specific
           'score_missing_bandpass_snrs',                     # ALMA specific
           'score_poor_phaseup_solutions',                    # ALMA specific
           'score_poor_bandpass_solutions',                   # ALMA specific
           'score_missing_phase_snrs',                        # ALMA specific
           'score_poor_phase_snrs',                           # ALMA specific
           'score_flagging_view_exists',                      # ALMA specific
           'score_file_exists',
           'score_path_exists',
           'score_flags_exist',
           'score_applycmds_exist',
           'score_caltables_exist',
           'score_setjy_measurements',
           'score_missing_intents',
           'score_ephemeris_coordinates',
           'score_online_shadow_agents',
           'score_applycal_agents',
           'score_total_data_flagged',
           'score_ms_model_data_column_present',
           'score_ms_history_entries_present',
           'score_contiguous_session']

LOG = logging.get_logger(__name__)

#- utility functions ---------------------------------------------------------

def log_qa(method):
    """
    Decorator that logs QA evaluations as they return with a log level of
    INFO for scores of 1.0 and WARNING for any other level.
    """
    def f(self, *args, **kw):
        # get the size of the CASA log before task execution
        qascore = method(self, *args, **kw)
        if qascore.score == 1.0:
            LOG.info(qascore.longmsg)
        else:
            LOG.warning(qascore.longmsg)
        return qascore

    return f

# struct to hold flagging statistics
AgentStats = collections.namedtuple("AgentStats", "name flagged total")

def calc_flags_per_agent(summaries):
    stats = []
    for idx in range(0, len(summaries)):
        flagcount = int(summaries[idx]['flagged'])
        totalcount = int(summaries[idx]['total'])

        # From the second summary onwards, subtract counts from the previous 
        # one
        if idx > 0:
            flagcount = flagcount - int(summaries[idx-1]['flagged'])
        
        stat = AgentStats(name=summaries[idx]['name'],
                          flagged=flagcount,
                          total=totalcount)
        stats.append(stat)

    return stats

def linear_score(x, x1, x2, y1=0.0, y2=1.0):
    """
    Calculate the score for the given data value, assuming the
    score follows a linear gradient between the low and high values.
    
    x values will be clipped to lie within the range x1->x2
    """
    x1 = float(x1)
    x2 = float(x2)
    y1 = float(y1)
    y2 = float(y2)
    
    clipped_x = sorted([x1, x, x2])[1]
    m = (y2-y1) / (x2-x1)
    c = y1 - m*x1
    return m*clipped_x + c

def score_data_flagged_by_agents(ms, summaries, min_frac, max_frac, 
                                 agents=None):
    """
    Calculate a score for the agentflagger summaries based on the fraction of
    data flagged by certain flagging agents.

    min_frac < flagged < max_frac maps to score of 1-0
    """
    agent_stats = calc_flags_per_agent(summaries)

    if agents is None:
        agents = []
    match_all_agents = True if len(agents) is 0 else False

    # sum the number of flagged rows for the selected agents     
    frac_flagged = reduce(operator.add, 
                          [float(s.flagged)/s.total for s in agent_stats
                           if s.name in agents or match_all_agents], 0)

    score = linear_score(frac_flagged, min_frac, max_frac, 1.0, 0.0)
    percent = 100.0*frac_flagged
    longmsg = ('%0.2f%% data in %s flagged by %s flagging agents'
               '' % (percent, ms.basename, utils.commafy(agents, False)))
    shortmsg = '%0.2f%% data flagged' % percent
    
    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, 
                       vis=ms.basename)
    
#- exported scoring functions ------------------------------------------------

def score_ms_model_data_column_present(all_mses, mses_with_column):
    """
    Give a score for a group of mses based on the number with modeldata 
    columns present.
    None with modeldata - 100% with modeldata = 1.0 -> 0.5
    """
    num_with = len(mses_with_column)
    num_all = len(all_mses)
    f = float(num_with) / num_all

    if mses_with_column:
        # log a message like 'No model columns found in a.ms, b.ms or c.ms'
        basenames = [ms.basename for ms in mses_with_column]
        s = utils.commafy(basenames, quotes=False)
        longmsg = 'Model data column found in %s' % s
        shortmsg = '%s/%s have MODELDATA' % (num_with, num_all) 
    else:
        # log a message like 'Model data column was found in a.ms and b.ms'
        basenames = [ms.basename for ms in all_mses]
        s = utils.commafy(basenames, quotes=False, conjunction='or')
        longmsg = ('No model data column found in %s' % s)            
        shortmsg = 'MODELDATA empty' 

    score = linear_score(f, 0.0, 1.0, 1.0, 0.5)

    return pqa.QAScore(score, longmsg, shortmsg)


@log_qa
def score_ms_history_entries_present(all_mses, mses_with_history):
    """
    Give a score for a group of mses based on the number with history 
    entries present.
    None with history - 100% with history = 1.0 -> 0.5
    """
    num_with = len(mses_with_history)
    num_all = len(all_mses)

    if mses_with_history:
        # log a message like 'Entries were found in the HISTORY table for 
        # a.ms and b.ms'
        basenames = utils.commafy([ms.basename for ms in mses_with_history],
                                  quotes=False)
        if len(mses_with_history) is 1:
            longmsg = ('Unexpected entries were found in the HISTORY table of %s. '
                        'This measurement set may already be processed.'
                        '' % basenames)
        else:
            longmsg = ('Unexpected entries were found in the HISTORY tables of %s. '
                       'These measurement sets may already be processed.'
                        '' % basenames)                
        shortmsg = '%s/%s have HISTORY' % (num_with, num_all) 

    else:
        # log a message like 'No history entries were found in a.ms or b.ms'
        basenames = [ms.basename for ms in all_mses]
        s = utils.commafy(basenames, quotes=False, conjunction='or')
        longmsg = 'No HISTORY entries found in %s' % s
        shortmsg = 'No HISTORY entries'

    f = float(num_with) / num_all
    score = linear_score(f, 0.0, 1.0, 1.0, 0.5)

    return pqa.QAScore(score, longmsg, shortmsg)

@log_qa
def score_bwswitching(mses):
    """
    Score a MeasurementSet object based on the presence of 
    bandwidth switching observings. For bandwidth switched
    observations the TARGET and PHASE spws are different.
    """

    score = 1.0
    num_mses = len(mses)
    all_ok = True
    complaints = []

    # analyse each MS
    for ms in mses:

        # Get the science spws
        scispws = set([spw.id for spw in ms.get_spectral_windows(science_windows_only=True)])

        # Get phase calibrator science spw ids
        phasespws = []
        for scan in ms.get_scans(scan_intent='PHASE'):
            phasespws.extend([spw.id for spw in scan.spws])
        phasespws = set(phasespws).intersection(scispws)

        # Get science target science spw ids
        targetspws = []
        for scan in ms.get_scans(scan_intent='TARGET'):
            targetspws.extend([spw.id for spw in scan.spws])
        targetspws = set(targetspws).intersection(scispws)

        # Determine the difference between the two
        nophasecals = targetspws.difference(phasespws)
        if len(nophasecals) == 0:
            continue

        # Score the difference
        all_ok = False
        for _ in nophasecals:
            score += (-1.0 / num_mses / len(nophasecals))
        longmsg = ('%s contains no phase calibrations for target spws %s'
            '' % (ms.basename, utils.commafy(nophasecals, False)))
        complaints.append(longmsg)

    if all_ok:
        longmsg = ('Phase calibrations found for all target spws in %s.' % (
                   utils.commafy([ms.basename for ms in mses], False)))
        shortmsg = 'Phase calibrations found for all target spws' 
    else:
        longmsg = '%s.' % utils.commafy(complaints, False)
        shortmsg = 'No phase calibrations found for target spws %s' % list(nophasecals)
        
    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)

@log_qa
def score_bands(mses):
    """
    Score a MeasurementSet object based on the presence of 
    ALMA bands with calibration issues.
    """

    # ALMA receiver bands. Warnings will be raised for any 
    # measurement sets containing the following bands.
    score = 1.0
    score_map = {'8'  : -1.0,
                 '9'  : -1.0}

    unsupported = set(score_map.keys())

    num_mses = len(mses)
    all_ok = True
    complaints = []

    # analyse each MS
    for ms in mses:
        msbands = []
        for spw in ms.get_spectral_windows(science_windows_only=True):
            # This does not work for old data
            #match = re.match(r'ALMA_RB_(?P<band>\d+)', spw.name)
            # Get rid of the leading 0 in the band number
            #bandnum = str(int(match.groupdict()['band']))
            bandnum = spw.band.split(' ')[2]
            msbands.append(bandnum)
        msbands = set(msbands)
        overlap = unsupported.intersection(msbands)
        if not overlap:
            continue
        all_ok = False
        for m in overlap:
            score += (score_map[m] / num_mses)
        longmsg = ('%s contains band %s data'
            '' % (ms.basename, utils.commafy(overlap, False)))
        complaints.append(longmsg)

    if all_ok:
        longmsg = ('No high frequency %s band data were found in %s.' % (list(unsupported),
                   utils.commafy([ms.basename for ms in mses], False)))
        shortmsg = 'No high frequency band data found' 
    else:
        longmsg = '%s.' % utils.commafy(complaints, False)
        shortmsg = 'High frequency band data found' 
        
    # Make score linear
    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)

@log_qa
def score_polintents(mses):
    """
    Score a MeasurementSet object based on the presence of 
    polarization intents.
    """

    # Polarization intents. Warnings will be raised for any 
    # measurement sets containing these intents. Ignore the
    # array type for now.
    score = 1.0
    score_map = {'POLARIZATION'  : -1.0,
                 'POLANGLE'      : -1.0,
                 'POLLEAKAGE'    : -1.0}

    unsupported = set(score_map.keys())

    num_mses = len(mses)
    all_ok = True
    complaints = []

    # analyse each MS
    for ms in mses:
        # are these intents present in the ms
        overlap = unsupported.intersection(ms.intents)
        if not overlap:
            continue
        all_ok = False
        for m in overlap:
            score += (score_map[m] / num_mses)

        longmsg = ('%s contains %s polarization calibration intents'
            '' % (ms.basename, utils.commafy(overlap, False)))
        complaints.append(longmsg)

    if all_ok:
        longmsg = ('No polarization calibration intents were found in '
                   '%s.' % utils.commafy([ms.basename for ms in mses], False))
        shortmsg = 'No polarization calibrators found'
    else:
        longmsg = '%s.' % utils.commafy(complaints, False)
        shortmsg = 'Polarization calibrators found'
        
    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)

@log_qa
def score_missing_intents(mses, array_type='ALMA_12m'):
    """
    Score a MeasurementSet object based on the presence of certain
    observing intents.
    """
    # Required calibration intents. Warnings will be raised for any 
    # measurement sets missing these intents 
    score = 1.0
    if array_type == 'ALMA_TP':
        score_map = {'ATMOSPHERE' : -1.0}
    else:
        score_map = {'PHASE'     : -1.0,
                     'BANDPASS'  : -0.1,
                     'AMPLITUDE' : -0.1}

    required = set(score_map.keys())

    num_mses = len(mses)
    all_ok = True
    complaints = []

    # analyse each MS
    for ms in mses:
        # do we have the necessary calibrators?
        if not required.issubset(ms.intents):
            all_ok = False
            missing = required.difference(ms.intents)
            for m in missing:
                score += (score_map[m] / num_mses)

            longmsg = ('%s is missing %s calibration intents'
                       '' % (ms.basename, utils.commafy(missing, False)))
            complaints.append(longmsg)
            
    if all_ok:
        longmsg = ('All required calibration intents were found in '
                   '%s.' % utils.commafy([ms.basename for ms in mses], False))
        shortmsg = 'All calibrators found'
    else:
        longmsg = '%s.' % utils.commafy(complaints, False)
        shortmsg = 'Calibrators missing'
        
    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)

@log_qa
def score_ephemeris_coordinates(mses):

    """
    Score a MeasurementSet object based on the presence of possible
    ephemeris coordinates.
    """

    score = 1.0

    num_mses = len(mses)
    all_ok = True
    complaints = []
    zerodirection = casatools.measures.direction('j2000', '0.0deg', '0.0deg')

    # analyse each MS
    for ms in mses:

    # Examine each source
        for source in ms.sources:
            if source.ra == casatools.quanta.formxxx(zerodirection['m0'], format='hms', prec=3) or \
                source.dec == casatools.quanta.formxxx(zerodirection['m1'], format='dms', prec=2):
                all_ok = False
                score += (-1.0 / num_mses)
                longmsg =  ('Suspicious source coordinates for  %s in %s'
                    '' % (source.name, ms.basename))
                complaints.append(longmsg)

    if all_ok:
        longmsg = ('All source coordinates OK in '
                   '%s.' % utils.commafy([ms.basename for ms in mses], False))
        shortmsg = 'All source coordinates OK'
    else:
        longmsg = '%s.' % utils.commafy(complaints, False)
        shortmsg = 'Suspicious source coordinates'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)

@log_qa
def score_online_shadow_agents(ms, summaries):
    """
    Get a score for the fraction of data flagged by online and shadow agents.

    0 < score < 1 === 60% < frac_flagged < 5%
    """
    return score_data_flagged_by_agents(ms, summaries, 0.05, 0.6, 
                                        ['online', 'shadow', 'qa0', 'before', 'template'])

@log_qa
def score_applycal_agents(ms, summaries):
    """
    Get a score for the fraction of data flagged by online and shadow agents.

    0 < score < 1 === 60% < frac_flagged < 5%
    """
    return score_data_flagged_by_agents(ms, summaries, 0.05, 0.6, ['applycal'])

@log_qa
def score_flagging_view_exists(filename, view):
    """
    Assign a score of zero if the flagging view cannot be computed
    """

    if not view:
        score = 0.0
        longmsg = 'No flagging views for %s' % (filename)
        shortmsg = 'No flagging views'
    else:
        score = 1.0
        longmsg = 'Flagging views exist for %s' % (filename)
        shortmsg = 'Flagging views exist'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=filename)


@log_qa
def score_total_data_flagged(filename, summaries):
    """
    Calculate a score for the flagging task based on the total fraction of
    data flagged.
    
    0%-5% flagged   -> 1
    5%-50% flagged  -> 0.5
    50-100% flagged -> 0
    """    
    agent_stats = calc_flags_per_agent(summaries)

    # sum the number of flagged rows for the selected agents     
    frac_flagged = reduce(operator.add, 
                          [float(s.flagged)/s.total for s in agent_stats], 0)

    if frac_flagged > 0.5:
        score = 0
    else:
        score = linear_score(frac_flagged, 0.05, 0.5, 1.0, 0.5)

    percent = 100.0 * frac_flagged
    longmsg = '%0.2f%% of data in %s was flagged' % (percent, filename)
    shortmsg = '%0.2f%% data flagged' % percent
    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=os.path.basename(filename))

@log_qa
def score_fraction_newly_flagged(filename, summaries, vis):
    """
    Calculate a score for the flagging task based on the fraction of
    data newly flagged.
    
    0%-5% flagged   -> 1
    5%-50% flagged  -> 0.5
    50-100% flagged -> 0
    """    
    agent_stats = calc_flags_per_agent(summaries)

    # sum the number of flagged rows for the selected agents     
    frac_flagged = reduce(operator.add, 
                          [float(s.flagged)/s.total for s in agent_stats[1:]], 0)
        
    if frac_flagged > 0.5:
        score = 0
    else:
        score = linear_score(frac_flagged, 0.05, 0.5, 1.0, 0.5)

    percent = 100.0 * frac_flagged
    longmsg = '%0.2f%% of data in %s was newly flagged' % (percent, filename)
    shortmsg = '%0.2f%% data flagged' % percent

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=os.path.basename(vis))

@log_qa
def linear_score_fraction_newly_flagged(filename, summaries, vis):
    """
    Calculate a score for the flagging task based on the fraction of
    data newly flagged.
    
    fraction flagged   -> score
    """    
    agent_stats = calc_flags_per_agent(summaries)

    # sum the number of flagged rows for the selected agents     
    frac_flagged = reduce(operator.add, 
                          [float(s.flagged)/s.total for s in agent_stats[1:]], 0)

    score = 1.0 - frac_flagged        

    percent = 100.0 * frac_flagged
    longmsg = '%0.2f%% of data in %s was newly flagged' % (percent, filename)
    shortmsg = '%0.2f%% data flagged' % percent
    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=os.path.basename(vis))

@log_qa
def score_contiguous_session(mses, tolerance=datetime.timedelta(hours=1)):
    """
    Check whether measurement sets are contiguous in time. 
    """
    # only need to check when given multiple measurement sets
    if len(mses) < 2:
        return pqa.QAScore(1.0,
                           longmsg='%s forms one continuous observing session.' % mses[0].basename,
                           shortmsg='Unbroken observing session',
                           vis=mses[0].basename)

    # reorder MSes by start time
    by_start = sorted(mses, 
                      key=lambda m : utils.get_epoch_as_datetime(m.start_time)) 

    # create an interval for each one, including our tolerance    
    intervals = []    
    for ms in by_start:
        start = utils.get_epoch_as_datetime(ms.start_time)
        end = utils.get_epoch_as_datetime(ms.end_time)
        interval = measures.TimeInterval(start - tolerance, end + tolerance)
        intervals.append(interval)

    # check whether the intervals overlap
    bad_mses = []
    for i, (interval1, interval2) in enumerate(zip(intervals[0:-1], 
                                                   intervals[1:])):
        if not interval1.overlaps(interval2):
            bad_mses.append(utils.commafy([by_start[i].basename,
                                           by_start[i+1].basename]))

    if bad_mses:
        basenames = utils.commafy(bad_mses, False)
        longmsg = ('Measurement sets %s are not contiguous. They may be '
                   'miscalibrated as a result.' % basenames)
        shortmsg = 'Gaps between observations'
        score = 0.5
    else:
        basenames = utils.commafy([ms.basename for ms in mses])
        longmsg = ('Measurement sets %s are contiguous.' % basenames)
        shortmsg = 'Unbroken observing session'
        score = 1.0

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)


@log_qa
def score_wvrgcal(ms_name, wvr_score):
    if wvr_score < 1.0:
        score = 0
    else:
        score = linear_score(wvr_score, 1.0, 2.0, 0.5, 1.0)

    longmsg = 'RMS improvement was %0.2f for %s' % (wvr_score, ms_name)
    shortmsg = '%0.2fx improvement' % wvr_score
    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, 
                       vis=os.path.basename(ms_name))

@log_qa
def score_sdtotal_data_flagged(name, ant, spw, pol, frac_flagged):
    """
    Calculate a score for the flagging task based on the total fraction of
    data flagged.
    
    0%-5% flagged   -> 1
    5%-50% flagged  -> 0.5
    50-100% flagged -> 0
    """
    if frac_flagged > 0.5:
        score = 0
    else:
        score = linear_score(frac_flagged, 0.05, 0.5, 1.0, 0.5)
    
    percent = 100.0 * frac_flagged
    longmsg = '%0.2f%% of data in %s (Ant=%s, SPW=%d, Pol=%d) was flagged' % (percent, name, ant, spw, pol)
    shortmsg = '%0.2f%% data flagged' % percent
    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, 
                       vis=os.path.basename(name))

@log_qa
def score_tsysspwmap (ms, unmappedspws):
    '''
    Score is equal to the fraction of unmapped windows
    '''

    if len(unmappedspws) <= 0:
        score = 1.0
        longmsg = 'Tsys spw map is complete for %s ' % ms.basename
        shortmsg = 'Tsys spw map is complete'
    else:
        nscispws = len([spw.id for spw in ms.get_spectral_windows(science_windows_only=True)])
        if nscispws <= 0:
            score = 0.0
        else:
            score = float(nscispws - len (unmappedspws)) / float(nscispws)
        longmsg = 'Tsys spw map is incomplete for %s science window%s ' % (ms.basename, utils.commafy(unmappedspws, False, 's'))
        shortmsg = 'Tsys spw map is incomplete'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=ms.basename)

@log_qa
def score_setjy_measurements (ms, reqfields, reqintents, reqspws, measurements):
    '''
    Score is equal to the ratio of the number of actual flux
    measurements to expected number of flux measurements
    '''

    # Expected fields
    scifields = set ([field for field in ms.get_fields (reqfields, intent=reqintents)])

    # Expected science windows
    scispws = set([spw.id for spw in ms.get_spectral_windows(reqspws, science_windows_only=True)])

    # Loop over the expected fields
    nexpected = 0
    for scifield in scifields:
        validspws = set([spw.id for spw in scifield.valid_spws])
        nexpected = nexpected + len(validspws.intersection(scispws))

    # Loop over the measurements
    nmeasured = 0
    for key, value in measurements.iteritems():
        # Loop over the flux measurements
        for flux in value:
            nmeasured = nmeasured + 1

    # Compute score
    if nexpected == 0:
        score = 0.0
        longmsg = 'No flux calibrators for %s ' % ms.basename
        shortmsg = 'No flux calibrators'
    elif nmeasured == 0:
        score = 0.0
        longmsg = 'No flux measurements for %s ' % ms.basename
        shortmsg = 'No flux measurements'
    elif nexpected == nmeasured:
        score = 1.0
        longmsg = 'All expected flux calibrator measurements present for %s ' % ms.basename
        shortmsg = 'All expected flux calibrator measurements present'
    elif nmeasured < nexpected:
        score = float(nmeasured) / float(nexpected)
        longmsg = 'Missing flux calibrator measurements for %s %d/%d ' % (ms.basename, nmeasured, nexpected)
        shortmsg = 'Missing flux calibrator measurements'
    else:
        score = 0.0
        longmsg = 'Too many flux calibrator measurements for %s %d/%d' % (ms.basename, nmeasured, nexpected)
        shortmsg = 'Too many flux measurements'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=ms.basename)

@log_qa
def score_number_antenna_offsets (ms, antenna, offsets):
    '''
    Score is equal to the ratio number of antennas without corrections
    to the number of antennas with them. Make this more sophisticated
    later
    '''

    nant_with_offsets = len(offsets) / 3
    nant = len([ant.id for ant in ms.get_antenna()])

    if nant_with_offsets == 0:
        score = 1.0
        longmsg = 'No antenna position offsets for %s ' % ms.basename
        shortmsg = 'No antenna postion offsets'
    else:
        score = float(nant - nant_with_offsets) / float (nant)
        longmsg = '%d nonzero antenna position offsets for %s ' % (nant_with_offsets, ms.basename)
        shortmsg = 'Nonzero antenna postion offsets'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=ms.basename)

@log_qa
def score_missing_derived_fluxes (ms, reqfields, reqintents, measurements):

    '''
    Score is equal to the ratio of actual flux
    measurement to expected flux measurements
    '''

    # Expected fields
    scifields = set ([field for field in ms.get_fields (reqfields, intent=reqintents)])

    # Expected science windows
    scispws = set([spw.id for spw in ms.get_spectral_windows(science_windows_only=True)])

    # Loop over the expected fields
    nexpected = 0
    for scifield in scifields:
        validspws = set([spw.id for spw in scifield.valid_spws])
        nexpected = nexpected + len(validspws.intersection(scispws))

    # Loop over measurements
    nmeasured = 0
    for key, value in measurements.iteritems():
        # Loop over the flux measurements
        for flux in value:
            fluxjy = getattr (flux, 'I').to_units(measures.FluxDensityUnits.JANSKY)
            uncjy = getattr (flux.uncertainty, 'I').to_units(measures.FluxDensityUnits.JANSKY)
            if fluxjy <= 0.0 or uncjy <= 0.0: 
                 continue
            nmeasured = nmeasured + 1

    # Compute score
    if nexpected == 0:
        score = 0.0
        longmsg = 'No secondary calibrators for %s ' % ms.basename
        shortmsg = 'No secondary calibrators'
    elif nmeasured == 0:
        score = 0.0
        longmsg = 'No derived fluxes for %s ' % ms.basename
        shortmsg = 'No derived fluxes'
    elif nexpected == nmeasured:
        score = 1.0
        longmsg = 'All expected derived fluxes present for %s ' % ms.basename
        shortmsg = 'All expected derived fluxes present'
    elif nmeasured < nexpected:
        score = float(nmeasured) / float(nexpected)
        longmsg = 'Missing derived fluxes for %s %d/%d' % (ms.basename, nmeasured, nexpected)
        shortmsg = 'Missing derived fluxes'
    else:
        score = 0.0
        longmsg = 'Extra derived fluxes for %s %d/%d' % (ms.basename, nmeasured, nexpected)
        shortmsg = 'Extra derived fluxes'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=ms.basename)

@log_qa
def score_phaseup_mapping_fraction(ms, reqfields, reqintents, phaseup_spwmap):
    '''
    Compute the fraction of science spws that have not been
    mapped to other probably  wider windows.

    Note that reqfields and reqintents are no longer used. Remove at some point
    '''

    if not phaseup_spwmap:
        score = 1.0
        longmsg = 'No mapped science spws for %s ' % ms.basename
        shortmsg = 'No mapped science spws'
    else:
        # Expected fields
        #scifields = set ([field for field in ms.get_fields (reqfields, intent=reqintents)])

        # Expected science windows
        scispws = set([spw.id for spw in ms.get_spectral_windows(science_windows_only=True)])
        nexpected = len(scispws)

        # Loop over the expected fields
        #nexpected = 0
        #for scifield in scifields:
            #validspws = set([spw.id for spw in scifield.valid_spws])
            #nexpected = nexpected + len(validspws.intersection(scispws))

        nunmapped = 0
        for spwid in scispws:
            if spwid == phaseup_spwmap[spwid]: 
                nunmapped = nunmapped + 1
        
        if nunmapped >= nexpected:
            score = 1.0
            longmsg = 'No mapped science spws for %s ' % ms.basename
            shortmsg = 'No mapped science spws'
        else:
            score =  float(nunmapped) / float(nexpected) 
            longmsg = 'There are %d mapped science spws for %s ' % (nexpected - nunmapped, ms.basename)
            shortmsg = 'There are mapped science spws'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=ms.basename)

@log_qa
def score_missing_phaseup_snrs(ms, spwids, phsolints):

    '''
    Score is the fraction of spws with phaseup SNR estimates
    '''

    # Compute the number of expected and missing SNR measurements
    nexpected = len(spwids)
    missing_spws = []
    for i in range (len(spwids)):
        if not phsolints[i]:
            missing_spws.append(spwid[i])
    nmissing = len(missing_spws) 

    if nexpected <= 0:
        score = 0.0
        longmsg = 'No phaseup SNR estimates for %s ' % ms.basename
        shortmsg = 'No phaseup SNR estimates'
    elif nmissing <= 0:
        score = 1.0
        longmsg = 'No missing phaseup SNR estimates for %s ' % ms.basename
        shortmsg = 'No missing phaseup SNR estimates'
    else:
        score = float (nexpected - nmissing) / nexpected
        longmsg = 'Missing phaseup SNR estimates for spws %s in %s ' % \
            (missing_spws, ms.basename)
        shortmsg = 'Missing phaseup SNR estimates'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=ms.basename)

@log_qa
def score_poor_phaseup_solutions(ms, spwids, nphsolutions, min_nsolutions):
    '''
    Score is the fraction of spws with poor phaseup solutions
    '''

    # Compute the number of expected and poor SNR measurements
    nexpected = len(spwids)
    poor_spws = []
    for i in range (len(spwids)):
        if not nphsolutions[i]:
            poor_spws.append(spwids[i])
        elif nphsolutions[i] < min_nsolutions:
            poor_spws.append(spwids[i])
    npoor = len(poor_spws) 

    if nexpected <= 0:
        score = 0.0
        longmsg = 'No phaseup solutions for %s ' % ms.basename
        shortmsg = 'No phaseup solutions'
    elif npoor <= 0:
        score = 1.0
        longmsg = 'No poorly determined phaseup solutions for %s ' % ms.basename
        shortmsg = 'No poorly determined phaseup solutions'
    else:
        score = float (nexpected - npoor) / nexpected
        longmsg = 'Poorly determined phaseup solutions for spws %s in %s ' % \
            (poor_spws, ms.basename)
        shortmsg = 'Poorly determined phaseup solutions'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=ms.basename)

@log_qa
def score_missing_bandpass_snrs(ms, spwids, bpsolints):

    '''
    Score is the fraction of spws with bandpass SNR estimates
    '''

    # Compute the number of expected and missing SNR measurements
    nexpected = len(spwids)
    missing_spws = []
    for i in range (len(spwids)):
        if not bpsolints[i]:
            missing_spws.append(spwids[i])
    nmissing = len(missing_spws) 

    if nexpected <= 0:
        score = 0.0
        longmsg = 'No bandpass SNR estimates for %s ' % ms.basename
        shortmsg = 'No bandpass SNR estimates'
    elif nmissing <= 0:
        score = 1.0
        longmsg = 'No missing bandpass SNR estimates for %s ' % ms.basename
        shortmsg = 'No missing bandpass SNR estimates'
    else:
        score = float (nexpected - nmissing) / nexpected
        longmsg = 'Missing bandpass SNR estimates for spws %s in%s ' % \
            (missing_spws, ms.basename)
        shortmsg = 'Missing bandpass SNR estimates'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=ms.basename)

@log_qa
def score_poor_bandpass_solutions(ms, spwids, nbpsolutions, min_nsolutions):

    '''
    Score is the fraction of spws with poor bandpass solutions
    '''

    # Compute the number of expected and poor solutions
    nexpected = len(spwids)
    poor_spws = []
    for i in range (len(spwids)):
        if not nbpsolutions[i]:
            poor_spws.append(spwids[i])
        elif nbpsolutions[i] < min_nsolutions:
            poor_spws.append(spwids[i])
    npoor = len(poor_spws) 

    if nexpected <= 0:
        score = 0.0
        longmsg = 'No bandpass solutions for %s ' % \
            ms.basename
        shortmsg = 'No bandpass solutions'
    elif npoor <= 0:
        score = 1.0
        longmsg = 'No poorly determined bandpass solutions for %s ' % \
            ms.basename
        shortmsg = 'No poorly determined bandpass solutions'
    else:
        score = float (nexpected - npoor) / nexpected
        longmsg = 'Poorly determined bandpass solutions for spws %s in %s ' % \
            (poor_spws, ms.basename)
        shortmsg = 'Poorly determined bandpass solutions'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=ms.basename)

@log_qa
def score_missing_phase_snrs(ms, spwids, snrs):

    '''
    Score is the fraction of spws with SNR estimates
    '''

    # Compute the number of expected and missing SNR measurements
    nexpected = len(spwids)
    missing_spws = []
    for i in range (len(spwids)):
        if not snrs[i]:
            missing_spws.append(spwid[i])
    nmissing = len(missing_spws) 

    if nexpected <= 0:
        score = 0.0
        longmsg = 'No gaincal SNR estimates for %s ' % ms.basename
        shortmsg = 'No gaincal SNR estimates'
    elif nmissing <= 0:
        score = 1.0
        longmsg = 'No missing gaincal SNR estimates for %s ' % ms.basename
        shortmsg = 'No missing gaincal SNR estimates'
    else:
        score = float (nexpected - nmissing) / nexpected
        longmsg = 'Missing gaincal SNR estimates for spws %s in %s ' % \
            (missing_spws, ms.basename)
        shortmsg = 'Missing gaincal SNR estimates'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=ms.basename)

@log_qa
def score_poor_phase_snrs(ms, spwids, minsnr, snrs):

    '''
    Score is the fraction of spws with poor snr estimates
    '''

    # Compute the number of expected and poor solutions
    nexpected = len(spwids)
    poor_spws = []
    for i in range (len(spwids)):
        if not snrs[i]:
            poor_spws.append(spwids[i])
        elif snrs[i] < minsnr:
            poor_spws.append(spwids[i])
    npoor = len(poor_spws) 

    if nexpected <= 0:
        score = 0.0
        longmsg = 'No gaincal SNR estimates for %s ' % \
            ms.basename
        shortmsg = 'No gaincal SNR estimates'
    elif npoor <= 0:
        score = 1.0
        longmsg = 'No low gaincal SNR estimates for %s ' % \
            ms.basename
        shortmsg = 'No low gaincal SNR estimates'
    else:
        score = float (nexpected - npoor) / nexpected
        longmsg = 'Low gaincal SNR estimates for spws %s in %s ' % \
            (poor_spws, ms.basename)
        shortmsg = 'Low gaincal SNR estimates'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=ms.basename)


@log_qa
def score_derived_fluxes_snr(ms, measurements):
    '''
    Score the SNR of the derived flux measurements.
        1.0 if SNR > 20.0
        0.0 if SNR < 5.0
        linear scale between 0.0 and 1.0 in between
    '''

    # Loop over measurements
    nmeasured = 0
    score = 0.0
    minscore = 1.0
    for _, value in measurements.iteritems():
        # Loop over the flux measurements
        for flux in value:
            fluxjy = flux.I.to_units(measures.FluxDensityUnits.JANSKY)
            uncjy = flux.uncertainty.I.to_units(measures.FluxDensityUnits.JANSKY)
            if fluxjy <= 0.0 or uncjy <= 0.0: 
                continue
            snr = fluxjy / uncjy
            nmeasured = nmeasured + 1
            score1 = linear_score (float(snr), 5.0, 20.0, 0.0, 1.0)
            minscore = min (minscore, score1)
            score = score + score1
    if nmeasured > 0:
        score = score / nmeasured

    if nmeasured == 0:
        score = 0.0
        longmsg = 'No derived fluxes for %s ' % ms.basename
        shortmsg = 'No derived fluxes'
    elif minscore >= 1.0:
        score = 1.0
        longmsg = 'No low SNR derived fluxes for %s ' % ms.basename
        shortmsg = 'No low SNR derived fluxes'
    else:
        longmsg = 'Low SNR derived fluxes for %s ' % ms.basename
        shortmsg = 'Low SNR derived fluxes'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg,
                       vis=ms.basename)

@log_qa
def score_path_exists(mspath, path, pathtype):
    '''
    Score the existence of the path
        1.0 if it exist
        0.0 if it does not
    '''

    if os.path.exists(path):
        score = 1.0
        longmsg = 'The %s file %s for %s was created' % (pathtype, os.path.basename(path),
	    os.path.basename(mspath))
        shortmsg = 'The %s file was created' % (pathtype)
    else:
        score = 0.0
        longmsg = 'The %s file %s for %s was not created' % (pathtype,
	    os.path.basename(path), os.path.basename(mspath))
        shortmsg = 'The %s file was not created' % (pathtype)

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)

@log_qa
def score_file_exists(filedir, filename, filetype):
    '''
    Score the existence of a products file
        1.0 if it exists
        0.0 if it does not
    '''

    if filename is None:
        score = 1.0
        longmsg = 'The %s file is undefined' % (filetype)
        shortmsg = 'The %s file is undefined' % (filetype)
        return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)

    file = os.path.join(filedir, os.path.basename(filename)) 
    if os.path.exists(file):
        score = 1.0
        longmsg = 'The %s file has been exported' % (filetype)
        shortmsg = 'The %s file has been exported' % (filetype)
    else:
        score = 0.0
        longmsg = 'The %s file %s does not exist' % (filetype, os.path.basename(filename))
        shortmsg = 'The %s file does not exist' % (filetype)

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)

@log_qa
def score_flags_exist(filedir, visdict):
    '''
    Score the existence of the flagging products files 
        1.0 if they all exist
        n / nexpected if some of them exist
        0.0 if none exist
    '''

    nexpected = len(visdict)
    nfiles = 0; missing=[]
    for visname in visdict:
        file = os.path.join(filedir, os.path.basename(visdict[visname][0])) 
        if os.path.exists(file):
            nfiles = nfiles + 1
        else:
            missing.append(os.path.basename(visdict[visname][0]))

    if nfiles <= 0:
        score = 0.0
        longmsg = 'Final flag version files %s are missing' % (','.join(missing))
        shortmsg = 'Missing final flags version files'
    elif nfiles < nexpected:
        score = float (nfiles) / float (nexpected)
        longmsg = 'Final flag version files %s are missing' % (','.join(missing))
        shortmsg = 'Missing final flags version files'
    else:
        score = 1.0
        longmsg = 'No missing final flag version files'
        shortmsg = 'No missing final flags version files'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)

@log_qa
def score_applycmds_exist(filedir, visdict):
    '''
    Score the existence of the apply commands products files
        1.0 if they all exist
        n / nexpected if some of them exist
        0.0 if none exist
    '''

    nexpected = len(visdict)
    nfiles = 0; missing=[]
    for visname in visdict:
        file = os.path.join(filedir, os.path.basename(visdict[visname][1])) 
        if os.path.exists(file):
            nfiles = nfiles + 1
        else:
            missing.append(os.path.basename(visdict[visname][1]))

    if nfiles <= 0:
        score = 0.0
        longmsg = 'Final apply commands files %s are missing' % (','.join(missing))
        shortmsg = 'Missing final apply commands files'
    elif nfiles < nexpected:
        score = float (nfiles) / float (nexpected)
        longmsg = 'Final apply commands files %s are missing' % (','.join(missing))
        shortmsg = 'Missing final apply commands files'
    else:
        score = 1.0
        longmsg = 'No missing final apply commands files'
        shortmsg = 'No missing final apply commands files'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)

@log_qa
def score_caltables_exist(filedir, sessiondict):
    '''
    Score the existence of the caltables products files
        1.0 if theu all exist
        n / nexpected if some of them exist
        0.0 if none exist
    '''
    nexpected = len(sessiondict)
    nfiles = 0; missing=[]
    for sessionname in sessiondict:
        file = os.path.join(filedir, os.path.basename(sessiondict[sessionname][1])) 
        if os.path.exists(file):
            nfiles = nfiles + 1
        else:
            missing.append(os.path.basename(sessiondict[sessionname][1]))

    if nfiles <= 0:
        score = 0.0
        longmsg = 'Caltables files %s are missing' % (','.join(missing))
        shortmsg = 'Missing caltables files'
    elif nfiles < nexpected:
        score = float (nfiles) / float (nexpected)
        longmsg = 'Caltables files %s are missing' % (','.join(missing))
        shortmsg = 'Missing caltables files'
    else:
        score = 1.0
        longmsg = 'No missing caltables files'
        shortmsg = 'No missing caltables files'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)

@log_qa
def score_sd_line_detection(group_id_list, spw_id_list, lines_list):
    detected_spw = []
    detected_group = []
    for group_id, spw_id, lines in zip(group_id_list, spw_id_list, lines_list):
        if any([l[2] for l in lines]):
            LOG.trace('detected lines exist at group_id %s spw_id %s'%(group_id, spw_id))
            unique_spw_id = set(spw_id)
            if len(unique_spw_id) == 1:
                detected_spw.append(unique_spw_id.pop())
            else:
                detected_spw.append(-1)
            detected_group.append(group_id)
    if len(detected_spw) == 0:
        score = 0.0
        longmsg = 'No spectral lines are detected'
        shortmsg = 'No spectral lines are detected'
    else:
        score = 1.0
        if detected_spw.count(-1) == 0:
            longmsg = 'Spectral lines are detected at Spws %s'%(', '.join(map(str,detected_spw)))
        else:
            longmsg = 'Spectral lines are detected at ReductionGroups %s'%(','.join(map(str,detected_group)))
        shortmsg = 'Spectral lines are detected'
    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg)
