import os
import math
import numpy as np
import scipy as sp

import pipeline.infrastructure.casatools as casatools

# Physical constants
#
# Speed of light in MKS system
c_mks = 2.99792458e8

def mosaicOverlapFactorMS(ms, source, spw, diameter, intent='TARGET',
    fwhmfactor=1.13, taper=10.0, obscuration=0.75):

    """
    This routine computes the maximum sensitivity increase factor for a 
    mosaic. It handles an arbitrary mosaic pattern. It does not account for
    flagging or different observing depths at different pointings. Defaulted
    parameters take values appropriate for ALMA. The use2007formula parameter
    in the primaryBeamArsec and gaussianBeamResponse methods default to True.
    This parameter could be pulled up to the interface of this routine if
    required.

    Inputs:

             ms: The ms object from the pipeline context
         source: The target source id or name 
            spw: Determine the frequency based on this spw or list of spws.
       diameter: The effective antenna diameter in meters
         intent: The target source intent
     fwhmfactor: Factor passed to gaussianBeamResponse and primaryBeamArcsec
          taper: The taper in DB which is used if fwhmfactor = None
    obscuration: The diameter of the central obscuration in meters

    Returns:

    A single floating point value for the maximum sensitivity increase factor,
    which is typically for the center of the mosaic but not necessarily so.  It is
    meant to be applied to a theoretical rms computed based on the total time
    spent on only one of the pointings of the mosaic.

    """


    # Find the fields associated with the selected mosaic source
    msource = []
    if source.isdigit():
        msource = [s for s in ms.sources if s.id == source] 
    else:
        msource = [s for s in ms.sources if s.name == source] 
    if not msource:
        return None
    fields = [f.id for f in msource[0].fields]

    # Get all the fields for the selected intent
    science_fields = [f.id for f in ms.get_fields(intent=intent)]

    # Compute the intersection of the two field lists
    #    Replace the specified fields with the intersection
    #    if the length of the intersection list is greater than 0
    myfields = np.intersect1d(science_fields, fields)
    if len(myfields) > 0:
         fields = myfields

    # Make a list of the selected spw ids
    if type(spw) == str:
        spws = [int(i) for i in spw.split(',')]
    elif type(spw) != list:
        # i.e. integer
        spws = [spw]
    else:
        spws = spw

    # Find all the spw ids associated with the
    # specified intent
    science_spws = [s.id for s in ms.get_spectral_windows() if intent in s.intents] 

    # Compute the intersection of the two
    #    Replace the specified spws with the intersection if
    #    the length of the intersection list is greater than 0
    myspws = np.intersect1d(science_spws, spws)
    if len(myspws) > 0:
        spws = myspws

    # Compute the mean frequency for the spws in Hz
    frequencies = []
    for spwid in spws: 
        frequency = float(ms.get_spectral_window(spwid).centre_frequency.value)
        frequencies.append(frequency)
    frequency = np.mean(frequencies)

    # Compute the primary beam size in arc seconds.
    primaryBeam = primaryBeamArcsec (frequency, diameter,
        taper=taper, obscuration=obscuration, fwhmfactor=fwhmfactor)

    # Compute the radius at first null
    #    Approximately correct for Bessel function
    radiusFirstNull = primaryBeam

    # Initialize
    timeOnSource = np.zeros(len(fields))
    separations = []
    separationDict = {}

    # Loop over the fields
    for i, f in enumerate(fields):
        for field in fields:

            # Compute the separation between each field entering it into
            # the separation dictionary as appropriate.
            if (f != field):
                # Fill the upper half of a matrix to hold separations
                # in order to avoid calculating each field separation twice.
                smallerField = np.min([f,field])
                largerField = np.max([f,field])
                if (smallerField not in separationDict.keys()):
                    separationDict[smallerField] = {}
                if (largerField not in separationDict[smallerField]):
                    df = ms.get_fields(field_id=f)[0].mdirection
                    dfield = ms.get_fields(field_id=field)[0].mdirection
                    separation = np.degrees(angularSeparationOfDirections( \
                        df, dfield)) * 3600.0
                    separationDict[smallerField][largerField] = separation
                else:
                    separation = separationDict[smallerField][largerField]
                separations.append(separation)
            else:
                separation = 0

            # Compute the fractional flux factors per field
            if (separation < radiusFirstNull):
                # Truncate the Gaussian at the first null of the Bessel
                # function
                fractionalFlux = gaussianBeamResponse(separation,
                    frequency, diameter, taper=taper, obscuration=obscuration,
                    fwhmfactor=fwhmfactor)
                timeOnSource[i] += fractionalFlux ** 2

    # Compute and return the maximum response
    maxresponse = np.max(timeOnSource) ** 0.5
    return maxresponse

