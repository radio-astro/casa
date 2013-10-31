from __future__ import absolute_import
import datetime
import itertools
import os
import re
import string

import numpy

from . import casatools
from . import logging
from . import utils
import pipeline.domain as domain
import pipeline.domain.measures as measures

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


class MeasurementSetReader(object):
    @staticmethod
    def get_scans(ms):
        LOG.debug('Analysing scans in {0}'.format(ms.name))
        with utils.open_table(ms.name) as openms:
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

                # times are specified per data description, so we must
                # re-mask and calculate times per dd
                scan_times = {}  
                for dd in data_descriptions:
                    dd_mask = (scan_number_col==scan_id) & (data_desc_id_col==dd.id)

                    raw_midpoints = list(time_col[dd_mask])
                    unique_midpoints = set(raw_midpoints)
                    exposures = list(exposure_col[dd_mask])
    
                    # get the exposure times that correspond to the unique midpoint
                    # times that we just found 
                    exposure_for_midpoints = []
                    for midpoint in unique_midpoints:
                        idx = raw_midpoints.index(midpoint)
                        exposure_for_midpoints.append(exposures[idx])
                        
                    dd_times = []
                    for raw_midpoint, exposure in zip(unique_midpoints, 
                                                      exposure_for_midpoints):
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
                        
                        dd_times.append((start, end, exposure))

                    scan_times[dd] = dd_times

                scan = domain.Scan(id=scan_id, states=states, fields=fields,
                    data_descriptions=data_descriptions, antennas=antennas,
                    scan_times=scan_times, intents=intents)
                scans.append(scan)

                LOG.trace('{0}'.format(scan))

            return scans

    @staticmethod
    def add_band_to_spws(ms):
        for spw in ms.spectral_windows:
            if 'WVR' in spw.name:
                spw.band = 'WVR'
                continue
        
            # Expected format is something like ALMA_RB_03#BB_1#SW-01#FULL_RES
            m = re.match(r'ALMA_RB_(?P<band>\d+)', spw.name)
            if m:
                band_str = m.groupdict()['band']
                band_num = int(band_str)
                spw.band = 'ALMA Band %s' % band_num
                continue
            
            spw.band = BandDescriber.get_description(spw.ref_frequency, 
                    observatory=ms.antenna_array.name)
            if spw.num_channels == 4 and ms.antenna_array.name == 'ALMA':
                spw.band = 'WVR'
        
    @staticmethod
    def link_intents_to_spws(ms):
        container = ms.states if ms.states else ms.fields
        column = 'STATE_ID' if ms.states else 'FIELD_ID'
        
        with utils.open_table(ms.name) as table:
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
        with utils.open_table(ms.name) as table:
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
        with utils.open_table(ms.name) as table:
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
        ms.polarizations = PolarizationTable.get_polarizations(ms)
        for dd in ms.data_descriptions:
            with utils.open_ms(ms.name) as openms:
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

        (observer, project_id, schedblock_id, execblock_id) = ObservationTable.get_project_info(ms)

        # work around NumPy bug with empty strings
        # http://projects.scipy.org/numpy/ticket/1239
        ms.observer = str(observer)
        ms.project_id = str(project_id)

        ms.schedblock_id = schedblock_id
        ms.execblock_id = execblock_id
        
#        # ..and main table properties too
#        ms.all_field_ids = map(int, 
#            MeasurementSetReader._get_range(ms.name, 'field_id'))

        # get names, types of fields
        ms.scans = MeasurementSetReader.get_scans(ms)

        return ms

    @staticmethod
    def _get_range(filename, column):
        with utils.open_ms(filename) as ms:
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
                             ref_freq, chan_width, chan_freqs, name, sideband,
                             baseband):
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
                                    chan_freqs, name, sideband, baseband)
        group.add_spw(spw)
        
        return spw

    @staticmethod
    def _read_table(ms):
        """Read the SPECTRAL_WINDOW table of the given measurement set.
        """
        LOG.debug('Analysing SPECTRAL_WINDOW table')
        ms = _get_ms_name(ms)
        spectral_window_table = os.path.join(ms, 'SPECTRAL_WINDOW')        
        with utils.open_table(spectral_window_table) as table:
            group_ids = table.getcol('FREQ_GROUP')
            group_names = table.getcol('FREQ_GROUP_NAME')
            bandwidths = table.getcol('TOTAL_BANDWIDTH')
            ref_frequency = table.getcol('REF_FREQUENCY')
            vchan_widths = table.getvarcol('CHAN_WIDTH')
            vchan_freqs = table.getvarcol('CHAN_FREQ')
            spw_ids = range(len(group_ids))
            names = table.getcol('NAME')
            sidebands = table.getcol('NET_SIDEBAND')

            if 'BBC_NO' in table.colnames():
                basebands = table.getcol('BBC_NO')
            else:
                basebands = [None] * table.ncols()
             
            chan_widths = []
            chan_freqs = []
            for i in spw_ids:
                chan_widths.append(vchan_widths['r%s'%(i+1)])
                chan_freqs.append(vchan_freqs['r%s'%(i+1)])

            chan_widths = itertools.chain(chan_widths)
            chan_freqs = itertools.chain(chan_freqs)

            rows = zip(spw_ids, group_ids, group_names, bandwidths, ref_frequency,
                       chan_widths, chan_freqs, names, sidebands, basebands)
            return rows


