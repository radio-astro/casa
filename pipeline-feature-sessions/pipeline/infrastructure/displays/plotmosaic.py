import os
import math
import matplotlib
import numpy as np
import pylab as pb
from casac import casac

# Constants that are sometimes useful.  Warning these are cgs, we might want to change them
c_mks=2.99792458e8

# Ephmeris codes
JPL_HORIZONS_ID = {'ALMA': '-7',
                   'VLA': '-5',
                   'GBT': '-9',
                   'MAUNAKEA': '-80',
                   'OVRO': '-81',
                   'geocentric': '500'
}

def plotMosaic(vis='',sourceid='',figfile='', coord='relative', skipsource=-1,
               doplot=True, help=False):
  """
  Produce a plot of the pointings with the primary beam FWHM and field names.
  """
  return plotmosaic(vis,sourceid,figfile,coord,skipsource,doplot,help)


def plotmosaic(vis='',sourceid='',figfile='', coord='relative', skipsource=-1,
               doplot=True, help=False):
  """
  Produce a plot of the pointings with the primary beam FWHM and field names.
  """

  if (help):
      print "Usage: plotmosaic(vis, sourceid='', figfile='', coord='relative',"
      print "                  skipsource=-1, doplot=True, help=False)"
      print "  The sourceid may be either an integer, integer string, or the string name"
      print "     of the source but it cannot be a list."
      print "  If doplot=False, then the central field ID is returned as an integer. "
      print "     Otherwise, a list is returned: "
      print "         [central field,  maxRA, minRA, minDec, maxDec]"
      print "     where the angles are in units of arcsec relative to the center."
      print "  If coord='absolute', then nothing is returned."
      return

  # open the ms table
  if (coord.find('abs') < 0 and coord.find('rel') < 0):
      print "Invalid option for coord, must be either 'rel'ative or 'abs'olute."
      return

  mytb = casac.table()
  try:
      fieldTable = vis+'/FIELD'
      mytb.open(fieldTable)
  except:
      print "Could not open table = %s" % fieldTable 
      return

  delayDir = mytb.getcol('DELAY_DIR')
  sourceID = mytb.getcol('SOURCE_ID')
  name = mytb.getcol('NAME')
  if (type(sourceid) == str):
      try:
          sourceid = int(sourceid)
      except:
          # need to convert name to id
          matches=np.where(name==sourceid)[0]
          srcs=np.unique(sourceID[matches])
          nsrcs=len(srcs)
          if (nsrcs>1):
              print "More than one source ID matches this name: ",sourceID
              print "Try again using one of these."
              return
          elif (nsrcs==0):
              if (sourceid != ''):
                  print "No sources match this name = %s" % sourceid
                  print "Available sources = ",np.unique(name)
                  return
              else:
                  print "No source specified in the second argument, so plotting all sources."
          else:
              sourceid = srcs[0]

  sourcename = name[sourceid]
  fields = np.array(range(len(sourceID)))
  if (sourceid != ''):
      matches = np.where(sourceID == int(sourceid))[0]
      fields = fields[matches]
      matches = np.where(fields != skipsource)[0]
      fields = fields[matches]
      if (len(fields) < 1):
          print "No fields contain source ID = ", sourceid
          return
      print "Field IDs with matching source ID = ", fields
  name = mytb.getcol('NAME')
  mytb.close()

  try:
      antennaTable = vis+'/ANTENNA'
      mytb.open(antennaTable)
  except:
      print "Could not open table = %s" % antennaTable 
      return

  dishDiameter = np.unique(mytb.getcol('DISH_DIAMETER'))
  mytb.close()

  try:
      spwTable = vis+'/SPECTRAL_WINDOW'
      mytb.open(spwTable)
      num_chan = mytb.getcol('NUM_CHAN')
      refFreqs = mytb.getcol('REF_FREQUENCY')
      mytb.close()
  except:
      print "Could not open table = %s" % antennaTable 
      print "Will not print primary beam circles"
      titleString = vis.split('/')[-1]
      dishDiameter =  [0]
  [latitude,longitude,obs] = getObservatoryLatLong('ALMA') 
  print "Got longitude = %.3f deg" % (longitude)

  if (3840 in num_chan):
      matches = np.where(num_chan == 3840)[0]
  else:
      matches = np.where(num_chan > 4)[0]  # this kills the WVR and channel averaged data
   
  meanRefFreq = np.mean(refFreqs[matches])
  print "Mean frequency = %f GHz" % (meanRefFreq*1e-9)
  lambdaMeters = c_mks / meanRefFreq
  ra = delayDir[0,:][0]*12/math.pi
  dec = delayDir[1,:][0]*180/math.pi
  ra *= 15
  raAverageDegrees = np.mean(ra[fields])
  decAverageDegrees = np.mean(dec[fields])
  #cosdec = 1.0/cos(decAverageDegrees*np.pi/180)
  cosdec = 1.0/math.cos(decAverageDegrees*np.pi/180)

  # Here we scale by cos(dec) to make then pointing pattern in angle on sky
  #raRelativeArcsec = 3600*(ra - raAverageDegrees)*cos(decAverageDegrees*math.pi/180.)
  raRelativeArcsec = 3600*(ra - raAverageDegrees)*math.cos(decAverageDegrees*math.pi/180.)
  decRelativeArcsec = 3600*(dec - decAverageDegrees)

  markersize = 4
  #print "Found %d pointings" % (shape(ra)[0])
  print "Found %d pointings" % (np.shape(ra)[0])
  [centralField,smallestSeparation] = findNearestField(ra[fields],dec[fields],
                                          raAverageDegrees, decAverageDegrees)
  # This next step is crucial, as it converts from the field number 
  # determined from a subset list back to the full list.
  centralField = fields[centralField]
  
  print "Field %d is closest to the center of the area covered (%.1f arcsec away)." % (centralField,smallestSeparation*3600)
  if (doplot==False):
      return(centralField)

  pb.clf()
  desc = pb.subplot(111)
  if (coord.find('abs')>=0):
    raunit = 'deg'  # nothing else is supported (yet)
    desc = pb.subplot(111,aspect=cosdec)
    # SHOW ABSOLUTE COORDINATES
    pb.plot(ra[fields],dec[fields],"k+",markersize=markersize)
    for j in dishDiameter:
      for i in range(len(ra)):
          if (i in fields):
              if (j > 0):
                  arcsec = 0.5*primaryBeamArcsec(wavelength=lambdaMeters*1000,diameter=j)
                  radius = arcsec/3600.0
                  cir = matplotlib.patches.Ellipse((ra[i], dec[i]), width=2*radius*cosdec,
                                                   height=2*radius, facecolor='none', edgecolor='b',
                                                   linestyle='dotted')
                  pb.gca().add_patch(cir)
      titleString = vis.split('/')[-1]+', %s, average freq. = %.1f GHz, beam = %.1f"'%(sourcename,meanRefFreq*1e-9,2*arcsec)
    resizeFonts(desc, 10)
    if (raunit.find('deg') >= 0):
        pb.xlabel('Right Ascension (deg)')
    else:
        pb.xlabel('Right Ascension (hour)')
    pb.ylabel('Declination (deg)')
    raRange = np.max(ra[fields])-np.min(ra[fields])
    decRange = np.max(dec[fields])-np.min(dec[fields])
    x0 = np.max(ra[fields]) + 1.2*radius*cosdec
    x1 = np.min(ra[fields]) - 1.2*radius*cosdec
    y1 = np.max(dec[fields]) + 1.2*radius
    y0 = np.min(dec[fields]) - 1.2*radius
    pb.xlim([x0,x1])
    pb.ylim([y0,y1])
    pb.title(titleString,size=10)
    for i in range(len(ra)):
      if (i in fields):
        pb.text(ra[i]-0.02*raRange, dec[i]+0.02*decRange, str(i),fontsize=12, color='k')
  elif (coord.find('rel')>=0):
    # SHOW RELATIVE COORDINATES
    pb.plot(raRelativeArcsec[fields], decRelativeArcsec[fields], 'k+', markersize=markersize)
    for j in dishDiameter:
        for i in range(len(ra)):
            if (i in fields):
                if (j > 0):
                    arcsec = 0.5*1.18*lambdaMeters*3600*180/j/math.pi
                    radius = arcsec
                    cir = pb.Circle((raRelativeArcsec[i], decRelativeArcsec[i]),
                                    radius=radius, facecolor='none', edgecolor='b', linestyle='dotted')
                    pb.gca().add_patch(cir)
    titleString = vis.split('/')[-1]+', %s, average freq. = %.1f GHz, beam = %.1f"'%(sourcename,meanRefFreq*1e-9,2*arcsec)
    resizeFonts(desc, 10)
    pb.xlabel('Right ascension offset (arcsec)')
    pb.ylabel('Declination offset (arcsec)')
    pb.title(titleString,size=10)
    raRange = np.max(raRelativeArcsec[fields])-np.min(raRelativeArcsec[fields])
    decRange = np.max(decRelativeArcsec[fields])-np.min(decRelativeArcsec[fields])
    for i in range(len(ra)):
      if (i in fields):
        pb.text(raRelativeArcsec[i]-0.02*raRange, decRelativeArcsec[i]+0.02*decRange, str(i),fontsize=12, color='k')
    x0 = np.max(raRelativeArcsec[fields]) + 1.2*radius # 0.25*raRange
    x1 = np.min(raRelativeArcsec[fields]) - 1.2*radius # - 0.25*raRange
    y1 = np.max(decRelativeArcsec[fields]) + 1.2*radius # 0.25*decRange
    y0 = np.min(decRelativeArcsec[fields]) - 1.2*radius # 0.25*decRange
    mosaicInfo = []
    mosaicInfo.append(centralField)
    mosaicInfo.append(np.max(raRelativeArcsec[fields]) + 2*radius)
    mosaicInfo.append(np.min(raRelativeArcsec[fields]) - 2*radius)
    mosaicInfo.append(np.max(decRelativeArcsec[fields]) + 2*radius)
    mosaicInfo.append(np.min(decRelativeArcsec[fields]) - 2*radius)
    pb.xlim(x0,x1)
    pb.ylim(y0,y1)
    pb.axis('equal')
  else:
    print "Invalid option for coord, must be either 'rel'ative or 'abs'olute."
    return

  #yFormatter = ScalarFormatter(useOffset=False)
  yFormatter = matplotlib.ticker.ScalarFormatter(useOffset=False)
  desc.yaxis.set_major_formatter(yFormatter)
  desc.xaxis.set_major_formatter(yFormatter)
  desc.xaxis.grid(True,which='major')
  desc.yaxis.grid(True,which='major')
  pb.draw()
  autoFigureName = "%s.pointings.%s.png" % (vis,coord)

  if (figfile==True):
      try:
        pb.savefig(autoFigureName)
        print "Wrote file = %s" % (autoFigureName)
      except:
        print "WARNING:  Could not save plot file.  Do you have write permission here?"
  elif (len(figfile) > 0):
      try:
        pb.savefig(figfile)
        print "Wrote file = %s" % (figfile)
      except:
        print "WARNING:  Could not save plot file.  Do you have write permission here?"
  else:
        print "To save a plot, re-run with either:"
        print "  plotmosaic('%s',figfile=True) to produce the automatic name=%s" % (vis,autoFigureName)
        print "  plotmosaic('%s',figfile='myname.png')" % (vis)
  if (coord.find('rel')>=0):
      return mosaicInfo
  else:
      return

