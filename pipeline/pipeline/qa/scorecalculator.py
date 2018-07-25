"""
Created on 9 Jan 2014

@author: sjw
"""
import collections
import datetime
import math
import operator
import os

import numpy as np

import pipeline.domain.measures as measures
import pipeline.infrastructure.basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.renderer.rendererutils as rutils
import pipeline.infrastructure.utils as utils
import pipeline.qa.checksource as checksource

__all__ = ['score_polintents',                                # ALMA specific
           'score_bands',                                     # ALMA specific
           'score_bwswitching',                               # ALMA specific
           'score_tsysspwmap',                                # ALMA specific
           'score_number_antenna_offsets',                    # ALMA specific
           'score_missing_derived_fluxes',                    # ALMA specific
           'score_derived_fluxes_snr',                        # ALMA specific
           'score_phaseup_mapping_fraction',                  # ALMA specific
           'score_refspw_mapping_fraction',                   # ALMA specific
           'score_missing_phaseup_snrs',                      # ALMA specific
           'score_missing_bandpass_snrs',                     # ALMA specific
           'score_poor_phaseup_solutions',                    # ALMA specific
           'score_poor_bandpass_solutions',                   # ALMA specific
           'score_missing_phase_snrs',                        # ALMA specific
           'score_poor_phase_snrs',                           # ALMA specific
           'score_flagging_view_exists',                      # ALMA specific
           'score_checksources',                              # ALMA specific
           'score_gfluxscale_k_spw',                          # ALMA specific
           'score_file_exists',
           'score_path_exists',
           'score_flags_exist',
           'score_mses_exist',
           'score_applycmds_exist',
           'score_caltables_exist',
           'score_setjy_measurements',
           'score_missing_intents',
           'score_ephemeris_coordinates',
           'score_online_shadow_template_agents',
           'score_applycal_agents',
           'score_vla_agents',
           'score_total_data_flagged',
           'score_total_data_flagged_vla',
           'score_ms_model_data_column_present',
           'score_ms_history_entries_present',
           'score_contiguous_session',
           'score_multiply']

LOG = logging.get_logger(__name__)


# - utility functions --------------------------------------------------------------------------------------------------

def log_qa(method):
    """
    Decorator that logs QA evaluations as they return with a log level of
    INFO for scores between perfect and 'slightly suboptimal' scores and
    WARNING for any other level. These messages are meant for pipeline runs
    without a weblog output.
    """
    def f(self, *args, **kw):
        # get the size of the CASA log before task execution
        qascore = method(self, *args, **kw)
        if pipeline.infrastructure.basetask.DISABLE_WEBLOG:
            if isinstance(qascore, tuple):
                _qascore = qascore[0]
            else:
                _qascore = qascore
            if _qascore.score >= rutils.SCORE_THRESHOLD_SUBOPTIMAL:
                LOG.info(_qascore.longmsg)
            else:
                LOG.warning(_qascore.longmsg)
        return qascore

    return f


# struct to hold flagging statistics
AgentStats = collections.namedtuple("AgentStats", "name flagged total")


def calc_flags_per_agent(summaries, scanids=None):
    """
    Calculate flagging statistics per agents. If scanids are provided,
    restrict statistics to just those scans.
    """
    stats = []
    flagsum = 0

    # Go through summary for each agent.
    for idx, summary in enumerate(summaries):
        if scanids:
            # Add up flagged and total for specified scans.
            flagcount = 0
            totalcount = 0
            for scanid in scanids:
                if scanid in summary['scan']:
                    flagcount += int(summary['scan'][scanid]['flagged'])
                    totalcount += int(summary['scan'][scanid]['total'])
        else:
            # Add up flagged and total for all data.
            flagcount = int(summary['flagged'])
            totalcount = int(summary['total'])

        # From the second summary onwards, subtract counts from the previous
        # one.
        if idx > 0:
            flagcount -= flagsum

        # Create agent stats object, append to output.
        stat = AgentStats(name=summary['name'],
                          flagged=flagcount,
                          total=totalcount)
        stats.append(stat)

        # Keep count of total number of flags found in summaries, for
        # subsequent summaries.
        flagsum += flagcount

    return stats


def calc_frac_total_flagged(summaries, agents=None, scanids=None):
    """
    Calculate total fraction of data that is flagged. If agents are provided,
    then restrict to statistics for those agents. If scanids are provided,
    then restrict to statistics for those scans.
    """

    agent_stats = calc_flags_per_agent(summaries, scanids=scanids)

    # sum the number of flagged rows for the selected agents
    frac_flagged = reduce(operator.add,
                          [float(s.flagged)/s.total for s in agent_stats if not agents or s.name in agents], 0)

    return frac_flagged


def calc_frac_newly_flagged(summaries, agents=None, scanids=None):
    """
    Calculate fraction of data that is newly flagged, i.e. exclude pre-existing
    flags (assumed to be represented in first summary). If agents are provided,
    then restrict to statistics for those agents. If scanids are provided,
    then restrict to statistics for those scans.
    """
    agent_stats = calc_flags_per_agent(summaries, scanids=scanids)

    # sum the number of flagged rows for the selected agents
    frac_flagged = reduce(operator.add,
                          [float(s.flagged)/s.total for s in agent_stats[1:] if not agents or s.name in agents], 0)

    return frac_flagged


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


def score_data_flagged_by_agents(ms, summaries, min_frac, max_frac, agents, intents=None):
    """
    Calculate a score for the agentflagger summaries based on the fraction of
    data flagged by certain flagging agents. If intents are provided, then
    restrict scoring to the scans that match one or more of these intents.

    min_frac < flagged < max_frac maps to score of 1-0
    """
    # If intents are provided, identify which scans to calculate flagging
    # fraction for.
    if intents:
        scanids = {str(scan.id) for intent in intents for scan in ms.get_scans(scan_intent=intent)}
        if not scanids:
            LOG.warning("Cannot restrict QA score to intent(s) {}, since no matching scans were found."
                        " Score will be based on scans for all intents.".format(utils.commafy(intents, quotes=False)))
    else:
        scanids = None

    # Calculate fraction of flagged data.
    frac_flagged = calc_frac_total_flagged(summaries, agents=agents, scanids=scanids)

    # Convert fraction of flagged data into a score.
    score = linear_score(frac_flagged, min_frac, max_frac, 1.0, 0.0)

    # Set score messages and origin.
    percent = 100.0 * frac_flagged
    longmsg = ('%0.2f%% data in %s flagged by %s flagging agents'
               '' % (percent, ms.basename, utils.commafy(agents, quotes=False)))
    if intents:
        longmsg += ' for intent(s): {}'.format(utils.commafy(intents, quotes=False))
    shortmsg = '%0.2f%% data flagged' % percent

    origin = pqa.QAOrigin(metric_name='score_data_flagged_by_agents',
                          metric_score=frac_flagged,
                          metric_units='Fraction of data newly flagged')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)


