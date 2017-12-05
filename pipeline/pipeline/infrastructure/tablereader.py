from __future__ import absolute_import
import datetime
import itertools
import operator
import os
import re
import string
from bisect import bisect_left

import cachetools
import numpy

from . import casatools
from . import logging
import pipeline.domain as domain
import pipeline.domain.measures as measures
# import pipeline.extern.profilehooks as profilehooks


LOG = logging.get_logger(__name__)


def find_EVLA_band(frequency, bandlimits=None, BBAND='?4PLSCXUKAQ?'):
    """identify VLA band"""
    if bandlimits is None:
        bandlimits = [0.0e6, 150.0e6, 700.0e6, 2.0e9, 4.0e9, 8.0e9, 12.0e9,
                      18.0e9, 26.5e9, 40.0e9, 56.0e9]
    i = bisect_left(bandlimits, frequency)

    return BBAND[i]


def _get_ms_name(ms):
    return ms.name if isinstance(ms, domain.MeasurementSet) else ms

def _get_science_goal_value(science_goals, goal_keyword):
    value = None
    for science_goal in science_goals:
        keyword = science_goal.split('=')[0].replace(' ', '')
        if keyword != goal_keyword:
            continue
        value = science_goal.split('=')[1].replace(' ', '')
        return value
    return value


class ObservingRunReader(object):
    @staticmethod
    def get_observing_run(ms_files):
        if isinstance(ms_files, str):
            ms_files = [ms_files]

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
            data_desc_id_col = openms.getcol('DATA_DESC_ID')

            # get columns and tools needed to create scan times
            time_colkeywords = openms.getcolkeywords('TIME')
            time_unit = time_colkeywords['QuantumUnits'][0]
            time_ref = time_colkeywords['MEASINFO']['Ref']    
            mt = casatools.measures
            qt = casatools.quanta

            scans = []
            statesforscans = msmd.statesforscans()
            fieldsforscans = msmd.fieldsforscans(asmap=True, arrayid=0, obsid=0)
            spwsforscans = msmd.spwsforscans()

            for scan_id in msmd.scannumbers():
                states = [s for s in ms.states
                          if s.id in statesforscans[str(scan_id)]]

                intents = reduce(lambda s, t: s.union(t.intents), states, set())
                
                fields = [f for f in ms.fields
                          if f.id in fieldsforscans[str(scan_id)]]

                # can't use msmd.timesforscan as we need unique times grouped by 
                # spw
