# unit test for the wvrgcal task

import os
import shutil

from __main__ import default
from tasks import *
from taskinit import *
import unittest

class wvrgcal_test(unittest.TestCase):
    
    vis_g = 'M51.ms'
    out = 'mycaltable.wvr'
    rval = False
    
    def setUp(self):    
        self.rval = False

        if(not os.path.exists(self.vis_g)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/exportasdm/input/M51.ms .')

        default(wvrgcal)

    def tearDown(self):
        os.system('rm -rf myinput.ms')
        os.system('rm -rf ' + self.out)

# Test cases    
    def test1(self):
        '''Test 1: Testing default'''
        self.rval = wvrgcal()
        print "Expected error ..."
        self.assertFalse(self.rval)

    def test2(self):
        '''Test 1: Testing with some random values for each parameter'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = wvrgcal(vis="myinput.ms",caltable="xy.wvr",toffset=2.3,nsol=2,segsource=False,
                            reversespw="0~3",disperse=True,cont=False,wvrflag=['DV02', 'DV03'],
                            sourceflag=['0', '1'],statfield="0",statsource="1",tie=[0, 1],
                            smooth=3,scale=3.5)
        print "Expected error ..."
        self.assertFalse(self.rval)


def suite():
    return [wvrgcal_test]

