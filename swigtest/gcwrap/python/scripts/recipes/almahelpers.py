#!/usr/bin/env python
#
# almahelpers.py
#
# History:
#  v1.0 (scorder, gmoellen, jkern; 2012Apr26) == initial version
#
# This script defines a function that takes the Caltable you wish
# to apply to a MeasurementSet and generates a "apply-cal-ready"
# spwmap that provides the appropriate information regarding
# the transfer Tsys calibration from TDM spectral windows to
# FDM spectral windows.  To import this function, type (at
# the CASA prompt):
#
# from recipies.almahelpers import tsysspwmap
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
import numpy
import taskinit

class SpwMap:
    """
    This object is basically set up to hold the information needed 
    """
    def __init__(self,calSpwId):
        self.calSpwId = calSpwId
        self.validFreqRange = []
        self.mapsToSpw = []
        self.bbNo = None
        
class SpwInfo:
    def __init__(self,msfile,spwId) :
        self.tb = taskinit.tbtool.create()
        self.setTableAndSpwId(msfile,spwId)

    def setTableAndSpwId(self,msfile,spwId) :
        self.setTable(msfile)
        self.setSpwId(spwId)

    def setTable(self,msfile) :
        self.tableName = "%s/SPECTRAL_WINDOW" % msfile
        self.tb.open(self.tableName)
        self.parameters = self.tb.colnames()
        self.tb.close()
        
    def setSpwId(self,spwId) :
        self.tb.open(self.tableName)
        self.values = {}
        for i in self.parameters :
            self.values[i] = self.tb.getcell(i,spwId)
        self.tb.close()
    
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
    evenPoint = compare[-1]
    for i in compare :
        if compare[i:] == spwMap[i:] :
            evenPoint = i
            break
    return spwMap[:i]
        
        
def tsysspwmap(vis,tsystable,trim=True,relax=False) :
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
    localTb = taskinit.tbtool.create()
    spwMaps = []
    # Get the spectral windows with entries in the solution table
    localTb.open(tsystable)
    measuredTsysSpw = numpy.unique(localTb.getcol("SPECTRAL_WINDOW_ID"))
    localTb.close()
    # Get the frequency ranges for the allowed 
    localTb.open("%s/SPECTRAL_WINDOW" % tsystable)
    for i in measuredTsysSpw:
        spwMap = SpwMap(i)
        chanFreqs = localTb.getcell("CHAN_FREQ",i)
        chanWidth = abs(chanFreqs[1]-chanFreqs[0])
        spwMap.validFreqRange = [chanFreqs.min()-0.5*chanWidth,\
                                 chanFreqs.max()+0.5*chanWidth]
        spwMaps.append(spwMap)
    localTb.close()
    # Now loop through the main table's spectral window table
    # to map the spectral windows as desired.
    localTb.open("%s/SPECTRAL_WINDOW" % vis)
    it = localTb.nrows()
    localTb.close()
    for j in spwMaps :
        for i in range(it) :
            localTb.open("%s/SPECTRAL_WINDOW" % vis)
            chanFreqs = localTb.getcell("CHAN_FREQ",i)
            if len(chanFreqs) > 1 :
                chanWidth = localTb.getcell("CHAN_WIDTH",i)[0]
                freqMin = chanFreqs.min()-0.5*chanWidth
                freqMax = chanFreqs.max()+0.5*chanWidth
            else :
                chanWidth = localTb.getcell("CHAN_WIDTH",i)
                freqMin = chanFreqs-0.5*chanWidth
                freqMax = chanFreqs+0.5*chanWidth
            if freqMin >= j.validFreqRange[0] and \
               freqMax <= j.validFreqRange[1] :
                  j.mapsToSpw.append(i)
            localTb.close()
            calSpw = SpwInfo(tsystable,j.calSpwId)
            msSpw  = SpwInfo(vis,i)
            if areIdentical(calSpw,msSpw) :
                j.bbNo = msSpw.values['BBC_NO']
    applyCalSpwMap = []
    localTb.open("%s/SPECTRAL_WINDOW" % vis)
    for i in range(it) :
        useSpw = None
        for j in spwMaps :
            if i in j.mapsToSpw :
                if useSpw is not None :
                    if localTb.getcell("BBC_NO") == j.bbNo :
                        useSpw = j.calSpwId
                else :
                    useSpw = j.calSpwId
        if useSpw == None : useSpw = i
        applyCalSpwMap.append(int(useSpw))        
    if trim :
        return trimSpwmap(applyCalSpwMap)
    else :
        return applyCalSpwMap
            
    
    
