import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for task fixplanets.

Features tested:                                                       
  1. Does a standard fixplanets work on an MS imported from an ASDM from April 2011
  2. Does the setting of a given direction work on an MS imported from an ASDM from April 2011
  3. Does the setting of a given direction with ref !=J2000 and != sol.sys. object give the expected error?
  4. Does the setting of a given direction work with a sol system ref frame

'''
datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/listvis/'
inpms = 'uid___A002_X1c6e54_X223-thinned.ms'
outms = 'output.ms'

class fixplanets_test1(unittest.TestCase):
    def setUp(self):
        res = None
        shutil.rmtree(outms, ignore_errors=True)
        shutil.copytree(datapath + inpms, outms)
        default(fixplanets)
        
    def tearDown(self):
        shutil.rmtree(outms, ignore_errors=True)
        
    def test1(self):
        '''Does a standard fixplanets work on an MS imported from an ASDM from April 2011'''
        rval = fixplanets(outms, 'Titan', True)
                
        self.assertTrue(rval)

    def test2(self):
        '''Does the setting of a given direction work on an MS imported from an ASDM from April 2011'''
        rval = fixplanets(outms, 'Titan', False, 'J2000 0h0m0s 0d0m0s')
                
        self.assertTrue(rval)

    def test3(self):
        '''Does the setting of a given direction with ref !=J2000 and != sol.sys. object give the expected error?'''
        rval = fixplanets(outms, 'Titan', False, 'B1950 0h0m0s 0d0m0s')
                
        self.assertFalse(rval)

    def test4(self):
        '''Does the setting of a given direction work with a sol system ref frame?'''
        rval = fixplanets(outms, 'Titan', False, 'SATURN 0h0m0s 0d0m0s')
                
        self.assertTrue(rval)


    
def suite():
    return [fixplanets_test1]        
        
    
