from __future__ import absolute_import
import datetime
import itertools
import os
import re
import string
from bisect import bisect_left

import numpy

from . import casatools
from . import logging
import pipeline.domain as domain
import pipeline.domain.measures as measures

LOG = logging.get_logger(__name__)

def find_EVLA_band(frequency, bandlimits=[0.0e6, 150.0e6, 700.0e6, 2.0e9, 4.0e9, 8.0e9, 12.0e9, 18.0e9, 26.5e9, 40.0e9, 56.0e9], BBAND='?4PLSCXUKAQ?'):
    """identify VLA band"""
    i = bisect_left(bandlimits, frequency)

    return BBAND[i]


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
    def get_scans(msmd, ms):
        LOG.debug('Analysing scans in {0}'.format(ms.name))
        with casatools.TableReader(ms.name) as openms:
            scan_number_col = openms.getcol('SCAN_NUMBER')
            time_col = openms.getcol('TIME')
            antenna1_col = openms.getcol('ANTENNA1')
            antenna2_col = openms.getcol('ANTENNA2')
#             exposure_col = openms.getcol('EXPOSURE')
            data_desc_id_col = openms.getcol('DATA_DESC_ID')
#             field_id_col = openms.getcol('FIELD_ID')
#             state_id_col = openms.getcol('STATE_ID')

            # get columns and tools needed to create scan times
            time_colkeywords = openms.getcolkeywords('TIME')
            time_unit = time_colkeywords['QuantumUnits'][0]
            time_ref = time_colkeywords['MEASINFO']['Ref']    
            mt = casatools.measures
            qt = casatools.quanta

            scans = []
            for scan_id in msmd.scannumbers():
                states = [s for s in ms.states
                          if s.id in msmd.statesforscan(scan_id)]

                intents = reduce(lambda s, t: s.union(t.intents), states, set())
                
                fields = [f for f in ms.fields 
                          if f.id in msmd.fieldsforscan(scan_id)]

                # can't use msmd.timesforscan as we need unique times grouped by 
                # spw
#                 scan_times = msmd.timesforscan(scan_id)
                
                exposures = {spw_id : msmd.exposuretime(scan=scan_id, spwid=spw_id)
                             for spw_id in msmd.spwsforscan(scan_id)}
                
                scan_mask = (scan_number_col==scan_id)                

                # get the antennas used for this scan 
                LOG.trace('Calculating antennas used for scan %s', scan_id)
                antenna_ids = set()
                scan_antenna1 = antenna1_col[scan_mask]
                scan_antenna2 = antenna2_col[scan_mask] 
                antenna_ids.update(scan_antenna1)
                antenna_ids.update(scan_antenna2)
                antennas = [o for o in ms.antennas if o.id in antenna_ids]

                # get the data descriptions for this scan
                LOG.trace('Calculating data descriptions used for scan %s', scan_id)
                scan_data_desc_id = set(data_desc_id_col[scan_mask])
                data_descriptions = [o for o in ms.data_descriptions 
                                     if o.id in scan_data_desc_id]

                # times are specified per data description, so we must
                # re-mask and calculate times per dd
                scan_times = {}  
                LOG.trace('Processing scan times for scan %s', scan_id)
                for dd in data_descriptions:
                    dd_mask = (scan_number_col==scan_id) & (data_desc_id_col==dd.id)

                    raw_midpoints = list(time_col[dd_mask])
                    unique_midpoints = set(raw_midpoints)
                    epoch_midpoints = [mt.epoch(time_ref, qt.quantity(o, time_unit))
                                       for o in unique_midpoints]
                    
