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
import csv # used by editIntentscsv

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

def editIntentscsv(intentcsv, dryrun=False, verbose=False):
    """
    Reads a list of parameters from a csv text file and runs editIntents on them.
    File format: 
    # any number of comment lines
    vis,field,scan,"WVR,BANDPASS"
    vis,field,,'FLUX,WVR'
    ...

    * field is a required argument, while scan is optional
    * scan: can be a single integer, integer list, or comma-delimited string list
         multiple scans or intents must be quoted, either single or double quote
    -Todd Hunter
    """
    if (not os.path.exists(intentcsv)):
        print "Could not find intentcsv file: ", intentcsv
        return
    f = open(intentcsv,'r')
    lines = f.readlines()
    f.close() 
    for originalLine in lines:
        if (originalLine[0] == '#'): continue
        line = originalLine.replace("'",'"').replace(' ','')
        if verbose:
            print "Parsing reformatted line: ", line[:-1]
        if (len(line.split(',')) > 3):
            token = []
            for i in csv.reader([line]):
                token.append(i)
            token = token[0]
            if (len(token) < 4):
                print "Skipping invalid line: ", originalLine
                continue
            vis = token[0].strip()
            field = token[1].strip()
            scan = token[2].strip()
            intents = token[3].strip()
            if (os.path.exists(vis)):
                cmd = "au.editIntents('%s','%s','%s','%s')" % (vis,field,scan,intents)
                if dryrun:
                    print "Would call %s\n" % (cmd)
                else:
                    print "Calling %s" % (cmd)
                    editIntents(vis,field,scan,intents)
            else:
                print "Could not find requested measurement set: ", vis

def createCasaTool(mytool):
    """
    A wrapper to handle the changing ways in which casa tools are invoked.
    Todd Hunter
    """
    if (type(casac.Quantity) != type):  # casa 4.x
        myt = mytool()
    else:  # casa 3.x
        myt = mytool.create()
    return(myt)

