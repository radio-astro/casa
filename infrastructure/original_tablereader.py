from __future__ import absolute_import
import itertools
import os
import string

import numpy

import pipeline.infrastructure.casatools as casatools
import pipeline.domain as domain
import pipeline.domain.measures as measures
import pipeline.domain.virtualms as virtualms
#import pipeline.heuristics as heuristics
import pipeline.hsd.heuristics as sdheuristics
import pipeline.infrastructure.logging as logging

LOG = logging.get_logger(__name__)


def _get_ms_name(ms):
    return ms.name if isinstance(ms, domain.MeasurementSet) else ms


class ObservingRunReader(object):
    @staticmethod
    def get_observing_run(ms_files):
        observing_run = domain.ObservingRun()
        for ms_file in ms_files:
            ms = MeasurementSetReader.get_measurement_set(ms_file)
            observing_run.add_measurement_set(ms)
        return observing_run

    @staticmethod
    def get_observing_run_for_sd(st_files, ms_files=[], st_ms_map=[]):
        observing_run = domain.singledish.ScantableList()
        if ms_files is None or len(ms_files) == 0:
            for st_file in st_files:
                st = ScantableReader.get_scantable(st_file)
                ms = VirtualMeasurementSetFiller.get_measurement_set(st)
                observing_run.append(st)
                observing_run.add_measurement_set(ms)
        else:
            for ms_file in ms_files:
                ms = MeasurementSetReader.get_measurement_set(ms_file)
                observing_run.add_measurement_set(ms)
            LOG.info('measurement_sets=%s'%(observing_run.measurement_sets))
            for (st_file,id) in zip(st_files,st_ms_map):
                ms = observing_run.measurement_sets[id] if id >= 0 else None
                st = ScantableReaderFromMS.get_scantable(st_file, ms)
                observing_run.append(st)
        return observing_run

