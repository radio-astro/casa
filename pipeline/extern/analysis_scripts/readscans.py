######################################################################
#
# readscans.py
#
#    readscans: reads Scan.xml and Source.xml SDM table and parses
#               into returned dictionary
#

# Created  S.T. Myers 2010-12-01  v1.0 from readflags.py
# Modified T.R. Hunter 2010-12-01  v1.1 to print durations in listobs
#           
# Usage:
#         from readscans import *
#         sdmfile = '10B-148.sb2269046.eb2519548.55530.44014204861'
#         myscans, mysources = readscans(sdmfile)
#    then
#         listscans(myscans, mysources)
#    To show receiver bands used:
#         readrx(sdmfile)
#
#
# Notes: readflags
#    Returns a dictionary with the parameters
#    from the SDM Scan.xml tables indexed by scan_no: 
#       myscans[scan_no]['start']        start date/time (string)
#       myscans[scan_no]['end']          end date/time (string)
#       myscans[scan_no]['timerange']    start~end date/time (string)
#       myscans[scan_no]['source']       source name (string)
#       myscans[scan_no]['intent']       scan intent(s) (string)
#       myscans[scan_no]['nsubs']        number of subscans (int)
#
#    Time strings are in format YYYY/MM/DD/HH:MM:SS.SS
#    e.g. 2010/11/30/10:33:51.50
#    with precision set to 2 sub-second digits (10ms)
#
# (I)Python Notes: (courtesy A.Deller)
#
#    Nominally, the flagant.py needs to be in the same directory you
#    are running casapy from.  If you want to put the source in a
#    different directory, then:
#
#    1) Set up a place where you will put your modules and point a 
#       variable at it, e.g.:
#         export CASAPYUTILS="~/src/python_mods/"
#    2) Then make sure ipython knows about it by putting:
#         ip.ex("sys.path.append(os.environ['CASAPYUTILS'])")
#       in your ~/.ipython/ipy_user_conf.py (under the main: block).
#
######################################################################

import os
import math
from operator import itemgetter
import numpy as np

def call_qatime(arg, form='', prec=0):
    """
    This is a wrapper for qa.time(), which in casa 3.5 returns a list of strings instead
    of just a scalar string.  In this case, return the first value in the list.
    - Todd Hunter
    """
    try:
        import casac
    except ImportError, e:
        print "failed to load casa:\n", e
        exit(1)
    if ('qa' in dir(casac)):
        qa = casac.qa
    else:
        qa = casac.casac.qa
    result = qa.time(arg, form=form, prec=prec)
    if (type(result) == list or type(result)==np.ndarray):
        return(result[0])
    else:
        return(result)