#                 scan_times = msmd.timesforscan(scan_id)
                
                exposures = {spw_id: msmd.exposuretime(scan=scan_id, spwid=spw_id)
                             for spw_id in spwsforscans[str(scan_id)]}
                
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
            m = re.search(r'ALMA_RB_(?P<band>\d+)', spw.name)
            if m:
                band_str = m.groupdict()['band']
                band_num = int(band_str)
                spw.band = 'ALMA Band %s' % band_num
                continue
            
            spw.band = BandDescriber.get_description(spw.ref_frequency, 
                    observatory=ms.antenna_array.name)
            
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

        scansforspws = msmd.scansforspws()

        # with one scan containing many spws, many of the statesforscan
        # arguments are repeated. This cache speeds up the subsequent
        # duplicate calls.
        cached_states = cachetools.LRUCache(maxsize=1000, missing=msmd.statesforscan)

        for spw in ms.spectral_windows:
            scan_ids = scansforspws[str(spw.id)]
            state_ids = [cached_states[i] for i in scan_ids]
            state_ids = set(itertools.chain(*state_ids))
            states = [s for s in ms.states if s.id in state_ids] 

            for state in states:
                spw.intents.update(state.intents)

            LOG.trace('Intents for spw #{0}: {1}'
                      ''.format(spw.id, ','.join(spw.intents)))
            
    @staticmethod
    def link_fields_to_states(msmd, ms):
        # for each field..

        # SJW commented out for CASA 4.7.74
        # scansforfields = msmd.scansforfields()

        cached_states = cachetools.LRUCache(1000, missing=msmd.statesforscan)

        for field in ms.fields:
            # Find the state IDs for the field by first identifying the scans
            # for the field, then finding the state IDs for those scans

            try:
                scan_ids = msmd.scansforfield(field.id)
            except:
                LOG.debug("Field " + str(field.id) + " not in scansforfields dictionary.")
                continue

            state_ids = [cached_states[i] for i in scan_ids]
            # flatten the state IDs to a 1D list
            state_ids = set(itertools.chain(*state_ids))
            states = [ms.get_state(i) for i in state_ids]

            # some scans may have multiple fields and/or intents so
            # it is necessary to distinguish which intents belong to
            # each field
            obs_modes_for_field = set(msmd.intentsforfield(field.id))
            states_for_field = [s for s in states \
                if not obs_modes_for_field.isdisjoint(s.obs_mode.split(','))]

            field.states.update(states_for_field)
            for state in states_for_field:
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
        spwsforfields = msmd.spwsforfields()
        for field in ms.fields:
            try:
                spws = [spw for spw in ms.spectral_windows if spw.id in spwsforfields[str(field.id)]]
                field.valid_spws.update(spws)
            except:
                LOG.debug("Field "+str(field.id) + " not in spwsforfields dictionary.")

    
    @staticmethod
    def get_measurement_set(ms_file):
        LOG.info('Analysing {0}'.format(ms_file))
        ms = domain.MeasurementSet(ms_file)
        
        # populate ms properties with results of table readers 
        with casatools.MSMDReader(ms_file) as msmd:
            LOG.info('Populating ms.antenna_array...')
            ms.antenna_array = AntennaTable.get_antenna_array(msmd)
            LOG.info('Populating ms.spectral_windows...')
            ms.spectral_windows = SpectralWindowTable.get_spectral_windows(msmd)
            LOG.info('Populating ms.states...')
            ms.states = StateTable.get_states(msmd)
            LOG.info('Populating ms.fields...')
            ms.fields = FieldTable.get_fields(msmd)
            LOG.info('Populating ms.sources...')
            ms.sources = SourceTable.get_sources(msmd)
            LOG.info('Populating ms.data_descriptions...')
            ms.data_descriptions = DataDescriptionTable.get_descriptions(msmd, ms)
            LOG.info('Populating ms.polarizations...')
            ms.polarizations = PolarizationTable.get_polarizations(msmd)
            # For now the SBSummary table is ALMA specific
            if 'ALMA' in msmd.observatorynames():
                sbinfo = SBSummaryTable.get_sbsummary_info(ms, msmd.observatorynames())
                if not sbinfo[0]:
                    LOG.warn('Unable to identify representative target for %s' % (ms.basename))
                else:
                    LOG.info('Populating ms.representative_target ...')
                    ms.representative_target = (sbinfo[0], sbinfo[1], sbinfo[2])
                if not (sbinfo[3] and sbinfo[4]):
                    observing_mode = SBSummaryTable.get_observing_mode(ms)
                    # Only warn if the number of 12m antennas is greater than the number of 7m antennas
                    # and if the observation is not single dish
                    if len([a for a in ms.get_antenna() if a.diameter == 12.0]) > len([a for a in ms.get_antenna() if a.diameter == 7.0]) \
                      and 'Standard Single Dish' not in observing_mode:
                        LOG.warn('Undefined angular resolution limits for %s' % (ms.basename))
                    ms.science_goals = {'minAcceptableAngResolution': '0.0arcsec', 'maxAcceptableAngResolution': '0.0arcsec'}
                else:
                    LOG.info('Populating ms.science_goals ...')
                    ms.science_goals = {'minAcceptableAngResolution': sbinfo[3], 'maxAcceptableAngResolution': sbinfo[4]}
            LOG.info('Populating ms.array_name ...')
            # No MSMD functions to help populating the ASDM_EXECBLOCK table
            ms.array_name = ExecblockTable.get_execblock_info(ms)

            with casatools.MSReader(ms.name) as openms:
                for dd in ms.data_descriptions:
                    openms.selectinit(reset=True)
                    openms.selectinit(datadescid=dd.id)
                    ms_info = openms.getdata(['axis_info','time'])

                    dd.obs_time = numpy.mean(ms_info['time'])
                    dd.chan_freq = ms_info['axis_info']['freq_axis']['chan_freq'].tolist()
                    dd.corr_axis = ms_info['axis_info']['corr_axis'].tolist()
    
            # now back to pure MSMD calls
            LOG.info('Linking fields to states...')
            MeasurementSetReader.link_fields_to_states(msmd, ms)
            LOG.info('Linking fields to sources...')
            MeasurementSetReader.link_fields_to_sources(msmd, ms)
            LOG.info('Linking intents to spws...')
            MeasurementSetReader.link_intents_to_spws(msmd, ms)
            LOG.info('Linking spectral windows to fields...')
            MeasurementSetReader.link_spws_to_fields(msmd, ms)
            LOG.info('Populating ms.scans...')
            ms.scans = MeasurementSetReader.get_scans(msmd, ms)

            (observer, project_id, schedblock_id, execblock_id) = ObservationTable.get_project_info(msmd)

        MeasurementSetReader.add_band_to_spws(ms)

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
        spw_types = {i: 'FDM' for i in msmd.fdmspws()}
        spw_types.update({i: 'TDM' for i in msmd.tdmspws()})
        spw_types.update({i: 'WVR' for i in msmd.wvrspws()})
        spw_types.update({i: 'CHANAVG' for i in msmd.chanavgspws()})
        spw_types.update({i: 'SQLD' for i in msmd.almaspws(sqld=True)})

        # these msmd functions don't need a spw argument. They return a list of
        # values, one for each spw
        spw_names = msmd.namesforspws()            
        bandwidths = msmd.bandwidths()

        # We need the first TARGET source ID to get the correct transitions
        try:
            first_target_field_id = msmd.fieldsforintent('*TARGET*')[0]
            first_target_source_id = msmd.sourceidforfield(first_target_field_id)
        except:
            first_target_source_id = 0

        target_spw_ids = msmd.spwsforintent('*TARGET*')

        spws = []
        for i, spw_name in enumerate(spw_names):
            # get this spw's values from our precalculated lists and dicts
            bandwidth = bandwidths[i]
            spw_type = spw_types.get(i, 'UNKNOWN')

            # the following msmd functions need a spw argument, so they have
            # to be contained within the spw loop
            mean_freq = msmd.meanfreq(i)
            chan_freqs = msmd.chanfreqs(i)
            chan_widths = msmd.chanwidths(i)            
            chan_effective_bws = msmd.chaneffbws(i)
            sideband = msmd.sideband(i)
            ### BBC_NO column is optional
            if 'NRO' in msmd.observatorynames():
                ### For Nobeyama (TODO: how to define BBC_NO for NRO)
                baseband = i
            else:
                baseband = msmd.baseband(i)

            ref_freq = msmd.reffreq(i)
            # Read transitions for target spws. Other spws may cause severe
            # messages because the target source IDs may not have the spw.
            if i in target_spw_ids:
                try: ### TRANSITIONS column does not exist in old data
                    # TODO: Are the transitions of a given spw the same for all
                    #       target source IDs ?
                    transitions = msmd.transitions(sourceid=first_target_source_id, spw=i)
                    if transitions is False:
                        transitions = ['Unknown']
                except:
                    transitions = ['Unknown']
            else:
                transitions = ['Unknown']

            spw = domain.SpectralWindow(i, spw_name, spw_type, bandwidth,
                    ref_freq, mean_freq, chan_freqs, chan_widths,
                    chan_effective_bws, sideband, baseband, transitions=transitions)
            spws.append(spw)

        return spws


