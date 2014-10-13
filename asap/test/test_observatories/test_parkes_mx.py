#!/usr/bin/env python
import os
from asap import *
from nose.tools import assert_almost_equals
rcParams['verbose'] = 0
rcParams['plotter.gui'] = 0

DATA=os.path.join(os.path.split(__file__)[0], "..", "data", 
                  "2011-09-27_0551-P000_WKK5999_test.mbf")

class TestParkes(object):
    def setup(self):
        self.st = scantable(DATA)

    def test_reduction(self):
        q = self.st.mx_quotient()
        # Average all beam data together
        avb = q.average_beam()
        avp = avb.average_pol()
        # Fit a polynomial, avoiding the galactic emission
        msk = avp.create_mask([430,580],invert=True)
        avp.poly_baseline(mask=msk, order=3)

        avp.set_freqframe('TOPO')
        avp.set_unit('km/s')
        avp.auto_sinusoid_baseline()
        rms = avp.stats("rms")
        mx = avp.stats("max_abc")
        assert_almost_equals(rms[0], 0.04148, 5)
        assert_almost_equals(mx[0], 86.67328, 5)
