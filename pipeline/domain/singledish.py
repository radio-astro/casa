from __future__ import absolute_import
import numpy
import os
import re

from . import antenna
from . import frequencygroup
from . import measures
from . import observingrun
from . import source
from . import spectralwindow
from .datatable import DataTableImpl as DataTable

import pipeline.hsd.heuristics as heuristics
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)


def to_numeric_freq(m, unit=measures.FrequencyUnits.HERTZ):
    return float(m.convert_to(unit).value)

class ScantableList(observingrun.ObservingRun, list):
    def __init__(self):
        super(ScantableList,self).__init__()
        self.reduction_group = {}
        self.grid_position = {}
        self.datatable_instance = None
        self.datatable_name = None#os.path.join(context.name,'DataTable.tbl')

    def __getstate__(self):
        mydict = self.__dict__.copy()
        # not to pickle DataTable instance
        mydict['datatable_instance'] = None
        return mydict

    def __setstate__(self, d):
        self.__dict__ = d
        name = self.__dict__['datatable_name']
        if name is not None and self.__dict__['datatable_instance'] is None:
            datatable = DataTable(name)
            self.__dict__['datatable_instance'] = datatable
        

    @property
    def start_time(self):
        if len(self) == 0:
            return None
        qt = casatools.quanta
        s = sorted(self, 
                   key=lambda st: st.start_time['m0'],
                   cmp=lambda x,y: 1 if qt.gt(x,y) else 0 if qt.eq(x,y) else -1)
        return s[0].start_time

    @property
    def end_time(self):
        if len(self) == 0:
            return None
        qt = casatools.quanta
        s = sorted(self, 
                   key=lambda ms: ms.end_time['m0'],
                   cmp=lambda x,y: 1 if qt.gt(x,y) else 0 if qt.eq(x,y) else -1)
        return s[-1].end_time

    @property
    def st_names(self):
        return [st.basename for st in self]

    def set_datatable(self, instance, name=None):
        self.datatable_instance = instance
        if name is not None:
            self.datatable_name = name

        # compile domain object with datatable to generate
        # strategies and to create groups
        self.compile(self.datatable_instance)

    def add_scantable(self, s):
        if s.basename in self.st_names:
            msg = '%s is already in the pipeline context'%(s.name)
            LOG.error(msg)
            raise Exception, msg

        self.append(s)
        if s.ms_name:
            ms = self.get_ms(name=s.ms_name)
            if hasattr(ms, 'scantables'):
                ms.scantables.append(s)
            else:
                ms.scantables = [s]

    def get_spw_for_wvr(self, name):
        st = self.get_scantable(name)
        spw = st.spectral_window
        returned_list = []
        for (k,v) in spw.items():
            if v.type == 'WVR':
                returned_list.append(k)
        return returned_list

    def get_spw_without_wvr(self, name):
        st = self.get_scantable(name)
        spw = st.spectral_window
        returned_list = []
        for (k,v) in spw.items():
            if v.type == 'WVR':
                continue
            returned_list.append(k)
        return returned_list

    def get_calmode(self, name):
        st = self.get_scantable(name)
        return st.calibration_strategy['calmode']
        
    def get_scantable(self, name):
        if isinstance(name, str):
            for entry in self:
                if entry.basename == name:
                    return entry
            return None
        else:
            # should be integer index
            return self[name]

    def compile(self, datatable):
        # update reduction group
        self.update_reduction_group()

        # establish calibration strategy
        self.establish_calibration_strategy()

        # set beam size
        self.set_beam_size()

        # select rows
        self.select_data(datatable)

        # grouping
        self.group_data(datatable)

        # configure grid property
        self.configure_grid(datatable)

    def update_reduction_group(self):
        # reset reduction_group
        self.reduction_group.clear()
        
        for idx in xrange(len(self)):
            entry = self[idx]
            spw_list = entry.spectral_window

            # filter out non-target spws
            target_spw = []
            for (k,v) in spw_list.items():
                if v.type == 'WVR':
                    continue
                if re.match('TARGET',v.intent):
                    target_spw.append(k)

            # update reduction_group 
            for spw in target_spw:
                spw_property = self[idx].spectral_window[spw]
                pol_properties = self[idx].polarization
                freq_range = spw_property.frequency_range
                nchan = spw_property.nchan
                pols = pol_properties[spw_property.pol_association[0]].polno
                match = False
                for (k,v) in self.reduction_group.items():
                    group_range = v['freq_range']
                    group_nchan = v['nchan']
                    overlap = max( 0.0, min(group_range[1],freq_range[1]) \
                                   - max(group_range[0],freq_range[0]))
                    width = max(group_range[1],freq_range[1]) \
                            - min(group_range[0],freq_range[0])
                    coverage = overlap/width
                    if nchan == group_nchan and coverage >= 0.99:
                        match = k
                        break
                if match is False:
                    # add new group
                    key = len(self.reduction_group)
                    self.reduction_group[key] = {
                        'freq_range': freq_range,
                        'nchan': nchan,
                        'member': [[idx,spw,pols]]
                        }
                else:
                    self.reduction_group[match]['member'].append([idx,spw,pols])

        print 'reduction_group:\n%s'%(self.reduction_group)
                    

    def establish_calibration_strategy(self):
        for item in self:
            # strategy for tsys transfer
            if item.tsys_transfer:
                tsys_strategy = item.tsys_transfer_list
            else:
                tsys_strategy = None

            # strategy for off-position calibration
            h = heuristics.CalibrationTypeHeuristics()
            calmode = h(item.name)
            
            entry = {
                'tsys': item.tsys_transfer,
                'tsys_strategy': tsys_strategy,
                'calmode': calmode
                }

            LOG.info('calibration strategy for %s:\n%s'%(item.basename,entry))

            # register strategy to domain object
            item.calibration_strategy = entry
            

        LOG.todo('implement strategy for elevation correction')
        LOG.todo('implement strategy for flux calibration')
        
    def set_beam_size(self):
        qa = casatools.quanta

        h = heuristics.SingleDishBeamSizeFromName()
        for item in self:
            antenna = item.antenna.name
            entry = {}
            for (spw,v) in item.spectral_window.items():
                center_freq = 0.5 * sum(v.frequency_range) * 1.0e-9 #GHz
                beam_size = qa.quantity(h(antenna, center_freq), 'arcsec')
                entry[spw] = beam_size

            # register beam size to domain object
            item.beam_size = entry

    def select_data(self, datatable):
        from asap import srctype as st
        qa = casatools.quanta

        # data selection
        self.by_antenna = {}
        self.by_spw = {}
        self.by_pol = {}
        on_source = [st.pson, st.nod, st.fson]
        ant = datatable.getcol('ANTENNA')
        srt = datatable.getcol('SRCTYPE')
        spw = datatable.getcol('IF')
        pol = datatable.getcol('POL')
        for i in xrange(datatable.nrows()):
            thisant = ant[i]
            thisspw = spw[i]
            thispol = pol[i]
            spw_prop = self[thisant].spectral_window[thisspw]
            if spw_prop.type == 'WVR' or \
                   re.match('TARGET',spw_prop.intent) is None or \
                   srt[i] not in on_source:
                continue

            if not self.by_antenna.has_key(thisant):
                self.by_antenna[thisant] = set()
            self.by_antenna[thisant].add(i)

            if not self.by_spw.has_key(thisspw):
                self.by_spw[thisspw] = set()
            self.by_spw[thisspw].add(i)

            if not self.by_pol.has_key(thispol):
                self.by_pol[thispol] = set()
            self.by_pol[thispol].add(i)

    def group_data(self, datatable):
        qa = casatools.quanta

        pos_heuristic2 = heuristics.GroupByPosition2()
        obs_heuristic2 = heuristics.ObservingPattern2()
        time_heuristic2 = heuristics.GroupByTime2()
        merge_heuristic2 = heuristics.MergeGapTables2()
        ra = datatable.getcol('RA')
        dec = datatable.getcol('DEC')
        row = datatable.getcol('ROW')
        elapsed = datatable.getcol('ELAPSED')
        beam = datatable.getcol('BEAM')
        posgrp = numpy.zeros(len(datatable), dtype=int)
        timegrp = [numpy.zeros(len(datatable), dtype=int),
                   numpy.zeros(len(datatable), dtype=int)]
        posgrp_rep = {}
        posgrp_list = {}
        timegrp_list = {}
        timegap = [{},{}]
        last_ra = None
        last_dec = None
        last_time = None
        pos_dict = None
        pos_gap = None
        time_table = None
        time_gap = None
        merge_table = None
        merge_gap = None
        self.pattern = {}

        posgrp_id = 0
        timegrp_id = [0,0]

        for (ant,vant) in self.by_antenna.items():
            pattern_dict = {}
            st = self[ant]
            for i in (0,1):
                timegap[i][ant] = {}
            posgrp_list[ant] = {}
            timegrp_list[ant] = {}
            for (spw,vspw) in self.by_spw.items():
                spectral_window = st.spectral_window[spw]
                pattern_dict[spw] = {}
                # beam radius
                radius = qa.mul(st.beam_size[spw],0.5)
                r_combine = radius
                r_allowance = qa.mul(radius, 0.1)
                for i in (0,1):
                    timegap[i][ant][spw] = {}
                posgrp_list[ant][spw] = {}
                timegrp_list[ant][spw] = {}
                for (pol,vpol) in self.by_pol.items():
                    id_list = list(vant & vspw & vpol)
                    id_list.sort()
                    row_sel = numpy.take(row, id_list)
                    ra_sel = numpy.take(ra, id_list)
                    dec_sel = numpy.take(dec, id_list)
                    time_sel = numpy.take(elapsed, id_list)
                    beam_sel = numpy.take(beam, id_list)

                    ### new GroupByPosition with translation ###
                    update_pos = (last_ra is None or \
                                  len(ra_sel) != len(last_ra) or \
                                  len(dec_sel) != len(last_dec) or \
                                  not (all(ra_sel==last_ra) and \
                                       all(dec_sel==last_dec)))
                    if update_pos:
                        (pos_dict,pos_gap) = pos_heuristic2(ra_sel, dec_sel,
                                                            r_combine, r_allowance)
                        last_ra = ra_sel
                        last_dec = dec_sel

                        ### ObsPatternAnalysis ###
                        pattern = obs_heuristic2(pos_dict)
                        
                    
                    ### prepare for DataTable ###
                    posgrp_list[ant][spw][pol] = []
                    for (k,v) in pos_dict.items():
                        if v[0] == -1:
                            continue
                        posgrp_rep[posgrp_id] = id_list[v[0]]
                        for id in v:
                            posgrp[id_list[id]] = posgrp_id
                        posgrp_list[ant][spw][pol].append(posgrp_id)
                        posgrp_id += 1
                    ###

                    ### new GroupByTime with translation ###
                    time_diff = time_sel[1:] - time_sel[:-1]
                    update_time = (last_time is None \
                                   or len(time_diff) != len(last_time) or \
                                   not all(time_diff == last_time))
                    if update_time:
                        (time_table,time_gap) = time_heuristic2(time_sel, time_diff)
                        last_time = time_diff

                    ### new MergeGapTable with translation ###
                    if update_pos or update_time:
                        (merge_table,merge_gap) = merge_heuristic2(time_gap, time_table, pos_gap, beam_sel)

                    ### prepare for DataTable ###
                    key = ['small','large']
                    grp_list = {key[0]:[], key[1]:[]}
                    for idx in (0,1):
                        table = merge_table[idx]
                        for item in table:
                            for id in item:
                                timegrp[idx][id_list[id]] = timegrp_id[idx]
                            grp_list[key[idx]].append(timegrp_id[idx])
                            timegrp_id[idx] = timegrp_id[idx] + 1
                        gap = merge_gap[idx]
                        gap_id = []
                        for v in gap:
                            gap_id.append(id_list[v])
                        timegap[idx][ant][spw][pol] = gap_id
                    timegrp_list[ant][spw][pol] = grp_list
                    ###

                    pattern_dict[spw][pol] = pattern

            # register observing pattern to domain object
            self[ant].pattern = pattern_dict

        # update datatable
        datatable.putcol('POSGRP', posgrp)
        datatable.putcol('TIMEGRP_S', timegrp[0])
        datatable.putcol('TIMEGRP_L', timegrp[1])
        datatable.putkeyword('POSGRP_REP', posgrp_rep)
        datatable.putkeyword('POSGRP_LIST', posgrp_list)
        datatable.putkeyword('TIMEGRP_LIST', timegrp_list)
        datatable.putkeyword('TIMEGAP_S', timegap[0])
        datatable.putkeyword('TIMEGAP_L', timegap[1])
        datatable.exportdata()

        LOG.todo('implement rowbase selection for grouping')
        

    def configure_grid(self, datatable):
        qa = casatools.quanta

        ra = datatable.getcol('RA')
        dec = datatable.getcol('DEC')
        h = heuristics.GenerateGrid()
        self.grid_position.clear()
        last_ra = None
        last_dec = None
        grid = None
        for (ant,vant) in self.by_antenna.items():
            grid_position = {}
            st = self[ant]
            source = st.source
            spectral_window = st.spectral_window
            for (src,vsrc) in source.items():
                grid_position[src] = {}
                ref_dir = vsrc._direction
                for (spw,vspw) in self.by_spw.items():
                    beam_size = st.beam_size[spw]
                    radius = qa.mul(beam_size,0.5)
                    r_allowance = qa.mul(radius, 0.1)
                    grid_position[src][spw] = {}
                    for (pol,vpol) in self.by_pol.items():
                        id_list = list(vant & vspw & vpol)
                        id_list.sort()
                        ra_sel = numpy.take(ra, id_list)
                        dec_sel = numpy.take(dec, id_list)
                        if grid is None or \
                           len(ra_sel) != len(last_ra) or \
                           len(dec_sel) != len(dec_sel) or \
                           not all(ra_sel == last_ra) or \
                           not all(dec_sel == last_dec):
                            grid = h.calculate(st.pattern[spw][pol],
                                               ref_dir,
                                               radius,
                                               r_allowance,
                                               ra_sel,
                                               dec_sel)
                            last_ra = ra_sel
                            last_dec = dec_sel
                            LOG.info('nx=%s\nny=%s\ngrid=%s'%(len(grid[0]),len(grid[1]),grid))
                        grid_position[src][spw][pol] = grid
            self.grid_position[ant] = grid_position

    

