#!/usr/bin/env python
#
# almahelpers.py
#
# History:
#  v1.0 (scorder, gmoellen, jkern; 2012Apr26) == initial version
#  v1.1 (gmoellen; 2013Mar07) Lots of improvements from Eric Villard
#  v1.2 (gmoellen; 2013Apr09) Added fixsyscaltimes and calantsub methods
#                             to handle Tsys irregularities
#
# This script defines several functions useful for ALMA Tsys processing.
#
# tsysspwmap  - generate an "applycal-ready" spwmap for TDM to FDM
#                 transfer of Tsys
# fixsyscaltimes - repairs the TIME and INTERVAL columns of the MS
#                    SYSCAL subtable so that gencal properly generates
#                    the Tsys caltable
# calantsub - provides for substitution of cal solutions by antenna
#
# To access these functions, type (at the CASA prompt):
#
# from recipes.almahelpers import *
#
# For more information about each function type
#
# help tsysspwmap
# help fixsyscaltimes
# help calantsub

#
#
import numpy
import taskinit
import os

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
        self.tb = taskinit.tbtool()
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
        
        
def tsysspwmap(vis,tsystable,trim=True,relax=False, tsysChanTol=0):
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

     This function takes the Tsys Caltable you wish to apply to a
     MeasurementSet and generates a "applycal-ready" spwmap that
     provides the appropriate information regarding the transfer
     Tsys calibration from TDM spectral windows to FDM spectral
     windows.  To execute the function:

     tsysmap=tsysspwmap(vis='my.ms',tsystable='mytsys.cal')

     tsysmap can then be supplied to the applycal spwmap parameter
     to ensure proper Tsys calibration application.

    """

    localTb = taskinit.tbtool()
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
        spwMap.chanWidth = chanWidth
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
        localTb.open("%s/SPECTRAL_WINDOW" % vis)
        j.bbNo = localTb.getcell("BBC_NO",j.calSpwId)
        localTb.close()
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
            msSpw  = SpwInfo(vis,i)
            if j.bbNo == msSpw.values['BBC_NO']:
                if freqMin >= j.validFreqRange[0]-tsysChanTol*j.chanWidth and \
                   freqMax <= j.validFreqRange[1]+tsysChanTol*j.chanWidth :
                    j.mapsToSpw.append(i)
            localTb.close()
    applyCalSpwMap = []
    spwWithoutMatch = []
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
        if useSpw == None :
            useSpw = i
            spwWithoutMatch.append(i)
        applyCalSpwMap.append(int(useSpw))        
    if len(spwWithoutMatch) != 0:
        taskinit.casalog.post('Found no match for following spw ids: '+str(spwWithoutMatch))
    if trim :
        return trimSpwmap(applyCalSpwMap)
    else :
        return applyCalSpwMap


def fixsyscaltimes(vis,newinterval=2.0):
    """
    Fix TIME,INTERVAL columns in MS SYSCAL subtable
    Input:
     vis          the MS containing the offending SYSCAL subtable
     newinterval  the interval to use in revised entries

     This function is intended to repair MS SYSCAL tables that suffer from
     multiple TIME values (over antennas) per Tsys measurement.  The gencal
     task (mode='tsys' expects all antennas to share the same TIME value
     for each Tsys measurement (and this is usually true).  The function
     finds those measurements that have multiple TIMEs and replaces them
     with a common TIME value which takes the value
     mean(oldTIME-INTERVAL/2)+newinterval/2.
     Usually (always?), oldTIME-INTERVAL/2 is constant over antennas
     and represents the physical timestamp of the Tsys measurment.
     If the function finds no pathological timestamps, it does not
     revise the table.
    """

    import pylab as mypl
    import math as mymath
    myqa=taskinit.qatool()
    mytb=taskinit.tbtool()
    mytb.open(vis+'/SYSCAL',nomodify=False)

    spws=mypl.unique(mytb.getcol("SPECTRAL_WINDOW_ID"))

    for ispw in spws:
        st=mytb.query('SPECTRAL_WINDOW_ID=='+str(ispw),name='byspw')
        times=st.getcol('TIME')
        interval=st.getcol('INTERVAL')
        timestamps=times-interval/2
        t0=86400.0*mymath.floor(timestamps[0]/86400.0)

        utimes=mypl.unique(times-t0)
        nT=len(utimes)
        utimestamps=mypl.unique(mypl.floor(timestamps)-t0)
        nTS=len(utimestamps)
        
        msg='In spw='+str(ispw)+' found '+str(nTS)+' Tsys measurements with '+str(nT)+' TIMEs...'
        if nT==nTS:
            msg+='OK.'
            print msg

        else:
            msg+=' which is too many, so fixing it:'
            print msg 

            for uts in utimestamps:
                mask = ((mypl.floor(timestamps))-t0==uts)
                uTIMEs=mypl.unique(times[mask])
                nTIMEs=len(uTIMEs)
                newtime = mypl.mean(times[mask]-interval[mask]/2) + newinterval/2
                msg='  Found '+str(nTIMEs)+' TIMEs at timestamp='+str(myqa.time(str(newtime-newinterval/2)+'s',form='ymd')[0])
                if nTIMEs>1:
                    msg+=':'
                    print msg
                    print '   TIMEs='+str([myqa.time(str(t)+'s',form='ymd')[0] for t in uTIMEs])+' --> '+str(myqa.time(str(newtime)+'s',form='ymd')[0])+' w/ INTERVAL='+str(newinterval)
                    times[mask]=newtime
                    interval[mask]=newinterval
                    st.putcol('TIME',times)
                    st.putcol('INTERVAL',interval)
                else:
                    msg+='...ok.'
                    print msg
        st.close()
    mytb.close()


def calantsub(incaltable,outcaltable='',
              spw='',scan='',
              ant='',subant=''):

    """
    Substitute cal solutions by antenna
    Input:
     incaltable  Input caltable
     outcaltable Output caltable (if '', overwrite result on incaltable)
     spw         Spectral Window selection (no channel selection permitted)
     scan        Scan selection
     ant         Antenna (indices) which need replaced solutions
     subant      Antenna (indices) with which to replace those in ant

    This function provides a means to replace solutions by antenna,
    e.g., to substitute one antenna's Tsys spectra with another.

    The processing can be limited to specific spectral windows and/or
    scans.  The spw and scan parameters should be specified in the
    standard MS selection manner (comma-separated integers in a string),
    except no channel selection is supported.

    The ant parameter specifies one or more antenna indices 
    (comma-separated in a string) for which solutions are to be
    replaced.  The subant parameter lists the antenna indices
    from which the substitute solutions are to be obtained. E.g.,
    ant='3,5,7',subant='6,8,10' will cause the solutions from
    antenna id 6 to be copied to antenna id 5, id 8 to id 5 and id 10
    to id 7.  The number of antennas specified in ant and subant
    must match.
    
    """

    import pylab as mypl
    
    # trap insufficient ant subant specifications
    if len(ant)==0 or len(subant)==0:
        raise Exception, "Must specify at least one ant and subant."

    antlist=ant.split(',')
    sublist=subant.split(',')

    # trap dumb cases
    nant=len(antlist)
    nsub=len(sublist)
    if nant!=nsub:
        raise Exception, "Must specify equal number of ant and subant."

    # local tb tool
    mytb=taskinit.tbtool()

    # parse selection
    selstr=''
    if len(spw)>0:
        selstr+='SPECTRAL_WINDOW_ID IN ['+spw+']'
        if len(scan)>0:
            selstr+=' && '
    if len(scan)>0:
        selstr+='SCAN_NUMBER IN ['+scan+']'
        print "selstr = '"+selstr+"'"

    # verify selection (if any) selects non-zero rows
    if len(selstr)>0:
        mytb.open(incaltable)
        st=mytb.query(query=selstr)
        nselrows=st.nrows()
        st.close()
        mytb.close()
        if nselrows==0:
            raise Exception, 'Error: scan and/or spw selection selects no rows!'

    # manage the output table
    if outcaltable=='':
        outcaltable=incaltable
        print "No outcaltable specified; will overwrite incaltable."
    if outcaltable!=incaltable:
        os.system('cp -r '+incaltable+' '+outcaltable)

    # open the output table for adjustment
    mytb.open(outcaltable,nomodify=False)

    stsel=mytb
    if len(selstr)>0:
        stsel=mytb.query(query=selstr,name='selected')

    # cols to substitute:
    collist=['TIME','INTERVAL','PARAMERR','SNR','FLAG']
    cols=mytb.colnames()
    if cols.count('CPARAM')>0:
        collist.append('CPARAM')
    else:
        collist.append('FPARAM')

    # scan list
    scans=mypl.unique(stsel.getcol('SCAN_NUMBER'))

    print 'Found scans = ',scans

    # do one scan at a time
    for scan in scans:
        st1=stsel.query(query='SCAN_NUMBER=='+str(scan),name='byscan')
        spws=mypl.unique(st1.getcol('SPECTRAL_WINDOW_ID'))

        print 'Scan '+str(scan)+' has spws='+str(spws)

        # do one spw at a time
        for ispw in spws:
            st2=st1.query(query='SPECTRAL_WINDOW_ID=='+str(ispw),name='byspw');

            for ia in range(nant):
                stsub=st2.query(query='ANTENNA1=='+sublist[ia],
                                name='subant')
                stant=st2.query(query='ANTENNA1=='+antlist[ia],
                                name='ant')


                # go to next ant if nothing to do
                if stant.nrows()<1:
                    continue

                print ' scan='+str(scan)+' spw='+str(ispw)+' ants: '+str(sublist[ia])+'->'+str(antlist[ia])

                # trap (unlikely?) pathological case
                if stsub.nrows()!=stant.nrows():
                    raise Exception, "In spw "+str(ispw)+" antenna ids "+str(antlist[ia])+" and "+str(sublist[ia])+" have a different number of solutions."

                # substitute values 
                for col in collist:
                    stant.putcol(col,stsub.getcol(col))

                stsub.close()
                stant.close()
            st2.close()
        st1.close()
    stsel.close()
    mytb.close()