def mosaicOverlapFactorVIS(vis, source, spw, diameter, fwhmfactor=1.13,
    taper=10.0, obscuration=0.75, intent='OBSERVE_TARGET#ON_SOURCE'):

    """
    This routine computes the maximum sensitivity increase factor for an ALMA
    mosaic. It handles an arbitrary mosaic pattern. It does not account for
    flagging or different observing depths at different pointings.

    Inputs:

           vis: The name of the measurement set
        source: The source id or name 
           spw: determine the frequency based on this spw or list of spws.
                Defaults to all with the specified intent.
      diameter: The effective outer antenna diameter in meters
    fwhmfactor: Factor passed to gaussianBeamResponse and primaryBeamArcsec
         taper: The taper in DB
   obscuration: The diameter of the central obscuration in meters
        intent: The CASA data selection intent

    Returns:

      A single floating point value for the maximum sensitivity increase factor,
      which is typically the center of the mosaic but not necessarily so.  It is
      meant to be applied to a theoretical rms computed based on the total time
      spent on only one of the pointings of the mosaic.
    """


    # Get the science fields and frequencies
    with casatools.MSMDReader(vis) as msmd:

        # Get the specified science fields
        if (source.isdigit()):
            fields = msmd.fieldsforsource(source)
        else:
            fields = msmd.fieldsforname(source)

        # Get all science fields that match the given intent
        science_fields = msmd.fieldsforintent(intent)

        # Compute the intersection of the two
        #    Replace the specified fields with the intersection
        myfields = np.intersect1d(science_fields, fields)
        if len(myfields) > 0:
            fields = myfields

        # Get the specified science spws
        # the mean
        if type(spw) == str:
            spws = [int(i) for i in spw.split(',')]
        elif type(spw) != list:
            # i.e. integer
            spws = [spw]
        else:
            spws = spw

        # Get all science spws that match the given intent
        science_spws = msmd.spwsforintent(intent)

        # Compute the intersection of the two
        #    Replace the specified spws with the intersection
        myspws = np.intersect1d(science_spws, spws)
        if len(myspws) > 0:
            spws = myspws

        # Compute the mean frequency in Hz
        frequencies = []
        for spwid in spws: 
            frequencies.append(msmd.meanfreq(spwid))
        frequency = np.mean(frequencies)

    # Compute the primary beam and radius at first null
    #    Note that in this call fwhmfactor is set 
    primaryBeam = primaryBeamArcsec (frequency, diameter,
        taper=taper, obscuration=obscuration, fwhmfactor=fwhmfactor)

    # Approximately correct for Bessel function
    #    Same as above
    radiusFirstNull = primaryBeam

    # Initialize
    timeOnSource = np.zeros(len(fields))
    separations = []
    separationDict = {}

    # Loop over the fields examining the overlaps
    with casatools.MSMDReader(vis) as msmd:
        for i, f in enumerate(fields):
            for field in fields:
                if (f != field):
                    # Fill the upper half of a matrix to hold separations
                    # in order to avoid calculating each field separation twice.
                    smallerField = np.min([f,field])
                    largerField = np.max([f,field])
                    if (smallerField not in separationDict.keys()):
                        separationDict[smallerField] = {}
                    if (largerField not in separationDict[smallerField]):
                        separation = np.degrees(angularSeparationOfDirections( \
                            msmd.phasecenter(f), msmd.phasecenter(field))) * \
                            3600.0
                        separationDict[smallerField][largerField] = separation
                    else:
                        separation = separationDict[smallerField][largerField]
                    separations.append(separation)
                else:
                    separation = 0
                if (separation < radiusFirstNull):
                    # Truncate the Gaussian at the first null of the Bessel
                    # function
                    fractionalFlux = gaussianBeamResponse(separation,
                        frequency, diameter, taper=taper,
                        obscuration=obscuration, fwhmfactor=fwhmfactor)
                    timeOnSource[i] += fractionalFlux ** 2

    # Compute and return the maximum response
    maxresponse = np.max(timeOnSource) ** 0.5
    return maxresponse