class SingleDishBase(object):
    def __repr__(self):
        s = '%s:\n'%(self.__class__.__name__)
        for (k,v) in self.__dict__.items():
            if k[0] == '_':
                key = k[1:]
            else:
                key = k
            s += '\t%s=%s\n'%(key,v)
        return s

    def _init_properties(self, properties={}, kw_ignore=['self']):
        for (k,v) in properties.items():
            if k not in kw_ignore:
                setattr(self, k, v)

class ScantableRep(SingleDishBase):
    tolerance = 1.0e-3

    def __init__(self, name, ms=None, session=None):
        self.basename = os.path.basename(name.rstrip('/'))
        self.name = name
        self.ms = ms
        self.ms_name = ms.basename if ms is not None else None
        self.session = session

        # observation property
        self.observer = None
        self.start_time = None
        self.end_time = None

        # polarization property
        self.polarization = {}

        # spectral window property
        self.spectral_window = {}

        # calibration property
        self.tsys_transfer = None
        self.tsys_transfer_list = []

        # source property
        self.source = {}

        # antenna property
        self.antenna = None
        

class Polarization(SingleDishBase):
    polarization_map = { 'linear': { 0: ['XX',  9],
                                     1: ['YY', 12],
                                     2: ['XY', 10],
                                     3: ['YX', 11] },
                         'circular': { 0: ['RR', 5],
                                       1: ['LL', 8],
                                       2: ['RL', 6],
                                       3: ['LR', 7] },
                         'stokes': { 0: ['I', 1],
                                     1: ['Q', 2],
                                     2: ['U', 3],
                                     3: ['V', 4] },
                         'linpol': { 0: ['Ptotal',   28],
                                     1: ['Plinear',  29],
                                     2: ['PFtotal',  30],
                                     3: ['PFlinear', 31],
                                     4: ['Pangle',   32] } }

    @staticmethod
    def from_data_desc(datadesc):
        npol = datadesc.num_polarizations
        polno = numpy.arange(npol, dtype=int)
        corrs = datadesc.polarizations
        if 'X' in corrs:
            ptype = 'linear'
        elif 'R' in corrs:
            ptype = 'circular'
        else:
            ptype = 'stokes'
        corrstr = ['%s%s'%(corrs[i],corrs[i]) for i in xrange(min(2,npol))]
        if npol > 2:
            corrstr = corrstr + ['%s%s'%(corrs[i],corrs[j]) for (i,j) in [(0,1),(1,0)]]
        polmap = Polarization.polarization_map[ptype]
        entry = Polarization(type=ptype,
                             polno=polno,
                             spw_association=[datadesc.spw.id],
                             corr_string=corrstr,
                             corr_index=[polmap[i][0] for i in xrange(npol)])
        return entry
        

    def __init__(self, type=None, polno=None, corr_index=None, corr_string=None, spw_association=[]):
        self._init_properties(vars())

        
