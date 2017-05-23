from __future__ import absolute_import
import glob
import math
import os
import string
import time as timeUtilities
import xml.dom.minidom as minidom

import matplotlib
import numpy
import pylab
import matplotlib.pyplot as pyplot

import casadef

import numpy as np
import pylab as pb

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging

LOG = logging.get_logger(__name__)

casaVersionWithMSMD = '4.1.0'
casaVersionWithMSMDFieldnames = '4.5'
casaVersionWithUndefinedFrame = '4.3.0'

at = casatools.atmosphere
qa = casatools.quanta

# Constants that are sometimes useful.  Warning these are cgs, we might want to change them
# to SI given the propensity in CASA to use SI.


class CONSTANTS(object):
    h = 6.6260755e-27
    k = 1.380658e-16

ALMA_LONGITUDE = -67.754748  # ICT-4143,  -67.754694=JPL-Horizons,  -67.754929=CASA observatories
ALMA_LATITUDE = -23.029211  # ICT-4143,  -23.029167=JPL-Horizons,  -23.022886=CASA observatories
ARCSEC_PER_RAD = 206264.80624709636

JPL_HORIZONS_ID = {
    'ALMA': '-7',
    'VLA': '-5',
    'GBT': '-9',
    'MAUNAKEA': '-80',
    'OVRO': '-81',
    'geocentric': '500'
}

# imported from analysisUtils - this can be refactored to use domain objects
# TODO Refactor htmlrenderer.compuzeAzElForMS
def computeAzElForMS(vis, value='mean', forcePositiveAzim=True, ignorePointingSBAtmScans=True):
    """
    Computes the min/max/mean or median azim and elev for a measurement set.
    vis: measurement set
    value: 'min', 'max', 'mean' or 'median'
    verbose: if True, then print the per scan values
    Returns: 2 values (azim and elev)
    -Todd Hunter
    """
    with casatools.MSMDReader(vis) as mymsmd:
        scans = mymsmd.scannumbers()
        if ignorePointingSBAtmScans:
            pointingScans = list(mymsmd.scansforintent('CALIBRATE_POINTING*'))
            sbScans = list(mymsmd.scansforintent('CALIBRATE_SIDEBAND*'))
            atmScans = list(mymsmd.scansforintent('CALIBRATE_ATMOSPHERE*'))
            calscans = sorted(pointingScans + sbScans + atmScans)
            if len(calscans) > 0:
                LOG.trace('Dropping pointing+sideband+atmosphere scans: %s', calscans)
                scans = [x for x in scans if x not in calscans]
                LOG.trace('Kept scans: %s', scans)
        observatory = mymsmd.observatorynames()[0]
        func = numpy.mean
        if value == 'median':
            func = numpy.median
        elif value == 'min':
            func = numpy.min
        elif value == 'max':
            func = numpy.max
        az = []
        el = []
        for scan in scans:
            field = mymsmd.fieldsforscan(scan)[0]
            raDecString = direction2radec(mymsmd.phasecenter(field))
            tfs = mymsmd.timesforscan(scan) / 86400.
            mjd = numpy.min(tfs)
            az0, el0 = computeAzElFromRADecMJD(raDecString, mjd, observatory,
                                               degrees=True, frame='AZELGEO',
                                               forcePositiveAzim=forcePositiveAzim)
            mjd = numpy.max(tfs)
            az1, el1 = computeAzElFromRADecMJD(raDecString, mjd, observatory,
                                               degrees=True, frame='AZELGEO',
                                               forcePositiveAzim=forcePositiveAzim)
            az.append(func([az0, az1]))
            el.append(func([el0, el1]))
            LOG.trace('scan %d = %s, az=%f, el=%f', scan, raDecString, az[-1], el[-1])
    return func(az), func(el)


def direction2radec(direction=None, prec=5):
    """
    Convert a direction dictionary to a sexagesimal string.
    Todd Hunter
    """
    ra = direction['m0']['value']
    dec = direction['m1']['value']
    qa = casatools.quanta

    s_ra = qa.formxxx('%.12frad' % ra, format='hms', prec=prec)
    s_dec = qa.formxxx('%.12frad' % dec, format='dms', prec=prec).replace('.', ':', 2)
    return '%s, %s' % (s_ra, s_dec)


def computeAzElFromRADecMJD(raDec, mjd, observatory='ALMA', verbose=False,
                            degrees=False, frame='AZEL', forcePositiveAzim=False):
    """
    Computes the az/el for a specified J2000 RA/Dec, MJD and observatory using
    the CASA measures tool.

    raDec must either be a tuple in radians: [ra,dec],
        or a string of the form "hh:mm:ss.sss -dd:mm:ss.ss"
    mjd must either be in days, or a date string of the form:
               2011/10/15 05:00:00  or   2011/10/15-05:00:00
            or 2011-10-15 05:00:00  or   2011-10-15-05:00:00
    observatory: must be either a name recognized by the CASA me tool, or a JPL Horizons
                 ID listed in the JPL_HORIZONS_ID dictionary at the top of this module.
    degrees: if False, returns Az,El in radians;  otherwise degrees
    frame: 'AZEL' or 'AZELGEO'
    - Todd Hunter
    """
    me = casatools.measures
    qa = casatools.quanta

    if type(raDec) is str:
        # Then assume it is of the format "hh:mm:ss.sss -dd:mm:ss.ss"
        tokens = raDec.split()
        if len(tokens) < 2:
            tokens = raDec.split(',')
            if len(tokens) < 2:
                raise ValueError('raDec mus be of the format: hh:mm:ss.sss -dd:mm:ss.ss')
        raDec = radec2rad(raDec)
        LOG.trace('RA Dec in radians = %s', raDec)
    if type(mjd) is str:
        mjd = dateStringToMJD(mjd)
        if mjd is None:
            raise ValueError('Invalid date string')
    mydir = me.direction('J2000', qa.quantity(raDec[0], 'rad'), qa.quantity(raDec[1], 'rad'))
    me.doframe(me.epoch('mjd', qa.quantity(mjd, 'd')))
    me.doframe(me.observatory(observatory))
    myazel = me.measure(mydir, frame)
    myaz = myazel['m0']['value']
    myel = myazel['m1']['value']
    if forcePositiveAzim and myaz < 0:
        myaz += 2 * numpy.pi
    LOG.trace('%s: Azim = %.6f deg   Elev = %.6f deg', observatory, myaz * 180 / numpy.pi, myel * 180 / numpy.pi)
    if degrees:
        myaz *= 180 / numpy.pi
        myel *= 180 / numpy.pi
    return [myaz, myel]


def dateStringToMJD(datestring):
    """
    Convert a date/time string to floating point MJD
    Input date format: 2011/10/15 05:00:00  or   2011/10/15-05:00:00
                    or 2011-10-15 05:00:00
    The time portion is optional.
    -- Todd Hunter
    """
    if '/' not in datestring:
        if datestring.count('-') != 2:
            LOG.error('Date format: 2011/10/15 05:00:00, 2011/10/15-05:00:00 '
                      'or 2011-10-15 05:00:00')
            raise ValueError('Invalid datestring format: {!r}'.format(datestring))

        d = datestring.split('-')
        datestring = d[0] + '/' + d[1] + '/' + d[2]

    return dateStringToMJDSec(datestring) / 86400.0


def dateStringToMJDSec(datestring='2011/10/15 05:00:00', datestring2=''):
    """
    Converts a date string into MJD seconds.  This is useful for passing
    time ranges to plotms, because they must be specified in mjd seconds.
    Either of these formats is valid: 2011/10/15 05:00:00
                                      2011/10/15-05:00:00
    The time portion is optional.
    If a second string is given, both values will be converted and a
    string will be created that can be used as a plotrange in plotms.
    For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/DateStringToMJDSec
    Todd Hunter
    """
    me = casatools.measures

    mydate = datestring.split()
    if len(mydate) < 2:
        mydate = datestring.split('-')
    hours = 0
    if len(mydate) > 1:
        mytime = (mydate[1]).split(':')
        for i in range(len(mytime)):
            hours += float(mytime[i]) / (60.0 ** i)
    mjd = me.epoch('utc', mydate[0])['m0']['value'] + hours / 24.0
    mjdsec = 86400 * mjd
    if len(datestring2) > 0:
        mydate2 = datestring2.split()
        if len(mydate2) < 1:
            return mjdsec
        if len(mydate2) < 2:
            mydate2 = datestring2.split('-')
        hours = 0
        if len(mydate2) > 1:
            mytime2 = (mydate2[1]).split(':')
            if len(mytime2) > 1:
                for i in range(len(mytime2)):
                    hours += float(mytime2[i]) / (60.0 ** i)
        mjd = me.epoch('utc', mydate2[0])['m0']['value'] + hours / 24.0
        mjdsec2 = mjd * 86400
        return [mjdsec, mjdsec2]
    else:
        return mjdsec


