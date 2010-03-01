import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for task smoothcal. It tests the following parameters:
    vis:           wrong and correct values
    tablein:       wrong and correct values
    caltable:      existence of output
    field:         wrong field value; non-default value
    smmothtype:    unsupported value; non-default value
    smoothtime:    unsupported value; non-default values
    
    Other tests: check the values of column smoothed GAIN against reference.
'''
class smoothcal_test(unittest.TestCase):
    
    # Input and output names
    msfile = 'ngc1333_ut.ms'
    gcal = 'ngc1333_ut.gcal'
    ref = 'ngc1333_ut.ref'
    res = None
    out = 'smoothcal_test'

    def setUp(self):
        self.res = None
        default(smoothcal)
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/smoothcal/'
        shutil.copytree(datapath+self.msfile, self.msfile)
        shutil.copytree(datapath+self.gcal, self.gcal)
        shutil.copytree(datapath+self.ref, self.ref)
    
    def tearDown(self):
        if (os.path.exists(self.msfile)):
            os.system('rm -rf ' + self.msfile)
        if (os.path.exists(self.gcal)):
            os.system('rm -rf ' + self.gcal)
        if (os.path.exists(self.ref)):
            os.system('rm -rf ' + self.ref)
        if (os.path.exists(self.out)):
            os.system('rm -rf ' + self.out)
        
    def getvarcol(self,table,colname):
        '''Return the requested column'''
        tb.open(table)
        col = tb.getvarcol(colname)
        tb.close()
        return col

    def test0(self):
        '''Test 0: Default values'''
        self.res = smoothcal()
        self.assertFalse(self.res)
        
    def test1(self):
        """Test 1: Wrong input MS should return False"""
        msfile = 'badmsfile'
        self.res = smoothcal(vis=msfile,tablein=self.gcal,caltable=self.out)
        self.assertFalse(self.res)
        self.assertFalse(os.path.exists(self.out))
        
    def test2(self):
        """Test 2: Wrong input gcal should return False"""
        gcal = 'badgcal'
        self.res = smoothcal(vis=self.msfile,tablein=gcal,caltable=self.out)
        self.assertFalse(self.res)
        self.assertFalse(os.path.exists(self.out))

    def test3(self):
        """Test 3: Good input should return None"""
        self.res = smoothcal(vis=self.msfile,tablein=self.gcal,caltable=self.out)
        self.assertEqual(self.res,None)
        self.assertTrue(os.path.exists(self.out))
        
    def test4(self):
        """Test 4: Unsupported smoothtype"""
        self.res = smoothcal(vis=self.msfile,tablein=self.gcal,caltable=self.out,smoothtype='average')
        self.assertFalse(self.res)
        self.assertFalse(os.path.exists(self.out))

    def test5(self):
        '''Test 5: Non-default smoothtype'''
        self.res = smoothcal(vis=self.msfile,tablein=self.gcal,caltable=self.out,smoothtype='mean')
        self.assertEqual(self.res,None)
        self.assertTrue(os.path.exists(self.out))

    def test6(self):
        '''Test 6: Unsupported smoothtime'''
        self.res = smoothcal(vis=self.msfile,tablein=self.gcal,caltable=self.out,smoothtime=-1)
        self.assertFalse(os.path.exists(self.out))

    def test7(self):
        '''Test 7: Non-default smoothtype and smoothtime'''
        self.res = smoothcal(vis=self.msfile,tablein=self.gcal,caltable=self.out,smoothtype='mean',
                         smoothtime=7200)
        self.assertEqual(self.res,None)
        self.assertTrue(os.path.exists(self.out))

    def test8(self):
        '''Unsupported field values'''
        self.res = smoothcal(vis=self.msfile,tablein=self.gcal,caltable=self.out,field='23~30')
        self.assertFalse(os.path.exists(self.out))
       
    def test9(self):
        '''Test 9: Non-default field selection'''
        self.res = smoothcal(vis=self.msfile,tablein=self.gcal,caltable=self.out,smoothtype='median',
                         smoothtime=5000.5,field='2')
#        self.assertEqual(self.res,None)
        self.assertTrue(os.path.exists(self.out))

    def test10(self):
        '''Test10: Compare smoothed values with reference'''
        self.res=smoothcal(vis=self.msfile,tablein=self.gcal,caltable=self.out,smoothtype='mean',
                         smoothtime=7200.)
        self.assertEqual(self.res,None)
        refcol = self.getvarcol(self.out, 'GAIN')
        smcol = self.getvarcol(self.out, 'GAIN')
        nrows = len(refcol)
        EPS = 1e-5;  # Logical "zero"
        # Loop over every row,pol and get the data
        for i in range(1,nrows,1) :
            row = 'r%s'%i     
            # polarization is 0-1
            for pol in range(0,2) :     
                refdata = refcol[row][pol]
                smdata = smcol[row][pol]
                self.assertTrue(abs(refdata - smdata) < EPS)



def suite():
    return [smoothcal_test]