class MeasurementSetReader(object):
    @staticmethod
    def get_scans(ms):
        LOG.debug('Analysing scans in {0}'.format(ms.name))
        with casatools.TableReader(ms.name) as openms:
            scan_number_col = openms.getcol('SCAN_NUMBER')
            time_col = openms.getcol('TIME')
            antenna1_col = openms.getcol('ANTENNA1')
            antenna2_col = openms.getcol('ANTENNA2')
            exposure_col = openms.getcol('EXPOSURE')
            data_desc_id_col = openms.getcol('DATA_DESC_ID')
            field_id_col = openms.getcol('FIELD_ID')
            state_id_col = openms.getcol('STATE_ID')

            # get columns and tools needed to create scan times
            time_colkeywords = openms.getcolkeywords('TIME')
            time_unit = time_colkeywords['QuantumUnits'][0]
            time_ref = time_colkeywords['MEASINFO']['Ref']    
            mt = casatools.measures
            qt = casatools.quanta

            scans = []
            for scan_id in set(scan_number_col):
                scan_mask = (scan_number_col==scan_id)                

                # get the states, fields and intents used for this scan, 
                # using the intents on the state if possible, otherwise
                # falling back to the intents on the field
                state_ids = set(state_id_col[scan_mask])
                states = [ms.get_state(state_id=i) for i in state_ids]
                states = [state for state in states if state is not None]

                field_ids = set(field_id_col[scan_mask])
                fields = [ms.get_fields(field_id=i)[0] for i in field_ids]
                fields = [field for field in fields if field is not None]

                intents = set()
                if states:
                    for state in states:
                        intents.update(state.intents)
                else:
                    for field in fields:
                        intents.update(field.intents)
                        
                # get the antennas used for this scan 
                antenna_ids = set()
                scan_antenna1 = antenna1_col[scan_mask]
                scan_antenna2 = antenna2_col[scan_mask] 
                antenna_ids.update(scan_antenna1)
                antenna_ids.update(scan_antenna2)
                ant_ids = ','.join([str(i) for i in antenna_ids]) 
                antennas = ms.get_antenna(ant_ids)

                # get the data descriptions for this scan
                scan_data_desc_id = set(data_desc_id_col[scan_mask])                
                data_descriptions = [ms.get_data_description(id=i) 
                                     for i in scan_data_desc_id]

                raw_midpoints = list(set(time_col[scan_mask]))
                exposures = exposure_col[scan_mask]

                scan_times = []
                for raw_midpoint, exposure in zip(raw_midpoints, exposures):
                    # measurement set spec states that exposure is recorded in 
                    # seconds
                    exposure = qt.quantity(exposure, 's')
                    half_exposure = qt.div(exposure, 2)
                    
                    # add and subtract half the exposure to get the start and
                    # end times for the exposure
                    midpoint_epoch = qt.quantity(raw_midpoint, time_unit)
                    start_epoch = qt.sub(midpoint_epoch, half_exposure)
                    end_epoch = qt.add(midpoint_epoch, half_exposure)
                    
                    start = mt.epoch(time_ref, start_epoch)
                    end = mt.epoch(time_ref, end_epoch)
                    
                    scan_times.append((start, end, exposure))

                scan = domain.Scan(id=scan_id, states=states, fields=fields,
                    data_descriptions=data_descriptions, antennas=antennas,
                    scan_times=scan_times, intents=intents)
                scans.append(scan)

                LOG.trace('{0}'.format(scan))

            return scans

    @staticmethod
    def add_band_to_spws(ms):
        for spw in ms.spectral_windows:
            spw.band = BandDescriber.get_description(spw.ref_frequency, 
                    observatory=ms.antenna_array.name)
            if spw.num_channels == 4 and ms.antenna_array.name == 'ALMA':
                spw.band = 'WVR'
        
    @staticmethod
    def link_intents_to_spws(ms):
        container = ms.states if ms.states else ms.fields
        column = 'STATE_ID' if ms.states else 'FIELD_ID'
        
        with casatools.TableReader(ms.name) as table:
            for dd in ms.data_descriptions:
                spw = dd.spw
                for obj in container:
                    # if the spw has already been tagged with this state's
                    # intents, go on to the next state
                    if obj.intents.issubset(spw.intents):
                        continue
                        
                    subTable = table.query(
                        'DATA_DESC_ID=={dd_id} '
                        '&& {column}=={identifier}'
                        '&& NOT(FLAG_ROW) '
                        '&& NOT(ALL(FLAG))'.format(dd_id=dd.id, 
                                                   column=column,
                                                   identifier=obj.id))
                    
                    if subTable.nrows() > 0:
                        spw.intents.update(obj.intents)
		    subTable.close()
        
        for spw in ms.spectral_windows:
            LOG.trace('Intents for spw #{0}: {1}'
                      ''.format(spw.id, ','.join(spw.intents)))
    
    @staticmethod
    def link_fields_to_states(ms):
        with casatools.TableReader(ms.name) as table:
            field_ids = table.getcol('FIELD_ID')
            state_ids = table.getcol('STATE_ID')
            
            # for each field..
            for field in ms.fields:
                intents = set()
                # .. find the state IDs associated with this field, ..
                for state_id in set(state_ids[field_ids==field.id]):
                    # .. then find the State object with this ID..
                    state = ms.get_state(state_id=state_id)                    
                    if state != None:
                        # .. and add its associated intents
                        intents.update(state.intents)
                        field.states.add(state)

                # if we resolved states to intents for this field..
                if intents:
                    # .. set the field intents accordingly
                    field.intents.update(intents)

    @staticmethod
    def link_fields_to_sources(ms):
        for source in ms.sources:
            for field in ms.fields:
                if source.id == field.source_id:
                    source.fields.append(field)
                    field.source = source

    @staticmethod
    def add_valid_spws_to_fields(ms):
        # get list of field/spw combinations that actually contain data.
        with casatools.TableReader(ms.name) as table:
            data_desc_id = table.getcol('DATA_DESC_ID')
            field_id = table.getcol('FIELD_ID')
            antenna1 = table.getcol('ANTENNA1')
            antenna2 = table.getcol('ANTENNA2')

            for field in ms.fields:
                for dd in ms.data_descriptions:
                    if numpy.any((data_desc_id==dd.id) & (field_id==field.id) & 
                                 (antenna1 != antenna2)):
                        field.valid_spws.add(dd.spw)
    
    @staticmethod
    def get_measurement_set(ms_file):
        LOG.info('Analysing {0}'.format(ms_file))
        ms = domain.MeasurementSet(ms_file)
        
        # populate ms properties with results of table readers 
        ms.antenna_array = AntennaTable.get_antenna_array(ms)
        ms.frequency_groups = SpectralWindowTable.get_frequency_groups(ms)        
        ms.data_descriptions = DataDescriptionTable.get_descriptions(ms)
        for dd in ms.data_descriptions:
            with casatools.MSReader(ms.name) as openms:
                openms.selectinit(dd.id)
                ms_info = openms.getdata(['axis_info','time'])
                dd.obs_time = numpy.mean(ms_info['time'])                
                dd.chan_freq = ms_info['axis_info']['freq_axis']['chan_freq'].tolist()
                dd.corr_axis = ms_info['axis_info']['corr_axis'].tolist()
                dd.group_name = dd.spw.group.name

        ms.fields = FieldTable.get_fields(ms)
        ms.sources = SourceTable.get_sources(ms)        
        ms.states = StateTable.get_states(ms)
        MeasurementSetReader.link_fields_to_states(ms)
        MeasurementSetReader.link_fields_to_sources(ms)
        MeasurementSetReader.add_valid_spws_to_fields(ms)
        MeasurementSetReader.link_intents_to_spws(ms)
        MeasurementSetReader.add_band_to_spws(ms)
        
#        # ..and main table properties too
#        ms.all_field_ids = map(int, 
#            MeasurementSetReader._get_range(ms.name, 'field_id'))

        # get names, types of fields
        ms.scans = MeasurementSetReader.get_scans(ms)

        return ms

    @staticmethod
    def _get_range(filename, column):
        with casatools.MSReader(filename) as ms:
            data = ms.range([column])
            return data.values()[0]


