"""
casapy functions for converting ASCII ephemerides from JPL-Horizons into
CASA tables and installing them where casapy can find them.
                    
jplfiles_to_repository() puts it all together, so it is most likely the
function you want.

There are various utilities like convert_radec, datestr*, get_num_from_str,
mean_radius*, and construct_tablepath defined in here as well.
"""

from glob import glob
import os
import re
import scipy.special
import time                  # We can always use more time.

## # This seems like overkill just to get me and qa, but it creates local copies.
from taskinit import gentools, qa, casalog
im, cb, ms, tb, fg, af, me, ia, po, sm, cl, cs, rg, sl, dc, vp = gentools()

from dict_to_table import dict_to_table

# Possible columns, as announced by their column titles.
# The data is scooped up by 'pat'.  Either use ONE group named by the column
# key, or mark it as unwanted.  '-' is not valid in group names.
# Leading and trailing whitespace will be taken care of later.
# Sample lines:
#  Date__(UT)__HR:MN     R.A.___(ICRF/J2000.0)___DEC Ob-lon Ob-lat Sl-lon Sl-lat   NP.ang   NP.dist               r        rdot            delta      deldot    S-T-O   L_s
#  2010-May-01 00:00     09 01 43.1966 +19 04 28.673 286.52  18.22 246.99  25.34 358.6230      3.44  1.661167637023  -0.5303431 1.28664311447968  15.7195833  37.3033   84.50
cols = {
    'MJD': {'header': r'Date__\(UT\)__HR:MN',
            'comment': 'date',
            'pat':     r'(?P<MJD>\d+-\w+-\d+ \d+:\d+)'},
    'RA': {'header': r'R.A._+\([^)]+',
           'comment': 'Right Ascension',
           'pat':    r'(?P<RA>(\d+ \d+ )?\d+\.\d+)'}, # require a . for safety
    'DEC': {'header': r'\)_+DEC.',
            'comment': 'Declination',
            'pat':    r'(?P<DEC>([-+]?\d+ \d+ )?[-+]?\d+\.\d+)'},
    'illu': {'header': r'Illu%',
             'comment': 'Illumination',
             'pat':    r'(?P<illu>[0-9.]+)',
             'unit': r'%'},
    'DiskLong': {'header': r'Ob-lon',
               'comment': 'Sub-observer longitude',
               'pat':    r'(?P<DiskLong>[0-9.]+|n\.a\.)',
               'unit': 'deg'},
    'DiskLat': {'header': r'Ob-lat',
               'comment': 'Sub-observer longitude',
               'pat':    r'(?P<DiskLat>[-+0-9.]+|n\.a\.)',
               'unit': 'deg'},
    'sl_lon': {'header': r'Sl-lon',
               'comment': 'Sub-Solar longitude',
               'pat':    r'(?P<sl_lon>[0-9.]+|n\.a\.)',
               'unit': 'deg'},
    'sl_lat': {'header': r'Sl-lat',
               'comment': 'Sub-Solar longitude',
               'pat':    r'(?P<sl_lat>[-+0-9.]+|n\.a\.)',
               'unit': 'deg'},

    # These are J2000 whether or not ra and dec are apparent directions.
    'NP_RA': {'header': r'N\.Pole-RA',
              'comment': 'North Pole right ascension',
              'pat':    r'(?P<NP_RA>(\d+ \d+ )?\d+\.\d+)'}, # require a . for safety
    'NP_DEC': {'header': r'N\.Pole-DC',
               'comment': 'North Pole declination',
               'pat':    r'(?P<NP_DEC>([-+]?\d+ \d+ )?[-+]?\d+\.\d+)'},
    
    'r': {'header': 'r',
          'comment': 'heliocentric distance',
          'unit':    'AU',
          'pat':     r'(?P<r>[0-9.]+)'},
    'rdot': {'header': 'rdot',
             'pat': r'[-+0-9.]+',
             'unwanted': True},
    'Rho': {'header': 'delta',
            'comment': 'geocentric distance',
            'unit':    'AU',
            'pat':     r'(?P<Rho>[0-9.]+)'},
    'RadVel': {'header': 'deldot',
               'comment': 'Radial velocity relative to the observer',
               'pat': r'(?P<RadVel>[-+0-9.]+)',
               'unit': 'km/s'},
    'phang': {'header':  'S-T-O',
              'comment': 'phase angle',
              'unit':    'deg',
              'pat':     r'(?P<phang>[0-9.]+)'},
    'ang_sep': {'header': 'ang-sep/v',
                'comment': 'Angular separation from primary',
                'pat': r'(?P<ang_sep>[0-9.]+/.)'},  # arcsec, "visibility code".
                                                    # t: transiting primary
                                                    # O: occulted by primary
                                                    # p: partial umbral eclipse
                                                    # P: occulted partial umbral eclipse
                                                    # u: total umbral eclipse
                                                    # U: occulted total umbral eclipse
                                                    # *: none of the above
                                                    # -: target is primary
    'L_s': {'header': 'L_s',  # 08/2010: JPL does not supply this and
            'unit': 'deg',    # says they cannot.  Ask Bryan Butler.
            'comment': 'Season angle',
            'pat': r'(?P<L_s>[-+0-9.]+)'}
    }

