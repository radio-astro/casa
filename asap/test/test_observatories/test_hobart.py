#!/usr/bin/env python
import os
from asap import *
from nose.tools import assert_almost_equals
rcParams['verbose'] = 0
rcParams['plotter.gui'] = 0
# HOBART specific
rcParams["scantable.reference"] = ".*(e|w|_R|\+R|\-R)$"

if not os.path.exists("output"):
    os.makedirs("output")

DATA=os.path.join(os.path.split(__file__)[0], "..", "data", 
                  "hobart.rpf")

class TestHobart(object):
    def setup(self):
        self.st = scantable(DATA)

    def _plotfit(self, scan, polno):
        scan.set_selection(pols=[polno])
        f = fitter()
        f.set_scan(scan)
        f.set_function(gauss=1) # fit gaussian(s)
        f.set_parameters(params=[2,11,1])
        f.fit()
        f.plot(filename='output/hobartfit_pol%d.png' % polno)
        fp = f.get_parameters()
        assert True
        
    def test_reduction(self):
        q = self.st.auto_quotient()
        q.set_unit("km/s")               # set the unit to be used from now on
        q.set_freqframe("LSRK")          # set frequency frame
        q.auto_poly_baseline(insitu=True, order=1)
        q.set_selection(scans=[0])
        for polno in q.getpolnos():
            self._plotfit(q, polno)