def primaryBeamArcsec (frequency, diameter, taper=10.0, obscuration=0.75,
    fwhmfactor=None, use2007formula=True):

    """
    Implements the Baars formula: b * lambda / D.
        if use2007formula==False, use the formula from ALMA Memo 456
        if use2007formula==True, use the formula from Baars 2007 book
    In either case, the taper value is expected to be entered as positive.
    Note: if a negative value is entered, it is converted to positive.

    The effect of the central obstruction on the pattern is also accounted for
    by using a spline fit to Table 10.1 of Schroeder's Astronomical Optics.

    The default values correspond to our best knowledge of the ALMA 12m antennas.

    Inputs
         frequency: The frequency in Hz
          diameter: The outer diameter of the dish in meters
             taper: The taper in DB
       obscuration: The diameter of the central obscuration in meters
        fwhmfactor: The fwhm factor, if given ignore the taper
    use2007formula: If True use Baars formula
    """

    # Formulas do not work in this case
    if obscuration > 0.4 * diameter:
        return None

    # Compute the taper
    if not fwhmfactor:
        mytaper = taper
    else:
        mytaper = effectiveTaper (fwhmfactor, diameter,
            obscuration=obscuration, use2007formula=use2007formula) 
        if not mytaper:
            return None
    if mytaper < 0.0:
        mytaper = abs (mytaper)

    # Compute the primary beam factor. 
    lambdaMeters = c_mks / frequency
    bfactor = baarsTaperFactor(mytaper, use2007formula=use2007formula) * \
        centralObstructionFactor(diameter, obscuration=obscuration)
    primaryBeam = bfactor * lambdaMeters * 3600 * 180 / (diameter * math.pi)

    return primaryBeam


def gaussianBeamResponse (radius, frequency, diameter, taper=10.0,
    obscuration=0.75, fwhmfactor=None, use2007formula=True):

    """
    Computes the gain at the specified radial offset from the center
    of a Gaussian beam.

    Inputs:
            radius: The radius in arcseconds
         frequency: The frequency in Hz
          diameter: The outer diameter of the dish in m
             taper: The taper in DB 
       obscuration: Diameter of the central obscuration in meters
        fwhmfactor: The fwhm factor, if given ignore the taper
    use2007formula: If True use Baars formula
    """

    fwhm = primaryBeamArcsec(frequency, diameter, taper=taper,
        obscuration=obscuration, fwhmfactor=fwhmfactor,
        use2007formula=use2007formula)
    sigma = fwhm / 2.3548
    gain = np.exp(-((radius / sigma) **2) * 0.5)
    return gain

def baarsTaperFactor(taper_dB, use2007formula=True):

    """
    Converts a taper in dB to the constant X in the formula
    FWHM=X*lambda/D for the parabolic illumination pattern.
    We assume that taper_dB comes in as a positive value.
    use2007formula:  False --> use Equation 18 from ALMA Memo 456.
                     True --> use Equation 4.13 from Baars 2007 book

    Inputs
             taper: The taper in DB 
    use2007formula: If True use Baars formula

    """

    tau = 10 ** (-0.05 * taper_dB)
    if (use2007formula):
        return (1.269 - 0.566*tau + 0.534*(tau**2) - 0.208*(tau**3))
    else:
        return (1.243 - 0.343*tau + 0.12*(tau**2))

