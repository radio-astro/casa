from __future__ import absolute_import

import numpy
import os
import re

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.domain.singledish as singledish
from ... import heuristics
from . import reader

LOG = infrastructure.get_logger(__name__)

class SDMSInspection(object):
    def __init__(self, table_name, mses=None):
        self.mses = [] if mses is None else mses
        self.table_name = table_name
            
    def execute(self, dry_run=True):
        if dry_run:
            return None
        
        # per ms inspection: beam size and calibration strategy
        for ms in self.mses:
            self._inspect_beam_size(ms)
            self._inspect_calibration_strategy(ms)
            
        # MS-wide inspection: reduction group
        reduction_group = self._inspect_reduction_group()

        # generate MS-based DataTable
        datatable = self._generate_datatable(self.table_name)
        datatable.exportdata(minimal=False)
        
        # MS-wide inspection: data grouping
        grouping_result = self._group_data(datatable)
        
        # merge grouping result with MS-based DataTable
        datatable.putcol('POSGRP', grouping_result['POSGRP'])
        datatable.putkeyword('POSGRP_REP', grouping_result['POSGRP_REP'])
        datatable.putkeyword('POSGRP_LIST', grouping_result['POSGRP_LIST'])
        time_gap = grouping_result['TIMEGAP']
        time_group = grouping_result['TIMEGRP']
        time_group_list = grouping_result['TIMEGRP_LIST']
        datatable.putkeyword('TIMEGAP_S', time_gap[0])
        datatable.putkeyword('TIMEGAP_L', time_gap[1])
        for (group_id, member_list) in reduction_group.items():
            for member in member_list:
                ant = member.antenna
                spw = member.spw
                LOG.info('Adding time table for Reduction Group %s (antenna %s spw %s)'%(group_id,ant,spw))
                datatable.set_timetable(ant, spw, None, time_group_list[ant][spw], numpy.array(time_group[0]), numpy.array(time_group[1]))
        datatable.exportdata(minimal=False) 
        
        return reduction_group       

    def _inspect_reduction_group(self):
        reduction_group = {}
        group_spw_names = {}
        #antenna_index= 0
        for ms in self.mses:
            science_windows = ms.get_spectral_windows(science_windows_only=True)
            for spw in science_windows:
                name = spw.name
                nchan = spw.num_channels
                min_frequency = float(spw._min_frequency.value)
                max_frequency = float(spw._max_frequency.value)
                if len(name) > 0:
                    # grouping by name
                    match = self.__find_match_by_name(name, group_spw_names)
                else:
                    # grouping by frequency range
                    match = self.__find_match_by_coverage(nchan, min_frequency, max_frequency, 
                                                          reduction_group, fraction=0.99)
                if match == False:
                    # add new group
                    key = len(reduction_group)
                    group_spw_names[key] = name
                    newgroup = singledish.MSReductionGroupDesc(min_frequency=min_frequency, 
                                                               max_frequency=max_frequency, 
                                                               nchan=nchan)
                    reduction_group[key] = newgroup
                else:
                    key = match
                for antenna in ms.antennas:
                    reduction_group[key].add_member(ms, antenna.id, spw.id)
            #antenna_index += len(ms.antennas)
        
        return reduction_group
    
    def _generate_datatable(self, table_name):
        # create DataTableReader instance
        worker = reader.MetaDataReader(mses=self.mses, table_name=table_name)
        LOG.debug('table_name=%s'%(table_name))

        for ms in self.mses:
            dry_run = not os.path.exists(ms.name)
            worker.set_name(ms.name)
            worker.execute(dry_run=dry_run)

        return worker.get_datatable()
    
    def __select_data(self, datatable, ms_ant_map):
        by_antenna = {}
        by_spw = {}
        ant = datatable.getcol('ANTENNA')
        spw = datatable.getcol('IF')
        for i in xrange(datatable.nrow):
            thisant = ant[i]
            thisspw = spw[i]
            
            ms = ms_ant_map[thisant]
            
            spw_domain = ms.spectral_windows[thisspw]
            if re.search('^WVR#', spw_domain.name) is not None \
                or re.search('#CH_AVG$', spw_domain.name) is not None \
                or 'TARGET' not in spw_domain.intents:
                continue

            if not by_antenna.has_key(thisant):
                by_antenna[thisant] = set()
            by_antenna[thisant].add(i)

            if not by_spw.has_key(thisspw):
                by_spw[thisspw] = set()
            by_spw[thisspw].add(i)
        
        return by_antenna, by_spw
        
    def _group_data(self, datatable):
        ms_ant_map = {}
        id_ant_map = {}
        ant_offset = 0
        for ms in self.mses:
            nant = len(ms.antennas)
            for a in xrange(nant):
                key = a + ant_offset
                ms_ant_map[key] = ms
                id_ant_map[key] = ms.antennas[a].id
            ant_offset += nant
        
        by_antenna, by_spw = self.__select_data(datatable, ms_ant_map)
        
        qa = casatools.quanta

        pos_heuristic2 = heuristics.GroupByPosition2()
        obs_heuristic2 = heuristics.ObservingPattern2()
        time_heuristic2 = heuristics.GroupByTime2()
        merge_heuristic2 = heuristics.MergeGapTables2()
        ra = numpy.asarray(datatable.getcol('RA'))
        dec = numpy.asarray(datatable.getcol('DEC'))
        row = numpy.asarray(datatable.getcol('ROW'))
        elapsed = numpy.asarray(datatable.getcol('ELAPSED'))
        beam = numpy.asarray(datatable.getcol('BEAM'))
        posgrp = numpy.zeros(len(datatable), dtype=int)
        timegrp = [numpy.zeros(len(datatable), dtype=int) - 1,
                        numpy.zeros(len(datatable), dtype=int) - 1]
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
        observing_pattern = []

        posgrp_id = 0
        timegrp_id = [0,0]

        for (ant,vant) in by_antenna.items():
            pattern_dict = {}
            ms = ms_ant_map[ant]
            observatory = ms.antenna_array.name
            _beam_size = ms.beam_sizes[id_ant_map[ant]]
            for i in (0,1):
                timegap[i][ant] = {}
            posgrp_list[ant] = {}
            timegrp_list[ant] = {}
            for (spw,vspw) in by_spw.items():
                try:
                    spw_domain = ms.get_spectral_window(spw_id=spw)
                except KeyError:
                    continue
                pattern_dict[spw] = None
                # beam radius
                radius = qa.mul(_beam_size[spw],0.5)
                r_combine = radius
                r_allowance = qa.mul(radius, 0.1)
                for i in (0,1):
                    timegap[i][ant][spw] = None
                posgrp_list[ant][spw] = []
                timegrp_list[ant][spw] = None
                
                #for (pol,vpol) in self.by_pol.items():
                id_list = numpy.fromiter(vant & vspw, dtype=numpy.int32)
                if len(id_list) == 0:
                    continue
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
                #posgrp_list[ant][spw][pol] = []
                for (k,v) in pos_dict.items():
                    if v[0] == -1:
                        continue
                    posgrp_rep[posgrp_id] = id_list[v[0]]
                    for id in v:
                        posgrp[id_list[id]] = posgrp_id
                    posgrp_list[ant][spw].append(posgrp_id)
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
                            timegrp[idx][id_list[id]] = timegrp_id[idx]
                        grp_list[key[idx]].append(timegrp_id[idx])
                        timegrp_id[idx] = timegrp_id[idx] + 1
                    gap = merge_gap[idx]
                    gap_id = []
                    for v in gap:
                        gap_id.append(id_list[v])
                    timegap[idx][ant][spw] = gap_id
                timegrp_list[ant][spw] = grp_list
                ###

                pattern_dict[spw] = pattern

            # register observing pattern to domain object
            #self[ant].pattern = pattern_dict
            observing_pattern.append(pattern_dict)
            
        grouping_result= {}
        grouping_result['POSGRP'] = posgrp
        grouping_result['POSGRP_REP'] = posgrp_rep
        grouping_result['POSGRP_LIST']= posgrp_list
        grouping_result['TIMEGRP_LIST'] = timegrp_list
        grouping_result['TIMEGRP'] = timegrp
        grouping_result['TIMEGAP'] = timegap
        grouping_result['OBSERVING_PATTERN'] = observing_pattern
        
        return grouping_result
    def _inspect_calibration_strategy(self, ms):
        tsys_transfer = []
        calibration_type_heuristic = heuristics.CalibrationTypeHeuristics()
        spwmap_heuristic = heuristics.TsysSpwMapHeuristics()
        calibration_type = calibration_type_heuristic(ms.name)
        science_windows = ms.get_spectral_windows(science_windows_only=True)
        tsys_windows = [spw for spw in ms.spectral_windows \
                        if 'ATMOSPHERE' in spw.intents and spw.name.find('CH_AVG') == -1]
        TOL = singledish.ScantableRep.tolerance
        for spwa in tsys_windows:
            fmina = float(spwa._min_frequency.value)
            fmaxa = float(spwa._max_frequency.value)
            for spwt in science_windows:
                if spwa == spwt:
                    # identical spw, skip (not necessary to transfer Tsys)
                    continue
                elif spwa.baseband != spwt.baseband:
                    # different baseband, skip
                    continue
                else:
                    fmint = float(spwt._min_frequency.value)
                    fmaxt = float(spwt._max_frequency.value)
                    dfmin = (fmint - fmina) / fmina
                    dfmax = (fmaxt - fmaxa) / fmaxa
                    LOG.trace('(fmina,fmaxa) = (%s, %s)'%(fmina, fmaxa))
                    LOG.trace('(fmint,fmaxt) = (%s, %s)'%(fmint, fmaxt))
                    LOG.trace('dfmin = %s, dfmax=%s, TOL = %s'%(dfmin, dfmax, TOL))
                    if dfmin >= -TOL and dfmax <= TOL:
                        tsys_transfer.append([spwa.id, spwt.id])
        do_tsys_transfer = len(tsys_transfer) > 0
        spwmap = spwmap_heuristic(ms, tsys_transfer)
        calibration_strategy = {'tsys': do_tsys_transfer,
                                'tsys_strategy': spwmap,
                                'calmode': calibration_type}
        ms.calibration_strategy = calibration_strategy
        
    def _inspect_beam_size(self, ms):
        beam_size_heuristic = heuristics.SingleDishBeamSize()
        beam_sizes = {}
        for antenna in ms.antennas:
            diameter = antenna.diameter 
            antenna_id = antenna.id
            beam_size_for_antenna = {}
            for spw in ms.spectral_windows:
                spw_id = spw.id
                center_frequency = float(spw.centre_frequency.convert_to(measures.FrequencyUnits.GIGAHERTZ).value)
                beam_size = beam_size_heuristic(diameter=diameter, frequency=center_frequency)
                beam_size_quantity = casatools.quanta.quantity(beam_size, 'arcsec')
                beam_size_for_antenna[spw_id] = beam_size_quantity
            beam_sizes[antenna_id] = beam_size_for_antenna
        ms.beam_sizes = beam_sizes
        

    
    
    def __find_match_by_name(self, spw_name, spw_names):
        match = False
        for (group_key,group_spw_name) in spw_names.items():
            if (group_spw_name==''): 
                raise RuntimeError, "Got empty group spectral window name"
            elif spw_name == group_spw_name:
                match = group_key
                break
        return match

    def __find_match_by_coverage(self, nchan, min_frequency, max_frequency, reduction_group, fraction=0.99):
        if fraction<=0 or fraction>1.0:
            raise ValueError, "overlap fraction should be between 0.0 and 1.0"
        LOG.warn("Creating reduction group by frequency overlap. This may be not proper if observation dates extend over long period.")
        match = False
        for (group_key,group_desc) in reduction_group.items():
            group_range = group_desc.frequency_range
            group_nchan = group_desc.nchan
            overlap = max( 0.0, min(group_range[1], max_frequency)
                           - max(group_range[0], min_frequency))
            width = max(group_range[1], max_frequency) - min(group_range[0], min_frequency)
            coverage = overlap/width
            if nchan == group_nchan and coverage >= fraction:
                match = group_key
                break
        return match    