#!/usr/bin/env python

import sys
import numpy as np
import copy
from casac import casac
import scipy.special
import utility.scorers as scorers
import utility.filters as filters


def gpcal(caltable):

    gpcal_stats = gpcal_calc(caltable)
    gpcal_scores = gpcal_score(gpcal_stats)
    gpcal_qa = {'QANUMBERS': gpcal_stats, 'QASCORES': gpcal_scores}

    return gpcal_qa


def gpcal_calc(caltable):

    '''Calculate X-Y and X2-X1 phase calibration statistics for int based
       phase calibration tables.'''

    removeoutliers = True
 
    caLoc = casac.calanalysis()
    caLoc.open(caltable)
    antennaNames = caLoc.antenna1()
    fieldNames = caLoc.field()
    caLoc.close()

    tbLoc = casac.table()
    tbLoc.open(caltable)
 
    fieldIds = np.unique(tbLoc.getcol('FIELD_ID')).tolist()
    antIds = np.unique(tbLoc.getcol('ANTENNA1')).tolist()
 
    if 'SPECTRAL_WINDOW_ID' in tbLoc.colnames():
 
        spwIds = np.unique(tbLoc.getcol('SPECTRAL_WINDOW_ID')).tolist()
        tbLoc.close()
 
        tbLoc.open(caltable + '/SPECTRAL_WINDOW')
        chanfreqs = tbLoc.getcol('CHAN_FREQ')[0]
        tbLoc.close()
 
        caltableformat = 'new'
 
    else:
 
        if 'CAL_DESC' not in tbLoc.keywordnames(): sys.exit('ERROR')
        tbLoc.close()
 
        tbLoc.open(caltable + '/CAL_DESC')
        spwIds = tbLoc.getcol('SPECTRAL_WINDOW_ID')[0].tolist()
        tbLoc.close()
 
        caltableformat = 'old'

    gpcal_stats = {'FIELDS': {}, 'SPWS': {}, 'ANTENNAS': {}, 'STATS': {}}
 
    tbLoc.open(caltable)
 
    for fIndex in xrange(len(fieldIds)):

        fieldId = fieldIds[fIndex]
        gpcal_stats['FIELDS'][fieldId] = fieldNames[fIndex]

        if fieldId not in gpcal_stats['STATS']:
            gpcal_stats['STATS'][fieldId] = {}

        for sIndex in xrange(len(spwIds)):

            spwId = spwIds[sIndex]
            gpcal_stats['SPWS'][spwId] = spwId

            if spwId not in gpcal_stats['STATS'][fieldId]:
                gpcal_stats['STATS'][fieldId][spwId] = {}

            for aIndex in xrange(len(antIds)):

                antId = antIds[aIndex]
 
                gpcal_stats['ANTENNAS'][antId] = antennaNames[aIndex]

                if antId not in gpcal_stats['STATS'][fieldId][spwId]:
                    gpcal_stats['STATS'][fieldId][spwId][antId] = {}

                if caltableformat == 'new':
                    gpcal_stats['STATS'][fieldId][spwId][antId]['chanFreq (GHz)'] = chanfreqs[spwId]/1.e9

                    tb1 = tbLoc.query('FIELD_ID == '+str(fieldId)+' AND SPECTRAL_WINDOW_ID == '+str(spwId)+' AND ANTENNA1 == '+str(antId))
                else:
                    tb1 = tbLoc.query('FIELD_ID == '+str(fieldId)+' AND CAL_DESC_ID == '+str(spwids.index(spwId))+' AND ANTENNA1 == '+str(antId))
 
                ngains = tb1.nrows()
                if ngains == 0:
                    gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (deg)'] = 'C/C'
                    gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (m)'] = 'C/C'
                    gpcal_stats['STATS'][fieldId][spwId][antId]['X2-X1 (deg)'] = 'C/C'
                    gpcal_stats['STATS'][fieldId][spwId][antId]['X2-X1 (m)'] = 'C/C'
                    continue

                if caltableformat == 'new':
                    phase1 = tb1.getcol('CPARAM')
                    flag1 = tb1.getcol('FLAG')
                else:
                    phase1 = tb1.getcol('GAIN')
 
                phase1 = np.angle(phase1)
                phase1 = np.unwrap(phase1)
 

                if len(phase1) == 2:
 
                    phase2 = []
                    for i in range(ngains):
                        # don't use flagged points (see CAS-6804)
                        if ((flag1[0][0][i]==False) and (flag1[1][0][i]==False)):
                            phase2.append( phase1[0][0][i] - phase1[1][0][i] )
 
                    # Unwrap the difference *after* flagging
                    phase2 = np.unwrap(np.array(phase2))

                    # Outlier removal turned on according to new comments in
                    # CAS-6804. Threshold changed to 5 sigma.
                    if removeoutliers == True:
                        phase2 = filters.outlierFilter(phase2, 5.0)
 
                phase3 = []
                for i in range(ngains-1):
                    phase3.append( phase1[0][0][i+1] - phase1[0][0][i] )

                if removeoutliers == True:
                    phase3 = filters.outlierFilter(phase3, 3.0)

                if caltableformat == 'new':

                    if ((len(phase1) == 2) and (len(phase2) != 0)):
                        gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (deg)'] = np.rad2deg(np.std(phase2))
                        gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (m)'] = np.std(phase2)*(2.9979e8/(2*np.pi*chanfreqs[spwId]))
                    else:
                        gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (deg)'] = 'C/C'
                        gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (m)'] = 'C/C'
    
                    if (len(phase3) != 0):
                        gpcal_stats['STATS'][fieldId][spwId][antId]['X2-X1 (deg)'] = np.rad2deg(np.std(phase3))
                        gpcal_stats['STATS'][fieldId][spwId][antId]['X2-X1 (m)'] = np.std(phase3)*(2.9979e8/(2*np.pi*chanfreqs[spwId]))
                    else:
                        gpcal_stats['STATS'][fieldId][spwId][antId]['X2-X1 (deg)'] = 'C/C'
                        gpcal_stats['STATS'][fieldId][spwId][antId]['X2-X1 (m)'] = 'C/C'

                else:
 
                    if ((len(phase1) == 2) and (len(phase2) != 0)):
                        gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (deg)'] = np.rad2deg(np.std(phase2))
                    else:
                        gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (deg)'] = 'C/C'

                    gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (m)'] = 'C/C'
    
                    if (len(phase3) != 0):
                        gpcal_stats['STATS'][fieldId][spwId][antId]['X2-X1 (deg)'] = np.rad2deg(np.std(phase3))
                    else:
                        gpcal_stats['STATS'][fieldId][spwId][antId]['X2-X1 (deg)'] = 'C/C'
                    gpcal_stats['STATS'][fieldId][spwId][antId]['X2-X1 (m)'] = 'C/C'

    tb1.close()
    tbLoc.close()

    return gpcal_stats