class SpectralWindowTable(object):
    @staticmethod
    def get_frequency_groups(ms):
        groups = {}
        for row in SpectralWindowTable._read_table(ms):
            SpectralWindowTable._add_spectral_window(groups, *row)
        
        for group in groups.values():
            group.measurement_set = ms
        
        return groups
    
    @staticmethod
    def _add_spectral_window(groups, spw_id, group_id, group_name, bandwidth,
                             ref_freq, chan_width, chan_freqs):
        # ensure that group names are Python str strings, other string 
        # flavours such as numpy can cause trouble with pickling. Also
        # construct names if none set, otherwise all SpW get lumped into a
        # group with name ''.
        if group_name is None or string.strip(group_name) is '':
            group_name = 'Heuristics{0}'.format(group_id)
        
        # get the FrequencyGroup with the given name, creating it if it
        # doesn't exist
        group = groups.setdefault(group_name,
                                  domain.FrequencyGroup(group_id, group_name))

        # create and add a new SpectralWindow to the frequency group
        spw = domain.SpectralWindow(spw_id, bandwidth, ref_freq, chan_width,
                                    chan_freqs)
        group.add_spw(spw)
        
        return spw

    @staticmethod
    def _read_table(ms):
        """Read the SPECTRAL_WINDOW table of the given measurement set.
        """
        LOG.debug('Analysing SPECTRAL_WINDOW table')
        ms = _get_ms_name(ms)
        spectral_window_table = os.path.join(ms, 'SPECTRAL_WINDOW')        
        with casatools.TableReader(spectral_window_table) as table:
            group_ids = table.getcol('FREQ_GROUP')
            names = table.getcol('FREQ_GROUP_NAME')
            bandwidths = table.getcol('TOTAL_BANDWIDTH')
            ref_frequency = table.getcol('REF_FREQUENCY')
            vchan_widths = table.getvarcol('CHAN_WIDTH')
            vchan_freqs = table.getvarcol('CHAN_FREQ')
            spw_ids = range(len(group_ids))

            chan_widths = []
            chan_freqs = []
            for i in spw_ids:
                chan_widths.append(vchan_widths['r%s'%(i+1)])
                chan_freqs.append(vchan_freqs['r%s'%(i+1)])

            chan_widths = itertools.chain(chan_widths)
            chan_freqs = itertools.chain(chan_freqs)

            rows = zip(spw_ids, group_ids, names, bandwidths, ref_frequency,
                       chan_widths, chan_freqs)
            return rows


class ObservationTable(object):
    @staticmethod
    def get_telescope_name(ms):
        LOG.debug('Analysing OBSERVATION table')
        ms = _get_ms_name(ms)
        table_filename = os.path.join(ms, 'OBSERVATION')
        with casatools.TableReader(table_filename) as table:
            telescope_name = table.getcol('TELESCOPE_NAME')[0]
            return telescope_name


class AntennaTable(object):
    @staticmethod
    def get_antenna_array(ms):
        telescope_name = ObservationTable.get_telescope_name(ms)
        # create a new antenna array..
        array = domain.AntennaArray(telescope_name)
        # .. and add a new Antenna for each row in the ANTENNA table
        for antenna in AntennaTable.get_antennas(ms):
            array.add_antenna(antenna)
        return array
    
    @staticmethod
    def get_antennas(ms):
        return [AntennaTable._create_antenna(*row) 
                for row in AntennaTable._read_table(ms)]
    
    @staticmethod
    def _create_antenna(antenna_id, name, positions, keywords, flag, diameter,
                        station):
        # omit this antenna if it has been flagged
        if flag == True:
            return

        # get the x, y, z values from the positions tuple
        x = positions[0]
        y = positions[1]
        z = positions[2]

        # find out what units these values are in
        ref_keyword = keywords['MEASINFO']['Ref']
        x_units = keywords['QuantumUnits'][0]
        y_units = keywords['QuantumUnits'][1]
        z_units = keywords['QuantumUnits'][2]

        # save these as we'll need them to work round a CASA bug which converts
        # all positions to radians
        qt = casatools.quanta
        v0=qt.quantity(x, x_units)
        v1=qt.quantity(y, y_units)
        v2=qt.quantity(z, z_units)

        # so we can create a CASA position..
        mt = casatools.measures   
        rad_position = mt.position(rf=ref_keyword, v0=v0, v1=v1, v2=v2)

        # and now for our workaround..
        m_position = mt.position(rf=ref_keyword, v0=v0, v1=v1, v2=v2)
        m_position['m0'] = v0
        m_position['m1'] = v1
        m_position['m2'] = v2
    
        # .. with which we can create an Antenna
        antenna = domain.Antenna(antenna_id, name, m_position, rad_position,
                                 diameter, station)
        return antenna

    @staticmethod
    def _read_table(ms):
        """Read the ANTENNA table of the given measurement set, returning a
        list of tuples; each item in the list corresponds to a row in the 
        table. 
        
        """
        LOG.debug('Analysing ANTENNA table')
        ms = _get_ms_name(ms)

        table_filename = os.path.join(ms, 'ANTENNA')
        with casatools.TableReader(table_filename) as table:
            names = table.getcol('NAME')
            positions = table.getcol('POSITION')
            position_keywords = table.getcolkeywords('POSITION')
            flags = table.getcol('FLAG_ROW')
            diameter = table.getcol('DISH_DIAMETER')
            stations = table.getcol('STATION')
            
            # transpose list to get n tuples of x,y,z for each antenna rather
            # than 3 lists of all x,y,z values
            positions = zip(*positions)

            # construct a list of keywords of the same length as the other
            # lists so we can zip it together names, positions and flags
            keywords = [position_keywords] * len(names)

            ids = range(len(names))

            rows = zip(ids, names, positions, keywords, flags, diameter, 
                       stations)
            return rows


class DataDescriptionTable(object):
    @staticmethod
    def get_descriptions(ms):
        spws = ms.spectral_windows
        # read the data descriptions table and create the objects
        descriptions = [DataDescriptionTable._create_data_description(spws, *row) 
                        for row in DataDescriptionTable._read_table(ms)]
            
        return descriptions            
        
    @staticmethod
    def _create_data_description(spws, dd_id, spw_id, pol_id):
        # find the SpectralWindow matching the given spectral window ID
        matching_spws = [spw for spw in spws if spw.id == spw_id]
        spw = matching_spws[0]
        
        return domain.DataDescription(dd_id, spw, pol_id)
    
    @staticmethod
    def _read_table(ms):
        """Read the DATA_DESCRIPTION table of the given measurement set.
        """
        LOG.debug('Analysing DATA_DESCRIPTION table')
        ms = _get_ms_name(ms)
        data_description_table = os.path.join(ms, 'DATA_DESCRIPTION')        
        with casatools.TableReader(data_description_table) as table:
            spw_ids = table.getcol('SPECTRAL_WINDOW_ID')
            pol_ids = table.getcol('POLARIZATION_ID')
            dd_ids = range(len(spw_ids))

            rows = zip(dd_ids, spw_ids, pol_ids)
            return rows


