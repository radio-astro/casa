# unit test for the caltabconvert task

import os
import shutil
import numpy as np       

from __main__ import default
from tasks import *
from taskinit import *
import unittest
import testhelper as th

class caltabconvert_test(unittest.TestCase):

    vis_f = 'multisource_unittest.ms'
    ref = 'multisource_unittest_reference.wvr'
    ref2 = 'multisource_unittest_reference-newformat.wvr'
    out = 'mycaltable.wvr'
    rval = False
    
    def setUp(self):    
        self.rval = False

        if(not os.path.exists(self.vis_f)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/wvrgcal/input/multisource_unittest.ms .')
        if(not os.path.exists(self.ref)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/wvrgcal/input/multisource_unittest_reference.wvr .')
        if(not os.path.exists(self.ref2)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/wvrgcal/input/multisource_unittest_reference-newformat.wvr .')

        default(caltabconvert)

    def tearDown(self):
        os.system('rm -rf ' + self.out)


# Test cases    
    def test1(self):
        '''Test 1: Testing default'''
        self.rval = caltabconvert()
        print "Expected error ..."
        self.assertFalse(self.rval)

    def test2(self):
        '''Test 2: Testing with a WVR caltable'''
        os.system('rm -rf '+self.out)
        self.rval = caltabconvert(vis=self.vis_f,
                                  caltabold=self.ref,
                                  ptype='complex',
                                  caltabnew=self.out)

        if(self.rval):
            self.rval = th.compTables(self.ref2, self.out,['WEIGHT', 'PARAMERR']) # WEIGHT column is empty

            self.rval = self.rval and th.compTables(self.ref2+'/ANTENNA', self.out+'/ANTENNA', [])

            self.rval = self.rval and th.compTables(self.ref2+'/FIELD', self.out+'/FIELD', [])

            self.rval = self.rval and th.compTables(self.ref2+'/SPECTRAL_WINDOW', self.out+'/SPECTRAL_WINDOW', [])
                                                          

        self.assertTrue(self.rval)



def suite():
    return [caltabconvert_test]

