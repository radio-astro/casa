from __future__ import absolute_import

import string
import numpy
import re

import asap

from . import casatools
from . import logging
from . import utils
from . import tablereader
import pipeline.domain as domain
#import pipeline.hsd as hsd

LOG = logging.get_logger(__name__)


def _get_ms_name(ms):
    return ms.name if isinstance(ms, domain.MeasurementSet) else ms


class ObservingRunReader(object):
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
                ms = tablereader.MeasurementSetReader.get_measurement_set(ms_file)
                observing_run.add_measurement_set(ms)
            LOG.info('measurement_sets=%s'%(observing_run.measurement_sets))
            LOG.info('st_ms_map=%s'%(st_ms_map))
            for (st_file,index) in zip(st_files,st_ms_map):
                ms = observing_run.measurement_sets[index] if index >= 0 else None
                st = ScantableReaderFromMS.get_scantable(st_file, ms)
                observing_run.append(st)
        return observing_run


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
        with utils.open_table(st.name) as tb:
            st.observer = tb.getkeyword('Observer')
            # start/end time in MJD [day]
            timecol = tb.getcol('TIME')
            st.start_time = me.epoch(rf='UTC',
                                     v0=qa.quantity(timecol.min(),'d'))
            st.end_time = me.epoch(rf='UTC',
                                   v0=qa.quantity(timecol.max(),'d'))

    @staticmethod
    def get_antenna(st):
        with utils.open_table(st.name) as tb:
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
        #h = hsd.heuristics.AntennaDiameter()
        #diameter = h.calculate(name=name)
        diameter = _antenna_diameter(name=name)
        antenna = domain.Antenna(0, name, m_position, rad_position, diameter, station)

        LOG.trace('created Antenna object\n%s'%(antenna))

        return antenna

    @staticmethod
    def get_source(st):
        me = casatools.measures
        qa = casatools.quanta
        source = {}
        with utils.open_table(st.name) as tb:
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
                    
        with utils.open_table(st.name) as tb:
            frequencies = tb.getkeyword('FREQUENCIES').lstrip('Table: ')
            molecules = tb.getkeyword('MOLECULES').lstrip('Table: ')

        with utils.open_table(frequencies) as tb:
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

        with utils.open_table(molecules) as tb:
            rest_frequencies = {}
            for i in xrange(tb.nrows()):
                mid = tb.getcell('ID',i)
                if tb.iscelldefined('RESTFREQUENCY',i):
                    rest_frequencies[mid] = tb.getcell('RESTFREQUENCY',i)
                else:
                    rest_frequencies[mid] = []

        with utils.open_table(st.name) as tb:
            spw_ids = numpy.unique(tb.getcol('IFNO'))
            for spw in spw_ids:
                ts = tb.query('IFNO==%s'%(spw))

                # spectral setup
                freq_id = ts.getcell('FREQ_ID',0)
                mol_id = ts.getcell('MOLECULE_ID',0)
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
                #import asap.srctype as st
                st = asap.srctype
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
                                                      intent=string.join(intents,':'),
                                                      rest_frequencies=rest_frequencies[mol_id])
                spectral_window[spw] = entry
                                    
                ts.close()

        return spectral_window

    @staticmethod
    def get_polarization(st):
        polarization = {}
        polmap = domain.singledish.Polarization.polarization_map
        with utils.open_table(st.name) as tb:
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
            if spw.is_target and spw.nchan > 1:
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
        s = asap.scantable(st.name,False)
        antenna_name = s.get_antennaname()
        del s

        # get antenna from MS based on its name
        antenna = ms.antenna_array.get_antenna(name=antenna_name)

        return antenna

    @staticmethod
    def get_source(st, ms):
        with utils.open_table(st.name) as tb:
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
        with utils.open_table(name) as tb:
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

            spw_name = tb.getkeyword('SPECTRAL_WINDOW').lstrip('Table: ')
            source_name = tb.getkeyword('SOURCE').lstrip('Table: ')

        with utils.open_table(spw_name) as tb:
            for spw in spectral_window.keys():
                frame_id = tb.getcell('MEAS_FREQ_REF')
                spectral_window[spw].frame = domain.singledish.Frequencies.frame_map[frame_id]

        with utils.open_table(source_name) as tb:
            for spw in spectral_window.keys():
                ts = tb.query('SPECTRAL_WINDOW_ID==%s'%(spw))
                rest_frequencies = []
                for i in xrange(ts.nrows()):
                    if ts.iscelldefined('REST_FREQUENCY',i):
                        rest_frequencies.extend(ts.getcell('REST_FREQUENCY',i))
                rest_frequencies = numpy.unique(rest_frequencies)
                spectral_window[spw].rest_frequencies = rest_frequencies

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
        with utils.open_table(ms.name) as tb:
            spw_table = tb.getkeyword('SPECTRAL_WINDOW').lstrip('Table: ')

        with utils.open_table(spw_table) as tb:
            if 'BBC_NO' in tb.colnames():
                bbc_no = tb.getcol('BBC_NO')
            else:
                bbc_no = [-1] * tb.nrows()
        
        tolerance = domain.singledish.ScantableRep.tolerance
        spw_for_atmcal = []
        spw_for_target = []
        for spw in st.spectral_window.values():
            if spw.is_target and spw.nchan > 1:
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
            
        #import pipeline.domain.virtualms as virtualms
        ms = domain.virtualms.VirtualMeasurementSet(st)
        ms.antenna_array = VirtualMeasurementSetFiller.get_antenna_array(st)
        ms.sources = VirtualMeasurementSetFiller.get_source(st)
        ms.frequency_groups = VirtualMeasurementSetFiller.get_frequency_group(st)
        ms.fields = VirtualMeasurementSetFiller.get_field(st)
        ms.scans = VirtualMeasurementSetFiller.get_scan(st, ms)
        ms.data_descriptions = VirtualMeasurementSetFiller.get_data_description(st)

        ms.observer = st[0].observer
        ms.project_id = 'DUMMY_PROJECT_ID'
        ms.schedblock_id = 'DUMMY_SCHEDBLOCK_ID'
        ms.execblock_id = 'DUMMY_EXECBLOCK_ID'

        return ms

    @staticmethod
    def get_antenna_array(st):
        with utils.open_table(st[0].name) as tb:
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
        with utils.open_table(st[0].name) as tb:
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
        with utils.open_table(st[0].name) as tb:
            scannos = numpy.unique(tb.getcol('SCANNO'))
            for s in scannos:
                tsel = tb.query('SCANNO==%s'%(s))
                ifnos = numpy.unique(tsel.getcol('IFNO'))
                times = tsel.getcol('TIME').tolist()
                unique_times = set(times)
                intervals = tsel.getcol('INTERVAL').tolist()
                unique_intervals = [intervals[times.index(t)] for t in unique_times]
                scan_times = []
                field_names = numpy.unique(tsel.getcol('FIELDNAME'))
                field_names = [v.split('__')[0].replace(' ','_') for v in field_names]
                tsel.close()
                
                for (t,i) in zip(unique_times,unique_intervals):
                    interval = qa.quantity(i,'s')
                    half_interval = qa.div(interval, 2)
                    center_time = qa.quantity(t,'d')
                    start_time = qa.sub(center_time, half_interval)
                    end_time = qa.add(center_time, half_interval)
                    start = me.epoch('UTC', start_time)
                    end = me.epoch('UTC', end_time)
                    scan_times.append((start,end,interval))
                    
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

ANTENNA_LIST = { 'DV[0-6][0-9]': 12.0,
                 'DA[0-6][0-9]': 12.0,
                 'PM0[1-4]': 12.0,
                 'CM[0-1][0-9]': 7.0,
                 'APEX': 12.0,
                 'AP-': 12.0,
                 'NRO': 45.0,
                 'ASTE': 10.0,
                 'MRT': 30.0,
                 'IRAM30m': 30.0,
                 'Effelsberg': 100.0,
                 'GBT': 104.9,
                 'SMT': 10.0,
                 'HHT': 10.0,
                 # from asap/src/STAttr.cpp
                 'MOPRA': 22.0,
                 'PKS': 64.0,
                 'TIDBINBILLA': 70.0,
                 'CEDUNA': 30.0,
                 'HOBART': 26.0 }

def _antenna_diameter(name):
    d = None
    for (key,item) in ANTENNA_LIST.items():
        if re.match(key, name) is not None:
            #print 'matched %s'%(key)
            d = item
            break
    if d is None:
        #raise Exception('No data in lookup table: %s'%(name))
        LOG.warn('%s: No data in lookup table. Set diameter to zero.'%(name))
        d = 0.0
    return d
