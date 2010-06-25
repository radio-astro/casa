from tasks import *
from taskinit import *
from __main__ import inp
from __main__ import default
import os
import shutil
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
msfile = 'ngc5921.ms'

class visstat_test(unittest.TestCase):
    def setUp(self):
        if(not os.path.exists(msfile)):
            datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/visstat/'
            shutil.copytree(datapath+msfile, msfile)
            # flag the data once and use it in every test
            flagdata(vis=msfile, antenna='17')            
            
        default('visstat')

    def tearDown(self):
        shutil.rmtree(msfile)
        pass

    def test1(self):
        '''Visstat 1: Default values'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
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
            
        
#        flagdata(vis=msfile, antenna='17')
#        flagdata(vis=msfile, mode='summary')

        s = visstat(vis=msfile, axis='amp', datacolumn='data')

        if s.keys() != expected[msfile].keys():
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[msfile], s)
#            raise Exception("Wrong dictionary keys. Expected %s, got %s" % \
#                            (expected[msfile], s))
                            
        # Detailed check of values, column=DATA
        print "Expected =", expected[msfile]
        print "Got =", s
        if not s.has_key('DATA'):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Dictionary returned from visstat does not have key DATA"
#            raise Exception("Dictionary returned from visstat does not have key DATA")
        for e in expected[msfile]['DATA'].keys():
            print "Checking %s: %s vs %s" % \
                   (e, expected[msfile]['DATA'][e], s['DATA'][e])
            failed = False
            if expected[msfile]['DATA'][e] == 0:
                if s['DATA'][e] != 0:
                    failed = True
            else:
                if abs((expected[msfile]['DATA'][e] - s['DATA'][e])/expected[msfile]['DATA'][e]) > 0.0001:
                    failed = True
            if failed:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Numbers differ, expected %s, got %s" % \
                      (str(expected[msfile]['DATA'][e]), str(s['DATA'][e]))
#                raise Exception("Numbers differ, expected %s, got %s" % \
#                      (str(expected[msfile]['DATA'][e]), str(s['DATA'][e])))

        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def test2(self):     
        '''Visstat 2: Check channel selections'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
#        flagdata(vis=msfile, antenna='17')
        for ch in [1, 2, 4, 7, 13, 62]:
          for corr in ['ll', 'rr', 'll,rr']:
            print "Call with spw='0:1~"+str(ch)+"', correlation="+corr
            s = visstat(vis=msfile, axis='amp', datacolumn='data', spw='0:1~'+str(ch), correlation=corr)
            print s
            n_expected = 2660994/63 * ch   
            if corr in ['ll', 'rr']:
                n_expected /= 2
            
            n = int(s['DATA']['npts'])
            print "Checking npts: %s vs %s" % (n, n_expected)
            if n != n_expected:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError:"+str(n_expected) + " points expected, but npts = " + str(n)
#                raise Exception(str(n_expected) + " points expected, but npts = " + str(n))

        self.assertTrue(retValue['success'],retValue['error_msgs'])        

    def test3(self):
        '''Visstat 3: Test on different columns and axis'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
#        flagdata(vis=msfile, antenna='17')
        print "Create scratch columns. Expect and ignore error messages from applycal"
        applycal(vis=msfile)

        tb.open(msfile)
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
                    s = visstat(vis=msfile, axis=col, datacolumn=dc)
                else:
                    s = visstat(vis=msfile, axis=col)
                print "Result was", s
                if col.upper() in ["FLAG_CATEGORY", "EXPOSURE", "OBSERVATION_ID", "PROCESSOR_ID", "STATE_ID", "TIME_CENTROID"]:
                    # no support for FLAG_CATEGORY, EXPOSURE, OBSERVATION_ID, ...
                    # so expect failure
                    if s != None:
                        retValue['success']=False
                        retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: "+str(s)
#                        raise Exception("Error! " + str(s))
                elif not type(s) is dict:
                        retValue['success']=False
                        retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: Return value " + str(s) + " is not a dictionary"
#                    raise Exception("Error! Return value " + str(s) + " is not a dictionary")
                else:
                    if dc == '' and \
                            not col.upper() in s.keys() and \
                            not col.upper()+'_0' in s.keys():
                        retValue['success']=False
                        retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: Missing key " + col.upper() + " in result"
#                        raise Exception("Missing key " + col.upper() + " in result")
                    
                    if dc != '' and not dc.upper() in s.keys():
                        retValue['success']=False
                        retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: Missing key " + dc.upper() + " in result"
#                        raise Exception("Missing key " + dc.upper() + " in result")

        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def test4(self):
        '''Visstat 4: Test of special cases'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
