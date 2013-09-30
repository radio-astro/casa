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
  2. Does the setting of a given direction work on an MS imported from an ASDM from April 2011
  3. Does the setting of a given direction with ref !=J2000 and != sol.sys. object give the expected error?
  4. Does the setting of a given direction work with a sol system ref frame
  5. Does the use of an ephemeris via the direction parameter work

'''
datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/listvis/'
inpms = 'uid___A002_X1c6e54_X223-thinned.ms'
outms = inpms
inpms2 = 'uid___A002_X1c6e54_X223-thinned.mms'
outms2 = inpms2

class fixplanets_test1(unittest.TestCase):
    def setUp(self):
        res = None
        shutil.rmtree(outms, ignore_errors=True)
        shutil.copytree(datapath + inpms, outms)
        shutil.rmtree(outms2, ignore_errors=True)
        os.system('cp -R '+datapath+inpms2 + ' ' + outms2) 
        #shutil.copytree(datapath + inpms2, outms2)
        default(fixplanets)
        
    def tearDown(self):
        shutil.rmtree(outms, ignore_errors=True)
        shutil.rmtree(outms2, ignore_errors=True)
    
    def test1(self):
        '''Does a standard fixplanets work on an MS imported from an ASDM from April 2011'''
        for myms in [outms,outms2]:
            rval = fixplanets(myms, 'Titan', True)
                
            self.assertTrue(rval)

    def test2(self):
        '''Does the setting of a given direction work on an MS imported from an ASDM from April 2011'''
        for myms in [outms,outms2]:
            rval = fixplanets(myms, 'Titan', False, 'J2000 0h0m0s 0d0m0s')
                
            self.assertTrue(rval)

    def test3(self):
        '''Does the setting of a given direction with ref !=J2000 and != sol.sys. object give the expected error?'''
        for myms in [outms,outms2]:
            rval = fixplanets(myms, 'Titan', False, 'B1950 0h0m0s 0d0m0s')
                
            self.assertFalse(rval)

    def test4(self):
        '''Does the setting of a given direction work with a sol system ref frame?'''
        for myms in [outms,outms2]:
            rval = fixplanets(myms, 'Titan', False, 'SATURN 0h0m0s 0d0m0s')
                
            self.assertTrue(rval)

    def test5(self):
        '''Does a standard fixplanets work on an MS imported from an ASDM from April 2011 with parameter reftime'''
        for myms in [outms,outms2]:
            rval = fixplanets(vis=myms, field='Titan', fixuvw=True, reftime='median')
                
            self.assertTrue(rval)

    def test6(self):
        '''Does a standard fixplanets with put of bounds parameter reftime give the expected error'''
        for myms in [outms,outms2]:
            rval = fixplanets(vis=myms, field='Titan', fixuvw=True, reftime='2012/07/11/08:41:32')
                
            self.assertFalse(rval)

    def test7(self):
        '''Does a standard fixplanets with wrong parameter reftime give the expected error'''
        for myms in [outms,outms2]:
            rval = fixplanets(vis=myms, field='Titan', fixuvw=True, reftime='MUDIAN')
                
            self.assertFalse(rval)

    def test8(self):
        '''Does a fixplanets with an ephemeris work'''
        for myms in [outms,outms2]:
            rval = fixplanets(vis=myms, field='Titan', fixuvw=True,
                              direction=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/fixplanets/Titan_55437-56293dUTC.tab')
                
            self.assertTrue(rval)
            self.assertTrue(os.path.exists(myms+'/FIELD/EPHEM0_Titan.tab'))

    def test9(self):
        '''Does a fixplanets with an ephemeris in mime format work'''
        for myms in [outms,outms2]:
            os.system('rm -rf titan.eml')
            os.system('cp '+os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/fixplanets/titan.eml .')
            rval = fixplanets(vis=myms, field='Titan', fixuvw=True,
                              direction='titan.eml')
                
            self.assertTrue(rval)
            self.assertTrue(os.path.exists(myms+'/FIELD/EPHEM0_Titan.tab'))

    
def suite():
    return [fixplanets_test1]        
        
    
