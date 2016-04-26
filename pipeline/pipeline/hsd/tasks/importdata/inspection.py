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
    def __init__(self, table_name, ms=None):
        self.ms = ms
        self.table_name = table_name
            
    def execute(self, dry_run=True):
        if dry_run:
            return None
        
        # per ms inspection: beam size and calibration strategy
        LOG.debug('inspect_beam_size')
        self._inspect_beam_size()
        LOG.debug('inspect_calibration_strategy')
        self._inspect_calibration_strategy()
            
        # per ms inspection: reduction group
        LOG.debug('inspect_reduction_group')
        reduction_group = self._inspect_reduction_group()

        # generate MS-based DataTable
        LOG.debug('register meta data to DataTable')
        table_name = self.table_name
        worker = reader.MetaDataReader(ms=self.ms, table_name=table_name)
        LOG.debug('table_name=%s'%(table_name))
        
        dry_run = not os.path.exists(self.ms.name)
        #worker.set_name(ms.name)
        worker.execute(dry_run=dry_run)

        datatable = worker.get_datatable()
        datatable.exportdata(minimal=False)
        
        appended_row = worker.appended_row
        nrow = datatable.nrow
        startrow = nrow - appended_row
        LOG.debug('%s rows are appended (total %s, startrow %s)'%(appended_row, nrow, startrow))
    
        # MS-wide inspection: data grouping
        LOG.debug('_group_data: ms = %s'%(self.ms.basename))
        position_group_id = datatable.position_group_id
        time_group_id_small = datatable.time_group_id_small
        time_group_id_large = datatable.time_group_id_large
        grouping_result = self._group_data(datatable, position_group_id, 
                                           time_group_id_small, time_group_id_large,
                                           startrow=startrow, nrow=appended_row)
    
        # merge grouping result with MS-based DataTable
        position_group = grouping_result['POSGRP']
        LOG.debug('len(position_group) = %s appended_row = %s'%(len(position_group), appended_row))
        LOG.debug('position_group = %s'%(position_group))
        datatable.putcol('POSGRP', position_group[startrow:], startrow=startrow, nrow=appended_row)
    
        time_gap = grouping_result['TIMEGAP']
        def _g():
            yield 'POSGRP_REP', None
            yield 'POSGRP_LIST', None
            yield 'TIMEGAP_S', time_gap[0]
            yield 'TIMEGAP_L', time_gap[1]
        for key, value in _g():
            LOG.debug('key, value = %s, %s'%(key, value))
            if value is None:
                value = grouping_result[key]
                LOG.debug('updated value = %s'%(value))
            if datatable.has_key(key):
                LOG.debug('Updating %s'%(key))
                LOG.debug('before: %s'%(datatable.getkeyword(key)))
                current_value = datatable.getkeyword(key)
                current_value.update(value)
                datatable.tb2.putkeyword(key, current_value)
            else:
                LOG.debug('Adding %s'%(key))
                datatable.tb2.putkeyword(key, value)
            LOG.debug('after: %s'%(datatable.getkeyword(key)))
        #datatable.putkeyword('POSGRP_LIST', grouping_result['POSGRP_LIST'])
        time_group = grouping_result['TIMEGRP']
        time_group_list = grouping_result['TIMEGRP_LIST']
        #datatable.putkeyword('TIMEGAP_S', time_gap[0])
        #datatable.putkeyword('TIMEGAP_L', time_gap[1])
        for (group_id, member_list) in reduction_group.items():
            for member in member_list:
                ms = member.ms
                ant = member.antenna
                spw = member.spw
                field_id = member.field_id
                LOG.info('Adding time table for Reduction Group %s (ms %s antenna %s spw %s field_id %s)'%(group_id,ms.basename,ant,spw,field_id))
                datatable.set_timetable(ant, spw, None, time_group_list[ant][spw][field_id], 
                                        numpy.array(time_group[0]), numpy.array(time_group[1]),
                                        ms=ms.basename, field_id=field_id)
        datatable.exportdata(minimal=False) 
        
        return reduction_group       

    def _inspect_reduction_group(self):
        reduction_group = {}
        group_spw_names = {}
        ms = self.ms
        science_windows = ms.get_spectral_windows(science_windows_only=True)
        assert hasattr(ms, 'calibration_strategy')
        field_strategy = ms.calibration_strategy['field_strategy']
        for field_id in field_strategy.keys():
            fields = ms.get_fields(field_id)
            assert len(fields) == 1
            field = fields[0]
            field_name = field.name
            for spw in science_windows:
                spw_name = spw.name
                nchan = spw.num_channels
                min_frequency = float(spw._min_frequency.value)
                max_frequency = float(spw._max_frequency.value)
                if len(spw_name) > 0:
                    # grouping by name
                    match = self.__find_match_by_name(spw_name, field_name, group_spw_names)
                else:
                    # grouping by frequency range
                    match = self.__find_match_by_coverage(nchan, min_frequency, max_frequency, 
                                                          reduction_group, fraction=0.99, field_name=field_name)
                if match == False:
                    # add new group
                    key = len(reduction_group)
                    group_spw_names[key] = (spw_name, field_name)
                    newgroup = singledish.MSReductionGroupDesc(min_frequency=min_frequency, 
                                                               max_frequency=max_frequency, 
                                                               nchan=nchan,
                                                               field=field)
                    reduction_group[key] = newgroup
                else:
                    key = match
                for antenna in ms.antennas:
                    reduction_group[key].add_member(ms, antenna.id, spw.id, field_id)
        
        return reduction_group
    
    def __select_data(self, datatable, ms_ant_map, startrow=0, nrow=-1):
        ms_name = self.ms.name
        filenames = datatable.getkeyword('FILENAMES')
        assert ms_name in filenames
        ms_index = numpy.argwhere(filenames == ms_name)[0][0]
        
        ms_id = datatable.getcol('MS', startrow=startrow, nrow=nrow)
        assert numpy.all(ms_id == ms_index)
        
        by_antenna = {}
        by_spw = {}
        by_field = {}
        ant = datatable.getcol('ANTENNA', startrow=startrow, nrow=nrow)
        spw = datatable.getcol('IF', startrow=startrow, nrow=nrow)
        field_id = datatable.getcol('FIELD_ID', startrow=startrow, nrow=nrow)
        srctype = datatable.getcol('SRCTYPE', startrow=startrow, nrow=nrow)
        LOG.trace('ant=%s'%(ant))
        LOG.trace('spw=%s'%(spw))
        if nrow < 0:
            nrow = datatable.nrow - startrow
        LOG.debug('nrow = %s'%(nrow))
        for i in xrange(nrow):
            if srctype[i] != 0:
                continue
            
            thisant = ant[i]
            thisspw = spw[i]
            thisfield = field_id[i]
            
            spw_domain = self.ms.spectral_windows[thisspw]
            #LOG.debug('spw.name=\'%s\''%(spw_domain.name))
            #LOG.debug('spw.intents=%s'%(spw_domain.intents))
            if re.search('^WVR#', spw_domain.name) is not None \
                or re.search('#CH_AVG$', spw_domain.name) is not None \
                or 'TARGET' not in spw_domain.intents:
                continue

            if not by_antenna.has_key(thisant):
                by_antenna[thisant] = set()
            by_antenna[thisant].add(i + startrow)

            if not by_spw.has_key(thisspw):
                by_spw[thisspw] = set()
            by_spw[thisspw].add(i + startrow)
            
            if not by_field.has_key(thisfield):
                by_field[thisfield] = set()
            by_field[thisfield].add(i + startrow)
        
        return by_antenna, by_spw, by_field
        
    def _group_data(self, datatable, position_group_id, time_group_id_small, time_group_id_large, startrow=0, nrow=-1):
        ms_ant_map = {}
        id_ant_map = {}
        ant_offset = 0
        ms = self.ms
        nant = len(ms.antennas)
        for a in xrange(nant):
            key = a + ant_offset
            ms_ant_map[key] = ms
            id_ant_map[key] = ms.antennas[a].id
        ant_offset += nant
        
        if nrow < 0:
            nrow = datatable.nrow - startrow
        by_antenna, by_spw, by_field = self.__select_data(datatable, ms_ant_map, startrow=startrow, nrow=nrow)
        LOG.trace('by_antenna=%s'%(by_antenna))
        LOG.trace('by_spw=%s'%(by_spw))
        LOG.trace('len(by_antenna)=%s len(by_spw)=%s'%(len(by_antenna),len(by_spw)))
        
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
        posgrp = numpy.zeros(datatable.nrow, dtype=numpy.int32) - 1
        timegrp = [numpy.zeros(datatable.nrow, dtype=numpy.int32) - 1,
                        numpy.zeros(datatable.nrow, dtype=numpy.int32) - 1]
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
        observing_pattern = {}

        posgrp_id = position_group_id
        LOG.debug('POSGRP: starting ID is %s'%(posgrp_id))
        timegrp_id = [time_group_id_small, time_group_id_large]
        LOG.debug('TIMEGRP: starting ID is %s'%(timegrp_id))

        ms = self.ms
        for (ant,vant) in by_antenna.items():
            LOG.debug('Start ant %s'%(ant))
            pattern_dict = {}
            #ms = ms_ant_map[ant]
            observatory = ms.antenna_array.name
            _beam_size = ms.beam_sizes[id_ant_map[ant]]
            for i in (0,1):
                timegap[i][ant] = {}
            posgrp_list[ant] = {}
            timegrp_list[ant] = {}
            for (spw,vspw) in by_spw.items():
                LOG.debug('Start spw %s'%(spw))
                try:
                    spw_domain = ms.get_spectral_window(spw_id=spw)
                except KeyError:
                    continue
                pattern_dict[spw] = {}
                posgrp_list[ant][spw] = {}
                timegrp_list[ant][spw] = {}
                for i in (0,1):
                    timegap[i][ant][spw] = {}
                # beam radius
                radius = qa.mul(_beam_size[spw],0.5)
                r_combine = radius
                r_allowance = qa.mul(radius, 0.1)
                
                for (field_id,vfield) in by_field.items():
                    pattern_dict[spw][field_id] = None
                    for i in (0,1):
                        timegap[i][ant][spw][field_id] = None
                    posgrp_list[ant][spw][field_id] = []
                    timegrp_list[ant][spw][field_id] = None
                    
                    #for (pol,vpol) in self.by_pol.items():
                    id_list = numpy.fromiter(vant & vspw & vfield, dtype=numpy.int32)
                    if len(id_list) == 0:
                        continue
                    id_list.sort()
                    LOG.debug('id_list=%s'%(id_list))
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
                    LOG.debug('pos_dict = %s'%(pos_dict))
                    LOG.debug('last_ra = %s last_dec = %s'%(last_ra, last_dec))
                    for (k,v) in pos_dict.items():
                        if v[0] == -1:
                            continue
                        LOG.debug('POSGRP_REP: add %s as a representative of group %s'%(id_list[v[0]], posgrp_id))
                        posgrp_rep[int(posgrp_id)] = int(id_list[v[0]])
                        for id in v:
                            _id = id_list[id]
                            posgrp[_id] = posgrp_id
                        posgrp_list[ant][spw][field_id].append(posgrp_id)
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
                        timegap[idx][ant][spw][field_id] = gap_id
                    timegrp_list[ant][spw][field_id] = grp_list
                    ###
    
                    pattern_dict[spw][field_id] = pattern

            # register observing pattern to domain object
            #self[ant].pattern = pattern_dict
            observing_pattern[ant] = pattern_dict
            
        grouping_result= {}
        grouping_result['POSGRP'] = posgrp
        grouping_result['POSGRP_REP'] = posgrp_rep
        grouping_result['POSGRP_LIST']= posgrp_list
        grouping_result['TIMEGRP_LIST'] = timegrp_list
        grouping_result['TIMEGRP'] = timegrp
        grouping_result['TIMEGAP'] = timegap
        #grouping_result['OBSERVING_PATTERN'] = observing_pattern
        
        ms.observing_pattern = observing_pattern
        
        return grouping_result
    
    def _inspect_calibration_strategy(self):
        ms = self.ms
        tsys_transfer = []
        calibration_type_heuristic = heuristics.CalibrationTypeHeuristics()
        spwmap_heuristic = heuristics.TsysSpwMapHeuristics()
        calibration_type = calibration_type_heuristic(ms.name)
        science_windows = ms.get_spectral_windows(science_windows_only=True)
        tsys_windows = [spw for spw in ms.spectral_windows \
                        if 'ATMOSPHERE' in spw.intents and re.search('(CH_AVG|SQLD)', spw.name) is None]
        LOG.debug('tsys_windows={spws}'.format(spws=[spw.id for spw in tsys_windows]))
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
        
        # field mapping (for multi-source EB)
        # {target field: reference field}
        target_fields = ms.get_fields(intent='TARGET')
        reference_fields = ms.get_fields(intent='REFERENCE')
        field_map = {}
        for target in target_fields:
            target_name = target.name
            LOG.debug('target name: \'%s\''%(target_name))
            for reference in reference_fields:
                reference_name = reference.name
                LOG.debug('reference name: \'%s\''%(reference_name))
                tpattern = '^%s_[0-9]$'%(target_name)
                rpattern = '^%s_[0-9]$'%(reference_name)
                if target_name == reference_name:
                    field_map[target.id] = reference.id
                elif re.match(tpattern, reference_name) or re.match(rpattern, target_name):
                    field_map[target.id] = reference.id
        calibration_strategy = {'tsys': do_tsys_transfer,
                                'tsys_strategy': spwmap,
                                'calmode': calibration_type,
                                'field_strategy': field_map}
        ms.calibration_strategy = calibration_strategy
        
    def _inspect_beam_size(self):
        ms = self.ms
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
        

    
    
    def __find_match_by_name(self, spw_name, field_name, group_names):
        match = False
        for (group_key,names) in group_names.items():
            group_spw_name = names[0]
            group_field_name = names[1]
            if (group_spw_name==''): 
                raise RuntimeError, "Got empty group spectral window name"
            elif spw_name == group_spw_name and field_name == group_field_name:
                match = group_key
                break
        return match

    def __find_match_by_coverage(self, nchan, min_frequency, max_frequency, reduction_group, fraction=0.99, field_name=None):
        if fraction<=0 or fraction>1.0:
            raise ValueError, "overlap fraction should be between 0.0 and 1.0"
        LOG.warn("Creating reduction group by frequency overlap. This may not be proper if observation dates extend over long period.")
        match = False
        for (group_key,group_desc) in reduction_group.items():
            group_field_name = group_desc.field
            if field_name is not None and group_field_name != field_name:
                continue 
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