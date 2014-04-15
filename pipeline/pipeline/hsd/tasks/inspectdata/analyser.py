from __future__ import absolute_import

import re
import numpy

from asap import srctype as st

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.singledish as singledish
from ... import heuristics

LOG = infrastructure.get_logger(__name__)

class DataTableAnalyser(object):
    def __init__(self, scantablelist, datatable):
        self.scantablelist = scantablelist
        self.datatable = datatable

    def execute(self, dry_run=True):
        if dry_run:
            return
        
        self.analyse_reduction_group()
        self.analyse_calibration()
        self.analyse_beam_size()
        self.select_data()
        self.group_data()
        self.analyse_grid()
        
    def analyse_reduction_group(self):
        # reset reduction_group
        self.reduction_group = {}
        
        for idx in xrange(len(self.scantablelist)):
            entry = self.scantablelist[idx]
            spw_list = entry.spectral_window

            # filter out non-target spws
            target_spw = []
            for (k,v) in spw_list.items():
                if v.type == 'WVR' or v.nchan == 1:
                    continue
                if re.match('TARGET',v.intent):
                    target_spw.append(k)

            # update reduction_group 
            for spw in target_spw:
                spw_property = entry.spectral_window[spw]
                pol_properties = entry.polarization
                freq_range = spw_property.frequency_range
                nchan = spw_property.nchan
                pols = pol_properties[spw_property.pol_association[0]].polno
                match = False
                for (k,v) in self.reduction_group.items():
                    group_range = v.frequency_range#v['freq_range']
                    group_nchan = v.nchan#v['nchan']
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
                    newgroup = singledish.ReductionGroupDesc(freq_range, nchan)
                    newgroup.add_member(idx, spw, pols)
                    self.reduction_group[key] = newgroup
                else:
                    self.reduction_group[match].add_member(idx, spw, pols)

        LOG.debug('reduction_group:\n%s'%(self.reduction_group))


    def analyse_calibration(self):
        self.calibration_strategy = []
        for item in self.scantablelist:
            # strategy for tsys transfer
            if item.tsys_transfer:
                tsys_strategy = item.tsys_transfer_list
            else:
                tsys_strategy = None

            # strategy for off-position calibration
            h_calmode = heuristics.CalibrationTypeHeuristics()
            calmode = h_calmode(item.name)
            h_srctype = heuristics.SrcTypeHeuristics()
            srctype = h_srctype(calmode)
            
            entry = {
                'tsys': item.tsys_transfer,
                'tsys_strategy': tsys_strategy,
                'calmode': calmode,
                'srctype': srctype
                }

            LOG.info('calibration strategy for %s:\n%s'%(item.basename,entry))

            # register strategy to domain object
            #item.calibration_strategy = entry
            self.calibration_strategy.append(entry)

        LOG.todo('implement strategy for elevation correction')
        LOG.todo('implement strategy for flux calibration')


    def analyse_beam_size(self):
        self.beam_size = []
        qa = casatools.quanta

        h = heuristics.SingleDishBeamSizeFromName()
        for item in self.scantablelist:
            antenna = item.antenna.name
            entry = {}
            for (spw,v) in item.spectral_window.items():
                center_freq = 0.5 * sum(v.frequency_range) * 1.0e-9 #GHz
                _beam_size = qa.quantity(h(antenna, center_freq), 'arcsec')
                entry[spw] = _beam_size

            # register beam size to domain object
            #item.beam_size = entry
            self.beam_size.append(entry)
            
        
    def select_data(self):
        qa = casatools.quanta

        # data selection
        self.by_antenna = {}
        self.by_spw = {}
        self.by_pol = {}
        on_source = [st.pson, st.nod, st.fson]
        ant = self.datatable.getcol('ANTENNA')
        srt = self.datatable.getcol('SRCTYPE')
        spw = self.datatable.getcol('IF')
        pol = self.datatable.getcol('POL')
        for i in xrange(self.datatable.nrow):
            thisant = ant[i]
            thisspw = spw[i]
            thispol = pol[i]
            spw_prop = self.scantablelist[thisant].spectral_window[thisspw]
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

    def group_data(self):
        qa = casatools.quanta

        pos_heuristic2 = heuristics.GroupByPosition2()
        obs_heuristic2 = heuristics.ObservingPattern2()
        time_heuristic2 = heuristics.GroupByTime2()
        merge_heuristic2 = heuristics.MergeGapTables2()
        ra = self.datatable.getcol('RA')
        dec = self.datatable.getcol('DEC')
        row = self.datatable.getcol('ROW')
        elapsed = self.datatable.getcol('ELAPSED')
        beam = self.datatable.getcol('BEAM')
        self.posgrp = numpy.zeros(len(self.datatable), dtype=int)
        self.timegrp = [numpy.zeros(len(self.datatable), dtype=int) - 1,
                        numpy.zeros(len(self.datatable), dtype=int) - 1]
        self.posgrp_rep = {}
        self.posgrp_list = {}
        self.timegrp_list = {}
        self.timegap = [{},{}]
        last_ra = None
        last_dec = None
        last_time = None
        pos_dict = None
        pos_gap = None
        time_table = None
        time_gap = None
        merge_table = None
        merge_gap = None
        #self.pattern = {}
        self.observing_pattern = []

        posgrp_id = 0
        timegrp_id = [0,0]

        for (ant,vant) in self.by_antenna.items():
            pattern_dict = {}
            st = self.scantablelist[ant]
            observatory = st.ms.antenna_array.name
            _beam_size = self.beam_size[ant]
            for i in (0,1):
                self.timegap[i][ant] = {}
            self.posgrp_list[ant] = {}
            self.timegrp_list[ant] = {}
            for (spw,vspw) in self.by_spw.items():
                spectral_window = st.spectral_window[spw]
                pattern_dict[spw] = {}
                # beam radius
                radius = qa.mul(_beam_size[spw],0.5)
                r_combine = radius
                r_allowance = qa.mul(radius, 0.1)
                for i in (0,1):
                    self.timegap[i][ant][spw] = {}
                self.posgrp_list[ant][spw] = {}
                self.timegrp_list[ant][spw] = {}
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
                        # 2014/02/04 TN
                        # Temporary workaround for TP acceptance data issue
                        # Observing pattern is always 'RASTER' for ALMA
                        if observatory == 'ALMA':
                            pattern = 'RASTER'
                        else:
                            pattern = obs_heuristic2(pos_dict)
                    
                    ### prepare for Self.Datatable ###
                    self.posgrp_list[ant][spw][pol] = []
                    for (k,v) in pos_dict.items():
                        if v[0] == -1:
                            continue
                        self.posgrp_rep[posgrp_id] = id_list[v[0]]
                        for id in v:
                            self.posgrp[id_list[id]] = posgrp_id
                        self.posgrp_list[ant][spw][pol].append(posgrp_id)
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

                    ### prepare for Self.Datatable ###
                    key = ['small','large']
                    grp_list = {key[0]:[], key[1]:[]}
                    for idx in (0,1):
                        table = merge_table[idx]
                        for item in table:
                            for id in item:
                                self.timegrp[idx][id_list[id]] = timegrp_id[idx]
                            grp_list[key[idx]].append(timegrp_id[idx])
                            timegrp_id[idx] = timegrp_id[idx] + 1
                        gap = merge_gap[idx]
                        gap_id = []
                        for v in gap:
                            gap_id.append(id_list[v])
                        self.timegap[idx][ant][spw][pol] = gap_id
                    self.timegrp_list[ant][spw][pol] = grp_list
                    ###

                    pattern_dict[spw][pol] = pattern

            # register observing pattern to domain object
            #self[ant].pattern = pattern_dict
            self.observing_pattern.append(pattern_dict)

        # update self.datatable
        #self.datatable.putcol('POSGRP', self.posgrp)
        #self.datatable.putcol('TIMEGRP_S', self.timegrp[0])
        #self.datatable.putcol('TIMEGRP_L', self.timegrp[1])
        #self.datatable.putkeyword('POSGRP_REP', self.posgrp_rep)
        #self.datatable.putkeyword('POSGRP_LIST', self.posgrp_list)
        #self.datatable.putkeyword('TIMEGRP_LIST', self.timegrp_list)
        #self.datatable.putkeyword('TIMEGAP_S', self.timegap[0])
        #self.datatable.putkeyword('TIMEGAP_L', self.timegap[1])
        #self.datatable.exportdata(minimal=False)

        LOG.todo('implement rowbase selection for grouping')
        

    def analyse_grid(self):
        qa = casatools.quanta

        ra = self.datatable.getcol('RA')
        dec = self.datatable.getcol('DEC')
        h = heuristics.GenerateGrid()
        self.grid_position = []
        last_ra = None
        last_dec = None
        grid = None
        for (ant,vant) in self.by_antenna.items():
            _beam_size = self.beam_size[ant]
            grid_position = {}
            st = self.scantablelist[ant]
            source = st.source
            spectral_window = st.spectral_window
            for (src,vsrc) in source.items():
                grid_position[src] = {}
                ref_dir = vsrc._direction
                pattern = self.observing_pattern[ant]
                for (spw,vspw) in self.by_spw.items():
                    #beam_size = st.beam_size[spw]
                    radius = qa.mul(_beam_size[spw],0.5)
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
                            grid = h.calculate(pattern[spw][pol],
                                               ref_dir,
                                               radius,
                                               r_allowance,
                                               ra_sel,
                                               dec_sel)
                            last_ra = ra_sel
                            last_dec = dec_sel
                            LOG.info('(nx,ny)=(%s,%s)'%(len(grid[0]),len(grid[1])))
                            LOG.debug('grid=%s'%(grid))
                        grid_position[src][spw][pol] = grid
            self.grid_position.append(grid_position)

    
