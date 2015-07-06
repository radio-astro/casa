from __future__ import absolute_import

# Imports
import os
import datetime
import math
import numpy as np
import matplotlib
import pylab as pb
from scipy.interpolate import splev, splrep

# Pipeline imports
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)


# A useful sequence of 19 unique matplotlib colors to cycle through
overlayColors = [
      [0.00,  0.00,  1.00],
      [0.00,  0.50,  0.00],
      [1.00,  0.00,  0.00],
      [0.00,  0.75,  0.75],
      [0.75,  0.00,  0.75],
      [0.25,  0.25,  0.25],
      [0.75,  0.25,  0.25],
#      [0.95,  0.95,  0.00],  yellow
      [0.25,  0.25,  0.75],
      [1.00,  0.75,  0.75], # [0.75, 0.75, 0.75] is invisible on gray border
      [0.00,  1.00,  0.00],
      [0.76,  0.57,  0.17],
      [0.54,  0.63,  0.22],
      [0.34,  0.57,  0.92],
      [1.00,  0.10,  0.60],
      [0.70,  1.00,  0.70], # [0.88,  0.75,  0.73], hard to see on gray
      [0.10,  0.49,  0.47],
      [0.66,  0.34,  0.65],
      [0.99,  0.41,  0.23]]
overlayColors += overlayColors + overlayColors


def plotPWV(ms, figfile='', plotrange=[0,0,0,0], clip=True):

    """
    Read and plot the PWV values from the ms via the ASDM_CALWVR table.
    If that table is not found, read them from the ASDM_CALATMOSPHERE table.
    Different antennas are shown in different colored points.

    Arguments:
           ms: The measurement set
    plotrange: The ranges for the X and Y axes (default=[0,0,0,0] which is autorange)
         clip: True = do not plot outliers beyond 5 * MAD from the median.
      figfile: True, False, or a string

    If figfile is not a string, the file created will be <ms>.pwv.png.
    """

    if (os.path.exists(ms) == False):
        LOG.warn ("Could not find  ms: %s" % (ms))
        return

    if (os.path.exists(ms+'/ASDM_CALWVR') == False and os.path.exists(ms+'/ASDM_CALATMOSPHERE') == False):
        # Confirm that it is ALMA data
        observatory = getObservatoryName(ms)
        if (observatory.find('ALMA') < 0 and observatory.find('ACA') < 0):
            LOG.warn("This is not ALMA data.  No PWV plot made.")
        else:
            LOG.warn("Could not find either %s/ASDM_CALWVR or ASDM_CALATMOSPHERE" % (ms))
        return

    try:
        [watertime, water, antennaName] = readPWVFromMS(ms)
    except:
        LOG.warn("Could not open %s/ASDM_CALWVR nor ASDM_CALATMOSPHERE" % (ms))
        return

    # Initialize plotting
    pb.clf()
    adesc = pb.subplot(111)
    ms = ms.split('/')[-1]

    # Clip the PWV values
    water = np.array(water) * 1000
    if (clip):
        mad = MAD(water)
        median = np.median(water)
        if (mad <= 0):
            matches = range(len(water))
        else:
            matches = np.where(abs(water - median) < 5 * mad)[0]
            nonmatches = np.where(abs(water - median) >= 5 * mad)[0]
            if (len(nonmatches) > 0):
                mymedian = np.median(water[nonmatches])
        water = water[matches]
        watertime = watertime[matches]
        antennaName = antennaName[matches]

    uniqueAntennas = np.unique(antennaName)
    pb.hold(True)
    list_of_date_times = mjdSecondsListToDateTime(watertime)
    timeplot = pb.date2num(list_of_date_times)
    for a in range(len(uniqueAntennas)):
        matches = np.where(uniqueAntennas[a] == np.array(antennaName))[0]
        pb.plot_date(timeplot[matches], water[matches], '.', color=overlayColors[a])

    # Now sort to average duplicate timestamps to one value, then fit spline
    indices = np.argsort(watertime)
    watertime = watertime[indices]
    water = water[indices]
    newwater = []
    newtime = []
    for w in range(len(water)):
        if (watertime[w] not in newtime):
            matches = np.where(watertime[w] == watertime)[0]
            newwater.append(np.median(water[matches]))
            newtime.append(watertime[w])
    watertime = newtime
    water = newwater
    regularTime = np.linspace(watertime[0], watertime[-1], len(watertime))
    #ius = splrep(watertime, water,s=len(watertime)-math.sqrt(2*len(watertime)))
    order = 3
    if len(water) <= 3:
        order = 1
    if len(water) > 1:
        ius = splrep(watertime, water, s=len(watertime)-math.sqrt(2*len(watertime)), k=order)
        water = splev(regularTime, ius, der=0)
    list_of_date_times = mjdSecondsListToDateTime(regularTime)
    timeplot = pb.date2num(list_of_date_times)
    pb.plot_date(timeplot,water,'k-')

    # Plot limits and ranges
    if (plotrange[0] != 0 or plotrange[1] != 0):
        pb.xlim([plotrange[0],plotrange[1]])
    if (plotrange[2] != 0 or plotrange[3] != 0):
        pb.ylim([plotrange[2],plotrange[3]])
    xlim = pb.xlim()
    ylim = pb.ylim()
    xrange = xlim[1]-xlim[0]
    yrange = ylim[1]-ylim[0]

    for a in range(len(uniqueAntennas)):
        pb.text(xlim[1]+0.01*xrange, ylim[1]-0.024*yrange*(a-2), 
                uniqueAntennas[a], color=overlayColors[a], size=8)
    pb.xlabel('Universal Time (%s)' % (utdatestring(watertime[0])))
    pb.ylabel('PWV (mm)')
    adesc.xaxis.grid(True,which='major')
    adesc.yaxis.grid(True,which='major')

    pb.title(ms)
    if len(water) > 1:
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
        adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
        adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
        RescaleXAxisTimeTicks(pb.xlim(), adesc)
    autoFigureName = "%s.pwv.png" % (ms)
    pb.draw()

    # Save plot
    if (figfile==True):
        pb.savefig(autoFigureName)
    elif (len(figfile) > 0):
        pb.savefig(figfile)
    else:
        LOG.warn("Failed to create PWV plot")


