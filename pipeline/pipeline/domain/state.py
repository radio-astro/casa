from __future__ import absolute_import
import datetime

import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)


class State(object):
    obs_mode_mapping = {}

    __slots__ = ('id', 'obs_mode')

    def __getstate__(self):
        return self.id, self.obs_mode

    def __setstate__(self, state):
        self.id, self.obs_mode = state
    
    def __init__(self, state_id, obs_mode):
        self.id = state_id
        # work around NumPy bug with empty strings
        # http://projects.scipy.org/numpy/ticket/1239
        self.obs_mode = str(obs_mode)

    def __repr__(self):
        return '{0}({1!r}, {2!r})'.format(
            self.__class__.__name__, self.id, self.obs_mode)

    @property
    def intents(self):
        # return all intents
        return set([intent for mode, intent in self.obs_mode_mapping.iteritems()
                   if self.obs_mode.find(mode) != -1])

    @property
    def reduction_intents(self):
        # get the raw intents from the ms
        raw_intents = self.intents

        # translate these into intents understood by the pipeline
        red_intents = set()
        if 'CHECK' in raw_intents:
            red_intents.update(['CHECK'])
        if 'POLARIZATION' in raw_intents:
            red_intents.update(['POLARIZATION'])
        if 'POLANGLE' in raw_intents:
            red_intents.update(['POLANGLE'])
        if 'POLLEAKAGE' in raw_intents:
            red_intents.update(['POLLEAKAGE'])
        if 'BANDPASS' in raw_intents:
            red_intents.update(['BANDPASS'])
        if 'AMPLITUDE' in raw_intents:
            red_intents.update(['AMPLITUDE'])
        if 'FLUX' in raw_intents:
            red_intents.update(['AMPLITUDE'])
        if 'TARGET' in raw_intents:
            red_intents.update(['TARGET'])
        if ('PHASE' in raw_intents) and \
          not ('BANDPASS' in raw_intents) and \
          not ('AMPLITUDE' in raw_intents) and \
          not ('FLUX' in raw_intents) and \
          not ('TARGET' in raw_intents):
            red_intents.update(['PHASE'])

        return red_intents

    def get_obs_mode_for_intent(self, intent):
        intents = set([i.strip('*') for i in intent.split(',') 
                       if i is not None])
        return [mode for mode, pipeline_intent in self.obs_mode_mapping.items()
                if pipeline_intent in intents
                and self.obs_mode.find(mode) != -1]

    def __str__(self):
        return '{0}(id={1}, intents={2})'.format(self.__class__.__name__, 
                                                 self.id, self.intents)


