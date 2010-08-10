import os
import smtplib
import socket
from email.mime.text import MIMEText

## # This seems like overkill just to get me and qa.
## from taskinit import *
## im,cb,ms,tb,fg,af,me,ia,po,sm,cl,cs,rg,dc,vp=gentools()

# A map from object names to numbers that JPL-Horizons will recognize without
# fussing around with barycenters, substrings, etc..
# Use lower case keys.
objnums = {'ceres':      1,
           'pallas':     2,
           'juno':       3,
           'vesta':      4,
           #'astraea': 5,      # Clashes with Jupiter's barycenter.
           #'hygeia': 10,      # 10 clashes with the Sun.
           'parthenope': 11,
           'victoria':   12,
           'davida':     511,  # Clashes with Carme.
           'mercury':    199,
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
           'ymir':       619, # I've been to Ymir, so I have a soft spot for it.
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
           'caliban':    716,
           'sycorax':    717,
           'prospero':   718,
           'setebos':    719,
           'stephano':   720,
           'trinculo':   721,
           'francisco':  722,
           'margaret':   723,
           'ferdinand':  724,
           'perdita':    725,
           'mab':        726,
           'cupid':      727,
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
           'charon':     901,
           'nix':        902,
           'hydra':      903
}

def request_from_JPL(objnam, enddate,
                     startdate=None,
                     date_incr="1 d",
                     get_axis_orientation=None,
                     get_sub_long=None,
                     obsloc="",
                     return_address=None,
                     mailserver=None):
    """
    Request an ASCII ephemeris table from JPL-Horizons for a Solar System
    object.  If all goes well it should arrive by email in a few minutes to
    an hour.  (The return value from this function is whether or not it sent
    the request.)

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
        Default: 1 Earth day.
    get_axis_orientation:
        Request the orientation of the object's polar axis relative to the line
        of sight.  This is needed (along with the flattening) if treating the
        disk as an ellipse, but it is often unavailable.
        True or False
        Default: a guess based on objnam.
    get_sub_long:
        Request the planetographic (geodetic) longitudes and latitudes of the
        subobserver and sub-Solar points.  Only needed if the object has
        significant known surface features.
        True or False
        Default: a guess based on objnam.
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
    """
    lobjnam = objnam.lower()

    # Handle defaults
    if get_axis_orientation == None:        # remember False is valid.
        if lobjnam in ['mars', 'jupiter', 'uranus', 'neptune']:
            get_axis_orientation = True
        else:
            get_axis_orientation = False

    if get_sub_long == None:                # remember False is valid.
        if lobjnam in ['mars']:
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

    # Get to work.
    if not objnums.has_key(lobjnam):
        print objnam, "is not in the objnums dictionary.  Try looking it up at"
        print 'http://ssd.jpl.nasa.gov/horizons.cgi?s_body=1#top and adding it.'
        return False

    objnum = objnums[lobjnam]
    
    if obsloc and obsloc.lower() != 'geocentric':
        print "Topocentric coordinates are not yet supported by this script."
        print "Defaulting to geocentric."
    center = '500@399'

    quantities = [1, 10, 14, 15, 17, 19, 20, 24]
    if not get_axis_orientation:
        quantities.remove(17)
    if not get_sub_long:
        quantities.remove(14)
        quantities.remove(15)

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
                              "COMMAND= '%d'" % objnum,
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
                              "SUPPRESS_RANGE_RATE= 'YES'",
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
