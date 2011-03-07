import sys
import os
import shutil
import datetime
from asap import scantable, selector, mask_not
from asap.logging import asaplog
asaplog.disable()

from nose.tools import *

def tempdir_setup():
    os.makedirs("test_temp")

def tempdir_teardown():
    shutil.rmtree("test_temp", True)

class TestNRO(object):
    def setup(self):
        s = scantable('data/B68test.nro', average=False)
        sel = selector()
        # make sure this order is always correct - it can be random
        sel.set_order(["SCANNO", "POLNO"])
        s.set_selection(sel)
        self.st = s.copy()

    def test_init(self):
        assert_equal(self.st.nrow(), 36)
        assert_equal(self.st.get_fluxunit(), 'K')
        assert_equal(self.st.nchan(), 2048)
        assert_equal(self.st.nif(), 4)

    def test_spectrum(self):
        import numpy
        sp=numpy.array(self.st._getspectrum(0))
        assert_equal(sp.max(),1.9524071216583252)
        assert_equal(sp.min(),-0.0071961274370551109)
        assert_equal(sp.argmax(),1648)
        assert_equal(sp.argmin(),1614)

    def test_stats(self):
        assert_equal(self.st.stats('rms')[0],1.1064267158508301)
        assert_equal(self.st.stats('stddev')[0],0.29552212357521057)
        assert_equal(self.st.stats('median')[0],1.0714811086654663)
        assert_equal(self.st.stats('mean')[0],1.0662506818771362)
        assert_equal(self.st.stats('sum')[0],2183.681396484375)

    def test_frequency(self):
        rf=self.st.get_restfreqs()
        assert_equal(len(rf),2)
        assert_equal(rf[0][0],85162157000.0)
        assert_equal(rf[1][0],86754330000.0)
        self.st.set_unit('GHz')
        abc=self.st._getabcissa(0)
        assert_equal(abc[0],85.17337639557438)