def radec2rad(radecstring):
    """
    Convert a position from a single RA/Dec sexagesimal string to RA and
    Dec in radians.
    The RA and Dec portions can be separated by a comma or a space.
    The RA portion of the string must be colon-delimited, space-delimited,
        or 'h/m/s' delimited.
    The Dec portion of the string can be either ":", "." or space-delimited.
    If it is "." delimited, then it must have degrees, minutes, *and* seconds.
    See also rad2radec.
    -Todd Hunter
    """
    if radecstring.find('h') > 0 and radecstring.find('d') > 0:
        radecstring = radecstring.replace('h', ':').replace('m', ':').replace('d', ':').replace('s', '')
    radec1 = radecstring.replace(',', ' ')
    tokens = radec1.split()
    if len(tokens) is 2:
        (ra, dec) = radec1.split()
    elif len(tokens) is 6:
        h, m, s, d, dm, ds = radec1.split()
        ra = '%s:%s:%s' % (h, m, s)
        dec = '%+f:%s:%s' % (float(d), dm, ds)
    else:
        raise ValueError('Invalid format for RA/Dec string')
    tokens = ra.strip().split(':')
    hours = 0
    for i, t in enumerate(tokens):
        hours += float(t) / (60. ** i)
    if dec.find(':') > 0:
        tokens = dec.lstrip().split(':')
    elif dec.find('.') > 0:
        try:
            (d, m, s) = dec.lstrip().split('.')
        except:
            (d, m, s, sfraction) = dec.lstrip().split('.')
            s = s + '.' + sfraction
        tokens = [d, m, s]
    else:  # just an integer
        tokens = [dec]
    dec1 = 0
    for i, t in enumerate(tokens):
        dec1 += abs(float(t) / (60. ** i))
    if dec.lstrip().find('-') is 0:
        dec1 = -dec1
    decrad = dec1 * numpy.pi / 180.
    ra1 = hours * 15
    rarad = ra1 * numpy.pi / 180.
    return rarad, decrad


def plotAtmosphere(vis=None, spw=-1, scan=0, intent='OBSERVE_TARGET',
                       frequency=[0, 1000],
                       pwv=None, bandwidth=None, telescope='ALMA',
                       temperature=None, altitude=None, latitudeClass=None,
                       pressure=None, humidity=None, numchan=1000, airmass=1.0,
                       elevation=0, plotfile='', plotrange=None,
                       quantity='transmissionPercent', h0=None,
                       drawRectangles=[], drawVerticalBars=[], overlay=False,
                       fontsize=12, showgrid=False, linewidth=[1, 1, 1],
                       color=['b', 'b', 'b'], linestyle=['-', '--', ':'],
                       drawWVR=False, verbose=False):

    """
    Simple plotter of atmospheric transmission. You can enter either:
    a) vis ( + spw + scan)
    b) frequency
    frequency in GHz (either a single value, or a tuple for the range)
    vis: if specified, then use the specified spw (or first with specified intent)
    spw: integer or string
    scan: if specified (along with vis), then read the elevation for this scan
          if not specified, then use the first scan with specified intent
    pwv: in mm
    plotfile: if specified, then write a plot file
    verbose: if True, then print the spw and scan chosen
    bandwidth: in GHz (only used if frequency is a single value)
    temperature: in K
    altitude: in m
    latitudeClass: 'tropical', 'midLatitudeWinter'(default), or 'midLatitudeSummer'
    pressure: in mbar
    humidity: in percentage
    elevation: in degrees
    plotrange: for y axis, e.g. [0,100]
    telescope: if not '', then apply nominal values for 'ALMA' or 'EVLA' (ignored if vis is set)
    quantity: 'transmission', 'transmissionPercent', 'opacity', or 'tsky'
    h0 (scale height of H2O in km)
    drawRectangles ([[x0,x1],[x2,x3],...])
    drawVerticalBars ([x0,x1,...])
    overlay: if True, the show wet component, dry component and their sum (transmission or opacity only)
    fontsize: of tick labels and axis labels
    showgrid: if True, show dotted lines on major ticks
    linewidth: list of linewidths for each component: [sum, dry, wet]
    colors: list of pylab colors for each component
    linestyle: list of pylab linestyles for each component
    Note: uses the global physical constants: h and k
    --Todd Hunter
    """
    if (vis is not None):
        if (not os.path.exists(vis)):
            print "Could not find measurement set"
            return
        mymsmd = casatools.msmd
        mymsmd.open(vis)
        scan = int(scan)
        spw = int(spw)
        scans = mymsmd.scannumbers()
        if (scan < 1 or spw < 0):
            if (intent not in mymsmd.intents()):
                if (intent not in [i.split('#')[0] for i in mymsmd.intents()]):
                    if (intent not in [i.split('#')[0].split('_')[1] for i in mymsmd.intents()]):
                        print "No scans with intent = ", intent
                        return
        if (scan < 1):
            scans = mymsmd.scansforintent('*' + intent + '*')
            scan = scans[0]
            if verbose: print "Chose scan ", scan
        elif (scan not in scans):
            print "Scan %d not found. Available = " % (scan), scans
            return
        spws = mymsmd.spwsforintent('*')
        telescope = mymsmd.observatorynames()[0]
        if (spw < 0):
            spws = mymsmd.spwsforintent('*' + intent + '*')
            if (telescope.find('ALMA') >= 0):
                spws = np.intersect1d(spws, mymsmd.almaspws(tdm=True, fdm=True))
            if (len(spws) < 1):
                spws = mymsmd.spwsforscan(scan)
                spw = spws[-1]
            else:
                spw = spws[0]
            if verbose: print "Chose spw ", spw
        elif (spw not in spws):
            print "Spw %d not found. Available = " % (spw), spws
            return
        freqs = mymsmd.chanfreqs(spw) * 1e-9
        numchan = len(freqs)
        frequency = [np.min(freqs), np.max(freqs)]
        pwv, stdev = getMedianPWV(vis)
        mymsmd.close()
        mydict = getWeather(vis, scan=scan)
        elevation = mydict[0]['elevation']
        humidity = mydict[0]['humidity']
        temperature = mydict[0]['temperature'] + 273.15
        pressure = mydict[0]['pressure']
    if (elevation > 0):
        airmass = 1 / np.sin(elevation * np.pi / 180.)
    else:
        if (airmass < 1):
            print "Invalid airmass"
            return
        elevation = 180 * math.asin(1 / airmass) / np.pi
    tropical = 1
    midLatitudeSummer = 2
    midLatitudeWinter = 3
    if (latitudeClass is None):
        latitudeClass = tropical
    elif (latitudeClass == 'tropical'):
        latitudeClass = tropical
    elif (latitudeClass == 'midLatitudeWinter'):
        latitudeClass = midLatitudeWinter
    elif (latitudeClass == 'midLatitudeSummer'):
        latitudeClass = midLatitudeSummer
    else:
        print 'Unrecognized latitude class.'
        return

    # Apply default weather conditions, if not specified
    if (telescope == 'ALMA'):
        if (temperature is None):
            temperature = 270
        if (altitude is None):
            altitude = 5059
        if (pressure is None):
            pressure = 563
        if (humidity is None):
            humidity = 20
        if (pwv is None):
            pwv = 1.0
        if (h0 is None):
            h0 = 1.0
    elif (telescope.find('VLA') >= 0):
        if (temperature is None):
            temperature = 280
        if (altitude is None):
            altitude = 2124
        if (pressure is None):
            pressure = 785.5
        if (humidity is None):
            humidity = 20
        if (pwv is None):
            pwv = 5.0
        if (h0 is None):
            h0 = 2.0
    elif (telescope.find('SMA') >= 0):
        latitudeClass = tropical
        if (temperature is None):
            temperature = 280
        if (altitude is None):
            altitude = 4072
        if (pressure is None):
            pressure = 629.5
        if (humidity is None):
            humidity = 20
        if (pwv is None):
            pwv = 1.0
        if (h0 is None):
            h0 = 2.0
    else:
        if (temperature is None or altitude is None or pressure is None or humidity is None):
            print "If telescope is not specified, then you must specify pwv, temperature,"
            print " altitude, barometric pressure and relative humidity."
            return
        if (h0 is None):
            h0 = 2.0

    if (type(frequency) == list):
        frequencyRange = frequency
    elif (bandwidth > 0):
        if (numchan > 1):
            frequencyRange = [frequency - 0.5 * bandwidth, frequency + 0.5 * bandwidth]
        else:
            frequencyRange = [frequency]
    else:
        print "Bandwidth must be non-zero if frequency is a single value"
        return
    reffreq = np.mean(frequencyRange)
    chansep = (np.max(frequencyRange) - np.min(frequencyRange)) / (1.0 * numchan)
    nbands = 1
    if (numchan > 1):
        freqs = np.arange(frequencyRange[0], frequencyRange[1], chansep)
    else:
        freqs = frequencyRange

    myqa = casatools.quanta
    fCenter = create_casa_quantity(myqa, reffreq, 'GHz')
    if (numchan > 1):
        fResolution = create_casa_quantity(myqa, chansep, 'GHz')
        fWidth = create_casa_quantity(myqa, numchan * chansep, 'GHz')
    else:
        fResolution = create_casa_quantity(myqa, bandwidth, 'GHz')
        fWidth = create_casa_quantity(myqa, bandwidth, 'GHz')
    at.initAtmProfile(humidity=humidity, temperature=create_casa_quantity(myqa, temperature, "K"),
                      altitude=create_casa_quantity(myqa, altitude, "m"),
                      pressure=create_casa_quantity(myqa, pressure, 'mbar'),
                      h0=create_casa_quantity(myqa, h0, "km"),
                      atmType=latitudeClass)
    at.initSpectralWindow(nbands, fCenter, fWidth, fResolution)
    at.setUserWH2O(create_casa_quantity(myqa, pwv, 'mm'))
    dry, wet, TebbSky, rf, cs = getAtmDetails(at)
    if (overlay):
        dryOpacity = airmass * dry
        wetOpacity = airmass * wet
        dryTransmission = np.exp(-dryOpacity)
        wetTransmission = np.exp(-wetOpacity)
        if quantity == 'transmissionPercent':
            wetTransmission *= 100
            dryTransmission *= 100
    opacity = airmass * (wet + dry)
    transmission = np.exp(-opacity)
    if (quantity == 'transmissionPercent'):
        transmission *= 100
    TebbSky *= (1 - np.exp(-airmass * (wet + dry))) / (1 - np.exp(-wet - dry))
    fig = pyplot.figure()
    suptitle = fig.suptitle('Atmospheric Tranmission vs Frequency', fontsize=13, y=1.01)
    desc = pyplot.subplot(111)
    if (type(linewidth) != list):
        linewidth = [linewidth]
    if (type(color) != list):
        color = [color]
    if (quantity == 'opacity'):
        if (numchan > 1):
            pyplot.plot(freqs, opacity, ls=linestyle[0], lw=linewidth[0], color=color[0])
            if (overlay):
                pyplot.hold(True)
                pyplot.plot(freqs, dryOpacity, ls=linestyle[1], lw=linewidth[1], color=color[1])
                pyplot.plot(freqs, wetOpacity, ls=linestyle[2], lw=linewidth[2], color=color[2])
        else:
            pyplot.plot(freqs, opacity, 'bo')
        ylabel = 'Opacity'
        if (plotrange is not None):
            pyplot.ylim(plotrange)
    elif (quantity.find('transmission') >= 0):
        if (numchan > 1):
            pyplot.plot(freqs, transmission, ls=linestyle[0], lw=linewidth[0], color=color[0])
            if (overlay):
                pyplot.hold(True)
                pyplot.plot(freqs, dryTransmission, ls=linestyle[1], lw=linewidth[1], color=color[1])
                pyplot.plot(freqs, wetTransmission, ls=linestyle[2], lw=linewidth[2], color=color[2])
        else:
            pyplot.plot(freqs, transmission, 'bo')
        ylabel = 'Transmission'
        if (plotrange is None):
            if (quantity == 'transmissionPercent'):
                plotrange = [0, 105]
            else:
                plotrange = [0, 1.05]
        pyplot.ylim(plotrange)
    elif (quantity == 'tsky'):
        if (numchan > 1):
            pyplot.plot(freqs, TebbSky)
        else:
            pyplot.plot(freqs, TebbSky, 'bo')
        ylabel = 'Sky temperature (K)'
        if (plotrange is not None):
            pyplot.ylim(plotrange)
    else:
        print "Unrecognized quantity: %s" % (quantity)
        return
    if (spw >= 0):
        pyplot.ylabel(ylabel + ' (for scan %d)' % (scan), size=fontsize)
        pyplot.xlabel('Spw %d Frequency (GHz)' % (spw), size=fontsize)
    else:
        pyplot.ylabel(ylabel, size=fontsize)
        pyplot.xlabel('Frequency (GHz)', size=fontsize)
    resizeFonts(desc, fontsize)
    if (showgrid):
        desc.xaxis.grid(True, which='major')
        desc.yaxis.grid(True, which='major')
    title = ''
    y0, y1 = pyplot.ylim()
    gca = pyplot.gca()
    if (drawWVR):
        ifCenters = [1.25, 3.25, 5.5, 7.25]
        ifBandwidths = [1.5, 2.5, 2.0, 1.5]
        waterFreq = 183.31
        windows = []
        for i in range(len(ifCenters)):
            windows.append([waterFreq + ifCenters[i] - 0.5 * ifBandwidths[i],
                            waterFreq + ifCenters[i] + 0.5 * ifBandwidths[i]])
            windows.append([waterFreq - ifCenters[i] - 0.5 * ifBandwidths[i],
                            waterFreq - ifCenters[i] + 0.5 * ifBandwidths[i]])
        for bar in windows:
            gca.add_patch(pylab.Rectangle((bar[0], y0), bar[1] - bar[0], y1 - y0,
                                    facecolor=[0.9, 0.9, 0.9], edgecolor='k',
                                    linewidth=2))
    for bar in drawRectangles:
        gca.add_patch(pylab.Rectangle((bar[0], y0), bar[1] - bar[0], y1 - y0,
                                facecolor=[0.9, 0.9, 0.9], edgecolor='k',
                                linewidth=2))
    for bar in drawVerticalBars:
        pyplot.plot([bar, bar], [y0, y1], 'k--', linewidth=2)
    pyplot.xlim(frequencyRange)
    if (telescope != ''):
        title += '%s: ' % telescope

    pyplot.title('%sPWV=%.2fmm, alt.=%.0fm, h0=%.1fkm, elev=%.0fdeg, T=%.0fK, P=%.0fmb, H=%.0f%%' % (
    title, pwv, altitude, h0, elevation, temperature, pressure, humidity), size=10)
    pyplot.subplots_adjust(top=0.85)
    pyplot.draw()
    hvkT = CONSTANTS.h * np.mean(freqs) * 1e9 / (CONSTANTS.k * np.mean(TebbSky))
    J = hvkT / (np.exp(hvkT) - 1)
    if verbose:
        print "Mean opacity = %.4f, transmission = %.4f, Tsky = %.4f, Tsky_planck=%g" % (
        np.mean(opacity), np.mean(transmission), np.mean(TebbSky), np.mean(TebbSky) * J)
        print "Min/Max opacity = %.4f / %.4f   Min/Max transmission = %.4f / %.4f" % (
        np.min(opacity), np.max(opacity), np.min(transmission), np.max(transmission))
    if plotfile != '':
        pyplot.tight_layout()
        pyplot.savefig(plotfile, bbox_extra_artists=(suptitle,), bbox_inches="tight")


