from tasks import *
from taskinit import *
from __main__ import inp
import os
import unittest

#     Unit tests of visstat 
#
# # This example shows who to run it manually from outside casapy.
# casapy -c runUnitTest test_visstat
#
# or
#
# # This example shows who to run it manually from with casapy.
# sys.path.append( os.environ["CASAPATH"].split()[0]+'/code/xmlcasa/scripts/regressions/admin' )
# import runUnitTest
# runUnitTest.main(['test_visstat'])
#

epsilon = 0.0001

fits='ngc5921.fits'
vis = 'ngc5921.ms'

class visstat_test(unittest.TestCase):
    def setUp(self):
        if(os.path.exists(fits)):
            os.system('rm -rf ' +fits)

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/ngc5921/'
        os.system('cp -r ' +datapath + fits +' ' + fits)

    def tearDown(self):
        os.system('rm -rf ' +fits)        
        os.system('rm -rf ' +vis+'*')        
    
    def test_run(self):
        '''Visstat: Test several modes'''
    
        expected = {'ngc5921.ms':
                    {'DATA': {'max': 73.75,
                              'mean': 4.8371031336376049,
                              'medabsdevmed': 0.045013353228569031,
                              'median': 0.053559452295303345,
                              'min': 2.2130521756480448e-05,
                              'npts': 2660994.0,
                              'quartile': 0.30496686697006226,
                              'rms': 17.081207275390625,
                              'stddev': 16.382008275102407,
                              'sum': 12871502.415990865,
                              'sumsq': 776391995.30866611,
                              'var': 268.37019512552371}}}
            
        print "Getting data", vis, "..."
        
        importuvfits(fits, vis)
        listobs(vis=vis)
        
        flagdata(vis=vis, antenna='17')
        flagdata(vis=vis, mode='summary')

        s = visstat(vis=vis, axis='amp', datacolumn='data')

        if s.keys() != expected[vis].keys():
            raise Exception("Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[vis], s))
                            
        # Detailed check of values, column=DATA
        print "Expected =", expected[vis]
        print "Got =", s
        if not s.has_key('DATA'):
            raise Exception("Dictionary returned from visstat does not have key DATA")
        for e in expected[vis]['DATA'].keys():
            print "Checking %s: %s vs %s" % \
                   (e, expected[vis]['DATA'][e], s['DATA'][e])
            failed = False
            if expected[vis]['DATA'][e] == 0:
                if s['DATA'][e] != 0:
                    failed = True
            else:
                if abs((expected[vis]['DATA'][e] - s['DATA'][e])/expected[vis]['DATA'][e]) > 0.0001:
                    failed = True
            if failed:
                raise Exception("Numbers differ, expected %s, got %s" % \
                      (str(expected[vis]['DATA'][e]), str(s['DATA'][e])))

        # Check of channel selection
        for ch in [1, 2, 4, 7, 13, 62]:
          for corr in ['ll', 'rr', 'll,rr']:
            print "Call with spw='0:1~"+str(ch)+"', correlation="+corr
            s = visstat(vis=vis, axis='amp', datacolumn='data', spw='0:1~'+str(ch), correlation=corr)
            print s
            n_expected = 2660994/63 * ch   
            if corr in ['ll', 'rr']:
                n_expected /= 2
            
            n = int(s['DATA']['npts'])
            print "Checking npts: %s vs %s" % (n, n_expected)
            if n != n_expected:
                raise Exception(str(n_expected) + " points expected, but npts = " + str(n))

            
        # Test running on all columns
        print "Create scratch columns. Expect error messages from applycal, that is okay, this is just to create the scratch columns"
        applycal(vis=vis)

        tb.open(vis)
        cols = tb.colnames()
        tb.close()

        cplx = ['amp', 'amplitude', 'phase', 'imag', 'imaginary', 'real']
        for x in cplx:
            cols.append(x)
        print cols
        cols.remove('DATA')
        cols.remove('CORRECTED_DATA')
        cols.remove('MODEL_DATA')
        cols.append('UVRANGE')

        cols = [x.lower() for x in cols]

        print "Trying these column names", cols

        for col in cols:
            data_cols = ['']
            if col in cplx:
                data_cols = ['data', 'corrected', 'model'] # not supported: 'residual'
                
            for dc in data_cols:
                print "Call with axis =", col, "; datacolumn =", dc
                if dc != '':
                    s = visstat(vis=vis, axis=col, datacolumn=dc)
                else:
                    s = visstat(vis=vis, axis=col)
                print "Result was", s
                if col.upper() in ["FLAG_CATEGORY", "EXPOSURE", "OBSERVATION_ID", "PROCESSOR_ID", "STATE_ID", "TIME_CENTROID"]:
                    # no support for FLAG_CATEGORY, EXPOSURE, OBSERVATION_ID, ...
                    # so expect failure
                    if s != None:
                        raise Exception("Error! " + str(s))
                elif not type(s) is dict:
                    raise Exception("Error! Return value " + str(s) + " is not a dictionary")
                else:
                    if dc == '' and \
                           not col.upper() in s.keys() and \
                           not col.upper()+'_0' in s.keys():
                        raise Exception("Missing key " + col.upper() + " in result")
                    
                    if dc != '' and not dc.upper() in s.keys():
                        raise Exception("Missing key " + dc.upper() + " in result")

        # Few tests of special cases
        for a in range(1, 5):
            s = visstat(vis=vis, axis='ANTENNA1', antenna=str(a)+'&26')
            print "antenna =", a, "; mean = ", s['ANTENNA1']['mean']

            # Note there's a counting from 0 or 1 issue here
            # with the antenna numbering
            if vis == 'ngc5921.ms':
                offset = 1
            else:
                offset = 0
            if abs((s['ANTENNA1']['mean']+offset) - a) > epsilon:
                raise Exception("Error!")

        for scan in range(1, 8):
            s = visstat(vis=vis, axis='SCAN_NUMBER', scan=str(scan))
            
            print "scan =", scan, "; mean = ", s['SCAN_NUMBER']['mean']
            if abs(s['SCAN_NUMBER']['mean'] - scan) > epsilon:
                raise Exception("Error!")

        # Test that flagging impacts statistics
        s = visstat(vis=vis, axis='scan_number', scan='>=1')
        print "min = ", s['SCAN_NUMBER']['min']
        if abs(s['SCAN_NUMBER']['min'] - 1) > epsilon:
            raise Exception("Error")

        flagdata(vis=vis, scan="1")
        s = visstat(vis=vis, axis='scan_number', scan='>=1')
        print "min = ", s['SCAN_NUMBER']['min']
        if abs(s['SCAN_NUMBER']['min'] - 2) > epsilon:
            raise Exception("Error")

        flagdata(vis=vis, scan="2")
        s = visstat(vis=vis, axis='scan_number', scan='>=1')
        print "min = ", s['SCAN_NUMBER']['min']
        if abs(s['SCAN_NUMBER']['min'] - 3) > epsilon:
            raise Exception("Error")

        s = visstat(vis=vis, axis='scan_number', useflags=False, scan='>=1')
        print "min = ", s['SCAN_NUMBER']['min']
        if abs(s['SCAN_NUMBER']['min'] - 1) > epsilon:
            raise Exception("Error")
    

def suite():
    return [visstat_test]
    
    