class StateALMA(State):
    # dictionary to map from STATE table obs_mode to pipeline intent
    obs_mode_mapping = {
        'CALIBRATE_POLARIZATION#ON_SOURCE'   : 'POLARIZATION',
        'CALIBRATE_POLARIZATION.ON_SOURCE'   : 'POLARIZATION',
        'CALIBRATE_POLARIZATION_ON_SOURCE'   : 'POLARIZATION',
        'CALIBRATE_POL_ANGLE#ON_SOURCE'      : 'POLANGLE',
        'CALIBRATE_POL_ANGLE.ON_SOURCE'      : 'POLANGLE',
        'CALIBRATE_POL_ANGLE_ON_SOURCE'      : 'POLANGLE',
        'CALIBRATE_POL_LEAKAGE#ON_SOURCE'    : 'POLLEAKAGE',
        'CALIBRATE_POL_LEAKAGE.ON_SOURCE'    : 'POLLEAKAGE',
        'CALIBRATE_POL_LEAKAGE_ON_SOURCE'    : 'POLLEAKAGE',
        'CALIBRATE_BANDPASS#ON_SOURCE'       : 'BANDPASS',
        'CALIBRATE_BANDPASS.ON_SOURCE'       : 'BANDPASS',
        'CALIBRATE_BANDPASS_ON_SOURCE'       : 'BANDPASS',
        'CALIBRATE_AMPLI#ON_SOURCE'          : 'AMPLITUDE',
        'CALIBRATE_AMPLI.ON_SOURCE'          : 'AMPLITUDE',
        'CALIBRATE_AMPLI_ON_SOURCE'          : 'AMPLITUDE',
        'CALIBRATE_FLUX#ON_SOURCE'           : 'AMPLITUDE',
        'CALIBRATE_FLUX.ON_SOURCE'           : 'AMPLITUDE',
        'CALIBRATE_FLUX_ON_SOURCE'           : 'AMPLITUDE',
        'CALIBRATE_PHASE#ON_SOURCE'          : 'PHASE',
        'CALIBRATE_PHASE.ON_SOURCE'          : 'PHASE',
        'CALIBRATE_PHASE_ON_SOURCE'          : 'PHASE',
        'CALIBRATE_TARGET#ON_SOURCE'         : 'TARGET',
        'CALIBRATE_TARGET.ON_SOURCE'         : 'TARGET',
        'CALIBRATE_TARGET_ON_SOURCE'         : 'TARGET',
        'CALIBRATE_POINTING#ON_SOURCE'       : 'POINTING',
        'CALIBRATE_POINTING.ON_SOURCE'       : 'POINTING',
        'CALIBRATE_POINTING_ON_SOURCE'       : 'POINTING',
        'CALIBRATE_FOCUS#ON_SOURCE'          : 'FOCUS',
        'CALIBRATE_FOCUS.ON_SOURCE'          : 'FOCUS',
        'CALIBRATE_FOCUS_ON_SOURCE'          : 'FOCUS',
        'CALIBRATE_WVR#ON_SOURCE'            : 'WVR',
        'CALIBRATE_WVR.ON_SOURCE'            : 'WVR',
        'CALIBRATE_WVR_ON_SOURCE'            : 'WVR',
        'CALIBRATE_ATMOSPHERE#ON_SOURCE'     : 'ATMOSPHERE',
        'CALIBRATE_ATMOSPHERE.ON_SOURCE'     : 'ATMOSPHERE',
        'CALIBRATE_ATMOSPHERE_ON_SOURCE'     : 'ATMOSPHERE',
        'CALIBRATE_ATMOSPHERE#AMBIENT'       : 'ATMOSPHERE',
        'CALIBRATE_ATMOSPHERE#HOT'           : 'ATMOSPHERE',
        'CALIBRATE_ATMOSPHERE#OFF_SOURCE'    : 'ATMOSPHERE',
        'CALIBRATE_ATMOSPHERE.OFF_SOURCE'    : 'ATMOSPHERE',
        'CALIBRATE_ATMOSPHERE_OFF_SOURCE'    : 'ATMOSPHERE',
        'CALIBRATE_SIDEBAND_RATIO#ON_SOURCE' : 'SIDEBAND',
        'CALIBRATE_SIDEBAND_RATIO.ON_SOURCE' : 'SIDEBAND',
        'CALIBRATE_SIDEBAND_RATIO_ON_SOURCE' : 'SIDEBAND',
        'CALIBRATE_SIDEBAND_RATIO#OFF_SOURCE': 'SIDEBAND',
        'CALIBRATE_SIDEBAND_RATIO.OFF_SOURCE': 'SIDEBAND',
        'CALIBRATE_SIDEBAND_RATIO_OFF_SOURCE': 'SIDEBAND',
        'CALIBRATE_DELAY#ON_SOURCE'          : 'CHECK',
        'CALIBRATE_DELAY.ON_SOURCE'          : 'CHECK',
        'CALIBRATE_DELAY_ON_SOURCE'          : 'CHECK',
        'OBSERVE_CHECK_SOURCE#ON_SOURCE'     : 'CHECK',
        'OBSERVE_CHECK_SOURCE.ON_SOURCE'     : 'CHECK',
        'OBSERVE_CHECK_SOURCE_ON_SOURCE'     : 'CHECK',
        'OBSERVE_TARGET#ON_SOURCE'           : 'TARGET',
        'OBSERVE_TARGET.ON_SOURCE'           : 'TARGET',
        'OBSERVE_TARGET_ON_SOURCE'           : 'TARGET',
        'OBSERVE_TARGET#OFF_SOURCE'          : 'REFERENCE',
        'OBSERVE_TARGET.OFF_SOURCE'          : 'REFERENCE',
        'OBSERVE_TARGET_OFF_SOURCE'          : 'REFERENCE'
    }
    
    def __init__(self, state_id, obs_mode):
        super(StateALMA, self).__init__(state_id, obs_mode)

        if 'CALIBRATE_FLUX' in obs_mode:
            LOG.trace('Translating %s intent to AMPLITUDE for state #%s'
                      '' % (obs_mode, state_id))