# - exported scoring functions -----------------------------------------------------------------------------------------

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

    origin = pqa.QAOrigin(metric_name='score_ms_model_data_column_present',
                          metric_score=f,
                          metric_units='Fraction of MSes with modeldata columns present')

    return pqa.QAScore(score, longmsg, shortmsg, origin=origin)


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
        basenames = utils.commafy([ms.basename for ms in mses_with_history], quotes=False)
        if len(mses_with_history) is 1:
            longmsg = ('Unexpected entries were found in the HISTORY table of %s. '
                       'This measurement set may already be processed.' % basenames)
        else:
            longmsg = ('Unexpected entries were found in the HISTORY tables of %s. '
                       'These measurement sets may already be processed.' % basenames)
        shortmsg = '%s/%s have HISTORY' % (num_with, num_all) 

    else:
        # log a message like 'No history entries were found in a.ms or b.ms'
        basenames = [ms.basename for ms in all_mses]
        s = utils.commafy(basenames, quotes=False, conjunction='or')
        longmsg = 'No HISTORY entries found in %s' % s
        shortmsg = 'No HISTORY entries'

    f = float(num_with) / num_all
    score = linear_score(f, 0.0, 1.0, 1.0, 0.5)

    origin = pqa.QAOrigin(metric_name='score_ms_history_entries_present',
                          metric_score=f,
                          metric_units='Fraction of MSes with HISTORY')

    return pqa.QAScore(score, longmsg, shortmsg, origin=origin)


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
    nophasecals = []

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

    origin = pqa.QAOrigin(metric_name='score_bwswitching',
                          metric_score=len(nophasecals),
                          metric_units='Number of MSes without phase calibrators')

    return pqa.QAScore(max(0.0, score), longmsg=longmsg, shortmsg=shortmsg, origin=origin)


@log_qa
def score_bands(mses):
    """
    Score a MeasurementSet object based on the presence of 
    ALMA bands with calibration issues.
    """

    # ALMA receiver bands. Warnings will be raised for any 
    # measurement sets containing the following bands.
    score = 1.0
    score_map = {'8': -1.0,
                 '9': -1.0,
                 '10': -1.0}

    unsupported = set(score_map.keys())

    num_mses = len(mses)
    all_ok = True
    complaints = []

    # analyse each MS
    for ms in mses:
        msbands = []
        for spw in ms.get_spectral_windows(science_windows_only=True):
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

    origin = pqa.QAOrigin(metric_name='score_bands',
                          metric_score=score,
                          metric_units='MS score based on presence of high-frequency data')

    # Make score linear
    return pqa.QAScore(max(rutils.SCORE_THRESHOLD_SUBOPTIMAL, score), longmsg=longmsg, shortmsg=shortmsg, origin=origin)


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
    score_map = {
        'POLARIZATION': -1.0,
        'POLANGLE': -1.0,
        'POLLEAKAGE': -1.0
    }

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

    origin = pqa.QAOrigin(metric_name='score_polintents',
                          metric_score=score,
                          metric_units='MS score based on presence of polarisation data')

    return pqa.QAScore(max(0.0, score), longmsg=longmsg, shortmsg=shortmsg, origin=origin)


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
        score_map = {'ATMOSPHERE': -1.0}
    else:
        score_map = {
            'PHASE': -1.0,
            'BANDPASS': -0.1,
            'AMPLITUDE': -0.1
        }

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

    origin = pqa.QAOrigin(metric_name='score_missing_intents',
                          metric_score=score,
                          metric_units='Score based on missing calibration intents')

    return pqa.QAScore(max(0.0, score), longmsg=longmsg, shortmsg=shortmsg, origin=origin)


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
    zero_direction = casatools.measures.direction('j2000', '0.0deg', '0.0deg')
    zero_ra = casatools.quanta.formxxx(zero_direction['m0'], format='hms', prec=3)
    zero_dec = casatools.quanta.formxxx(zero_direction['m1'], format='dms', prec=2)

    # analyse each MS
    for ms in mses:
        # Examine each source
        for source in ms.sources:
            if source.ra == zero_ra and source.dec == zero_dec:
                all_ok = False
                score += (-1.0 / num_mses)
                longmsg = ('Suspicious source coordinates for  %s in %s. Check whether position of '
                           '00:00:00.0+00:00:00.0 is valid.' % (source.name, ms.basename))
                complaints.append(longmsg)

    if all_ok:
        longmsg = ('All source coordinates OK in '
                   '%s.' % utils.commafy([ms.basename for ms in mses], False))
        shortmsg = 'All source coordinates OK'
    else:
        longmsg = '%s.' % utils.commafy(complaints, False)
        shortmsg = 'Suspicious source coordinates'

    origin = pqa.QAOrigin(metric_name='score_ephemeris_coordinates',
                          metric_score=score,
                          metric_units='Score based on presence of ephemeris coordinates')

    return pqa.QAScore(max(0.0, score), longmsg=longmsg, shortmsg=shortmsg, origin=origin)


@log_qa
def score_online_shadow_template_agents(ms, summaries):
    """
    Get a score for the fraction of data flagged by online, shadow, and template agents.

    0 < score < 1 === 60% < frac_flagged < 5%
    """
    score = score_data_flagged_by_agents(ms, summaries, 0.05, 0.6,
                                         ['online', 'shadow', 'qa0', 'qa2', 'before', 'template'])

    new_origin = pqa.QAOrigin(metric_name='score_online_shadow_template_agents',
                              metric_score=score.origin.metric_score,
                              metric_units='Fraction of data newly flagged by online, shadow, and template agents')
    score.origin = new_origin

    return score


@log_qa
def score_vla_agents(ms, summaries):
    """
    Get a score for the fraction of data flagged by online, shadow, and template agents.

    0 < score < 1 === 60% < frac_flagged < 5%
    """
    score = score_data_flagged_by_agents(ms, summaries, 0.05, 0.6,
                                         ['online', 'shadow', 'qa0', 'qa2', 'before', 'template'])

    new_origin = pqa.QAOrigin(metric_name='score_vla_agents',
                              metric_score=score.origin.metric_score,
                              metric_units='Fraction of data newly flagged by online, shadow, and template agents')
    score.origin = new_origin

    return score


@log_qa
def score_applycal_agents(ms, summaries):
    """
    Get a score for the fraction of data flagged by applycal agents.

    0 < score < 1 === 60% < frac_flagged < 5%
    """
    # Get score for 'applycal' agent and 'TARGET' intent.
    score = score_data_flagged_by_agents(ms, summaries, 0.05, 0.6, ['applycal'], intents=['TARGET'])

    new_origin = pqa.QAOrigin(metric_name='score_applycal_agents',
                              metric_score=score.origin.metric_score,
                              metric_units=score.origin.metric_units)
    score.origin = new_origin

    return score


@log_qa
def score_total_data_flagged(filename, summaries):
    """
    Calculate a score for the flagging task based on the total fraction of
    data flagged.
    
    0%-5% flagged   -> 1
    5%-50% flagged  -> 0.5
    50-100% flagged -> 0
    """
    # Calculate fraction of flagged data.
    frac_flagged = calc_frac_total_flagged(summaries)

    # Convert fraction of flagged data into a score.
    if frac_flagged > 0.5:
        score = 0
    else:
        score = linear_score(frac_flagged, 0.05, 0.5, 1.0, 0.5)

    # Set score messages and origin.
    percent = 100.0 * frac_flagged
    longmsg = '%0.2f%% of data in %s was flagged' % (percent, filename)
    shortmsg = '%0.2f%% data flagged' % percent

    origin = pqa.QAOrigin(metric_name='score_total_data_flagged',
                          metric_score=frac_flagged,
                          metric_units='Total fraction of data that is flagged')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=os.path.basename(filename), origin=origin)


