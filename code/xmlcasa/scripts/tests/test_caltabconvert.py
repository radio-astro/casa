# unit test for the caltabconvert task

import os
import shutil

from __main__ import default
from tasks import *
from taskinit import *
import unittest

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
            self.rval = self.rval and self.compTables(self.ref2+'/ANTENNA', self.out+'/ANTENNA',
                                                      ['OFFSET',
                                                       'POSITION',
                                                       'TYPE',
                                                       'DISH_DIAMETER',
                                                       'FLAG_ROW',
                                                       'MOUNT',
                                                       'NAME',
                                                       'STATION']
                                                      )

            self.rval = self.rval and self.compTables(self.ref2+'/FIELD', self.out+'/FIELD',
                                                      ['DELAY_DIR',
                                                       'PHASE_DIR',
                                                       'REFERENCE_DIR',
                                                       'CODE',
                                                       'FLAG_ROW',
                                                       'NAME',
                                                       'NUM_POLY',
                                                       'SOURCE_ID',
                                                       'TIME']
                                                      )

            self.rval = self.rval and self.compTables(self.ref2+'/SPECTRAL_WINDOW', self.out+'/SPECTRAL_WINDOW',
                                                      ['MEAS_FREQ_REF',
                                                       'CHAN_FREQ',
                                                       'REF_FREQUENCY',
                                                       'CHAN_WIDTH',
                                                       'EFFECTIVE_BW',
                                                       'RESOLUTION',
                                                       'FLAG_ROW',
                                                       'FREQ_GROUP',
                                                       'FREQ_GROUP_NAME',
                                                       'IF_CONV_CHAIN',
                                                       'NAME',
                                                       'NET_SIDEBAND',
                                                       'NUM_CHAN',
                                                       'TOTAL_BANDWIDTH']
                                                      )
            

        self.assertTrue(self.rval)



def suite():
    return [caltabconvert_test]

