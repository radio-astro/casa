import scipy.special
import re
import time                  # We can always use more time.
from taskinit import me, qa
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
    'ra': {'header': r'R.A._+\([^)]+',
           'comment': 'Right Ascension',
           'pat':    r'(?P<ra>(\d+ \d+ )?\d+\.\d+)'}, # require a . for safety
    'dec': {'header': r'\)_+DEC.',
            'comment': 'Declination',
            'pat':    r'(?P<dec>([-+]?\d+ \d+ )?[-+]?\d+\.\d+)'},
    'illu': {'header': r'Illu%',
             'comment': 'Illumination',
             'pat':    r'(?P<illu>[0-9.]+)',
             'unit': r'%'},
    'ob_lon': {'header': r'Ob-lon',
               'comment': 'Sub-observer longitude',
               'pat':    r'(?P<ob_lon>[0-9.]+|n\.a\.)',
               'unit': 'deg'},
    'ob_lat': {'header': r'Ob-lat',
               'comment': 'Sub-observer longitude',
               'pat':    r'(?P<ob_lat>[-+0-9.]+|n\.a\.)',
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
    'np_ra': {'header': r'N\.Pole-RA',
              'comment': 'North Pole right ascension',
              'pat':    r'(?P<np_ra>(\d+ \d+ )?\d+\.\d+)'}, # require a . for safety
    'np_dec': {'header': r'N\.Pole-DC',
               'comment': 'North Pole declination',
               'pat':    r'(?P<np_dec>([-+]?\d+ \d+ )?[-+]?\d+\.\d+)'},
    
    'r': {'header': 'r',
          'comment': 'heliocentric distance',
          'unit':    'AU',
          'pat':     r'(?P<r>[0-9.]+)'},
    'rdot': {'header': 'rdot',
             'pat': r'[-+0-9.]+',
             'unwanted': True},
    'delta': {'header': 'delta',
              'comment': 'geocentric distance',
              'unit':    'AU',
              'pat':     r'(?P<delta>[0-9.]+)'},
    'deldot': {'header': 'deldot',
               'pat': r'[-+0-9.]+',
               'unwanted': True},
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
        'meanrad': {'pat': r'Mean radius \(km\)\s*=\s*([0-9.]+)',
                    'unit': 'km'},
        'radii': {'pat': r'Target radii\s*:\s*([0-9.]+\s*x\s*[0-9.]+\s*x\s*[0-9.]+)\s*km.*Equator, meridian, pole',
                  'unit': 'km'},
        'T_mean': {'pat': r'Mean Temperature \(K\)\s*=\s*([0-9.]+)',
                   'unit': 'K'},

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
                      + cols['ra']['header'], line):
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
    
    if retdict['data'].has_key('ang_sep'):
        retdict['data']['obs_code'] = {'comment': 'Obscuration code'}
    for dk in retdict['data']:
        if dk == 'obs_code':
            continue
        if cols[dk].has_key('unit'):
            retdict['data'][dk]['data'] = {'unit': cols[dk]['unit'],
                      'value': scipy.array([float(s) for s in retdict['data'][dk]['data']])}
        if re.match(r'.*(ra|dec)$', dk):
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
           and retdict['data'].has_key('np_ra') and retdict['data'].has_key('np_dec'):
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
    for c in ['ra', 'dec', 'np_ra', 'np_dec']:
        units[c] = retdict['data'][c]['data']['unit']
        values[c] = retdict['data'][c]['data']['value']
    av = 0.0
    nrows = len(retdict['data']['ra']['data']['value'])
    for i in xrange(nrows):
        radec = me.direction('app', {'unit': units['ra'], 'value': values['ra'][i]},
                             {'unit': units['dec'], 'value': values['dec'][i]})
        np = me.direction('j2000', {'unit': units['np_ra'], 'value': values['np_ra'][i]},
                          {'unit': units['np_dec'], 'value': values['np_dec'][i]})
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


def ephem_dict_to_table(fmdict, tablepath=''):
    """
    Converts a dictionary from readJPLephem() to a CASA table, and attempts to
    save it to tablepath.  Returns whether or not it was successful.
    """
    if not tablepath:
        tablepath = "ephem_JPL-Horizons_%s_%.0f-%.0f%s%s.tab" % (fmdict['NAME'],
                                                              fmdict['earliest']['m0']['value'],
                                                              fmdict['latest']['m0']['value'],
                                                              fmdict['latest']['m0']['unit'],
                                                              fmdict['latest']['refer'])
        print "Writing to", tablepath
        
    retval = True
    try:
        outdict = fmdict.copy() # Yes, I want a shallow copy.
        kws = fmdict.keys()
        kws.remove('data')
        cols = outdict['data'].keys()
        clashing_cols = []
        for c in cols:
            if c in kws:
                clashing_cols.append(c)
        if clashing_cols:
            raise ValueError, 'The input dictionary lists' + ', '.join(clashing_cols) + 'as both keyword(s) and column(s)'

        # This promotes the keys in outdict['data'] up one level, and removes
        # 'data' as a key of outdict.
        outdict.update(outdict.pop('data'))

        retval = dict_to_table(outdict, tablepath, kws, cols)
    except Exception, e:
        print 'Error', e, 'trying to export an ephemeris dict to', tablepath
        retval = False

    return retval
