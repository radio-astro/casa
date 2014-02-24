from __main__ import default
from tasks import *
from taskinit import *
import unittest
import copy
import math

class me_shift_test(unittest.TestCase):

    def setUp(self):
        pass
    
    def tearDown(self):
        pass

    def test_shift(self):
        """Test me.shift"""
        v = me.direction("J2000", "4h20m30s", "+30.20.30")
        got = me.shift(v, "20arcmin", "0deg")
        expec = copy.deepcopy(v)
        expec['m1'] = qa.add(expec['m1'], "20arcmin")
        self.assertTrue(got == expec)
        got = me.shift(v, "20arcmin", "90deg")
        expec = 1.1433867531223854
        self.assertTrue(abs(got['m0']['value']/expec - 1) < 1e-7)
        expec = 0.5295520783025025
        self.assertTrue(abs(got['m1']['value']/expec - 1) < 1e-7)
        got = me.shift(v, "20arcmin", "180deg")
        self.assertTrue(got['m0']['value'] == v['m0']['value'])
        expec = qa.sub(v['m1'], '20arcmin')
        self.assertTrue(abs(got['m1']['value']/expec['value'] - 1) < 1e-7)
        
def suite():
    return [me_shift_test]