def listAvailableObservatories():
        mytb = casac.table()
        repotable=os.getenv("CASAPATH").split()[0]+"/data/geodetic/Observatories"
        try:
            mytb.open(repotable)
            Name = mytb.getcol('Name')
            ns = ''
            for N in Name:
                ns += N + "  "
                if (len(ns) > 70):
                    print ns
                    ns = ''
            mytb.close()
        except:
            print "Could not open table = %s" % (repotable)

def getObservatoryLatLong(observatory='', help=False):
     """
     Opens the casa table of known observatories and returns the latitude and longitude
     in degrees for the specified observatory name string.
     """
     if (help):
        print "Opens the casa table of known observatories and returns the latitude"
        print "and longitude in degrees for the specified observatory (default=ALMA)."
        print "Usage: getObservatoryLatLong(observatory='ALMA')"
        print "Usage: getObservatoryLatLong(observatory=-7)"
        listAvailableObservatories()
        return

     mytb = casac.table()
     repotable=os.getenv("CASAPATH").split()[0]+"/data/geodetic/Observatories"
     try:
        mytb.open(repotable)
     except:
        print "Could not open table = %s, returning ALMA coordinates instead" % (repotable)
        longitude = -67.7549  # ALMA
        latitude = -23.0229   # ALMA
        observatory = 'ALMA'
        return([latitude,longitude,observatory])

     if (type(observatory) == 'int' or observatory in JPL_HORIZONS_ID.values()):
         if (str(observatory) in JPL_HORIZONS_ID.values()):
             observatory = JPL_HORIZONS_ID.keys()[JPL_HORIZONS_ID.values().index(str(observatory))]
         else:
            print "Did not recognize observatory='%s', using ALMA instead." % (observatory)
            observatory = 'ALMA'
             
     Name = mytb.getcol('Name')
     matches = np.where(np.array(Name)==observatory)
     if (len(matches) < 1 and str(observatory).find('500') < 0):
            print "Names = ", Name
            print "Did not find observatory='%s', using ALMA instead." % (observatory)
            for n in Name:
                if (n.find(observatory) >= 0):
                    print "Partial match: ", n
            observatory = 'ALMA'
            longitude = -67.7549  # ALMA
            latitude = -23.0229   # ALMA
     elif (str(observatory).find('500') >= 0 or
           str(observatory).lower().find('geocentric') >= 0):
         observatory = 'Geocentric'
         longitude = 0
         latitude = 0
     else:
         longitude = mytb.getcol('Long')[matches[0]]
         latitude = mytb.getcol('Lat')[matches[0]]
     mytb.close()
         
     return([latitude,longitude,observatory])

