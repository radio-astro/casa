import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for task hanningsmooth. It tests the following parameters:
    vis:   wrong and correct values
    vis:   check that output column is created
    
    Other tests: check that theoretical and calculated values of column CORRECTED_DATA 
                 are the same.
'''
class hanningsmooth_test(unittest.TestCase):
    
    # Input and output names
    msfile = 'ngc5921_ut.ms'
    res = None
    out = 'hanningsmooth_test'

    def setUp(self):
        self.res = None
        default(hanningsmooth)
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/hanningsmooth/'
        shutil.copytree(datapath+self.msfile, self.msfile)
    
    def tearDown(self):
        if (os.path.exists(self.msfile)):
            os.system('rm -rf ' + self.msfile)
        if (os.path.exists('cvelngc.ms')):
            os.system('rm -rf cvelngc.ms')
        if (os.path.exists(self.out)):
            os.system('rm -rf ' + self.out)
        
        shutil.rmtree('mynewms.ms',ignore_errors=True)

        
    def checkcol(self,table,colname):
        '''Check if requested column exists'''
        tb.open(table)
        col = tb.getvarcol(colname)
        if ((not col) or (len(col) == 0)):
            tb.close()
            return False
        else:
            tb.close()
            return True
        
    def calc(self,dataB,data,dataA):
        '''Calculate the hanning smooth of each element'''
        const0 = 0.25
        const1 = 0.5
        const2 = 0.25
        S = const0*dataB + const1*data + const2*dataA
        return S

    def getvarcol(self,table,colname):
        '''Return the requested column'''
        tb.open(table)
        col = tb.getvarcol(colname)
        tb.close()
        return col

    def test0(self):
        '''Test 0: Default values'''
        self.res = hanningsmooth()
        self.assertFalse(self.res)
        
    def test1(self):
        """Test 1: Wrong input MS should return False"""
        msfile = 'badmsfile'
        self.res = hanningsmooth(vis=msfile)
        self.assertFalse(self.res)

    def test2(self):
        '''Test 2: Check that output column is created'''
        self.res = hanningsmooth(vis=self.msfile, datacolumn='corrected')
        self.assertEqual(self.res,None)
        self.assertTrue(self.checkcol(self.msfile, 'CORRECTED_DATA'))
        
    def test3(self):
        '''Test 3: Theoretical and calculated values should be the same with datacolumn==CORRECTED'''

      # check correct flagging (just for one row as a sample)
        flag_col = self.getvarcol(self.msfile, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])

        self.res = hanningsmooth(vis=self.msfile, datacolumn='corrected')

      # check correct flagging (just for one row as a sample)
        flag_col = self.getvarcol(self.msfile, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [True])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [True])

        self.assertTrue(self.checkcol(self.msfile, 'CORRECTED_DATA'))
        data_col = self.getvarcol(self.msfile, 'DATA')
        corr_col = self.getvarcol(self.msfile, 'CORRECTED_DATA')
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
        
                    Smoothed = self.calc(dataB,data,dataA)
                    CorData = corr_col[row][pol][chan]
                    
                    # Check the difference
                    self.assertTrue(abs(CorData-Smoothed) < max )

    def test4(self):
        '''Test 4: Theoretical and calculated values should be the same with datacolumn==DATA'''

      # check correct flagging (just for one row as a sample)
        flag_col = self.getvarcol(self.msfile, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])
        
        data_col = self.getvarcol(self.msfile, 'DATA')
        self.res = hanningsmooth(vis=self.msfile,datacolumn='data')
        corr_col = self.getvarcol(self.msfile, 'DATA')
        nrows = len(corr_col)

      # check correct flagging (just for one row as a sample)
        flag_col = self.getvarcol(self.msfile, 'FLAG')
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
        
                    Smoothed = self.calc(dataB,data,dataA)
                    CorData = corr_col[row][pol][chan]
                    
                    # Check the difference
                    self.assertTrue(abs(CorData-Smoothed) < max )

    def test5(self):
        '''Test 5: Check that output MS is created '''
        shutil.rmtree('mynewms.ms',ignore_errors=True)
        self.res = hanningsmooth(vis=self.msfile, outputvis='mynewms.ms')
        self.assertTrue(self.checkcol('mynewms.ms', 'DATA'))

    def test6(self):
        '''Test 6: Flagging should be correct with datacolumn==ALL'''
        clearcal(vis=self.msfile)
        
      # check correct flagging (just for one row as a sample)
        flag_col = self.getvarcol(self.msfile, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])

        self.res = hanningsmooth(vis=self.msfile,datacolumn='all')

      # check correct flagging (just for one row as a sample)
        flag_col = self.getvarcol(self.msfile, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [True])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [True])

    def test7(self):
        '''Test 7: Flagging should be correct when hanning smoothing within cvel (no transform)'''
        clearcal(vis=self.msfile)
        
      # check correct flagging (just for one row as a sample)
        flag_col = self.getvarcol(self.msfile, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])

        self.res = cvel(vis=self.msfile, outputvis='cvelngc.ms', hanning=True)

      # check correct flagging (just for one row as a sample)
        flag_col = self.getvarcol('cvelngc.ms', 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [True])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [True])

    def test8(self):
        '''Test 8: Flagging should be correct when hanning smoothing within cvel (with transform)'''
        clearcal(vis=self.msfile)
        
      # check correct flagging (just for one row as a sample)
        flag_col = self.getvarcol(self.msfile, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])

        self.res = cvel(vis=self.msfile, outputvis='cvelngc.ms', hanning=True, outframe='bary')

      # check correct flagging (just for one row as a sample)
        flag_col = self.getvarcol('cvelngc.ms', 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [True])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][2] == [False])
        self.assertTrue(flag_col['r1'][0][60] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [True])
            
def suite():
    return [hanningsmooth_test]