@log_qa
def score_total_data_flagged_vla(filename, summaries):
    """
    Calculate a score for the flagging task based on the total fraction of
    data flagged.

    0%-5% flagged   -> 1
    5%-60% flagged  -> 1 to 0
    60-100% flagged -> 0
    """
    # Calculate fraction of flagged data.
    frac_flagged = calc_frac_newly_flagged(summaries)

    # Convert fraction of flagged data into a score.
    if frac_flagged > 0.6:
        score = 0
    else:
        score = linear_score(frac_flagged, 0.05, 0.6, 1.0, 0.0)

    # Set score messages and origin.
    percent = 100.0 * frac_flagged
    longmsg = '%0.2f%% of data in %s was flagged' % (percent, filename)
    shortmsg = '%0.2f%% data flagged' % percent

    origin = pqa.QAOrigin(metric_name='score_total_data_flagged_vla',
                          metric_score=frac_flagged,
                          metric_units='Total fraction of VLA data that is flagged')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=os.path.basename(filename), origin=origin)


@log_qa
def score_fraction_newly_flagged(filename, summaries, vis):
    """
    Calculate a score for the flagging task based on the fraction of
    data newly flagged.
    
    0%-5% flagged   -> 1
    5%-50% flagged  -> 0.5
    50-100% flagged -> 0
    """
    # Calculate fraction of flagged data.
    frac_flagged = calc_frac_newly_flagged(summaries)

    # Convert fraction of flagged data into a score.
    if frac_flagged > 0.5:
        score = 0
    else:
        score = linear_score(frac_flagged, 0.05, 0.5, 1.0, 0.5)

    # Set score messages and origin.
    percent = 100.0 * frac_flagged
    longmsg = '%0.2f%% of data in %s was newly flagged' % (percent, filename)
    shortmsg = '%0.2f%% data flagged' % percent

    origin = pqa.QAOrigin(metric_name='score_fraction_newly_flagged',
                          metric_score=frac_flagged,
                          metric_units='Fraction of data that is newly flagged')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=os.path.basename(vis), origin=origin)


@log_qa
def linear_score_fraction_newly_flagged(filename, summaries, vis):
    """
    Calculate a score for the flagging task based on the fraction of
    data newly flagged.
    
    fraction flagged   -> score
    """
    # Calculate fraction of flagged data.
    frac_flagged = calc_frac_newly_flagged(summaries)

    # Convert fraction of flagged data into a score.
    score = 1.0 - frac_flagged

    # Set score messages and origin.
    percent = 100.0 * frac_flagged
    longmsg = '%0.2f%% of data in %s was newly flagged' % (percent, filename)
    shortmsg = '%0.2f%% data flagged' % percent

    origin = pqa.QAOrigin(metric_name='linear_score_fraction_newly_flagged',
                          metric_score=frac_flagged,
                          metric_units='Fraction of data that is newly flagged')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=os.path.basename(vis), origin=origin)


@log_qa
def linear_score_fraction_unflagged_newly_flagged_for_intent(ms, summaries, intent):
    """
    Calculate a score for the flagging task based on the fraction of unflagged
    data for scans belonging to specified intent that got newly flagged.

    If no unflagged data was found in the before summary, then return a score
    of 0.0.
    """

    # Identify scan IDs belonging to intent.
    scanids = [str(scan.id) for scan in ms.get_scans(scan_intent=intent)]

    # Calculate flags for scans belonging to intent.
    agent_stats = calc_flags_per_agent(summaries, scanids=scanids)

    # Calculate counts of unflagged data.
    unflaggedcount = agent_stats[0].total - agent_stats[0].flagged

    # If the "before" summary had unflagged data, then proceed to compute
    # the fraction fo unflagged data that got newly flagged.
    if unflaggedcount > 0:
        frac_flagged = reduce(operator.add,
                              [float(s.flagged)/unflaggedcount for s in agent_stats[1:]], 0)

        score = 1.0 - frac_flagged
        percent = 100.0 * frac_flagged
        longmsg = '{:0.2f}% of unflagged data with intent {} in {} was newly ' \
                  'flagged.'.format(percent, intent, ms.basename)
        shortmsg = '{:0.2f}% unflagged data flagged.'.format(percent)

        origin = pqa.QAOrigin(metric_name='linear_score_fraction_unflagged_newly_flagged_for_intent',
                              metric_score=frac_flagged,
                              metric_units='Fraction of unflagged data for intent '
                                           '{} that is newly flagged'.format(intent))
    # If no unflagged data was found at the start, return score of 0.
    else:
        score = 0.0
        longmsg = 'No unflagged data with intent {} found in {}.'.format(intent, ms.basename)
        shortmsg = 'No unflagged data.'
        origin = pqa.QAOrigin(metric_name='linear_score_fraction_unflagged_newly_flagged_for_intent',
                              metric_score=False,
                              metric_units='Presence of unflagged data.')

    # Append extra warning to QA message if score falls at-or-below the "warning" threshold.
    if score <= rutils.SCORE_THRESHOLD_WARNING:
        longmsg += ' Please investigate!'
        shortmsg += ' Please investigate!'

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)


@log_qa
def score_contiguous_session(mses, tolerance=datetime.timedelta(hours=1)):
    """
    Check whether measurement sets are contiguous in time. 
    """
    # only need to check when given multiple measurement sets
    if len(mses) < 2:
        origin = pqa.QAOrigin(metric_name='score_contiguous_session',
                              metric_score=0,
                              metric_units='Non-contiguous measurement sets present')
        return pqa.QAScore(1.0,
                           longmsg='%s forms one continuous observing session.' % mses[0].basename,
                           shortmsg='Unbroken observing session',
                           vis=mses[0].basename,
                           origin=origin)

    # reorder MSes by start time
    by_start = sorted(mses, 
                      key=lambda m: utils.get_epoch_as_datetime(m.start_time))

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

    origin = pqa.QAOrigin(metric_name='score_contiguous_session',
                          metric_score=not bool(bad_mses),
                          metric_units='Non-contiguous measurement sets present')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, origin=origin)


@log_qa
def score_wvrgcal(ms_name, wvr_score):
    if wvr_score < 1.0:
        score = 0
    else:
        score = linear_score(wvr_score, 1.0, 2.0, 0.5, 1.0)

    longmsg = 'RMS improvement was %0.2f for %s' % (wvr_score, ms_name)
    shortmsg = '%0.2fx improvement' % wvr_score

    origin = pqa.QAOrigin(metric_name='score_wvrgcal',
                          metric_score=wvr_score,
                          metric_units='Phase RMS improvement after applying WVR correction')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=os.path.basename(ms_name), origin=origin)


@log_qa
def score_sdtotal_data_flagged(label, frac_flagged):
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
    longmsg = '%0.2f%% of data in %s was newly flagged' % (percent, label)
    shortmsg = '%0.2f%% data flagged' % percent

    origin = pqa.QAOrigin(metric_name='score_sdtotal_data_flagged',
                          metric_score=frac_flagged,
                          metric_units='Fraction of data newly flagged')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=None, origin=origin)


@log_qa
def score_sdtotal_data_flagged_old(name, ant, spw, pol, frac_flagged, field=None):
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
    if field is None:
        longmsg = '%0.2f%% of data in %s (Ant=%s, SPW=%d, Pol=%d) was flagged' % (percent, name, ant, spw, pol)
    else:
        longmsg = ('%0.2f%% of data in %s (Ant=%s, Field=%s, SPW=%d, Pol=%s) was '
                   'flagged' % (percent, name, ant, field, spw, pol))
    shortmsg = '%0.2f%% data flagged' % percent

    origin = pqa.QAOrigin(metric_name='score_sdtotal_data_flagged_old',
                          metric_score=frac_flagged,
                          metric_units='Fraction of data newly flagged')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=os.path.basename(name), origin=origin)


