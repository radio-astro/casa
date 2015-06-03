import unittest

import pipeline.domain.Antenna as Antenna
import pipeline.infrastructure.casatools as casatools


class CasaTestCase(unittest.TestCase):
    def assertQuantitiesEqual(self, x, y, num_places=5):
        qt = casatools.quanta
        xc = qt.canonical(x)
        yc = qt.canonical(y)

        xv = qt.getvalue(xc)
        yv = qt.getvalue(yc)

        self.assertAlmostEqual(xv, yv, num_places)

class TestAntenna(CasaTestCase):
    def setUp(self):
        pass
        
    def test_init(self):
        mt = casatools.measures
        qt = casatools.quanta
        
        v0 = qt.quantity('12deg')
        v1 = qt.quantity('34deg')
        v2 = qt.quantity('100m')

        a = Antenna(1, 'ant1', mt.position(v0=v0, v1=v1, v2=v2), 25)                
        self.assertEqual(1, a.id)
        self.assertEqual('ant1', a.name)
        self.assertQuantitiesEqual(a.longitude, v0)
        self.assertQuantitiesEqual(a.latitude, v1)
        self.assertQuantitiesEqual(a.height, v2)

        d = a.direction
        self.assertQuantitiesEqual(d['m0'], v0)
        self.assertQuantitiesEqual(d['m1'], v1)

if __name__ == "__main__":
    unittest.main()