def readPWVFromMS(vis):
    """
    Reads all the PWV values from a measurement set, returning a list
    of lists:   [[mjdsec], [pwv], [antennaName]]
    """

    mytb = casatools.table
    if (os.path.exists("%s/ASDM_CALWVR" % vis)):
        mytb.open("%s/ASDM_CALWVR" % vis)
        time = mytb.getcol('startValidTime')  # mjdsec
        antenna = mytb.getcol('antennaName')
        pwv = mytb.getcol('water')
        mytb.close()
        if (len(pwv) < 1):
            LOG.info ("The ASDM_CALWVR table is empty, switching to ASDM_CALATMOSPHERE")
            time, antenna, pwv = readPWVFromASDM_CALATMOSPHERE(vis)
    elif (os.path.exists("%s/ASDM_CALATMOSPHERE" % vis)):
        time, antenna, pwv = readPWVFromASDM_CALATMOSPHERE(vis)
    else:
        LOG.warn ("Did not find ASDM_CALWVR nor ASDM_CALATMOSPHERE")
        return[[0],[1],[0]]

    return [time, pwv, antenna]


def readPWVFromASDM_CALATMOSPHERE(vis):
    """
    Reads the PWV via the water column of the ASDM_CALATMOSPHERE table.
    """

    if (not os.path.exists(vis+'/ASDM_CALATMOSPHERE')):
        if (vis.find('.ms') < 0):
            vis += '.ms'
            if (not os.path.exists(vis)):
                LOG.warn ("Could not find measurement set")
                return
            elif (not os.path.exists(vis+'/ASDM_CALATMOSPHERE')):
                LOG.warn("Could not find ASDM_CALATMOSPHERE in the measurement set")
                return
        else:
            LOG.warn ("Could not find measurement set")
            return

    mytb = casatools.table
    mytb.open("%s/ASDM_CALATMOSPHERE" % vis)
    pwvtime = mytb.getcol('startValidTime')  # mjdsec
    antenna = mytb.getcol('antennaName')
    pwv = mytb.getcol('water')[0]  # There seem to be 2 identical entries per row, so take first one.
    mytb.close()

    return (pwvtime, antenna, pwv)