def create_casa_quantity(myqatool,value,unit):
    """
    A wrapper to handle the changing ways in which casa quantities are invoked.
    Todd Hunter
    """
    return myqatool.quantity(value, unit)


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
    if (verbose):
        print "in getMedianPWV with myTimes = ", myTimes
    try:
      if (os.path.exists("%s/ASDM_CALWVR"%vis)):
          tb.open("%s/ASDM_CALWVR" % vis)
          pwvtime = tb.getcol('startValidTime')  # mjdsec
          antenna = tb.getcol('antennaName')
          pwv = tb.getcol('water')
          tb.close()
          success = True
          if (len(pwv) < 1):
              if (os.path.exists("%s/ASDM_CALATMOSPHERE" % vis)):
                  pwvtime, antenna, pwv = readPWVFromASDM_CALATMOSPHERE(vis)
                  success = True
                  if (len(pwv) < 1):
                      print "Found no data in ASDM_CALWVR nor ASDM_CALATMOSPHERE table"
                      return(0,-1)
              else:
                  if (verbose):
                      print "Did not find ASDM_CALATMOSPHERE in the ms"
                  return(0,-1)
          if (verbose):
              print "Opened ASDM_CALWVR table, len(pwvtime)=", len(pwvtime)
      else:
          if (verbose):
              print "Did not find ASDM_CALWVR table in the ms. Will look for ASDM_CALATMOSPHERE next."
          if (os.path.exists("%s/ASDM_CALATMOSPHERE" % vis)):
              pwvtime, antenna, pwv = readPWVFromASDM_CALATMOSPHERE(vis)
              success = True
              if (len(pwv) < 1):
                  print "Found no data in ASDM_CALATMOSPHERE table"
                  return(0,-1)
          else:
              if (verbose):
                  print "Did not find ASDM_CALATMOSPHERE in the ms"
    except:
        if (verbose):
            print "Could not open ASDM_CALWVR table in the ms"
    finally:
    # try to find the ASDM table
     if (success == False):
       if (len(asdm) > 0):
           if (os.path.exists(asdm) == False):
               print "Could not open ASDM = ", asdm
               return(0,-1)
           try:
               [pwvtime,pwv,antenna] = readpwv(asdm)
           except:
               if (verbose):
                   print "Could not open ASDM = %s" % (asdm)
               return(pwvmean,-1)
       else:
           try:
               tryasdm = vis.split('.ms')[0]
               if (verbose):
                   print "No ASDM name provided, so I will try this name = %s" % (tryasdm)
               [pwvtime,pwv,antenna] = readpwv(tryasdm)
           except:
               try:
                   if (verbose):
                       print "Still did not find it.  Will look for CalWVR.xml in current directory."
                   [pwvtime, pwv, antenna] = readpwv('.')
               except:
                   try:
                       if (verbose):
                           print "Still did not find it.  Will look for CalWVR.xml in the .ms directory."
                       [pwvtime, pwv, antenna] = readpwv('%s/'%vis)
                   except:
                       if (verbose):
                           print "No CalWVR.xml file found, so no PWV retrieved. Copy it to this directory and try again."
                       return(pwvmean,-1)
    try:
        matches = np.where(np.array(pwvtime)>myTimes[0])[0]
    except:
        print "Found no times > %d" % (myTimes[0])
        return(0,-1)
    if (len(pwv) < 1):
        print "Found no PWV data"
        return(0,-1)
    if (verbose):
        print "%d matches = " % (len(matches)), matches
        print "%d pwv = " % (len(pwv)), pwv
    ptime = np.array(pwvtime)[matches]
    matchedpwv = np.array(pwv)[matches]
    matches2 = np.where(ptime<=myTimes[-1])[0]
    if (verbose):
        print "matchedpwv = %s" % (matchedpwv)
        print "pwv = %s" % (pwv)
    if (len(matches2) < 1):
        # look for the value with the closest start time
        mindiff = 1e12
        for i in range(len(pwvtime)):
            if (abs(myTimes[0]-pwvtime[i]) < mindiff):
                mindiff = abs(myTimes[0]-pwvtime[i])
