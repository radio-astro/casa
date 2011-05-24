import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for the vpmanager tool. Tested methods:
        createantresp()
        getrespimagename()
'''
class vpmanager_test(unittest.TestCase):
    
    # Input and output names
    res = None
    inputdir = 'mydir3'
    out = 'hanningsmooth_test'

    def setUp(self):
        self.res = None
    
    def tearDown(self):
        os.system('rm -rf ' + self.inputdir)
        
    def test0(self):
        '''Test 0: Default values'''
        self.res = vp.createantresp()
        self.assertFalse(self.res)
        
    def test1(self):
        """Test 1: No images"""
        os.system('mkdir '+self.inputdir)
        self.res = vp.createantresp(self.inputdir, "2011-02-02-12:00", ["band1","band2","band3"], ["83GHz","110GHz","230GHz"], ["110GHz","230GHz","350GHz"])
        self.assertFalse(self.res)

    def test2(self):
        '''Test 2: Two images have faulty band def'''
        os.system('mkdir '+self.inputdir)
        os.system('touch '+self.inputdir+'/ALMA_0_DV__0._0._360._0._45._90._80._100._110._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_DV__0._0._360._0._45._90._110._200._230._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_DV__0._0._360._0._45._90._230._300._350._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_ME__0._0._360._0._45._90._80._100._110._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_ME__0._0._360._0._45._90._110._200._230._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_ME__0._0._360._0._45._90._230._300._350._GHz_ticra2007_EFP.im')
        self.res = vp.createantresp(self.inputdir, "2011-02-02-12:00", ["band1","band2","band3"], ["83GHz","110GHz","230GHz"], ["110GHz","230GHz","350GHz"])
        self.assertFalse(self.res)
        
    def test3(self):
        '''Test 3: Good input: six images, two antenna types'''
        os.system('mkdir '+self.inputdir)
        os.system('touch '+self.inputdir+'/ALMA_0_DV__0._0._360._0._45._90._80._100._110._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_DV__0._0._360._0._45._90._110._200._230._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_DV__0._0._360._0._45._90._230._300._350._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_ME__0._0._360._0._45._90._80._100._110._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_ME__0._0._360._0._45._90._110._200._230._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_ME__0._0._360._0._45._90._230._300._350._GHz_ticra2007_EFP.im')
        self.res = vp.createantresp(self.inputdir, "2011-02-02-12:00", ["band1","band2","band3"], ["80GHz","110GHz","230GHz"], ["110GHz","230GHz","350GHz"])
        self.assertTrue(self.res)
        tb.open(self.inputdir+'/AntennaResponses')
        self.assertTrue(tb.nrows()==2)
        self.assertTrue(tb.ncols()==19)
        self.assertTrue(tb.getcell('NUM_SUBBANDS',0)==3)
        self.assertTrue(tb.getcell('NUM_SUBBANDS',1)==3)
        tb.close()

    def test4(self):
        '''Test 4: get image name from non-existant observatory'''
        self.res = vp.getrespimagename("ALMA2","2011/01/01/10:00","100GHz","AIF","DV","0deg","0deg","",0)
        self.assertFalse(self.res)

    def test5(self):
        '''Test 5: get image name (fails if AntennaResponses table not in repository)'''
        self.res = vp.getrespimagename("ALMA","2011/01/01/10:00","100GHz","EFP","DV","0deg","0deg","",0)
        self.assertTrue(self.res)

def suite():
    return [vpmanager_test]


