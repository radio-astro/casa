#!/usr/bin/env python
#
# tsysspwmap.py
#
# History:
#  v1.0 (scorder, gmoellen, jkern; 2012Apr26) == initial version
#  v1.1 (gmoellen; 2013Mar07) Lots of improvements from Eric Villard
#  v1.2 (ldavis; 2013May15) Ported to pipeline
#
# This script defines several functions useful for ALMA Tsys processing.
#
# tsysspwmap  - generate an "applycal-ready" spwmap for TDM to FDM
#                 transfer of Tsys
#
# For more information about each function type
#
# help tsysspwmap
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils

LOG = infrastructure.get_logger(__name__)

class SpwMap(object):
    """
    This object is basically set up to hold the information needed 
    """
    def __init__(self,calSpwId):
        self.calSpwId = calSpwId
        self.validFreqRange = []
        self.mapsToSpw = []
        self.bbNo = None
        
class SpwInfo(object):
    def __init__(self, mstable, spwId) :
        self.setTableAndSpwId(mstable,spwId)

    def setTableAndSpwId(self,mstable,spwId) :
        self.setTable(mstable)
        self.setSpwId(mstable, spwId)

    def setTable(self,mstable) :
        self.parameters = mstable.colnames()
        
    def setSpwId(self,mstable,spwId) :
        self.values = {}
        for i in self.parameters :
            self.values[i] = mstable.getcell(i,spwId)
    

def areIdentical(spwInfo1,spwInfo2) :
    if len(spwInfo1.parameters) <= len(spwInfo2.parameters) :
        minState = spwInfo1 ; maxState = spwInfo2
    else :
        minState = spwInfo2 ; maxState = spwInfo1
    valueCompare = []
    for i in minState.parameters :
        compare = (minState.values[i] == maxState.values[i])
        if numpy.ndarray not in [type(compare)] :
            compare = numpy.array(compare)
        if compare.all() : valueCompare.append(True)
        else : valueCompare.append(False)
    if False in valueCompare : return False
    else : return True

def trimSpwmap(spwMap) :
    compare = range(len(spwMap))
    for i in compare :
        if compare[i:] == spwMap[i:] :
            break
    return spwMap[:i]
        
        
def tsysspwmap(ms, tsystable, trim=True, relax=False, tsysChanTol=1):
    """
    Generate default spwmap for ALMA Tsys, including TDM->FDM associations
    Input:
     ms        the target MeasurementSet object 
     tsystable  the input Tsys caltable (w/ TDM Tsys measurements)
     trim       if True (the default), return minimum-length spwmap;
                    otherwise the spwmap will be exhaustive and include
                    the high-numbered (and usually irrelevant) wvr
                    spws
     relax      (not yet implemented)
    Output:
     spw list to use in applycal spwmap parameter for the Tsys caltable

     This function takes the Tsys Caltable you wish to apply to a
     MeasurementSet and generates a "applycal-ready" spwmap that
     provides the appropriate information regarding the transfer
     Tsys calibration from TDM spectral windows to FDM spectral
     windows.  To execute the function:

     tsysmap=tsysspwmap(vis='my.ms',tsystable='mytsys.cal')

     tsysmap can then be supplied to the applycal spwmap parameter
     to ensure proper Tsys calibration application.

    """

    spwMaps = []

    # Get the spectral windows with entries in the solution table
    with casatools.TableReader(tsystable) as table:
        measuredTsysSpw = numpy.unique(table.getcol("SPECTRAL_WINDOW_ID"))

    # Get the frequency ranges for the allowed 
    with casatools.TableReader("%s/SPECTRAL_WINDOW" % tsystable) as table:
        for i in measuredTsysSpw:
            spwMap = SpwMap(i)
            chanFreqs = table.getcell("CHAN_FREQ",i)
            chanWidth = abs(chanFreqs[1]-chanFreqs[0])
            spwMap.chanWidth = chanWidth
            spwMap.validFreqRange = [chanFreqs.min()-0.5*chanWidth,\
                                 chanFreqs.max()+0.5*chanWidth]
            spwMaps.append(spwMap)

    # Now loop through the main table's spectral window table
    # to map the spectral windows as desired.
    vis = ms.name
    with casatools.TableReader ("%s/SPECTRAL_WINDOW" % vis) as table:
        it = table.nrows()

    for j in spwMaps :
        with casatools.TableReader("%s/SPECTRAL_WINDOW" % vis) as table:
            j.bbNo = table.getcell("BBC_NO",j.calSpwId)
        for i in range(it) :
            with casatools.TableReader("%s/SPECTRAL_WINDOW" % vis) as table:
                chanFreqs = table.getcell("CHAN_FREQ",i)
                if len(chanFreqs) > 1 :
                    chanWidth = table.getcell("CHAN_WIDTH",i)[0]
                    freqMin = chanFreqs.min()-0.5*chanWidth
                    freqMax = chanFreqs.max()+0.5*chanWidth
                else :
                    chanWidth = table.getcell("CHAN_WIDTH",i)
                    freqMin = chanFreqs-0.5*chanWidth
                    freqMax = chanFreqs+0.5*chanWidth
                msSpw  = SpwInfo(table,i)
                if j.bbNo == msSpw.values['BBC_NO']:
                    if freqMin >= j.validFreqRange[0]-tsysChanTol*j.chanWidth and \
                       freqMax <= j.validFreqRange[1]+tsysChanTol*j.chanWidth :
                        j.mapsToSpw.append(i)

    applyCalSpwMap = []
    spwWithoutMatch = []
    with casatools.TableReader("%s/SPECTRAL_WINDOW" % vis) as table:
        for i in range(it) :
            useSpw = None
            for j in spwMaps :
                if i in j.mapsToSpw :
                    if useSpw is not None :
                        if table.getcell("BBC_NO") == j.bbNo :
                            useSpw = j.calSpwId
                    else :
                        useSpw = j.calSpwId
            if useSpw == None :
                useSpw = i
                spwWithoutMatch.append(i)
            applyCalSpwMap.append(int(useSpw))        

    science_window_ids = list(set([spw.id for spw in ms.get_spectral_windows(science_windows_only=True)]))
    unmatched_science_spws = list(set(spwWithoutMatch).intersection(science_window_ids))

    if len(unmatched_science_spws) != 0:
        no_match = utils.commafy(unmatched_science_spws, False)
        LOG.info('No Tsys match found for spws %s.' % no_match) 

    if trim :
        LOG.info('Computed tsysspwmap is: '+str(trimSpwmap(applyCalSpwMap)))
        #return spwWithoutMatch, trimSpwmap(applyCalSpwMap)
        return unmatched_science_spws, trimSpwmap(applyCalSpwMap)
    else :
        LOG.info('Computed tsysspwmap is: '+str(applyCalSpwMap))
        #return spwWithoutMatch, applyCalSpwMap
        return unmatched_science_spws, applyCalSpwMap