#                pwvmean = pwv[i]*1000
        matchedpwv = []
        for i in range(len(pwvtime)):
            if (abs(abs(myTimes[0]-pwvtime[i]) - mindiff) < 1.0):
                matchedpwv.append(pwv[i])
        pwvmean = 1000*np.median(matchedpwv)
        if (verbose):
            print "Taking the median of %d pwv measurements from all antennas = %.3f mm" % (len(matchedpwv),pwvmean)
        pwvstd = 1000*MAD(matchedpwv)
    else:
        pwvmean = 1000*np.median(matchedpwv[matches2])
        pwvstd = 1000*MAD(matchedpwv[matches2])
        if (verbose):
            print "Taking the median of %d pwv measurements from all antennas = %.3f mm" % (len(matches2),pwvmean)
    return(pwvmean,pwvstd)


def getWeather(vis, scan='', antenna='0', verbose=False, vm=0, mymsmd='', debug=False, obsid=0):
    """
    Queries the WEATHER and ANTENNA tables of an .ms by scan number or
    list of scan numbers in order to return:
    * a dictionary of mean values of: angleToSun,
      pressure, temperature, humidity, dew point, wind speed, wind direction,
      azimuth, elevation, solarangle, solarelev, solarazim.
    * a list of science data timestamps in MJD seconds (not weather data times!)
    * zero, or the ValueMapping object
    If the sun is below the horizon, the solarangle returns is negated.
    If run in casa 4.4 or 4.5, and a concatenated measurement set is used, then the
       obsid must be specified for the desired scan, due to a change in msmd.  This
       behavior is fixed in casa 4.6.
    If run in casa < 4.1.0, this function needs to run ValueMapping, unless
    a ValueMapping object is passed via the vm argument. Otherwise it will
    run msmd.open, unless an msmd tool is passed via the mymsmd argument.
    -- Todd Hunter
    """
    if (debug):
        print "Entered getWeather with vis,scan,antenna = ", vis, ",", scan, ",", antenna
    if (os.path.exists(vis) == False):
        print "au.getWeather(): Measurement set not found"
        return
    if (os.path.exists(vis + '/table.dat') == False):
        print "No table.dat.  This does not appear to be an ms."
        return
    try:
        antennaNames = getAntennaNames(vis)
        if str(antenna).isdigit():
            antennaName = antennaNames[int(str(antenna))]
        else:
            antennaName = antenna
            try:
                antenna = getAntennaIndex(vis, antennaName)
            except:
                antennaName = string.upper(antenna)
                antenna = getAntennaIndex(vis, antennaName)
    except:
        print "Either the ANTENNA table does not exist or antenna %s does not exist" % (antenna)
        return ([0, [], vm])
    mytb = casatools.table
    try:
        mytb.open("%s/POINTING" % vis)
    except:
        print "POINTING table does not exist"
        return ([0, 0, vm])
    subtable = mytb.query("ANTENNA_ID == %s" % antenna)
    mytb.close()
    needToClose_mymsmd = False
    if (vm == 0 or vm == ''):
        if (mymsmd == ''):
            mymsmd = casatools.msmd
            mymsmd.open(vis)
            needToClose_mymsmd = True
    else:
        if (verbose):
            print "getWeather: Using current ValueMapping result"
    try:
        mytb.open("%s/OBSERVATION" % vis)
        observatory = mytb.getcell("TELESCOPE_NAME", 0)
        mytb.close()
    except:
        print "OBSERVATION table does not exist, assuming observatory == ALMA"
        observatory = "ALMA"
    if (scan == ''):
        if (mymsmd != '' and casadef.casa_version >= casaVersionWithMSMD):
            scan = mymsmd.scannumbers()
        else:
            scan = vm.uniqueScans
    conditions = {}
    conditions['pressure'] = conditions['temperature'] = conditions['humidity'] = conditions['dewpoint'] = conditions[
        'windspeed'] = conditions['winddirection'] = 0
    conditions['scan'] = scan
    if (type(scan) == str):
        if (scan.find('~') > 0):
            tokens = scan.split('~')
            scan = [int(k) for k in range(int(tokens[0]), int(tokens[1]) + 1)]
        else:
            scan = [int(k) for k in scan.split(',')]
    if (type(scan) == np.ndarray):
        scan = list(scan)
    if (type(scan) == list):
        myTimes = np.array([])
        for sc in scan:
            try:
                if (mymsmd != '' and casadef.casa_version >= casaVersionWithMSMD):
                    if debug: print "1) Calling mymsmd.timesforscan(%d)" % (sc)
                    if (casadef.casa_version >= '4.4' and casadef.casa_version < '4.6'):
                        newTimes = mymsmd.timesforscan(sc, obsid=obsid)
                    else:
                        newTimes = mymsmd.timesforscan(sc)
                else:
                    if debug: print "1) Calling vm.getTimesForScan(%d)" % (sc)
                    newTimes = vm.getTimesForScan(sc)
            except:
                print "1) Error reading scan %d, is it in the data?" % (sc)
                if (needToClose_mymsmd): mymsmd.close()
                return ([conditions, [], vm])
            myTimes = np.concatenate((myTimes, newTimes))
    elif (scan is not None):
        try:
            if (mymsmd != '' and casadef.casa_version >= casaVersionWithMSMD):
                if debug: print "2) Calling mymsmd.timesforscan(%d)" % (scan)
                if (casadef.casa_version >= '4.4' and casadef.casa_version < '4.6'):
                    myTimes = mymsmd.timesforscan(scan, obsid=obsid)
                else:
                    myTimes = mymsmd.timesforscan(scan)
            else:
                if debug: print "2) Calling vm.getTimesForScan(%d)" % (scan)
                myTimes = vm.getTimesForScan(scan)
        except:
            print "2) Error reading scan %d, is it in the data?" % (scan)
            if (needToClose_mymsmd): mymsmd.close()
            return ([conditions, [], vm])
    else:
        print "scan = ", scan
        if (needToClose_mymsmd): mymsmd.close()
        return ([conditions, [], vm])
    if (type(scan) == str):
        scan = [int(k) for k in scan.split(',')]
    if (type(scan) == list):
        listscan = ""
        listfield = []
        for sc in scan:
            if debug: print "Processing scan ", sc
            if (mymsmd != '' and casadef.casa_version >= casaVersionWithMSMD):
                if (casadef.casa_version >= '4.4' and casadef.casa_version < '4.6'):
                    listfield.append(mymsmd.fieldsforscan(sc, obsid=obsid))
                else:
                    listfield.append(mymsmd.fieldsforscan(sc))
            else:
                listfield.append(vm.getFieldsForScan(sc))
            listscan += "%d" % sc
            if (sc != scan[-1]):
                listscan += ","
        if debug: print "listfield = ", listfield
        listfields = np.unique(listfield[0])
        listfield = ""
        for field in listfields:
            listfield += "%s" % field
            if (field != listfields[-1]):
                listfield += ","
    else:
        listscan = str(scan)
        if (mymsmd != '' and casadef.casa_version >= casaVersionWithMSMD):
            if (casadef.casa_version >= '4.4' and casadef.casa_version < '4.6'):
                listfield = mymsmd.fieldsforscan(scan, obsid=obsid)
            else:
                listfield = mymsmd.fieldsforscan(scan)
        else:
            listfield = vm.getFieldsForScan(scan)
    [az, el] = ComputeSolarAzEl(myTimes[0], observatory)
    [az2, el2] = ComputeSolarAzEl(myTimes[-1], observatory)
    azsun = np.mean([az, az2])
    elsun = np.mean([el, el2])
    direction = subtable.getcol("DIRECTION")
    azeltime = subtable.getcol("TIME")
    subtable.close()
    telescopeName = getObservatoryName(vis)
    if (len(direction) > 0 and telescopeName.find('VLA') < 0):
        azimuth = direction[0][0] * 180.0 / math.pi
        elevation = direction[1][0] * 180.0 / math.pi
        if debug:
            print "len(azimuth) = %d, len(myTimes)=%d" % (len(azimuth), len(myTimes))
            print "elevation = ", elevation
        npat = np.array(azeltime)
        matches = np.where(npat >= myTimes[0])[0]
        matches2 = np.where(npat <= myTimes[-1])[0]
        if (len(matches2) > 0 and len(matches) > 0):
            if debug: print "matches[0]=%d, matches2[-1]=%d" % (matches[0], matches[-1])
            matchingIndices = range(matches[0], matches2[-1] + 1)
        else:
            matchingIndices = []
        if (len(matchingIndices) > 0):
            if debug: print "matches[0]=%d, matches2[-1]=%d" % (matches[0], matches[-1])
            conditions['azimuth'] = np.mean(azimuth[matches[0]:matches2[-1] + 1])
            conditions['elevation'] = np.mean(elevation[matches[0]:matches2[-1] + 1])
            if debug: print "elevation = ", conditions['elevation']
        elif (len(matches) > 0):
            conditions['azimuth'] = np.mean(azimuth[matches[0]])
            conditions['elevation'] = np.mean(elevation[matches[0]])
        else:
            conditions['azimuth'] = np.mean(azimuth)
            conditions['elevation'] = np.mean(elevation)
        while (antennaName != antennaNames[-1] and (
                np.abs(conditions['elevation']) < 0.001 or conditions['elevation'] != conditions['elevation'])):
            # Try the next antenna (needed for uid___A002_Xad565b_X20c4)
            nextAntenna = int(antenna) + 1
            nextAntennaName = antennaNames[nextAntenna]
            print "Requested antenna = %s shows elevation at/near zero (%f). Trying next antenna = %s." % (
            antennaName, conditions['elevation'], nextAntennaName)
            antenna = str(nextAntenna)
            antennaName = nextAntennaName
            mytb.open(vis + '/POINTING')
            subtable = mytb.query("ANTENNA_ID == %s" % antenna)
            direction = subtable.getcol("DIRECTION")
            azeltime = subtable.getcol("TIME")
            subtable.close()
            azimuth = direction[0][0] * 180.0 / math.pi
            elevation = direction[1][0] * 180.0 / math.pi
            npat = np.array(azeltime)
            matches = np.where(npat >= myTimes[0])[0]
            matches2 = np.where(npat <= myTimes[-1])[0]
            if debug: print "matches[0]=%d, matches2[-1]=%d" % (matches[0], matches[-1])
            conditions['azimuth'] = np.mean(azimuth[matches[0]:matches2[-1] + 1])
            conditions['elevation'] = np.mean(elevation[matches[0]:matches2[-1] + 1])
            mytb.close()
        if debug:
            print "%d matches after %f, %d matches before %f" % (len(matches), myTimes[0], len(matches2), myTimes[-1])
            print "%d matchingIndices" % (len(matchingIndices))
        conditions['solarangle'] = angularSeparation(azsun, elsun, conditions['azimuth'], conditions['elevation'])
        conditions['solarelev'] = elsun
        conditions['solarazim'] = azsun
        if (verbose):
            print "Used antenna = %s to retrieve mean azimuth and elevation" % (antennaName)
            print "Separation from sun = %f deg" % (abs(conditions['solarangle']))
        if (elsun < 0):
            conditions['solarangle'] = -conditions['solarangle']
            if (verbose):
                print "Sun is below horizon (elev=%.1f deg)" % (elsun)
        else:
            if (verbose):
                print "Sun is above horizon (elev=%.1f deg)" % (elsun)
        if (verbose):
            print "Average azimuth = %.2f, elevation = %.2f degrees" % (conditions['azimuth'], conditions['elevation'])
    else:
        if (verbose):
            print "The POINTING table is blank."
        if (type(scan) == int or type(scan) == np.int32):
            # compute Az/El for this scan
            if (mymsmd != '' and casadef.casa_version >= casaVersionWithMSMD):
                if (casadef.casa_version >= '4.4' and casadef.casa_version < '4.6'):
                    myfieldId = mymsmd.fieldsforscan(scan, obsid=obsid)
                else:
                    myfieldId = mymsmd.fieldsforscan(scan)
                if (type(myfieldId) == list or type(myfieldId) == type(np.ndarray(0))):
                    myfieldId = myfieldId[0]
                fieldName = mymsmd.namesforfields(myfieldId)
                if (type(fieldName) == list):
                    fieldName = fieldName[0]
            else:
                fieldName = vm.getFieldsForScan(scan)
                if (type(fieldName) == list):
                    fieldName = fieldName[0]
                myfieldId = vm.getFieldIdsForFieldName(fieldName)
                if (type(myfieldId) == list or type(myfieldId) == type(np.ndarray(0))):
                    # If the same field name has two IDs (this happens in EVLA data)
                    myfieldId = myfieldId[0]
                    #            print "type(myfieldId) = ", type(myfieldId)
            if (mymsmd != '' and casadef.casa_version >= casaVersionWithMSMD):
                if (casadef.casa_version >= '4.4' and casadef.casa_version < '4.6'):
                    myscantime = np.mean(mymsmd.timesforscan(scan, obsid=obsid))
                else:
                    myscantime = np.mean(mymsmd.timesforscan(scan))
            else:
                myscantime = np.mean(vm.getTimesForScans(scan))
            mydirection = getRADecForField(vis, myfieldId, usemstool=True)
            if (len(telescopeName) < 1):
                telescopeName = 'ALMA'
            if debug:
                print "Running computeAzElFromRADecMJD(%s, %f, %s)" % (
                str(mydirection), myscantime / 86400., telescopeName)
            myazel = computeAzElFromRADecMJD(mydirection, myscantime / 86400., telescopeName, verbose=False)
            if (debug): print "myazel = ", myazel
            conditions['elevation'] = myazel[1] * 180 / math.pi
            conditions['azimuth'] = myazel[0] * 180 / math.pi
            conditions['solarangle'] = angularSeparation(azsun, elsun, conditions['azimuth'], conditions['elevation'])
            conditions['solarelev'] = elsun
            conditions['solarazim'] = azsun
            if (verbose):
                print "Separation from sun = %f deg" % (abs(conditions['solarangle']))
            if (elsun < 0):
                conditions['solarangle'] = -conditions['solarangle']
                if (verbose):
                    print "Sun is below horizon (elev=%.1f deg)" % (elsun)
            else:
                if (verbose):
                    print "Sun is above horizon (elev=%.1f deg)" % (elsun)
            if (verbose):
                print "Average azimuth = %.2f, elevation = %.2f degrees" % (
                conditions['azimuth'], conditions['elevation'])
        elif (type(scan) == list):
            myaz = []
            myel = []
            if (verbose):
                print "Scans to loop over = ", scan
            for s in scan:
                if (mymsmd != '' and casadef.casa_version >= casaVersionWithMSMD):
                    if (casadef.casa_version >= '4.4' and casadef.casa_version < '4.6'):
                        fieldName = mymsmd.fieldsforscan(s, obsid=obsid)
                    else:
                        fieldName = mymsmd.fieldsforscan(s)
                else:
                    fieldName = vm.getFieldsForScan(s)
                if (type(fieldName) == list or type(fieldName) == np.ndarray):
                    # take only the first pointing in the mosaic
                    fieldName = fieldName[0]
                if (mymsmd != '' and casadef.casa_version >= casaVersionWithMSMD):
                    if (type(fieldName) == int or type(fieldName) == np.int32):
                        myfieldId = fieldName
                    else:
                        myfieldId = mymsmd.fieldsforname(fieldName)
                else:
                    myfieldId = vm.getFieldIdsForFieldName(fieldName)
                if (type(myfieldId) == list or type(myfieldId) == np.ndarray):
                    # If the same field name has two IDs (this happens in EVLA data)
                    myfieldId = myfieldId[0]
                if (mymsmd != '' and casadef.casa_version >= casaVersionWithMSMD):
                    if (casadef.casa_version >= '4.4' and casadef.casa_version < '4.6'):
                        myscantime = np.mean(mymsmd.timesforscan(s, obsid=obsid))
                    else:
                        myscantime = np.mean(mymsmd.timesforscan(s))
                else:
                    myscantime = np.mean(vm.getTimesForScans(s))
                mydirection = getRADecForField(vis, myfieldId, usemstool=True)
                telescopeName = getObservatoryName(vis)
                if (len(telescopeName) < 1):
                    telescopeName = 'ALMA'
                myazel = computeAzElFromRADecMJD(mydirection, myscantime / 86400., telescopeName, verbose=False)
                myaz.append(myazel[0] * 180 / math.pi)
                myel.append(myazel[1] * 180 / math.pi)
            if debug: print "myel = ", myel
            conditions['azimuth'] = np.mean(myaz)
            conditions['elevation'] = np.mean(myel)
            conditions['solarangle'] = angularSeparation(azsun, elsun, conditions['azimuth'], conditions['elevation'])
            conditions['solarelev'] = elsun
            conditions['solarazim'] = azsun
            if (verbose):
                print "*Using antenna = %s to retrieve mean azimuth and elevation" % (antennaName)
                print "Separation from sun = %f deg" % (abs(conditions['solarangle']))
            if (elsun < 0):
                conditions['solarangle'] = -conditions['solarangle']
                if (verbose):
                    print "Sun is below horizon (elev=%.1f deg)" % (elsun)
            else:
                if (verbose):
                    print "Sun is above horizon (elev=%.1f deg)" % (elsun)
            if (verbose):
                print "Average azimuth = %.2f, elevation = %.2f degrees" % (
                conditions['azimuth'], conditions['elevation'])

    # now, get the weather
    try:
        mytb.open("%s/WEATHER" % vis)
    except:
        print "Could not open the WEATHER table for this ms."
        if (needToClose_mymsmd): mymsmd.close()
        return ([conditions, myTimes, vm])
    if (True):
        mjdsec = mytb.getcol('TIME')
        indices = np.argsort(mjdsec)
        mjd = mjdsec / 86400.
        pressure = mytb.getcol('PRESSURE')
        relativeHumidity = mytb.getcol('REL_HUMIDITY')
        temperature = mytb.getcol('TEMPERATURE')
        if (np.mean(temperature) > 100):
            # must be in units of Kelvin, so convert to C
            temperature -= 273.15
        dewPoint = mytb.getcol('DEW_POINT')
        if (np.mean(dewPoint) > 100):
            # must be in units of Kelvin, so convert to C
            dewPoint -= 273.15
        if (np.mean(dewPoint) == 0):
            # assume it is not measured and use NOAA formula to compute from humidity:
            dewPoint = ComputeDewPointCFromRHAndTempC(relativeHumidity, temperature)
        sinWindDirection = np.sin(mytb.getcol('WIND_DIRECTION'))
        cosWindDirection = np.cos(mytb.getcol('WIND_DIRECTION'))
        windSpeed = mytb.getcol('WIND_SPEED')
        mytb.close()

        # put values into time order (they mostly are, but there can be small differences)
        mjdsec = np.array(mjdsec)[indices]
        pressure = np.array(pressure)[indices]
        relativeHumidity = np.array(relativeHumidity)[indices]
        temperature = np.array(temperature)[indices]
        dewPoint = np.array(dewPoint)[indices]
        windSpeed = np.array(windSpeed)[indices]
        sinWindDirection = np.array(sinWindDirection)[indices]
        cosWindDirection = np.array(cosWindDirection)[indices]

        # find the overlap of weather measurement times and scan times
        matches = np.where(mjdsec >= np.min(myTimes))[0]
        matches2 = np.where(mjdsec <= np.max(myTimes))[0]
        if debug:
            print "len(matches)=%d, len(matches2)=%d, len(myTimes)=%d, len(mjdsec)=%d" % (
            len(matches), len(matches2), len(myTimes), len(mjdsec))
        noWeatherData = False
        if (len(matches) > 0 and len(matches2) > 0):
            # average the weather points enclosed by the scan time range
            selectedValues = range(matches[0], matches2[-1] + 1)
            if (selectedValues == []):
                # there was a either gap in the weather data, or an incredibly short scan duration
                if (verbose):
                    print "----  Finding the nearest weather value --------------------------- "
                selectedValues = findClosestTime(mjdsec, myTimes[0])
        elif (len(matches) > 0):
            # all points are greater than myTime, so take the first one
            selectedValues = matches[0]
        elif (len(matches2) > 0):
            # all points are less than myTime, so take the last one
            selectedValues = matches2[-1]
        else:
            # table has no weather data!
            noWeatherData = True
        if (noWeatherData):
            conditions['pressure'] = 563.0
            conditions['temperature'] = 0  # Celsius is expected
            conditions['humidity'] = 20.0
            conditions['dewpoint'] = -20.0
            conditions['windspeed'] = 0
            conditions['winddirection'] = 0
            print "WARNING: No weather data found in the WEATHER table!"
        else:
            if (type(selectedValues) == np.int64 or type(selectedValues) == np.int32 or
                        type(selectedValues) == np.int):
                conditions['readings'] = 1
                if (verbose):
                    print "selectedValues=%d, myTimes[0]=%.0f, len(matches)=%d, len(matches2)=%d" % (selectedValues,
                                                                                                     myTimes[0],
                                                                                                     len(matches),
                                                                                                     len(matches2))
                    if (len(matches) > 0):
                        print "matches[0]=%f, matches[-1]=%f" % (matches[0], matches[-1])
                    if (len(matches2) > 0):
                        print "matches2[0]=%f, matches2[-1]=%d" % (matches2[0], matches2[-1])
            else:
                conditions['readings'] = len(selectedValues)
            conditions['pressure'] = np.mean(pressure[selectedValues])
            if (conditions['pressure'] != conditions['pressure']):
                # A nan value got through, due to no selected values (should be impossible)"
                if (verbose):
                    print ">>>>>>>>>>>>>>>>>>>>>>>>  selectedValues = ", selectedValues
                    print "len(matches)=%d, len(matches2)=%d" % (len(matches), len(matches2))
                    print "matches[0]=%f, matches[-1]=%f, matches2[0]=%f, matches2[-1]=%d" % (
                    matches[0], matches[-1], matches2[0], matches2[-1])
            conditions['temperature'] = np.mean(temperature[selectedValues])
            conditions['humidity'] = np.mean(relativeHumidity[selectedValues])
            conditions['dewpoint'] = np.mean(dewPoint[selectedValues])
            conditions['windspeed'] = np.mean(windSpeed[selectedValues])
            conditions['winddirection'] = (180. / math.pi) * np.arctan2(np.mean(sinWindDirection[selectedValues]),
                                                                        np.mean(cosWindDirection[selectedValues]))
            if (conditions['winddirection'] < 0):
                conditions['winddirection'] += 360
            if (verbose and noWeatherData == False):
                print "Mean weather values for scan %s (field %s)" % (listscan, listfield)
                print "  Pressure = %.2f mb" % (conditions['pressure'])
                print "  Temperature = %.2f C" % (conditions['temperature'])
                print "  Dew point = %.2f C" % (conditions['dewpoint'])
                print "  Relative Humidity = %.2f %%" % (conditions['humidity'])
                print "  Wind speed = %.2f m/s" % (conditions['windspeed'])
                print "  Wind direction = %.2f deg" % (conditions['winddirection'])

    if (needToClose_mymsmd): mymsmd.close()
    return ([conditions, myTimes, vm])
    # end of getWeather   forscan


