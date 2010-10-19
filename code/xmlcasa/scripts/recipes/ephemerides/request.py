import os
import re
import smtplib
import socket
import time
from email.mime.text import MIMEText

"""
Utilities for having JPL-Horizons ephemerides mailed to you (or your enemies).
See JPL_ephem_reader.py for doing something with them.

Examples:

import recipes.ephemerides.request as jplreq

# I recommend you not ask for more than ~18 months of anything with date_incr
# ~ 1h, because the result would be split into multiple emails which you would
# have to stitch together.

for thing in jplreq.asteroids.keys() + jplreq.planets_and_moons.keys():
    jplreq.request_from_JPL(thing, '2012-12-31')

# A trick to avoid fast moving objects:
for thing in jplreq.asteroids.keys() + jplreq.planets_and_moons.keys():
    if thing not in jplreq.default_date_incrs:
        jplreq.request_from_JPL(thing, '2012-12-31')
"""

# Maps from object names to numbers that JPL-Horizons will recognize without
# fussing around with choosing between barycenters, substrings, etc..
# Use lower case keys.
# Do not use keys for asteroids that are also in planets_and_moons.  (The IAU
# might enforce this anyway.)

asteroids = {'ceres':        1,
             'pallas':       2,
             'juno':         3, # Large crater and temperature changes.
             'vesta':        4,
             'astraea':      5,
             'hygiea':      10, # Careful with the spelling.  It used to be
                                # Hygeia, and it is named after the Greek
                                # goddess Hygieia (or Hygeia).  Also, it is
                                # fairly oblate and eccentric.
             'parthenope':  11,
             'victoria':    12,
             'davida':     511,
             'interamnia': 704}

planets_and_moons = {'mercury':    199,
                     'venus':      299,
                     'mars':       499,
                     'phobos':     401,
                     'deimos':     402,
                     'jupiter':    599,
                     'io':         501,
                     'europa':     502,
                     'ganymede':   503,
                     'callisto':   504,
                     'saturn':     699,
                     'mimas':      601,
                     'enceladus':  602,
                     'tethys':     603,
                     'dione':      604,
                     'rhea':       605,
                     'titan':      606,
                     'hyperion':   607,
                     'iapetus':    608,
                     'phoebe':     609,
                     'janus':      610,
                     'epimetheus': 611,
                     'helene':     612,
                     'telesto':    613,
                     'calypso':    614,
                     'atlas':      615,
                     'prometheus': 616,
                     'pandora':    617,
                     'pan':        618,
                     # I've been to Ymir, so I have a soft spot for it, but it
                     # has an unknown radius (2010).
                     #'ymir':       619, 
                     'uranus':     799,
                     'ariel':      701,
                     'umbriel':    702,
                     'titania':    703,
                     'oberon':     704,
                     'miranda':    705,
                     'cordelia':   706,
                     'ophelia':    707,
                     'bianca':     708,
                     'cressida':   709,
                     'desdemona':  710,
                     'juliet':     711,
                     'portia':     712,
                     'rosalind':   713,
                     'belinda':    714,
                     'puck':       715,
                     # 'caliban':    716, Uncertain radius, 2010.
                     # 'sycorax':    717, Uncertain radius, 2010.
                     # 'prospero':   718, Unknown radius, 2010
                     # 'setebos':    719, Unknown radius, 2010
                     # 'stephano':   720, Unknown radius, 2010
                     # 'trinculo':   721, Unknown radius, 2010
                     # 'francisco':  722, "
                     # 'margaret':   723, Unknown radius, 2010
                     # 'ferdinand':  724, Unknown radius, 2010
                     # 'perdita':    725, Unknown radius, 2010
                     # 'mab':        726, Unknown radius, 2010
                     # 'cupid':      727, "
                     'neptune':    899,
                     'triton':     801,
                     'nereid':     802,
                     'naiad':      803,
                     'thalassa':   804,
                     'despina':    805,
                     'galatea':    806,
                     'larissa':    807,
                     'proteus':    808,
                     'pluto':      999,  # It's still a planet in this sense.
                     'charon':     901
                     # 'nix':        902 Unknown radius, 2010
                     # 'hydra':      903 Unknown radius, 2010
}

should_have_orientation = ['mars', 'deimos', 'phobos', 'vesta', 'jupiter', 'io',
                           'janus', 'enceladus', 'mimas', 'iapetus',
                           'phoebe', 'tethys', 'uranus', 'ariel', 'miranda',
                           'neptune']
should_have_sublong = ['mars', 'deimos', 'phobos', 'jupiter', 'io',
                       'janus', 'enceladus', 'phoebe', 'mimas', 'tethys',
                       'neptune']

# Getting positions once a day is not enough for many moons, if the position of
# the moon relative to its primary will be needed.  Note that a maximum
# suitable increment is imposed by Earth's motion.
default_date_incrs = {
    'default': "1 d",  # The default default.
    'ariel': '0.5d',
    'cordelia': '0.05d',
    'deimos': '0.25d',
    'dione': '0.5d',
    'enceladus': '0.25d',
    'io': '0.25d',
    'janus': '0.1d',
    'mimas': '0.2d',
    'miranda': '0.25 d',
    'phobos': '0.05d',
    'tethys': '0.4d'
    }