class SourceTable(object):
    @staticmethod
    def get_sources(ms):
        sources = [SourceTable._create_source(*row) 
                   for row in SourceTable._read_table(ms)]

        # duplicate source entries may be present due to duplicate entries
        # differing by non-essential columns, such as spw
        key_fn = lambda source: source.id
        data = sorted(sources, key=key_fn)
        grouped_by_source_id = []
        for _, g in itertools.groupby(data, key_fn):
            grouped_by_source_id.append(list(g))
        return [s[0] for s in grouped_by_source_id]             
        
    @staticmethod
    def _create_source(source_id, name, direction, direction_kw, motion, 
                       motion_kw):
        # get the x, y, values from the direction tuple
        x = direction[0]
        y = direction[1]

        # find out what units these values are in
        ref_keyword = direction_kw['MEASINFO']['Ref']
        x_units = direction_kw['QuantumUnits'][0]
        y_units = direction_kw['QuantumUnits'][1]

        # so we can create a CASA position..
        mt = casatools.measures   
        qt = casatools.quanta
        direction = mt.direction(rf=ref_keyword,
                                 v0=qt.quantity(x, x_units),
                                 v1=qt.quantity(y, y_units))
        
        motion_units = motion_kw['QuantumUnits'][0]
        motion_x = qt.quantity(motion[0], motion_units)
        motion_y = qt.quantity(motion[1], motion_units)
        
        return domain.Source(source_id, name, direction, motion_x, motion_y)
        
    @staticmethod
    def _read_table(ms):
        """Read the SOURCE table of the given measurement set.
        """
        LOG.debug('Analysing SOURCE table')
        ms = _get_ms_name(ms)
        source_table = os.path.join(ms, 'SOURCE')   
        with casatools.TableReader(source_table) as table:
            source_ids = table.getcol('SOURCE_ID')
            names = table.getcol('NAME')
            directions = table.getcol('DIRECTION')
            direction_keywords = table.getcolkeywords('DIRECTION')
            motions = table.getcol('PROPER_MOTION')
            motions_keywords = table.getcolkeywords('PROPER_MOTION')

            # transpose lists to get n tuples of x,y for each source rather
            # than 2 lists of all x,y values
            directions = zip(*directions)
            motions = zip(*motions)

            direction_keywords = [direction_keywords] * len(source_ids)
            motions_keywords = [motions_keywords] * len(source_ids)

            return zip(source_ids, names, directions, direction_keywords,
                       motions, motions_keywords)


class StateTable(object):
    @staticmethod
    def get_states(ms):
        states = [StateTable._create_state(*row) 
                  for row in StateTable._read_table(ms)]        
        return states            
        
    @staticmethod
    def _create_state(state_id, obs_mode):
        # find the SpectralWindow matching the given spectral window ID
        return domain.State(state_id, obs_mode)
    
    @staticmethod
    def _read_table(ms):
        """Read the STATE table of the given measurement set.
        """
        LOG.debug('Analysing STATE table')
        ms = _get_ms_name(ms)
        state_table = os.path.join(ms, 'STATE')      
        with casatools.TableReader(state_table) as table:
            obs_modes = table.getcol('OBS_MODE')
            state_ids = range(len(obs_modes))
            return zip(state_ids, obs_modes)


class FieldTable(object):
    @staticmethod
    def get_fields(ms):
        return [FieldTable._create_field(*row) 
                for row in FieldTable._read_table(ms)]
    
    @staticmethod
    def _create_field(field_id, name, source_id, time, source_type,
     phase_dir_ref_type, phase_dir_quanta):
        # .. with which we can create an Antenna
        field = domain.Field(field_id, name, source_id, time,
         phase_dir_ref_type, phase_dir_quanta)
        if source_type:
            field.set_source_type(source_type)
        return field
    
    @staticmethod
    def _read_table(ms):
        """Read the FIELD table of the given measurement set.
        """
        LOG.debug('Analysing FIELD table')
        ms = _get_ms_name(ms)
        field_table = os.path.join(ms, 'FIELD')      
        with casatools.TableReader(field_table) as table:
            names = table.getcol('NAME')
            source_ids = table.getcol('SOURCE_ID')
            times = table.getcol('TIME')

            # get array of phase dir quantities
            field_ids = range(len(names))
            phase_dir = table.getcol('PHASE_DIR')
            phase_dir_keywords = table.getcolkeywords('PHASE_DIR')
            phase_dir_quanta = []
            for field_id in field_ids:
                field_dir_quanta = ['%s%s' % (phase_dir[0,0,field_id],
                 phase_dir_keywords['QuantumUnits'][0]),
                 '%s%s' % (phase_dir[1,0,field_id],
                 phase_dir_keywords['QuantumUnits'][1])]
                phase_dir_quanta.append(field_dir_quanta)

            # get array of phase dir ref types
            if 'VarRefCol' in phase_dir_keywords['MEASINFO']:
                # ref varies with row
                phase_dir_ref = table.getcol(
                 phase_dir_keywords['MEASINFO']['VarRefCol'])
                phase_dir_ref_types = phase_dir_keywords['MEASINFO']\
                 ['TabRefTypes']
                phase_dir_ref_codes = phase_dir_keywords['MEASINFO']\
                 ['TabRefCodes']
            else:
                # ref fixed for all rows,
                # construct as though variable reference with all refs
                # the same
                phase_dir_ref = numpy.zeros([numpy.shape(phase_dir)[2]], 
                                            numpy.int)
                phase_dir_ref_types = numpy.array(
                        [phase_dir_keywords['MEASINFO']['Ref']])
                phase_dir_ref_codes = numpy.array([0])
            phase_dir_ref_type = []
            for field_id in field_ids:
                field_dir_ref_type = phase_dir_ref_types[
                        phase_dir_ref[field_id]==phase_dir_ref_codes][0]
                phase_dir_ref_type.append(field_dir_ref_type)

            # FIELD.SOURCE_TYPE contains the intents in non-APDM MS
            if 'SOURCE_TYPE' in table.colnames():
                source_types = table.getcol('SOURCE_TYPE')
            else:
                source_types = [None] * len(names)

            field_ids = range(len(names))

            return zip(field_ids, names, source_ids, times, source_types,
             phase_dir_ref_type, phase_dir_quanta)


