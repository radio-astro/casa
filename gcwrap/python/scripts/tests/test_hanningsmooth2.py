import os
import sys
import shutil
from __main__ import default
from tasks import hanningsmooth2, mstransform, partition, cvel, split2, clearcal
from taskinit import mstool
import exceptions
import testhelper as th
from parallel.parallel_data_helper import ParallelDataHelper
import unittest

'''
functional tests for task hanningsmooth2
'''

# Path for data
datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/hanningsmooth/'

# Pick up alternative data directory to run tests on MMSs
testmms = False
if os.environ.has_key('TEST_DATADIR'):   
    DATADIR = str(os.environ.get('TEST_DATADIR'))+'/hanningsmooth/'
    if os.path.isdir(DATADIR):
        testmms = True
        datapath = DATADIR

print 'hanningsmooth2 tests will use data from '+datapath         

class test_base(unittest.TestCase):

    def setUp_ngc5921(self):
        # only DATA column
        self.msfile = 'ngc5921_ut.ms'
        if testmms:
            self.msfile = 'ngc5921_ut.mms'
            
        if (not os.path.exists(self.msfile)):
            shutil.copytree(datapath+self.msfile, self.msfile)
            
        default(hanningsmooth2)
    
    def setUp_almams(self):
        # MS with DATA and CORRECTED_DATA
        self.msfile = 'ALMA-data-mst-science-testing-CAS-5013-one-baseline-one-timestamp.ms'
        if testmms:
            self.msfile = 'ALMA-data-mst-science-testing-CAS-5013-one-baseline-one-timestamp.mms'
            
        if (not os.path.exists(self.msfile)):
            shutil.copytree(datapath+self.msfile, self.msfile)
            
        default(hanningsmooth2)

    def createMMS(self, msfile, column='data', axis='auto',scans='',spws=''):
        '''Create MMSs for tests with input MMS'''
        prefix = msfile.rstrip('.ms')
        if not os.path.exists(msfile):
            os.system('cp -RL '+datapath + msfile +' '+ msfile)
        
        # Create an MMS for the tests
        self.testmms = prefix + ".test.mms"
        default(mstransform)
        
        if os.path.exists(self.testmms):
            os.system("rm -rf " + self.testmms)
            
        print "................. Creating test MMS .................."
        partition(vis=msfile, outputvis=self.testmms, datacolumn=column,
                    createmms=True,separationaxis=axis, scan=scans, spw=spws)