def request_from_JPL(objnam, enddate,
                     startdate=None,
                     date_incr=None,
                     get_axis_orientation=None,
                     get_sub_long=None,
                     obsloc="",
                     return_address=None,
                     mailserver=None,
                     use_apparent=True,
                     get_sep=None):
    """
    Request an ASCII ephemeris table from JPL-Horizons for a Solar System
    object.  If all goes well it should arrive by email in a few minutes to
    an hour.  (The return value from this function is whether or not it sent
    the request.)

    All but the first two parameters have hopefully sensible defaults:
    objnam:
        The name of the object (case-insensitive).  It will be used to refer to
        specifically its center, as opposed to other possible locations in the
        vicinity of the object.  For example, if objnam ="Mars", it will choose
        Mars, not the Mars barycenter or the Mars Reconnaissance Orbiter.
    enddate:
        The date that the ephemeris should end on.
        It can be an epoch measure or string (yyyy-mm-dd, assumes UT).
    startdate:
        Defaults to today, but it can be specified like enddate.
    date_incr:
        The increment between dates in the ephemeris.  casapy's setjy
        task and me tool automatically interpolate.  It can be a (time) quantity
        or a string (which will be interpreted as if it were a quantity).
        
        Unlike the JPL email interface, this does not need it to be an integer
        number of time units.  request_from_JPL() will do its best to convert
        it to fit JPL's required format.
        
        Default: 1 Earth day.
    get_axis_orientation:
        Request the orientation of the object's polar axis relative to the line
        of sight.  This is needed (along with the flattening) if treating the
        disk as an ellipse, but it is often unavailable.
        True or False
        Defaults to whether or not objnam is in should_have_orientation.
    get_sub_long:
        Request the planetographic (geodetic) longitudes and latitudes of the
        subobserver and sub-Solar points.  Only needed if the object has
        significant known surface features.
        True or False
        Defaults to whether or not objnam is in should_have_sublong.
    obsloc:
        Observatory name, used to get topocentric coordinates.
        Obviously not all observatories are recognized.
        Default: "" (geocentric)
    return_address:
        The email address that the ephemeris will be sent to.
        Default: <username>@<domainname>.
    mailserver:
        The computer at _your_ end to send the mail from.
        Default: a semi-intelligent guess.
    use_apparent:
        Get the apparent instead of J2000 RA and Dec.  No refraction by Earth's
        atmosphere will be applied; MeasComet assumes apparent directions and
        JPL_ephem_reader would be confused if both apparent and J2000
        directions were present.
        Default: True
    get_sep:
        Get the angular separation from the primary, and whether it is
        transiting, in eclipse, etc..  This only makes sense for moons and does
        not guarantee that nothing else (like Earth, Luna, a bright extrasolar
        object) is in the line of sight!
        Default: True if it is in the moons list, False otherwise.
    """
    lobjnam = objnam.lower()

    # Handle defaults
    if get_axis_orientation == None:        # remember False is valid.
        if lobjnam in should_have_orientation:
            get_axis_orientation = True
        else:
            get_axis_orientation = False

    if get_sub_long == None:                # remember False is valid.
        if lobjnam in should_have_sublong:
            get_sub_long = True
        else:
            get_sub_long = False
    
    if not return_address:    
        fqdn = socket.getfqdn()
    
        # Only use the top two levels, i.e. eso.org and nrao.edu, not
        # (faraday.)cv.nrao.edu.
        domain = '.'.join(fqdn.split('.')[-2:])

        return_address = os.getlogin() + '@' + domain

    if not mailserver:
        try:
            #mailserver = socket.getfqdn(socket.gethostbyname('mail'))
            mailserver = socket.getfqdn(socket.gethostbyname('smtp'))
        except socket.gaierror:
            print "Could not find a mailserver."
            return False

    if not startdate:
        syr, smon, s_d, s_h, smin, s_s, swday, syday, sisdst = time.gmtime()
        startdate = "%d-%02d-%02d" % (syr, smon, s_d)

    if not date_incr:
        date_incr = default_date_incrs.get(lobjnam,
                                           default_date_incrs['default'])

    if get_sep == None:
        get_sep = (planets_and_moons.get(lobjnam, 99) % 100) < 99

    # Get to work.
    if lobjnam in asteroids:
        objnum = str(asteroids[lobjnam]) + ';'
    elif lobjnam in planets_and_moons:
        objnum = str(planets_and_moons[lobjnam])
    else:
        print "The JPL object number for", objnam, "is not known.  Try looking it up at"
        print 'http://ssd.jpl.nasa.gov/horizons.cgi?s_body=1#top and adding it.'
        return False

    if obsloc and obsloc.lower() != 'geocentric':
        print "Topocentric coordinates are not yet supported by this script."
        print "Defaulting to geocentric."
    center = '500@399'

    quantities = [2, 10, 12, 19, 20, 24]
    if not use_apparent:
        quantities[0] = 1
    if get_axis_orientation:
        quantities.append(32)
    if get_sub_long:
        quantities.append(14)
        quantities.append(15)
    if not get_sep:
        quantities.remove(12)

    # It seems that STEP_SIZE must be an integer, but the unit can be changed
    # to hours or minutes.
    match = re.match(r'([0-9.]+)\s*([dhm])', date_incr)
    n_time_units = float(match.group(1))
    time_unit    = match.group(2)
    if n_time_units < 1.0:
        if time_unit == 'd':
            n_time_units *= 24.0
            time_unit = 'h'
        if time_unit == 'h' and n_time_units < 1.0:     # Note fallthrough.
            n_time_units *= 60.0
            time_unit = 'm'
        if n_time_units < 1.0:                          # Uh oh.
            print date_incr, "is an odd request for a date increment."
            print "Please change it or make your request manually."
            return False
        print "Translating date_incr from", date_incr,
        date_incr = "%.0f %s" % (n_time_units, time_unit)
        print "to", date_incr
    
    instructions = "\n".join(["!$$SOF",
                              "COMMAND= '%s'" % objnum,
                              'CENTER= ' + center,
                              "MAKE_EPHEM= 'YES'",
                              "TABLE_TYPE= 'OBSERVER'",
                              "START_TIME= '%s'" % startdate,
                              "STOP_TIME= '%s'" % enddate,
                              "STEP_SIZE= '%s'" % date_incr,
                              "CAL_FORMAT= 'CAL'",
                              "TIME_DIGITS= 'MINUTES'",
                              "ANG_FORMAT= 'DEG'",
                              "OUT_UNITS= 'KM-S'",
                              "RANGE_UNITS= 'AU'",
                              "APPARENT= 'AIRLESS'",
                              "SOLAR_ELONG= '0,180'",
                              "SUPPRESS_RANGE_RATE= 'NO'",
                              "SKIP_DAYLT= 'NO'",
                              "EXTRA_PREC= 'NO'",
                              "R_T_S_ONLY= 'NO'",
                              "REF_SYSTEM= 'J2000'",
                              "CSV_FORMAT= 'NO'",
                              "OBJ_DATA= 'YES'",
                              "QUANTITIES= '%s'" % ','.join([str(q) for q in quantities]),
                              '!$$EOF'])

    # Set up a MIMEText object (it's a dictionary)
    msg = MIMEText(instructions)

    msg['To'] = "horizons@ssd.jpl.nasa.gov"
    msg['Subject'] = 'JOB'
    msg['From'] = return_address
    msg['Reply-to'] = return_address

    # Establish an SMTP object and connect to the mail server
    s = smtplib.SMTP()
    s.connect(mailserver)

    # Send the email - real from, real to, extra headers and content ...
    s.sendmail(return_address, msg['To'], msg.as_string())
    s.close()

    return True

