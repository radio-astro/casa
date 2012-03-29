# unit test for the wvrgcal task

import os
import shutil

from __main__ import default
from tasks import *
from taskinit import *
import unittest

class wvrgcal_test(unittest.TestCase):

    vis_f = 'multisource_unittest.ms'
    vis_g = 'M51.ms'
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
        if(not os.path.exists(self.vis_g)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/exportasdm/input/M51.ms .')

        default(wvrgcal)

    def tearDown(self):
        os.system('rm -rf myinput.ms')
        os.system('rm -rf ' + self.out)
        os.system('rm -rf ' + self.ref)
        os.system('rm -rf ' + self.ref2)

    def compTables(self, referencetab, testtab, excludecols):

        rval = True

        tb2 = casac.homefinder.find_home_by_name('tableHome').create()
        
        tb.open(referencetab)
        cnames = tb.colnames()

        #print cnames

        tb2.open(testtab)
        
        for c in cnames:
            if c in excludecols:
                continue
            print c
            a = tb.getcol(c)
            #print a
            b = 0
            try:
                b = tb2.getcol(c)
            except:
                rval = False
                print 'Error accessing column ', c, ' in table ', testtab
                print sys.exc_info()[0]
                break
            #print b
            if not (a==b).all():
                print 'Column ',c,' differs in tables ', referencetab, ' and ', testtab
                rval = False
                break

        tb.close()
        tb2.close()
        
        if rval:
            print 'Tables ', referencetab, ' and ', testtab, ' agree.'

        return rval

# Test cases    
    def test1(self):
        '''Test 1: Testing default'''
        self.rval = wvrgcal()
        print "Expected error ..."
        self.assertFalse(self.rval)

    def test2(self):
        '''Test 2: Testing with some random values for each parameter'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = wvrgcal(vis="myinput.ms",caltable="mycaltable.wvr",toffset=2.3,
                            nsol=2,segsource=False,
                            reversespw="0~3",disperse=True,cont=False,
                            wvrflag=['DV02', 'DV03'],
                            sourceflag=['0', '1'],statfield="0",statsource="1",tie=['0,1','2,3'],
                            smooth=3,scale=3.5)
        print "Expected error ..."
        self.assertFalse(self.rval)

    def test3(self):
        '''Test 3: Testing with a multi-source dataset'''
        myvis = self.vis_f
        os.system('cp -R ' + myvis + ' myinput.ms')
        os.system('rm -rf '+self.out)
        self.rval = wvrgcal(vis="myinput.ms",caltable=self.out, wvrflag=['0', '1'])

        if(self.rval):
            if os.path.exists(self.out+'/CAL_DESC'):
                self.rval = self.compTables(self.ref, self.out,
                                            ['REF_ANT', 'REF_FEED', 'REF_RECEPTOR', 'REF_FREQUENCY',
                                             'REF_DIRECTION'])
            else:
                self.rval = self.compTables(self.ref2, self.out,
                                            ['TIME',
                                             'FIELD_ID',
                                             'SPECTRAL_WINDOW_ID',
                                             'ANTENNA1',
                                             'ANTENNA2',
                                             'INTERVAL',
                                             'SCAN_NUMBER',
                                             'CPARAM',
                                             'PARAMERR',
                                             'FLAG',
                                             'SNR',
                                             'WEIGHT']
                                            )

        self.assertTrue(self.rval)



def suite():
    return [wvrgcal_test]