class ObservationTable(object):
    @staticmethod
    def get_project_info(msmd):
        project_id = msmd.projects()[0]
        observer = msmd.observers()[0]

        schedblock_id = 'N/A'
        execblock_id = 'N/A'

        obsnames = msmd.observatorynames()

        if 'ALMA' in obsnames or 'VLA' in obsnames or 'EVLA' in obsnames:
            # TODO this would break if > 1 observation in an EB. Can that
            # ever happen?
            d = {}
            for cell in msmd.schedule(0):
                key, val = string.split(cell)
                d[key] = val

            schedblock_id = d.get('SchedulingBlock', 'N/A')
            execblock_id = d.get('ExecBlock', 'N/A')

        return observer, project_id, schedblock_id, execblock_id


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
        LOG.trace('Opening ANTENNA table to read ANTENNA.FLAG_ROW')
        with casatools.TableReader(antenna_table) as table:
            flags = table.getcol('FLAG_ROW')

        antennas = []
        for (i, name, station) in zip(msmd.antennaids(), 
                                      msmd.antennanames(),
                                      msmd.antennastations()):
            # omit this antenna if it has been flagged
            if flags[i]:
                continue

            position = msmd.antennaposition(i)
            offset = msmd.antennaoffset(i)
            diameter_m = casatools.quanta.convert(msmd.antennadiameter(i), 'm')
            diameter = casatools.quanta.getvalue(diameter_m)[0]

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

        return domain.Antenna(antenna_id, name, station, position, offset,
                              diameter)


