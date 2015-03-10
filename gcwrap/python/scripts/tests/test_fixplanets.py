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
mymst = mstool()
mymsmdt = msmdtool()

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

    def verify(self, thems, thefield, theref):
        therval = True
        mymsmdt.open(thems)
        thefieldids = mymsmdt.fieldsforname(thefield)
        mymsmdt.close()
        mymst.open(thems)
        thedir = mymst.getfielddirmeas(fieldid=thefieldids[0])
        mymst.close()
        print "Read direction result ", thedir
        if not (thedir['refer']==theref):
            print "ERROR: reference not as expected: expected "+theref+", got "+thedir['refer']
            therval = False
        return therval
    
    def test1(self):
        '''test1: Does a standard fixplanets work on an MS imported from an ASDM from April 2011'''
        for myms in [outms,outms2]:
            rval = fixplanets(myms, 'Titan', True)
                
            self.assertTrue(rval)


    def test2(self):
        '''test2: Does the setting of a given direction work on an MS imported from an ASDM from April 2011'''
        for myms in [outms,outms2]:
            rval = fixplanets(myms, 'Titan', False, 'J2000 0h0m0s 0d0m0s')
                
            self.assertTrue(rval)
            self.assertTrue(self.verify(myms, 'Titan', 'J2000'))

    def test3(self):
        '''test3: Does the setting of a given direction with ref !=J2000 and != sol.sys. object give the expected error?'''
        for myms in [outms,outms2]:
            rval = fixplanets(myms, 'Titan', False, 'B1950 0h0m0s 0d0m0s')
                
            self.assertFalse(rval)

    def test4(self):
        '''test4: Does the setting of a given direction work with a sol system ref frame?'''
        for myms in [outms,outms2]:
            rval = fixplanets(myms, 'Titan', False, 'SATURN 0h0m0s 0d0m0s')
                
            self.assertTrue(rval)
            self.assertTrue(self.verify(myms, 'Titan', 'SATURN'))

    def test5(self):
        '''test5: Does a standard fixplanets work on an MS imported from an ASDM from April 2011 with parameter reftime'''
        for myms in [outms,outms2]:
            rval = fixplanets(vis=myms, field='Titan', fixuvw=True, reftime='median')
                
            self.assertTrue(rval)

    def test6(self):
        '''test6: Does a standard fixplanets with put of bounds parameter reftime give the expected error'''
        for myms in [outms,outms2]:
            rval = fixplanets(vis=myms, field='Titan', fixuvw=True, reftime='2012/07/11/08:41:32')
                
            self.assertFalse(rval)

    def test7(self):
        '''test7: Does a standard fixplanets with wrong parameter reftime give the expected error'''
        for myms in [outms,outms2]:
            rval = fixplanets(vis=myms, field='Titan', fixuvw=True, reftime='MUDIAN')
                
            self.assertFalse(rval)

    def test8(self):
        '''test8: Does a fixplanets with an ephemeris work'''
        for myms in [outms,outms2]:
            rval = fixplanets(vis=myms, field='Titan', fixuvw=True,
                              direction=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/fixplanets/Titan_55437-56293dUTC.tab')
                
            self.assertTrue(rval)
            self.assertTrue(os.path.exists(myms+'/FIELD/EPHEM0_Titan.tab'))
            self.assertTrue(self.verify(myms, 'Titan', 'APP'))

    def test9(self):
        '''test9: Does a fixplanets with an ephemeris in mime format work'''
        for myms in [outms,outms2]:
            os.system('rm -rf titan.eml')
            os.system('cp '+os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/fixplanets/titan.eml .')
            rval = fixplanets(vis=myms, field='Titan', fixuvw=True,
                              direction='titan.eml')
                
            self.assertTrue(rval)
            self.assertTrue(os.path.exists(myms+'/FIELD/EPHEM0_Titan.tab'))
            self.assertTrue(self.verify(myms, 'Titan', 'J2000'))

    
def suite():
    return [fixplanets_test1]        
        
    