#                     dd_times = 
#                     for raw_midpoint in unique_midpoints:
#                         # measurement set spec states that exposure is recorded in 
#                         # seconds
#                         exposure = exposures[dd.spw.id]
#                         half_exposure = qt.div(exposure, 2)
#                         
#                         # add and subtract half the exposure to get the start and
#                         # end times for the exposure
#                         midpoint_epoch = qt.quantity(raw_midpoint, time_unit)
#                         start_epoch = qt.sub(midpoint_epoch, half_exposure)
#                         end_epoch = qt.add(midpoint_epoch, half_exposure)
#                         
#                         start = mt.epoch(time_ref, start_epoch)
#                         end = mt.epoch(time_ref, end_epoch)
#                         
#                         dd_times.append((raw_midpoint, exposure))

                    scan_times[dd.spw.id] = zip(epoch_midpoints,
                                                itertools.repeat(exposures[dd.spw.id]))

                LOG.trace('Creating domain object for scan %s', scan_id)
                scan = domain.Scan(id=scan_id, states=states, fields=fields,
                            data_descriptions=data_descriptions, antennas=antennas,
                            scan_times=scan_times, intents=intents)
                scans.append(scan)

                LOG.trace('{0}'.format(scan))

            return scans

    @staticmethod
    def add_band_to_spws(ms):
        for spw in ms.spectral_windows:
            if spw.type == 'WVR':
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
            
            #LOG.info('************************(1) '+spw.band+'********************')
            
            #Used EVLA band name from spw instead of frequency range
            observatory = string.upper(ms.antenna_array.name)
            if observatory in ('VLA', 'EVLA'):
                spw2band = ms.get_vla_spw2band()
                bands = spw2band.values()
            
                try:
                    EVLA_band = spw2band[spw.id]
                except:
                    LOG.info('Unable to get band from spw id - using reference frequency instead')
                    freqHz = float(spw.ref_frequency.value)
                    EVLA_band = find_EVLA_band(freqHz)
                
                EVLA_band_dict = {'4' : '4m (4)',
                                  'P' : '90cm (P)',
                                  'L' : '20cm (L)',
                                  'S' : '13cm (S)',
                                  'C' : '6cm (C)',
                                  'X' : '3cm (X)',
                                  'U' : '2cm (Ku)',
                                  'K' : '1.3cm (K)',
                                  'A' : '1cm (Ka)',
                                  'Q':  '0.7cm (Q)'}
                
                spw.band = EVLA_band_dict[EVLA_band]
       
            #LOG.info('************************(2) '+spw.band+'********************')
            
       
    @staticmethod
    def link_intents_to_spws(msmd, ms):
        # we can't use msmd.intentsforspw directly as we may have a modified
        # obsmode mapping
        for spw in ms.spectral_windows:
            scan_ids = msmd.scansforspw(spw.id)
            state_ids = [msmd.statesforscan(i) for i in scan_ids]
            state_ids = set(itertools.chain(*state_ids))
            states = [s for s in ms.states if s.id in state_ids] 

            for state in states:
                spw.intents.update(state.intents)

            LOG.trace('Intents for spw #{0}: {1}'
                      ''.format(spw.id, ','.join(spw.intents)))
            