def _make_range(f_min, f_max):
    return measures.FrequencyRange(measures.Frequency(f_min),
                                   measures.Frequency(f_max))
    
class BandDescriber(object):    
    alma_bands = {_make_range(31.3, 45) : 'ALMA Band 1',
                  _make_range(67, 90) : 'ALMA Band 2',
                  _make_range(84, 116) : 'ALMA Band 3',
                  _make_range(125, 163) : 'ALMA Band 4',
                  _make_range(163, 211) : 'ALMA Band 5',
                  _make_range(211, 275) : 'ALMA Band 6',
                  _make_range(275, 373) : 'ALMA Band 7',
                  _make_range(385, 500) : 'ALMA Band 8',
                  _make_range(602, 720) : 'ALMA Band 9',
                  _make_range(787, 950) : 'ALMA Band 10'}

    evla_bands = {_make_range(1, 2) : '20cm (L)',
                  _make_range(2, 4) : '13cm (S)',
                  _make_range(4, 8) : '6cm (C)',
                  _make_range(8, 12) : '3cm (X)',
                  _make_range(12, 18) : '2cm (Ku)',
                  _make_range(18, 26.5) : '1.3cm (K)',
                  _make_range(26.5, 40) : '1cm (Ka)',
                  _make_range(40, 50) : '0.7cm (Q)'}
    
    unknown = { measures.FrequencyRange() : 'Unknown'}

    @staticmethod
    def get_description(f, observatory='ALMA'):
        observatory = string.upper(observatory)
        if observatory in ('ALMA',):
            bands = BandDescriber.alma_bands
        elif observatory in ('VLA', 'EVLA'):
            bands = BandDescriber.evla_bands
        else:
            bands = BandDescriber.unknown
    
        for rng, description in bands.items():
            if rng.contains(f):
                return description
        
        return 'Unknown'