def list_moons():
    """
    List planets_and_moons in a more organized way.
    """
    # Gather the moons by planet number.
    planets = {}
    moons = {}
    for lcname in planets_and_moons:
        num = planets_and_moons[lcname]
        planet = num / 100
        if num % 100 == 99:
            planets[planet] = lcname.title()
        else:
            if not moons.has_key(planet):
                moons[planet] = {}
            moons[planet][num % 100] = lcname.title()

    #print "planets =", planets
    #print "moons:"
    #for p in planets:
    #    print planets[p]
    #    print " ", moons.get(p, "None")

    # For formatting the output table, find the column widths,
    # and maximum number of moons per planet.
    maxmoons = max([len(moons.get(p, '')) for p in planets])
    maxwidths = {}
    for planet in planets:
        if moons.has_key(planet):
            maxwidths[planet] = max([len(m) for m in moons[planet].values()])
        else:
            maxwidths[planet] = 0
        if len(planets[planet]) > maxwidths[planet]:
            maxwidths[planet] = len(planets[planet])

    # Set up the table columns.
    plannums = planets.keys()
    plannums.sort()
    sortedmoons = {}
    formstr = ''
    hrule   = ''
    for p in plannums:
        formstr += '| %-' + str(maxwidths[p]) + 's '
        if p == 1:
            hrule += '|'
        else:
            hrule += '+'
        hrule += '-' * (maxwidths[p] + 2)
        moonkeys = moons.get(p, {}).keys()
        moonkeys.sort()
        sortedmoons[p] = {}
        for row in xrange(len(moonkeys)):
            sortedmoons[p][row] = moons[p][moonkeys[row]]
    formstr += '|'
    hrule   += '|'

    print formstr % tuple([planets[p] for p in plannums])
    print hrule
    for row in xrange(maxmoons):
        print formstr % tuple([sortedmoons[p].get(row, '') for p in plannums])

def list_asteroids():
    """
    Like list_moons, but list the asteroids by their numbers
    (= order of discovery, ~ albedo * size)
    """
    astnums = asteroids.values()
    astnums.sort()
    invast = {}
    for a in asteroids:
        invast[asteroids[a]] = a.title()
    for n in astnums:
        print "%3d %s" % (n, invast[n])
