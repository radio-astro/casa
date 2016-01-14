#!/usr/bin/env python
import pylab as pb
import numpy as np
import os, math
import scipy
import glob
import matplotlib.dates
import time as timeUtilities
import datetime
from taskinit import *  # needed for tbtool
import casadef
c_mks=2.99792458e8
JPL_HORIZONS_ID = {'ALMA': '-7',
                   'VLA': '-5',
                   'GBT': '-9',
                   'MAUNAKEA': '-80',
                   'OVRO': '-81',
                   'geocentric': '500'
}
majorPlanets = ['SUN','MERCURY','VENUS','MOON','MARS','JUPITER','SATURN','URANUS','NEPTUNE','PLUTO']

def version(short=False):
    """
    Returns the CVS revision number.
    """
    myversion = "$Id: plotTPSampling.py,v 1.1 2015/09/03 20:31:20 thunter Exp $"
    if (short):
        myversion = myversion.split()[2]
    return myversion

def setXaxisTimeTicks(adesc, t0, t1, verbose=False):
    """
    Sets sensible major and minor tick intervals for a plot_date plot
    based on the start and end times.
    Inputs: t0 (startTime in seconds)
        and t1 (endTime in seconds)
        Only the difference matters.
   -Todd Hunter
    """
    timeRange = t1-t0
    if (verbose):
        print "timeRange = %f sec" % (timeRange)
    if (timeRange > 20000):
        adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H'))
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,int(60*np.floor((t1-t0)/3600)),60)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,15)))
        adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H')
    elif (timeRange > 2000):
        adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,15)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,5)))
        adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    elif (timeRange > 600):
        adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,5)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.SecondLocator(bysecond=range(0,60,30)))
        adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    elif (timeRange > 200):
        adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,3)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.SecondLocator(bysecond=range(0,60,10)))
        adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    elif (timeRange > 40):
        adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M:%S'))
        adesc.xaxis.set_major_locator(matplotlib.dates.SecondLocator(bysecond=range(0,60,20)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.SecondLocator(bysecond=range(0,60,5)))
        adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M:%S')
    else:
        adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M:%S'))
        adesc.xaxis.set_major_locator(matplotlib.dates.SecondLocator(bysecond=range(0,60,10)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.SecondLocator(bysecond=range(0,60,1)))
        adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M:%S')

def scansforfields(mymsmd, calAtmFields):
    scans = []
    for f in calAtmFields:
        scans += list(mymsmd.scansforfield(f))
    return(np.array(scans))

def ComputeJulianDayFromUnixTime(seconds):
    """
    Converts a time expressed in unix time (seconds since Jan 1, 1970)
    into Julian day number as a floating point value.
    - Todd Hunter
    """
    [tm_year, tm_mon, tm_mday, tm_hour, tm_min, tm_sec, tm_wday, tm_yday, tm_isdst] = timeUtilities.gmtime(seconds)
    if (tm_mon < 3):
        tm_mon += 12
        tm_year -= 1
    UT = tm_hour + tm_min/60. + tm_sec/3600.
    a =  floor(tm_year / 100.)
    b = 2 - a + floor(a/4.)
    day = tm_mday + UT/24.
    jd  = floor(365.25*((tm_year)+4716)) + floor(30.6001*((tm_mon)+1))  + day + b - 1524.5
    return(jd) 

def mjdsecToUT(mjdsec=None, prec=6):
    """
    Converts an MJD seconds value to a UT date and time string
    such as '2012-03-14 00:00:00 UT'
    """
    if mjdsec==None: mjdsec = getCurrentMJDSec()
    utstring = mjdSecondsToMJDandUT(mjdsec, prec=prec)[1]
    return(utstring)
        
def mjdSecondsListToDateTime(mjdsecList):
    """
    Takes a list of mjd seconds and converts it to a list of datetime 
    structures.
    - Todd Hunter
    """
    me = createCasaTool(metool)
    dt = []
    typelist = type(mjdsecList)
    if not (typelist == list or typelist == np.ndarray):
        mjdsecList = [mjdsecList]
    for mjdsec in mjdsecList:
        today = me.epoch('utc','today')
        mjd = mjdsec / 86400.
        today['m0']['value'] =  mjd
        hhmmss = call_qa_time(today['m0'])
        date = qa.splitdate(today['m0'])  # date is now a dict
        timeString = '%d-%d-%d %d:%d:%d.%06d'%(date['monthday'],date['month'],date['year'],date['hour'],date['min'],date['sec'],date['usec'])
        mydate = datetime.datetime.strptime(timeString,'%d-%m-%Y %H:%M:%S.%f')
        # previous implementation truncated to nearest second!
#        mydate = datetime.datetime.strptime('%d-%d-%d %d:%d:%f'%(date['monthday'],date['month'],date['year'],date['hour'],date['min'],date['s']),'%d-%m-%Y %H:%M:%S')
        dt.append(mydate)
    return(dt)

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
            aswp = swapaxes(a[good],0,axis)
        else:
            aswp = a[good]
        m = np.median(np.fabs(aswp - d) / c, axis=0)

    return m

def mjdSecondsToMJDandUT(mjdsec, use_metool=True, debug=False, prec=6):
    """
    Converts a value of MJD seconds into MJD, and into a UT date/time string.
    prec: 6 means HH:MM:SS,  7 means HH:MM:SS.S
    example: (56000.0, '2012-03-14 00:00:00 UT')
    Caveat: only works for a scalar input value
    Todd Hunter
    """
    if (os.getenv('CASAPATH') == None or use_metool==False):
        mjd = mjdsec / 86400.
        jd = mjdToJD(mjd)
        trialUnixTime = 1200000000
        diff  = ComputeJulianDayFromUnixTime(trialUnixTime) - jd
        if (debug): print "first difference = %f days" % (diff)
        trialUnixTime -= diff*86400
        diff  = ComputeJulianDayFromUnixTime(trialUnixTime) - jd
        if (debug): print "second difference = %f seconds" % (diff*86400)
        trialUnixTime -= diff*86400
        diff  = ComputeJulianDayFromUnixTime(trialUnixTime) - jd
        if (debug): print "third difference = %f seconds" % (diff*86400)
        # Convert unixtime to date string 
        utstring = timeUtilities.strftime('%Y-%m-%d %H:%M:%S UT', 
                       timeUtilities.gmtime(trialUnixTime))
    else:
        me = createCasaTool(metool)
        today = me.epoch('utc','today')
        mjd = np.array(mjdsec) / 86400.
        today['m0']['value'] =  mjd
        hhmmss = call_qa_time(today['m0'], prec=prec)
        date = qa.splitdate(today['m0'])
        utstring = "%s-%02d-%02d %s UT" % (date['year'],date['month'],date['monthday'],hhmmss)
    return(mjd, utstring)

def call_qa_time(arg, form='', prec=0, showform=False):
    """
    This is a wrapper for qa.time(), which in casa 4.0.0 returns a list 
    of strings instead of just a scalar string.  
    - Todd Hunter
    """
    if (type(arg) == dict):
        if (type(arg['value']) == list or 
            type(arg['value']) == np.ndarray):
            if (len(arg['value']) > 1):
                print "WARNING: call_qa_time() received a dictionary containing a list of length=%d rather than a scalar. Using first value." % (len(arg['value']))
            arg['value'] = arg['value'][0]
    result = qa.time(arg, form=form, prec=prec, showform=showform)
    if (type(result) == list or type(result) == np.ndarray):
        return(result[0])
    else:
        return(result)

def getRADecForField(vis, field, usemstool=True, forcePositiveRA=False, verbose=False):
    """
    Returns [RA,Dec] in radians for the specified field in the specified ms.
    field: can be integer or string integer.  
    -- Todd Hunter
    """
    if (not os.path.exists(vis)):
        print "Could not find measurement set"
        return
    if (casadef.casa_version >= '4.2.0' and  usemstool):
        result = parseFieldArgument(vis, field)
        if (result == None): return
        idlist, namelist = result
        myms = createCasaTool(mstool)
        myms.open(vis)
        if (type(field) == str):
            if (not field.isdigit()):
                field = idlist[0]
            field = int(field)
        if verbose: print "running myms.getfielddirmeas(fieldid=%d)" % (field)
        mydir = myms.getfielddirmeas(fieldid=field) # dircolname defaults to 'PHASE_DIR'
        mydir = np.array([[mydir['m0']['value']], [mydir['m1']['value']]]) # simulates tb.getcell
        myms.close()
    else:
        mytb = createCasaTool(tbtool)
        try:
            mytb.open(vis+'/FIELD')
        except:
            print "Could not open FIELD table for ms=%s" % (vis)
            return([0,0])
        mydir = mytb.getcell('DELAY_DIR',int(field))
        mytb.close()
    if (forcePositiveRA):
        if (mydir[0] < 0):
            mydir[0] += 2*math.pi
    return(mydir)

def parseFieldArgument(vis, field):
    """
    Takes a single field ID (integer or string) or field name, converts to 
    field name, then finds all field IDs associated with that name.
    Returns: a list of IDs and names.  
    This is useful for gathering all field IDs of the science target 
    in a mosaic, which is needed for tsysspwmapWithNoTable.
    -Todd Hunter
    """
    mymsmd = createCasaTool(msmdtool)
    mymsmd.open(vis)
    nfields = mymsmd.nfields()
    if (type(field) == int or type(field) == np.int32 or type(field) == np.int64):
        if (field >= nfields or int(field) < 0):
            print "There are only %d fields in this dataset" % (nfields)
            return
        fieldnames = mymsmd.namesforfields(field)
        fieldIDlist = mymsmd.fieldsforname(fieldnames[0])
    elif (field.isdigit()):
        if (int(field) >= nfields or int(field) < 0):
            print "There are only %d fields in this dataset" % (nfields)
            return
        fieldnames = mymsmd.namesforfields(int(field))
        fieldIDlist = mymsmd.fieldsforname(fieldnames[0])
    elif (type(field) == str): # assume string type
        if (field not in mymsmd.namesforfields()):
            print "Field name is not in this dataset."
            return
        fieldIDlist = mymsmd.fieldsforname(field)
        fieldnames = [field]
    else:
        print "field parameter must be an integer or string"
        return
    mymsmd.close()
    return(fieldIDlist, fieldnames)

def getOffSourceTimes(vis):
    """
    Get the timestamps when observing the OFF_SOURCE position.
    -Todd Hunter
    """
    mytb = createCasaTool(tbtool)
    mytb.open(vis+'/STATE')
    obsMode = mytb.getcol('OBS_MODE')
    offSourceStateIds = []
    for i,o in enumerate(obsMode):
        if ('OBSERVE_TARGET#OFF_SOURCE' in o):
            offSourceStateIds.append(i)
#    print "State_IDs with OBSERVE_TARGET#OFF_SOURCE: ", offSourceStateIds
    mytb.close()
    mytb.open(vis)
    stateId = mytb.getcol('STATE_ID')
    indices = np.array([],dtype=int)
    for o in range(len(offSourceStateIds)):
        indices = np.append(indices,np.where(stateId == offSourceStateIds[o]))
    print "Found %d/%d rows in the MS corresponding to OBSERVE_TARGET#OFF_SOURCE" % (len(indices),len(stateId))
    times = mytb.getcol('TIME')[indices]
    mytb.close()
    return(times)

def rad2radec(ra=0,dec=0,imfitdict=None, prec=5, verbose=True, component=0,
              replaceDecDotsWithColons=True, hmsdms=False, delimiter=', ',
              prependEquinox=False, hmdm=False):
    """
    Convert a position in RA/Dec from radians to sexagesimal string which
    is comma-delimited, e.g. '20:10:49.01, +057:17:44.806'.
    The position can either be entered as scalars via the 'ra' and 'dec' 
    parameters, as a tuple via the 'ra' parameter, as an array of shape (2,1)
    via the 'ra' parameter, or
    as an imfit dictionary can be passed via the 'imfitdict' argument, and the
    position of component 0 will be displayed in RA/Dec sexagesimal.
    replaceDecDotsWithColons: replace dots with colons as the Declination d/m/s delimiter
    hmsdms: produce output of format: '20h10m49.01s, +057d17m44.806s'
    hmdm: produce output of format: '20h10m49.01, +057d17m44.806' (for simobserve)
    delimiter: the character to use to delimit the RA and Dec strings output
    prependEquinox: if True, insert "J2000" before coordinates (i.e. for clean)
    Todd Hunter
    """
    if (type(imfitdict) == dict):
        comp = 'component%d' % (component)
        ra  = imfitdict['results'][comp]['shape']['direction']['m0']['value']
        dec = imfitdict['results'][comp]['shape']['direction']['m1']['value']
    if (type(ra) == tuple or type(ra) == list):
        dec = ra[1]
        ra = ra[0]
    if (np.shape(ra) == (2,1)):
        dec = ra[1][0]
        ra = ra[0][0]
    if (os.getenv('CASAPATH') == None):
        if (ra<0): ra += 2*pi
        rahr = ra*12/np.pi
        decdeg = dec*180/np.pi
        hr = int(rahr)
        min = int((rahr-hr)*60)
        sec = (rahr-hr-min/60.)*3600
        if (decdeg < 0):
            mysign = '-'
        else:
            mysign = '+'
        decdeg = abs(decdeg)
        d = int(decdeg)
        dm = int((decdeg-d)*60)
        ds = (decdeg-d-dm/60.)*3600
        mystring = '%02d:%02d:%08.5f, %c%02d:%02d:%08.5f' % (hr,min,sec,mysign,d,dm,ds)
    else:
        myqa = createCasaTool(qatool)
        myra = myqa.formxxx('%.12frad'%ra,format='hms',prec=prec+1)
        mydec = myqa.formxxx('%.12frad'%dec,format='dms',prec=prec-1)
        if replaceDecDotsWithColons:
            mydec = mydec.replace('.',':',2)
        mystring = '%s, %s' % (myra, mydec)
        myqa.done()
    if (hmsdms):
        mystring = convertColonDelimitersToHMSDMS(mystring)
        if (prependEquinox):
            mystring = "J2000 " + mystring
    elif (hmdm):
        mystring = convertColonDelimitersToHMSDMS(mystring, s=False)
        if (prependEquinox):
            mystring = "J2000 " + mystring
    if (delimiter != ', '):
        mystring = mystring.replace(', ', delimiter)
    if (verbose):
        print mystring
    return(mystring)

def angularSeparationRadians(ra0,dec0,ra1,dec1,returnComponents=False):
  """
  Computes the great circle angle between two celestial coordinates.
  using the Vincenty formula (from wikipedia) which is correct for all
  angles, as long as you use atan2() to handle a zero denominator.  
     See  http://en.wikipedia.org/wiki/Great_circle_distance
  Input and output are in radians.  It also works for the az,el coordinate system.
  returnComponents=True will return: [separation, raSeparation, decSeparation, raSeparationCosDec]
  See also angularSeparation()
  -- Todd Hunter
  """
  result = angularSeparation(ra0*180/math.pi, dec0*180/math.pi, ra1*180/math.pi, dec1*180/math.pi,returnComponents)
  if (returnComponents):
      return(np.array(result)*math.pi/180.)
  else:
      return(result*math.pi/180.)

def angularSeparation(ra0,dec0,ra1,dec1, returnComponents=False):
  """
  Computes the great circle angle between two celestial coordinates.
  using the Vincenty formula (from wikipedia) which is correct for all
  angles, as long as you use atan2() to handle a zero denominator.  
     See  http://en.wikipedia.org/wiki/Great_circle_distance
  ra,dec must be given in degrees, as is the output.
  It also works for the az,el coordinate system.
  Component separations are field_0 minus field_1.
  See also angularSeparationRadians()
  -- Todd Hunter
  """
  ra0 *= math.pi/180.
  dec0 *= math.pi/180.
  ra1 *= math.pi/180.
  dec1 *= math.pi/180.
  deltaLong = ra0-ra1
  argument1 = (((math.cos(dec1)*math.sin(deltaLong))**2) +
               ((math.cos(dec0)*math.sin(dec1)-math.sin(dec0)*math.cos(dec1)*math.cos(deltaLong))**2))**0.5
  argument2 = math.sin(dec0)*math.sin(dec1) + math.cos(dec0)*math.cos(dec1)*math.cos(deltaLong)
  angle = math.atan2(argument1, argument2) / (math.pi/180.)
  if (angle > 360):
      angle -= 360
  if (returnComponents):
      cosdec = math.cos((dec1+dec0)*0.5)
      radegreesCosDec = np.degrees(ra0-ra1)*cosdec
      radegrees = np.degrees(ra0-ra1)
      decdegrees = np.degrees(dec0-dec1)
      if (radegrees > 360):
          radegrees -= 360
      if (decdegrees > 360):
          decdegrees -= 360
#      positionAngle = -math.atan2(decdegrees*math.pi/180., radegreesCosDec*math.pi/180.)*180/math.pi
      retval = angle,radegrees,decdegrees, radegreesCosDec
  else:
      retval = angle
  return(retval)

def computeRADecFromAzElMJD(azel, mjd, observatory='ALMA', verbose=True,
                            my_metool=None, refractionCorrection=False,
                            nutationCorrection=False):
    """
    Computes the J2000 RA/Dec for a specified AZELGEO coordinate, MJD and
    observatory.

    azel must either be a tuple in radians
    mjd must either be in days, or a date string of the form:
               2011/10/15 05:00:00  or   2011/10/15-05:00:00
            or 2011-10-15 05:00:00  or   2011-10-15-05:00:00
    observatory: must be either a name recognized by the CASA me tool, or a
         JPL Horizons ID listed in the JPL_HORIZONS_ID dictionary at the top
         of this module.
    refractionCorrection: subtract the (positive) refractionCorrection to the
         elevation prior to conversion
    nutationCorrection: apply the nutation correction after conversion
    returns the [RA,Dec] in radians
    - Todd Hunter
    """
    if (observatory == 'MAUNAKEA'):
        # Convert from a value known to JPL Horizons to a value known to CASA"
        observatory = 'SMA'
    elif (observatory in JPL_HORIZONS_ID.values()):
        observatory = JPL_HORIZONS_ID.keys()[JPL_HORIZONS_ID.values().index(str(observatory))]
    myme = my_metool
    if (my_metool == None):
        myme = createCasaTool(metool)
    if (type(mjd) == str):
        mjd = dateStringToMJD(mjd)
        if (mjd == None):
            print "Invalid date string"
            return
        print "MJD = ", mjd
    if (refractionCorrection):
        azel[1] -= refraction(azel[1]*180/np.pi) / (180*3600/np.pi)
    mydir = myme.direction('AZELGEO', qa.quantity(azel[0],'rad'), qa.quantity(azel[1],'rad'))
    myme.doframe(myme.epoch('mjd', qa.quantity(mjd, 'd')))
    myme.doframe(myme.observatory(observatory))
    myradec = myme.measure(mydir,'J2000')
    myra = myradec['m0']['value']
    if (myra < 0):
        myra += 2*np.pi
    mydec = myradec['m1']['value']
    if (nutationCorrection):
        dRA, dDec = nutation([myra,mydec], mjdsec=mjd*86400.)
        myra += dRA*np.pi/180./3600.
        mydec += dDec*np.pi/180./3600.
    if (verbose):
        print "RA = %.3f hr   Dec = %.3f deg" % (myra*12/np.pi, mydec*180/np.pi)
    if (my_metool == None):
        myme.done()
    return([myra,mydec])

def utstring(mjdsec, xframeStart=110):
    (mjd, dateTimeString) = mjdSecondsToMJDandUT(mjdsec)
    tokens = dateTimeString.split()
    hoursMinutes = tokens[1][0:len(tokens[1])-3]
    hoursMinutesSeconds = tokens[1][0:len(tokens[1])]
    if (xframeStart == 110):  # 2011-01-01 UT 00:00
        return(tokens[0]+' '+tokens[2]+' '+hoursMinutes)
    elif (xframeStart == 3):
        return(hoursMinutesSeconds)
    else:  # 00:00
        return(hoursMinutes)
    
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

def primaryBeamArcsec(vis='', spw='', frequency='',wavelength='',
                      diameter=12.0, taper=10.0, obscuration=0.75,
                      verbose=False, showEquation=True, use2007formula=True,
                      fwhmfactor=None):
    """
    Implements the Baars formula: b*lambda / D.
      if use2007formula==False, use the formula from ALMA Memo 456        
      if use2007formula==True, use the formula from Baars 2007 book
        (see au.baarsTaperFactor)     
      In either case, the taper value is expected to be entered as positive.
        Note: if a negative value is entered, it is converted to positive.
    The effect of the central obstruction on the pattern is also accounted for
    by using a spline fit to Table 10.1 of Schroeder's Astronomical Optics.
    The default values correspond to our best knowledge of the ALMA 12m antennas.
      diameter: outer diameter of the dish in meters
      obscuration: diameter of the central obstruction in meters
      fwhmfactor: if given, then ignore the taper
    Specify one of the following combinations:
    0) vis and spw (uses median dish diameter)
    1) vis (uses median freq of OBSERVE_TARGET spws, and median dish diameter)
    2) frequency in GHz (assumes 12m)
    3) wavelength in mm (assumes 12m)
    4) frequency in GHz and diameter (m)
    5) wavelength in mm and diameter (m)
    For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/PrimaryBeamArcsec
    -- Todd Hunter
    """
    if (type(vis) != str):
        print "The first argument of primaryBeamArcsec is vis, which must be a string."
        print "Use frequency=100 or frequency='100GHz' to request 100 GHz."
        return
    if (fwhmfactor != None):
        taper = effectiveTaper(fwhmfactor,diameter,obscuration,use2007formula)
        if (taper == None): return
    if (taper < 0):
        taper = abs(taper)
    if (obscuration>0.4*diameter):
        print "This central obscuration is too large for the method of calculation employed here."
        return
    if (vis != ''):
      try:
          antennaTable = vis+'/ANTENNA'
          tb.open(antennaTable)
      except:
          print "Could not open table = %s" % antennaTable 
          return(0)
      diameter = np.median(np.unique(tb.getcol('DISH_DIAMETER')))
      print "Median dish diameter = %.1fm" % (diameter)
      tb.close()
      if (spw != ''):
        try:
            spwTable = vis+'/SPECTRAL_WINDOW'
            tb.open(spwTable)
            num_chan = tb.getcol('NUM_CHAN')
            refFreqs = tb.getcol('REF_FREQUENCY')
            tb.close()
        except:
            print "Could not open table = %s" % antennaTable 
            return(0)
        frequencyHz = refFreqs[spw]
        frequency = frequencyHz*1e-9
        if (verbose):
            print "Found frequency = %f GHz and dish diameter = %.1fm" % (frequency,diameter)
      else: # only the vis was given
          frequencyHz = medianFrequencyOfIntent(vis)
          frequency = frequencyHz * 1e-9
          print "Median OBSERVE_TARGET frequency = %.3f GHz" % (frequency)
    else:
        if (frequency != '' and wavelength != ''):
            print "You must specify either frequency or wavelength, not both!"
            return(0)
    if (frequency != ''):
        if (type(frequency) == str):
            frequency = parseFrequencyArgument(frequency)
        elif (frequency < 10000):
            frequency *= 1e9
        lambdaMeters = c_mks/frequency
    elif (wavelength != ''):
        lambdaMeters = wavelength*0.001
    else:
        print "You must specify either frequency (in GHz or a string with units) or wavelength (in mm)"
        return(0)
#  print "wavelength = %.3f mm" % (lambdaMeters*1000)
    b = baarsTaperFactor(taper,use2007formula) * centralObstructionFactor(diameter, obscuration)
    if (showEquation):
        if (use2007formula):
            formula = "Baars (2007) Eq 4.13"
        else:
            formula = "ALMA memo 456 Eq. 18"
        print "Coefficient from %s for a -%.1fdB edge taper and obscuration ratio=%g/%g = %.3f*lambda/D" % (formula, taper, obscuration, diameter, b)
    return(b*lambdaMeters*3600*180/(diameter*math.pi))

def effectiveTaper(fwhmFactor=1.16, diameter=12, obscuration=0.75, 
                   use2007formula=True):
    """
    The inverse of (Baars formula multiplied by the central
    obstruction factor).  Converts an observed value of the constant X in
    the formula FWHM=X*lambda/D into a taper in dB (positive value).
    if use2007formula == False, use Equation 18 from ALMA Memo 456     
    if use2007formula == True, use Equation 4.13 from Baars 2007 book
    -- Todd Hunter
    """
    cOF = centralObstructionFactor(diameter, obscuration)
    if (fwhmFactor < 1.02 or fwhmFactor > 1.22):
        print "Invalid fwhmFactor (1.02<fwhmFactor<1.22)"
        return
    if (baarsTaperFactor(10,use2007formula)*cOF<fwhmFactor):
        increment = 0.01
        for taper_dB in np.arange(10,10+increment*1000,increment):
            if (baarsTaperFactor(taper_dB,use2007formula)*cOF-fwhmFactor>0): break
    else:
        increment = -0.01
        for taper_dB in np.arange(10,10+increment*1000,increment):
            if (baarsTaperFactor(taper_dB,use2007formula)*cOF-fwhmFactor<0): break
    return(taper_dB)

def medianFrequencyOfIntent(vis, intent='OBSERVE_TARGET#ON_SOURCE', verbose=False,
                            ignoreChanAvgSpws=True):
    """
    Returns the median of the mean frequency (in Hz) of the spws observed with
    the specified intent (default = OBSERVE_TARGET#ON_SOURCE).
    ignoreChanAvgSpws: if True, then ignore single channel spws, unless they 
                       are the only ones present!
    -Todd Hunter
    """
    if (not os.path.exists(vis)):
        print "Could not find measurement set."
        return
    mymsmd = createCasaTool(msmdtool)
    mymsmd.open(vis)
    try:
        spws = mymsmd.spwsforintent(intent)
        spws = list(set(spws).difference(set(mymsmd.wvrspws())))
        myspws = spws[:]
        if (ignoreChanAvgSpws):
            spws = list(set(spws).difference(set(mymsmd.almaspws(chavg=True))))
            if (len(spws) == 0):
                spws = list(set(myspws).difference(set(mymsmd.wvrspws())))
    except:
        spws = []
    if (len(spws) < 1):
        print "No spws with intent = '%s'" % (intent)
        print "intents = ", mymsmd.intents()
        mymsmd.close()
        return None
    if verbose: print "spws = ", spws
    freq = []
    for spw in spws:
        freq.append(mymsmd.meanfreq(spw))
        if verbose: print "spws %d = %f GHz" % (spw,freq[-1]*1e-9)
    return(np.median(freq))

def centralObstructionFactor(diameter=12.0, obscuration=0.75):
    """
    Computes the scale factor of an Airy pattern as a function of the
    central obscuration, using Table 10.1 of Schroeder's "Astronomical Optics".
    -- Todd Hunter
    """
    epsilon = obscuration/diameter
    myspline = scipy.interpolate.UnivariateSpline([0,0.1,0.2,0.33,0.4], [1.22,1.205,1.167,1.098,1.058], s=0)
    factor = myspline(epsilon)/1.22
    if (type(factor) == np.float64):
        # casapy 4.2
        return(factor)
    else:
        # casapy 4.1 and earlier
        return(factor[0])
    
def baarsTaperFactor(taper_dB, use2007formula=True):
    """
    Converts a taper in dB to the constant X
    in the formula FWHM=X*lambda/D for the parabolic illumination pattern.
    We assume that taper_dB comes in as a positive value.
    use2007formula:  False --> use Equation 18 from ALMA Memo 456.
                     True --> use Equation 4.13 from Baars 2007 book
    - Todd Hunter
    """
    tau = 10**(-0.05*taper_dB)
    if (use2007formula):
        return(1.269 - 0.566*tau + 0.534*(tau**2) - 0.208*(tau**3))
    else:
        return(1.243 - 0.343*tau + 0.12*(tau**2))

def mjdSecondsToMJDandUT(mjdsec, use_metool=True, debug=False, prec=6):
    """
    Converts a value of MJD seconds into MJD, and into a UT date/time string.
    prec: 6 means HH:MM:SS,  7 means HH:MM:SS.S
    example: (56000.0, '2012-03-14 00:00:00 UT')
    Caveat: only works for a scalar input value
    Todd Hunter
    """
    if (os.getenv('CASAPATH') == None or use_metool==False):
        mjd = mjdsec / 86400.
        jd = mjdToJD(mjd)
        trialUnixTime = 1200000000
        diff  = ComputeJulianDayFromUnixTime(trialUnixTime) - jd
        if (debug): print "first difference = %f days" % (diff)
        trialUnixTime -= diff*86400
        diff  = ComputeJulianDayFromUnixTime(trialUnixTime) - jd
        if (debug): print "second difference = %f seconds" % (diff*86400)
        trialUnixTime -= diff*86400
        diff  = ComputeJulianDayFromUnixTime(trialUnixTime) - jd
        if (debug): print "third difference = %f seconds" % (diff*86400)
        # Convert unixtime to date string 
        utstring = timeUtilities.strftime('%Y-%m-%d %H:%M:%S UT', 
                       timeUtilities.gmtime(trialUnixTime))
    else:
        me = createCasaTool(metool)
        today = me.epoch('utc','today')
        mjd = np.array(mjdsec) / 86400.
        today['m0']['value'] =  mjd
        hhmmss = call_qa_time(today['m0'], prec=prec)
        date = qa.splitdate(today['m0'])
        utstring = "%s-%02d-%02d %s UT" % (date['year'],date['month'],date['monthday'],hhmmss)
    return(mjd, utstring)

def mjdToUT(mjd=None, use_metool=True, prec=6):
    """
    Converts an MJD value to a UT date and time string
    such as '2012-03-14 00:00:00 UT'
    use_metool: whether or not to use the CASA measures tool if running from CASA.
         This parameter is simply for testing the non-casa calculation.
    """
    if mjd==None:
        mjdsec = getCurrentMJDSec()
    else:
        mjdsec = mjd*86400
    utstring = mjdSecondsToMJDandUT(mjdsec, use_metool, prec=prec)[1]
    return(utstring)
        
def getCurrentMJDSec():
    """
    Returns the current MJD in seconds.
    Todd Hunter
    """
    mjdsec = getMJD() * 86400
    return(mjdsec)

def getMJD():
    """
    Returns the current MJD.  See also getCurrentMJDSec().
    -Todd
    """
    myme = createCasaTool(metool)
    mjd = me.epoch('utc','today')['m0']['value']
    myme.done()
    return(mjd)
    
def getObservationStart(vis, obsid=0):
    """
    Read the start time of the observation and report it in MJD seconds.
    -Todd Hunter
    """
    if (os.path.exists(vis) == False):
        print "vis does not exist = %s" % (vis)
        return
    if (os.path.exists(vis+'/table.dat') == False):
        print "No table.dat.  This does not appear to be an ms."
        print "Use au.getObservationStartDateFromASDM()."
        return
    mytb = createCasaTool(tbtool)
    try:
        mytb.open(vis+'/OBSERVATION')
    except:
        print "ERROR: failed to open OBSERVATION table on file "+vis
        return(3)

    time_range = mytb.getcol('TIME_RANGE')[:,obsid]
    mytb.close()
    if (type(time_range) == np.ndarray):
        time_range = np.min(time_range)
    return(time_range)

def getEphemeris(vis, ephemerisId=-1, useASDM_Ephemeris=False, verbose=True):
    """
    Reads the ephemeris information from a measurement set.
    Inputs:
    use_ASDM_EPHEMERIS: if True then use the ASDM_EPHEMERIS table
          if False, then search the FIELD subdirectory 
          for files of the name format: EPHEMx_fieldname_MJDsec.tab.
    ephemerisId: if specified, then limit the return value to that ephemeris ID.
    Returns a dictionary keyed by ephemerisID with a value of a list of 3 lists:
    * {'0': [MJD seconds, RA_radian, Dec_radian]}
    """
    result = {}
    if (not os.path.exists(vis)):
        print "Could not find measurement set"
        return result
    if (not os.path.exists(vis+'/FIELD')):
        print "Could not find FIELD directory.  This does not appear to be an ms."
        return result
    if (not os.path.exists(vis+'/ASDM_EPHEMERIS') or not useASDM_Ephemeris):
        ephemerisTables = glob.glob(vis+'/FIELD/EPHEM*.tab')
        if (len(ephemerisTables) < 1):
            if verbose: print "Could not find any EPHEMx*.tab files in the FIELD directory"
            return result
        mytb = createCasaTool(tbtool)
        nEphem = len(ephemerisTables)
        print "There are %d ephemeris tables in the FIELD directory." % (nEphem)
        if (ephemerisId < 0):
            if (nEphem == 1):
                ephemerisId = 0
            else:
                print "You must select one using the ephemerisId parameter."
                return result
        for i in range(nEphem):
            mytb.open(ephemerisTables[i])
            keywords = mytb.getkeywords()
            if ('obsloc' in keywords):
                obsloc = keywords['obsloc']
            else:
                obsloc = ''
            mjd = mytb.getcol('MJD')
            raDeg = mytb.getcol('RA')
            decDeg = mytb.getcol('DEC')
            difference = (mjd[0]*86400-getObservationStart(vis))/60.
            if (obsloc != ''):
                obsloc  = '(obsloc=%s)' % (obsloc)
            else:
                obsloc = ''
            if (difference < 0):
                print "Ephemeris %d %s begins %.1f minutes before observation begins and has %d rows." % (i, obsloc, abs(difference), len(mjd))
            else:
                print "Ephemeris %d %s begins %.1f minutes after observation begins and has %d rows." % (i, obsloc, difference, len(mjd))
            difference = (mjd[-1]*86400-getObservationStop(vis))/60.
            minutes = 1440*(mjd[1]-mjd[0])
            if (difference < 0):
                print "Ephemeris %d ends %f minutes before observation ends and row spacing is %g minutes." % (i, abs(difference), minutes)
            else:
                print "Ephemeris %d ends %f minutes after observation ends and row spacing is %g minutes." % (i, difference, minutes)
            if (ephemerisId == i or ephemerisId < 0):
                mjdsec = mjd[:]*86400
                ra = np.radians(raDeg[:])
                dec = np.radians(decDeg[:])
                result[i] = [mjdsec, ra, dec]
                
        mytb.close()
        return(result)

    if (not os.path.exists(vis+'/ASDM_EPHEMERIS')):
        print "Could not find ASDM_EPHEMERIS table in this ms."
        return
    mytb = createCasaTool(tbtool)
    mytb.open(vis+'/ASDM_EPHEMERIS')
    direction = mytb.getcol('dir')
    ephemerisIds = mytb.getcol('ephemerisId')
    if (len(direction) == 0):
        print "The ephemeris table is blank."
        mytb.close()
        return result
    timeInterval = mytb.getcol('timeInterval')
    nEphem = len(np.unique(ephemerisIds))
    print "There are %d ephemerides in the table and %d total rows." % (nEphem, len(ephemerisIds))
    mjdsec = timeInterval[0]
    raRadian = direction[0][0]
    decRadian = direction[0][1]
    if (ephemerisId >= 0):
        idx = np.where(ephemerisId==ephemerisIds)
        mjdsec = mjdsec[idx]
        raRadian = raRadian[idx]
        decRadian = decRadian[idx]
    mytb.close()
    difference = (mjdsec[0]-getObservationStart(vis))/60.
    if (difference < 0):
        if (ephemerisId < 0): 
            ephemerisId = ephemerisIds[0]
        print "Ephemeris %d begins %f minutes before observation begins." % (ephemerisId, abs(difference))
    else:
        print "Ephemeris begins %f minutes after observation begins." % (difference)
    print "and it is %f minutes long" % ((mjdsec[-1]-mjdsec[0])/60.)
    result = {ephemerisId: [mjdsec, raRadian, decRadian]}
    return(result)
    
def ignoreMostCommonPosition(x,y):
    """
    Given two equal-lengths lists of coordinates, return the index of all pairs that
    *excludes* the most common pair.
    -Todd Hunter
    """
    roundedY = list(np.round(y))
    y_mode = max(set(roundedY), key=roundedY.count)
    threshold = max(1,abs(y_mode*0.005))
    idx1_ignoreOffPositionY = np.where(np.abs(np.round(y) - y_mode) > threshold)
    roundedX = list(np.round(x))
    x_mode = max(set(roundedX), key=roundedX.count)
    idx1_ignoreOffPositionX = np.where(np.round(x) != x_mode)
    idx1_ignoreOffPosition = np.intersect1d(idx1_ignoreOffPositionX[0], idx1_ignoreOffPositionY[0])
    return(idx1_ignoreOffPosition, idx1_ignoreOffPositionX, idx1_ignoreOffPositionY)

def plotTPSampling(vis, obsid=0, plotfile='', debug=False,
                   labelFirstNSamples=0, labelIncrement=1, convert=True,
                   field='auto', plotrange=[0,0,0,0], antenna=0, scan=None,
                   refractionCorrection=False, nutationCorrection=False, 
                   timerange=None, trimPointingData=True, showComponents=False,
                   pickFirstRaster=False, useApparentPositionForPlanets=False, 
                   connectDots=False, intent='OBSERVE_TARGET', 
                   findSubscans=False, magnification=0.3):
    """
    This function reads the main table of the ms, finds the
    start and end time of the specified observation ID, then
    read the POINTING table and extracts the pointing directions
    within that timerange.  It computes successive differences
    on both axes to determine which is the scan direction  (RA or Dec)
    and what the sampling is in each axes, which it returns in
    units of arcseconds on-the-sky, i.e. corrected for the
    right ascension axis by multiplying by cos(declination).
    It also generates a png file showing the observed points
    in relative coordinates in units of arc seconds.
    Note: This function does not support arbitrary scanning angles!

    vis: the name of the measurement set
    obsid: the number of the OBSERVATION_ID to analyze
    plotfile: default='' -->  '<vis>.obsid0.sampling.png'
    labelFirstNSamples: put labels on the first N samples
    labelIncrement: the number of samples between labels
    convert: if True, convert AZELGEO coords to RA Dec in the plot
    field: plot the offset coordinates relative to this field ID (or name)
           default = 'auto' == the first field with OBSERVE_TARGET intent
    plotrange: set the plot axes ranges [x0,x1,y0,y1]
    pickFirstRaster: automatically set to True for major planets
        This avoids confusion due to the slowly changing source position.
    antenna: which antenna ID (or name) to use to determine the scan parameters
    scan: default: use all scans on the first target with OBSERVE_TARGET intent
    timerange: limit the data to this timerange,  e.g. '05:00:00 06:00:00'
            or  '05:00~06:00' or '2011/10/15-05:00:00 2011/10/15-06:00:00'
    trimPointingData: if False, then don't exclude data outside of scan times
    showComponents: if True, then also show X and Y vs. time on upper plot
    intent: the intent for which to pick the scans to use
    findSubscans: if True, then run class Atmcal to find subscan times
    magnification: control how to determine when scans change direction
    
    Returns:
    xSampling, ySampling, largestDimension (all in units of arcsec)
    
    -Todd Hunter
    """
    if (os.path.exists(vis) == False):
        print "The ms does not exist."
        return
    if (os.path.exists(vis+'/table.dat') == False):
        print "No table.dat.  This does not appear to be an ms."
        return
    showplot = True
    if (field==''): 
        field = None
    mytb = createCasaTool(tbtool)
    mytb.open(vis)
    allObsIDs = mytb.getcol('OBSERVATION_ID')
    obsIDs = np.unique(allObsIDs)
    nObsIDs = len(obsIDs)
    print "There are %d rows and %d OBSERVATION_IDs in this dataset = %s" % (len(allObsIDs), nObsIDs, str(obsIDs))

    subtable = mytb.query('OBSERVATION_ID == %d' % (obsid))
    mytb.close()
    times = subtable.getcol('TIME')
    subtable.close()
    if (len(times) < 1):
        print "No rows found for OBSERVATION_ID=%d" % (obsid)
        return

    startTime = np.min(times)
    stopTime = np.max(times)
    mymsmd = createCasaTool(msmdtool)
    mymsmd.open(vis)
    timeranges = {}
    timecenters = []
    intent = intent.split('#')[0]
    if (intent+'#ON_SOURCE' not in mymsmd.intents()):
        if ('MAP_ANTENNA_SURFACE#ON_SOURCE' in mymsmd.intents()):
            intent = 'MAP_ANTENNA_SURFACE'
        else:
            print "%s#ON_SOURCE is not an intent in this measurement set." % (intent)
            print "Available intents = ", mymsmd.intents()
            mymsmd.close()
            return
    scansOnSource = list(mymsmd.scansforintent(intent+'#ON_SOURCE'))
    if (intent+'#OFF_SOURCE' in mymsmd.intents()):
        scansOnSource += list(mymsmd.scansforintent(intent+'#OFF_SOURCE'))
    scansOnSource = np.unique(scansOnSource)
    if (scan != None and scan != ''):
        scansToUse = [int(s) for s in str(scan).split(',')]
        for scan in scansToUse:
            if (scan not in scansOnSource):
                print "Scan %d is not an %s scan.  Available scans = %s" % (scan,intent,str(scansOnSource))
                mymsmd.close()
                return
        print "Using only the specified scans: %s" % (str(scansToUse))
    else:
        # should default to the first scan with specified intent
        if (len(scansOnSource) == 0):
            print "There are no scans on the with intent=%s to use." % (intent)
            print "Available intents = ", mymsmd.intents()
            print "Use the intent parameter to select one."
            mymsmd.close()
            return
        myfield = mymsmd.fieldsforscan(scansOnSource[0])[0]
        scansToUse = scansOnSource # mymsmd.scansforfield(myfield)
        print "Using only the %s scans on the science target: %s" % (intent,str(scansToUse))

    calAtmTimes = []
    firstCalAtmScan = -1
    if ('CALIBRATE_ATMOSPHERE#ON_SOURCE' in mymsmd.intents()):
        calAtmFields = mymsmd.fieldsforname(mymsmd.namesforfields(mymsmd.fieldsforintent(intent+'#ON_SOURCE')[0])[0])
        scans1 = mymsmd.scansforintent('CALIBRATE_ATMOSPHERE#ON_SOURCE')
        scans2 = scansforfields(mymsmd,calAtmFields)
        calAtmScans = np.intersect1d(scans1,scans2)
        if (len(calAtmScans) > 0):
            firstCalAtmScan = calAtmScans[0]
            firstCalAtmScanMeanMJD = np.mean(mymsmd.timesforscan(firstCalAtmScan))/86400.
            calAtmField = mymsmd.fieldsforscan(firstCalAtmScan)[0]
            calAtmTimes = mymsmd.timesforscan(firstCalAtmScan)
            calAtmTimesMax = np.max(calAtmTimes)
            calAtmTimesMin = np.min(calAtmTimes)
            print "first AtmCal scan on target = %d has %d times (%s-%s)" % (firstCalAtmScan,len(calAtmTimes),
                                                                             utstring(calAtmTimesMin,3),
                                                                             utstring(calAtmTimesMax,3))
            if (findSubscans):
                ac = Atmcal(vis)
                skyTimes = ac.timestamps[firstCalAtmScan][ac.skysubscan]
                print "sky subscan has times (%s-%s)" % (utstring(np.min(skyTimes),3),
                                                         utstring(np.max(skyTimes),3))
        else:
            print "No AtmCals were done on a field with intent %s (%s)" % (intent,calAtmField)
            print "scans1 = ", scans1
            print "scans2 = ", scans2
    else:
        print "No AtmCals in this dataset."

    timesforscan = {}
    if (timerange != None and timerange != ''):
        timerange = parseTimerangeArgument(timerange,vis)
        if (debug):
            print "timerange[0] = ", str(timerange[0])
            print "timerange[1] = ", str(timerange[1])
    for i in scansToUse:
        mytimes = mymsmd.timesforscan(i)
        if (timerange != None and timerange != ''):
            idx1 = np.where(mytimes > timerange[0])[0]
            idx2 = np.where(mytimes < timerange[1])[0]
            mytimes = mytimes[np.intersect1d(idx1,idx2)]
        if (len(mytimes) > 0):
            timesforscan[i] = {'begin': np.min(mytimes), 'end': np.max(mytimes)}
            timeranges[i] = [np.min(mytimes),np.max(mytimes)]
            timecenters += list(mytimes)
        else:
            scansToUse = np.delete(scansToUse,list(scansToUse).index(i))
            if (debug):
                print "No times found for scan %d" % (i)
    timecenters = np.array(timecenters)
    tSuccessiveDifferences = timecenters[1:] - timecenters[:-1]
    medianSamplingIntervalDataTable = np.median(tSuccessiveDifferences)
    print "median sampling interval in data table with OBSERVE_TARGET#ON_SOURCE (scans=%s) = %f sec" % (str(scansOnSource), medianSamplingIntervalDataTable)
    if (casadef.casa_version >= '4.2.0'):
        try:
            spw = np.intersect1d(mymsmd.spwsforintent(intent+'#ON_SOURCE'), mymsmd.almaspws(fdm=True,tdm=True))[0]
            pol = 0
            exposureTime = mymsmd.exposuretime(scan=scansToUse[0], spwid=spw, polid=pol)['value']
            print "Exposure time = %g in spw %d, pol %d" % (exposureTime,spw,pol)
            medianSamplingIntervalDataTable = exposureTime
        except:
            pass

    # find the name of the science field, and (if present) the scan numbers on it
    fieldName = None
    if (field == 'auto'):
        intent = intent + '#ON_SOURCE'
        if (intent not in mymsmd.intents()):
            print "No science target found.  Checking for amplitude or flux calibrator"
            if ('CALIBRATE_FLUX#ON_SOURCE' in mymsmd.intents()):
                intent = 'CALIBRATE_FLUX#ON_SOURCE'
            elif ('CALIBRATE_AMPLI#ON_SOURCE' in mymsmd.intents()):
                intent = 'CALIBRATE_AMPLI#ON_SOURCE'
            else:
                field = None
        if (field != None):
            field = mymsmd.fieldsforintent(intent)
            if (len(field) < 1):
                field = None
            else:
                field = field[0]
                fieldName = mymsmd.namesforfields(field)[0]
                print "Found first source with %s = %d = %s" % (intent.split('#')[0], field, fieldName)
    elif (field != None):
        if (field in mymsmd.namesforfields(range(mymsmd.nfields()))):
            fieldName = field
            field = mymsmd.fieldsforname(fieldName)[0]
        elif (str(field) in [str(a) for a in range(mymsmd.nfields())]):
            fieldName = mymsmd.namesforfields(int(field))[0]
        else:
            print "Field %s is not in this dataset.  Available fields = %s" % (field,str(mymsmd.namesforfields(range(mymsmd.nfields()))))
            mymsmd.close()
            return
    if (field != None):
        scansforfield = mymsmd.scansforfield(field)
    nantennas = mymsmd.nantennas()
    antennanames = mymsmd.antennanames(range(nantennas))
    
    mytb.open(vis+'/POINTING')
    coordSys = mytb.getcolkeyword('DIRECTION',1)['Ref']
    originalCoordSys = coordSys
    print "Map coordinate system from the POINTING table = ", coordSys
    alltimes = mytb.getcol('TIME')
    direction = mytb.getcol('DIRECTION')
    antenna_ids = mytb.getcol('ANTENNA_ID')
    uniqueAntennas = len(np.unique(antenna_ids))
    mytb.close()
    if (antenna not in antenna_ids):
        if (antenna not in antennanames):
            print "Antenna %s is not in the POINTING table" % (str(antenna))
            mymsmd.close()
            return
        else:
            antenna = mymsmd.antennaids(antenna)

    matches = np.where(antenna_ids == antenna)[0]
    print "Picked %d/%d rows, which correspond to antenna %d=%s (of %d)" % (len(matches), len(antenna_ids), antenna, antennanames[antenna], uniqueAntennas)
    times = alltimes[matches]
    xdirection = direction[0][0][matches]
    ydirection = direction[1][0][matches]

    # make sure RA/azimuth is in positive units
    negatives = np.where(xdirection < 0)[0]
    print "RA was negative at %d points" % (len(negatives))
    xdirection[negatives] += 2*np.pi

    uniqueTimes, uniqueTimesIndices = np.unique(times, return_index=True)
    if (debug):
        print "Mean time: %f, MJD = %f =  %s" % (np.mean(uniqueTimes),np.mean(uniqueTimes)/86400.,
                                                 mjdSecondsToMJDandUT(np.mean(uniqueTimes))[1])
    timeSortedIndices = np.argsort(uniqueTimes)

    # single-antenna times (sorted by time)
    pointingTime = times[uniqueTimesIndices[timeSortedIndices]]
    xdirection = xdirection[uniqueTimesIndices[timeSortedIndices]]
    ydirection = ydirection[uniqueTimesIndices[timeSortedIndices]]
    tdirection = times[uniqueTimesIndices[timeSortedIndices]]

    totalTimes = len(pointingTime)

    # Keep only the points from the requested ObsID
    matches1 = np.where(pointingTime >= startTime)[0]
    matches2 = np.where(pointingTime <= stopTime)[0]
    matches = np.intersect1d(matches1,matches2)
    matchesObsID = matches[:]
    myTimes = len(matches)
    if (debug):
        print "Selected %d of %d unique times (i.e. from one spw) matching OBSERVATION_ID=%d" % (myTimes, totalTimes, obsid)
    if (myTimes == 0):
        mymsmd.close()
        return

    if (trimPointingData):
      # Trim off any pointing table entries not associated with an integration in the selected scan(s)
        scanmatches = []
        scanfieldmatches = []
        pointingTime = np.array(pointingTime)
        for match in matches:
          for i in timeranges.keys():
              if (pointingTime[match] > timeranges[i][0] and  pointingTime[match] < timeranges[i][1]):
                  scanmatches.append(match)
                  if (field != None):
                      if (i in scansforfield):
                          scanfieldmatches.append(match)
                  break
        matches = np.array(scanmatches)  # times within any scan
        scanfieldmatches = np.array(scanfieldmatches)  # times within scans on the field
        myTimes = len(matches)
        if (debug):
            print "Selected %d times as being within a data scan (%s)" % (myTimes, scansToUse)
        if (myTimes == 0):
            mymsmd.close()
            return
        x = xdirection[matches]  # These are the longitude values in the pointing table for the selected scan
        y = ydirection[matches]  # These are the latitude values in the pointing table for the selected scan
        times = tdirection[matches]  # These are the timestamps in the pointing table for the selected scan
        if (firstCalAtmScan != -1):
            scanmatches = []
            for match in matchesObsID:
                if (pointingTime[match] > calAtmTimesMin and  pointingTime[match] < calAtmTimesMax):
                    scanmatches.append(match)
            matches = np.array(scanmatches)  
            xCalAtm = xdirection[matches]
            yCalAtm = ydirection[matches]
            timesCalAtmPointing = tdirection[matches]
            if (findSubscans):
                scanmatches = []
                for match in matchesObsID:
                    if (pointingTime[match] > np.min(skyTimes) and pointingTime[match] < np.max(skyTimes)):
                        scanmatches.append(match)
                matches = np.array(scanmatches)
                xCalAtmSky = xdirection[matches]
                yCalAtmSky = ydirection[matches]
                timesCalAtmSkyPointing = tdirection[matches]
    else:
        x = xdirection
        y = ydirection
        times = tdirection
    tSuccessiveDifferences = times[1:] - times[:-1]
    medianSamplingIntervalPointingTable = np.median(tSuccessiveDifferences)
    print "median sampling interval in POINTING table = %f sec" % (medianSamplingIntervalPointingTable)

    if (False):
        # Try to speed up the calculation by reducing the number of points.
        # Group the pointing table values into the correlator data time bins.
        # Does not yet work right.
        pointsPerIntegration = {}
        print "Assigning %d pointing table values to %d integrations" % (len(times),len(timecenters))
        for t in range(len(times)):
            mindiff = 1e30
            for integration in range(len(timecenters)):
                absdiff = abs(times[t]-timecenters[integration])
                if (absdiff < mindiff):
                    mindiff = absdiff
                    whichIntegration = integration
            if (whichIntegration not in pointsPerIntegration.keys()):
                pointsPerIntegration[whichIntegration] = {}
                pointsPerIntegration[whichIntegration]['x'] = []
                pointsPerIntegration[whichIntegration]['y'] = []
            pointsPerIntegration[whichIntegration]['x'].append(x[t])
            pointsPerIntegration[whichIntegration]['y'].append(y[t])
        xnew = []
        ynew = []
        times = timecenters
        for key in pointsPerIntegration.keys():
            xnew.append(np.median(pointsPerIntegration[key]['x']))
            ynew.append(np.median(pointsPerIntegration[key]['y']))
        x = np.array(xnew[:])
        y = np.array(ynew[:])
    
    # Keep a copy of the original values in radians
    xrad = x[:]
    yrad = y[:]

    if (field==None):
        rightAscension = np.median(x)
        declination = np.median(y)
        if (debug):
            print "Median coordinates = ", rightAscension, declination
    else:
        rightAscension, declination = getRADecForField(vis, field, forcePositiveRA=True, usemstool=True)
        if (debug):
            print "field coordinates in radians = %f, %f = %s" % (rightAscension, declination,
                                                                  rad2radec(rightAscension, declination))
        if (coordSys.find('AZEL') >= 0 and convert==False):
            rightAscension, declination = computeAzElFromRADecMJD([rightAscension,declination],
                                                                  mjd=np.median(times)/86400.,verbose=False)
            if (debug):
                print "field coordinates in az/el = ", rightAscension, declination

    apparentCoordinates = False
    offSourceTimes = getOffSourceTimes(vis)
    if (len(offSourceTimes) < 1):
        print "No off source intent found in the MS."

    offSourceRA = []
    offSourceDec = []
    calAtmRA = []
    calAtmDec = []
    if (coordSys.find('AZEL') >= 0 and convert):
        coordSys = 'J2000'
        print "Converting %d coordinates from AZELGEO to J2000..." % (len(x))
        my_metool = createCasaTool(metool)
        for i in range(len(x)):
            if ((i+1) % 10000 == 0): print "%d/%d" % (i+1,len(x))
            ra,dec = computeRADecFromAzElMJD([xrad[i],yrad[i]], mjd=times[i]/86400.,
                                             verbose=False,my_metool=my_metool,
                                             refractionCorrection=refractionCorrection,
                                             nutationCorrection=nutationCorrection)
            if (ra < 0):
                ra += 2*pi
            x[i] = ra
            y[i] = dec
            if (times[i] in offSourceTimes):
                offSourceRA.append(ra)
                offSourceDec.append(dec)
        if (len(offSourceRA) > 0):
            xOffSource = np.median(offSourceRA)
            yOffSource = np.median(offSourceDec)
            print "     Field source position = %s" % (rad2radec(rightAscension, declination,verbose=False))
            separation = np.degrees(angularSeparationRadians(rightAscension, declination, xOffSource, yOffSource))
            print "Median off source position = %s (separation = %.1farcmin = %.1fdeg)" % (rad2radec(xOffSource, yOffSource, verbose=False), separation*60, separation)
        elif (len(offSourceTimes) > 0):
            print "No times in the POINTING table match the times of off source integrations."


        medianRA = np.median(x)
        medianDec = np.median(y)
        separation = []
        for i in range(len(x)):
            separation.append(angularSeparationRadians(medianRA,medianDec,x[i],y[i]))
        madSeparation = MAD(separation)
        if (debug):
            print "MAD of the separation of points from median (%s) = %f arcsec" % (rad2radec(medianRA,medianDec),madSeparation*180*3600/np.pi)

        if (firstCalAtmScan != -1):
            print "Converting %d coordinates (for AtmCal scan %d) from AZELGEO to J2000..." % (len(xCalAtm),firstCalAtmScan)
            for i in range(len(xCalAtm)):
                if ((i+1) % 10000 == 0): print "%d/%d" % (i+1,len(x))
                ra,dec = computeRADecFromAzElMJD([xCalAtm[i],yCalAtm[i]], mjd=timesCalAtmPointing[i]/86400.,
                                                 verbose=False,my_metool=my_metool,
                                                 refractionCorrection=refractionCorrection,
                                                 nutationCorrection=nutationCorrection)
                calAtmRA.append(ra)
                calAtmDec.append(dec)
            if (len(calAtmRA) > 0):
                xCalAtm = np.median(calAtmRA)
                yCalAtm = np.median(calAtmDec)
                print "Median atm calib. position = %s" % (rad2radec(xCalAtm, yCalAtm, verbose=False))
                xCalAtm = np.mean(calAtmRA)
                yCalAtm = np.mean(calAtmDec)
                print "  Mean atm calib. position = %s" % (rad2radec(xCalAtm, yCalAtm, verbose=False))
            elif (len(calAtmTimes) > 0):
                print "No times in the POINTING table match the times of AtmCal integrations."
            else:
                print "There are no times found on AtmCal scans."
            if (findSubscans):
                calAtmSkyRA = []
                calAtmSkyDec = []
                for i in range(len(xCalAtmSky)):
                    if ((i+1) % 10000 == 0): print "%d/%d" % (i+1,len(x))
                    ra,dec = computeRADecFromAzElMJD([xCalAtmSky[i],yCalAtmSky[i]], mjd=timesCalAtmSkyPointing[i]/86400.,
                                                     verbose=False,my_metool=my_metool,
                                                     refractionCorrection=refractionCorrection,
                                                     nutationCorrection=nutationCorrection)
                    calAtmSkyRA.append(ra)
                    calAtmSkyDec.append(dec)
                if (len(calAtmSkyRA) > 0):
                    xCalAtmSky = np.median(calAtmSkyRA)
                    yCalAtmSky = np.median(calAtmSkyDec)
                    print "Median atmcal sky position = %s" % (rad2radec(xCalAtmSky, yCalAtmSky, verbose=False))
                    xCalAtmSky = np.mean(calAtmSkyRA)
                    yCalAtmSky = np.mean(calAtmSkyDec)
                    print "  Mean atmcal sky position = %s" % (rad2radec(xCalAtmSky, yCalAtmSky, verbose=False))
                
        my_metool.done()
        if (field == None):
            rightAscension = np.median(x)
            declination = np.median(y)
        else:
            newEphemerisTechnique = False
            if (casadef.casa_version >= '4.5'):
                if (getEphemeris(vis,verbose=False) != {}):
                     newEphemerisTechnique = True
            if (useApparentPositionForPlanets==False and fieldName.upper() in majorPlanets and 
                not newEphemerisTechnique):
                try:
                    rightAscension, declination = planet(fieldName,mjd=times[0]/86400.)['directionRadians']
                    if (firstCalAtmScan != -1):
                        rightAscensionCalAtm, declinationCalAtm = planet(fieldName,mjd=firstCalAtmScanMeanMJD)['directionRadians']
                except:
                    if (casadef.casa_version >= '4.0.0'):
                        print "Failed to contact JPL, reverting to using CASA ephemerides to get the J2000 position."
                        rightAscension, declination = planet(fieldName,mjd=times[0]/86400.,useJPL=False)['directionRadians']
                        if (firstCalAtmScan != -1):
                            rightAscensionCalAtm, declinationCalAtm = planet(fieldName,mjd=firstCalAtmScanMeanMJD, useJPL=False)['directionRadians']
                    else:
                        print "Failed to contact JPL, reverting to showing apparent position (from the ms)."
                        rightAscension, declination = getRADecForField(vis, field, forcePositiveRA=True, usemstool=True)
                        apparentCoordinates = True
                        if (firstCalAtmScan != -1):
                            rightAscensionCalAtm, declinationCalAtm = getRADecForField(vis, calAtmField, forcePositiveRA=True, usemstool=True)
            else:
                rightAscension, declination = getRADecForField(vis, field, forcePositiveRA=True, usemstool=True)
                apparentCoordinates = True
                if (firstCalAtmScan != -1):
                    rightAscensionCalAtm, declinationCalAtm = getRADecForField(vis, calAtmField, forcePositiveRA=True, usemstool=True)
        if (len(offSourceRA) > 0):
            separation, deltaRaRadians, deltaDecRadians, ignore = angularSeparationRadians(xOffSource, yOffSource,
                                                                                       rightAscension, declination,
                                                                                       returnComponents=True)
            print "Separation from the off source position in relative coordinates = (%+.1f, %+.1f) arcsec" % (deltaRaRadians*3600*180/np.pi,
                                                                        deltaDecRadians*3600*180/np.pi)
        if (len(calAtmRA) > 0):
            separation, deltaRaRadians, deltaDecRadians, ignore = angularSeparationRadians(xCalAtm, yCalAtm,
                                                                                       rightAscensionCalAtm, declinationCalAtm,
                                                                                       returnComponents=True)
            print "Separation from mean atm calib position in relative coordinates = (%+.1f, %+.1f) arcsec" % (deltaRaRadians*3600*180/np.pi,
                                                                        deltaDecRadians*3600*180/np.pi)
            if (findSubscans):
                separation, deltaRaRadians, deltaDecRadians, ignore = angularSeparationRadians(xCalAtmSky, yCalAtmSky,
                                                                                       rightAscensionCalAtm, declinationCalAtm,
                                                                                       returnComponents=True)
                print "Separation from mean atmcal sky position in relative coordinates = (%+.1f, %+.1f) arcsec" % (deltaRaRadians*3600*180/np.pi,
                                                                        deltaDecRadians*3600*180/np.pi)
                
            
    # convert absolute coordinates to relative arcsec on-the-sky
    for i in range(len(x)):
        if (x[i] < 0):
            print "x is negative"
        if (rightAscension < 0):
            print "rightAscension is negative"
        separation, dx, dy, ignore = angularSeparationRadians(x[i],y[i],rightAscension,declination,True)
        x[i] = dx*180*3600/np.pi
        y[i] = dy*180*3600/np.pi
    totalOffset = (x**2 + y**2)**0.5

    # determine largest dimension of the map
    if (casadef.casa_version < '4.3.0'):
        idx1_ignoreOffPosition, idx1_ignoreOffPositionX, idx1_ignoreOffPositionY = ignoreMostCommonPosition(x,y)
    else:
        onSourceTimes = mymsmd.timesforintent('OBSERVE_TARGET#ON_SOURCE')
        idx1_ignoreOffPosition = np.nonzero(np.in1d(times, onSourceTimes))
        idx1_ignoreOffPositionX = idx1_ignoreOffPosition
        idx1_ignoreOffPositionY = idx1_ignoreOffPosition
        xOnSource = x[idx1_ignoreOffPosition]
        yOnSource = y[idx1_ignoreOffPosition]
        xmad = MAD(xOnSource)
        ymad = MAD(yOnSource)
        xExtremeOffset = np.max(np.abs(xOnSource-np.median(xOnSource)))
        yExtremeOffset = np.max(np.abs(yOnSource-np.median(yOnSource)))
        if (xExtremeOffset > 5*xmad or yExtremeOffset > 5*ymad):
            # Resort to the old method if the identification by time has failed. - 2015-08-07
            # For example, for uid___A002_X9fa4e2_Xca8
            if (debug):
                print "*********** xmad=%f, ymad=%f, xExtreme=%f, yExtreme=%f ********" % (xmad, ymad, xExtremeOffset, yExtremeOffset)
            idx1_ignoreOffPosition, idx1_ignoreOffPositionX, idx1_ignoreOffPositionY = ignoreMostCommonPosition(x,y)
        
    xOnSource = x[idx1_ignoreOffPosition]
    yOnSource = y[idx1_ignoreOffPosition]
    mymsmd.close()
    ymaxloc = yOnSource.argmax()
    yminloc = yOnSource.argmin()
    xmaxloc = xOnSource.argmax()
    xminloc = xOnSource.argmin()
    locs = [xminloc,xmaxloc,yminloc,ymaxloc]
    distances = []
    for l in range(3): # 0; 1; 2
        for i in range(l+1,4): # 1,2,3;  2,3;  3
            distance = ((xOnSource[locs[l]]-xOnSource[locs[i]])**2 + (yOnSource[locs[l]]-yOnSource[locs[i]])**2)**0.5
            distances.append(distance)
    largestDimension = round(np.max(distances))
    
    # Determine the scan direction and sampling
    # It does not seem to be necessary to trim off the OFF position times for this 
    # algorithm to get the right answer
    xSuccessiveDifferences = x[1:] - x[:-1]
    ySuccessiveDifferences = y[1:] - y[:-1]
    successiveDifferences = (xSuccessiveDifferences**2 + ySuccessiveDifferences**2)**0.5

    # Find the axis with most rapidly changing values
    mybeam = primaryBeamArcsec(vis, showEquation=False)
    finestSampling = np.median(np.abs(successiveDifferences))
    xSampling = np.median(np.abs(xSuccessiveDifferences))
    ySampling = np.median(np.abs(ySuccessiveDifferences))
    if debug:
        print "xSuccessiveDifferences[:500]=", xSuccessiveDifferences[:500]
        print "ySuccessiveDifferences[:500]=", ySuccessiveDifferences[:500]
        print "xSampling = ", xSampling
        print "ySampling = ", ySampling
    arbitraryScanAngle = False
    if (np.round(abs(xSampling),2) < np.round(abs(finestSampling),2) and 
        np.round(abs(ySampling),2) < np.round(abs(finestSampling),2)):
        xSampling = finestSampling * medianSamplingIntervalDataTable / medianSamplingIntervalPointingTable
        raScan = False
        arbitraryScanAngle = True
        defaultXSampling = xSampling
        defaultYSampling = mybeam/3.0
    elif (abs(xSampling-finestSampling) < abs(ySampling-finestSampling)):
        print "This raster was scanned along RA."
        raScan = True
    else:
        print "This raster was scanned along Dec."
        raScan = False
        swap = x[:]
        x = y[:]
        y = swap[:]
        swap = xSuccessiveDifferences[:]
        xSuccessiveDifferences = ySuccessiveDifferences[:]
        ySuccessiveDifferences = swap[:]
    print "The finest sampling (in the pointing table) = %.3f arcsec." % (finestSampling)
        
    xSampling = finestSampling
    # correct for the ratio of pointing table data interval (0.048s) to correlator data rate (0.144s)
    print "Scaling sampling from pointing table interval to visibility data table interval: *%f" % (medianSamplingIntervalDataTable / medianSamplingIntervalPointingTable)
    xSampling *= medianSamplingIntervalDataTable / medianSamplingIntervalPointingTable

    # Find where the scan reverses direction
    xReversalPoints = (np.diff(np.sign(np.round(magnification*xSuccessiveDifferences)/magnification)) != 0)*1
    yReversalPoints = (np.diff(np.sign(np.round(magnification*ySuccessiveDifferences)/magnification)) != 0)*1
    reversalPoints = xReversalPoints + yReversalPoints
    indices = np.where(reversalPoints > 0)[0]
    rowChanges = indices[1::2]
    if (debug):
        print "%d xReversalPoints = %s" % (len(np.where(xReversalPoints>0)[0]), str(xReversalPoints))
        print "%d yReversalPoints = %s" % (len(np.where(yReversalPoints>0)[0]), str(yReversalPoints))
        print "Found %d row changes = %s" % (len(rowChanges), str(rowChanges))

    successiveRowDifferences = (xSuccessiveDifferences[rowChanges]**2 + ySuccessiveDifferences[rowChanges]**2)**0.5
    if (debug):
        print "successiveRowDifferences = ", successiveRowDifferences
        print "median = ", np.median(successiveRowDifferences)
    ySampling = np.median(successiveRowDifferences)
    xAtRowChanges = x[rowChanges]
    yAtRowChanges = y[rowChanges]
    if (originalCoordSys.find('AZEL') >= 0):
        roundedRowChanges = list(np.round(yAtRowChanges))
        try:
            ymode = max(set(roundedRowChanges), key=roundedRowChanges.count)
        except:
            ymode = np.median(yAtRowChanges) # old method, did not always work 
        threshold = max(1,abs(ymode*0.005))
        if (debug):
            print "abs(yAtRowChanges-ymode) = ", abs(yAtRowChanges - ymode)
            print "ymode = ", ymode
        keeprows = np.where(abs(yAtRowChanges - ymode) > threshold)[0]
        if (debug):
            print "1) yAtRowChanges = ", str(yAtRowChanges)
        yAtRowChanges = yAtRowChanges[keeprows]
        xAtRowChanges = xAtRowChanges[keeprows]
        if (debug):
            print "Kept %d/%d rows" % (len(yAtRowChanges), len(x[rowChanges]))
            print "2) yAtRowChanges = ", str(yAtRowChanges)
        successiveRowDifferences = abs(yAtRowChanges[1:] - yAtRowChanges[:-1])
        if (debug):
            print "successiveRowDifferences = ", successiveRowDifferences
            print "median = ", np.median(successiveRowDifferences)
        ySampling = np.median(successiveRowDifferences)

    if (fieldName != None):  # comment this block out for Crystal's temporary usage
        if (fieldName.upper() in majorPlanets):
            if (pickFirstRaster == False):
                print "Setting pickFirstRaster=True"
            pickFirstRaster = True

    if (pickFirstRaster):
        # Recalculate the y Sampling over only the first raster
        # Find the range of the raster rows
        maxYoffset = np.max(y[idx1_ignoreOffPositionY])
        minYoffset = np.min(y[idx1_ignoreOffPositionY])
        medianYoffset = np.median(y[idx1_ignoreOffPositionY])
        yrange = maxYoffset-minYoffset
        # idx1 = points within the upper part of the raster
        idx1 = np.where(y > maxYoffset-0.1*yrange)[0]
        idx1 = np.intersect1d(idx1, idx1_ignoreOffPositionY[0])
        idx1 = np.intersect1d(idx1, idx1_ignoreOffPositionX[0])
        if (debug):
            print "y[:400]=", y[:400]
            print "maxYoffset=%f, minYoffset=%f, yrange=%f, maxY-0.1*yrange=%f" % (maxYoffset, minYoffset,yrange,maxYoffset-0.1*yrange)
            print "idx1=upper_portion_of_map=%s" % (str(idx1))
        # idx2 = points within the lower part of raster
        idx2 = np.where(y < minYoffset+0.1*yrange)[0]
        idx2 = np.intersect1d(idx2,idx1_ignoreOffPositionY[0])
        if (debug):
            print "idx2=lower_portion_of_map=%s" % (str(idx2))
        # idx3 = overlap of upper with lower
        idx3 = np.where(idx2 > idx1[0])[0]  # assumes that idx1 is increasing on the first raster row
        if (debug):
            print "idx3=%s" % (str(idx3))
            if (len(idx3) > 0):
                print "idx2[idx3[0]] = ", idx2[idx3[0]]
        myx = x
        myy = y
        mytimes = times
        if (len(idx3) > 0):
            if (debug): print "We found a second raster. Clipping data to first raster."
            if (len(idx2)>idx3[0]):
                endOfFirstRaster = idx2[idx3[0]]
                if (debug): 
                    print "endOfFirstRaster = %d [(%f,%f),(%f,%f),(%f,%f)]" % (endOfFirstRaster,
                        x[endOfFirstRaster-1],y[endOfFirstRaster-1],
                        x[endOfFirstRaster],y[endOfFirstRaster],
                        x[endOfFirstRaster+1],y[endOfFirstRaster+1])
                myx = x[:endOfFirstRaster]
                myy = y[:endOfFirstRaster]
                mytimes = times[:endOfFirstRaster]
        else:
            if (debug): print "There is only one raster in this dataset."
    
        # determine the scan direction and sampling
        xSuccessiveDifferences = myx[1:] - myx[:-1]
        ySuccessiveDifferences = myy[1:] - myy[:-1]
        successiveDifferences = (xSuccessiveDifferences**2 + ySuccessiveDifferences**2)**0.5
    
        # Find where the scan reverses direction
        xReversalPoints = (np.diff(np.sign(np.round(magnification*xSuccessiveDifferences)/magnification)) != 0)*1
        yReversalPoints = (np.diff(np.sign(np.round(magnification*ySuccessiveDifferences)/magnification)) != 0)*1
        reversalPoints = xReversalPoints + yReversalPoints
        indices = np.where(reversalPoints > 0)[0]
        rowChanges = indices[1::2]
        if (debug):
            print "%d xReversalPoints = %s" % (len(np.where(xReversalPoints>0)[0]), str(xReversalPoints))
            print "%d yReversalPoints = %s" % (len(np.where(yReversalPoints>0)[0]), str(yReversalPoints))
            print "Found %d row changes = %s" % (len(rowChanges), str(rowChanges))
        successiveRowDifferences = (xSuccessiveDifferences[rowChanges]**2 + ySuccessiveDifferences[rowChanges]**2)**0.5
        if (debug):
            print "successiveRowDifferences at rowChanges = ", successiveRowDifferences
        myxAtRowChanges = myx[rowChanges]
        myyAtRowChanges = myy[rowChanges]
        if (originalCoordSys.find('AZEL') >= 0):
            roundedRowChanges = list(np.round(myyAtRowChanges))
            try:
                ymode = max(set(roundedRowChanges), key=roundedRowChanges.count)
            except:
                ymode = np.median(myyAtRowChanges)  # old method, did not always work
            if (debug):
                print "ymode = ", ymode
            threshold = max(1,abs(ymode*0.005))
            keeprows = np.where(abs(myyAtRowChanges - ymode) > threshold)[0]
            if (debug):
                print "3) yAtRowChanges = ", str(myyAtRowChanges)
            myyAtRowChanges = np.array(sorted(myyAtRowChanges[keeprows]))  # note the sorted() which is essential
            myxAtRowChanges = myxAtRowChanges[keeprows]
            if (debug):
                print "4) yAtRowChanges = ", str(myyAtRowChanges)
            successiveRowDifferences = abs(myyAtRowChanges[1:] - myyAtRowChanges[:-1])
            if (debug):
                print "successiveRowDifferences = ", successiveRowDifferences
            roundedDiff = list(np.round(successiveRowDifferences))
            x_mode = max(set(roundedDiff), key=roundedDiff.count)
            try:
                ySampling = np.median(successiveRowDifferences[np.where(np.round(successiveRowDifferences) == x_mode)])
            except:
                ySampling = np.median(successiveRowDifferences)  # old method which did not always work
        else:
            roundedDiff = list(np.round(successiveRowDifferences))
            x_mode = max(set(roundedDiff), key=roundedDiff.count)
            try:
                ySampling = np.median(successiveRowDifferences[np.where(np.round(successiveRowDifferences) == x_mode)])
            except:
                ySampling = np.median(successiveRowDifferences)  # old method which did not always work

    if (not raScan):
        swap = xSampling
        xSampling = ySampling
        ySampling = swap
        swap = x
        x = y
        y = swap
        swap = xAtRowChanges
        xAtRowChanges = yAtRowChanges
        yAtRowChanges = swap
    if (showplot or plotfile!=''):
        if (showplot == False):
            pb.ioff()
        mjdsec = getObservationStart(vis)
        obsdateString = mjdToUT(mjdsec/86400.)
        pb.clf()
        adesc = pb.subplot(211)
        lineStyleUpperPlot = '.'
        if (connectDots):
            lineStyle = '-'
        else:
            lineStyle = '.'
        pb.plot_date(pb.date2num(mjdSecondsListToDateTime(times)),totalOffset,
                     'k'+lineStyleUpperPlot,
                     markeredgecolor='k',markerfacecolor='k',markersize=2.0)
        pb.hold(True)
        if (showComponents):
            pb.plot_date(pb.date2num(mjdSecondsListToDateTime(times)),x,'r-')
            pb.plot_date(pb.date2num(mjdSecondsListToDateTime(times)),y,'g-')
        pb.xlabel('Universal Time on %s' % (mjdsecToUT(times[0]).split()[0]))
        pb.ylabel('Angle from origin (arcsec)')
        adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M:%S'))
        setXaxisTimeTicks(adesc, np.min(times), np.max(times))
        y0,y1 = pb.ylim()
        for s in timesforscan.keys():
            b = timesforscan[s]['begin']
            pb.plot_date(pb.date2num(mjdSecondsListToDateTime([b,b])), [y0,y1], 'k-')
            e = timesforscan[s]['end']
            pb.plot_date(pb.date2num(mjdSecondsListToDateTime([e,e])), [y0,y1], 'k--')
            pb.text(pb.date2num(mjdSecondsListToDateTime([0.5*(b+e)]))[0], 0.8*(y1-y0)+y0,
                    'Scan '+str(s),size=8,rotation='vertical')
        if (timerange == None or timerange == ''):
            newStartTime = timesforscan[scansToUse[0]]['begin'] - 20
            newEndTime = timesforscan[scansToUse[-1]]['end'] + 20
            newlimits = pb.date2num(mjdSecondsListToDateTime([newStartTime, newEndTime]))
            pb.xlim(newlimits)
            setXaxisTimeTicks(adesc, newStartTime, newEndTime)
        else:
            pb.xlim(pb.date2num(mjdSecondsListToDateTime(timerange)))
            setXaxisTimeTicks(adesc, timerange[0], timerange[1])
        adesc.xaxis.grid(True,which='major')
        adesc.yaxis.grid(True,which='major')
        if (field != None):
            fieldString = "  (0,0)=%s" % (fieldName)
        else:
            fieldString = ""
        pb.title(os.path.basename(vis) + ', ' + antennanames[antenna] + ', ' + obsdateString +  ', OBS_ID=%d,  %s' % (obsid,fieldString), fontsize=10)
        if (debug):
            print "subplot 211 : xlim=%s, ylim=%s" % (str(pb.xlim()),str(pb.ylim()))
        
        adesc = pb.subplot(212)
        pb.plot(x,y,'b.')
        if connectDots:
            pb.plot(x,y,'b'+linestyle)
        xlimits = pb.xlim()
        ylimits = pb.ylim()
        pb.hold(True)
        pb.plot(xAtRowChanges, yAtRowChanges, 'r.', x[0], y[0], 'ro')
        pb.xlim(xlimits)
        pb.ylim(ylimits)
        if (plotrange != [0,0,0,0]):
            if (plotrange[0] != 0 or plotrange[1] != 0):
                pb.xlim([plotrange[0],plotrange[1]])
            if (plotrange[2] != 0 or plotrange[3] != 0):
                pb.ylim([plotrange[2],plotrange[3]])
        else:
            pb.xlim([np.max(x), np.min(x)])
            pb.axis('equal')
        for i in range(np.min([labelFirstNSamples, len(x)/labelIncrement])):
            sample = i*labelIncrement
            pb.text(x[sample],y[sample],str(sample),size=8)
        if (coordSys.find('AZEL') >= 0):
            azimString = qa.formxxx('%frad'%(rightAscension), format='deg', prec=5)
            elevString = qa.formxxx('%frad'%(declination), format='deg', prec=5)
            pb.xlabel('Azimuth offset (arcsec) from %s deg' % azimString)
            pb.ylabel('Elevation offset (arcsec) from %s deg' % elevString)
            pb.xlim([np.min(x), np.max(x)])
        else:
            raString = qa.formxxx('%frad'%(rightAscension), format='hms', prec=2)
            decString = qa.formxxx('%frad'%(declination), format='dms', prec=0).replace('.',':',2).replace('-0','-').replace('+0','+')
            if (apparentCoordinates):
                basis = '(apparent)'
            else:
                basis = '(J2000)'
            pb.ylabel('Dec offset (arcsec) from %s' % (decString), size=10)
            if (convert):
                pb.xlabel('Right Ascension offset (arcsec) from %s %s' % (raString,basis))
        adesc.xaxis.grid(True,which='major')
        adesc.yaxis.grid(True,which='major')
        pb.plot([0.025], [0.96], 'ro', transform=adesc.transAxes)
        pb.plot([0.025], [0.91], 'r.', transform=adesc.transAxes)
        pb.text(0.05, 0.93, 'OFF position', transform=adesc.transAxes)
        pb.text(0.05, 0.86, 'end of row', transform=adesc.transAxes)
        if (debug):
            print "subplot 212 : xlim=%s, ylim=%s" % (str(pb.xlim()),str(pb.ylim()))
        if (showplot):
            pb.draw()
        if (plotfile != ''):
            if (plotfile == True):
                plotfile = vis+'.obsid%d.sampling.png' % (obsid)
            if (os.path.exists(plotfile)):
                if (os.access(plotfile, os.W_OK) == False):
                    plotfile = '/tmp/' + os.path.basename(plotfile)
            else:
                mydir = os.path.dirname(plotfile)
                if (mydir == ''):
                    mydir = os.getcwd()
                if (os.access(mydir, os.W_OK) == False):
                    plotfile = '/tmp/' + os.path.basename(plotfile)
            pb.savefig(plotfile)
            print "Saved plot in %s" % (plotfile)
        if (showplot == False):
            pb.ion()
    if (convert==False and coordSys.find('AZEL')>=0):
        print "To determine the reference position, re-run with convert=True"
    # end of plotTPSampling
    
