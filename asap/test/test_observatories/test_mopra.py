#!/usr/bin/env python
import os
from asap import *
from nose.tools import assert_almost_equals
rcParams['verbose'] = 0
rcParams['plotter.gui'] = 0


DATA=os.path.join(os.path.split(__file__)[0], "..", "data", 
                  "mopra_dualif.rpf")


class TestMopra(object):
    def setup(self):
        self.st = scantable(DATA)

    def _plotfit(self, scan, ifno):
        scan.set_selection(ifs=ifno)
        f = fitter()
        f.set_scan(scan)
        f.set_function(gauss=ifno+1) # fit gaussian(s)
        f.fit()
        f.plot(filename='output/moprafit_if%d.png' % ifno)
        fp = f.get_parameters()
        assert True
        
    def test_reduction(self):
        restfreqs = [110.201, 86.243]     # 13CO-1/0, SiO the two IF
        self.st.set_restfreqs(restfreqs,"GHz")
        q = self.st.auto_quotient()
        q.set_unit("km/s")               # set the unit to be used from now on
        q.set_freqframe("LSRK")          # set frequency frame
        q.auto_poly_baseline(insitu=True)      # determine and subtract a poly 
        # baseline automatically
        q.convert_flux() # K -> Jy
        msk = q.create_mask([-70,20], [20,70]) # line free region - two windows
        rms = q.stats("rms", msk)
        med = q.stats("median", msk)
        regrms = [35.801266, 38.808666]
        regmed = [1.197934, 2.593804]
        for i in range(len(rms)):
            assert_almost_equals(rms[i], regrms[i], 6)
            assert_almost_equals(med[i], regmed[i], 6)

        for ifno in q.getifnos():
            self._plotfit(q, ifno)