class hanningsmooth2_test1(test_base):
    
    def setUp(self):
        self.setUp_ngc5921()

    def tearDown(self):
        if (os.path.exists(self.outputms)):
            shutil.rmtree(self.outputms,ignore_errors=True)        
        
    def test1(self):
        """hanningsmooth2 - Test 1: Wrong input MS should raise an exception"""
        msfile = 'badmsfile'
        self.outputms = 'none.ms'
        try:
            hanningsmooth2(vis=msfile)
        except exceptions.RuntimeError, instance:
            print 'Expected error: %s'%instance
        
    def test2(self):
        '''hanningsmooth2 - Test 2: Check that output MS is created'''
        self.outputms = 'hann2.ms'
        hanningsmooth2(vis=self.msfile, outputvis=self.outputms, datacolumn='corrected')
        # Smoothed data should be saved in DATA column of outupt MS
        self.assertTrue(os.path.exists(self.outputms))
                
    def test3(self):
        '''hanningsmooth2 - Test 3: Check theoretical and calculated values on non-existing CORRECTED column'''
        self.outputms = 'hann3.ms'

      # check correct flagging (just for one row as a sample)
        flag_col = th.getVarCol(self.msfile, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])

        # It should fall-back and use the input DATA column
        hanningsmooth2(vis=self.msfile, outputvis=self.outputms, datacolumn='corrected')

      # check correct flagging (just for one row as a sample)
        flag_col = th.getVarCol(self.outputms, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [True])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [True])

        data_col = th.getVarCol(self.msfile, 'DATA')
        corr_col = th.getVarCol(self.outputms, 'DATA')
        nrows = len(corr_col)
        
      # Loop over every 2nd row,pol and get the data for each channel
        max = 1e-05
        for i in range(1,nrows,2) :
            row = 'r%s'%i            
            # polarization is 0-1
            for pol in range(0,2) :
                # array's channels is 0-63
                for chan in range(1,62) :
                    # channels must start from second and end before the last
                    data = data_col[row][pol][chan]
                    dataB = data_col[row][pol][chan-1]
                    dataA = data_col[row][pol][chan+1]
        
                    Smoothed = th.calculateHanning(dataB,data,dataA)
                    CorData = corr_col[row][pol][chan]
                    
                    # Check the difference
                    self.assertTrue(abs(CorData-Smoothed) < max )

    def test4(self):
        '''hanningsmooth2 - Test 4: Theoretical and calculated values should be the same for MMS-case'''
	
        # Split the input to decrease the running time
        split2(self.msfile, outputvis='splithan.ms',scan='1,2',datacolumn='data')
        self.msfile = 'splithan.ms'
        
        # create a test MMS. It creates self.testmms
        self.createMMS(self.msfile)
        self.outputms = 'hann4.mms'
        
      # check correct flagging (just for one row as a sample)
        mslocal = mstool()
        mslocal.open(self.msfile)
        mslocal.sort('sorted.ms',['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME'])
        mslocal.close()
        self.msfile = 'sorted.ms'
        flag_col = th.getVarCol(self.msfile, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])
        
        data_col = th.getVarCol(self.msfile, 'DATA')        
        hanningsmooth2(vis=self.testmms, outputvis=self.outputms, datacolumn='data', keepmms=True)
        self.assertTrue(ParallelDataHelper.isParallelMS(self.outputms), 'Output should be an MMS')

      # Sort the MMS
        mslocal.open(self.outputms)
        mslocal.sort('sorted.mms',['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME'])
        mslocal.close()
        self.outputms = 'sorted.mms'
        
        corr_col = th.getVarCol(self.outputms, 'DATA')
        nrows = len(corr_col)

      # check correct flagging (just for one row as a sample)
        flag_col = th.getVarCol(self.outputms, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [True])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [True])
        
      # Loop over every 2nd row,pol and get the data for each channel
        max = 1e-05
        for i in range(1,nrows,2) :
            row = 'r%s'%i            
            # polarization is 0-1
            for pol in range(0,2) :
                # array's channels is 0-63
                for chan in range(1,62) :
                    # channels must start from second and end before the last
                    data = data_col[row][pol][chan]
                    dataB = data_col[row][pol][chan-1]
                    dataA = data_col[row][pol][chan+1]
        
                    Smoothed = th.calculateHanning(dataB,data,dataA)
                    CorData = corr_col[row][pol][chan]
                    
                    # Check the difference
                    self.assertTrue(abs(CorData-Smoothed) < max )

    def test6(self):
        '''hanningsmooth2 - Test 6: Flagging should be correct with datacolumn==ALL'''
        self.outputms = 'hann6.ms'
        
      # check correct flagging (just for one row as a sample)
        flag_col = th.getVarCol(self.msfile, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])

        hanningsmooth2(vis=self.msfile,outputvis=self.outputms, datacolumn='all')

      # check correct flagging (just for one row as a sample)
        flag_col = th.getVarCol(self.outputms, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [True])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [True])

    def test7(self):
        '''hanningsmooth2 - Test 7: Flagging should be correct when hanning smoothing within cvel (no transform)'''
        self.outputms = 'cvelngc.ms'
        clearcal(vis=self.msfile)
        
      # check correct flagging (just for one row as a sample)
        flag_col = th.getVarCol(self.msfile, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])

        cvel(vis=self.msfile, outputvis=self.outputms, hanning=True)

      # check correct flagging (just for one row as a sample)
        flag_col = th.getVarCol(self.outputms, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [True])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [True])

    def test8(self):
        '''hanningsmooth2 - Test 8: Flagging should be correct when hanning smoothing within mstransform (with regrid)'''
        self.outputms = 'cvelngc.ms'
        
      # check correct flagging (just for one row as a sample)
        flag_col = th.getVarCol(self.msfile, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])

        # CAS-4114 cvel doesn't support MMS. Compare with mstransform instead.
