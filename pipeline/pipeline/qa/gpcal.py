#!/usr/bin/env python

import sys
import numpy as np
import copy
from casac import casac
import scipy.special


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

    gpcal_stats = {'FIELDS': {}, 'ANTENNAS': {}, 'STATS': {}}
 
    tbLoc.open(caltable)
 
    for fIndex in xrange(len(fieldIds)):

        fieldId = fieldIds[fIndex]
        gpcal_stats['FIELDS'][fieldId] = fieldNames[fIndex]

        if fieldId not in gpcal_stats['STATS']:
            gpcal_stats['STATS'][fieldId] = {}

        for sIndex in xrange(len(spwIds)):

            spwId = spwIds[sIndex]

            if spwId not in gpcal_stats['STATS'][fieldId]:
                gpcal_stats['STATS'][fieldId][spwId] = {}

            for aIndex in xrange(len(antIds)):

                antId = antIds[aIndex]
 
                gpcal_stats['ANTENNAS'][antId] = antennaNames[aIndex]

                if antId not in gpcal_stats['STATS'][fieldId][spwId]:
                    gpcal_stats['STATS'][fieldId][spwId][antId] = {}

                if caltableformat == 'new':
                    tb1 = tbLoc.query('FIELD_ID == '+str(fieldId)+' AND SPECTRAL_WINDOW_ID == '+str(spwId)+' AND ANTENNA1 == '+str(antId))
                else:
                    tb1 = tbLoc.query('FIELD_ID == '+str(fieldId)+' AND CAL_DESC_ID == '+str(spwids.index(spwId))+' AND ANTENNA1 == '+str(antId))
 
                ngains = tb1.nrows()
                if ngains == 0:
                    continue

                if caltableformat == 'new':
                    phase1 = tb1.getcol('CPARAM')
                else:
                    phase1 = tb1.getcol('GAIN')
 
                phase1 = np.angle(phase1)
                phase1 = np.unwrap(phase1)
 
                if len(phase1) == 2:
 
                    phase2 = []
                    for i in range(ngains):
                        phase2.append( phase1[0][0][i] - phase1[1][0][i] )
 
                    if removeoutliers == True:
                        phase2Median = np.median(phase2)
                        phase2MAD = np.median(abs(phase2-phase2Median)) / 0.6745
                        phase2 = [kl for kl in phase2 if abs(kl-phase2Median) < 3*phase2MAD]
 
                    if len(phase2) == 0: continue

                phase3 = []
                for i in range(ngains-1):
                    phase3.append( phase1[0][0][i+1] - phase1[0][0][i] )

                if removeoutliers == True:
                    phase3Median = np.median(phase3)
                    phase3MAD = np.median(abs(phase3-phase3Median)) / 0.6745
                    phase3 = [kl for kl in phase3 if abs(kl-phase3Median) < 3*phase3MAD]

                if len(phase3) == 0: continue

                if caltableformat == 'new':

                    gpcal_stats['STATS'][fieldId][spwId][antId]['chanFreq (GHz)'] = chanfreqs[spwId]/1.e9

                    if len(phase1) == 2:
                        gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (deg)'] = np.rad2deg(np.std(phase2))
                        gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (m)'] = np.std(phase2)*(2.9979e8/(2*np.pi*chanfreqs[spwId]))
    
                    gpcal_stats['STATS'][fieldId][spwId][antId]['X2-X1 (deg)'] = np.rad2deg(np.std(phase3))
                    gpcal_stats['STATS'][fieldId][spwId][antId]['X2-X1 (m)'] = np.std(phase3)*(2.9979e8/(2*np.pi*chanfreqs[spwId]))

                else:
 
                    if len(phase1) == 2:
                        gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (deg)'] = np.rad2deg(np.std(phase2))
    
                    gpcal_stats['STATS'][fieldId][spwId][antId]['X2-X1 (deg)'] = np.rad2deg(np.std(phase3))

    tb1.close()
    tbLoc.close()

    return gpcal_stats


def gpcal_score(gpcal_stats):

    '''Calculate scores for phasee statistics.'''

    gpcal_scores = {'SCORES': {'TOTAL': 1.0, 'XY_TOTAL': 1.0, 'X2X1_TOTAL': 1.0}}
    gpcal_scores['FIELDS'] = copy.deepcopy(gpcal_stats['FIELDS'])
    gpcal_scores['ANTENNAS'] = copy.deepcopy(gpcal_stats['ANTENNAS'])

    # Using average sigmas for now. Eric's report lists sigmas per band / frequency.
    # Need to check if we have to distinguish by band.

    xySig1 = 4.25e-6
    xySig3 = 7.955e-5
    xyM = 6.0 / np.sqrt(2.0) / (xySig1 - xySig3)
    xyB = 3.0 / np.sqrt(2.0) * (1.0 - 2.0 * xySig1 / (xySig1 - xySig3))

    x2x1Sig1 = 3.08e-5
    x2x1Sig3 = 2.24e-4
    x2x1M = 6.0 / np.sqrt(2.0) / (x2x1Sig1 - x2x1Sig3)
    x2x1B = 3.0 / np.sqrt(2.0) * (1.0 - 2.0 * x2x1Sig1 / (x2x1Sig1 - x2x1Sig3))


    for fieldId in gpcal_stats['STATS'].iterkeys():

        if fieldId not in gpcal_scores['SCORES']:
            gpcal_scores['SCORES'][fieldId] = {}

        for spwId in gpcal_stats['STATS'][fieldId].iterkeys():

            if spwId not in gpcal_scores['SCORES'][fieldId]:
                gpcal_scores['SCORES'][fieldId][spwId] = {}

            for antId in gpcal_stats['STATS'][fieldId][spwId].iterkeys():

                if antId not in gpcal_scores['SCORES'][fieldId][spwId]:
                    gpcal_scores['SCORES'][fieldId][spwId][antId] = {}

                try:
                    gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_XY'] = (scipy.special.erf(xyM * gpcal_stats['STATS'][fieldId][spwId][antId]['X-Y (m)'] + xyB) + 1.0) / 2.0
                    gpcal_scores['SCORES']['TOTAL'] = min(gpcal_scores['SCORES']['TOTAL'], gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_XY'])
                    gpcal_scores['SCORES']['XY_TOTAL'] = min(gpcal_scores['SCORES']['XY_TOTAL'], gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_XY'])
                    gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_X2X1'] = (scipy.special.erf(x2x1M * gpcal_stats['STATS'][fieldId][spwId][antId]['X2-X1 (m)'] + x2x1B) + 1.0) / 2.0
                    gpcal_scores['SCORES']['TOTAL'] = min(gpcal_scores['SCORES']['TOTAL'], gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_X2X1'])
                    gpcal_scores['SCORES']['X2X1_TOTAL'] = min(gpcal_scores['SCORES']['X2X1_TOTAL'], gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_X2X1'])
                except Exception as e:
                    gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_XY'] = 0.0
                    gpcal_scores['SCORES'][fieldId][spwId][antId]['PHASE_SCORE_X2X1'] = 0.0
                    # Don't count these in the totals since they are likely due to missing solutions because of
                    # flagged antennas. Need to decide how to account for these cases.

    return gpcal_scores
