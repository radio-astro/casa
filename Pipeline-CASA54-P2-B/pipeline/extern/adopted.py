from __future__ import absolute_import

import os
import xml.dom.minidom as minidom

import numpy as np

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging

LOG = logging.get_logger(__name__)

qa = casatools.quanta


def getMedianPWV(vis='.', myTimes=[0,999999999999], asdm='', verbose=False):
    """
    Extracts the PWV measurements from the WVR on all antennas for the
    specified time range.  The time range is input as a two-element list of
    MJD seconds (default = all times).  First, it tries to find the ASDM_CALWVR
    table in the ms.  If that fails, it then tries to find the
    ASDM_CALATMOSPHERE table in the ms.  If that fails, it then tried to find
    the CalWVR.xml in the specified ASDM, or failing that, an ASDM of the
    same name (-.ms).  If neither of these exist, then it tries to find
    CalWVR.xml in the present working directory. If it still fails, it looks
    for CalWVR.xml in the .ms directory.  Thus, you only need to copy this
    xml file from the ASDM into your ms, rather than the entire ASDM. Returns
    the median and standard deviation in millimeters.
    Returns:
    The median PWV, and the median absolute deviation (scaled to match rms)
    For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/GetMedianPWV
    -- Todd Hunter
    """
    tb = casatools.table

    pwvmean = 0
    success = False
    if verbose:
        print("in getMedianPWV with myTimes = {}".format(myTimes))
    try:
        if os.path.exists("%s/ASDM_CALWVR" % vis):
            tb.open("%s/ASDM_CALWVR" % vis)
            pwvtime = tb.getcol('startValidTime')  # mjdsec
            antenna = tb.getcol('antennaName')
            pwv = tb.getcol('water')
            tb.close()
            success = True
            if len(pwv) < 1:
                if os.path.exists("%s/ASDM_CALATMOSPHERE" % vis):
                    pwvtime, antenna, pwv = readPWVFromASDM_CALATMOSPHERE(vis)
                    success = True
                    if len(pwv) < 1:
                        print("Found no data in ASDM_CALWVR nor ASDM_CALATMOSPHERE table")
                        return 0, -1
                else:
                    if verbose:
                        print("Did not find ASDM_CALATMOSPHERE in the ms")
                    return 0, -1
            if verbose:
                print("Opened ASDM_CALWVR table, len(pwvtime)={}".format(len(pwvtime)))
        else:
            if verbose:
                print("Did not find ASDM_CALWVR table in the ms. Will look for ASDM_CALATMOSPHERE next.")
            if os.path.exists("%s/ASDM_CALATMOSPHERE" % vis):
                pwvtime, antenna, pwv = readPWVFromASDM_CALATMOSPHERE(vis)
                success = True
                if len(pwv) < 1:
                    print("Found no data in ASDM_CALATMOSPHERE table")
                    return 0, -1
            else:
                if verbose:
                    print("Did not find ASDM_CALATMOSPHERE in the ms")
    except:
        if verbose:
            print("Could not open ASDM_CALWVR table in the ms")
    finally:
        # try to find the ASDM table
        if not success:
            if len(asdm) > 0:
                if not os.path.exists(asdm):
                    print("Could not open ASDM = {}".format(asdm))
                    return 0, -1
                try:
                    [pwvtime, pwv, antenna] = readpwv(asdm)
                except:
                    if verbose:
                        print("Could not open ASDM = {}".format(asdm))
                    return pwvmean, -1
            else:
                try:
                    tryasdm = vis.split('.ms')[0]
                    if verbose:
                        print("No ASDM name provided, so I will try this name = {}".format(tryasdm))
                    [pwvtime, pwv, antenna] = readpwv(tryasdm)
                except:
                    try:
                        if verbose:
                            print("Still did not find it.  Will look for CalWVR.xml in current directory.")
                        [pwvtime, pwv, antenna] = readpwv('.')
                    except:
                        try:
                            if verbose:
                                print("Still did not find it.  Will look for CalWVR.xml in the .ms directory.")
                            [pwvtime, pwv, antenna] = readpwv('%s/' % vis)
                        except:
                            if verbose:
                                print("No CalWVR.xml file found, so no PWV retrieved. Copy it to this directory and try"
                                      " again.")
                            return pwvmean, -1
    try:
        matches = np.where(np.array(pwvtime) > myTimes[0])[0]
    except:
        print("Found no times > {:d}".format(myTimes[0]))
        return 0, -1
    if len(pwv) < 1:
        print("Found no PWV data")
        return 0, -1
    if verbose:
        print("{:d} matches = {}".format(len(matches), matches))
        print("{:d} pwv = {}".format(len(pwv), pwv))
    ptime = np.array(pwvtime)[matches]
    matchedpwv = np.array(pwv)[matches]
    matches2 = np.where(ptime <= myTimes[-1])[0]
    if verbose:
        print("matchedpwv = {}".format(matchedpwv))
        print("pwv = {}".format(pwv))
    if len(matches2) < 1:
        # look for the value with the closest start time
        mindiff = 1e12
        for i in range(len(pwvtime)):
            if abs(myTimes[0] - pwvtime[i]) < mindiff:
                mindiff = abs(myTimes[0] - pwvtime[i])
        matchedpwv = []
        for i in range(len(pwvtime)):
            if abs(abs(myTimes[0]-pwvtime[i]) - mindiff) < 1.0:
                matchedpwv.append(pwv[i])
        pwvmean = 1000 * np.median(matchedpwv)
        if verbose:
            print("Taking the median of {:d} pwv measurements from all antennas = {:.3f} mm".format(len(matchedpwv),
                                                                                                    pwvmean))
        pwvstd = 1000 * MAD(matchedpwv)
    else:
        pwvmean = 1000 * np.median(matchedpwv[matches2])
        pwvstd = 1000 * MAD(matchedpwv[matches2])
        if verbose:
            print("Taking the median of {:d} pwv measurements from all antennas = {:.3f} mm".format(len(matches2),
                                                                                                    pwvmean))
    return pwvmean, pwvstd