class DataDescriptionTable(object):
    @staticmethod
    def get_descriptions(msmd, ms):
        spws = ms.spectral_windows
        # read the data descriptions table and create the objects
        descriptions = [DataDescriptionTable._create_data_description(spws, *row) 
                        for row in DataDescriptionTable._read_table(msmd)]
            
        return descriptions            
        
    @staticmethod
    def _create_data_description(spws, dd_id, spw_id, pol_id):
        # find the SpectralWindow matching the given spectral window ID
        matching_spws = [spw for spw in spws if spw.id == spw_id]
        spw = matching_spws[0]
        
        return domain.DataDescription(dd_id, spw, pol_id)
    
    @staticmethod
    def _read_table(msmd):
        """
        Read the DATA_DESCRIPTION table of the given measurement set.
        """
        LOG.debug('Analysing DATA_DESCRIPTION table')

        dd_ids = msmd.datadescids()
        spw_ids = msmd.spwfordatadesc()
        pol_ids = msmd.polidfordatadesc()

        return zip(dd_ids, spw_ids, pol_ids)


class SBSummaryTable(object):
    @staticmethod
    def get_sbsummary_info(ms, obsnames):
        try:
            sbsummary_info = [SBSummaryTable._create_sbsummary_info(*row) 
               for row in SBSummaryTable._read_table(ms)]
            return sbsummary_info[0]
        except:
            if 'ALMA' in obsnames:
                LOG.warn('Error reading science goals for %s' % (ms.basename))
            return (None, None, None, None, None)

    @staticmethod
    def get_observing_mode(ms):
        msname = _get_ms_name(ms)
        sbsummary_table = os.path.join(msname, 'ASDM_SBSUMMARY')
        observing_modes = []
        with casatools.TableReader(sbsummary_table) as tb:
            observing_mode = tb.getcol('observingMode')
            for irow in xrange(tb.nrows()):
                cell = observing_mode[:,irow]
                for mode in cell:
                    if mode not in observing_modes:
                        observing_modes.append(mode)
        return observing_modes

    @staticmethod
    def _create_sbsummary_info(repSource, repFrequency, repBandwidth, minAngResolution, maxAngResolution):
       return (repSource, repFrequency, repBandwidth, minAngResolution, maxAngResolution) 

    @staticmethod
    def _read_table(ms):
        """
        Read the ASDM_SBSummary table
        For all practical purposes this table consists of a single row
        but handle the more general case
        """
        LOG.debug('Analysing ASDM_SBSummary table')
        me = casatools.measures
        qa = casatools.quanta
        msname = _get_ms_name(ms)
        sbsummary_table = os.path.join(msname, 'ASDM_SBSUMMARY')        
        with casatools.TableReader(sbsummary_table) as table:
            try:
                scienceGoals = table.getcol('scienceGoal')
                numScienceGoals = table.getcol('numScienceGoal')
            except:
                #LOG.warn('Error reading science goals for %s' % (ms.basename))
                raise 

            repSources = []
            repFrequencies = []
            repBandWidths = []
            minAngResolutions = []
            maxAngResolutions = []
            for i in range(table.nrows()):

                # Create source
                repSource = _get_science_goal_value (scienceGoals[0:numScienceGoals[i],i],
                    'representativeSource')
                repSources.append(repSource)

                # Create frequency
                repFrequency = qa.quantity(_get_science_goal_value (scienceGoals[0:numScienceGoals[i],i],
                    'representativeFrequency'))
                if repFrequency['value'] <= 0.0 or repFrequency['unit'] == '':
                    repFrequency = None
                repFrequencies.append(repFrequency)

                # Create representative bandwidth
                repBandWidth = qa.quantity(_get_science_goal_value (scienceGoals[0:numScienceGoals[i],i],
                    'representativeBandwidth'))
                if repBandWidth['value'] <= 0.0 or repBandWidth['unit'] == '':
                    repBandWidth = None
                repBandWidths.append(repBandWidth)

                # Create minimum angular resolution
                minAngResolution = qa.quantity(_get_science_goal_value (scienceGoals[0:numScienceGoals[i],i],
                    'minAcceptableAngResolution'))
                if minAngResolution['value'] <= 0.0 or minAngResolution['unit'] == '':
                    minAngResolution = None
                minAngResolutions.append(minAngResolution)

                # Create maximum angular resolution
                maxAngResolution = qa.quantity(_get_science_goal_value (scienceGoals[0:numScienceGoals[i],i],
                    'maxAcceptableAngResolution'))
                if maxAngResolution['value'] <= 0.0 or maxAngResolution['unit'] == '':
                    maxAngResolution = None
                maxAngResolutions.append(maxAngResolution)

        rows = zip(repSources, repFrequencies, repBandWidths, minAngResolutions, maxAngResolutions)
        return rows

