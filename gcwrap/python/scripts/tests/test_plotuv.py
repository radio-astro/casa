import os
import string
import sys
import shutil
import unittest
from __main__ import default
from tasks import *
from taskinit import *

'''
Unit tests for task plotuv. It tests the following parameters:
    vis:           wrong and correct values
    figfile:       if output is created
'''
class plotuv_test(unittest.TestCase):
    # Input and output names
    datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/ic2233/'
    msfile = datapath + 'ic2233_1.ms'
    res = None
    fig = 'plotuvtest.png'

    def setUp(self):
        self.res = None
        default(plotuv)
    
    def tearDown(self):
        pass
        
    def test0(self):
       '''Test 0: Default parameters'''
       self.res = plotuv()
       self.assertFalse(self.res)  
       
    def test1(self):
        '''Test 1: Bad input file'''
        msfile = 'badfile'
        self.res = plotuv(vis=msfile)
        self.assertFalse(self.res)
        
    def test2(self):
        '''Test 2: Good input file and output exists'''
        if os.uname()[0] == "Darwin" and \
           os.system("sw_vers -productVersion | grep 10.6") == 0 and \
           not os.getenv("DISPLAY"):
            print >> sys.stderr, "Warning: The DISPLAY environment variable is unset, " + \
            "required on OS X 10.6, skipping test"
        else:
            self.res = plotuv(vis=self.msfile, figfile=self.fig)
            self.assertTrue(self.res)
            expfigparts = self.fig.split('.')
            expfig = '.'.join(expfigparts[:-1]) + '_fld0.' + expfigparts[-1]
            self.assertTrue(os.path.exists(expfig))
            os.unlink(expfig)
        
def suite():
    return [plotuv_test]

        
        
        
        
        