def readJPLephem(fmfile):
    """
    Reads a JPL Horizons text file (see
    http://ssd.jpl.nasa.gov/horizons.cgi#top ) for a solar system object and
    returns various quantities in a dictionary.  The dict will be blank ({}) if
    there is a failure.
    """
    retdict = {}
    casalog.origin('readJPLephem')

    # Try opening fmfile now, because otherwise there's no point continuing.
    try:
        ephem = open(fmfile)
    except IOError:
        casalog.post("Could not open ephemeris file " + fmfile,
                     priority="SEVERE")
        return {}

    # Setup the regexps.

    # Headers (one time only things)
    
    # Dictionary of quantity label: regexp pattern pairs that will be searched
    # for once.  The matching quantity will go in retdict[label].  Only a
    # single quantity (group) will be retrieved per line.
    headers = {
        'NAME': {'pat': r'^Target body name:\s+\d*\s*(\w+)'},   # object name, w.o. number
        'ephtype': {'pat': r'\?s_type=1#top>\]\s*:\s+\*(\w+)'}, # e.g. OBSERVER
        'obsloc': {'pat': r'^Center-site name:\s+(\w+)'},        # e.g. GEOCENTRIC
        # Catch either an explicit mean radius or a solitary target radius.
        'meanrad': {'pat': r'(?:Mean radius \(km\)\s*=|^Target radii\s*:)\s*([0-9.]+)(?:\s*km)?\s*$',
                    'unit': 'km'},
        # Triaxial target radii
        'radii': {'pat': r'Target radii\s*:\s*([0-9.]+\s*x\s*[0-9.]+\s*x\s*[0-9.]+)\s*km.*Equator, meridian, pole',
                  'unit': 'km'},
        'T_mean': {'pat': r'Mean Temperature \(K\)\s*=\s*([0-9.]+)',
                   'unit': 'K'},

         # Figure out the units later.
        'rot_per': {'pat': r'(?i)(?<!Inferred )\b(rot(ation(al)?|\.)?\s*per.*=\s*([-0-9.]+\s*[dhr]*|Synchronous))'},
        'orb_per': {'pat': r'Orbital period((, days)?\s*=\s*[-0-9.]+\s*[dhr](\s*\(?R\)?)?)'},

        # MeasComet does not read units for these! E-lon(deg),  Lat(deg),     Alt(km)
        'GeoLong': {'pat': r'^Center geodetic\s*: ([-+0-9.]+,\s*[-+0-9.]+,\s*[-+0-9.]+)'},
        'dMJD':    {'pat': r'^Step-size\s*:\s*(.+)'},

        #                     request method v  wday mth   mday  hh  mm  ss   yyyy
        'VS_CREATE': {'pat': r'^Ephemeris / \w+ \w+ (\w+\s+\d+\s+\d+:\d+:\d+\s+\d+)'}
        }
    for hk in headers:
        headers[hk]['pat'] = re.compile(headers[hk]['pat'])

    # Data ("the rows of the table")
    
    # need date, r (heliocentric distance), delta (geocentric distance), and phang (phase angle).
    # (Could use the "dot" time derivatives for Doppler shifting, but it's
    # likely unnecessary.)
    datapat = r'^\s*'

    stoppat = r'\$\$EOE$'  # Signifies the end of data.

    # Read fmfile into retdict.
    num_cols = 0
    in_data = False
    comp_mismatches = []
    print_datapat = False
    for line in ephem:
        if in_data:
            if re.match(stoppat, line):
                break
            matchobj = re.search(datapat, line)
            if matchobj:
                gdict = matchobj.groupdict()
                for col in gdict:
                    retdict['data'][col]['data'].append(gdict[col])
                if len(gdict) < num_cols:
                    print "Partially mismatching line:"
                    print line
                    print "Found:"
                    print gdict
                    print_datapat = True
            else:
                print_datapat = True
                # Chomp trailing whitespace.
                comp_mismatches.append(re.sub(r'\s*$', '', line))
        elif re.match(r'^\s*' + cols['MJD']['header'] + r'\s+'
                      + cols['RA']['header'], line):
            # See what columns are present, and finish setting up datapat and
            # retdict.
            havecols = []
            # Chomp trailing whitespace.
            myline = re.sub(r'\s*$', '', line)
            titleline = myline
            remaining_cols = cols.keys()
            found_col = True
            # This loop will terminate one way or another.
            while myline and remaining_cols and found_col:
                found_col = False
                #print "myline = '%s'" % myline
                #print "remaining_cols =", ', '.join(remaining_cols)
                for col in remaining_cols:
                    if re.match(r'^\s*' + cols[col]['header'], myline):
                        #print "Found", col
                        havecols.append(col)
                        remaining_cols.remove(col)
                        myline = re.sub(r'^\s*' + cols[col]['header'],
                                        '', myline)
                        found_col = True
                        break
            datapat += r'\s+'.join([cols[col]['pat'] for col in havecols])
            sdatapat = datapat
            casalog.post("Found columns: " + ', '.join(havecols))
            datapat = re.compile(datapat)
            retdict['data'] = {}
            for col in havecols:
                if not cols[col].get('unwanted'):
                    retdict['data'][col] = {'comment': cols[col]['comment'],
                                            'data':    []}
            num_cols = len(retdict['data'])
        elif re.match(r'^\$\$SOE\s*$', line):  # Start of ephemeris
            casalog.post("Starting to read data.", priority='INFO2')
            in_data = True
        else:
            #print "line =", line
            #print "looking for", 
            for hk in headers:
                if not retdict.has_key(hk):
                    #print hk,
                    matchobj = re.search(headers[hk]['pat'], line)
                    if matchobj:
                        retdict[hk] = matchobj.group(1) # 0 is the whole line
                        break
    ephem.close()

    # If there were errors, provide debugging info.
    if comp_mismatches:
        print "Completely mismatching lines:"
        print "\n".join(comp_mismatches)
    if print_datapat:
        print "The apparent title line is:"
        print titleline
        print "datapat = r'%s'" % sdatapat

    # Convert numerical strings into actual numbers.
    try:
        retdict['earliest'] = datestr_to_epoch(retdict['data']['MJD']['data'][0])
        retdict['latest'] = datestr_to_epoch(retdict['data']['MJD']['data'][-1])
    except Exception, e:
        print "Error!"
        if retdict.has_key('data'):
            if retdict['data'].has_key('MJD'):
                if retdict['data']['MJD'].has_key('data'):
                    #print "retdict['data']['MJD']['data'] =", retdict['data']['MJD']['data']
                    print "retdict['data'] =", retdict['data']
                else:
                    print "retdict['data']['MJD'] has no 'data' key."
                    print "retdict['data']['MJD'].keys() =", retdict['data']['MJD'].keys()
            else:
                print "retdict['data'] has no 'MJD' key."
                print "retdict['data'].keys() =", retdict['data'].keys()
        else:
            print "retdict has no 'data' key."
        raise e

    for hk in headers:
        if retdict.has_key(hk):
            if headers[hk].has_key('unit'):
                if hk == 'radii':
                    radii = retdict[hk].split('x')
                    a, b, c = [float(r) for r in radii]
                    retdict[hk] = {'unit': headers[hk]['unit'],
                                   'value': (a, b, c)}
                    retdict['meanrad'] = {'unit': headers[hk]['unit'],
                                          'value': mean_radius(a, b, c)}
                else:
                    try:
                        # meanrad might already have been converted.
                        if type(retdict[hk]) != dict:
                            retdict[hk] = {'unit': headers[hk]['unit'],
                                           'value': float(retdict[hk])}
                    except Exception, e:
                        print "Error converting header", hk, "to a Quantity."
                        print "retdict[hk] =", retdict[hk]
                        raise e
            elif hk == 'GeoLong':
                long_lat_alt = retdict[hk].split(',')
                retdict['GeoLong'] = float(long_lat_alt[0])
                retdict['GeoLat']  = float(long_lat_alt[1])
                retdict['GeoDist'] = float(long_lat_alt[2])
            elif hk == 'dMJD':
                retdict[hk] = qa.convert(qa.totime(retdict[hk].replace('minutes', 'min')),
                                         'd')['value']
            elif hk == 'orb_per':
                unit = 'h'
                retrograde = False
                if 'd' in retdict[hk].lower():
                    unit = 'd'                 # Actually this is most common.
                if 'r' in retdict[hk].lower():
                    retrograde = True
                value = get_num_from_str(retdict[hk], 'orbital period')
                if value != False:
                    if retrograde and value > 0.0:
                        value = -value
                    retdict[hk] = {'unit': unit, 'value': value}
                else:
                    del retdict[hk]
                    
    # The rotation period might depend on the orbital period ("Synchronous"),
    # so handle it after all the other headers have been done.
    if 'rot_per' in retdict:
        rpstr = retdict['rot_per']
        if 'ROTPER' in rpstr:                            # Asteroid
            retdict['rot_per'] = {'unit': 'h',         # Always seems to be for asteroids.
                                  'value': get_num_from_str(rpstr, 'rotation period')}
        elif 'Synchronous' in rpstr:
            retdict['rot_per'] = retdict['orb_per']
        else:  # Most likely a planet.
            match = re.search(r'(\d+)h\s*(\d+)m\s*([0-9.]+)s', rpstr)
            if match:
                hms = [float(match.group(i)) for i in range(1, 4)]
                retdict['rot_per'] = {'unit': 'h',
                                      'value': hms[0] + (hms[1] + hms[2] / 60.0) / 60.0}
            else:
                # DON'T include the optional r in hr!  qa.totime can't handle it.
                try:
                    match = re.search(r'([-0-9.]+)(?:\s*\+-[0-9.]+)?\s*([dh])', rpstr)
                    if match:
                        retdict['rot_per'] = {'unit': match.group(2),
                                              'value': float(match.group(1))}
                except:
                    print "Error parsing the rotation period from"
                    print rpstr
    
    if retdict['data'].has_key('ang_sep'):
        retdict['data']['obs_code'] = {'comment': 'Obscuration code'}
    for dk in retdict['data']:
        if dk == 'obs_code':
            continue
        if cols[dk].has_key('unit'):
            retdict['data'][dk]['data'] = {'unit': cols[dk]['unit'],
                      'value': scipy.array([float(s) for s in retdict['data'][dk]['data']])}
            if dk == 'RadVel':
                # Convert from km/s to AU/d.  Blame MeasComet, not me.
                retdict['data'][dk]['data']['unit'] = 'AU/d'
                kmps_to_AUpd = qa.convert('1km/s', 'AU/d')['value']
                retdict['data'][dk]['data']['value'] *= kmps_to_AUpd

        if re.match(r'.*(RA|DEC)$', dk):
            retdict['data'][dk] = convert_radec(retdict['data'][dk])
        elif dk == 'MJD':
            retdict['data']['MJD'] = datestrs_to_MJDs(retdict['data']['MJD'])
        elif dk == 'ang_sep':
            angseps = []
            obscodes = []
            for asoc in retdict['data'][dk]['data']:
                angsep, obscode = asoc.split('/')
                angseps.append(float(angsep))
                obscodes.append(obscode)
            retdict['data'][dk]['data'] = {'unit': 'arcseconds',
                                           'value': angseps}
            retdict['data']['obs_code']['data'] = obscodes

    if len(retdict.get('radii', {'value': []})['value']) == 3 \
           and retdict['data'].has_key('NP_RA') and retdict['data'].has_key('NP_DEC'):
        # Do a better mean radius estimate using the actual theta.
        retdict['meanrad']['value'] = mean_radius_with_known_theta(retdict)

    # To be eventually usable as a MeasComet table, a few more keywords are needed.
    retdict['VS_TYPE'] = 'Table of comet/planetary positions'
    retdict['VS_VERSION'] = '0003.0001'
    if retdict.has_key('VS_CREATE'):
        dt = time.strptime(retdict['VS_CREATE'], "%b %d %H:%M:%S %Y")
    else:
        casalog.post("The ephemeris creation date was not found.  Using the current time.",
                     priority="WARN")
        dt = time.gmtime()
    retdict['VS_CREATE'] = time.strftime('%Y/%m/%d/%H:%M', dt)

    # VS_DATE is required by MeasComet, but it doesn't seem to be actually used.
    retdict['VS_DATE'] = time.strftime('%Y/%m/%d/%H:%M', time.gmtime())

    if retdict['data'].has_key('MJD'):
        retdict['MJD0'] = retdict['data']['MJD']['value'][0] - retdict['dMJD']
    else:
        print "The table will not be usable with me.framecomet because it lacks MJD."

    return retdict

