import unittest

import pipeline.domain.Antenna as Antenna
import pipeline.domain.AntennaArray as AntennaArray
import pipeline.infrastructure.casatools as casatools


class CasaTestCase(unittest.TestCase):
    def assertQuantitiesEqual(self, x, y, num_places=5):
        qt = casatools.quanta
        xc = qt.canonical(x)
        yc = qt.canonical(y)

        xv = qt.getvalue(xc)
        yv = qt.getvalue(yc)

        self.assertAlmostEqual(xv, yv, num_places)


class TestAntennaArray(CasaTestCase):
    def setUp(self):
        array = AntennaArray('name')
        self.a1=self._create_antenna(1, 'a1', 22, 20, 1);
        self.a2=self._create_antenna(2, 'a2', -11, -10, 1);
        self.a3=self._create_antenna(3, 'a3', 33, 30, 1);
        self.a4=self._create_antenna(4, 'a4', 0, 0, 1);
        self.a5=self._create_antenna(5, 'a5', 10, 10, 1);
        array.antennas.extend([self.a1, self.a2, self.a3, self.a4, self.a5])
        
        self.testee = array

    def _create_antenna(self, ant_id, name, lon, lat, height):
        mt = casatools.measures   
        qt = casatools.quanta
        position = mt.position(v0=qt.quantity(lon, 'deg'),
                               v1=qt.quantity(lat, 'deg'),
                               v2=qt.quantity(height, 'm'))
        diameter = 25
        return Antenna(ant_id, name, position, diameter)        

    def test_median_direction(self):
        qt = casatools.quanta
        d = self.testee.median_direction
        self.assertQuantitiesEqual(d['m0'], qt.quantity('10deg'))
        self.assertQuantitiesEqual(d['m1'], qt.quantity('10deg'))

 
if __name__ == "__main__":
    unittest.main()