class ExecblockTable(object):
    @staticmethod
    def get_execblock_info(ms):
        try:
            execblock_info = [ExecblockTable._create_execblock_info(*row) 
               for row in ExecblockTable._read_table(ms)]
            if execblock_info[0][0] == 'ALMA':
                if execblock_info[0][1] == 'A':
                    return None
                else:
                    return execblock_info[0][1]             
            else:
                return execblock_info[0][1]             
        except:
            return None
           
    @staticmethod
    def _create_execblock_info(telescopeName, configName):
       return (telescopeName, configName) 

    @staticmethod
    def _read_table(ms):
        """
        Read the ASDM_EXECBLOCK table
        For all practical purposes this table consists of a single row
        but handle the more general case
        """
        LOG.debug('Analysing ASDM_EXECBLOCK table')
        msname = _get_ms_name(ms)
        execblock_table = os.path.join(msname, 'ASDM_EXECBLOCK')        
        with casatools.TableReader(execblock_table) as table:
            telescopeNames = table.getcol('telescopeName')
            configNames = table.getcol('configName')

        # In case multiple columns are extracted at some point
        # in which case rows would be constructed from the zipped
        # columns
        rows = zip(telescopeNames, configNames)
        return rows


class PolarizationTable(object):
    @staticmethod
    def get_polarizations(msmd):
        pol_ids = sorted(set(msmd.polidfordatadesc()))

        num_corrs = [msmd.ncorrforpol(i) for i in pol_ids]
        corr_types = [msmd.corrtypesforpol(i) for i in pol_ids]
        corr_products = [msmd.corrprodsforpol(i) for i in pol_ids]

        rows = zip(pol_ids, num_corrs, corr_types, corr_products)

        return [PolarizationTable._create_pol_description(*row) for row in rows]

    @staticmethod
    def _create_pol_description(id, num_corr, corr_type, corr_product):
        return domain.Polarization(id, num_corr, corr_type, corr_product)


