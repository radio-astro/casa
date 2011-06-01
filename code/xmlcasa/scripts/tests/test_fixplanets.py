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
        self.res = fixplanets(outms, 'Titan', True)
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
                
        self.assertTrue(retValue['success'])

    
def suite():
    return [fixplanets_test1]        
        
    