@log_qa
def score_tsysspwmap(ms, unmappedspws):
    """
    Score is equal to the fraction of unmapped windows
    """

    if len(unmappedspws) <= 0:
        score = 1.0
        longmsg = 'Tsys spw map is complete for %s ' % ms.basename
        shortmsg = 'Tsys spw map is complete'
    else:
        nscispws = len([spw.id for spw in ms.get_spectral_windows(science_windows_only=True)])
        if nscispws <= 0:
            score = 0.0
        else:
            score = float(nscispws - len(unmappedspws)) / float(nscispws)
        longmsg = 'Tsys spw map is incomplete for %s science window%s ' % (ms.basename,
                                                                           utils.commafy(unmappedspws, False, 's'))
        shortmsg = 'Tsys spw map is incomplete'

    origin = pqa.QAOrigin(metric_name='score_tsysspwmap',
                          metric_score=score,
                          metric_units='Fraction of unmapped Tsys windows')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)


@log_qa
def score_setjy_measurements(ms, reqfields, reqintents, reqspws, measurements):
    """
    Score is equal to the ratio of the number of actual flux
    measurements to expected number of flux measurements
    """

    # Expected fields
    scifields = {field for field in ms.get_fields(reqfields, intent=reqintents)}

    # Expected science windows
    scispws = {spw.id for spw in ms.get_spectral_windows(reqspws, science_windows_only=True)}

    # Loop over the expected fields
    nexpected = 0
    for scifield in scifields:
        validspws = set([spw.id for spw in scifield.valid_spws])
        nexpected += len(validspws.intersection(scispws))

    # Loop over the measurements
    nmeasured = 0
    for value in measurements.itervalues():
        # Loop over the flux measurements
        nmeasured += len(value)

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

    origin = pqa.QAOrigin(metric_name='score_setjy_measurements',
                          metric_score=score,
                          metric_units='Ratio of number of flux measurements to number expected')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)


@log_qa
def score_number_antenna_offsets(ms, offsets):
    """
    Score is 1.0 if no antenna needed a position offset correction, and
    set to the "suboptimal" threshold if at least one antenna needed a 
    correction.
    """
    nant_with_offsets = len(offsets) / 3

    if nant_with_offsets == 0:
        score = 1.0
        longmsg = 'No antenna position offsets for %s ' % ms.basename
        shortmsg = 'No antenna position offsets'
    else:
        # CAS-8877: if at least 1 antenna needed correction, then set the score
        # to the "suboptimal" threshold. 
        score = rutils.SCORE_THRESHOLD_SUBOPTIMAL
        longmsg = '%d nonzero antenna position offsets for %s ' % (nant_with_offsets, ms.basename)
        shortmsg = 'Nonzero antenna position offsets'

    origin = pqa.QAOrigin(metric_name='score_number_antenna_offsets',
                          metric_score=nant_with_offsets,
                          metric_units='Number of antennas requiring position offset correction')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)


@log_qa
def score_missing_derived_fluxes(ms, reqfields, reqintents, measurements):
    """
    Score is equal to the ratio of actual flux
    measurement to expected flux measurements
    """
    # Expected fields
    scifields = {field for field in ms.get_fields(reqfields, intent=reqintents)}

    # Expected science windows
    scispws = {spw.id for spw in ms.get_spectral_windows(science_windows_only=True)}

    # Loop over the expected fields
    nexpected = 0
    for scifield in scifields:
        validspws = {spw.id for spw in scifield.valid_spws}
        nexpected += len(validspws.intersection(scispws))

    # Loop over measurements
    nmeasured = 0
    for key, value in measurements.iteritems():
        # Loop over the flux measurements
        for flux in value:
            fluxjy = getattr(flux, 'I').to_units(measures.FluxDensityUnits.JANSKY)
            uncjy = getattr(flux.uncertainty, 'I').to_units(measures.FluxDensityUnits.JANSKY)
            if fluxjy <= 0.0 or uncjy <= 0.0: 
                continue
            nmeasured += 1

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

    origin = pqa.QAOrigin(metric_name='score_missing_derived_fluxes',
                          metric_score=score,
                          metric_units='Ratio of number of flux measurements to number expected')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)


@log_qa
def score_refspw_mapping_fraction(ms, ref_spwmap):
    """
    Compute the fraction of science spws that have not been
    mapped to other windows.
    """
    if ref_spwmap == [-1]:
        score = 1.0
        longmsg = 'No mapped science spws for %s ' % ms.basename
        shortmsg = 'No mapped science spws'

        origin = pqa.QAOrigin(metric_name='score_refspw_mapping_fraction',
                              metric_score=0,
                              metric_units='Number of unmapped science spws')
    else:
        # Expected science windows
        scispws = set([spw.id for spw in ms.get_spectral_windows(science_windows_only=True)])
        nexpected = len(scispws)

        nunmapped = 0
        for spwid in scispws:
            if spwid == ref_spwmap[spwid]: 
                nunmapped += 1
        
        if nunmapped >= nexpected:
            score = 1.0
            longmsg = 'No mapped science spws for %s ' % ms.basename
            shortmsg = 'No mapped science spws'
        else:
            # Replace the previous score with a warning
            score = rutils.SCORE_THRESHOLD_WARNING
            longmsg = 'There are %d mapped science spws for %s ' % (nexpected - nunmapped, ms.basename)
            shortmsg = 'There are mapped science spws'

        origin = pqa.QAOrigin(metric_name='score_refspw_mapping_fraction',
                              metric_score=nunmapped,
                              metric_units='Number of unmapped science spws')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)


@log_qa
def score_phaseup_mapping_fraction(ms, fullcombine, phaseup_spwmap):
    """
    Compute the fraction of science spws that have not been
    mapped to other probably wider windows.
    """

    if not phaseup_spwmap:
        nunmapped = len([spw for spw in ms.get_spectral_windows(science_windows_only=True)])
        score = 1.0
        longmsg = 'No spw mapping for %s ' % ms.basename
        shortmsg = 'No spw mapping'
    elif fullcombine is True:
        nunmapped = 0
        score = rutils.SCORE_THRESHOLD_WARNING
        longmsg = 'Combined spw mapping for %s ' % ms.basename
        shortmsg = 'Combined spw mapping'
    else:
        # Expected science windows
        scispws = [spw for spw in ms.get_spectral_windows(science_windows_only=True)]
        scispwids = [spw.id for spw in ms.get_spectral_windows(science_windows_only=True)]
        nexpected = len(scispwids)

        nunmapped = 0
        samesideband = True
        for spwid, scispw in zip(scispwids, scispws):
            if spwid == phaseup_spwmap[spwid]:
                nunmapped += 1
            else:
                if scispw.sideband != ms.get_spectral_window(phaseup_spwmap[spwid]).sideband:
                    samesideband = False
        
        if nunmapped >= nexpected:
            score = 1.0
            longmsg = 'No spw mapping for %s ' % ms.basename
            shortmsg = 'No spw mapping'
        else:
            # Replace the previous score with a warning
            if samesideband is True:
                score = rutils.SCORE_THRESHOLD_SUBOPTIMAL
                longmsg = 'Spw mapping within sidebands for %s' % ms.basename
                shortmsg = 'Spw mapping within sidebands'
            else:
                score = rutils.SCORE_THRESHOLD_WARNING
                longmsg = 'Spw mapping across sidebands required for %s' % ms.basename
                shortmsg = 'Spw mapping across sidebands'

    origin = pqa.QAOrigin(metric_name='score_phaseup_mapping_fraction',
                          metric_score=nunmapped,
                          metric_units='Number of unmapped science spws')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)


