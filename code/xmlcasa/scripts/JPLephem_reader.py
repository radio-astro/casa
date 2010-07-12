import numpy
import re
from taskinit import me, qa

def readJPLephem(fmfile):
    """
    Reads a JPL Horizons text file (ask Bryan Butler) for a solar system object
    and returns various quantities in a dictionary.  The dict will be blank
    ({}) if there is a failure.
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

    # Data ("the rows of the table")
    
    # need date, r (heliocentric distance), delta (geocentric distance), and phang (phase angle).
    # (Could use the "dot" time derivatives for Doppler shifting, but it's
    # likely unnecessary.)
    # Use named groups!
    #  Date__(UT)__HR:MN     R.A.___(ICRF/J2000.0)___DEC Ob-lon Ob-lat Sl-lon Sl-lat   NP.ang   NP.dist               r        rdot            delta      deldot    S-T-O   L_s
    #  2010-May-01 00:00     09 01 43.1966 +19 04 28.673 286.52  18.22 246.99  25.34 358.6230      3.44  1.661167637023  -0.5303431 1.28664311447968  15.7195833  37.3033   84.50
    cols = {}              # Descriptions and patterns for things that will be stored.
    cols['date'] = {'comment': 'date',
                    'pat':     r'^\s*(?P<date>\d+-\w+-\d+ \d+:\d+)'}
    radecpat        = r'\s+\d+ \d+ [0-9.]+ [-+ ]?\d+ \d+ [0-9.]+'
    lonlatpat       = r'\s+[0-9.]+\s+[-+]?[0-9.]+'
    cols['r']    = {'comment': 'heliocentric distance',
                    'unit':    'AU',
                    'pat':     r'\s+(?P<r>[0-9.]+)'}
    rdotpat         = r'\s+[-0-9.]+'
    cols['delta'] = {'comment': 'geocentric distance',
                     'unit':    'AU',
                     'pat':     r'\s+(?P<delta>[0-9.]+)'}
    deltadotpat = rdotpat
    cols['phang'] = {'comment': 'phase angle',
                     'unit':    'deg',
                     'pat':     r'\s+(?P<phang>[0-9.]+)'}
    datapat = re.compile(cols['date']['pat'] + radecpat
                         + lonlatpat             # Ob
                         + lonlatpat             # Sl
                         + lonlatpat             # NP
                         + cols['r']['pat'] + rdotpat
                         + cols['delta']['pat'] + deltadotpat
                         + cols['phang']['pat'])

    stoppat = r'^Column meaning:\s*$'  # Signifies the end of data.

    # Read fmfile into retdict.
    retdict['data'] = {}
    for col in cols:
        retdict['data'][col] = {'comment': cols[col]['comment'],
                                'data':    []}
    for line in ephem:
        if retdict.has_key('isOK'):
            matchobj = re.search(datapat, line)
            if matchobj:
                for col in matchobj.groupdict():
                    retdict['data'][col]['data'].append(matchobj.groupdict()[col])
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
    ephem.close()

    # Convert numerical strings into actual numbers.
    retdict['earliest'] = datestr_to_epoch(retdict['data']['date']['data'][0])
    retdict['latest'] = datestr_to_epoch(retdict['data']['date']['data'][-1])

    for hk in ['meanrad', 'T_mean']:
        retdict[hk] = float(retdict[hk])
    retdict['data']['date'] = datestrs_to_epochs(retdict['data']['date']['data'])
    for dk in ['r', 'delta', 'phang']:
        retdict['data'][dk]['data'] = {'unit': cols[dk]['unit'],
                                       'value': numpy.array([float(s) for s in retdict['data'][dk]['data']])}
    
    return retdict

def datestr_to_epoch(datestr):
    """
    Given a UT date like "2010-May-01 00:00", returns an epoch measure.
    """
    return me.epoch(rf='UTC', v0=qa.totime(datestr))

def datestrs_to_epochs(datestrlist):
    """
    Like datestr_to_epoch, but more so.  All of the date strings must have the
    same reference frame (i.e. UT).
    """
    timeq = {}
    # Do first conversion to get unit.
    firsttime = qa.totime(datestrlist[0])
    timeq['unit'] = firsttime['unit']
    timeq['value'] = [firsttime['value']]
    for i in xrange(1, len(datestrlist)):
        timeq['value'].append(qa.totime(datestrlist[i])['value'])

    # me.epoch doesn't take array values, so make a vector epoch measure manually.
    #return me.epoch(rf='UT', v0=timeq)
    return {'m0': {'unit': timeq['unit'],
                   'value': numpy.array(timeq['value'])},
            'refer': 'UTC',
            'type': 'epoch'}


def ephem_dict_to_table(fmdict, tablepath):
    """
    Converts a dictionary from readJPLephem() to a CASA table, and attempts to
    save it to tablepath.  Returns whether or not it was successful.
    """
    pass
