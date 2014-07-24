import os
import sys
import shutil
import re
import numpy

from __main__ import default
from tasks import *
from taskinit import *
import unittest
#
#import listing
#from numpy import array

import asap as sd
from ssdreduce import ssdreduce
#from sdstat import sdstat

def gaussian(nchan, width):
    data = numpy.zeros(nchan, dtype=float)
    peak_channel = nchan / 2 - 1
    sigma = width / numpy.sqrt(8.0 * numpy.log(2.0))
    for i in xrange(nchan):
        data[i] = numpy.exp(- 0.5 / (sigma * sigma) * float(i - peak_channel) * float(i - peak_channel))
    data /= data.sum()
    return data

class ssdreduce_test(unittest.TestCase):
    """
    Basic unit tests for task ssdreduce. No interactive testing.

    The list of tests:
    test00    --- default parameters (raises an errror)
    test01    --- Default parameters + valid input filename
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/singledish/'
    # Input and output names
    # uncalibrated data
    prefix = 'analytic_spectra'
    infile = prefix + '.ms'
    skytable = prefix + '_sky.ms'
    tsystable = prefix + '_tsys.ms'
    
    outprefix = 'ssdreduce_test'

    peak = [10.53907204,  13.53907585,  16.53907776]
    flagged = [50, 100, 150, 200, 250, 300, 350, 400, 450]
    edge = 30
    peak_channel = 256
    blmask = '0~255;257~499'

    def setUp(self):
        for f in [self.infile, self.skytable, self.tsystable]:
            if os.path.exists(f):
                shutil.rmtree(f)
            shutil.copytree(os.path.join(self.datapath,f), f)
        default(ssdreduce)

    def tearDown(self):
        os.system('rm -rf %s*'%(self.prefix))
        os.system('rm -rf %s*'%(self.outprefix))

    ## def test00(self):
    ##     """Test 0: Default parameters (raises an errror)"""
    ##     #print blfunc
    ##     with self.assertRaises(RuntimeError) as cm:
    ##         result = ssdreduce()
    ##     the_exception = cm.exception
    ##     message = the_exception.message

    def __onsource_state_id(self):
        tb.open(os.path.join(self.infile, 'STATE'))
        obs_mode = tb.getcol('OBS_MODE')
        tb.close()

        pattern = 'OBSERVE_TARGET#ON_SOURCE'
        state_id = [i for i in xrange(len(obs_mode))
                    if re.search(pattern, obs_mode[i])]
        #print 'STATE_ID', state_id
        return state_id

    def __expected_spectra(self, shape):
        expected = numpy.zeros(shape, dtype=float)
        nchan,nrow = shape
        kernel = gaussian(nchan, 5)
        #print kernel.max(), kernel.argmax()
        for irow in xrange(nrow):
            expected[self.peak_channel,irow] = self.peak[irow]
            expected[:,irow] = numpy.convolve(expected[:,irow], kernel, 'same')
        return expected
    
    def _verify(self):
        tb.open(self.infile)
        colnames = tb.colnames()
        tb.close()

        print 'Check if CORRECTED_DATA exists'
        self.assertTrue('CORRECTED_DATA' in colnames)

        state_id = self.__onsource_state_id()
        tb.open(self.infile)
        tsel = tb.query('STATE_ID IN %s'%(state_id))
        corrected = tsel.getcol('CORRECTED_DATA').real[0]
        floatdata = tsel.getcol('FLOAT_DATA')[0]
        flag = tsel.getcol('FLAG')[0]
        tsel.close()
        tb.close()

        nchan,nrow = corrected.shape

        print 'Check if nrow and nchan are correct'
        self.assertEqual(nchan, 500)
        self.assertEqual(nrow, 3)

        #print corrected.max(axis=0)
        #print corrected.argmax(axis=0)
        expected = self.__expected_spectra(corrected.shape)
        #print expected.max(axis=0)
        #print expected.argmax(axis=0)

        # Verification
        # Expected spectra are simple gaussian whose peak values
        # are given in self.peak. Baseline value is ideally 0.0 
        # but there are residuals of baseline subtraction (~10^-7)
        # which are regarded as numerical noise. The verification
        # below takes into account such residual. Effective
        # tolerance for such data is o(d/v) where d is a required
        # tolerance for residual-free case and v is a resulting
        # residual.
        err = 1.0e-7
        tol = 1.0e-6
        print 'Verify resulting spectra with tolerance %s'%(tol)
        for irow in xrange(nrow):
            for ichan in xrange(nchan):
                e = expected[ichan,irow]
                v = corrected[ichan,irow]
                if e < err:
                    #print irow, ichan, e, v
                    self.assertLess(abs(v), tol)
                else:
                    thetol = abs(tol / v)
                    diff = abs((v - e) / e)
                    #print irow, ichan, e, v, thetol, diff
                    self.assertLess(diff, thetol)

        print 'Verify resulting flag'
        for irow in xrange(nrow):
            v = floatdata[:,irow]
            f = flag[:,irow]
            e = numpy.zeros(nchan, dtype=bool)
            e[:self.edge] = True
            e[-self.edge:] = True
            e[self.flagged] = True
            self.assertTrue(all(f==e))
    
    def test01(self):
        """Test 1: e2e reduction"""
        result = ssdreduce(vis=self.infile, selectdata=True, antenna='0',
                           gaintable=[self.skytable, self.tsystable],
                           spwmap={'1':0}, maskmode='auto', edge=[self.edge],
                           blmask=self.blmask, blfunc='poly',
                           order=5, clipthresh=3.0, clipniter=2,
                           clipminmax=[-20, 40], kernel='gaussian',
                           kwidth=5)
        self.assertTrue(result is None)
                           
        self._verify()

def suite():
    return [ssdreduce_test]
