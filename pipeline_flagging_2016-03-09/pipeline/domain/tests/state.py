import unittest

import pipeline.domain.State as State


class TestState(unittest.TestCase):
    def test_intents_from_single_obsmode(self):
        test_args = [ ('CALIBRATE_BANDPASS#ON_SOURCE'   , ['BANDPASS']),
                      ('CALIBRATE_BANDPASS.ON_SOURCE'   , ['BANDPASS']),
                      ('CALIBRATE_BANDPASS_ON_SOURCE'   , ['BANDPASS']),
                      ('CALIBRATE_AMPLI#ON_SOURCE'      , ['AMPLITUDE']),
                      ('CALIBRATE_AMPLI.ON_SOURCE'      , ['AMPLITUDE']),
                      ('CALIBRATE_AMPLI_ON_SOURCE'      , ['AMPLITUDE']),
                      ('CALIBRATE_PHASE#ON_SOURCE'      , ['PHASE']),
                      ('CALIBRATE_PHASE.ON_SOURCE'      , ['PHASE']),
                      ('CALIBRATE_PHASE_ON_SOURCE'      , ['PHASE']),
                      ('CALIBRATE_TARGET#ON_SOURCE'     , ['TARGET']),
                      ('CALIBRATE_TARGET.ON_SOURCE'     , ['TARGET']),
                      ('CALIBRATE_TARGET_ON_SOURCE'     , ['TARGET']),
                      ('CALIBRATE_POINTING#ON_SOURCE'   , ['POINTING']),
                      ('CALIBRATE_POINTING.ON_SOURCE'   , ['POINTING']),
                      ('CALIBRATE_POINTING_ON_SOURCE'   , ['POINTING']),
                      ('CALIBRATE_WVR#ON_SOURCE'        , ['WVR']),
                      ('CALIBRATE_WVR.ON_SOURCE'        , ['WVR']),
                      ('CALIBRATE_WVR_ON_SOURCE'        , ['WVR']),
                      ('CALIBRATE_ATMOSPHERE#ON_SOURCE' , ['ATMOSPHERE']),
                      ('CALIBRATE_ATMOSPHERE.ON_SOURCE' , ['ATMOSPHERE']),
                      ('CALIBRATE_ATMOSPHERE_ON_SOURCE' , ['ATMOSPHERE']),
                      ('CALIBRATE_SIDEBAND_RATIO#ON_SOURCE' , ['SIDEBAND']),
                      ('CALIBRATE_SIDEBAND_RATIO.ON_SOURCE' , ['SIDEBAND']),
                      ('CALIBRATE_SIDEBAND_RATIO_ON_SOURCE' , ['SIDEBAND'])]
        
        for (obs_mode, expected) in test_args:
            state = State(1, obs_mode)
            self.assertEqual(expected, state.intents)

    def test_intents_from_multiple_obsmodes(self):
        # now for something more complex.
        state = State(1, ('CALIBRATE_BANDPASS#ON_SOURCE,'
                          'CALIBRATE_WVR.ON_SOURCE,'
                          'CALIBRATE_TARGET_ON_SOURCE'))
        
        expected = ['BANDPASS', 'WVR', 'TARGET']
        for intent in state.intents:
            self.assertTrue(intent in expected)
        self.assertEqual(3, len(state.intents))

if __name__ == "__main__":
    unittest.main()
