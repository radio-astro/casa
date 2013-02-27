from __main__ import default
from tasks import *
from taskinit import *
from tests.test_split import check_eq, datapath
import unittest

'''
Unit tests for the measures tool.

Features tested:
  1. me.cometdist
  2. me.cometangdiam
'''

class Ganymede(unittest.TestCase):
    """
    Base class for Ganymede tests.
    """
    def setUp(self):
        cometdir = os.getenv("CASAPATH").split()[0] + "/data/ephemerides/JPL-Horizons/"
        me.framecomet(cometdir + "Ganymede_55437-56293dUTC.tab")
        me.doframe(me.epoch("utc", "2011/01/03/17:00:00"))
        me.doframe(me.observatory("ALMA"))

    def tearDown(self):
        pass

class me_test_cometdist(Ganymede):
    def test_cometdist(self):
        """Is the distance to Ganymede just right?"""
        check_eq(me.cometdist(), {'unit': 'AU', 'value': 5.1241}, 0.001)
            
class me_test_cometangdiam(Ganymede):
    def test_cometangdiam(self):
        """Is Ganymede's angular diameter correct?"""
        check_eq(me.cometangdiam(), {'unit': 'rad', 'value': 6.868e-06},
                 1.0e-9)

def suite():
    return [me_test_cometdist, me_test_cometangdiam]
