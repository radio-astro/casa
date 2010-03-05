import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
import math

'''
Unit tests for task imstat.
'''
class imstat_test(unittest.TestCase):
    
    # Input and output names
    moment = 'moment_map.im'
    res = None

    def setUp(self):
        self.res = None
        default(clean)
        self.datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/imstat/'
    
    #def tearDown(self):

    def test001(self):
        """Test 0: verify moment maps can have flux densities computed in statistics"""
        shutil.copytree(self.datapath+self.moment, self.moment)
        stats = imstat(imagename=self.moment)
        mean = stats['mean']
        npts = stats['npts']

        ia.open(self.moment)
        summary = ia.summary()
        ia.close()
        rainc = qa.abs(qa.quantity(summary['header']['incr'][0],'rad'))
        rainc = qa.convert(rainc,'arcsec')
        decinc = qa.abs(qa.quantity(summary['header']['incr'][1],'rad'))
        decinc = qa.convert(decinc,'arcsec')
        beam = summary['header']['restoringbeam']['restoringbeam']
        major = beam['major']
        minor = beam['minor']
        pixperbeam = qa.div(qa.mul(major,minor),(qa.mul(rainc,decinc)))['value']*(math.pi/(4*math.log(2)))
        got = stats['flux'][0]
        expected = (mean*npts/pixperbeam)[0]
        print "expect " + str(expected) + " got " + str(got)
        self.assertTrue(abs(got - expected) < 1e-13)
 
    

def suite():
    return [imstat_test]