#         container = ms.states if ms.states else ms.fields
#         column = 'STATE_ID' if ms.states else 'FIELD_ID'
#        
#         with casatools.TableReader(ms.name) as table:
#             for dd in ms.data_descriptions:
#                 spw = dd.spw
#                 for obj in container:
#                     # if the spw has already been tagged with this state's
#                     # intents, go on to the next state
#                     if obj.intents.issubset(spw.intents):
#                         continue
#                         
#                     subTable = table.query(
#                         'DATA_DESC_ID=={dd_id} '
#                         '&& {column}=={identifier}'
#                         '&& NOT(FLAG_ROW) '
#                         '&& NOT(ALL(FLAG))'.format(dd_id=dd.id, 
#                                                    column=column,
#                                                    identifier=obj.id))
#                     
#                     if subTable.nrows() > 0:
#                         spw.intents.update(obj.intents)
#                     subTable.close()
#         
#         for spw in ms.spectral_windows:
#             LOG.trace('Intents for spw #{0}: {1}'
#                       ''.format(spw.id, ','.join(spw.intents)))
    
    @staticmethod
    def link_fields_to_states(msmd, ms):
        # for each field..
        for field in ms.fields:
            # Find the state IDs for the field by first identifying the scans
            # for the field, then finding the state IDs for those scans
            scan_ids = msmd.scansforfield(field.id)
            state_ids = [msmd.statesforscan(i) for i in scan_ids]
            # flatten the state IDs to a 1D list
            state_ids = set(itertools.chain(*state_ids))            
            states = [ms.get_state(i) for i in state_ids]
            field.states.update(states)
            for state in states:
                field.intents.update(state.intents)
            
    @staticmethod
    def link_fields_to_sources(msmd, ms):        
        for source in ms.sources:
            field_ids = msmd.fieldsforsource(source.id, False)
            fields = [f for f in ms.fields if f.id in field_ids]

            source.fields[:] = fields
            for field in fields:
                field.source = source

    @staticmethod
    def link_spws_to_fields(msmd, ms):
        for field in ms.fields:
            spws = [spw for spw in ms.spectral_windows
                    if spw.id in msmd.spwsforfield(field.id)]
            field.valid_spws.update(spws)
    
    @staticmethod
    def get_measurement_set(ms_file):
        LOG.info('Analysing {0}'.format(ms_file))
        ms = domain.MeasurementSet(ms_file)
        
        # populate ms properties with results of table readers 
        with casatools.MSMDReader(ms_file) as msmd:
            ms.antenna_array = AntennaTable.get_antenna_array(msmd)
            ms.spectral_windows = SpectralWindowTable.get_spectral_windows(msmd)
            ms.states = StateTable.get_states(msmd)
            ms.fields = FieldTable.get_fields(msmd)

            # No MSMD functions to help populating DDs and pols yet
            ms.data_descriptions = DataDescriptionTable.get_descriptions(ms)
            ms.polarizations = PolarizationTable.get_polarizations(ms)
            with casatools.MSReader(ms.name) as openms:
                for dd in ms.data_descriptions:
                    openms.selectinit(dd.id)
                    ms_info = openms.getdata(['axis_info','time'])
                    dd.obs_time = numpy.mean(ms_info['time'])                
                    dd.chan_freq = ms_info['axis_info']['freq_axis']['chan_freq'].tolist()
                    dd.corr_axis = ms_info['axis_info']['corr_axis'].tolist()
    
            ms.sources = SourceTable.get_sources(ms)        

            # now back to pure MSMD calls
            MeasurementSetReader.link_fields_to_states(msmd, ms)
            MeasurementSetReader.link_fields_to_sources(msmd, ms)
            MeasurementSetReader.link_intents_to_spws(msmd, ms)
            MeasurementSetReader.link_spws_to_fields(msmd, ms)
            ms.scans = MeasurementSetReader.get_scans(msmd, ms)

        MeasurementSetReader.add_band_to_spws(ms)

        (observer, project_id, schedblock_id, execblock_id) = ObservationTable.get_project_info(ms)

        # work around NumPy bug with empty strings
        # http://projects.scipy.org/numpy/ticket/1239
        ms.observer = str(observer)
        ms.project_id = str(project_id)

        ms.schedblock_id = schedblock_id
        ms.execblock_id = execblock_id

        return ms

    @staticmethod
    def _get_range(filename, column):
        with casatools.MSReader(filename) as ms:
            data = ms.range([column])
            return data.values()[0]


class SpectralWindowTable(object):
    @staticmethod
    def get_spectral_windows(msmd):
        # map spw ID to spw type
        spw_types = {i:'FDM' for i in msmd.fdmspws()}
        spw_types.update({i:'TDM' for i in msmd.tdmspws()})
        spw_types.update({i:'WVR' for i in msmd.wvrspws()})
        spw_types.update({i:'CHANAVG' for i in msmd.chanavgspws()})
        spw_types.update({i:'SQLD' for i in msmd.almaspws(sqld=True)})

        LOG.trace('Opening SPECTRAL_WINDOW table to read '
                  'SPECTRAL_WINDOW.REF_FREQUENCY')
        spectral_window_table = os.path.join(msmd.name(), 'SPECTRAL_WINDOW')        
        with casatools.TableReader(spectral_window_table) as table:
            ref_frequency = table.getcol('REF_FREQUENCY')

        # these msmd functions don't need a spw argument. They return a list of
        # values, one for each spw
        spw_names = msmd.namesforspws()            
        bandwidths = msmd.bandwidths()

        spws = []        
        for i, spw_name in enumerate(spw_names):
            # get this spw's values from our precalculated lists and dicts
            bandwidth = bandwidths[i]
            ref_freq = ref_frequency[i]
            spw_type = spw_types.get(i, 'UNKNOWN')

            # the following msmd functions need a spw argument, so they have
            # to be contained within the spw loop
            mean_freq = msmd.meanfreq(i)
            chan_freqs = msmd.chanfreqs(i)
            chan_widths = msmd.chanwidths(i)            
            sideband = msmd.sideband(i)
            baseband = msmd.baseband(i)                            
            
            spw = domain.SpectralWindow(i, spw_name, spw_type, bandwidth,
                    ref_freq, mean_freq, chan_freqs, chan_widths, sideband,
                    baseband)
            spws.append(spw)

        return spws