def getAtmDetails(at):
    """
    A wrapper to handle the changing ways in which the at tool is accessed.
    Todd Hunter
    """
    dry = np.array(at.getDryOpacitySpec(0)[1])
    wet = np.array(at.getWetOpacitySpec(0)[1]['value'])
    TebbSky = at.getTebbSkySpec(spwid=0)[1]['value']
    # readback the values to be sure they got set
    rf = at.getRefFreq()['value']
    cs = at.getChanSep()['value']

    return dry, wet, TebbSky, rf, cs


def resizeFonts(adesc,fontsize):
    """
    Disable offset units and resize fonts.
    Todd Hunter
    """
    yFormat = matplotlib.ticker.ScalarFormatter(useOffset=False)
    adesc.yaxis.set_major_formatter(yFormat)
    adesc.xaxis.set_major_formatter(yFormat)
    pyplot.setp(adesc.get_xticklabels(), fontsize=fontsize)
    pyplot.setp(adesc.get_yticklabels(), fontsize=fontsize)


def readPWVFromASDM_CALATMOSPHERE(vis):
    """
    Reads the PWV via the water column of the ASDM_CALATMOSPHERE table.
    - Todd Hunter
    """
    if (not os.path.exists(vis + '/ASDM_CALATMOSPHERE')):
        if (vis.find('.ms') < 0):
            vis += '.ms'
            if (not os.path.exists(vis)):
                print "Could not find measurement set = ", vis
                return
            elif (not os.path.exists(vis + '/ASDM_CALATMOSPHERE')):
                print "Could not find ASDM_CALATMOSPHERE in the measurement set"
                return
        else:
            print "Could not find measurement set"
            return
    mytb = casatools.table
    mytb.open("%s/ASDM_CALATMOSPHERE" % vis)
    pwvtime = mytb.getcol('startValidTime')  # mjdsec
    antenna = mytb.getcol('antennaName')
    pwv = mytb.getcol('water')[0]  # There seem to be 2 identical entries per row, so take first one.
    mytb.close()
    return (pwvtime, antenna, pwv)


