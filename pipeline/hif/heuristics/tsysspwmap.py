#!/usr/bin/env python

# almahelpers.py

# History:
#  v1.0 (scorder, gmoellen, jkern; 2012Apr26) == initial version
#  v1.1 (jlightfoot; 2012May15) == heuristics version, rejigged to use
#       'with' construct for table access.
#  v1.2 (jlightfoot; 2012May24) == extended Tsys bandwidth by 0.5 
#       chanWidth to allow for FDM overlapping end of matching TDM in
#       some cases (message from scorder)

# This script defines a function that takes the Caltable you wish
# to apply to a MeasurementSet and generates a "apply-cal-ready"
# spwmap that provides the appropriate information regarding
# the transfer Tsys calibration from TDM spectral windows to
# FDM spectral windows.  To import this function, type (at
# the CASA prompt):
#
# from pipeline.heuristics import tsysspwmap
#
# and then execute the function:
#
# tsysmap=tsysspwmap(vis='my.ms',tsystable='mytsys.cal')
#
# tsysmap can then be supplied to the applycal spwmap function
#  to ensure proper Tsys calibration application
#
# Type 'help tsysspwmap' for more info.
#

import numpy as np

import pipeline.infrastructure.casatools as casatools

class SpwMap:
    """
    This object is basically set up to hold the information needed 
    """
    def __init__(self, calSpwId):
        self.calSpwId = calSpwId
        self.validFreqRange = []
        self.mapsToSpw = []
        self.bbNo = None


class SpwInfo:
    def __init__(self, msfile, spwId):
        self.tableName = "%s/SPECTRAL_WINDOW" % msfile
        with casatools.TableReader(self.tableName) as table:
            self.parameters = table.colnames()
            self.values = {}
            for i in self.parameters :
                self.values[i] = table.getcell(i, spwId)

    
def areIdentical(spwInfo1,spwInfo2) :

    if len(spwInfo1.parameters) <= len(spwInfo2.parameters) :
        minState = spwInfo1
        maxState = spwInfo2
    else :
        minState = spwInfo2
        maxState = spwInfo1

    valueCompare = []

    for i in minState.parameters :
        compare = (minState.values[i] == maxState.values[i])
        if np.ndarray not in [type(compare)] :
            compare = np.array(compare)
        if compare.all():
            valueCompare.append(True)
        else:
            valueCompare.append(False)

    if False in valueCompare:
        return False
    else:
        return True


def trimSpwmap(spwMap) :
    compare = range(len(spwMap))
    for i in compare :
        if compare[i:] == spwMap[i:] :
            break

    return spwMap[:i]

        
def tsysspwmap(vis,tsystable,trim=True,relax=False):
    """
    Generate default spwmap for ALMA Tsys, including TDM->FDM associations
    Input:
     vis        the target MeasurementSet 
     tsystable  the input Tsys caltable (w/ TDM Tsys measurements)
     trim       if True (the default), return minimum-length spwmap;
                    otherwise the spwmap will be exhaustive and include
                    the high-numbered (and usually irrelevant) wvr
                    spws
     relax      (not yet implemented)

    Output:
     spw list to use in applycal spwmap parameter for the Tsys caltable
    """

    spwMaps = []

    # Get the spectral windows with entries in the Tsys table
    with casatools.TableReader(tsystable) as table:
        measuredTsysSpw = np.unique(table.getcol("SPECTRAL_WINDOW_ID"))

    # Get the frequency ranges for these spws - range extended by 0.5 
    # chanWidth from channel centres to allow for FDM extending outside 
    # nominal TDM range by up to 0.5 chanWidth
    with casatools.TableReader("%s/SPECTRAL_WINDOW" % tsystable) as table:
        for i in measuredTsysSpw:
            spwMap = SpwMap(i)
            chanFreqs = table.getcell("CHAN_FREQ", i)
            chanWidth = abs(chanFreqs[1] - chanFreqs[0])
            spwMap.validFreqRange = [min(chanFreqs)-chanWidth,
             max(chanFreqs)+chanWidth]
            spwMaps.append(spwMap)

    # Now loop through the main table's spectral window table
    # to map the spectral windows as desired.
    with casatools.TableReader("%s/SPECTRAL_WINDOW" % vis) as table:
        it = table.nrows()
        for i in range(it):
            chanFreqs = table.getcell("CHAN_FREQ",i)

            if len(chanFreqs) > 1 :
                chanWidth = table.getcell("CHAN_WIDTH",i)[0]
                freqMin = chanFreqs.min()-0.5*chanWidth
                freqMax = chanFreqs.max()+0.5*chanWidth
            else :
                chanWidth = table.getcell("CHAN_WIDTH",i)
                freqMin = chanFreqs-0.5*chanWidth
                freqMax = chanFreqs+0.5*chanWidth

            for j in spwMaps :
                if freqMin >= j.validFreqRange[0] and \
                 freqMax <= j.validFreqRange[1] :
                    j.mapsToSpw.append(i)


    # are they identical
    for j in spwMaps:
        calSpw = SpwInfo(tsystable, j.calSpwId)
        for i in j.mapsToSpw:
            msSpw  = SpwInfo(vis, i)

            if areIdentical(calSpw, msSpw) :
                j.bbNo = msSpw.values['BBC_NO']

    # finally go through all MS spws and assign one tsys spw
    applyCalSpwMap = []
    with casatools.TableReader("%s/SPECTRAL_WINDOW" % vis) as table:
        it = table.nrows()
        for i in range(it):
            useSpw = None

            for j in spwMaps :
                if i in j.mapsToSpw :
                    if useSpw is not None :
                        if table.getcell("BBC_NO") == j.bbNo :
                            useSpw = j.calSpwId
                    else :
                        useSpw = j.calSpwId

            if useSpw is None:
                useSpw = i

            applyCalSpwMap.append(int(useSpw))        

    if trim:
        return trimSpwmap(applyCalSpwMap)
    else:
        return applyCalSpwMap
