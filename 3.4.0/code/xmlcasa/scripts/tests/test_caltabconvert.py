# unit test for the caltabconvert task

import os
import shutil
import numpy as np       

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
            if not (len(a)==len(b)):
                print 'Column ',c,' has different length in tables ', referencetab, ' and ', testtab
                print a
                print b
                rval = False
                break
            else:
                if not (a==b).all():
                    differs = False
                    for i in range(0,len(a)):
                        if (type(a[i])==float):
                            if (abs(a[i]-b[i]) > 0.001*abs(a[i]+b[i])):
                                print 'Column ',c,' differs in tables ', referencetab, ' and ', testtab
                                print i
                                print a[i]
                                print b[i]
                                differs = True
                        elif (type(a[i])==int):
                            if (abs(a[i]-b[i]) > 0):
                                print 'Column ',c,' differs in tables ', referencetab, ' and ', testtab
                                print i
                                print a[i]
                                print b[i]
                                differs = True
                        elif (type(a[i])==str):
                            if not (a[i]==b[i]):
                                print 'Column ',c,' differs in tables ', referencetab, ' and ', testtab
                                print i
                                print a[i]
                                print b[i]
                                differs = True
                        elif (type(a[i])==list or type(a[i])==np.ndarray):
                            for j in range(0,len(a[i])):
                                if (type(a[i][j])==float):
                                    if (abs(a[i][j]-b[i][j]) > 0.001*abs(a[i][j]+b[i][j])):
                                        print 'Column ',c,' differs in tables ', referencetab, ' and ', testtab
                                        print i, j
                                        print a[i][j]
                                        print b[i][j]
                                        differs = True
                                elif (type(a[i][j])==list or type(a[i][j])==np.ndarray):
                                    for k in range(0,len(a[i][j])):
                                        if (abs(a[i][j][k]-b[i][j][k]) > 0.001*abs(a[i][j][k]+b[i][j][k])):
                                            print 'Column ',c,' differs in tables ', referencetab, ' and ', testtab
                                            print i, j, k
                                            print a[i][j][k]
                                            print b[i][j][k]
                                            differs = True
                    if differs:
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
            self.rval = self.compTables(self.ref2, self.out,['WEIGHT', 'PARAMERR']) # WEIGHT column is empty

            self.rval = self.rval and self.compTables(self.ref2+'/ANTENNA', self.out+'/ANTENNA', [])

            self.rval = self.rval and self.compTables(self.ref2+'/FIELD', self.out+'/FIELD', [])

            self.rval = self.rval and self.compTables(self.ref2+'/SPECTRAL_WINDOW', self.out+'/SPECTRAL_WINDOW', [])
                                                          

        self.assertTrue(self.rval)



def suite():
    return [caltabconvert_test]

