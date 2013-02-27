from __future__ import absolute_import

class State(object):
    # dictionary to map from STATE table obs_mode to pipeline intent
    obs_mode_mapping = {
        'CALIBRATE_BANDPASS#ON_SOURCE'       : 'BANDPASS',
        'CALIBRATE_BANDPASS.ON_SOURCE'       : 'BANDPASS',
        'CALIBRATE_BANDPASS_ON_SOURCE'       : 'BANDPASS',
        'CALIBRATE_AMPLI#ON_SOURCE'          : 'AMPLITUDE',
        'CALIBRATE_AMPLI.ON_SOURCE'          : 'AMPLITUDE',
        'CALIBRATE_AMPLI_ON_SOURCE'          : 'AMPLITUDE',
        'CALIBRATE_PHASE#ON_SOURCE'          : 'PHASE',
        'CALIBRATE_PHASE.ON_SOURCE'          : 'PHASE',
        'CALIBRATE_PHASE_ON_SOURCE'          : 'PHASE',
        'CALIBRATE_TARGET#ON_SOURCE'         : 'TARGET',
        'CALIBRATE_TARGET.ON_SOURCE'         : 'TARGET',
        'CALIBRATE_TARGET_ON_SOURCE'         : 'TARGET',
        'CALIBRATE_POINTING#ON_SOURCE'       : 'POINTING',
        'CALIBRATE_POINTING.ON_SOURCE'       : 'POINTING',
        'CALIBRATE_POINTING_ON_SOURCE'       : 'POINTING',
        'CALIBRATE_WVR#ON_SOURCE'            : 'WVR',
        'CALIBRATE_WVR.ON_SOURCE'            : 'WVR',
        'CALIBRATE_WVR_ON_SOURCE'            : 'WVR',
        'CALIBRATE_ATMOSPHERE#ON_SOURCE'     : 'ATMOSPHERE',
        'CALIBRATE_ATMOSPHERE.ON_SOURCE'     : 'ATMOSPHERE',
        'CALIBRATE_ATMOSPHERE_ON_SOURCE'     : 'ATMOSPHERE',
        'CALIBRATE_SIDEBAND_RATIO#ON_SOURCE' : 'SIDEBAND',
        'CALIBRATE_SIDEBAND_RATIO.ON_SOURCE' : 'SIDEBAND',
        'CALIBRATE_SIDEBAND_RATIO_ON_SOURCE' : 'SIDEBAND',
        'OBSERVE_TARGET#ON_SOURCE'           : 'TARGET',
        'OBSERVE_TARGET.ON_SOURCE'           : 'TARGET',
        'OBSERVE_TARGET_ON_SOURCE'           : 'TARGET',
        'OBSERVE_TARGET#OFF_SOURCE'          : 'REFERENCE',
        'OBSERVE_TARGET.OFF_SOURCE'          : 'REFERENCE',
        'OBSERVE_TARGET_OFF_SOURCE'          : 'REFERENCE'     }
    
    def __init__(self, state_id, obs_mode):
        self.id = state_id
        self.obs_mode = obs_mode
    
    @property
    def intents(self):
        # return all intents
        return set([intent for mode, intent in self.obs_mode_mapping.iteritems()
                   if self.obs_mode.find(mode) != -1])

    def get_obs_mode_for_intent(self, intent):
        intents = set([i.strip('*') for i in intent.split(',') 
                       if i is not None])
        return [mode for mode, pipeline_intent in self.obs_mode_mapping.items()
                if pipeline_intent in intents
                and self.obs_mode.find(mode) != -1]

    def __repr__(self):
        return 'State(id={0}, intents={1}'.format(self.id, self.intents)