def convert_radec(radec_col):
    """
    Returns a column of RAs or declinations as strings, radec_col, as a
    quantity column.  (Unfortunately MeasComet assumes the columns are
    Quantities instead of Measures, and uses GeoDist == 0.0 to toggle between
    APP and TOPO.)
    """
    angstrlist = radec_col['data']
    angq = {}
    nrows = len(angstrlist)

    if len(angstrlist[0].split()) > 1:
        # Prep angstrlist for qa.toangle()
        if radec_col['comment'][:len("declination")].lower() == 'declination':
            for i in xrange(nrows):
                dms = angstrlist[i].replace(' ', 'd', 1)
                angstrlist[i] = dms.replace(' ', 'm') + 's'
        else:                                                  # R.A.
            for i in xrange(nrows):
                angstrlist[i] = angstrlist[i].replace(' ', ':')        

        # Do first conversion to get unit.
        try:
            firstang = qa.toangle(angstrlist[0])
        except Exception, e:
            print "Error: Could not convert", angstrlist[0], "to an angle."
            raise e
        angq['unit'] = firstang['unit']
        angq['value'] = [firstang['value']]

        for angstr in angstrlist[1:]:
            angq['value'].append(qa.toangle(angstr)['value'])
    else:
        angq['unit'] = 'deg'                    # This is an assumption!
        angq['value'] = [float(a) for a in angstrlist]

    return {'comment': radec_col['comment'],
            'data': {'unit': angq['unit'],
                     'value': scipy.array(angq['value'])}}