#        flagdata(vis=msfile, antenna='17')
        for a in range(1, 5):
            s = visstat(vis=msfile, axis='ANTENNA1', antenna=str(a)+'&26')
            print "antenna =", a, "; mean = ", s['ANTENNA1']['mean']

            # Note there's a counting from 0 or 1 issue here
            # with the antenna numbering
            if msfile == 'ngc5921.ms':
                offset = 1
            else:
                offset = 0
            if abs((s['ANTENNA1']['mean']+offset) - a) > epsilon:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: Failed when antenna= "+str(a)+'&26'
#                raise Exception("Error!")

        for scan in range(1, 8):
            s = visstat(vis=msfile, axis='SCAN_NUMBER', scan=str(scan))
            
            print "scan =", scan, "; mean = ", s['SCAN_NUMBER']['mean']
            if abs(s['SCAN_NUMBER']['mean'] - scan) > epsilon:        
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: Failed for scan = "+str(scan)
#                raise Exception("Error!")

        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def test5(self):
        '''Visstat 5: Test that flagging impact statistics'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            

#        flagdata(vis=msfile, antenna='17')
        s = visstat(vis=msfile, axis='scan_number', scan='>=1')
        print "min = ", s['SCAN_NUMBER']['min']
        if abs(s['SCAN_NUMBER']['min'] - 1) > epsilon:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: Failed with visstat(vis=msfile, axis='scan_number', scan='>=1') "
#            raise Exception("Error")

        flagdata(vis=msfile, scan="1")
        s = visstat(vis=msfile, axis='scan_number', scan='>=1')
        print "min = ", s['SCAN_NUMBER']['min']
        if abs(s['SCAN_NUMBER']['min'] - 2) > epsilon:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: Failed with visstat(vis=msfile, axis='scan_number', scan='>=1') "\
                +"when data is flagged as flagdata(vis=msfile, scan='1')"
#            raise Exception("Error")

        flagdata(vis=msfile, scan="2")
        s = visstat(vis=msfile, axis='scan_number', scan='>=1')
        print "min = ", s['SCAN_NUMBER']['min']
        if abs(s['SCAN_NUMBER']['min'] - 3) > epsilon:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: Failed with visstat(vis=msfile, axis='scan_number', scan='>=1') "\
                +"when data is flagged as flagdata(vis=msfile, scan='2')"
#            raise Exception("Error")

        s = visstat(vis=msfile, axis='scan_number', useflags=False, scan='>=1')
        print "min = ", s['SCAN_NUMBER']['min']
        if abs(s['SCAN_NUMBER']['min'] - 1) > epsilon:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: Failed with visstat(vis=msfile, axis='scan_number', useflags=False, scan='>=1')"
#            raise Exception("Error")

        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def test6(self):
        '''Visstat 6: Test when all selected rows are flagged'''
        flagdata(vis=msfile,mode='manualflag',antenna='1;1&&1')
        res = visstat(vis=msfile,antenna='1',useflags=True)
        self.assertFalse(res, 'All data are flagged. An exception should have been raised')

class visstat_cleanup(unittest.TestCase):

    def setUp(self):
        pass
    
    def tearDown(self):
        # It will ignore errors in case the files don't exist
        shutil.rmtree(msfile,ignore_errors=True)
        
    def test1a(self):
        '''Visstat: Cleanup'''
        pass

def suite():
    return [visstat_test]
    
    
