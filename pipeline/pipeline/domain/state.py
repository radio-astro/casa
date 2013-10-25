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
    
    def __init__(self, state_id, obs_mode, is_cycle0=False):
        self.id = state_id
        # work around NumPy bug with empty strings
        # http://projects.scipy.org/numpy/ticket/1239
        self.obs_mode = str(obs_mode)
    
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

    def __repr__(self):
        return '{0}(id={1}, intents={2})'.format(self.__class__.__name__, 
                                                 self.id, self.intents)


class StateALMA(State):
    # dictionary to map from STATE table obs_mode to pipeline intent
    obs_mode_mapping = {
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
        'CALIBRATE_SIDEBAND_RATIO#ON_SOURCE' : 'SIDEBAND',
        'CALIBRATE_SIDEBAND_RATIO.ON_SOURCE' : 'SIDEBAND',
        'CALIBRATE_SIDEBAND_RATIO_ON_SOURCE' : 'SIDEBAND',
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
    _PHASE_BYPASS_INTENTS = frozenset(('BANDPASS','AMPLITUDE'))

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
            phase_obs_modes = [k for k,v in self.obs_mode_mapping.items()
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
        'CALIBRATE_FLUX#UNSPECIFIED'         : 'AMPLITUDE',
        'CALIBRATE_BANDPASS#UNSPECIFIED'     : 'BANDPASS',    
        'CALIBRATE_FLUX#UNSPECIFIED'         : 'AMPLITUDE',
        'CALIBRATE_PHASE#UNSPECIFIED'        : 'PHASE',
        'CALIBRATE_AMPLI#UNSPECIFIED'        : 'AMPLITUDE'
    }
    
    def __init__(self, state_id, obs_mode):
        super(StateVLA, self).__init__(state_id, obs_mode)


class StateFactory(object):
    def __init__(self, observatory, start=None):
        if observatory == 'ALMA':
            if start and start < datetime.datetime(2013,01,21):
                self._constructor = StateALMACycle0
            else:
                self._constructor = StateALMA
        elif (observatory == 'VLA' or observatory =='EVLA'):
            self._constructor = StateVLA
        else:
            raise KeyError('%s has no matching State class' % observatory)

    def create_state(self, state_id, obs_mode):
        return self._constructor(state_id, obs_mode)
    