def get_num_from_str(fstr, wanted="float"):
    """
    Like float(fstr) on steroids, in that it ignores things in fstr that aren't
    numbers.  Returns False on failure.

    wanted: an optional label for the type of number you wanted.
            Only used for distinguishing error messages.
            
    Example:
    >>> from JPLephem_reader import get_num_from_str
    >>> get_num_from_str('  Sidereal rot. period  =    58.6462 d  ')
    58.6462
    >>> get_num_from_str('Rotation period = 16.11+-0.01 hr', wanted='rotation period')
    16.109999999999999
    >>> get_num_from_str('Rotation period = Synchronous', wanted='rotation period')
    Could not convert "Rotation period = Synchronous" to a rotation period.
    False
    """
    match = re.search(r'([-+]?(\d+(\.\d*)?|\d*\.\d+)([eEdD][-+]?\d+)?)', fstr)
    if match:
        value = float(match.group(1))
    else:
        print "Could not convert \"%s\" to a %s." % (fstr, wanted)
        value = False
    return value

def mean_radius(a, b, c):
    """
    Return the average apparent mean radius of an ellipsoid with semiaxes
    a >= b >= c.
    "average" means average over time naively assuming the pole orientation
    is uniformly distributed over the whole sphere, and "apparent mean radius"
    means a radius that would give the same area as the apparent disk.
    """
    # This is an approximation, but it's not bad.
    # The exact equations for going from a, b, c, and the Euler angles to the
    # apparent ellipse are given in Drummond et al, Icarus, 1985a.
    # It's the integral over the spin phase that I have approximated, so the
    # approximation is exact for b == a, and appears to hold well for b << a.
    R = 0.5 * c**2 * (1.0 / b**2 + 1.0 / a**2)   # The magic ratio.
    if R < 0.95:
        sqrt1mR = scipy.sqrt(1.0 - R)
        # There is fake singularity (RlnR) at R = 0, but it is unlikely to
        # be a problem.
        try:
            Rterm = 0.5 * R * scipy.log((1.0 + sqrt1mR) / (1.0 - sqrt1mR)) / sqrt1mR
        except:
            Rterm = 0.0
    else:
        # Use a (rapidly converging) series expansion to avoid a fake
        # singularity at R = 1.
        Rterm = 1.0               # 0th order
        onemR = 1.0 - R
        onemRtothei = 1.0
        for i in xrange(1, 5):    # Start series at 1st order.
            onemRtothei *= onemR
            Rterm -= onemRtothei / (0.5 + 2.0 * i**2)
    avalfabeta = 0.5 * a * b * (1.0 + Rterm)
    return scipy.sqrt(avalfabeta)