### single dish specific
class ScantableReader(object):
    @staticmethod
    def get_scantable(name):
        st = domain.singledish.ScantableRep(name)
        st.antenna = ScantableReader.get_antenna(st)
        st.source = ScantableReader.get_source(st)
        st.spectral_window = ScantableReader.get_spectral_window(st)
        st.polarization = ScantableReader.get_polarization(st)
        ScantableReader.configure_observation(st)
        ScantableReader.configure_calibration(st)
        return st

    @staticmethod
    def configure_observation(st):
        me = casatools.measures
        qa = casatools.quanta
        with casatools.TableReader(st.name) as tb:
            st.observer = tb.getkeyword('Observer')
            # start/end time in MJD [day]
            timecol = tb.getcol('TIME')
            st.start_time = me.epoch(rf='UTC',
                                     v0=qa.quantity(timecol.min(),'d'))
            st.end_time = me.epoch(rf='UTC',
                                   v0=qa.quantity(timecol.max(),'d'))

    @staticmethod
    def get_antenna(st):
        with casatools.TableReader(st.name) as tb:
            name = tb.getkeyword('AntennaName')
            position = tb.getkeyword('AntennaPosition')
            
        site_split = name.split('//')
        if len(site_split) > 1:
            sitename = site_split[0]
            name = site_split[1]
        else:
            sitename = 'UNKNOWN'
            name = site_split[0]
        station_split = name.split('@')
        name = station_split[0]
        if len(station_split) > 1:
            station = station_split[1]
        else:
            station = 'UNKNOWN'
        LOG.debug('site=%s, station=%s, antenna=%s'%(sitename,station,name))
        me = casatools.measures
        qa = casatools.quanta
        m_position = me.position(rf='ITRF',
                                 v0=qa.quantity(position[0],'m'),
                                 v1=qa.quantity(position[1],'m'),
                                 v2=qa.quantity(position[2],'m'))
        rad_position = me.measure(m_position,
                                  rf='WGS84')
        h = sdheuristics.AntennaDiameter()
        diameter = h.calculate(name=name)
        antenna = domain.Antenna(0, name, m_position, rad_position, diameter, station)

        LOG.trace('created Antenna object\n%s'%(antenna))

        return antenna

    @staticmethod
    def get_source(st):
        me = casatools.measures
        qa = casatools.quanta
        source = {}
        with casatools.TableReader(st.name) as tb:
            source_names = numpy.unique(tb.getcol('SRCNAME'))
            for idx in xrange(len(source_names)):
                name = source_names[idx]
                ts = tb.query('SRCNAME == \"%s\"'%(name))
                src_dir = ts.getcell('DIRECTION',0)
                msrc_dir = me.direction(rf='J2000',
                                        v0=qa.quantity(src_dir[0],'rad'),
                                        v1=qa.quantity(src_dir[1],'rad'))
                src_pm = tb.getcell('SRCPROPERMOTION',0)
                pm_x = qa.quantity(src_pm[0],'rad/s')
                pm_y = qa.quantity(src_pm[1],'rad/s')
                entry = domain.Source(idx, name, msrc_dir, pm_x, pm_y)
                source[idx] = entry
                ts.close()

        return source
        
    @staticmethod
    def get_spectral_window(st):
        spectral_window = {}
        frequencies_dict = {}
                    
        with casatools.TableReader(st.name) as tb:
            frequencies = tb.getkeyword('FREQUENCIES').lstrip('Table: ')

        with casatools.TableReader(frequencies) as tb:
            id = tb.getcol('ID')
            refpix = tb.getcol('REFPIX')
            refval = tb.getcol('REFVAL')
            increment = tb.getcol('INCREMENT')
            frame = tb.getkeyword('BASEFRAME')
            for i in xrange(tb.nrows()):
                d = {}
                d['reference_pixel'] = refpix[i]
                d['reference_value'] = refval[i]
                d['increment'] = increment[i]
                d['frame'] = frame
                frequencies_dict[id[i]] = d

        with casatools.TableReader(st.name) as tb:
            spw_ids = numpy.unique(tb.getcol('IFNO'))
            for spw in spw_ids:
                ts = tb.query('IFNO==%s'%(spw))

                # spectral setup
                freq_id = ts.getcell('FREQ_ID',0)
                frequencies = frequencies_dict[freq_id]
                nchan = len(ts.getcell('FLAGTRA',0))
                tsyschan = len(ts.getcell('TSYS',0))
                refpix = frequencies['reference_pixel']
                refval = frequencies['reference_value']
                incr = frequencies['increment']
                frequency_range = [refval - (refpix+0.5) * incr,
                                   refval + (nchan-refpix-0.5) *incr]
                frequency_range.sort()
                spw_type = ('TP' if nchan == 1 else \
                            ('WVR' if nchan == 4 else 'SP'))

                # intents
                from asap import srctype as st
                srctype = numpy.unique(ts.getcol('SRCTYPE'))
                intents = set()
                for stype in srctype:
                    if stype in (st.pson, st.fson, st.fslo, st.nod):
                        intents.add('TARGET')
                    elif stype in (st.psoff, st.fsoff, st.fshi):
                        intents.add('REFERENCE')
                    else:
                        intents.add('CALIB')
                        if spw_type == 'SP' and nchan == tsyschan:
                            intents.add('ATMOSPHERE')
                        
                entry = domain.singledish.Frequencies(id=spw,
                                                      type=spw_type,
                                                      frame=frequencies['frame'],
                                                      nchan=nchan,
                                                      bandwidth=nchan*incr,
                                                      freq_min=frequency_range[0],
                                                      freq_max=frequency_range[1],
                                                      refpix=refpix,
                                                      refval=refval,
                                                      increment=incr,
                                                      intent=string.join(intents,':'))
                spectral_window[spw] = entry
                                    
                ts.close()

        return spectral_window

    @staticmethod
    def get_polarization(st):
        polarization = {}
        polmap = domain.singledish.Polarization.polarization_map
        with casatools.TableReader(st.name) as tb:
            poltype = tb.getkeyword('POLTYPE')
            for spw in st.spectral_window.keys():
                ts = tb.query('IFNO==%s'%(spw))
                polno = numpy.unique(ts.getcol('POLNO'))
                corr = [polmap[poltype][pol] for pol in polno]
                key = None
                for (k,v) in polarization.items():
                    if len(v.polno) == len(polno) \
                       and all(v.polno == polno):
                        key = k
                if key is None:
                    key = len(polarization)
                    entry = domain.singledish.Polarization(type=poltype,
                                                           polno=polno,
                                                           spw_association=[spw],
                                                           corr_string=[c[0] for c in corr],
                                                           corr_index=[c[1] for c in corr])
                    polarization[key] = entry
                else:
                    polarization[key].spw_association.append(spw)

                # register polarization to st.spectral_window
                st.spectral_window[spw].pol_association.append(key)
                ts.close()

        return polarization


    @staticmethod
    def configure_calibration(st):
        tolerance = domain.singledish.ScantableRep.tolerance
        spw_for_atmcal = []
        spw_for_target = []
        for spw in st.spectral_window.values():
            if spw.is_target:
                spw_for_target.append(spw.id)
            if spw.is_atmcal:
                spw_for_atmcal.append(spw.id)

        LOG.info('spw_for_atmcal=%s'%(spw_for_atmcal))
        LOG.info('spw_for_target=%s'%(spw_for_target))

        # inspect frequency coverage
        # tsys_transfer_list = [[from,to],[from,to],...]
        for a in spw_for_atmcal:
            afreq = st.spectral_window[a].frequency_range
            for t in spw_for_target:
                tfreq = st.spectral_window[t].frequency_range
                dfl = (tfreq[0]-afreq[0])/afreq[0]
                dfh = (tfreq[1]-afreq[1])/afreq[1]
                LOG.debug('tfreq=%s,afreq=%s'%(tfreq,afreq))
                LOG.debug('dfl=%s,dfh=%s'%(dfl,dfh))
                if dfl >= -tolerance and dfh <= tolerance:
                    st.tsys_transfer_list.append([a,t])
        st.tsys_transfer = len(st.tsys_transfer_list) > 0