def gpcal_score(gpcal_stats):

    '''Calculate scores for phasee statistics.'''

    gpcal_scores = {'SCORES': {'TOTAL': 1.0, 'XY_TOTAL': 1.0, 'X2X1_TOTAL': 1.0}}
    gpcal_scores['FIELDS'] = copy.deepcopy(gpcal_stats['FIELDS'])
    gpcal_scores['SPWS'] = copy.deepcopy(gpcal_stats['SPWS'])
    gpcal_scores['ANTENNAS'] = copy.deepcopy(gpcal_stats['ANTENNAS'])

    # Using average sigmas for now. Eric's report lists sigmas per band / frequency.
    # Need to check if we have to distinguish by band.

    xyScorer = scorers.erfScorer(4.25e-6, 8.4e-5)
    #x2x1Scorer = scorers.erfScorer(3.08e-5, 2.24e-4)
    x2x1Scorer = scorers.erfScorer(3.08e-5, 2.24e-2)

    totalXYMetrics = []

    for fieldId in gpcal_stats['STATS'].iterkeys():

        fieldXYMetrics = []

        if fieldId not in gpcal_scores['SCORES']:
            gpcal_scores['SCORES'][fieldId] = {}
            gpcal_scores['SCORES'][fieldId]['XY_TOTAL'] = 1.0
            gpcal_scores['SCORES'][fieldId]['X2X1_TOTAL'] = 1.0
            gpcal_scores['SCORES'][fieldId]['TOTAL'] = 1.0

        for spwId in gpcal_stats['STATS'][fieldId].iterkeys():

            if spwId not in gpcal_scores['SCORES'][fieldId]:
                gpcal_scores['SCORES'][fieldId][spwId] = {}

            for antId in gpcal_stats['STATS'][fieldId][spwId].iterkeys():

                if antId not in gpcal_scores['SCORES'][fieldId][spwId]:
                    gpcal_scores['SCORES'][fieldId][spwId][antId] = {}

                try:
                    gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_XY'] = xyScorer(gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (m)'])
                    fieldXYMetrics.append(gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (m)'])
                    totalXYMetrics.append(gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (m)'])
                    #gpcal_scores['SCORES'][fieldId]['XY_TOTAL'] = \
                    #    min(gpcal_scores['SCORES'][fieldId]['XY_TOTAL'], gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_XY'])
                    #gpcal_scores['SCORES'][fieldId]['TOTAL'] = \
                    #    min(gpcal_scores['SCORES'][fieldId]['TOTAL'], gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_XY'])
                    #gpcal_scores['SCORES']['XY_TOTAL'] = \
                    #    min(gpcal_scores['SCORES']['XY_TOTAL'], gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_XY'])
                    #gpcal_scores['SCORES']['TOTAL'] = \
                    #    min(gpcal_scores['SCORES']['TOTAL'], gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_XY'])
                except Exception as e:
                    gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_XY'] = 'C/C'
                    # Don't count this in the totals since it is likely due to missing solutions because of
                    # flagged antennas. Need to decide how to account for these cases.

                try:
                    gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_X2X1'] = x2x1Scorer(gpcal_stats['STATS'][fieldId][spwId][antId]['X2-X1 (m)'])
                    gpcal_scores['SCORES'][fieldId]['X2X1_TOTAL'] = \
                        min(gpcal_scores['SCORES'][fieldId]['X2X1_TOTAL'], gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_X2X1'])
                    gpcal_scores['SCORES'][fieldId]['TOTAL'] = \
                        min(gpcal_scores['SCORES'][fieldId]['TOTAL'], gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_X2X1'])
                    gpcal_scores['SCORES']['X2X1_TOTAL'] = \
                        min(gpcal_scores['SCORES']['X2X1_TOTAL'], gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_X2X1'])

                    gpcal_scores['SCORES']['TOTAL'] = \
                        min(gpcal_scores['SCORES']['TOTAL'], gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_X2X1'])
                except Exception as e:
                    gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_X2X1'] = 'C/C'
                    # Don't count this in the totals since it is likely due to missing solutions because of
                    # flagged antennas. Need to decide how to account for these cases.

        fieldXYMetrics = filters.outlierFilter(fieldXYMetrics, 5.0)
        if (len(fieldXYMetrics) > 0):
            gpcal_scores['SCORES'][fieldId]['XY_TOTAL'] = xyScorer(max(fieldXYMetrics))
            gpcal_scores['SCORES'][fieldId]['TOTAL'] = min(gpcal_scores['SCORES'][fieldId]['TOTAL'], gpcal_scores['SCORES'][fieldId]['XY_TOTAL'])
        else:
            gpcal_scores['SCORES'][fieldId]['XY_TOTAL'] = 'C/C'

    totalXYMetrics = filters.outlierFilter(totalXYMetrics, 5.0)
    if (len(totalXYMetrics) > 0):
        gpcal_scores['SCORES']['XY_TOTAL'] = xyScorer(max(totalXYMetrics))
        gpcal_scores['SCORES']['TOTAL'] = min(gpcal_scores['SCORES']['TOTAL'], gpcal_scores['SCORES']['XY_TOTAL'])
    else:
        gpcal_scores['SCORES']['XY_TOTAL'] = 'C/C'

    return gpcal_scores
