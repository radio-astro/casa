import unittest

import pipeline.domain.FrequencyGroup as FrequencyGroup
import pipeline.domain.SpectralWindow as SpectralWindow


class TestFrequencyGroup(unittest.TestCase):
    def setUp(self):
        self.testee = FrequencyGroup(1, 'Group 1')
        rf = '1234' # ref frequency
        self.wvr_windows = [SpectralWindow(1, 4,  1, rf),
                            SpectralWindow(2, 4, 10, rf)]
        self.continuum_windows = [SpectralWindow(3, 1, 100, rf),
                                  SpectralWindow(4, 1,  95, rf),
                                  SpectralWindow(5, 1,  97, rf),
                                  SpectralWindow(6, 1, 102, rf)]
        self.line_windows = [SpectralWindow(7, 10, 1, rf),
                             SpectralWindow(8, 20, 1, rf),
                             SpectralWindow(9, 1,  1, rf)]
        
        map(self.testee.add_spw, self.wvr_windows)
        map(self.testee.add_spw, self.continuum_windows)
        map(self.testee.add_spw, self.line_windows)

    def test_line_windows_are_identified_correctly(self):
        for spw in self.testee.get_line_windows():
            self.assertTrue(spw in self.line_windows)
            self.assertFalse(spw in self.continuum_windows)
            self.assertFalse(spw in self.wvr_windows)

    def test_continuum_windows_are_identified_correctly(self):
        for spw in self.testee.get_continuum_windows():
            self.assertFalse(spw in self.line_windows)
            self.assertTrue(spw in self.continuum_windows)
            self.assertFalse(spw in self.wvr_windows)

    def test_line_windows_are_sorted_by_bandwidth(self):
        bandwidths = [spw.bandwidth for spw in self.testee.get_line_windows()]
        for i in range(len(bandwidths)-1):
            self.assertTrue(bandwidths[i] <= bandwidths[i+1])
        
    def test_continuum_windows_are_sorted_by_bandwidth(self):
        bandwidths = [spw.bandwidth for spw in self.testee.get_continuum_windows()]
        for i in range(len(bandwidths)-1):
            self.assertTrue(bandwidths[i] <= bandwidths[i+1])


if __name__ == "__main__":
    unittest.main()