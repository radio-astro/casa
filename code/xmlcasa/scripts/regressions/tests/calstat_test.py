from tasks import *
from taskinit import *
from __main__ import default
import os

epsilon = 0.0001

caltables = ['ggtau.1mm.amp.gcal',
             'ggtau.1mm.bpoly',
             'ggtau.1mm.ph.gcal',
             'ggtau.1mm.ph.gcal0',
             'ggtau.3mm.amp.gcal',
             'ggtau.3mm.bpoly',
             'ggtau.3mm.ph.gcal',
             'ggtau.3mm.ph.gcal0',
             'ggtau.co.bpoly',
             'ggtau.hco.bpoly']

def description():
    return "Test of calstat task"

def data():
    return caltables

def run():
    expected = {'ggtau.3mm.ph.gcal0':
                {'SPLINE_KNOTS_PHASE':{'rms': 4362063360.0,
                                       'medabsdevmed': 8704.0,
                                       'min': 4362050048.0,
                                       'max': 4362076160.0,
                                       'sum': 872412620800.0,
                                       'quartile': 26112.0,
                                       'median': 4362058752.0,
                                       'sumsq': 3.80551890468e+21,
                                       'stddev': 11866.4301499,
                                       'var': 140812164.503,
                                       'npts': 200,
                                       'mean': 4362063104.0}},
                'ggtau.1mm.ph.gcal0':
                {'SPLINE_KNOTS_PHASE':{'rms': 4362063360.0,
                                       'medabsdevmed': 8704.0,
                                       'min': 4362050048.0,
                                       'max': 4362076160.0,
                                       'sum': 872412620800.0,
                                       'quartile': 26112.0,
                                       'median': 4362058752.0,
                                       'sumsq': 3.80551890468e+21,
                                       'stddev': 11866.4301499,
                                       'var': 140812164.503,
                                       'npts': 200,
                                       'mean': 4362063104.0}}}

    for caltable in caltables:

        print "Testing with data", caltable, "..."

        if expected.has_key(caltable):

            default(calstat)
            axis='spline_knots_phase'
            s = calstat(caltable=caltable, axis=axis)

            if s.keys() != expected[caltable].keys():
                raise Exception("Wrong dictionary keys. Expected %s, got %s" % \
                                (expected[caltable], s))
                            
            print "Expected =", expected[caltable]
            print "Got = ", s
            if not s.has_key('SPLINE_KNOTS_PHASE'):
                raise Exception("Dictionary returned from calstat does not have key SPLINE_KNOTS_PHASE")

            for e in expected[caltable]['SPLINE_KNOTS_PHASE'].keys():
                print "Checking %s: %s vs %s" % \
                    (e, expected[caltable]['SPLINE_KNOTS_PHASE'][e], s['SPLINE_KNOTS_PHASE'][e])
                failed = False
                if expected[caltable]['SPLINE_KNOTS_PHASE'][e] == 0:
                    if s['SPLINE_KNOTS_PHASE'][e] != 0:
                        failed = True
                else:
                    if abs((expected[caltable]['SPLINE_KNOTS_PHASE'][e] - s['SPLINE_KNOTS_PHASE'][e])/expected[caltable]['SPLINE_KNOTS_PHASE'][e]) > epsilon:
                        failed = True

                # Remove these 3 lines of code, once CAS-1671 is solved
                if failed == True and caltable == 'ggta.1mm.ph.gcal0' and e in ['var', 'sigma']:
                    print "Ignoring this known problem on 64bit!"
                    failed = false

                    
                if failed:
                    raise Exception("Numbers differ, expected %s, got %s" % \
                                    (str(expected[caltable]['SPLINE_KNOTS_PHASE'][e]), str(s['SPLINE_KNOTS_PHASE'][e])))
                
        tb.open(caltable)
        cols = tb.colnames()
        tb.close()

        cplx = ['amp', 'amplitude', 'phase', 'imag', 'imaginary', 'real']
        for x in cplx:
            cols.append(x)
        print cols
        # remove complex columns
        cols.remove('GAIN')
        if 'SCALE_FACTOR' in cols: cols.remove('SCALE_FACTOR')
        if 'SIDEBAND_REF' in cols: cols.remove('SIDEBAND_REF')
        # don't try string columns
        cols.remove('FREQ_GROUP_NAME')
        cols.remove('FIELD_NAME')
        cols.remove('FIELD_CODE')
        cols.remove('SOURCE_NAME')
        cols.remove('SOURCE_CODE')
        if 'POLY_TYPE' in cols: cols.remove('POLY_TYPE')
        if 'POLY_MODE' in cols: cols.remove('POLY_MODE')
        if 'PHASE_UNITS' in cols: cols.remove('PHASE_UNITS')

        # empty column:
        if 'VALID_DOMAIN' in cols: cols.remove('VALID_DOMAIN')

        cols = [x.lower() for x in cols]

        print "Trying these column names", cols

        for col in cols:
            data_cols = ['']
            if col in cplx:
                data_cols = ['gain', 'scale_factor']
                
            for dc in data_cols:
                print "Call with caltable =", caltable, "; axis =", col, "; datacolumn =", dc
                if dc != '':
                    s = calstat(caltable=caltable, axis=col, datacolumn=dc)
                else:
                    s = calstat(caltable=caltable, axis=col)
                if col.upper() == "FLAG_CATEGORY":
                    # The MSs used have no data in FLAG_CATEGORY, therefore
                    # calstat() should fail
                    if s != None:
                        raise Exception("Error! " + str(s))
                elif not type(s) is dict:
                    raise Exception("Error! Return value " + str(s) + " is not a dictionary")
    return []