def mean_radius_with_known_theta(retdict):
    """
    Return the average apparent mean radius of an ellipsoid with semiaxes
    a >= b >= c (= retdict['radii']['value']).
    "average" means average over a rotation period, and "apparent mean radius"
    means the radius of a circle with the same area as the apparent disk.
    """
    a = retdict['radii']['value'][0]
    b2 = retdict['radii']['value'][1]**2
    c2 = retdict['radii']['value'][2]**2
    onemboa2 = 1.0 - b2 / a**2
    units = {}
    values = {}
    for c in ['RA', 'DEC', 'NP_RA', 'NP_DEC']:
        units[c] = retdict['data'][c]['data']['unit']
        values[c] = retdict['data'][c]['data']['value']
    av = 0.0
    nrows = len(retdict['data']['RA']['data']['value'])
    for i in xrange(nrows):
        radec = me.direction('app', {'unit': units['RA'], 'value': values['RA'][i]},
                             {'unit': units['DEC'], 'value': values['DEC'][i]})
        np = me.direction('j2000', {'unit': units['NP_RA'], 'value': values['NP_RA'][i]},
                          {'unit': units['NP_DEC'], 'value': values['NP_DEC'][i]})
        szeta2 = scipy.sin(qa.convert(me.separation(radec, np), 'rad')['value'])**2
        csinz2 = c2 * szeta2
        bcosz2 = b2 * (1.0 - szeta2)
        bcz2pcsz2 = bcosz2 + csinz2
        m = csinz2 * onemboa2 / bcz2pcsz2
        av += (scipy.sqrt(bcz2pcsz2) * scipy.special.ellipe(m) - av) / (i + 1.0)
    return scipy.sqrt(2.0 * a * av / scipy.pi)