class ObservationTable(object):
    @staticmethod
    def get_telescope_name(ms):
        LOG.debug('Analysing OBSERVATION table')
        vis = _get_ms_name(ms)
        with casatools.MSMDReader(vis) as msmd:
            names = set(msmd.observatorynames())
            assert len(names) is 1
            return names.pop()

    @staticmethod
    def get_project_info(ms):
        ms = _get_ms_name(ms)
        table_filename = os.path.join(ms, 'OBSERVATION')
        with casatools.TableReader(table_filename) as table:
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

        with casatools.TableReader(ms) as openms:
            # get columns and tools needed to create scan times
            time_colkeywords = openms.getcolkeywords('TIME')
            time_unit = time_colkeywords['QuantumUnits'][0]
            time_ref = time_colkeywords['MEASINFO']['Ref']    
            me = casatools.measures
            qa = casatools.quanta
                
        table_filename = os.path.join(ms, 'OBSERVATION')
        with casatools.TableReader(table_filename) as table:
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
    def get_antenna_array(msmd):
        position = msmd.observatoryposition()            
        names = set(msmd.observatorynames())
        assert len(names) is 1
        name = names.pop()
        array = domain.AntennaArray(name, position)

        # .. and add a new Antenna for each row in the ANTENNA table
        for antenna in AntennaTable.get_antennas(msmd):
            array.add_antenna(antenna)
        return array

    @staticmethod
    def get_antennas(msmd):
        antenna_table = os.path.join(msmd.name(), 'ANTENNA')
        LOG.trace('Opening ANTENNA table to read ANTENNA.FLAG_ROW and '
                  'ANTENNA.DISH_DIAMETER')
        with casatools.TableReader(antenna_table) as table:
            flags = table.getcol('FLAG_ROW')
            diameters = table.getcol('DISH_DIAMETER')

        antennas = []
        for (i, name, station) in zip(msmd.antennaids(), 
                                      msmd.antennanames(),
                                      msmd.antennastations()):
            # omit this antenna if it has been flagged
            if flags[i]:
                continue

            position = msmd.antennaposition(i)
            offset = msmd.antennaoffset(i)
            diameter = diameters[i]
            
            antenna = domain.Antenna(i, name, station, position, offset,
                                     diameter)
            antennas.append(antenna)
            
        return antennas
    
    @staticmethod
    def _create_antenna(antenna_id, name, station, diameter, position, offset,
                        flag):
        # omit this antenna if it has been flagged
        if flag is True:
            return

#         # get the x, y, z values from the positions tuple
#         x = position['m0']
#         y = position['m1']
#         z = position['m2']
# 
#         # find out what units these values are in
#         ref_keyword = keywords['MEASINFO']['Ref']
#         x_units = keywords['QuantumUnits'][0]
#         y_units = keywords['QuantumUnits'][1]
#         z_units = keywords['QuantumUnits'][2]
# 
#         # save these as we'll need them to work round a CASA bug which converts
#         # all positions to radians
#         qt = casatools.quanta
#         v0=qt.quantity(x, x_units)
#         v1=qt.quantity(y, y_units)
#         v2=qt.quantity(z, z_units)
# 
#         # so we can create a CASA position..
#         mt = casatools.measures   
#         rad_position = mt.position(rf=ref_keyword, v0=v0, v1=v1, v2=v2)
# 
#         # and now for our workaround..
#         m_position = mt.position(rf=ref_keyword, v0=v0, v1=v1, v2=v2)
#         m_position['m0'] = v0
#         m_position['m1'] = v1
#         m_position['m2'] = v2
    
        # .. with which we can create an Antenna
        antenna = domain.Antenna(antenna_id, name, station, position, offset, 
                                 diameter)
        return antenna


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
        with casatools.TableReader(polarization_table) as table:
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
    def get_states(msmd):
        state_factory = StateTable.get_state_factory(msmd)

        LOG.trace('Opening STATE table to read STATE.OBS_MODE')
        state_table = os.path.join(msmd.name(), 'STATE')
        with casatools.TableReader(state_table) as table:
            obs_modes = table.getcol('OBS_MODE')
        
        states = []
        for i in range(msmd.nstates()):
            obs_mode = obs_modes[i]
            state = state_factory.create_state(i, obs_mode)
            states.append(state)
        return states

    @staticmethod
    def get_state_factory(msmd):
        names = set(msmd.observatorynames())
        assert len(names) is 1
        facility = names.pop()

        first_scan = min(msmd.scannumbers())
        scan_start = min(msmd.timesforscan(first_scan))

        LOG.trace('Opening MS to read TIME keyword to avoid '
                  'msmd.timesforscan() units ambiguity')
        with casatools.TableReader(msmd.name()) as table:
            time_colkeywords = table.getcolkeywords('TIME')
            time_unit = time_colkeywords['QuantumUnits'][0]
            time_ref = time_colkeywords['MEASINFO']['Ref']    
        
        me = casatools.measures
        qa = casatools.quanta
                
        epoch_start = me.epoch(time_ref, qa.quantity(scan_start, time_unit))
        str_start = qa.time(epoch_start['m0'], form=['fits'])[0]
        dt_start = datetime.datetime.strptime(str_start, '%Y-%m-%dT%H:%M:%S')

        return domain.state.StateFactory(facility, dt_start)        


