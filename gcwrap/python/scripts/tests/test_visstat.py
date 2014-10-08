from tasks import *
from taskinit import *
from __main__ import inp
from __main__ import default
import os
import shutil
import unittest

#     Functional tests of visstat 

epsilon = 0.0001

#fits='ngc5921.fits'
#msfile = 'ngc5921.ms'

# Path for data
datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/visstat/"

# Pick up alternative data directory to run tests on MMSs
testmms = False
if os.environ.has_key('TEST_DATADIR'):   
    DATADIR = str(os.environ.get('TEST_DATADIR'))+'/visstat/'
    if os.path.isdir(DATADIR):
        testmms = True
        datapath = DATADIR

print 'visstat tests will use data from '+datapath         

class visstat_test(unittest.TestCase):
    def setUp(self):
        # MS in repository was flagged with the following command:
        # flagdata(vis=self.msfile, antenna='17', flagbackup=False)
        self.msfile = "ngc5921.ms"
        if testmms:
            self.msfile = "ngc5921.mms"
        if(not os.path.exists(self.msfile)):
            shutil.copytree(datapath+self.msfile, self.msfile)
            
        default('visstat')

    def tearDown(self):
        shutil.rmtree(self.msfile)

    def test1(self):
        '''Visstat 1: Default values'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
        expected = {self.msfile:
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
            
        
        s = visstat(vis=self.msfile, axis='amp', datacolumn='data')

        if s.keys() != expected[self.msfile].keys():
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[self.msfile], s)
#            raise Exception("Wrong dictionary keys. Expected %s, got %s" % \
#                            (expected[self.msfile], s))
                            
        # Detailed check of values, column=DATA
        print "Expected =", expected[self.msfile]
        print "Got =", s
        if not s.has_key('DATA'):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Dictionary returned from visstat does not have key DATA"
#            raise Exception("Dictionary returned from visstat does not have key DATA")
        for e in expected[self.msfile]['DATA'].keys():
            print "Checking %s: %s vs %s" % \
                   (e, expected[self.msfile]['DATA'][e], s['DATA'][e])
            failed = False
            if expected[self.msfile]['DATA'][e] == 0:
                if s['DATA'][e] != 0:
                    failed = True
            else:
                if abs((expected[self.msfile]['DATA'][e] - s['DATA'][e])/expected[self.msfile]['DATA'][e]) > 0.0001:
                    failed = True
            if failed:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Numbers differ, expected %s, got %s" % \
                      (str(expected[self.msfile]['DATA'][e]), str(s['DATA'][e]))
#                raise Exception("Numbers differ, expected %s, got %s" % \
#                      (str(expected[self.msfile]['DATA'][e]), str(s['DATA'][e])))

        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def test2(self):     
        '''Visstat 2: Check channel selections'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
        for ch in [1, 2, 4, 7, 13, 62]:
          for corr in ['ll', 'rr', 'll,rr']:
            print "Call with spw='0:1~"+str(ch)+"', correlation="+corr
            s = visstat(vis=self.msfile, axis='amp', datacolumn='data', spw='0:1~'+str(ch), correlation=corr)
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
        print "Create scratch columns."
        if testmms:
            clearcal(self.msfile)
        else:
            cblocal = cbtool()
            cblocal.open(self.msfile)
            cblocal.close()

        tblocal = tbtool()
        tblocal.open(self.msfile)
        cols = tblocal.colnames()
        tblocal.close()

        cplx = ['amp', 'amplitude', 'phase', 'imag', 'imaginary', 'real']
        for x in cplx:
            cols.append(x)
        print cols
        cols.remove('DATA')
        have_corr = False
	if('CORRECTED_DATA' in cols):
            cols.remove('CORRECTED_DATA')
            have_corr = True
        have_model = False
        if('MODEL_DATA' in cols):
            cols.remove('MODEL_DATA')
            have_model = True
        cols.append('UVRANGE')

        cols = [x.lower() for x in cols]

        print "Trying these column names", cols

        for col in cols:
            data_cols = ['']
	    print 'col ',col
	    print 'cplx ',cplx
            if col in cplx:
                data_cols = ['data'] # not supported: 'residual'
                if have_corr:
                    data_cols.append('corrected')
                if have_model:
                    data_cols.append('model')
                
	    print data_cols
            for dc in data_cols:
                print "Call with axis =", col, "; datacolumn =", dc
                if dc != '':
                    s = visstat(vis=self.msfile, axis=col, datacolumn=dc)
                else:
                    s = visstat(vis=self.msfile, axis=col)
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

        print 'retValue[\'success\'] ', retValue['success']
        print 'retValue[\'error_msgs\'] ', retValue['error_msgs']
        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def test4(self):
        '''Visstat 4: Test of special cases'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
        for a in range(1, 5):
            s = visstat(vis=self.msfile, axis='ANTENNA1', antenna=str(a)+'&26')
            print "antenna =", a, "; mean = ", s['ANTENNA1']['mean']

            # Note there's a counting from 0 or 1 issue here
            # with the antenna numbering
            if self.msfile == self.msfile:
                offset = 1
            else:
                offset = 0
            if abs((s['ANTENNA1']['mean']+offset) - a) > epsilon:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: Failed when antenna= "+str(a)+'&26'
#                raise Exception("Error!")

        for scan in range(1, 8):
            s = visstat(vis=self.msfile, axis='SCAN_NUMBER', scan=str(scan))
            
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

        s = visstat(vis=self.msfile, axis='scan_number', scan='>=1')
        print "min = ", s['SCAN_NUMBER']['min']
        if abs(s['SCAN_NUMBER']['min'] - 1) > epsilon:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: Failed with visstat(vis=self.msfile, axis='scan_number', scan='>=1') "
#            raise Exception("Error")

        flagdata(vis=self.msfile, scan="1", flagbackup=False)
        s = visstat(vis=self.msfile, axis='scan_number', scan='>=1')
        print "min = ", s['SCAN_NUMBER']['min']
        if abs(s['SCAN_NUMBER']['min'] - 2) > epsilon:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: Failed with visstat(vis=msfile, axis='scan_number', scan='>=1') "\
                +"when data is flagged as flagdata(vis=msfile, scan='1')"
#            raise Exception("Error")

        flagdata(vis=self.msfile, scan="2", flagbackup=False)
        s = visstat(vis=self.msfile, axis='scan_number', scan='>=1')
        print "min = ", s['SCAN_NUMBER']['min']
        if abs(s['SCAN_NUMBER']['min'] - 3) > epsilon:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: Failed with visstat(vis=msfile, axis='scan_number', scan='>=1') "\
                +"when data is flagged as flagdata(vis=msfile, scan='2')"
#            raise Exception("Error")

        s = visstat(vis=self.msfile, axis='scan_number', useflags=False, scan='>=1')
        print "min = ", s['SCAN_NUMBER']['min']
        if abs(s['SCAN_NUMBER']['min'] - 1) > epsilon:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: Failed with visstat(vis=msfile, axis='scan_number', useflags=False, scan='>=1')"
#            raise Exception("Error")

        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def test6(self):
        '''Visstat 6: Test when all selected rows are flagged'''
        flagdata(vis=self.msfile,mode='manual',antenna='1;1&&1', flagbackup=False)
        res = visstat(vis=self.msfile,antenna='1',useflags=True)
        self.assertFalse(res, 'All data are flagged. An exception should have been raised')

class visstat_cleanup(unittest.TestCase):

    def setUp(self):
        pass
    
    def tearDown(self):
        # It will ignore errors in case the files don't exist
        shutil.rmtree(self.msfile,ignore_errors=True)
        
    def test1a(self):
        '''Visstat: Cleanup'''
        pass

def suite():
    return [visstat_test]
    
    