@log_qa
def score_missing_phaseup_snrs(ms, spwids, phsolints):
    """
    Score is the fraction of spws with phaseup SNR estimates
    """
    # Compute the number of expected and missing SNR measurements
    nexpected = len(spwids)
    missing_spws = []
    for i in range(len(spwids)):
        if not phsolints[i]:
            missing_spws.append(spwids[i])
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
        score = float(nexpected - nmissing) / nexpected
        longmsg = 'Missing phaseup SNR estimates for spws %s in %s ' % \
            (missing_spws, ms.basename)
        shortmsg = 'Missing phaseup SNR estimates'

    origin = pqa.QAOrigin(metric_name='score_missing_phaseup_snrs',
                          metric_score=nmissing,
                          metric_units='Number of spws with missing SNR measurements')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)


@log_qa
def score_poor_phaseup_solutions(ms, spwids, nphsolutions, min_nsolutions):
    """
    Score is the fraction of spws with poor phaseup solutions
    """
    # Compute the number of expected and poor SNR measurements
    nexpected = len(spwids)
    poor_spws = []
    for i in range(len(spwids)):
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
        score = float(nexpected - npoor) / nexpected
        longmsg = 'Poorly determined phaseup solutions for spws %s in %s ' % \
            (poor_spws, ms.basename)
        shortmsg = 'Poorly determined phaseup solutions'

    origin = pqa.QAOrigin(metric_name='score_poor_phaseup_solutions',
                          metric_score=npoor,
                          metric_units='Number of poor phaseup solutions')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)


@log_qa
def score_missing_bandpass_snrs(ms, spwids, bpsolints):
    """
    Score is the fraction of spws with bandpass SNR estimates
    """

    # Compute the number of expected and missing SNR measurements
    nexpected = len(spwids)
    missing_spws = []
    for i in range(len(spwids)):
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
        score = float(nexpected - nmissing) / nexpected
        longmsg = 'Missing bandpass SNR estimates for spws %s in%s ' % \
            (missing_spws, ms.basename)
        shortmsg = 'Missing bandpass SNR estimates'

    origin = pqa.QAOrigin(metric_name='score_missing_bandpass_snrs',
                          metric_score=nmissing,
                          metric_units='Number of missing bandpass SNR estimates')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)


@log_qa
def score_poor_bandpass_solutions(ms, spwids, nbpsolutions, min_nsolutions):
    """
    Score is the fraction of spws with poor bandpass solutions
    """
    # Compute the number of expected and poor solutions
    nexpected = len(spwids)
    poor_spws = []
    for i in range(len(spwids)):
        if not nbpsolutions[i]:
            poor_spws.append(spwids[i])
        elif nbpsolutions[i] < min_nsolutions:
            poor_spws.append(spwids[i])
    npoor = len(poor_spws) 

    if nexpected <= 0:
        score = 0.0
        longmsg = 'No bandpass solutions for %s ' % ms.basename
        shortmsg = 'No bandpass solutions'
    elif npoor <= 0:
        score = 1.0
        longmsg = 'No poorly determined bandpass solutions for %s ' % ms.basename
        shortmsg = 'No poorly determined bandpass solutions'
    else:
        score = float(nexpected - npoor) / nexpected
        longmsg = 'Poorly determined bandpass solutions for spws %s in %s ' % (poor_spws, ms.basename)
        shortmsg = 'Poorly determined bandpass solutions'

    origin = pqa.QAOrigin(metric_name='score_missing_bandpass_snrs',
                          metric_score=npoor,
                          metric_units='Number of poor bandpass solutions')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)


@log_qa
def score_missing_phase_snrs(ms, spwids, snrs):
    """
    Score is the fraction of spws with SNR estimates
    """
    # Compute the number of expected and missing SNR measurements
    nexpected = len(spwids)
    missing_spws = []
    for i in range(len(spwids)):
        if not snrs[i]:
            missing_spws.append(spwids[i])
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
        score = float(nexpected - nmissing) / nexpected
        longmsg = 'Missing gaincal SNR estimates for spws %s in %s ' % (missing_spws, ms.basename)
        shortmsg = 'Missing gaincal SNR estimates'

    origin = pqa.QAOrigin(metric_name='score_missing_phase_snrs',
                          metric_score=nmissing,
                          metric_units='Number of missing phase SNR estimates')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)


@log_qa
def score_poor_phase_snrs(ms, spwids, minsnr, snrs):
    """
    Score is the fraction of spws with poor snr estimates
    """
    # Compute the number of expected and poor solutions
    nexpected = len(spwids)
    poor_spws = []
    for i in range(len(spwids)):
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
        score = float(nexpected - npoor) / nexpected
        longmsg = 'Low gaincal SNR estimates for spws %s in %s ' % \
            (poor_spws, ms.basename)
        shortmsg = 'Low gaincal SNR estimates'

    origin = pqa.QAOrigin(metric_name='score_poor_phase_snrs',
                          metric_score=npoor,
                          metric_units='Number of poor phase SNR estimates')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)


@log_qa
def score_derived_fluxes_snr(ms, measurements):
    """
    Score the SNR of the derived flux measurements.
        1.0 if SNR > 20.0
        0.0 if SNR < 5.0
        linear scale between 0.0 and 1.0 in between
    """
    # Loop over measurements
    nmeasured = 0
    score = 0.0
    minscore = 1.0
    minsnr = None

    for _, value in measurements.iteritems():
        # Loop over the flux measurements
        for flux in value:
            fluxjy = flux.I.to_units(measures.FluxDensityUnits.JANSKY)
            uncjy = flux.uncertainty.I.to_units(measures.FluxDensityUnits.JANSKY)
            if fluxjy <= 0.0 or uncjy <= 0.0: 
                continue
            snr = fluxjy / uncjy
            minsnr = snr if minsnr is None else min(minsnr, snr)
            nmeasured += 1
            score1 = linear_score(float(snr), 5.0, 20.0, 0.0, 1.0)
            minscore = min(minscore, score1)
            score += score1

    if nmeasured > 0:
        score /= nmeasured

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

    origin = pqa.QAOrigin(metric_name='score_derived_fluxes_snr',
                          metric_score=minsnr,
                          metric_units='Minimum SNR of derived flux measurement')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=ms.basename, origin=origin)


@log_qa
def score_path_exists(mspath, path, pathtype):
    """
    Score the existence of the path
        1.0 if it exist
        0.0 if it does not
    """
    if os.path.exists(path):
        score = 1.0
        longmsg = 'The %s file %s for %s was created' % (pathtype, os.path.basename(path), os.path.basename(mspath))
        shortmsg = 'The %s file was created' % pathtype
    else:
        score = 0.0
        longmsg = 'The %s file %s for %s was not created' % (pathtype, os.path.basename(path), os.path.basename(mspath))
        shortmsg = 'The %s file was not created' % pathtype

    origin = pqa.QAOrigin(metric_name='score_path_exists',
                          metric_score=bool(score),
                          metric_units='Path exists on disk')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, origin=origin)