def datestr_to_epoch(datestr):
    """
    Given a UT date like "2010-May-01 00:00", returns an epoch measure.
    """
    return me.epoch(rf='UTC', v0=qa.totime(datestr))

def datestrs_to_MJDs(cdsdict):
    """
    All of the date strings must have the same reference frame (i.e. UT).
    """
    datestrlist = cdsdict['data']

    # Convert to FITS format, otherwise qa.totime() will silently drop the hours.
    datestrlist = [d.replace(' ', 'T') for d in datestrlist]
    
    timeq = {}
    # Do first conversion to get unit.
    firsttime = qa.totime(datestrlist[0])
    timeq['unit'] = firsttime['unit']
    timeq['value'] = [firsttime['value']]
    
    for datestr in datestrlist[1:]:
        timeq['value'].append(qa.totime(datestr)['value'])

    return {'unit': timeq['unit'],
            'value': scipy.array(timeq['value'])}

def construct_tablepath(fmdict, prefix=''):
    """
    Construct a suitable pathname for a CASA table made from fmdict,
    starting with prefix.  prefix can contain a /.

    If prefix is not given, it will be set to
    "ephem_JPL-Horizons_%s" % fmdict['NAME']
    """
    if not prefix:
        prefix = "ephem_JPL-Horizons_%s" % fmdict['NAME']
    return prefix + "_%.0f-%.0f%s%s.tab" % (fmdict['earliest']['m0']['value'],
                                            fmdict['latest']['m0']['value'],
                                            fmdict['latest']['m0']['unit'],
                                            fmdict['latest']['refer'])