def findNearestField(ra, dec, raAverageDegrees, decAverageDegrees):
    """
    Finds the field in a list that is nearest to the specified average position.
    ra and dec must be equal-sized lists.
    raAvergeDegrees and decAverageDegrees must be scalar values.
    """

    nearestField = -1
    smallestSeparation = 1e15
    for i in range(len(ra)):
        separation = angularSeparation(ra[i],dec[i],raAverageDegrees,decAverageDegrees)
        if (separation < smallestSeparation):
            smallestSeparation = separation
            nearestField = i
    return([nearestField,smallestSeparation])


def angularSeparation(ra0, dec0, ra1, dec1):
  """
  Computes the great circle angle between two celestial coordinates.
  using the Vincenty formula (from wikipedia) which is correct for all
  angles, as long as you use atan2() to handle a zero denominator.  
  ra,dec must be given in degrees, as is the output.
  It also works for the az,el coordinate system.
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
  return(angle)


def primaryBeamArcsec(vis='', spw='', frequency='',wavelength='', diameter=12.0, verbose=False, help=False):
  """
  Implements the formula: 1.18 * lambda / D.  Specify one of the following combinations:
    1) vis and spw (uses median dish diameter)
    2) frequency in GHz (assumes 12m)"
    3) wavelength in mm (assumes 12m)"
    4) frequency in GHz and diameter (m)
    5) wavelength in mm and diameter (m)
  """
  
  if (help == True):
      print "Usage: primaryBeamArcsec(vis='', spw='', frequency='',wavelength='', diameter=12.0, verbose=False, help=False)"
      print "Implements the formula: 1.18 * lambda / D.  Specify one of the following combinations:"
      print "     1) vis and spw (uses median dish diameter)"
      print "     2) frequency in GHz (assumes 12m)"
      print "     3) wavelength in mm (assumes 12m)"
      print "     4) frequency in GHz and diameter"
      print "     5) wavelength in mm and diameter"
      return (0)

  mytb = casac.table()
  if (vis!='' and spw!=''):

      try:
          antennaTable = vis+'/ANTENNA'
          mytb.open(antennaTable)
      except:
          print "Could not open table = %s" % antennaTable 
          return(0)
      diameter = np.median(np.unique(mytb.getcol('DISH_DIAMETER')))
      mytb.close()

      try:
          spwTable = vis+'/SPECTRAL_WINDOW'
          mytb.open(spwTable)
          num_chan = mytb.getcol('NUM_CHAN')
          refFreqs = mytb.getcol('REF_FREQUENCY')
          mytb.close()
      except:
          print "Could not open table = %s" % antennaTable 
          return(0)

      frequencyHz = refFreqs[spw]
      frequency = frequencyHz*1e-9
      if (verbose):
          print "Found frequency = %f GHz and dish diameter = %.1fm" % (frequency,diameter)

  else:
      if (frequency != '' and wavelength != ''):
          print "You must specify either frequency or wavelength, not both!"
          return(0)

  if (frequency != ''):
      lambdaMeters = c_mks / (frequency * 1e9)
  elif (wavelength != ''):
      lambdaMeters = wavelength*0.001
  else:
      print "You must specify either frequency (in GHz) or wavelength (in mm)"
      return(0)

  return(1.18*lambdaMeters*3600*180/diameter/math.pi)

def resizeFonts(adesc, fontsize):
    """
    """
    yFormat = matplotlib.ticker.ScalarFormatter(useOffset=False)
    adesc.yaxis.set_major_formatter(yFormat)
    adesc.xaxis.set_major_formatter(yFormat)
    pb.setp(adesc.get_xticklabels(), fontsize=fontsize)
    pb.setp(adesc.get_yticklabels(), fontsize=fontsize)