def readscans(sdmfile):
    if (os.path.exists(sdmfile) == False):
        print "Could not find the SDM file = ", sdmfile
        return([],[])
    if (os.path.exists(sdmfile+'/Scan.xml') == False):
        print "Could not find the Scan.xml file.  Are you sure this is an ASDM?"
        return([],[])
    try:
        import casac
    except ImportError, e:
        print "failed to load casa:\n", e
        exit(1)
    if ('qa' in dir(casac)):
        qa = casac.qa
    else:
        qa = casac.casac.qa
        
    try:
        from xml.dom import minidom
    except ImportError, e:
        print "failed to load xml.dom.minidom:\n", e
        exit(1)

    # read Scan.xml into dictionary also and make a list
    xmlscans = minidom.parse(sdmfile+'/Scan.xml')
    scandict = {}
    rowlist = xmlscans.getElementsByTagName("row")
    for rownode in rowlist:
        rowfid = rownode.getElementsByTagName("scanNumber")
        fid = int(rowfid[0].childNodes[0].nodeValue)
        # number of subscans
        try:
            # ALMA
            rowsubs = rownode.getElementsByTagName("numSubScan")
            nsubs = int(rowsubs[0].childNodes[0].nodeValue)
        except:
            # EVLA
            rowsubs = rownode.getElementsByTagName("numSubscan")
            nsubs = int(rowsubs[0].childNodes[0].nodeValue)
        # intents
        rownint = rownode.getElementsByTagName("numIntent")
        nint = int(rownint[0].childNodes[0].nodeValue)

        rowintents = rownode.getElementsByTagName("scanIntent")
        sint = str(rowintents[0].childNodes[0].nodeValue)
        sints = sint.split()
        rint = ''
        for r in range(nint):
            intent = sints[2+r]
            if rint=='':
                rint = intent
            else:
                rint += ' '+intent

        # start and end times in mjd ns
        rowstart = rownode.getElementsByTagName("startTime")
        start = int(rowstart[0].childNodes[0].nodeValue)
        startmjd = float(start)*1.0E-9/86400.0
        t = qa.quantity(startmjd,'d')
        starttime = call_qatime(t,form="ymd",prec=8)
        rowend = rownode.getElementsByTagName("endTime")
        end = int(rowend[0].childNodes[0].nodeValue)
        endmjd = float(end)*1.0E-9/86400.0
        t = qa.quantity(endmjd,'d')
        endtime = call_qatime(t,form="ymd",prec=8)
        # source name
        rowsrc = rownode.getElementsByTagName("sourceName")
        if (len(rowsrc) < 1):
            print "Scan %d appears to be corrupt." % (len(scandict)+1)
        else:
            src = str(rowsrc[0].childNodes[0].nodeValue)
            # to find out what all is available,
#            print rownode.getElementsByTagName("*")
            scandict[fid] = {}
            scandict[fid]['start'] = starttime
            scandict[fid]['end'] = endtime
#            print "starttime = ", starttime
#            print "endtime = ", endtime
            timestr = starttime+'~'+endtime
            scandict[fid]['timerange'] = timestr
            scandict[fid]['source'] = src
            scandict[fid]['intent'] = rint
            scandict[fid]['nsubs'] = nsubs
            scandict[fid]['duration'] = endmjd-startmjd
    print '  Found ',rowlist.length,' scans in Scan.xml'

    # read Source.xml into dictionary also and make a list
    xmlsources = minidom.parse(sdmfile+'/Source.xml')
    sourcedict = {}
    sourcelist = []
    sourceId = []
    rowlist = xmlsources.getElementsByTagName("row")
    for rownode in rowlist:
        rowfid = rownode.getElementsByTagName("sourceId")
        fid = int(rowfid[0].childNodes[0].nodeValue)

        # source name
        rowsrc = rownode.getElementsByTagName("sourceName")
        src = str(rowsrc[0].childNodes[0].nodeValue)
        try:
            rowsrc = rownode.getElementsByTagName("directionCode")
            directionCode = str(rowsrc[0].childNodes[0].nodeValue)
        except:
            directionCode = ''
        rowsrc = rownode.getElementsByTagName("direction")
        (ra,dec) = rowsrc[0].childNodes[0].nodeValue.split()[2:4]
        ra = float(ra)
        dec = float(dec)
        if (src not in sourcelist):
            sourcelist.append(src)
            sourceId.append(fid)
            sourcedict[fid] = {}
#            sourcedict[fid]['sourceName'] = src
            sourcedict[fid]['source'] = src
            sourcedict[fid]['directionCode'] = directionCode
            sourcedict[fid]['ra'] = ra
            sourcedict[fid]['dec'] = dec
#            print "Loading source %s to index %d" % (src,fid)
        else:
            ai = sourceId[sourcelist.index(src)]
#            print "Source %s is already at index %d = ID:%d" % (src,sourcelist.index(src),ai)
            if (ra != sourcedict[ai]['ra'] or dec != sourcedict[ai]['dec']):
                print "WARNING: Multiple directions found for source %d = %s" % (fid,src)
                ras = (ra - sourcedict[ai]['ra'])*180*3600*math.cos(dec)/math.pi
                decs = (dec - sourcedict[ai]['dec'])*180*3600/math.pi
                print "The difference is (%f,%f) arcseconds." % (ras,decs)
