import unittest

import pipeline.domain.Field as Field


class TestField(unittest.TestCase):
    def test_set_source_type(self):
        f = Field(1, 'field 1', 2, 3)

        for source_type, intent in [ (' tArGeT ', 'SOURCE'),
                                     (' pHaSe ', 'GAIN'),
                                     (' aMpLiTuDe ', 'FLUX')]:
            f.set_source_type(source_type)
            self.assertTrue(intent in f.intents, 
                            '{0} not in intents'.format(intent))
        self.assertEqual(3, len(f.intents))
        
    def test_clean_name(self):
        f = Field(1, 't-e+s(t)! ', 2, 3)
        self.assertEqual('t_e_s_t_!_', f.clean_name)

    def test_multiple_intents(self):
        f = Field(1, 'Field 1', 2, 3)
        f.set_source_type('BANDPASS+FLUX')
        for intent in f.intents:
            self.assertTrue(intent in ['BANDPASS', 'FLUX'])
        self.assertEqual(2, len(f.intents))

if __name__ == "__main__":
    unittest.main()