@log_qa
def score_file_exists(filedir, filename, filetype):
    """
    Score the existence of a products file
        1.0 if it exists
        0.0 if it does not
    """
    if filename is None:
        score = 1.0
        longmsg = 'The %s file is undefined' % filetype
        shortmsg = 'The %s file is undefined' % filetype

        origin = pqa.QAOrigin(metric_name='score_file_exists',
                              metric_score=None,
                              metric_units='No %s file to check' % filetype)

        return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, origin=origin)

    file_path = os.path.join(filedir, os.path.basename(filename))
    if os.path.exists(file_path):
        score = 1.0
        longmsg = 'The %s file has been exported' % filetype
        shortmsg = 'The %s file has been exported' % filetype
    else:
        score = 0.0
        longmsg = 'The %s file %s does not exist' % (filetype, os.path.basename(filename))
        shortmsg = 'The %s file does not exist' % filetype

    origin = pqa.QAOrigin(metric_name='score_file_exists',
                          metric_score=bool(score),
                          metric_units='File exists on disk')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, origin=origin)


@log_qa
def score_mses_exist(filedir, visdict):
    """
    Score the existence of the flagging products files 
        1.0 if they all exist
        n / nexpected if some of them exist
        0.0 if none exist
    """

    nexpected = len(visdict)
    nfiles = 0
    missing = []

    for visname in visdict:
        file_path = os.path.join(filedir, os.path.basename(visdict[visname]))
        if os.path.exists(file_path):
            nfiles += 1
        else:
            missing.append(os.path.basename(visdict[visname]))

    if nfiles <= 0:
        score = 0.0
        longmsg = 'Final ms files %s are missing' % (','.join(missing))
        shortmsg = 'Missing final ms files'
    elif nfiles < nexpected:
        score = float(nfiles) / float(nexpected)
        longmsg = 'Final ms files %s are missing' % (','.join(missing))
        shortmsg = 'Missing final ms files'
    else:
        score = 1.0
        longmsg = 'No missing final ms  files'
        shortmsg = 'No missing final ms files'

    origin = pqa.QAOrigin(metric_name='score_mses_exist',
                          metric_score=len(missing),
                          metric_units='Number of missing ms product files')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, origin=origin)


@log_qa
def score_flags_exist(filedir, visdict):
    """
    Score the existence of the flagging products files 
        1.0 if they all exist
        n / nexpected if some of them exist
        0.0 if none exist
    """
    nexpected = len(visdict)
    nfiles = 0
    missing = []

    for visname in visdict:
        file_path = os.path.join(filedir, os.path.basename(visdict[visname][0]))
        if os.path.exists(file_path):
            nfiles += 1
        else:
            missing.append(os.path.basename(visdict[visname][0]))

    if nfiles <= 0:
        score = 0.0
        longmsg = 'Final flag version files %s are missing' % (','.join(missing))
        shortmsg = 'Missing final flags version files'
    elif nfiles < nexpected:
        score = float(nfiles) / float(nexpected)
        longmsg = 'Final flag version files %s are missing' % (','.join(missing))
        shortmsg = 'Missing final flags version files'
    else:
        score = 1.0
        longmsg = 'No missing final flag version files'
        shortmsg = 'No missing final flags version files'

    origin = pqa.QAOrigin(metric_name='score_flags_exist',
                          metric_score=len(missing),
                          metric_units='Number of missing flagging product files')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, origin=origin)


@log_qa
def score_flagging_view_exists(filename, result):
    """
    Assign a score of zero if the flagging view cannot be computed
    """

    # By default, assume no flagging views were found.
    score = 0.0
    longmsg = 'No flagging views for %s' % filename
    shortmsg = 'No flagging views'

    # Check if this is a flagging result for a single metric, where
    # the flagging view is stored directly in the result.
    try:
        view = result.view
        if view:
            score = 1.0
            longmsg = 'Flagging views exist for %s' % filename
            shortmsg = 'Flagging views exist'
    except AttributeError:
        pass

    # Check if this flagging results contains multiple metrics,
    # and look for flagging views among components.
    try:
        # Set score to 1 as soon as a single metric contains a
        # valid flagging view.
        for metricresult in result.components.values():
            view = metricresult.view
            if view:
                score = 1.0
                longmsg = 'Flagging views exist for %s' % filename
                shortmsg = 'Flagging views exist'
    except AttributeError:
        pass

    origin = pqa.QAOrigin(metric_name='score_flagging_view_exists',
                          metric_score=bool(score),
                          metric_units='Presence of flagging view')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=filename, origin=origin)


@log_qa
def score_applycmds_exist(filedir, visdict):
    """
    Score the existence of the apply commands products files
        1.0 if they all exist
        n / nexpected if some of them exist
        0.0 if none exist
    """
    nexpected = len(visdict)
    nfiles = 0
    missing = []

    for visname in visdict:
        file_path = os.path.join(filedir, os.path.basename(visdict[visname][1]))
        if os.path.exists(file_path):
            nfiles += 1
        else:
            missing.append(os.path.basename(visdict[visname][1]))

    if nfiles <= 0:
        score = 0.0
        longmsg = 'Final apply commands files %s are missing' % (','.join(missing))
        shortmsg = 'Missing final apply commands files'
    elif nfiles < nexpected:
        score = float(nfiles) / float(nexpected)
        longmsg = 'Final apply commands files %s are missing' % (','.join(missing))
        shortmsg = 'Missing final apply commands files'
    else:
        score = 1.0
        longmsg = 'No missing final apply commands files'
        shortmsg = 'No missing final apply commands files'

    origin = pqa.QAOrigin(metric_name='score_applycmds_exist',
                          metric_score=len(missing),
                          metric_units='Number of missing apply command files')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, origin=origin)


@log_qa
def score_caltables_exist(filedir, sessiondict):
    """
    Score the existence of the caltables products files
        1.0 if theu all exist
        n / nexpected if some of them exist
        0.0 if none exist
    """
    nexpected = len(sessiondict)
    nfiles = 0
    missing = []

    for sessionname in sessiondict:
        file_path = os.path.join(filedir, os.path.basename(sessiondict[sessionname][1]))
        if os.path.exists(file_path):
            nfiles += 1
        else:
            missing.append(os.path.basename(sessiondict[sessionname][1]))

    if nfiles <= 0:
        score = 0.0
        longmsg = 'Caltables files %s are missing' % (','.join(missing))
        shortmsg = 'Missing caltables files'
    elif nfiles < nexpected:
        score = float(nfiles) / float(nexpected)
        longmsg = 'Caltables files %s are missing' % (','.join(missing))
        shortmsg = 'Missing caltables files'
    else:
        score = 1.0
        longmsg = 'No missing caltables files'
        shortmsg = 'No missing caltables files'

    origin = pqa.QAOrigin(metric_name='score_caltables_exist',
                          metric_score=len(missing),
                          metric_units='Number of missing caltables')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, origin=origin)


@log_qa
def score_images_exist(filesdir, imaging_products_only, calimages, targetimages):
    if imaging_products_only:
        if len(targetimages) <= 0:
            score = 0.0
            metric = 0
            longmsg = 'No target images were exported'
            shortmsg = 'No target images exported'
        else:
            score = 1.0
            metric = len(targetimages)
            longmsg = '%d target images were exported' % (len(targetimages))
            shortmsg = 'Target images exported'
    else:
        if len(targetimages) <= 0 and len(calimages) <= 0:
            score = 0.0
            metric = 0
            longmsg = 'No images were exported'
            shortmsg = 'No images exported'
        else:
            score = 1.0
            metric = len(calimages) + len(targetimages)
            longmsg = '%d images were exported' % (len(calimages) + len(targetimages))
            shortmsg = 'Images exported'

    origin = pqa.QAOrigin(metric_name='score_images_exist',
                          metric_score=metric,
                          metric_units='Number of exported images')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, origin=origin)


