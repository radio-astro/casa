import re
from taskinit import *

def readJPLephem(fmfile):
    """
    Reads a JPL Horizons text file (ask Bryan Butler) for a solar system object
    and returns various quantities in a dictionary.  The dict will be blank
    ({}) if there is a failure.
    """
    retdict = {}
    casalog.origin('readJPLephem')
    try:
        ephem = open(fmfile)
    except IOError:
        casalog.post("Could not open ephemeris file " + fmfile,
                     priority="SEVERE")
        return {}
    else:
        fmlines = ephem.readlines()
        ephem.close()

    # Dictionary of quantity label: regexp pattern pairs that will be searched
    # for once.  The matching quantity will go in retdict[label].  Only a
    # single quantity (group) will be retrieved per line.
    headers = {
        'body': r'^Target body name:\s+(\w+)',          # object name
        'ephtype': r'\?s_type=1#top>\]\s*:\s+\*(\w+)', # e.g. OBSERVER
        'obsloc': r'^Center-site name:\s+(\w+)',        # e.g. GEOCENTRIC
        'meanrad': r'Mean radius \(km\)\s*=\s*([0-9.]+)',
        'T_mean': r'Mean Temperature \(K\)\s*=\s*([0-9.]+)',
        # Check data format
        'isOK': r'^\s*(Date__\(UT\)__HR:MN\s+R.A._+\([^_]+\)_+DEC\s+Ob-lon\s+Ob-lat\s+Sl-lon\s+Sl-lat\s+NP.ang\s+NP.dist\s+r\s+rdot\s+delta\s+deldot\s+S-T-O)',      
        }
    for hk in headers:
        headers[hk] = re.compile(headers[hk])

    # need date, r (heliocentric distance), and delta (geocentric distance).
    # (Could use the "dot" time derivatives for Doppler shifting, but it's
    # likely unnecessary.)
    # Use named groups!
    #  Date__(UT)__HR:MN     R.A.___(ICRF/J2000.0)___DEC Ob-lon Ob-lat Sl-lon Sl-lat   NP.ang   NP.dist               r        rdot            delta      deldot    S-T-O   L_s
    #  2010-May-01 00:00     09 01 43.1966 +19 04 28.673 286.52  18.22 246.99  25.34 358.6230      3.44  1.661167637023  -0.5303431 1.28664311447968  15.7195833  37.3033   84.50
    datepat   = r'^\s*(?P<date>\d+-\w+-\d+ \d+:\d+)'
    radecpat  = r'\s+\d+ \d+ [0-9.]+ [-+ ]?\d+ \d+ [0-9.]+'
    lonlatpat = r'\s+[0-9.]+\s+[-+]?[0-9.]+'
    rpat      = r'\s+(?P<r>[0-9.]+)'
    rdotpat   = r'\s+[-0-9.]+'
    deltapat  = r'\s+(?P<delta>[0-9.]+)'
    deltadotpat = rdotpat
    phangpat  = r'\s+(?P<phang>[0-9.]+)'
    datapat = re.compile(datepat + radecpat
                         + lonlatpat             # Ob
                         + lonlatpat             # Sl
                         + lonlatpat             # NP
                         + rpat + rdotpat
                         + deltapat + deltadotpat
                         + phangpat)
    
    stoppat = r'^Column meaning:\s*$'

    retdict['data'] = []
    for line in fmlines:
        if retdict.has_key('isOK'):
            matchobj = re.search(datapat, line)
            if matchobj:
                retdict['data'].append(matchobj.groupdict())
            elif re.match(stoppat, line):
                break
        else:
            #print "line =", line
            #print "looking for", 
            for hk in headers:
                if not retdict.has_key(hk):
                    #print hk,
                    matchobj = re.search(headers[hk], line)
                    if matchobj:
                        retdict[hk] = matchobj.group(1) # 0 is the whole line
                        #print "found", retdict[hk]
                        break
    return retdict