#    for src in range(len(sourcedict)):
#        print "%s direction = %f, %f" % (sourcedict[src]['sourceName'],
#                                         sourcedict[src]['ra'],
#                                         sourcedict[src]['dec'])
        
    # return the dictionary for later use
    return [scandict, sourcedict]
# Done readscans

def listscans(dicts):
    try:
        import casac
    except ImportError, e:
        print "failed to load casa:\n", e
        exit(1)
    if ('qa' in dir(casac)):
        qa = casac.qa
    else:
        qa = casac.casac.qa
    myscans = dicts[0]
    mysources = dicts[1]
    if (myscans == []): return
    # Loop over scans
    for key in myscans.keys():
        mys = myscans[key]
        src = mys['source']
        tim = mys['timerange']
        sint= mys['intent']
        dur = mys['duration']*1440
        print '%8i %24s %48s  %.1f minutes  %s ' % (key, src, tim, dur, sint)
    durations = duration(myscans)
    print '  Found ', len(mysources),' sources in Source.xml'
    for key in durations:
        for mysrc in mysources.keys():
#            if (key[0] == mysources[mysrc]['sourceName']):
            if (key[0] == mysources[mysrc]['source']):
                ra = mysources[mysrc]['ra']
                dec = mysources[mysrc]['dec']
                directionCode = mysources[mysrc]['directionCode']
                break
        raString = qa.formxxx('%.12frad'%ra,format('hms'))
        decString = qa.formxxx('%.12frad'%dec,format('dms')).replace('.',':',2)
        print '   Total %24s (%d)  %5.1f minutes  (%.3f, %+.3f radian) %s: %s %s' % (key[0], int(mysrc), key[1], ra, dec, directionCode, raString, decString)
    durations = duration(myscans,nocal=True)
    for key in durations:
        print '   Total %24s      %5.1f minutes (neglecting pntg, atm & sideband cal. scans)' % (key[0],key[1])
    return
# Done

def duration(myscans, nocal=False):
    durations = []
    for key in myscans.keys():
        mys = myscans[key]
        src = mys['source']
        if (nocal and (mys['intent'].find('CALIBRATE_SIDEBAND')>=0 or
                       mys['intent'].find('CALIBRATE_POINTING')>=0 or
                       mys['intent'].find('CALIBRATE_ATMOSPHERE')>=0)):
            dur = 0
        else:
            dur = mys['duration']*1440
        new = 1
        for s in range(len(durations)):
            if (src == durations[s][0]):
                new = 0
                source = s
        if (new == 1):
            durations.append([src,dur])
        else:
            durations[source][1] = durations[source][1] + dur
    return(durations)
    
def readrx(sdmfile):
    try:
        import casac
    except ImportError, e:
        print "failed to load casa:\n", e
        exit(1)
    if ('qa' in dir(casac)):
        qa = casac.qa
    else:
        qa = casac.casac.qa
    try:
        from xml.dom import minidom
    except ImportError, e:
        print "failed to load xml.dom.minidom:\n", e
        exit(1)

    # read Scan.xml into dictionary also and make a list
    xmlrx = minidom.parse(sdmfile+'/Receiver.xml')
    rxdict = {}
    rxlist = []
    rowlist = xmlrx.getElementsByTagName("row")
    for rownode in rowlist:
        a = rownode.getElementsByTagName("*")
        rowrxid = rownode.getElementsByTagName("receiverId")
        rxid = int(rowrxid[0].childNodes[0].nodeValue)
        rowfreqband = rownode.getElementsByTagName("frequencyBand")
        freqband = str(rowfreqband[0].childNodes[0].nodeValue)
        print "rxid = %d, freqband = %s" % (rxid,freqband)
    # return the dictionary for later use
    return rxdict