def readpwv(asdm):
    """
    This function assembles the dictionary returned by readwvr() into arrays
    containing the PWV measurements written by TelCal into the ASDM.
    Units are in meters.
    -- Todd Hunter
    """
    dict = readwvr(asdm)
    bigantlist = []
    for entry in dict:
        bigantlist.append(dict[entry]['antenna'])
    watertime = []
    water = []
    antenna = []
    for entry in dict:
        measurements = 1
        for i in range(measurements):
            watertime.append(dict[entry]['startmjdsec'] + (i * 1.0 / measurements) * dict[entry]['duration'])
            water.append(dict[entry]['water'])
            antenna.append(dict[entry]['antenna'])
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
    if (os.path.exists(sdmfile) == False):
        print "readwvr(): Could not find file = ", sdmfile
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

    if verbose: print '  Found ', rowlist.length, ' rows in CalWVR.xml'

    # return the dictionary for later use
    return scandict


def call_qa_time(arg, form='', prec=0, showform=False):
    """
    This is a wrapper for qa.time(), which in casa 4.0.0 returns a list
    of strings instead of just a scalar string.
    - Todd Hunter
    """
    if type(arg) is dict:
        if type(arg['value']) in (list, np.ndarray):
            if len(arg['value']) > 1:
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
    good = (a==a)
    a = np.asarray(a, np.float64)
    if a.ndim == 1:
        d = np.median(a[good])
        m = np.median(np.fabs(a[good] - d) / c)
#        print  "mad = %f" % (m)
    else:
        d = np.median(a[good], axis=axis)
        # I don't want the array to change so I have to copy it?
        if axis > 0:
            aswp = numpy.swapaxes(a[good],0,axis)
        else:
            aswp = a[good]
        m = np.median(np.fabs(aswp - d) / c, axis=0)

    return m


def getAntennaNames(msFile):
    """
    Returns the list of antenna names in the specified ms ANTENNA table.
    Obsoleted by msmd.antennanames(range(msmd.nantennas())).
    """
    if '*' in msFile:
        mylist = glob.glob(msFile)
        if not mylist:
            raise IOError('File not found: {!r}'.format(msFile))
        msFile = mylist[0]
    mytb = casatools.table
    mytb.open(msFile + '/ANTENNA')
    names = mytb.getcol('NAME')
    mytb.close()
    return names


def getAntennaIndex(msFile, antennaName):
    """
    Returns the index number of the specified antenna in the specified ms.
    The antennaName must be a string, e.g. DV01.  Obsoleted by msmd.antennaids('DV01')[0].
    """
    if str(antennaName).isdigit():
        return antennaName
    else:
        ids = getAntennaNames(msFile)
        if antennaName not in ids:
            print "Antenna %s is not in the dataset.  Available antennas = %s" % (antennaName, str(ids))
            return -1
        return np.where(ids == antennaName)[0][0]