def editIntents(msName='', field='', scan='', newintents='', 
                append=False, verbose=True):
    """
    Change the observation intents for a specified field.  
    Inputs:
    * field: required argument (integer or string ID, or name)
    * scan: optional, can be a single integer, integer list, or comma-delimited string list
    * append: set to True to add the specified intent to the existing intents
    * newintents: enter as a python list, or as single comma-delimited string
    * valid intents: the first 12 are simply a shorthand way to specify the latter 12
       'AMPLITUDE', 'ATMOSPHERE', 'BANDPASS', 'CHECK_SOURCE', 'WVR',
       'DELAY', 'FLUX', 'FOCUS', 'PHASE', 'POINTING', 'SIDEBAND_RATIO', 'TARGET',
       'CALIBRATE_AMPLI', 'CALIBRATE_ATMOSPHERE', 'CALIBRATE_BANDPASS',
       'CALIBRATE_DELAY', 'CALIBRATE_FLUX', 'CALIBRATE_FOCUS',
       'CALIBRATE_PHASE', 'CALIBRATE_POINTING', 'CALIBRATE_SIDEBAND_RATIO',
       'OBSERVE_TARGET', 'CALIBRATE_WVR', 'OBSERVE_CHECK_SOURCE'
      The modifier #ON_SOURCE will be added to all specified intents.
    - T. Hunter
    """
    validIntents = ['AMPLITUDE','ATMOSPHERE','BANDPASS','CHECK_SOURCE',
                    'DELAY','FLUX','FOCUS', 'PHASE','POINTING', 
                    'SIDEBAND_RATIO', 'TARGET','WVR', 'CALIBRATE_AMPLI', 
                    'CALIBRATE_ATMOSPHERE', 'CALIBRATE_BANDPASS',
                    'CALIBRATE_DELAY', 'CALIBRATE_FLUX', 'CALIBRATE_FOCUS',
                    'CALIBRATE_PHASE', 'CALIBRATE_POINTING', 
                    'CALIBRATE_SIDEBAND_RATIO','OBSERVE_TARGET',
                    'CALIBRATE_WVR', 'OBSERVE_CHECK_SOURCE'
                    ]
    if (msName == ''):
        print "You must specify a measurement set."
        return
    if (field == ''):
        print "You must specify a field ID or name."
        return
    mytb = createCasaTool(tbtool)
    mytb.open(msName + '/FIELD')
    fieldnames = mytb.getcol('NAME')
    if verbose:
        print "Found fieldnames = ", fieldnames
    mytb.close()

    mytb.open(msName + '/STATE')
    intentcol = mytb.getcol('OBS_MODE')
    intentcol = intentcol
    mytb.close()

    mytb.open(msName, nomodify=False)
    naddedrows = 0
    if (type(newintents)==list):
        desiredintents = ''
        for n in newintents:
            desiredintents += n
            if (n != newintents[-1]):
                desiredintents += ','
    else:
        desiredintents = newintents
    desiredintentsList = desiredintents.split(',')
        
    for intent in desiredintentsList:
        if ((intent in validIntents)==False):
            print "Invalid intent = %s.  Valid intents = %s" % (intent,validIntents)
            mytb.close()
            return
    foundField = False
    if (type(scan) != list):
        scan = str(scan)
    for id,name in enumerate(fieldnames):
      if (name == field or str(id) == str(field)):
        foundField = True
        if verbose:
            print 'FIELD_ID %s has name %s' % (id, name)
        if scan == '': 
            s = mytb.query('FIELD_ID==%s' % id)
            if verbose:
                print "Got %d rows in the ms matching field=%s" % (s.nrows(),id)
        else:
            if (type(scan) == str):
                scans = [int(x) for x in scan.split(',')]
            elif (type(scan) != list):
                scans = [scan]
            else:
                scans = scan
            if verbose:
                print "Querying: 'FIELD_ID==%s AND SCAN_NUMBER in %s'" % (id, str(scans))
            s = mytb.query('FIELD_ID==%s AND SCAN_NUMBER in %s' % (id, str(scans)))
            if verbose:
                print "Got %d rows in the ms matching field=%s and scan in %s" % (s.nrows(),id,str(scans))
        if (s.nrows() == 0):
            mytb.close()
            print "Found zero rows for this field (and/or scan). Stopping."
            return
        state_ids = s.getcol('STATE_ID')
        # original code from J. Lightfoot, can probably be replaced
        # by the np.unique() above
        states = []
        for state_id in state_ids:
            if state_id not in states:
                states.append(state_id)
        for ij in range(len(states)):
            states[ij] = intentcol[states[ij]]

        print 'current intents are:'
        for state in states:
            print state

        if append == False: states = []
        for desiredintent in desiredintentsList:
            if desiredintent.find('TARGET')>=0:
                states.append('OBSERVE_TARGET#ON_SOURCE')
            elif desiredintent.find('CHECK_SOURCE')>=0:
                states.append('OBSERVE_CHECK_SOURCE#ON_SOURCE')
            elif desiredintent.find('BANDPASS')>=0:
                states.append('CALIBRATE_BANDPASS#ON_SOURCE')
            elif desiredintent.find('PHASE')>=0:
                states.append('CALIBRATE_PHASE#ON_SOURCE')
            elif desiredintent.find('AMPLI')>=0:
                states.append('CALIBRATE_AMPLI#ON_SOURCE')
            elif desiredintent.find('FLUX')>=0:
                states.append('CALIBRATE_FLUX#ON_SOURCE')
            elif desiredintent.find('ATMOSPHERE')>=0:
                states.append('CALIBRATE_ATMOSPHERE#ON_SOURCE')
            elif desiredintent.find('WVR')>=0:
                states.append('CALIBRATE_WVR#ON_SOURCE')
            elif desiredintent.find('SIDEBAND_RATIO')>=0:
                states.append('CALIBRATE_SIDEBAND_RATIO#ON_SOURCE')
            elif desiredintent.find('DELAY')>=0:
                states.append('CALIBRATE_DELAY#ON_SOURCE')
            elif desiredintent.find('FOCUS')>=0:
                states.append('CALIBRATE_FOCUS#ON_SOURCE')
            elif desiredintent.find('POINTING')>=0:
                states.append('CALIBRATE_POINTING#ON_SOURCE')
            else:
                print "Unrecognized intent = %s" % desiredintent
                continue
            print 'setting %s' % (states[-1])

        if states != []:
            state = reduce(lambda x,y: '%s,%s' % (x,y), states)
            if state not in intentcol:
                if verbose:
                    print 'adding intent to state table'
                intentcol = list(intentcol)
                intentcol.append(state)
                intentcol = np.array(intentcol)
                state_id = len(intentcol) - 1
                naddedrows += 1
                if verbose:
                    print 'state_id is', state_id
                state_ids[:] = state_id
            else:
                if verbose:
                    print 'intent already in state table'
                state_id = np.arange(len(intentcol))[intentcol==state]
                if verbose:
                    print 'state_id is', state_id
                if (type(state_id) == list or type(state_id)==np.ndarray):
                    # ms can have identical combinations of INTENT, so just
                    # pick the row for the first appearance - T. Hunter
                    state_ids[:] = state_id[0]
                else:
                    state_ids[:] = state_id
            s.putcol('STATE_ID', state_ids)
    if (foundField == False):
        print "Field not found"
        return
    mytb.close()

    if verbose:
        print 'writing new STATE table'
    mytb.open(msName + '/STATE', nomodify=False)
    if naddedrows > 0:
        mytb.addrows(naddedrows)
    mytb.putcol('OBS_MODE', intentcol)
    mytb.close()