class Frequencies(spectralwindow.SpectralWindow,SingleDishBase):

    frame_map = { 0: 'REST',
                  1: 'LSRK',
                  2: 'LSRD',
                  3: 'BARY',
                  4: 'GEO',
                  5: 'TOPO',
                  6: 'GALACTO',
                  7: 'LGROUP',
                  8: 'CMB' }
    
    @staticmethod
    def from_spectral_window(spw):
        nchan = spw.num_channels
        spw_type = ('TP' if nchan == 1 else \
                    ('WVR' if nchan == 4 else 'SP'))
        channel0 = spw.channels[0]
        refpix = 0
        refval = to_numeric_freq(channel0.getCentreFrequency())
        increment = to_numeric_freq(channel0.getWidth())
        entry = Frequencies(id=spw.id,
                            type=spw_type,
                            nchan=nchan,
                            bandwidth=float(to_numeric_freq(spw.bandwidth)),
                            refpix=refpix,
                            refval=refval,
                            increment=increment,
                            freq_min=to_numeric_freq(spw.min_frequency),
                            freq_max=to_numeric_freq(spw.max_frequency))
        return entry
        
    def __init__(self, id=None, type=None, frame=None, nchan=None, refpix=None, refval=None, increment=None, bandwidth=None, intent=None, freq_min=None, freq_max=None, pol_association=None):
        if increment is not None and nchan is not None:
            chan_widths = [[increment]] * nchan
        else:
            chan_widths = None
        if refpix is not None and refval is not None:
            chan_freqs = [[refval + refpix * increment * ichan] for ichan in xrange(nchan)]
        else:
            chan_freqs = None
        super(Frequencies,self).__init__(id, bandwidth, freq_min, chan_widths, chan_freqs)
        self._init_properties(vars(),kw_ignore=['self','bandwidth'])
        intents = self.intent.split(':')
        for intent in intents:
            if self.type == 'WVR':
                self.intents.add(self.type)
            else:
                self.intents.add(intent)

        # dummy group
        self.group = frequencygroup.FrequencyGroup(0,'default')

    @property
    def frequency_range(self):
        return [self.freq_min, self.freq_max]

    @property
    def intent(self):
        return self._intent

    @intent.setter
    def intent(self, value):
        self._intent = '' if value is None else value

    @property
    def pol_association(self):
        return self._pol_association

    @pol_association.setter
    def pol_association(self, value):
        self._pol_association = [] if value is None else value

    @property
    def is_target(self):
        #return (self.type == 'SP' and self.intent.find('TARGET') != -1)
        return (self.intent.find('TARGET') != -1)

    @property
    def is_atmcal(self):
        return (self.type == 'SP' and self.intent.find('ATMOSPHERE') != -1)