def ComputeSolarAzEl(mjdsec=None, observatory='ALMA'):
    """
    Return the az and el of the Sun in degrees for the specified
    time (default=now).  Default observatory is ALMA. See also ComputeSolarRADec().
    Todd Hunter
    """
    if mjdsec is None:
        mjdsec = getCurrentMJDSec()
    (latitude, longitude, _) = getObservatoryLatLong(observatory)
    return ComputeSolarAzElLatLong(mjdsec, latitude, longitude)


def getCurrentMJDSec():
    """
    Returns the current MJD in seconds.
    Todd Hunter
    """
    return getMJD() * 86400


def getMJD():
    """
    Returns the current MJD.  See also getCurrentMJDSec().
    -Todd
    """
    myme = casatools.measures
    mjd = myme.epoch('utc','today')['m0']['value']
    myme.done()
    return mjd


def getObservatoryLatLong(observatory, verbose=False):
    """
    Opens the casa table of known observatories and returns the latitude and longitude
    in degrees for the specified observatory name string.
    observatory: string name, JPL integer, or integer string  (e.g. 'ALMA' == -7)
    -- Todd Hunter
    """
    repotable = os.getenv("CASAPATH").split()[0] + "/data/geodetic/Observatories"

    tb = casatools.table
    try:
        tb.open(repotable)
    except:
        print "Could not open table = %s, returning ALMA coordinates in au instead" % (repotable)
        longitude = ALMA_LONGITUDE
        latitude = ALMA_LATITUDE
        observatory = 'ALMA'
        return [latitude, longitude, observatory]
    if type(observatory) is int or str(observatory) in JPL_HORIZONS_ID.values():
        if str(observatory) in JPL_HORIZONS_ID.values():
            observatory = JPL_HORIZONS_ID.keys()[JPL_HORIZONS_ID.values().index(str(observatory))]
            if (verbose):
                print "Recognized observatory = %s" % observatory
            if (observatory == 'MAUNAKEA'): observatory = 'SMA'
            if (observatory == 'OVRO'): observatory = 'OVRO_MMA'
        else:
            print "Did not recognize observatory='%s' in %s, using ALMA instead." % (
                observatory, str(JPL_HORIZONS_ID.values()))
            observatory = 'ALMA'

    Name = tb.getcol('Name')
    matches = np.where(np.array(Name) == observatory)
    if len(matches) < 1 and str(observatory).find('500') < 0:
        print "Names = ", Name
        print "Did not find observatory='%s', using ALMA value in au instead." % (observatory)
        for n in Name:
            if (n.find(observatory) >= 0):
                print "Partial match: ", n
        observatory = 'ALMA'
        longitude = ALMA_LONGITUDE
        latitude = ALMA_LATITUDE
    elif (str(observatory).find('500') >= 0 or
                  str(observatory).lower().find('geocentric') >= 0):
        observatory = 'Geocentric'
        longitude = 0
        latitude = 0
    else:
        longitude = tb.getcol('Long')[matches[0]]
        latitude = tb.getcol('Lat')[matches[0]]
    tb.close()

    return [latitude, longitude, observatory]


def getObservatoryName(ms):
    """
    Returns the observatory name in the specified ms.
    -- Todd Hunter
    """
    antTable = ms+'/OBSERVATION'
    try:
        mytb = casatools.table
        mytb.open(antTable)
        myName = mytb.getcell('TELESCOPE_NAME')
        mytb.close()
    except:
        print "Could not open OBSERVATION table to get the telescope name: %s" % (antTable)
        myName = ''
    return myName


def angularSeparation(ra0, dec0, ra1, dec1, returnComponents=False):
    """
    Computes the great circle angle between two celestial coordinates.
    using the Vincenty formula (from wikipedia) which is correct for all
    angles, as long as you use atan2() to handle a zero denominator.
       See  http://en.wikipedia.org/wiki/Great_circle_distance
    ra,dec must be given in degrees, as is the output.
    It also works for the az,el coordinate system.
    Component separations are field_0 minus field_1.
    See also angularSeparationRadians()
    returnComponents: if True, then also compute angular separation in both
           coordinates and the position angle of the separation vector on the sky
    -- Todd Hunter
    """
    ra0 *= math.pi / 180.
    dec0 *= math.pi / 180.
    ra1 *= math.pi / 180.
    dec1 *= math.pi / 180.
    deltaLong = ra0 - ra1
    argument1 = (((math.cos(dec1) * math.sin(deltaLong)) ** 2) +
                 (
                 (math.cos(dec0) * math.sin(dec1) - math.sin(dec0) * math.cos(dec1) * math.cos(deltaLong)) ** 2)) ** 0.5
    argument2 = math.sin(dec0) * math.sin(dec1) + math.cos(dec0) * math.cos(dec1) * math.cos(deltaLong)
    angle = math.atan2(argument1, argument2) / (math.pi / 180.)
    if angle > 360:
        angle -= 360
    if returnComponents:
        cosdec = math.cos((dec1 + dec0) * 0.5)
        radegreesCosDec = np.degrees(ra0 - ra1) * cosdec
        radegrees = np.degrees(ra0 - ra1)
        decdegrees = np.degrees(dec0 - dec1)
        if radegrees > 360:
            radegrees -= 360
        if decdegrees > 360:
            decdegrees -= 360
        retval = angle, radegrees, decdegrees, radegreesCosDec
    else:
        retval = angle
    return retval


def getRADecForField(vis, field, usemstool=True, forcePositiveRA=False, verbose=False,
                     blendByName=True):
    """
    Returns [RA,Dec] in radians for the specified field in the specified ms.
    field: can be integer or string integer.
    -- Todd Hunter
    """
    if not os.path.exists(vis):
        print "Could not find measurement set"
        return
    if usemstool and casadef.casa_version >= '4.2.0':
        result = parseFieldArgument(vis, field, blendByName)
        if result is None:
            return
        idlist, namelist = result
        myms = casatools.ms
        myms.open(vis)
        if type(field) is str:
            if not field.isdigit():
                field = idlist[0]
            field = int(field)
        if verbose:
            print "running myms.getfielddirmeas(fieldid=%d)" % (field)
        mydir = myms.getfielddirmeas(fieldid=field)  # dircolname defaults to 'PHASE_DIR'
        mydir = np.array([[mydir['m0']['value']], [mydir['m1']['value']]])  # simulates tb.getcell
        myms.close()
    else:
        mytb = casatools.table
        try:
            mytb.open(vis + '/FIELD')
        except:
            print "Could not open FIELD table for ms=%s" % (vis)
            return [0, 0]
        mydir = mytb.getcell('DELAY_DIR', int(field))
        mytb.close()
    if forcePositiveRA:
        if mydir[0] < 0:
            mydir[0] += 2 * math.pi
    return mydir


def ComputeDewPointCFromRHAndTempC(relativeHumidity, temperature):
    """
    inputs:  relativeHumidity in percentage, temperature in C
    output: in degrees C
    Uses formula from http://en.wikipedia.org/wiki/Dew_point#Calculating_the_dew_point
    Todd Hunter
    """
    temperature = np.array(temperature)  # protect against it being a list
    relativeHumidity = np.array(relativeHumidity)  # protect against it being a list
    es = 6.112 * np.exp(17.67 * temperature / (temperature + 243.5))
    e = relativeHumidity * 0.01 * es
    if len(np.where(e <= 0)[0]) == 0:
        dewPoint = 243.5 * np.log(e / 6.112) / (17.67 - np.log(e / 6.112))
    else:
        dewPoint = np.zeros(len(e))
    return dewPoint


def findClosestTime(mytimes, mytime):
    myindex = 0
    mysep = np.abs(mytimes[0]-mytime)
    for m in range(1, len(mytimes)):
        if np.abs(mytimes[m] - mytime) < mysep:
            mysep = np.abs(mytimes[m] - mytime)
            myindex = m
    return myindex


def ComputeSolarAzElLatLong(mjdsec, latitude, longitude):
    """
    Computes the apparent Az,El of the Sun for a specified time and location
    on Earth.  Latitude and longitude must arrive in degrees, with positive
    longitude meaning east of Greenwich.
    -- Todd Hunter
    """
    DEG_TO_RAD = math.pi / 180.
    RAD_TO_DEG = 180 / math.pi
    HRS_TO_RAD = math.pi / 12.
    [RA, Dec] = ComputeSolarRADec(mjdsec)
    LST = ComputeLST(mjdsec, longitude)

    phi = latitude * DEG_TO_RAD
    hourAngle = HRS_TO_RAD * (LST - RA)
    azimuth = RAD_TO_DEG * math.atan2(math.sin(hourAngle), (
    math.cos(hourAngle) * math.sin(phi) - math.tan(Dec * DEG_TO_RAD) * math.cos(phi)))

    # the following is to convert from South=0 (which the French formula uses)
    # to North=0, which is what the rest of the world uses */
    azimuth += 180.0;

    if azimuth > 360.0:
        azimuth -= 360.0
    if azimuth < 0.0:
        azimuth += 360.0

    argument = math.sin(phi) * math.sin(Dec * DEG_TO_RAD) + math.cos(phi) * math.cos(Dec * DEG_TO_RAD) * math.cos(
        hourAngle)
    elevation = RAD_TO_DEG * math.asin(argument)

    return [azimuth, elevation]


