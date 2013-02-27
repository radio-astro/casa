import numpy

import pipeline.infrastructure.api as api
import pipeline.infrastructure.casatools as casatools

# spw is calculated by CASA syntax parser now..
#class Spw(api.Heuristic):
#    def calculate(self, msobject, spw):
#        
#        spws = msobject.spectral_windows
#        if spw is None or spw=='':
#            spw = [s.id for s in spws if s.num_channels != 4]
#        else:
#            spw = spw.split(',')
#            spw = map(int, spw)
#            spw = set(spw)
#
#            for s in spws:
#                if s.num_channels == 4:
#                    spw.discard(s)
#
#        spw = ','.join(str(s) for s in spw)
#   
#        return spw


class BestSpwForBandpassHeuristic(api.Heuristic):
    def calculate(self, measurement_set, frequency_group, intent):
        """Method to return the SpW best suited to being used to phase-up a
        calibration group for the given intent.
    
        It returns the highest bandwidth spectral window in the continuum
        group or, if a line observation, the last window in the line group.
    
        Keyword arguments:
        measurement_set -- name of the measurement set
        frequency_group -- The FrequencyGroup to select from 
        field           -- The field of interest for this calibration.    
        """
        # This is a pipeline-provided heuristic, and we're comfortable using
        # the pipeline domain objects, so use them instead of the raw
        # primitives
        frequency_group = self._frequency_group
        ms = frequency_group.measurement_set

        # get the state IDs containing this calibration intent 
        state_ids = [state.id for state in ms.states 
                     if intent in state.intents]
        calibrator_taql = 'STATE_ID IN {0}'.format(state_ids)

        if not state_ids:
            # VLA datasets have an empty STATE table, in which case we need to
            # find the calibrator observations by field
            field_ids = [field.id for field in ms.fields
                         if intent in field.intents]
            calibrator_taql = 'FIELD_ID IN {0}'.format(field_ids)

        # Candidate phase-up spectral windows are continuum windows.. 
        spw_pool = frequency_group.get_continuum_windows()
        # .. unless there are no continuum windows, in which case use the line
        # windows
        if not spw_pool:
            spw_pool = frequency_group.get_line_windows()
        
        # find out how many data rows exist for each window in the pool for 
        # for the calibrator
        spw_and_rows = []
        with casatools.TableReader(ms.name) as table:
            for spw in spw_pool:
                # get the data description for this spectral window
                dd = ms.get_data_description(spw)

                subTable = table.query(
                    'DATA_DESC_ID==%s && %s '
                    '&& NOT(FLAG_ROW) && NOT(ALL(FLAG))' % (dd.id,
                                                            calibrator_taql))
                spw_and_rows.append((spw, subTable.nrows()))
		subTable.close()

        num_rows = [num_rows for (spw, num_rows) in spw_and_rows]
        threshold = 0.7 * numpy.median(num_rows)

        # remove spw with more flagged rows than the threshold
        spw_pool = [spw for (spw, num_rows) in spw_and_rows 
                    if num_rows > threshold]

        # desired spectral window is the remaining window with the highest
        # bandwidth
        by_bandwidth = sorted(spw_pool, 
                              key=lambda spw: spw.bandwidth,
                              reverse=True)

        if by_bandwidth:
            return by_bandwidth[0].id
        else:
            return None

    def parse_calculation(self, frequency_group, intent):
        self._frequency_group = frequency_group
        
        # convert the pipeline-domain object to primitives..
        ms_name = frequency_group.measurement_set.name
        group_id = frequency_group.id
        # . and call the heuristic, ..
        spw_id = self.calculate(ms_name, group_id, intent)
        # .. finally converting back to the pipeline domain

        return frequency_group.get_spectral_window(spw_id)
