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
           'wvrgcalctest_disperse.W',
           'multisource_unittest_reference-mod.wvr',
           'wvrgcalctest-test19.W']

## 2   'wvrgcalctest.W': '',
## 3   'wvrgcalctest_toffset.W': '--toffset -1', ........................ test3
## 4   'wvrgcalctest_segsource.W': '--segsource',
## 5   'wvrgcalctest_wvrflag1.W': '--wvrflag DV03',
## 6   'wvrgcalctest_wvrflag2.W': '--wvrflag DV03 --wvrflag PM02',
## 7   'wvrgcalctest_reverse.W': '--reverse', 
## 8   'wvrgcalctest_reversespw.W': '--reversespw 1', ................... test4
## 9   'wvrgcalctest_smooth.W':'smooth 3 seconds',........................ test5
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
            rval = os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/wvrgcal/input/multisource_unittest.ms .')
            if rval!=0:
                raise Exception, "Error copying input data"
        if(not os.path.exists(self.vis_g)):
            rval = os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/wvrgcal/input/wvrgcal4quasar_10s.ms .')
            if rval!=0:
                raise Exception, "Error copying input data"
        for i in range(0,len(self.ref)):
            if(not os.path.exists(self.ref[i])):
                rval = os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/wvrgcal/input/'+self.ref[i]+' .')
                if rval!=0:
                    raise Exception, "Error copying input data"

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
        print self.rval
        print "Expected error ..."
        self.assertFalse(self.rval)

    def test2(self):
        '''Test 2: Testing with a multi-source dataset'''
        myvis = self.vis_f
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        rvaldict = wvrgcal(vis="myinput.ms",caltable=self.out, wvrflag=['0', '1'], toffset=0.)

        print rvaldict

        self.rval = rvaldict['success']

        if(self.rval):
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
        rvaldict = wvrgcal(vis="myinput.ms",caltable=self.out, segsource=False, toffset=-1.)

        print rvaldict

        self.rval = rvaldict['success']

        if(self.rval):
            self.rval = th.compTables(self.ref[3], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)

    def test4(self):
        '''Test 4:  wvrgcal4quasar_10s.ms, reversespw, segsource False'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        rvaldict = wvrgcal(vis="myinput.ms",caltable=self.out, reversespw='1', segsource=False, toffset=0.)

        print rvaldict

        self.rval = rvaldict['success']


        if(self.rval):
            self.rval = th.compTables(self.ref[8], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)


    def test5(self):
        '''Test 5:  wvrgcal4quasar_10s.ms, smooth, segsource False'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        rvaldict = wvrgcal(vis="myinput.ms",caltable=self.out, smooth='3s', segsource=False, toffset=0.)

        print rvaldict

        self.rval = rvaldict['success']

        self.assertTrue(os.path.exists(self.out))
        self.assertTrue(os.path.exists(self.out+'_unsmoothed'))
        smoothcal(vis = "myinput.ms",
		  tablein = self.out+'_unsmoothed',
		  caltable = self.out+'_ref',
		  smoothtype = 'mean',
		  smoothtime = 3.)
        if(self.rval):
            self.rval = th.compTables(self.out+'_ref', self.out, ['WEIGHT'], # ignore WEIGHT because it is empty
                                      0.01) # tolerance 1 % to accomodate differences between Linux and Mac OSX
        self.assertTrue(self.rval)

    def test6(self):
        '''Test 6:  wvrgcal4quasar_10s.ms, scale, segsource=False'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        rvaldict = wvrgcal(vis="myinput.ms",caltable=self.out, scale=0.8, segsource=False, toffset=0.)

        print rvaldict

        self.rval = rvaldict['success']

        if(self.rval):
            self.rval = th.compTables(self.ref[10], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)

    def test7(self):
        '''Test 7:  wvrgcal4quasar_10s.ms, tie three sources'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        rvaldict = wvrgcal(vis="myinput.ms", caltable=self.out, tie=['0,1,2'], toffset=0.)

        print rvaldict

        self.rval = rvaldict['success']

        if(self.rval):
            self.rval = th.compTables(self.ref[11], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)

    def test8(self):
        '''Test 8:  wvrgcal4quasar_10s.ms, tie two times two sources'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        rvaldict = wvrgcal(vis="myinput.ms", caltable=self.out, tie=['0,3', '1,2'], toffset=0.)

        print rvaldict

        self.rval = rvaldict['success']

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
        rvaldict = wvrgcal(vis="myinput.ms", caltable=self.out, sourceflag=['0455-462','0132-169'], toffset=0.)

        print rvaldict

        self.rval = rvaldict['success']

        if(self.rval):
            self.rval = th.compTables(self.ref[14], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)

    def test10(self):
        '''Test 10:  wvrgcal4quasar_10s.ms, statsource, segsource=False'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        rvaldict = wvrgcal(vis="myinput.ms", caltable=self.out, segsource=False, statsource='0455-462', toffset=0.)

        print rvaldict

        self.rval = rvaldict['success']

        if(self.rval):
            self.rval = th.compTables(self.ref[15], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)

    def test11(self):
        '''Test 11:  wvrgcal4quasar_10s.ms, nsol, segsource=False'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        rvaldict = wvrgcal(vis="myinput.ms", caltable=self.out, segsource=False, nsol=5, toffset=0.)

        print rvaldict

        self.rval = rvaldict['success']

        if(self.rval):
            self.rval = th.compTables(self.ref[16], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)

    def test12(self):
        '''Test 12:  wvrgcal4quasar_10s.ms, disperse'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        rvaldict = wvrgcal(vis="myinput.ms",caltable=self.out, disperse=True, toffset=-1.)

        print rvaldict

        self.rval = rvaldict['success']

        if(self.rval):
            self.rval = th.compTables(self.ref[17], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)

    def test13(self):
        '''Test 13:  wvrgcal4quasar_10s.ms,  totally flagged main table'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')

        flagdata(vis="myinput.ms", spw='0', mode='manual')
        
        os.system('rm -rf '+self.out)
        rvaldict = wvrgcal(vis="myinput.ms", caltable=self.out, disperse=True, toffset=-1.)

        print rvaldict

        self.rval = rvaldict['success']

        print "Expected error ..."

        self.assertFalse(self.rval)


    def test14(self):
        '''Test 14:  wvrgcal4quasar_10s.ms, first seconds flagged for one antenna'''
        myvis = self.vis_g
        os.system('rm -rf myinput2.ms comp.W comp2.W')
        os.system('cp -R ' + myvis + ' myinput.ms')

        flagdata(vis="myinput.ms", timerange='09:10:11~09:10:15', antenna='DV14&&*', mode='manual')
        #flagdata(vis="myinput.ms", scan='2', mode='manual')
        split(vis='myinput.ms', outputvis='myinput2.ms', datacolumn='data', keepflags=False)
        
        rvaldict = wvrgcal(vis="myinput.ms", caltable='comp.W', toffset=0.)
        rvaldict2 = wvrgcal(vis="myinput2.ms", caltable='comp2.W', toffset=0.)

        print rvaldict
        print rvaldict2

        self.rval = rvaldict['success'] and rvaldict2['success']

        if(self.rval):
            rvaldict2['Disc_um'][14]= 64.299999999999997 # The value for antenna14 is the only one expected to be different
            rvaldict2['RMS_um'][14]= 55.600000000000001 # The value for antenna14 is the only one expected to be different
            
            self.rval = (rvaldict==rvaldict2)
               
        self.assertTrue(self.rval)
        

    def test15(self):
        '''Test 15:  wvrgcal4quasar_10s.ms, one antenna flagged'''
        myvis = self.vis_g
        os.system('rm -rf myinput2.ms comp.W')
        os.system('cp -R ' + myvis + ' myinput.ms')

        os.system('rm -rf '+self.out)

        rvaldict = wvrgcal(vis="myinput.ms", caltable=self.out, wvrflag='DA41', toffset=-1.)

        tb.open('myinput.ms/ANTENNA', nomodify=False)
        fr = tb.getcol('FLAG_ROW')
        fr[2] = True
        tb.putcol('FLAG_ROW', fr)
        tb.close()
        
        rvaldict2 = wvrgcal(vis="myinput.ms", caltable='comp.W', toffset=-1.)

        print rvaldict
        print rvaldict2

        self.rval = rvaldict['success'] and rvaldict2['success']

        if(self.rval):
            rvaldict2['Disc_um'][2]=49.100000000000001 # The value for antenna2 is the only one expected to be different
                                                       # as it was flagged. Replace by value for the unflagged case
                                                       # to make following test pass if all else agrees.
            self.rval = (rvaldict==rvaldict2)
               
        self.assertTrue(self.rval)

    def test16(self):
        '''Test 16: Test the maxdistm and minnumants parameters'''
        myvis = self.vis_f
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        rvaldict = wvrgcal(vis="myinput.ms",caltable=self.out, wvrflag=['0', '1'], toffset=0., maxdistm=40., minnumants=2)

        print rvaldict

        self.rval = rvaldict['success']

        if(self.rval):
            self.rval = th.compTables(self.ref[18], self.out, ['WEIGHT', 'CPARAM'] # ignore WEIGHT because it is empty
                                      )
            if(self.rval):
                tb.open(self.out)
                a = tb.getcol('ANTENNA1')
                c = tb.getcol('CPARAM')[0][0]
                tb.close()
                i = 1
                for i in range(len(a)):
                    if (a[i]==1 and not (c[i]==(1+0j))):
                        self.rval=False
                        print "CPARAM for antenna 1 has value ", c[i], " expected (1+0j)."
                        break
            
        self.assertTrue(self.rval)

    def test17(self):
        '''Test 17:  wvrgcal4quasar_10s.ms, two antennas flagged in main table, one only partially, use of mingoodfrac'''
        myvis = self.vis_g
        os.system('rm -rf myinput2.ms comp.W')
        os.system('cp -R ' + myvis + ' myinput.ms')

        os.system('rm -rf '+self.out)
        rvaldict = wvrgcal(vis="myinput.ms", caltable=self.out, wvrflag='DA41', toffset=-1.)

        flagdata(vis='myinput.ms', mode='manual', antenna='DA41&&*')
        flagdata(vis='myinput.ms', mode='manual', antenna='CM01&&*', scan='1') # antenna 0, scan 1 only!
        
        rvaldict2 = wvrgcal(vis="myinput.ms", caltable='comp.W', toffset=-1., mingoodfrac=0.5)

        print rvaldict
        print rvaldict2

        self.rval = rvaldict['success'] and rvaldict2['success']

        if(self.rval):
            rvaldict2['Disc_um'][2]=49.100000000000001 # The value for antenna 2 is the only one expected to be different
                                                       # as it was flagged. Replace by value for the unflagged case
                                                       # to make following test pass if all else agrees.
            rvaldict2['Flag'][2]=True # by the same logic as above
            rvaldict2['RMS_um'][2]=66.900000000000006 # by the same logic as above
            for mykey in ['Name', 'WVR', 'RMS_um', 'Disc_um']:  
                print mykey+" "+str(rvaldict[mykey]==rvaldict2[mykey])
            self.rval = (rvaldict==rvaldict2)
               
        self.assertTrue(self.rval)

    def test18(self):
        '''Test 18:  wvrgcal4quasar_10s.ms, two antennas flagged in main table, one only partially'''
        myvis = self.vis_g
        os.system('rm -rf myinput2.ms comp.W')
        os.system('cp -R ' + myvis + ' myinput.ms')

        os.system('rm -rf '+self.out)
        rvaldict = wvrgcal(vis="myinput.ms", caltable=self.out, wvrflag='DA41', toffset=-1.)

        flagdata(vis='myinput.ms', mode='manual', antenna='DA41&&*')
        flagdata(vis='myinput.ms', mode='manual', antenna='DV12&&*', timerange='9:10:12~9:10:13,9:12:31~9:12:32') # antenna 12, a few non-contiguous scans!
        
        rvaldict2 = wvrgcal(vis="myinput.ms", caltable='comp.W', toffset=-1.)

        print rvaldict
        print rvaldict2

        self.rval = rvaldict['success'] and rvaldict2['success']

        if(self.rval):
            rvaldict2['Disc_um'][12]=42.100000000000001 # The value for antenna 2 is the only one expected to be different
                                                       # as it was flagged. Replace by value for the unflagged case
                                                       # to make following test pass if all else agrees.
            rvaldict2['Flag'][12]=False # by the same logic as above
            rvaldict2['RMS_um'][12]=66.0 # by the same logic as above
            for mykey in ['Name', 'WVR', 'RMS_um', 'Disc_um']:  
                print mykey+" "+str(rvaldict[mykey]==rvaldict2[mykey])
            self.rval = (rvaldict==rvaldict2)
               
        self.assertTrue(self.rval)

    def test19(self):
        '''Test 19:  wvrgcal4quasar_10s.ms, PM02 partially flagged in main table, DV41 with wvrflag, PM02 necessary for interpol of DV41'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')

        os.system('rm -rf '+self.out)

        flagdata(vis='myinput.ms', mode='manual', antenna='PM02&&*', scan='3')

        rvaldict = wvrgcal(vis="myinput.ms", caltable=self.out, wvrflag='DA41', toffset=-1., mingoodfrac=0.2)

        print rvaldict

        self.rval = rvaldict['success']

        if(self.rval):
            self.rval = th.compTables(self.ref[19], self.out, ['WEIGHT']) # ignore WEIGHT because it is empty
        self.assertTrue(self.rval)




def suite():
    return [wvrgcal_test]