class FieldTable(object):
    @staticmethod
    def _read_table(msmd):
        num_fields = msmd.nfields()
        field_ids = range(num_fields)
        field_names = msmd.namesforfields()
        times = [msmd.timesforfield(i) for i in field_ids]
            
        LOG.trace('Opening FIELD table to read FIELD.PHASE_DIR, '
                  'FIELD.SOURCE_ID and FIELD.SOURCE_TYPE')
        field_table = os.path.join(msmd.name(), 'FIELD')
        with casatools.TableReader(field_table) as table:
            # we need msmd.sourcesforfield() to eliminate this read
            source_ids = table.getcol('SOURCE_ID')

            phase_dir = table.getcol('PHASE_DIR')
            phase_dir_keywords = table.getcolkeywords('PHASE_DIR')
            phase_dir_quanta = []
            for field_id in field_ids:
                field_dir_quanta = [
                        '%s%s' % (phase_dir[0,0,field_id],
                                  phase_dir_keywords['QuantumUnits'][0]),
                        '%s%s' % (phase_dir[1,0,field_id],
                                  phase_dir_keywords['QuantumUnits'][1])]
                phase_dir_quanta.append(field_dir_quanta)

            # get array of phase dir ref types
            meas_info = phase_dir_keywords['MEASINFO']
            if 'VarRefCol' in meas_info:
                # ref varies with row
                phase_dir_ref = table.getcol(meas_info['VarRefCol'])
                phase_dir_ref_types = meas_info['TabRefTypes']
                phase_dir_ref_codes = meas_info['TabRefCodes']
            else:
                # ref fixed for all rows,
                # construct as though variable reference with all refs
                # the same
                phase_dir_ref = numpy.zeros([numpy.shape(phase_dir)[2]], 
                                            numpy.int)
                phase_dir_ref_types = numpy.array([meas_info['Ref']])
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
                source_types = [None] * num_fields

            return zip(field_ids, field_names, source_ids, times, 
                       source_types, phase_dir_ref_type, phase_dir_quanta)
    
    @staticmethod
    def get_fields(msmd):
        return [FieldTable._create_field(*row) 
                for row in FieldTable._read_table(msmd)]
    
    @staticmethod
    def _create_field(field_id, name, source_id, time, source_type,
                      phase_dir_ref_type, phase_dir_quanta):
        # .. with which we can create an Antenna
        field = domain.Field(field_id, name, source_id, time,
                             phase_dir_ref_type, phase_dir_quanta)
        if source_type:
            field.set_source_type(source_type)
        return field    


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

    #From original EVLA pipeline script
    #FLOW = [ 0.0e6, 150.0e6, 700.0e6, 2.0e9, 4.0e9, 8.0e9, 12.0e9, 18.0e9, 26.5e9, 40.0e9 ]
    #FHIGH = [ 150.0e6, 700.0e6, 2.0e9, 4.0e9, 8.0e9, 12.0e9, 18.0e9, 26.5e9, 40.0e9, 56.0e9 ]
    #BBAND = [ '4', 'P', 'L', 'S', 'C', 'X', 'U', 'K', 'A', 'Q' ]

    evla_bands = {_make_range(0.7, 2.0) : '20cm (L)',
                  _make_range(2.0, 4.0) : '13cm (S)',
                  _make_range(4, 8) : '6cm (C)',
                  _make_range(8, 12) : '3cm (X)',
                  _make_range(12, 18) : '2cm (Ku)',
                  _make_range(18, 26.5) : '1.3cm (K)',
                  _make_range(26.5, 40) : '1cm (Ka)',
                  _make_range(40, 56.0) : '0.7cm (Q)'}
    
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