class ObservationTable(object):
    @staticmethod
    def get_telescope_name(ms):
        LOG.debug('Analysing OBSERVATION table')
        ms = _get_ms_name(ms)
        table_filename = os.path.join(ms, 'OBSERVATION')
        with utils.open_table(table_filename) as table:
            telescope_name = table.getcol('TELESCOPE_NAME')[0]
            return telescope_name

    @staticmethod
    def get_project_info(ms):
        ms = _get_ms_name(ms)
        table_filename = os.path.join(ms, 'OBSERVATION')
        with utils.open_table(table_filename) as table:
            telescope_name = table.getcol('TELESCOPE_NAME')[0]
            project_id = table.getcol('PROJECT')[0]
            observer = table.getcol('OBSERVER')[0]

            schedblock_id = 'N/A'
            execblock_id = 'N/A'

            if telescope_name == 'ALMA':
                schedule = table.getcol('SCHEDULE')[0]
            
                d = {}
                for cell in schedule:
                    key, val = string.split(cell)
                    d[key] = val
                                    
                schedblock_id = d.get('SchedulingBlock', 'N/A')
                execblock_id = d.get('ExecBlock', 'N/A')

            return observer, project_id, schedblock_id, execblock_id

    @staticmethod
    def get_time_range(ms):
        ms = _get_ms_name(ms)

        with utils.open_table(ms) as openms:
            # get columns and tools needed to create scan times
            time_colkeywords = openms.getcolkeywords('TIME')
            time_unit = time_colkeywords['QuantumUnits'][0]
            time_ref = time_colkeywords['MEASINFO']['Ref']    
            me = casatools.measures
            qa = casatools.quanta
                
        table_filename = os.path.join(ms, 'OBSERVATION')
        with utils.open_table(table_filename) as table:
            start_s, end_s = table.getcol('TIME_RANGE')

            # start_s, end_s are arrays with a single entry for
            # simple datasets. Measurement sets that are
            # concats of simple datasets have more than 1 entry,
            # so use the 'min' and 'max' to handle this possibiity.
            start_s = numpy.min(start_s)
            end_s = numpy.max(end_s)

            epoch_start = me.epoch(time_ref, qa.quantity(start_s, time_unit))
            epoch_end = me.epoch(time_ref, qa.quantity(end_s, time_unit))

            str_start = qa.time(epoch_start['m0'], form=['fits'])[0]
            str_end = qa.time(epoch_end['m0'], form=['fits'])[0]

            dt_start = datetime.datetime.strptime(str_start, 
                                                  '%Y-%m-%dT%H:%M:%S')
            dt_end = datetime.datetime.strptime(str_end, 
                                                '%Y-%m-%dT%H:%M:%S')
    
            return (dt_start, dt_end)


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
        with utils.open_table(table_filename) as table:
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
        with utils.open_table(data_description_table) as table:
            spw_ids = table.getcol('SPECTRAL_WINDOW_ID')
            pol_ids = table.getcol('POLARIZATION_ID')
            dd_ids = range(len(spw_ids))

            rows = zip(dd_ids, spw_ids, pol_ids)
            return rows


class PolarizationTable(object):
    @staticmethod
    def get_polarizations(ms):
        # read the polarization table and create the objects
        polarizations = [PolarizationTable._create_pol_description(*row) 
                        for row in PolarizationTable._read_table(ms)]
            
        return polarizations            
        
    @staticmethod
    def _create_pol_description(id, num_corr, corr_type, corr_product, flag):
        return domain.Polarization(id, num_corr, corr_type, corr_product, flag)
    
    @staticmethod
    def _read_table(ms):
        """Read the POLARIZATION table of the given measurement set.
        """
        LOG.debug('Analysing POLARIZATION table')
        ms = _get_ms_name(ms)
        polarization_table = os.path.join(ms, 'POLARIZATION')        
        with utils.open_table(polarization_table) as table:
            num_corrs = table.getcol('NUM_CORR')
            vcorr_types = table.getvarcol('CORR_TYPE')
            vcorr_products = table.getvarcol('CORR_PRODUCT')
            flag_rows = table.getcol('FLAG_ROW')

            rowids = []
            corr_types = []
            corr_products = []
            for i in range(table.nrows()):
                rowids.append(i)
                corr_types.append(vcorr_types['r%s'%(i+1)])
                corr_products.append(vcorr_products['r%s'%(i+1)])

            rows = zip(rowids, num_corrs, corr_types, corr_products, flag_rows)
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
        with utils.open_table(source_table) as table:
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
        facility = ObservationTable.get_telescope_name(ms)
        start, _ = ObservationTable.get_time_range(ms)
        state_factory = domain.state.StateFactory(facility, start)        

        states = [state_factory.create_state(*row) 
                  for row in StateTable._read_table(ms)]        
        return states            
        
    @staticmethod
    def _read_table(ms):
        """Read the STATE table of the given measurement set.
        """
        LOG.debug('Analysing STATE table')
        ms = _get_ms_name(ms)
        state_table = os.path.join(ms, 'STATE')

        with utils.open_table(state_table) as table:
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
        with utils.open_table(field_table) as table:
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