#        cvel(vis=self.msfile, outputvis=self.outputms, hanning=True, outframe='cmb')
        mstransform(vis=self.msfile, outputvis=self.outputms, datacolumn='data',
                    hanning=True, regridms=True, outframe='cmb')

        # check correct flagging (just for one row as a sample)
        flag_col = th.getVarCol(self.outputms, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [True])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][2] == [False])
        self.assertTrue(flag_col['r1'][0][60] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [True])
        self.assertTrue(flag_col['r1'][0][62] == [True])


class hanningsmooth2_test2(test_base):
    
    def setUp(self):
        self.setUp_almams()

    def tearDown(self):
        if (os.path.exists(self.outputms)):
            shutil.rmtree(self.outputms,ignore_errors=True)       
             
    def test_default_cols(self):
        '''hanningsmooth2: Default datacolumn=all and MMS output'''
        
        self.createMMS(self.msfile,column='all')
        self.outputms = 'hannall.ms'

        hanningsmooth2(vis=self.testmms, outputvis=self.outputms)
        self.assertTrue(ParallelDataHelper.isParallelMS(self.outputms), 'Output should be an MMS')
        
        # Should have all scratch columns in output
        cd = th.getColDesc(self.outputms, 'DATA')
        self.assertGreater(len(cd), 0, 'DATA column does not exist')
        cc = th.getColDesc(self.outputms, 'CORRECTED_DATA')
        self.assertGreater(len(cc), 0, 'CORRECTED_DATA does not exist')
        
        # Now repeat the above steps but create an output MS by setting keepmms=False
        os.system('rm -rf '+self.outputms)
        hanningsmooth2(vis=self.testmms, outputvis=self.outputms, keepmms=False)
        self.assertFalse(ParallelDataHelper.isParallelMS(self.outputms), 'Output should be a normal MS')
        
        # Should have all scratch columns in output
        cd = th.getColDesc(self.outputms, 'DATA')
        self.assertGreater(len(cd), 0, 'DATA column does not exist')
        cc = th.getColDesc(self.outputms, 'CORRECTED_DATA')
        self.assertGreater(len(cc), 0, 'CORRECTED_DATA does not exist')

    def test_corrected_col(self):
        '''hanningsmooth2: Apply smoothing in CORRECTED column'''
        self.outputms = 'hanncorr.ms'

      # check correct flagging before (just for one row as a sample)
        flag_col = th.getVarCol(self.msfile, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][3838] == [False])
        self.assertTrue(flag_col['r1'][0][3839] == [False])
        
        # input column
        data_col = th.getVarCol(self.msfile, 'CORRECTED_DATA') 
               
        hanningsmooth2(vis=self.msfile, outputvis=self.outputms, datacolumn='corrected')
        
        # output smoothed column
        corr_col = th.getVarCol(self.outputms, 'DATA')
        nrows = len(corr_col)
        
      # check correct flagging after (just for one row as a sample)
        flag_col = th.getVarCol(self.outputms, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [True])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][3838] == [False])
        self.assertTrue(flag_col['r1'][0][3839] == [True])

      # Loop over every 2nd row,pol and get the data for each channel
        max = 1e-04
        for i in range(1,nrows,2) :
            row = 'r%s'%i            
            # polarization is 0-1
            for pol in range(0,2) :
                # array's channels is 0-3840
                for chan in range(1,3839) :
                    # channels must start from second and end before the last
                    data = data_col[row][pol][chan]
                    dataB = data_col[row][pol][chan-1]
                    dataA = data_col[row][pol][chan+1]
        
                    Smoothed = th.calculateHanning(dataB,data,dataA)
                    CorData = corr_col[row][pol][chan]
                    
                    # Check the difference
                    self.assertTrue(abs(CorData-Smoothed) < max, 
                                    'CorData=%s Smoothed=%s in row=%s pol=%s chan=%s'%(CorData,Smoothed,row,pol,chan))


class Cleanup(test_base):

    def tearDown(self):
        shutil.rmtree('ngc5921_ut.ms', ignore_errors=True)
        shutil.rmtree('ALMA-data-mst-science-testing-CAS-5013-one-baseline-one-timestamp.ms', ignore_errors=True)
        
    def test_runTest(self):
        '''hanningsmooth2: Cleanup'''
        pass
            
def suite():
    return [hanningsmooth2_test1,hanningsmooth2_test2,Cleanup]