def centralObstructionFactor(diameter, obscuration=0.75):

    """
    Computes the scale factor of an Airy pattern as a function of the
    central obscuration, using Table 10.1 of Schroeder's "Astronomical Optics".

    Inputs
       diameter: The outer dish diameter in meters
    obscuration: The diameter of the central obscuration in meters
    """

    epsilon = obscuration / diameter
    myspline = sp.interpolate.UnivariateSpline([0,0.1,0.2,0.33,0.4],
        [1.22,1.205,1.167,1.098,1.058], s=0)
    factor = myspline(epsilon) / 1.22

    return factor

def effectiveTaper(fwhmFactor, diameter, obscuration=0.75,
                   use2007formula=True):
    """
    The inverse of Baars formula multiplied by the central obstruction
    factor. Converts an observed value of the constant X in the formula
    FWHM = X * lambda / D into a taper in dB which must be a positive
    value.

    if use2007formula == False, use Equation 18 from ALMA Memo 456
    if use2007formula == True, use Equation 4.13 from Baars 2007 book

    Inputs
        fwhmfactor: The fwhm factor, if given ignore the taper
          diameter: The outer dish diameter in meters
       obscuration: The diameter of the central obscuration in meters
    use2007formula: If True use Baars formula
    """

    cOF = centralObstructionFactor (diameter, obscuration=obscuration)
    if (fwhmFactor < 1.02 or fwhmFactor > 1.22):
        return None
    if (baarsTaperFactor(10, use2007formula=use2007formula) * cOF < fwhmFactor):
        increment = 0.01
        for taper_dB in np.arange(10, 10 + increment * 1000, increment):
            if (baarsTaperFactor(taper_dB, use2007formula=use2007formula) * \
                cOF - fwhmFactor > 0):
                break
    else:
        increment = -0.01
        for taper_dB in np.arange(10, 10 + increment * 1000, increment):
            if (baarsTaperFactor(taper_dB, use2007formula=use2007formula) * \
                cOF - fwhmFactor < 0):
                break

    return taper_dB

def angularSeparationOfDirections(dir1, dir2, returnComponents=False):

    """
    Accepts two direction dictionaries and returns the separation in radians.
    It computes great circle angle using the Vincenty formula.
    """

    rad = angularSeparationRadians (dir1['m0']['value'], dir1['m1']['value'],
        dir2['m0']['value'], dir2['m1']['value'],returnComponents)

    return rad


def angularSeparationRadians(ra0, dec0, ra1, dec1, returnComponents=False):

    """
    Computes the great circle angle between two celestial coordinates.
    using the Vincenty formula from wikipedia which is correct for all
    angles, as long as you use atan2() to handle a zero denominator.

        See  http://en.wikipedia.org/wiki/Great_circle_distance

    Input and output are in radians. This routine also works for the az, el
    coordinate system.

    returnComponents=True will return: [separation, raSeparation,
                                        decSeparation, raSeparationCosDec]
    """

    result = angularSeparation(ra0 * 180 / math.pi, dec0 * 180 / math.pi,
        ra1 * 180 / math.pi, dec1 * 180 / math.pi, returnComponents)
    if (returnComponents):
        return (np.array(result) * math.pi / 180.)
    else:
        return (result * math.pi / 180.)

def angularSeparation(ra0, dec0, ra1, dec1, returnComponents=False):

    """
    Computes the great circle angle between two celestial coordinates.
    using the Vincenty formula from wikipedia which is correct for all
    angles, as long as atan2() is used to handle a zero denominator.

        See  http://en.wikipedia.org/wiki/Great_circle_distance

    ras and decs must be given in degrees. The output is returned
    in degrees.

    This routine also works for the az, el coordinate system.

    Component separations are field_0 minus field_1.
    See also angularSeparationRadians()
    returnComponents: if True, then also compute angular separation in both
         coordinates and the position angle of the separation vector on the sky
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
#        positionAngle = -math.atan2(decdegrees*math.pi/180., radegreesCosDec*math.pi/180.)*180/math.pi
        retval = angle,radegrees,decdegrees, radegreesCosDec
    else:
        retval = angle

    return(retval)