class StateALMACycle0(StateALMA):
    # Check whether these states co-exist with PHASE
    _PHASE_BYPASS_INTENTS = frozenset(('BANDPASS', 'AMPLITUDE'))

    def __init__(self, state_id, obs_mode):
        super(StateALMACycle0, self).__init__(state_id, obs_mode)
        
        # For Cycle 0, check whether this state has PHASE and another cal
        # intent. If so, the PHASE obsmode will be removed.
        
        # First collect the intents using the raw obsmodes recorded in the
        # state table.. 
        intents = self.intents
        # .. and test to see if any of these intents require phase removal
        has_bypass_intent = intents.isdisjoint(StateALMACycle0._PHASE_BYPASS_INTENTS)

        # if so, and PHASE is indeed included as an intent, ..
        if 'PHASE' in intents and not has_bypass_intent:
            LOG.info('Cycle 0 mislabeled data workaround: removing PHASE '
                     'intent for State %s' % self.id)
            
            # .. find the obs_mode(s) responsible for the addition of the
            # phase intent..
            phase_obs_modes = [k for k, v in self.obs_mode_mapping.items()
                               if v == 'PHASE']
            # and remove them from the obsmodes we should register
            dephased_obs_modes = [m for m in obs_mode.split(',')
                                  if m not in phase_obs_modes]

            LOG.trace('Before: %s' % self.__repr__())
            # .. so that in resetting this object's obs_modes to the 
            # corrected value, we remove the registration of the pipeline
            # PHASE intent
            self.obs_mode = ','.join(dephased_obs_modes)
            LOG.trace('After: %s' % self.__repr__())


class StateVLA(State):
    # dictionary to map from STATE table obs_mode to pipeline intent
    obs_mode_mapping = {
        'CALIBRATE_POLARIZATION#ON_SOURCE'   : 'POLARIZATION',
        'CALIBRATE_POLARIZATION.ON_SOURCE'   : 'POLARIZATION',
        'CALIBRATE_POLARIZATION_ON_SOURCE'   : 'POLARIZATION',
        'CALIBRATE_POLARIZATION#UNSPECIFIED' : 'POLARIZATION',
        'CALIBRATE_POL_ANGLE#ON_SOURCE'      : 'POLANGLE',
        'CALIBRATE_POL_ANGLE.ON_SOURCE'      : 'POLANGLE',
        'CALIBRATE_POL_ANGLE_ON_SOURCE'      : 'POLANGLE',
        'CALIBRATE_POL_ANGLE#UNSPECIFIED'    : 'POLANGLE',
        'CALIBRATE_POL_LEAKAGE#ON_SOURCE'    : 'POLLEAKAGE',
        'CALIBRATE_POL_LEAKAGE.ON_SOURCE'    : 'POLLEAKAGE',
        'CALIBRATE_POL_LEAKAGE_ON_SOURCE'    : 'POLLEAKAGE',
        'CALIBRATE_POL_LEAKAGE#UNSPECIFIED'  : 'POLLEAKAGE',
        'CALIBRATE_BANDPASS#ON_SOURCE'       : 'BANDPASS',
        'CALIBRATE_BANDPASS.ON_SOURCE'       : 'BANDPASS',
        'CALIBRATE_BANDPASS_ON_SOURCE'       : 'BANDPASS',
        'CALIBRATE_AMPLI#ON_SOURCE'          : 'PHASE',  # Was amplitude
        'CALIBRATE_AMPLI.ON_SOURCE'          : 'PHASE',  # Was amplitude
        'CALIBRATE_AMPLI_ON_SOURCE'          : 'PHASE',  # Was amplitude
        'CALIBRATE_FLUX#ON_SOURCE'           : 'AMPLITUDE',
        'CALIBRATE_FLUX.ON_SOURCE'           : 'AMPLITUDE',
        'CALIBRATE_FLUX_ON_SOURCE'           : 'AMPLITUDE',
        'CALIBRATE_PHASE#ON_SOURCE'          : 'PHASE',
        'CALIBRATE_PHASE.ON_SOURCE'          : 'PHASE',
        'CALIBRATE_PHASE_ON_SOURCE'          : 'PHASE',
        'CALIBRATE_TARGET#ON_SOURCE'         : 'TARGET',
        'CALIBRATE_TARGET.ON_SOURCE'         : 'TARGET',
        'CALIBRATE_TARGET_ON_SOURCE'         : 'TARGET',
        'CALIBRATE_POINTING#ON_SOURCE'       : 'POINTING',
        'CALIBRATE_POINTING.ON_SOURCE'       : 'POINTING',
        'CALIBRATE_POINTING_ON_SOURCE'       : 'POINTING',
        'CALIBRATE_FOCUS#ON_SOURCE'          : 'FOCUS',
        'CALIBRATE_FOCUS.ON_SOURCE'          : 'FOCUS',
        'CALIBRATE_FOCUS_ON_SOURCE'          : 'FOCUS',
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
        'OBSERVE_TARGET#UNSPECIFIED'         : 'TARGET',
        'OBSERVE_TARGET#OFF_SOURCE'          : 'REFERENCE',
        'OBSERVE_TARGET.OFF_SOURCE'          : 'REFERENCE',
        'OBSERVE_TARGET_OFF_SOURCE'          : 'REFERENCE',
        'CALIBRATE_BANDPASS#UNSPECIFIED'     : 'BANDPASS',    
        'CALIBRATE_FLUX#UNSPECIFIED'         : 'AMPLITUDE',
        'CALIBRATE_PHASE#UNSPECIFIED'        : 'PHASE',
        'CALIBRATE_AMPLI#UNSPECIFIED'        : 'PHASE',  # Was amplitude
        'UNSPECIFIED#UNSPECIFIED'            : 'UNSPECIFIED#UNSPECIFIED',
        'SYSTEM_CONFIGURATION'               : 'SYSTEM_CONFIGURATION',
        'SYSTEM_CONFIGURATION#UNSPECIFIED'   : 'SYSTEM_CONFIGURATION'
    }
    
    def __init__(self, state_id, obs_mode):
        super(StateVLA, self).__init__(state_id, obs_mode)