class SourceTable(object):
    @staticmethod
    def get_sources(msmd):
        rows = SourceTable._read_table(msmd)

        # duplicate source entries may be present due to duplicate entries
        # differing by non-essential columns, such as spw
        key_fn = operator.itemgetter(0)
        data = sorted(rows, key=key_fn)
        grouped_by_source_id = []
        for _, g in itertools.groupby(data, key_fn):
            grouped_by_source_id.append(list(g))

        no_dups = [s[0] for s in grouped_by_source_id]

        return [SourceTable._create_source(*row)
                for row in no_dups]

    @staticmethod
    def _create_source(source_id, name, direction, proper_motion):
        return domain.Source(source_id, name, direction, proper_motion)
        
    @staticmethod
    def _read_table(msmd):
        """
        Read the SOURCE table of the given measurement set.
        """
        LOG.debug('Analysing SOURCE table')
        ids = msmd.sourceidsfromsourcetable()
        sourcenames = msmd.sourcenames()
        directions = [v for _, v in sorted(msmd.sourcedirs().items(),
                                           key=lambda (k, _): int(k))]
        propermotions = [v for _, v in sorted(msmd.propermotions().items(),
                                              key=lambda (k, _): int(k))]

        all_sources = zip(ids, sourcenames, directions, propermotions)

        # Only return sources for which scans are present.
        # Create a mapping of source id to a boolean of whether any
        # scans are present for that source, using fields to link
        # between sources and scans.
        source_id_to_scans = {}
        for source_id in set(msmd.sourceidsfromsourcetable()):
            fields_for_source = set(msmd.fieldsforsource(source_id))
            # Fields do not necessarily have associated scans. If only the
            # first field is tested and gives a negative result, CAS-9499
            # results (AttributeError: 'Field' object has no attribute
            # 'source'). Prevent this by testing all fields for the
            # presence of scans.
            source_id_to_scans[source_id] = any([len(msmd.scansforfield(field_id)) is not 0
                                                 for field_id in fields_for_source])

        return [row for row in all_sources if source_id_to_scans.get(row[0], False)]


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
        phase_centres = [msmd.phasecenter(i) for i in field_ids]
        source_ids = [msmd.sourceidforfield(i) for i in field_ids]

        LOG.trace('Opening FIELD table to read FIELD.SOURCE_TYPE')
        field_table = os.path.join(msmd.name(), 'FIELD')
        with casatools.TableReader(field_table) as table:
            # TODO can this old code be removed? We've not handled non-APDMs
            # for a *long* time!
            #
            # FIELD.SOURCE_TYPE contains the intents in non-APDM MS
            if 'SOURCE_TYPE' in table.colnames():
                source_types = table.getcol('SOURCE_TYPE')
            else:
                source_types = [None] * num_fields

        all_fields = zip(field_ids, field_names, source_ids, times, source_types, phase_centres)

        # only return sources for which scans are present
        # create a mapping of source id to a boolean of whether any scans are present for that source
        field_id_to_scans = {field_id: (len(msmd.scansforfield(field_id)) is not 0)
                             for field_id in set(field_ids)}

        return [row for row in all_fields if field_id_to_scans.get(row[0], False)]

    @staticmethod
    def get_fields(msmd):
        return [FieldTable._create_field(*row) 
                for row in FieldTable._read_table(msmd)]
    
    @staticmethod
    def _create_field(field_id, name, source_id, time, source_type, phase_centre):
        field = domain.Field(field_id, name, source_id, time, phase_centre)

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