@log_qa
def score_sd_line_detection(group_id_list, spw_id_list, lines_list):
    detected_spw = []
    detected_group = []

    for group_id, spw_id, lines in zip(group_id_list, spw_id_list, lines_list):
        if any([l[2] for l in lines]):
            LOG.trace('detected lines exist at group_id %s spw_id %s' % (group_id, spw_id))
            unique_spw_id = set(spw_id)
            if len(unique_spw_id) == 1:
                detected_spw.append(unique_spw_id.pop())
            else:
                detected_spw.append(-1)
            detected_group.append(group_id)

    if len(detected_spw) == 0:
        score = 0.0
        longmsg = 'No spectral lines were detected'
        shortmsg = 'No spectral lines were detected'
    else:
        score = 1.0
        if detected_spw.count(-1) == 0:
            longmsg = 'Spectral lines were detected in spws %s' % (', '.join(map(str, detected_spw)))
        else:
            longmsg = 'Spectral lines were detected in ReductionGroups %s' % (','.join(map(str, detected_group)))
        shortmsg = 'Spectral lines were detected'

    origin = pqa.QAOrigin(metric_name='score_sd_line_detection',
                          metric_score=len(detected_spw),
                          metric_units='Number of spectral lines detected')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, origin=origin)


@log_qa
def score_sd_line_detection_for_ms(group_id_list, field_id_list, spw_id_list, lines_list):
    detected_spw = []
    detected_field = []
    detected_group = []

    for group_id, field_id, spw_id, lines in zip(group_id_list, field_id_list, spw_id_list, lines_list):
        if any([l[2] for l in lines]):
            LOG.trace('detected lines exist at group_id %s field_id %s spw_id %s' % (group_id, field_id, spw_id))
            unique_spw_id = set(spw_id)
            if len(unique_spw_id) == 1:
                detected_spw.append(unique_spw_id.pop())
            else:
                detected_spw.append(-1)
            unique_field_id = set(field_id)
            if len(unique_field_id) == 1:
                detected_field.append(unique_field_id.pop())
            else:
                detected_field.append(-1)
            detected_group.append(group_id)

    if len(detected_spw) == 0:
        score = 0.0
        longmsg = 'No spectral lines are detected'
        shortmsg = 'No spectral lines are detected'
    else:
        score = 1.0
        if detected_spw.count(-1) == 0 and detected_field.count(-1) == 0:
            longmsg = 'Spectral lines are detected at Spws (%s) Fields (%s)' % (', '.join(map(str, detected_spw)),
                                                                                ', '.join(map(str, detected_field)))
        else:
            longmsg = 'Spectral lines are detected at ReductionGroups %s' % (','.join(map(str, detected_group)))
        shortmsg = 'Spectral lines are detected'

    origin = pqa.QAOrigin(metric_name='score_sd_line_detection_for_ms',
                          metric_score=len(detected_spw),
                          metric_units='Number of spectral lines detected')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, origin=origin)


@log_qa
def score_checksources(mses, fieldname, spwid, imagename, rms, gfluxscale, gfluxscale_err):
    """
    Score a single field image of a point source by comparing the source
    reference position to the fitted position and the source reference flux
    to the fitted flux.

    The source is assumed to be near the center of the image.
    The fit is performed using pixels in a circular regions
    around the center of the image
    """
    qa = casatools.quanta
    me = casatools.measures

    # Get the reference direction of the check source field 
    #    There is at least one field with check source intent
    #    Protect against the same source having multiple fields
    #    with different intent.
    #    Assume that the same field as defined by its field name
    #    has the same direction in all the mses that contributed
    #    to the input image. Loop through the ms(s) and find the
    #    first occurrence of the specified field. Convert the
    #    direction to ICRS to match the default image coordinate
    #    system

    refdirection = None
    for ms in mses:
        field = ms.get_fields(name=fieldname)
        # No fields with the check source name. Should be
        # impossible at this point but check just in case
        if not field:
            continue
        # Find check field for that ms
        chkfield = None
        for fielditem in field:
            if 'CHECK' not in fielditem.intents:
                continue
            chkfield = fielditem
            break
        # No matching check field for that ms, next ms
        if chkfield is None:
            continue
        # Found field, get reference direction in ICRS coordinates
        LOG.info('Using field name %s id %s to determine check source reference direction' %
                 (chkfield.name, str(chkfield.id)))
        refdirection = me.measure(chkfield.mdirection, 'ICRS')
        break

    # Get the reference flux of the check source field
    #    Loop over all the ms(s) extracting the derived flux
    #    values for the specified field and spw. Set the reference
    #    flux to the maximum of these values.
    reffluxes = []
    for ms in mses:
        if not ms.derived_fluxes:
            continue
        for field_arg, measurements in ms.derived_fluxes.items():
            mfield = ms.get_fields(field_arg)
            chkfield = None
            for mfielditem in mfield:
                if mfielditem.name != fieldname:
                    continue
                if 'CHECK' not in mfielditem.intents:
                    continue
                chkfield = mfielditem
                break
            # No matching check field for this ms
            if chkfield is None:
                continue
            LOG.info('Using field name %s id %s to identify check source flux densities' %
                     (chkfield.name, str(chkfield.id)))
            for measurement in sorted(measurements, key=lambda m: int(m.spw_id)):
                if int(measurement.spw_id) != spwid:
                    continue
                for stokes in ['I']:
                    try:
                        flux = getattr(measurement, stokes)
                        flux_jy = float(flux.to_units(measures.FluxDensityUnits.JANSKY))
                        reffluxes.append(flux_jy)
                    except:
                        pass

    # Use the maximum reference flux
    if not reffluxes:
        refflux = None
    else:
        median_flux = np.median(np.array(reffluxes))
        refflux = qa.quantity(median_flux, 'Jy')

    # Do the fit and compute positions offsets and flux ratios
    fitdict = checksource.checkimage(imagename, rms, refdirection, refflux)

    msnames = ','.join([os.path.basename(ms.name).strip('.ms') for ms in mses])

    # Compute the scores the default score is the geometric mean of
    # the position and flux scores if both are available.
    if not fitdict:
        offset = None
        offset_err = None
        beams = None
        beams_err = None
        fitflux = None
        fitflux_err = None
        fitpeak = None
        score = 0.0
        longmsg = 'Check source fit failed for %s field %s spwid %d' % (msnames, fieldname, spwid)
        shortmsg = 'Check source fit failed'
        metric_score = 'N/A'
        metric_units = 'Check source fit failed'

    else:
        offset = fitdict['positionoffset']['value'] * 1000.0
        offset_err = fitdict['positionoffset_err']['value'] * 1000.0
        beams = fitdict['beamoffset']['value']
        beams_err = fitdict['beamoffset_err']['value']
        fitflux = fitdict['fitflux']['value']
        fitflux_err = fitdict['fitflux_err']['value']
        fitpeak = fitdict['fitpeak']['value']
        shortmsg = 'Check source fit successful'

        warnings = []

        if refflux is not None:
            coherence = fitdict['fluxloss']['value'] * 100.0
            flux_score = max(0.0, 1.0 - fitdict['fluxloss']['value'])
            flux_metric = fitdict['fluxloss']['value']
            flux_unit = 'flux loss'
        else:
            flux_score = 0.0
            flux_metric = 'N/A'
            flux_unit = 'flux loss'

        offset_score = 0.0
        offset_metric = 'N/A'
        offset_unit = 'beams'
        if beams is None:
            warnings.append('unfitted offset')
        else:
            offset_score = max(0.0, 1.0 - min(1.0, beams))
            offset_metric = beams
            if beams > 0.15:
                warnings.append('large fitted offset of %.2f marcsec and %.2f synth beam' % (offset, beams))

        fitflux_score = 0.0
        fitflux_metric = 'N/A'
        fitflux_unit = 'fitflux/refflux'
        if gfluxscale is None:
            warnings.append('undefined gfluxscale result')
        elif gfluxscale == 0.0:
            warnings.append('gfluxscale value of 0.0 mJy')
        else:
            chk_fitflux_gfluxscale_ratio = fitflux * 1000. / gfluxscale
            fitflux_score = max(0.0, 1.0 - abs(1.0 - chk_fitflux_gfluxscale_ratio))
            fitflux_metric = chk_fitflux_gfluxscale_ratio
            if chk_fitflux_gfluxscale_ratio < 0.8:
                warnings.append('low [Fitted / gfluxscale] Flux Density Ratio of %.2f' % (chk_fitflux_gfluxscale_ratio))

        fitpeak_score = 0.0
        fitpeak_metric = 'N/A'
        fitpeak_unit = 'fitpeak/fitflux'
        if fitflux is None:
            warnings.append('undefined check fit result')
        elif fitflux == 0.0:
            warnings.append('Fitted Flux Density value of 0.0 mJy')
        else:
            chk_fitpeak_fitflux_ratio = fitpeak / fitflux
            fitpeak_score = max(0.0, 1.0 - abs(1.0 - (chk_fitpeak_fitflux_ratio)))
            fitpeak_metric = chk_fitpeak_fitflux_ratio
            if chk_fitpeak_fitflux_ratio < 0.7:
                warnings.append('low Fitted [Peak Intensity / Flux Density] Ratio of %.2f' % (chk_fitpeak_fitflux_ratio))

        snr_msg = ''
        if gfluxscale is not None and gfluxscale_err is not None:
            if gfluxscale_err != 0.0:
                chk_gfluxscale_snr = gfluxscale / gfluxscale_err
                if chk_gfluxscale_snr < 20.:
                   snr_msg = ', however, the S/N of the gfluxscale measurement is low'

        if any(np.array([offset_score, fitflux_score, fitpeak_score]) < 1.0):
            score = math.sqrt(offset_score * fitflux_score * fitpeak_score)
        else:
            score = offset_score * fitflux_score * fitpeak_score
        metric_score = [offset_metric, fitflux_metric, fitpeak_metric]
        metric_units = '%s, %s, %s' % (offset_unit, fitflux_unit, fitpeak_unit)

        if warnings != []:
            longmsg = '%s field %s spwid %d: has a %s%s' % (msnames, fieldname, spwid, ' and a '.join(warnings), snr_msg)
            # Log warnings only if they would not be logged by the QA system (score <= 0.66)
            if score > 0.66:
                LOG.warn(longmsg)
        else:
            longmsg = 'Check source fit successful'

        if score <= 0.9:
            shortmsg = 'Check source fit not optimal'

    origin = pqa.QAOrigin(metric_name='score_checksources',
                          metric_score=metric_score,
                          metric_units=metric_units)

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, origin=origin), offset, offset_err, beams, beams_err, fitflux, fitflux_err, fitpeak