class ScantableReaderFromMS(ScantableReader):
    @staticmethod
    def get_scantable(name, ms):
        st = domain.singledish.ScantableRep(name, ms=ms)
        st.antenna = ScantableReaderFromMS.get_antenna(st, ms)
        st.source = ScantableReaderFromMS.get_source(st, ms)
        st.spectral_window = ScantableReaderFromMS.get_spectral_window(st, ms)
        st.polarization = ScantableReaderFromMS.get_polarization(st, ms)
        ScantableReaderFromMS.configure_observation(st)
        ScantableReaderFromMS.configure_calibration(st, ms)
        return st

    @staticmethod
    def get_antenna(st, ms):
        # get antenna name
        sd = casatools.sd
        s = sd.scantable(st.name,False)
        antenna_name = s.get_antennaname()
        del s

        # get antenna from MS based on its name
        antenna = ms.antenna_array.get_antenna(name=antenna_name)

        return antenna

    @staticmethod
    def get_source(st, ms):
        with casatools.TableReader(st.name) as tb:
##             source_names = map((lambda s: s.replace(' ','_')),
##                                numpy.unique(tb.getcol('SRCNAME')))
            source_names = numpy.unique(tb.getcol('SRCNAME'))
        sources = [s for s in ms.sources if s.name in source_names]
        source = {}
        for idx in xrange(len(sources)):
            source[idx] = sources[idx]
        return source
        

    @staticmethod
    def get_spectral_window(st, ms):
        spectral_window = {}
        spw_list = ms.get_all_spectral_windows()
        for spw in spw_list:
            entry = domain.singledish.Frequencies.from_spectral_window(spw)
            spectral_window[spw.id] = entry
                                

        name = ms.name
        with casatools.TableReader(name) as tb:
            for spw in spectral_window.keys():
                datadesc = ms.get_data_description(spw)
                if datadesc is None:
                    continue
                ts = tb.query('DATA_DESC_ID==%s'%(datadesc.id))
                state_ids = set(ts.getcol('STATE_ID'))
                intents = set()
                for id in state_ids:
                    intent = ms.states[id].intents
                    if intent.issuperset(['ATMOSPHERE']):
                        intents.add('CALIB')
                        intents.add('ATMOSPHERE')
                    if intent.issuperset(['TARGET']):
                        intents.add('TARGET')
                    if intent.issuperset(['REFERENCE']):
                        intents.add('REFERENCE')
                    if len(intent.intersection(['BANDPASS','AMPLITUDE','PHASE','POINTING','SIDEBAND'])) > 0:
                        intents.add('CALIB')
                spectral_window[spw].intent = string.join(intents,':')
                ts.close()

            subtable_name = tb.getkeyword('SPECTRAL_WINDOW').lstrip('Table: ')

        with casatools.TableReader(subtable_name) as tb:
            for spw in spectral_window.keys():
                frame_id = tb.getcell('MEAS_FREQ_REF')
                spectral_window[spw].frame = domain.singledish.Frequencies.frame_map[frame_id]
 

        keys = spectral_window.keys()
        for spw in keys:
            if len(spectral_window[spw].intent) == '':
                spectral_window.pop(spw)

        return spectral_window

    @staticmethod
    def get_polarization(st, ms):
        polarization = {}
        for spw in st.spectral_window.keys():
            datadesc = ms.get_data_description(spw)
            if datadesc is None:
                print 'datadesc for spw %s is None'%(spw)
                continue
            npol = datadesc.num_polarizations
            polno = numpy.arange(npol, dtype=int)
            key = None
            for (k,v) in polarization.items():
                if len(v.polno) == npol \
                   and all(v.polno == polno):
                    key = k
            if key is None:
                key = len(polarization)
                entry = domain.singledish.Polarization.from_data_desc(datadesc)
                polarization[key] = entry
            else:
                polarization[key].spw_association.append(spw)
                
            # register polarization to st.spectral_window
            st.spectral_window[spw].pol_association.append(key)

        return polarization
        
    @staticmethod
    def configure_calibration(st, ms):
        with casatools.TableReader(ms.name) as tb:
            spw_table = tb.getkeyword('SPECTRAL_WINDOW').lstrip('Table: ')

        with casatools.TableReader(spw_table) as tb:
            if 'BBC_NO' in tb.colnames():
                bbc_no = tb.getcol('BBC_NO')
            else:
                bbc_no = [-1] * tb.nrows()
        
        tolerance = domain.singledish.ScantableRep.tolerance
        spw_for_atmcal = []
        spw_for_target = []
        for spw in st.spectral_window.values():
            if spw.is_target:
                spw_for_target.append(spw.id)
            if spw.is_atmcal:
                spw_for_atmcal.append(spw.id)

        LOG.info('spw_for_atmcal=%s'%(spw_for_atmcal))
        LOG.info('spw_for_target=%s'%(spw_for_target))

        # inspect frequency coverage
        # self.tsys_transfer_list = [[from,to],[from,to],...]
        for a in spw_for_atmcal:
            afreq = st.spectral_window[a].frequency_range
            for t in spw_for_target:
                if bbc_no[a] != bbc_no[t]:
                    continue
                
                tfreq = st.spectral_window[t].frequency_range
                dfl = (tfreq[0]-afreq[0])/afreq[0]
                dfh = (tfreq[1]-afreq[1])/afreq[1]
                LOG.debug('tfreq=%s,afreq=%s'%(tfreq,afreq))
                LOG.debug('dfl=%s,dfh=%s'%(dfl,dfh))
                if dfl >= -tolerance and dfh <= tolerance:
                    st.tsys_transfer_list.append([a,t])
        st.tsys_transfer = len(st.tsys_transfer_list) > 0


