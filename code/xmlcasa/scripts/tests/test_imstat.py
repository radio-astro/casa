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
    s150 = '150arcsec_pix.im'
    s15 = '15arcsec_pix.im'
    s0_015 = '0.015arcsec_pix.im'
    s0_0015 = '0.0015arcsec_pix.im'
    s0_00015 = '0.00015arcsec_pix.im'
    res = None

    def setUp(self):
        self.res = None
        default(clean)
        self.datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/imstat/'
    
    #def tearDown(self):

    def test001(self):
        """Test 1: verify moment maps can have flux densities computed in statistics"""
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
        self.assertTrue(abs(got - expected) < 1e-13)
 
    def test002(self):
        """ Test 2: test position format for 150 arcsec pixel image is correct """
        shutil.copytree(self.datapath+self.s150, self.s150)
        ia.open(self.s150)
        stats = ia.statistics()
        ia.close()
        self.assertTrue(stats['blcf'] == '15:43:21.873, -00.17.47.274, I, 1.41332e+09Hz') 
        self.assertTrue(stats['maxposf'] == '15:22:40.165, +05.11.29.923, I, 1.41332e+09Hz')
        self.assertTrue(stats['minposf'] == '15:43:25.618, +04.22.40.617, I, 1.41332e+09Hz')
        self.assertTrue(stats['trcf'] == '15:00:27.115, +10.20.37.699, I, 1.41332e+09Hz')

    def test003(self):
        """ Test 3: test position format for 15 arcsec pixel image is correct """
        shutil.copytree(self.datapath+self.s15, self.s15)
        ia.open(self.s15)
        stats = ia.statistics()
        ia.close()
        self.assertTrue(stats['blcf'] == '15:24:08.404, +04.31.59.181, I, 1.41332e+09Hz') 
        self.assertTrue(stats['maxposf'] == '15:22:04.016, +05.04.44.999, I, 1.41332e+09Hz')
        self.assertTrue(stats['minposf'] == '15:24:08.491, +04.59.59.208, I, 1.41332e+09Hz')
        self.assertTrue(stats['trcf'] == '15:19:52.390, +05.35.44.246, I, 1.41332e+09Hz')

    def test004(self):
        """ Test 4: test position format for 0.015 arcsec pixel image is correct """
        shutil.copytree(self.datapath+self.s0_015, self.s0_015)
        ia.open(self.s0_015)
        stats = ia.statistics()
        ia.close()
        self.assertTrue(stats['blcf'] == '15:22:00.1285, +05.03.58.0800, I, 1.41332e+09Hz') 
        self.assertTrue(stats['maxposf'] == '15:22:00.0040, +05.04.00.0450, I, 1.41332e+09Hz')
        self.assertTrue(stats['minposf'] == '15:22:00.1285, +05.03.59.7600, I, 1.41332e+09Hz')
        self.assertTrue(stats['trcf'] == '15:21:59.8725, +05.04.01.9050, I, 1.41332e+09Hz')

    def test005(self):
        """ Test 5: test position format for 0.0015 arcsec pixel image is correct """
        shutil.copytree(self.datapath+self.s0_0015, self.s0_0015)
        ia.open(self.s0_0015)
        stats = ia.statistics()
        ia.close()
        self.assertTrue(stats['blcf'] == '15:22:00.01285, +05.03.59.80800, I, 1.41332e+09Hz') 
        self.assertTrue(stats['maxposf'] == '15:22:00.00040, +05.04.00.00450, I, 1.41332e+09Hz')
        self.assertTrue(stats['minposf'] == '15:22:00.01285, +05.03.59.97600, I, 1.41332e+09Hz')
        self.assertTrue(stats['trcf'] == '15:21:59.98725, +05.04.00.19050, I, 1.41332e+09Hz')

    def test006(self):
        """ Test 6: test position format for 0.00015 arcsec pixel image is correct """
        shutil.copytree(self.datapath+self.s0_00015, self.s0_00015)
        ia.open(self.s0_00015)
        stats = ia.statistics()
        ia.close()
        self.assertTrue(stats['blcf'] == '15:22:00.001285, +05.03.59.980800, I, 1.41332e+09Hz') 
        self.assertTrue(stats['maxposf'] == '15:22:00.000040, +05.04.00.000450, I, 1.41332e+09Hz')
        self.assertTrue(stats['minposf'] == '15:22:00.001285, +05.03.59.997600, I, 1.41332e+09Hz')
        self.assertTrue(stats['trcf'] == '15:21:59.998725, +05.04.00.019050, I, 1.41332e+09Hz')

def suite():
    return [imstat_test]


