# unit test for the wvrgcal task

import os
import shutil
import numpy as np

from __main__ import default
from tasks import *
from taskinit import *
import unittest
import testhelper as th

class wvrgcal_test(unittest.TestCase):

    vis_f = 'multisource_unittest.ms'
    vis_g = 'wvrgcal4quasar_10s.ms'
    ref = ['multisource_unittest_reference.wvr',
           'multisource_unittest_reference-newformat.wvr',
           'wvrgcalctest.W',
           'wvrgcalctest_toffset.W',
           'wvrgcalctest_segsource.W',
           'wvrgcalctest_wvrflag1.W',
           'wvrgcalctest_wvrflag2.W',
           'wvrgcalctest_reverse.W',
           'wvrgcalctest_reversespw.W',
           'wvrgcalctest_smooth.W',
           'wvrgcalctest_scale.W',
           'wvrgcalctest_tie1.W',
           'wvrgcalctest_tie2.W',
           'wvrgcalctest_sourceflag1.W',
           'wvrgcalctest_sourceflag2.W',
           'wvrgcalctest_statsource.W',
           'wvrgcalctest_nsol.W',
           'wvrgcalctest_disperse.W']

## 2   'wvrgcalctest.W': '',
## 3   'wvrgcalctest_toffset.W': '--toffset -1', ........................ test3
## 4   'wvrgcalctest_segsource.W': '--segsource',
## 5   'wvrgcalctest_wvrflag1.W': '--wvrflag DV03',
## 6   'wvrgcalctest_wvrflag2.W': '--wvrflag DV03 --wvrflag PM02',
## 7   'wvrgcalctest_reverse.W': '--reverse', 
## 8   'wvrgcalctest_reversespw.W': '--reversespw 1', ................... test4
## 9   'wvrgcalctest_smooth.W':'--smooth 3', ............................ test5
## 10   'wvrgcalctest_scale.W':'--scale 0.8', ............................ test6
## 11   'wvrgcalctest_tie1.W':'--segsource --tie 0,1,2', ................. test7
## 12   'wvrgcalctest_tie2.W':'--segsource --tie 0,3 --tie 1,2', ......... test8
## 13   'wvrgcalctest_sourceflag1.W':'--sourceflag 0455-462 --segsource',
## 14   'wvrgcalctest_sourceflag2.W':'--sourceflag 0455-462 --sourceflag 0132-169 --segsource', ...test9
## 15   'wvrgcalctest_statsource.W':'--statsource 0455-462', ..............test10
## 16   'wvrgcalctest_nsol.W':'--nsol 5' ..................................test11
## 17   'wvrgcalctest_disperse.W':'--disperse', .......................... test12


    out = 'mycaltable.wvr'
    rval = False
    
    def setUp(self):    
        self.rval = False

        if(not os.path.exists(self.vis_f)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/wvrgcal/input/multisource_unittest.ms .')
        if(not os.path.exists(self.vis_g)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/wvrgcal/input/wvrgcal4quasar_10s.ms .')
        for i in range(0,len(self.ref)):
            if(not os.path.exists(self.ref[i])):
                os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/wvrgcal/input/'+self.ref[i]+' .')

        default(wvrgcal)

    def tearDown(self):
        os.system('rm -rf myinput.ms')
        os.system('rm -rf ' + self.out)
        for i in range(0,len(self.ref)):
            os.system('rm -rf ' + self.ref[i])



# Test cases    
    def test1(self):
        '''Test 1: Testing default'''
        self.rval = wvrgcal()
        print "Expected error ..."
        self.assertFalse(self.rval)

    def test2(self):
        '''Test 2: Testing with a multi-source dataset'''
        myvis = self.vis_f
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        self.rval = wvrgcal(vis="myinput.ms",caltable=self.out, wvrflag=['0', '1'], toffset=0.)

        if(self.rval):
            if os.path.exists(self.out+'/CAL_DESC'):
                self.rval = th.compTables(self.ref[0], self.out,
                                            ['REF_ANT', 'REF_FEED', 'REF_RECEPTOR', 'REF_FREQUENCY',
                                             'REF_DIRECTION'] # ignore these columns because they are empty
                                            )
            else:
                self.rval = th.compTables(self.ref[1], self.out, ['WEIGHT'] # ignore WEIGHT because it is empty
##                                             ['TIME',
##                                              'FIELD_ID',
##                                              'SPECTRAL_WINDOW_ID',
##                                              'ANTENNA1',
##                                              'ANTENNA2',
##                                              'INTERVAL',
##                                              'SCAN_NUMBER',
##                                              'CPARAM',
##                                              'PARAMERR',
##                                              'FLAG',
##                                              'SNR',
##                                              'WEIGHT']
                                            )

        self.assertTrue(self.rval)

    def test3(self):
        '''Test 3:  wvrgcal4quasar_10s.ms, segsource False'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        self.rval = wvrgcal(vis="myinput.ms",caltable=self.out, segsource=False)
        if(self.rval):
            self.rval = th.compTables(self.ref[3], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)

    def test4(self):
        '''Test 4:  wvrgcal4quasar_10s.ms, reversespw, segsource False'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        self.rval = wvrgcal(vis="myinput.ms",caltable=self.out, reversespw='1', segsource=False, toffset=0.)
        if(self.rval):
            self.rval = th.compTables(self.ref[8], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)


    def test5(self):
        '''Test 5:  wvrgcal4quasar_10s.ms, smooth, segsource False'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        self.rval = wvrgcal(vis="myinput.ms",caltable=self.out, smooth=3, segsource=False, toffset=0.)
        if(self.rval):
            self.rval = th.compTables(self.ref[9], self.out, ['WEIGHT'], # ignore WEIGHT because it is empty
                                      0.01) # tolerance 1 % to accomodate differences between Linux and Mac OSX
        self.assertTrue(self.rval)

    def test6(self):
        '''Test 6:  wvrgcal4quasar_10s.ms, scale, segsource=False'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        self.rval = wvrgcal(vis="myinput.ms",caltable=self.out, scale=0.8, segsource=False, toffset=0.)
        if(self.rval):
            self.rval = th.compTables(self.ref[10], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)

    def test7(self):
        '''Test 7:  wvrgcal4quasar_10s.ms, tie three sources'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        self.rval = wvrgcal(vis="myinput.ms", caltable=self.out, tie=['0,1,2'], toffset=0.)
        if(self.rval):
            self.rval = th.compTables(self.ref[11], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)

    def test8(self):
        '''Test 8:  wvrgcal4quasar_10s.ms, tie two times two sources'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        self.rval = wvrgcal(vis="myinput.ms", caltable=self.out, tie=['0,3', '1,2'], toffset=0.)
        if(self.rval):
            self.rval = th.compTables(self.ref[12], self.out, ['WEIGHT'], 0.01) # ignore WEIGHT because it is empty,
                                                                                  # increase tolerance to 1 % to temporarily
                                                                                  # overcome difference between 32bit and 64bit output
        self.assertTrue(self.rval)

    def test9(self):
        '''Test 9:  wvrgcal4quasar_10s.ms, sourceflag two sources'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        self.rval = wvrgcal(vis="myinput.ms", caltable=self.out, sourceflag=['0455-462','0132-169'], toffset=0.)
        if(self.rval):
            self.rval = th.compTables(self.ref[14], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)

    def test10(self):
        '''Test 10:  wvrgcal4quasar_10s.ms, statsource, segsource=False'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        self.rval = wvrgcal(vis="myinput.ms", caltable=self.out, segsource=False, statsource='0455-462', toffset=0.)
        if(self.rval):
            self.rval = th.compTables(self.ref[15], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)

    def test11(self):
        '''Test 11:  wvrgcal4quasar_10s.ms, nsol, segsource=False'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        self.rval = wvrgcal(vis="myinput.ms", caltable=self.out, segsource=False, nsol=5, toffset=0.)
        if(self.rval):
            self.rval = th.compTables(self.ref[16], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)

    def test12(self):
        '''Test 12:  wvrgcal4quasar_10s.ms, disperse'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        self.rval = wvrgcal(vis="myinput.ms",caltable=self.out, disperse=True)
        if(self.rval):
            self.rval = th.compTables(self.ref[17], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)


def suite():
    return [wvrgcal_test]