@log_qa
def score_multiply(scores_list):
    score = reduce(operator.mul, scores_list, 1.0)
    longmsg = 'Multiplication of scores.'
    shortmsg = 'Multiplication of scores.'
    origin = pqa.QAOrigin(metric_name='score_multiply',
                          metric_score=len(scores_list),
                          metric_units='Number of multiplied scores.')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, origin=origin)


@log_qa
def score_sd_skycal_elevation_difference(ms, resultdict):
    """
    """
    field_ids = resultdict.keys()
    assert len(field_ids) == 1
    field_id = field_ids[0]
    field = ms.fields[field_id]
    eldiff = resultdict[field_id]
    el_threshold = 3.0
    warned_antennas = []
    metric_score = []
    for antenna_id, eld in eldiff.items():
        preceding = eld.eldiff0
        subsequent = eld.eldiff1
        max_pred = np.abs(preceding).max()
        max_subq = np.abs(subsequent).max()
        metric_score.extend([max_pred, max_subq])
        if max_pred >= el_threshold or max_subq >= el_threshold:
            warned_antennas.append(antenna_id)
    
    if len(warned_antennas) > 0:
        antenna_names = ', '.join([ms.antennas[a].name for a in warned_antennas])
        longmsg = '{} field {} antennas {}: elevation difference between ON and OFF exceed {}deg'.format(ms.basename,
                                                                                                         field.name,
                                                                                                         antenna_names,
                                                                                                         el_threshold)
    else:
        longmsg = 'Elevation difference between ON and OFF is below threshold ({}deg)'.format(el_threshold)
        
    if np.max(metric_score) >= el_threshold:
        score = 0.0
    else:
        score = 1.0
    origin = pqa.QAOrigin(metric_name='OnOffElevationDifference',
                          metric_score=np.max(metric_score),
                          metric_units='deg')
    
    if score < 1.0:
        shortmsg = 'Elevation difference between ON and OFF exceeds the limit'
    else:
        shortmsg = 'Elevation difference between ON and OFF is below the limit'
    
    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, origin=origin, vis=ms.basename)||||||| .r41742


@log_qa
def score_gfluxscale_k_spw(vis, field, spw_id, k_spw):
    """ Convert internal spw_id-spw_id consistency ratio to a QA score.

    k_spw is equal to the ratio of the derived flux:catalogue flux divided by
    the ratio of derived flux:catalogue flux for the highest SNR window.

    See CAS-10792 for full specification.

    :param k_spw: numeric k_spw ratio, as per spec
    :param vis: name of measurement set to which k_spw applies
    :param field: field domain object to which k_spw applies
    :param spw_id: name of spectral window to which k_spw applies
    :return: QA score
    """

    #     if Q_total < 0.1, QA score 1 = 1.0
    #     if 0.1 <= Q_total < 0.2, QA score 1 = 0.75 (Blue/below standard)
    #     if 0.2 <= Q_total < 0.3, QA score 1 = 0.5 (Yellow/warning)
    #     if Q_total >= 0.3, QA score 1 = 0.2 (Red/Error)
    q_spw = abs(1-k_spw)
    if q_spw < 0.1:
        score = 1.0
    elif q_spw < 0.2:
        score = 0.75
    elif q_spw < 0.3:
        score = 0.5
    else:
        score = 0.2

    longmsg = ('Flux density for {} ({}) in {} spw {} deviates by {:.0%} from the expected value'
               ''.format(utils.dequote(field.name), ','.join(field.intents), vis, spw_id, q_spw))
    shortmsg = 'Internal spw-spw consistency'

    origin = pqa.QAOrigin(metric_name='score_gfluxscale_k_spw',
                          metric_score=float(k_spw),
                          metric_units='Number of spws with missing SNR measurements')

    return pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg, vis=vis, origin=origin)