def ephem_dict_to_table(fmdict, tablepath='', prefix=''):
    """
    Converts a dictionary from readJPLephem() to a CASA table, and attempts to
    save it to either to tablepath or a constructed directory name.
    Returns whether or not it was successful.

    If tablepath is blank and prefix is not given, the table will go to
    something like ephem_JPL-Horizons_NAME_EARLIEST-LATESTdUTC.tab.

    If tablepath is blank and prefix is given, the table will go to
    something like prefix_EARLIEST-LATESTdUTC.tab.  prefix can contain a /.
    """
    if not tablepath:
        tablepath = construct_tablepath(fmdict, prefix)
        print "Writing to", tablepath
        
    retval = True
    try:
        outdict = fmdict.copy() # Yes, I want a shallow copy.
        kws = fmdict.keys()
        kws.remove('data')
        collist = outdict['data'].keys()

        # For cosmetic reasons, encourage a certain order to the columns, i.e.
        # start with alphabetical order,
        collist.sort()
        # but put these ones first, in the listed order (ignore the reverse and
        # the pops) if they are present.
        put_these_first = ['MJD', 'RA', 'DEC', 'Rho', 'RadVel', 'NP_RA', 'NP_DEC',
                           'DiskLong', 'DiskLat', 'sl_lon', 'sl_lat', 'r',
                           'ang_sep', 'obs_code']
        # Like l.sort(), reverse() acts on its instance instead of returning a value.
        put_these_first.reverse()
        for c in put_these_first:
            if c in collist:
                collist.remove(c)
                collist.insert(0, c)
        
        clashing_cols = []
        for c in collist:
            if c in kws:
                clashing_cols.append(c)
        if clashing_cols:
            raise ValueError, 'The input dictionary lists' + ', '.join(clashing_cols) + 'as both keyword(s) and column(s)'

        # This promotes the keys in outdict['data'] up one level, and removes
        # 'data' as a key of outdict.
        outdict.update(outdict.pop('data'))

        # This is primarily because MeasComet insists on it, not because it
        # ever gets used.  Maybe subType should be changed to 'Asteroid',
        # 'Moon', or 'Planet', but I'm leaving it at 'Comet' for now.
        info = {'readme': 'Derived by JPLephem_reader.py from a JPL-Horizons ephemeris (http://ssd.jpl.nasa.gov/horizons.cgi#top)',
                'subType': 'Comet', 'type': 'IERS'}

        retval = dict_to_table(outdict, tablepath, kws, collist, info)
    except Exception, e:
        print 'Error', e, 'trying to export an ephemeris dict to', tablepath
        retval = False

    return retval


def jplfiles_to_repository(objs, jpldir='.', jplext='.ephem',
                           log='null'):
    """
    For each Solar System object obj in the list objs,
        look for matching JPL-Horizons ASCII files with jplext in jpldir,
        read them into python dictionaries,
        write the dicts to CASA tables in $CASAROOT/data/ephemerides/JPL-Horizons/,
        and check that they can be read by me.framecomet().
    Returns the number of ephemerides processed + readable by me.framecomet.

    jpldir and jplext can be glob patterns.

    $CASAROOT is derived from $CASAPATH.

    Log messages will be directed to log for the duration of this function.
    Note that 'null' makes a NullLogSink, so it might be better than /dev/null.

    Example:
    import recipes.ephemerides.request as jplreq
    objs = jplreq.asteroids.keys() + jplreq.planets_and_moons.keys()
    jplfiles_to_repository(objs, os.getenv('CASAPATH').split()[0])
    """
    neph = 0
    casapath = os.getenv('CASAPATH')
    if not casapath:
        print "CASAPATH is not set."
        return 0
    datadir = casapath.split()[0] + '/data/ephemerides/JPL-Horizons'
    if not os.path.isdir(datadir):
        try:
            os.mkdir(datadir)
            print "Created", datadir
            print "You should probably svn add it."
        except Exception, e:
            "Error", e, "creating", datadir
            return 0
    datadir += '/'

    #oldlog = casalog.logfile()
    # This is needed to stop WARN and above from printing to the console,
    # but it permanently severs the logger window.
    #casalog.setglobal(True)
    #casalog.setlogfile(log)

    if jpldir[-1] != '/':
        jpldir += '/'
    for sob in objs:
        capob = sob.capitalize()
        lob = sob.lower()
        jplfiles = glob(jpldir + lob + jplext) + glob(jpldir + capob + jplext)
        for jplfile in jplfiles:
            casalog.post('Reading ' + jplfile)
            fmdict = readJPLephem(jplfile)
            tabpath = construct_tablepath(fmdict, datadir + capob)
            ephem_dict_to_table(fmdict, tabpath)

            # Check if it is readable by me.framecomet.
            epoch = fmdict['earliest']
            epoch['m0']['value'] += 0.5 * (fmdict['latest']['m0']['value'] -
                                           epoch['m0']['value'])
            me.doframe(epoch)
            if me.framecomet(tabpath):
                neph += 1
            else:
                casalog.post(tabpath + " was not readable by me.framecomet.",
                             'WARN')

    #casalog.setlogfile(oldlog)

    return neph
