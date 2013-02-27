import numpy as np
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging

LOG = logging.get_logger('pipeline.hif.heuristics.wvrgcal')

class WvrgcalHeuristics(object):
    def __init__(self, context, vis, hm_tie, tie, hm_smooth, smooth,
      sourceflag, nsol, segsource):
        self.vis = vis
        self.hm_smooth = hm_smooth
        self._smooth = smooth

        ms = context.observing_run.get_ms(name=vis)

        # get info on all spectral windows
        self.spws = ms.spectral_windows
        for spw in self.spws:
            LOG.debug('SpW id:%s nchan:%s intents:%s' % (spw.id, spw.channels,
              spw.intents))

        # Get the data_desc_ids of the wvr data.
        wvr_spw = [spw for spw in self.spws
          if spw.channels==4 and spw.intents != set()]
        if wvr_spw == []:
            LOG.info('assuming WVR spw.id=0')
            wvr_spw = [spw for spw in self.spws if spw.id==0]
        wvr_dd_ids = [ms.get_data_description(spw).id for spw in wvr_spw]
        LOG.info('WVR data_desc_id is %s' % wvr_dd_ids)

        # now get the science spws, those used for scientific intent
        science_spws = [spw for spw in self.spws if 
          spw.channels not in (1,4) and not spw.intents.isdisjoint(
          ['BANDPASS', 'AMPLITUDE', 'PHASE', 'TARGET'])]
        LOG.info('science spws are: %s' % [spw.id for spw in science_spws])

        # looking at just PHASE and TARGET intents, find the integration times
        # associated with wvr data and with science data for each data_desc
        science_field_ids = [field.id for field in ms.fields
          if not set(field.intents).isdisjoint(['PHASE', 'TARGET'])]
        science_state_ids = [state.id for state in ms.states
          if not set(state.intents).isdisjoint(['PHASE', 'TARGET'])]

        with casatools.TableReader(ms.name) as table:
            taql = '''(STATE_ID IN %s AND FIELD_ID IN %s AND
              DATA_DESC_ID in %s)''' % (science_state_ids, science_field_ids,
              wvr_dd_ids)
            subtable = table.query(taql)
            integration = subtable.getcol('INTERVAL')
            self.wvr_integration = np.median(integration)
            wvr_max_integration = np.max(integration)
            wvr_min_integration = np.min(integration)
            subtable.done()

        LOG.info('median WVR integration time: %s' % self.wvr_integration)
        if wvr_max_integration != self.wvr_integration:
            LOG.warning('max WVR integration time is different: %s' %
              wvr_max_integration)
        if wvr_min_integration != self.wvr_integration:
            LOG.warning('min WVR integration time is different: %s' %
              wvr_min_integration)

        self.science_integration = {}
        science_max_integration = {}
        science_min_integration = {}
        with casatools.TableReader(ms.name) as table:
            for spw in science_spws:
                dd_id = ms.get_data_description(spw).id
                taql = '''(STATE_ID IN %s AND FIELD_ID IN %s AND
                  DATA_DESC_ID in %s)''' % (science_state_ids,
                  science_field_ids, dd_id)
                subtable = table.query(taql)
                integration = subtable.getcol('INTERVAL')

                self.science_integration[spw.id] = np.median(integration)
                science_max_integration[spw.id] = np.max(integration)
                science_min_integration[spw.id] = np.min(integration)
                LOG.info('SpW %s median WVR integration time: %s' %
                  (spw.id, self.science_integration[spw.id]))

                if self.science_integration[spw.id] != \
                  science_max_integration[spw.id]:
                    LOG.warning('max WVR integration time is different: %s' %
                      science_max_integration[spw.id])
                if self.science_integration[spw.id] != \
                  science_min_integration[spw.id]:
                    LOG.warning('min WVR integration time is different: %s' %
                      science_max_integration[spw.id])

                # free the resources held by the sub-table
                subtable.done()

        if hm_tie == 'manual':
            self._tie = tie
        else:
            # get target names and directions
            targets = [(field.name, field.mdirection) for field in ms.fields if
              'TARGET' in field.intents and 
              'WVR' in field.intents]

            # tie all target fields as assume they are close to each other
            tied_targets = [target[0] for target in targets]
        
            # get names and directions of phase calibrators
            phases = [(field.name, field.mdirection) for field in ms.fields if
              'PHASE' in field.intents and 
              'WVR' in field.intents and 
              'BANDPASS' not in field.intents and
              'AMPLITUDE' not in field.intents]

            # add phase calibrator to tie if it's less than 15 degress from
            # target
            tied_phases = []
            for phase in phases:
                separation = casatools.measures.separation(phase[1],
                  targets[0][1])
                LOG.info('Calibrator: %s distance from target: %s%s' % (phase[0],
                  separation['value'], separation['unit']))
                if casatools.quanta.le(separation, '15deg'):
                    tied_phases.append(phase[0])
            LOG.info('phase calibrators tied to target: %s' % tied_phases)

            # assemble the full tie
            tied = tied_phases + tied_targets

            # and format it
            if len(tied) > 1:
                # eliminate duplicate names, remove quotes from names; these 
                # cause wvrgcal to segfault
                self._tie = set()
                for name in set(tied):
                    name = name.replace('"', '')
                    name = name.replace("'", "")
                    self._tie.add(name)                
 
                self._tie = ','.join([name for name in self._tie])
                self._tie = ['%s' % self._tie]
            else:
                self._tie = []

        # sort out segsource whose valid value is dependent on tie
        if self._tie != []:
            self._segsource = True
        else:
            if segsource is None:
                self._segsource = True
            else:
                self._segsource = segsource

        # sort out sourceflag whose valid value is dependent on segsource
        if sourceflag is None:
            self._sourceflag = []
        else:
            self._sourceflag = sourceflag
        if self._sourceflag != [] and not self._segsource:
            raise Exception, \
              'WvrgHeuristics:sourceflag incompatible with segsource False'  
        
        if nsol is None:
            self._nsol = 1
        else:
            self._nsol = nsol
        if self._nsol != 1 and self._segsource:
            raise Exception, \
              'WvrgHeuristics: nsol <>1 incompatible with segsource True'  

    def nsol(self):
        return self._nsol

    def segsource(self):
        return self._segsource

    def smooth(self, spw):
        if self.hm_smooth == 'automatic':
            temp = int(self.science_integration[spw] / self.wvr_integration)
            # 1 is the minimum sensible value for smooth
            temp = max(temp, 1)
            return temp        
        else:
            return self._smooth

    def sourceflag(self):
        return self._sourceflag

    def tie(self):
        return self._tie

    def toffset(self):
        return -1
