import unittest

from pipeline.domain import AntennaArray
from pipeline.domain import Antenna
import pipeline.casatools as casatools
import pipeline.hif.tasks.referenceantenna as referenceantenna

class TestScoreByPosition(unittest.TestCase):
    def _create_antenna(self, ant_id, name, lon, lat, height):
        mt = casatools.measures   
        qt = casatools.quanta
        position = mt.position(v0=qt.quantity(lon, 'deg'),
                               v1=qt.quantity(lat, 'deg'),
                               v2=qt.quantity(height, 'm'))
        return Antenna(ant_id, name, position, 25)        

    def test_get_scores_by_separation(self):
        array = AntennaArray('name')
        a1=self._create_antenna(1, 'a1', 22, 20, 1);
        a2=self._create_antenna(2, 'a2', -11, -10, 1);
        a3=self._create_antenna(3, 'a3', 33, 30, 1);
        a4=self._create_antenna(4, 'a4', 0, 0, 1);
        a5=self._create_antenna(5, 'a5', 10, 10, 1);
        map(array.add_antenna, [a1, a2, a3, a4, a5])

        heuristic = referenceantenna.ScoreAntennaByPosition()        
        scores = heuristic.calculate(array)      

        self.assertEqual(scores[a5], 4)
        self.assertEqual(scores[a4], 3)
        self.assertEqual(scores[a1], 2)
        self.assertEqual(scores[a2], 1)
        self.assertEqual(scores[a3], 0)