# The following routines are general purpose and may eventually
# be useful elsewhere.

def getObservatoryName(ms):
    """
    Returns the observatory name in the specified ms.
    """

    obsTable = ms+'/OBSERVATION'
    try:
        mytb = casatools.table
        mytb.open(obsTable)
        myName = mytb.getcell('TELESCOPE_NAME')
        mytb.close()
    except:
        LOG.warn("Could not open OBSERVATION table to get the telescope name: %s" % (obsTable))
        myName = ''
    return(myName)


def MAD(a, c=0.6745, axis=0):
    """
    Median Absolute Deviation along given axis of an array:

    median(abs(a - median(a))) / c

    c = 0.6745 is the constant to convert from MAD to std; it is used by
    default

    """

    a = np.array(a)
    good = (a==a)
    a = np.asarray(a, np.float64)
    if a.ndim == 1:
        d = np.median(a[good])
        m = np.median(np.fabs(a[good] - d) / c)
    else:
        d = np.median(a[good], axis=axis)
        # I don't want the array to change so I have to copy it?
        if axis > 0:
            aswp = swapaxes(a[good],0,axis)
        else:
            aswp = a[good]
        m = np.median(np.fabs(aswp - d) / c, axis=0)

    return m


def utdatestring(mjdsec):
    (mjd, dateTimeString) = mjdSecondsToMJDandUT(mjdsec)
    tokens = dateTimeString.split()
    return(tokens[0])

def mjdSecondsToMJDandUT(mjdsec, prec=6):
    """
    Converts a value of MJD seconds into MJD, and into a UT date/time string.
    prec: 6 means HH:MM:SS,  7 means HH:MM:SS.S
    example: (56000.0, '2012-03-14 00:00:00 UT')
    Caveat: only works for a scalar input value
    """

    me = casatools.measures
    today = me.epoch('utc','today')
    mjd = np.array(mjdsec) / 86400.
    today['m0']['value'] =  mjd

    hhmmss = call_qa_time(today['m0'], prec=prec)
    date = casatools.quanta.splitdate(today['m0'])
    utstring = "%s-%02d-%02d %s UT" % (date['year'],date['month'],date['monthday'],hhmmss)

    me.done()

    return(mjd, utstring)


def mjdSecondsListToDateTime(mjdsecList):
    """
    Takes a list of mjd seconds and converts it to a list of datetime
    structures.
    """

    me = casatools.measures
    dt = []
    typelist = type(mjdsecList)
    if not (typelist == list or typelist == np.ndarray):
        mjdsecList = [mjdsecList]
    for mjdsec in mjdsecList:
        today = me.epoch('utc','today')
        mjd = mjdsec / 86400.
        today['m0']['value'] =  mjd
        hhmmss = call_qa_time(today['m0'])   # don't fully understand this
        date = casatools.quanta.splitdate(today['m0'])  # date is now a dict
        timeString = '%d-%d-%d %d:%d:%d.%06d'%(date['monthday'],date['month'],date['year'],date['hour'],date['min'],date['sec'],date['usec'])
        mydate = datetime.datetime.strptime(timeString,'%d-%m-%Y %H:%M:%S.%f')
        dt.append(mydate)
    me.done()
    return(dt)

def call_qa_time(arg, form='', prec=0, showform=False):
    """
    This is a wrapper for qa.time(), which in casa 4.0.0 returns a list
    of strings instead of just a scalar string.
    """

    if (type(arg) == dict):
        if (type(arg['value']) == list or
            type(arg['value']) == np.ndarray):
            arg['value'] = arg['value'][0]
    result = casatools.quanta.time(arg, form=form, prec=prec, showform=showform)
    if (type(result) == list or type(result) == np.ndarray):
        return(result[0])
    else:
        return(result)

def RescaleXAxisTimeTicks(xlim, adesc):
    if (xlim[1] - xlim[0] < 10/1440.):
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,1)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.SecondLocator(bysecond=range(0,60,30)))
    elif (xlim[1] - xlim[0] < 0.5/24.):
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,5)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,1)))
    elif (xlim[1] - xlim[0] < 1/24.):
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,2)))