def readPWVFromASDM_CALATMOSPHERE(vis):
    """
    Reads the PWV via the water column of the ASDM_CALATMOSPHERE table.
    - Todd Hunter
    """
    if not os.path.exists(vis + '/ASDM_CALATMOSPHERE'):
        if vis.find('.ms') < 0:
            vis += '.ms'
            if not os.path.exists(vis):
                print("Could not find measurement set = {}".format(vis))
                return
            elif not os.path.exists(vis + '/ASDM_CALATMOSPHERE'):
                print("Could not find ASDM_CALATMOSPHERE in the measurement set")
                return
        else:
            print("Could not find measurement set")
            return
    mytb = casatools.table
    mytb.open("%s/ASDM_CALATMOSPHERE" % vis)
    pwvtime = mytb.getcol('startValidTime')  # mjdsec
    antenna = mytb.getcol('antennaName')
    pwv = mytb.getcol('water')[0]  # There seem to be 2 identical entries per row, so take first one.
    mytb.close()
    return pwvtime, antenna, pwv


def readpwv(asdm):
    """
    This function assembles the dictionary returned by readwvr() into arrays
    containing the PWV measurements written by TelCal into the ASDM.
    Units are in meters.
    -- Todd Hunter
    """
    wvrdict = readwvr(asdm)
    bigantlist = []
    for entry in wvrdict:
        bigantlist.append(wvrdict[entry]['antenna'])
    watertime = []
    water = []
    antenna = []
    for entry in wvrdict:
        measurements = 1
        for i in range(measurements):
            watertime.append(wvrdict[entry]['startmjdsec'] + (i * 1.0 / measurements) * wvrdict[entry]['duration'])
            water.append(wvrdict[entry]['water'])
            antenna.append(wvrdict[entry]['antenna'])
    return [watertime, water, antenna]