def ComputeSolarRADec(mjdsec=None, vis=None):
    """
    Computes the RA,Dec of the Sun (in hours and degrees) for a specified time
    (default=now), or for the mean time of a measurement set
    -- Todd Hunter
    """
    RAD_TO_DEG = 180/math.pi
    RAD_TO_HRS = (1.0/0.2617993877991509)
    ra,dec = ComputeSolarRADecRadians(mjdsec, vis)
    return(ra*RAD_TO_HRS, dec*RAD_TO_DEG)


def ComputeSolarRADecRadians(mjdsec=None, vis=None):
    """
    Computes the RA,Dec of the Sun (in radians) for a specified time
    (default=now), or for the mean time of a measurement set.
    -- Todd Hunter
    """
    if mjdsec is None:
        if vis is not None:
            mjdsec = np.mean(getObservationMJDSecRange(vis))
        else:
            mjdsec = getCurrentMJDSec()
    jd = mjdToJD(mjdsec / 86400.)
    RAD_TO_DEG = 180 / math.pi
    RAD_TO_HRS = (1.0 / 0.2617993877991509)
    DEG_TO_RAD = math.pi / 180.
    T = (jd - 2451545.0) / 36525.0
    Lo = 280.46646 + 36000.76983 * T + 0.0003032 * T * T
    M = 357.52911 + 35999.05029 * T - 0.0001537 * T * T
    Mrad = M * DEG_TO_RAD
    e = 0.016708634 - 0.000042037 * T - 0.0000001267 * T * T
    C = (1.914602 - 0.004817 * T - 0.000014 * T * T) * math.sin(Mrad) + (0.019993 - 0.000101 * T) * math.sin(
        2 * Mrad) + 0.000289 * math.sin(3 * Mrad)
    L = Lo + C
    nu = DEG_TO_RAD * (M + C)
    R = 1.000001018 * (1 - e * e) / (1 + e * math.cos(nu))
    Omega = DEG_TO_RAD * (125.04 - 1934.136 * T)
    mylambda = DEG_TO_RAD * (L - 0.00569 - 0.00478 * math.sin(Omega))
    epsilon0 = (84381.448 - 46.8150 * T - 0.00059 * T * T + 0.001813 * T * T * T) / 3600.
    epsilon = (epsilon0 + 0.00256 * math.cos(Omega)) * DEG_TO_RAD
    rightAscension = math.atan2(math.cos(epsilon) * math.sin(mylambda), math.cos(mylambda))
    if rightAscension < 0:
        rightAscension += np.pi * 2
    argument = math.sin(epsilon) * math.sin(mylambda)
    declination = math.asin(argument)
    return [rightAscension, declination]


def getObservationMJDSecRange(vis):
    """
    Returns 2 floating point values in MJD seconds.
    """
    return([getObservationStart(vis),getObservationStop(vis)])


def getObservationStart(vis, obsid=0):
    """
    Read the start time of the observation and report it in MJD seconds.
    -Todd Hunter
    """
    if not os.path.exists(vis):
        print "vis does not exist = %s" % (vis)
        return
    if (os.path.exists(vis+'/table.dat') == False):
        print "No table.dat.  This does not appear to be an ms."
        print "Use au.getObservationStartDateFromASDM()."
        return
    mytb = casatools.table
    try:
        mytb.open(vis+'/OBSERVATION')
    except:
        print "ERROR: failed to open OBSERVATION table on file "+vis
        return 3

    time_range = mytb.getcol('TIME_RANGE')[:,obsid]
    mytb.close()

    if type(time_range) is np.ndarray:
        time_range = np.min(time_range)

    return time_range


def getObservationStop(vis, obsid=0):
    """
    Read the stop time of the observation and report it in MJD seconds.
    -Todd Hunter
    """
    if not os.path.exists(vis):
        print "vis does not exist = %s" % (vis)
        return 2
    if not os.path.exists(vis+'/table.dat'):
        print "No table.dat.  This does not appear to be an ms."
        return
    mytb = casatools.table
    try:
        mytb.open(vis+'/OBSERVATION')
    except:
        print "ERROR: failed to open OBSERVATION table on file "+vis
        return 3

    time_range = mytb.getcol('TIME_RANGE')[:,obsid]
    mytb.close()
    if type(time_range) is np.ndarray:
        time_range = np.max(time_range)
    return time_range


def parseFieldArgument(vis, field, blendByName=True):
    """
    Takes a single field ID (integer or string) or field name, converts to
    field name, then finds all field IDs associated with that name.
    Returns: a list of IDs and names.
    This is useful for gathering all field IDs of the science target
    in a mosaic, which is needed for tsysspwmapWithNoTable.
    blendByName: if True, then get fields for name after translating from ID.
    -Todd Hunter
    """
    mymsmd = casatools.msmd
    mymsmd.open(vis)
    nfields = mymsmd.nfields()
    if type(field) in (int, np.int32, np.int64):
        if (field >= nfields or int(field) < 0):
            print "There are only %d fields in this dataset" % (nfields)
            return
        fieldnames = mymsmd.namesforfields(field)
        if blendByName:
            fieldIDlist = mymsmd.fieldsforname(fieldnames[0])
        else:
            fieldIDlist = [field]
    elif field.isdigit():
        if int(field) >= nfields or int(field) < 0:
            print "There are only %d fields in this dataset" % (nfields)
            return
        fieldnames = mymsmd.namesforfields(int(field))
        if blendByName:
            fieldIDlist = mymsmd.fieldsforname(fieldnames[0])
        else:
            fieldIDlist = [field]
    elif type(field) is str: # assume string type
        if field not in mymsmd.namesforfields():
            print "Field name is not in this dataset."
            return
        fieldIDlist = mymsmd.fieldsforname(field)
        fieldnames = [field]
    else:
        print "field parameter must be an integer or string"
        return
    mymsmd.close()
    return fieldIDlist, fieldnames


def ComputeLST(mjdsec=None, longitude=ALMA_LONGITUDE, ut=None, hms=False,
               observatory=None, date=None, verbose=False, prec=0):
    """
    Computes the LST (in hours) for a specified time and longitude.
    The input longitude is in degrees, where east of Greenwich is > 0.
    Two options to specify the time:
    mjdsec: MJD seconds
    ut: ut time on the current day as a HH:MM:SS string or floating point hours
    hms: if True, return the time as a HH:MM:SS string
    observatory: if specified, ignore the longitude argument
    prec: digits of fractional seconds to show
    date: a date/time string
    Either of these formats is valid: 2011/10/15 05:00:00
                                      2011/10/15-05:00:00
                                      2011-10-15 05:00:00
                                      2011-10-15T05:00:00
                                      2011-Oct-15T05:00:00
    If mjdsec, ut and date are all None, then it uses the current date+time.
    -- Todd Hunter
    """
    if (mjdsec is None and ut is None and date is None):
        mjdsec = getCurrentMJDSec()
    elif (ut is not None):
        if (date is None):
            datestring = getCurrentDate()
        else:
            datestring = date
        if (type(ut) == str):
            datestring += ' ' + ut
        else:
            minutes = int(60 * (ut - int(ut)))
            datestring += ' %02d:%02d:%02.0f' % (int(ut), minutes, 3600 * (ut - int(ut) - minutes / 60.))
        mjdsec = dateStringToMJDSec(datestring, verbose=False)
    elif (date is not None):
        mjdsec = dateStringToMJDSec(date, verbose=False)
    if verbose: print "MJD seconds = ", mjdsec
    JD = mjdToJD(mjdsec / 86400.)
    T = (JD - 2451545.0) / 36525.0
    sidereal = 280.46061837 + 360.98564736629 * (JD - 2451545.0) + 0.000387933 * T * T - T * T * T / 38710000.

    # now we have LST in Greenwich, need to scale back to site
    if (observatory is not None):
        longitude = getObservatoryLongitude(observatory)
        if (longitude is None): return
    sidereal += longitude
    sidereal /= 360.
    sidereal -= np.floor(sidereal)
    sidereal *= 24.0
    if (sidereal < 0):
        sidereal += 24
    if (sidereal >= 24):
        sidereal -= 24
    if (hms):
        return (hoursToHMS(sidereal, prec=prec))
    else:
        return (sidereal)


def getCurrentDate(delimiter='/'):
    """
    returns date in format: 'YYYY-MM-DD'
    -Todd Hunter
    """
    return timeUtilities.strftime('%Y/%m/%d').replace('/',delimiter)


def hoursToHMS(hours, prec=3):
    """
    Converts decimal hours (or a list of hours) into HH:MM:SS.    See also hmsToHours.
    Todd Hunter
    """
    if type(hours) not in (list, np.ndarray):
        hrs = [hours]
    else:
        hrs = hours
    mystring = []
    for h in hrs:
        minutes = int(60 * (h - int(h)))
        seconds = 3600 * (h - int(h) - minutes / 60.)
        mystring.append('%02d:%02d:%02.*f' % (int(h), minutes, prec, seconds))
    if type(hours) not in (list, np.ndarray):
        mystring = mystring[0]
    return mystring


def mjdToJD(MJD=None):
    """
    Converts an MJD value to JD.  Default = now.
    """
    if MJD is None:
        MJD = getMJD()
    return MJD + 2400000.5


def getObservatoryLongitude(observatory='', verbose=False):
    """
    Opens the casa table of known observatories and returns the latitude and longitude
    in degrees for the specified observatory name string.
    observatory: string name, JPL integer, or integer string  (e.g. 'ALMA' == -7)
    -Todd Hunter
    """
    _, longitude, _ = getObservatoryLatLong(observatory, verbose)
    return longitude[0]