class VirtualMeasurementSetFiller(object):

    @staticmethod
    def get_measurement_set(scantables):
        if isinstance(scantables, domain.singledish.ScantableRep):
            st = [scantables]
        else:
            st = scantables
        ms = virtualms.VirtualMeasurementSet(st)
        ms.antenna_array = VirtualMeasurementSetFiller.get_antenna_array(st)
        ms.sources = VirtualMeasurementSetFiller.get_source(st)
        ms.frequency_groups = VirtualMeasurementSetFiller.get_frequency_group(st)
        ms.fields = VirtualMeasurementSetFiller.get_field(st)
        ms.scans = VirtualMeasurementSetFiller.get_scan(st, ms)
        ms.data_descriptions = VirtualMeasurementSetFiller.get_data_description(st)
        return ms

    @staticmethod
    def get_antenna_array(st):
        with casatools.TableReader(st[0].name) as tb:
            antenna_name = tb.getkeyword('AntennaName')
            site_name = antenna_name.split('//')[0]
            
        antenna_array = domain.AntennaArray(name=site_name)
        for s in st:
            antenna_array.add_antenna(s.antenna)

        return antenna_array

    @staticmethod
    def get_source(st):
        sources = []
        source_names = []
        for s in st:
            src_list = s.source
            LOG.info('src_list=%s'%(src_list))
            for (k,v) in src_list.items():
                if v.name not in source_names:
                    sources.append(v)
                    source_names.append(v.name)
        return sources

    @staticmethod
    def get_field(st):
        # assume all scantables observe same src/field
        me = casatools.measures
        qa = casatools.quanta
        fields = []
        with casatools.TableReader(st[0].name) as tb:
            # start/end time in MJD [day]
            source_names = numpy.unique(tb.getcol('SRCNAME'))
            field_names = numpy.unique(tb.getcol('FIELDNAME'))
            for idx in xrange(len(field_names)):
                s = field_names[idx].split('__')
                name = s[0]
                if len(s) > 1:
                    id = int(s[1])
                else:
                    id = 0
                ts = tb.query('FIELDNAME == \"%s\"'%(field_names[idx]))
                center_time = ts.getcol('TIME').mean()
                try:
                    source_id = source_names.index(name)
                    source_obj = st[0].sources[idx]
                    msrc_dir = source_obj._direction
                    
                except:
                    source_id = 0
                    src_dir = ts.getcell('DIRECTION',0)
                    msrc_dir = me.direction(rf='J2000',
                                            v0=qa.quantity(src_dir[0],'rad'),
                                            v1=qa.quantity(src_dir[1],'rad'))
                ts.close()
                ref = me.getref(msrc_dir)
                val = me.getvalue(msrc_dir)
                val = [val['m0'], val['m1']]
                fields.append(domain.Field(idx, name, source_id,
                                           center_time, ref, val))
        return fields
    

    @staticmethod
    def get_frequency_group(st):
        # assume all scantables have same frequency setting
        spectral_windows = [None] * len(st[0].spectral_window)
        for (k,v) in st[0].spectral_window.items():
            spectral_windows[k] = v

        group_id = 0
        group_name = 'DUMMY%s'%(group_id)
        frequency_group = domain.FrequencyGroup(group_id, group_name)
        for spw in spectral_windows:
            frequency_group.add_spw(spw)

        return {group_name: frequency_group}
            
    @staticmethod
    def get_scan(st, ms):
        scans = []
                
        # assume all scantables have same scan sequence
        me = casatools.measures
        qa = casatools.quanta
        with casatools.TableReader(st[0].name) as tb:
            scannos = numpy.unique(tb.getcol('SCANNO'))
            for s in scannos:
                tsel = tb.query('SCANNO==%s'%(s))
                ifnos = numpy.unique(tsel.getcol('IFNO'))
                times = tsel.getcol('TIME')
                intervals = tsel.getcol('INTERVAL')
                scan_times = []
                field_names = numpy.unique(tsel.getcol('FIELDNAME'))
                field_names = [v.split('__')[0].replace(' ','_') for v in field_names]
                for (t,i) in zip(times,intervals):
                    interval = qa.quantity(i,'s')
                    half_interval = qa.div(interval, 2)
                    center_time = qa.quantity(t,'d')
                    start_time = qa.sub(center_time, half_interval)
                    end_time = qa.add(center_time, half_interval)
                    start = me.epoch('UTC', start_time)
                    end = me.epoch('UTC', end_time)
                    scan_times.append((start,end,interval))
                    
                tsel.close()
                intents = set()
                fields = []
                for f in ms.fields:
                    name = f.clean_name
                    if name in field_names:
                        fields.append(f)
                for spw in ifnos:
                    spwin = st[0].spectral_window[spw]
                    intents = intents.union(spwin.intents)
                scans.append(domain.Scan(id=s,
                                         antennas=ms.antennas,
                                         fields=fields,
                                         intents=intents,
                                         scan_times=scan_times))

        return scans


    @staticmethod
    def get_data_description(st):
        data_descriptions = []
        
        # assume all scantables have same spectral setup
        for (id,spw) in st[0].spectral_window.items():
            pol_id = spw.pol_association[0]
            corr_axis = [v[0] for v in st[0].polarization[pol_id].corr_string]
            dd = domain.DataDescription(spw.id,spw,pol_id)
            dd.corr_axis = corr_axis
            data_descriptions.append(dd)

        return data_descriptions