def readwvr(sdmfile, verbose=False):
    """
    This function reads the CalWVR.xml table from the ASDM and returns a
    dictionary containing: 'start', 'end', 'startmjd', 'endmjd',
    'startmjdsec', 'endmjdsec',
    'timerange', 'antenna', 'water', 'duration'.
    'water' is the zenith PWV in meters.
    This function is called by readpwv(). -- Todd Hunter
    """
    if not os.path.exists(sdmfile):
        print("readwvr(): Could not find file = {}".format(sdmfile))
        return
    xmlscans = minidom.parse(sdmfile + '/CalWVR.xml')
    scandict = {}
    rowlist = xmlscans.getElementsByTagName("row")
    fid = 0
    for rownode in rowlist:
        rowpwv = rownode.getElementsByTagName("water")
        pwv = float(rowpwv[0].childNodes[0].nodeValue)
        water = pwv
        scandict[fid] = {}

        # start and end times in mjd ns
        rowstart = rownode.getElementsByTagName("startValidTime")
        start = int(rowstart[0].childNodes[0].nodeValue)
        startmjd = float(start) * 1.0E-9 / 86400.0
        t = qa.quantity(startmjd, 'd')
        starttime = call_qa_time(t, form="ymd", prec=8)
        rowend = rownode.getElementsByTagName("endValidTime")
        end = int(rowend[0].childNodes[0].nodeValue)
        endmjd = float(end) * 1.0E-9 / 86400.0
        t = qa.quantity(endmjd, 'd')
        endtime = call_qa_time(t, form="ymd", prec=8)
        # antenna
        rowantenna = rownode.getElementsByTagName("antennaName")
        antenna = str(rowantenna[0].childNodes[0].nodeValue)

        scandict[fid]['start'] = starttime
        scandict[fid]['end'] = endtime
        scandict[fid]['startmjd'] = startmjd
        scandict[fid]['endmjd'] = endmjd
        scandict[fid]['startmjdsec'] = startmjd * 86400
        scandict[fid]['endmjdsec'] = endmjd * 86400
        timestr = starttime + '~' + endtime
        scandict[fid]['timerange'] = timestr
        scandict[fid]['antenna'] = antenna
        scandict[fid]['water'] = water
        scandict[fid]['duration'] = (endmjd - startmjd) * 86400
        fid += 1

    if verbose:
        print('  Found {} rows in CalWVR.xml'.format(rowlist.length))

    # return the dictionary for later use
    return scandict


def call_qa_time(arg, form='', prec=0, showform=False):
    """
    This is a wrapper for qa.time(), which in casa 4.0.0 returns a list
    of strings instead of just a scalar string.
    - Todd Hunter
    """
    if type(arg) is dict and type(arg['value']) in (list, np.ndarray) and len(arg['value']) > 1:
        LOG.warning('qa_time() received a dictionary containing a list'
                    'of length=%d rather than a scalar. Using first '
                    'value.', len(arg['value']))
        arg['value'] = arg['value'][0]
    result = qa.time(arg, form=form, prec=prec, showform=showform)
    if type(result) in (list, np.ndarray):
        return result[0]
    else:
        return result


def MAD(a, c=0.6745, axis=0):
    """
    Median Absolute Deviation along given axis of an array:

    median(abs(a - median(a))) / c

    c = 0.6745 is the constant to convert from MAD to std; it is used by
    default
    """
    a = np.array(a)
    good = (a == a)
    a = np.asarray(a, np.float64)
    if a.ndim == 1:
        d = np.median(a[good])
        m = np.median(np.fabs(a[good] - d) / c)
    else:
        d = np.median(a[good], axis=axis)
        # I don't want the array to change so I have to copy it?
        if axis > 0:
            aswp = np.swapaxes(a[good], 0, axis)
        else:
            aswp = a[good]
        m = np.median(np.fabs(aswp - d) / c, axis=0)

    return m


def getMJD():
    """
    Returns the current MJD.  See also getCurrentMJDSec().
    -Todd
    """
    myme = casatools.measures
    mjd = myme.epoch('utc', 'today')['m0']['value']
    myme.done()
    return mjd