class StateAPEX(State):
    # dictionary to map from STATE table obs_mode to pipeline intent
    obs_mode_mapping = {
        'OBSERVE_TARGET#ON_SOURCE': 'TARGET'
    }
    
    def __init__(self, state_id, obs_mode):
        super(StateAPEX, self).__init__(state_id, obs_mode)


class StateSMT(State):
    # dictionary to map from STATE table obs_mode to pipeline intent
    obs_mode_mapping = {
        'OBSERVE_TARGET#ON_SOURCE': 'TARGET'
    }
    
    def __init__(self, state_id, obs_mode):
        super(StateSMT, self).__init__(state_id, obs_mode)


class StateNAOJ(State):
    # dictionary to map from STATE table obs_mode to pipeline intent
    obs_mode_mapping = {
        'OBSERVE_TARGET#ON_SOURCE'           : 'TARGET',
        'OBSERVE_TARGET#OFF_SOURCE'          : 'REFERENCE',
        'CALIBRATE_ATMOSPHERE#R_SOURCE'      : 'ATMOSPHERE',
        'CALIBRATE_ATMOSPHERE#SKY_SOURCE'    : 'ATMOSPHERE',
        'CALIBRATE_ATMOSPHERE#ZERO_SOURCE'   : 'ATMOSPHERE'
    }
    
    def __init__(self, state_id, obs_mode):
        super(StateNAOJ, self).__init__(state_id, obs_mode)


class StateFactory(object):
    def __init__(self, observatory, start=None):
        if observatory == 'ALMA':
            if start and start < datetime.datetime(2013, 1, 21):
                self._constructor = StateALMACycle0
            else:
                self._constructor = StateALMA
        elif observatory == 'VLA' or observatory == 'EVLA':
            self._constructor = StateVLA
        elif observatory == 'APEX':
            self._constructor = StateAPEX
        elif observatory == 'SMT':
            self._constructor = StateSMT
        elif observatory == 'NRO' or observatory == 'ASTE':
            self._constructor = StateNAOJ
        else:
            raise KeyError('%s has no matching State class' % observatory)

    def create_state(self, state_id, obs_mode):
        return self._constructor(state_id, obs_mode)
