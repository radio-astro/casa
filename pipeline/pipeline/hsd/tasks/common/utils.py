from __future__ import absolute_import 

import sys
import os
import numpy
import contextlib
import re
import time
import collections
import functools

from logging import CRITICAL, ERROR, WARNING, INFO, DEBUG, NOTSET

LogLevelMap = {'critical': 0,
               'error': 0,
               'warning': 1,
               'info': 2,
               'debug': 3,
               'todo': 4,
               'trace': 4}

LogLevelMap2 = {'critical': CRITICAL, # 50
                'error': ERROR,       # 40
                'warning': WARNING,   # 30
                'info': INFO,         # 20
                'debug': DEBUG,       # 10
                'todo': NOTSET,       # 0
                'trace': NOTSET }     # 0

import pipeline.infrastructure.mpihelpers as mpihelpers
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from pipeline.domain.datatable import OnlineFlagIndex
import pipeline.infrastructure.tablereader as tablereader

LOG = infrastructure.get_logger(__name__)

def profiler(func):
    @functools.wraps(func)
    def wrapper(*args, **kw):
        start = time.time()
        result = func(*args, **kw)
        end = time.time()
        
        LOG.info('#PROFILE# %s: elapsed %s sec'%(func.__name__, end - start))
    
        return result
    return wrapper

def asdm_name(scantable_object):
    """
    Return ASDM name that target scantable belongs to.
    Assumptions are:
       - scantable is generated from MS
       - MS is generated from ASDM
       - MS name is <uid>.ms
    """
    return asdm_name_from_ms(scantable_object.ms)

def asdm_name_from_ms(ms_domain):
    """
    Return ASDM name that target ms originates from.
    Assumptions are:
       - MS is generated from ASDM
       - MS name is <uid>.ms
    """
    ms_basename = ms_domain.basename
    index_for_suffix = ms_basename.rfind('.')
    asdm = ms_basename[:index_for_suffix] if index_for_suffix > 0 \
           else ms_basename
    return asdm

def get_parent_ms_idx(context, msname):
    """
    Returns index of corresponding ms in context
    The method maps both work_data and original MS to a proper index
    The return value is -1 if no match found.
    """
    mslist = context.observing_run.measurement_sets
    idx_found = -1
    for idx in xrange(len(mslist)):
        msobj = mslist[idx]
        search_list = [msobj.name, msobj.basename]
        if hasattr(msobj, "work_data"):
            search_list += [msobj.work_data, os.path.basename(msobj.work_data)]
        if msname in search_list:
            idx_found = idx
            break
    return idx_found    

def get_parent_ms_name(context, msname):
    """
    Returns name of corresponding parent ms in context
    The method maps both work_data and original MS to a proper index
    The return value is "" if no match found.
    """
    idx = get_parent_ms_idx(context, msname)
    return context.observing_run.measurement_sets[idx].name if idx >=0 else ""

####
# ProgressTimer
#
# Show the progress bar on the console if LogLevel is lower than or equal to 2.
#
####
class ProgressTimer(object):
    def __init__(self, length=80, maxCount=80, LogLevel='info'):
        """
        Constructor:
            length: length of the progress bar (default 80 characters)
        """
        self.currentLevel = 0
        self.maxCount = maxCount
        self.curCount = 0
        self.scale = float(length)/float(maxCount)
        if isinstance(LogLevel, str):
            self.LogLevel = LogLevelMap2[LogLevel] if LogLevelMap2.has_key(LogLevel) else INFO
        else:
            # should be integer
            self.LogLevel = LogLevel
        if self.LogLevel >= INFO:
            print '\n|' + '='*((length-8)/2) + ' 100% ' + '='*((length-8)/2) + '|'

    def __del__(self):
        if self.LogLevel >= INFO:
            print '\n'

    def count(self, increment=1):
        if self.LogLevel >= INFO:
            self.curCount += increment
            newLevel = int(self.curCount * self.scale)
            if newLevel != self.currentLevel:
                print '\b' + '*' * (newLevel - self.currentLevel),
                sys.stdout.flush()
                self.currentLevel = newLevel

# parse edge parameter to tuple
def parseEdge(edge):
    if isinstance(edge,int) or isinstance(edge,float):
        EdgeL = edge
        EdgeR = edge
    elif len(edge) == 0:
        EdgeL = 0
        EdgeR = 0
    elif len(edge) == 1:
        EdgeL = edge[0]
        EdgeR = edge[0]
    else:
        (EdgeL, EdgeR) = edge[:2]
    return(EdgeL, EdgeR)

def mjd_to_datestring( t, unit='sec' ):
    """
    MJD ---> date string

    t: MJD
    unit: sec or day
    """
    if unit == 'sec':
        mjd = t
    elif unit == 'day':
        mjd = t * 86400.0
    else:
        mjd = 0.0
    import time
    import datetime
    mjdzero=datetime.datetime(1858,11,17,0,0,0)
    zt=time.gmtime(0.0)
    timezero=datetime.datetime(zt.tm_year,zt.tm_mon,zt.tm_mday,zt.tm_hour,zt.tm_min,zt.tm_sec)
    dtd = timezero-mjdzero
    dtsec=mjd-(float(dtd.days)*86400.0+float(dtd.seconds)+float(dtd.microseconds)*1.0e-6)
    mjdstr=time.asctime(time.gmtime(dtsec))+' UTC'
    return mjdstr 


def to_list(s):
    if s is None:
        return None
    elif isinstance(s, list) or isinstance(s, numpy.ndarray):
        return s
    elif isinstance(s, str):
        if s.startswith('['):
            if s.lstrip('[')[0].isdigit():
                return eval(s)
            else:
                # maybe string list
                return eval(s.replace('[','[\'').replace(']','\']').replace(',','\',\''))
        else:
            try:
                return [float(s)]
            except:
                return [s]
    else:
        return [s]

def to_bool(s):
    if s is None:
        return None
    elif isinstance(s, bool):
        return s
    elif isinstance(s, str):
        if s.upper() == 'FALSE' or s == 'F':
            return False
        elif s.upper() == 'TRUE' or s == 'T':
            return True
        else:
            return s
    else:
        return bool(s)

def to_numeric(s):
    if s is None:
        return None
    elif isinstance(s, str):
        try:
            return float(s)
        except:
            return s
    else:
        return s

def get_mask_from_flagtra(flagtra):
    """Convert FLAGTRA (unsigned char) to a mask array (1=valid, 0=flagged)"""
    return (numpy.asarray(flagtra) == 0).astype(int)


@contextlib.contextmanager
def temporary_filename(name='_heuristics.temporary.table', removesubfiles=False):
    try:
        yield name
    finally:
        command = 'rm -rf %s'%(name)
        if removesubfiles:
            command += '*'
        os.system(command)

def _get_index_list(datatable, antenna, spw, pols=None, srctype=None):
    LOG.info('get_index_list_org start')
    start_time = time.time()
    assert len(antenna) == len(spw)
    table1 = datatable.tb1
    table2 = datatable.tb2
    antenna_column = table1.getcol('ANTENNA')
    spw_column = table1.getcol('IF')
    pol_column = table1.getcol('POL')
    srctype_column = table1.getcol('SRCTYPE')
    permanent_flag = table2.getcol('FLAG_PERMANENT')
    # skip if the row is flagged by online flagging
    f = lambda i, j: antenna_column[i] == antenna[j] and spw_column[i] == spw[j] and permanent_flag[OnlineFlagIndex,i] == 1
    if pols is None or len(pols) == 0:
        g = f
    else:
        g = lambda i, j: f(i,j) and (pols[j] is None or pol_column[i] in pols[j])
    if srctype is None:
        #f = lambda i, j: antenna_column[i] == antenna[j] and spw_column[i] == spw[j]
        sel = g
    else:
        #f = lambda i, j: antenna_column[i] == antenna[j] and spw_column[i] == spw[j] and srctype_column[i] == srctype
        sel = lambda i, j: g(i,j) and srctype_column[i] == srctype

    nrow = table1.nrows()
    nval = len(antenna)
    for irow in xrange(nrow):
        for ival in xrange(nval):
            if sel(irow, ival):
                yield irow
    end_time = time.time()
    LOG.info('get_index_list_org end: Elapsed time %s sec'%(end_time - start_time))
    
def _get_index_list2(datatable, antenna, spw, pols):
    assert pols is not None
    online_flag = datatable.tb2.getcolslice('FLAG_PERMANENT', [OnlineFlagIndex], [OnlineFlagIndex], 1)[0]
    for (_ant, _spw, _pols) in zip(antenna, spw, pols):
        for _pol in _pols:
            time_table = datatable.get_timetable(_ant, _spw, _pol)
            # time table separated by large time gap
            the_table = time_table[1]
            for group in the_table:
                for row in group[1]:
                    if online_flag[row] == 1:
                        yield row

def get_index_list(datatable, antenna, spw, pols=None, srctype=None):
    LOG.info('new get_index_list start')
    start_time = time.time()
    
    if pols is None or pols.count(None) > 0 or srctype is None:
        index_list = list(_get_index_list(datatable, antenna, spw, pols, srctype))
    else:
        index_list = list(_get_index_list2(datatable, antenna, spw, pols))
    index_list.sort()
    
    end_time = time.time()
    LOG.info('new get_index_list end: Elapsed time %s sec'%(end_time - start_time))
    
    return index_list

def get_index_list_for_ms(datatable, vis_list, antennaid_list, fieldid_list, 
                          spwid_list, srctype=None):
    return numpy.fromiter(_get_index_list_for_ms(datatable, vis_list, antennaid_list, fieldid_list, 
                                                spwid_list, srctype), dtype=numpy.int64)
    
def _get_index_list_for_ms(datatable, vis_list, antennaid_list, fieldid_list, 
                          spwid_list, srctype=None):
    # use time_table instead of data selection
    #online_flag = datatable.tb2.getcolslice('FLAG_PERMANENT', [0, OnlineFlagIndex], [-1, OnlineFlagIndex], 1)[0]
    #LOG.info('online_flag=%s'%(online_flag))
    for (_vis, _field, _ant, _spw) in zip(vis_list, fieldid_list, antennaid_list, spwid_list):
        time_table = datatable.get_timetable(_ant, _spw, None, os.path.basename(_vis), _field)
        # time table separated by large time gap
        the_table = time_table[1]
        for group in the_table:
            for row in group[1]:
                permanent_flag = datatable.tb2.getcell('FLAG_PERMANENT', row)
                online_flag = permanent_flag[:,OnlineFlagIndex]
                if any(online_flag == 1):
                    yield row    

def get_valid_members(group_desc, antenna_filter, spwid_filter):
    for i in xrange(len(group_desc)):
        member = group_desc[i]
        antenna = member.antenna
        spwid = member.spw
        _spwid_filter = _get_spwid_filter(spwid_filter, antenna)
        LOG.debug('_spwid_filter=%s'%(_spwid_filter))
        if antenna in antenna_filter:
            if _spwid_filter is None or len(_spwid_filter) == 0 or spwid in _spwid_filter:
                yield i
                
def get_valid_ms_members(group_desc, msname_filter, ant_selection, field_selection, spw_selection):
    for member_id in xrange(len(group_desc)):
        member = group_desc[member_id]
        spw_id = member.spw_id
        field_id = member.field_id
        ant_id = member.antenna_id
        msobj = member.ms
        if msobj.name in [os.path.abspath(name) for name in msname_filter]:
            mssel = casatools.ms.msseltoindex(vis=msobj.name, spw=spw_selection,
                                              field=field_selection, baseline=ant_selection)
            spwsel = mssel['spw']
            fieldsel = mssel['field']
            antsel = mssel['antenna1']
            if (len(spwsel) == 0 or spw_id in spwsel) and \
            (len(fieldsel) == 0 or field_id in fieldsel) and \
            (len(antsel) == 0 or ant_id in antsel):
                yield member_id

def _get_spwid_filter(spwid_filter, file_id):
    if spwid_filter is None:
        return None
    elif isinstance(spwid_filter, str):
        return selection_to_list(spwid_filter)
    elif file_id < len(spwid_filter):
        _spwid_filter = spwid_filter[file_id]
        if _spwid_filter is None:
            return None
        else:
            return selection_to_list(spwid_filter[file_id])
    else:
        _spwid_filter = spwid_filter[0]
        if _spwid_filter is None:
            return None
        else:
            return selection_to_list(spwid_filter[0])

def _collect_logrecords(logger):
    capture_handlers = [h for h in logger.handlers if h.__class__.__name__ == 'CapturingHandler']
    logrecords = []
    for handler in capture_handlers:
        logrecords.extend(handler.buffer[:])
    return logrecords

def selection_to_list(sel, maxid=99):
    """Convert idx selction string to a list of ids"""
    def _selection_to_list(sel):
        elements = sel.split(',')
        for _elem in elements:
            elem = _elem.strip()
            if elem.isdigit():
                yield int(elem)
            elif re.match('^[0-9]+~[0-9]+$', elem):
                s = [int(e) for e in elem.split('~')]
                for i in xrange(s[0], s[1]+1):
                    yield i
            elif re.match('^<[0-9]+$', elem):
                for i in xrange(int(elem[1:])):
                    yield i
            elif re.match('^<=[0-9]+$', elem):
                for i in xrange(int(elem[2:])+1):
                    yield i
            elif re.match('^>[0-9]+$', elem):
                for i in xrange(int(elem[1:])+1,maxid):
                    yield i
            elif re.match('^>=[0-9]+$', elem):
                for i in xrange(int(elem[2:]),maxid):
                    yield i
    l = set(_selection_to_list(sel))
    return list(l)

def list_to_selection(rows, dosort=True):
    unique_list = numpy.unique(rows)
    sorted_list = numpy.sort(unique_list) if dosort else unique_list
    if len(sorted_list) == 0:
        sel = ''
    elif len(sorted_list) == 1:
        sel = str(sorted_list[0])
    else:
        index = 0
        sel = ''
        #print 'test: %s'%(sorted_list)
        increments = sorted_list[1:] - sorted_list[:-1]
        #print increments
        while index < len(sorted_list):
            start = index
            while index < len(increments) and increments[index] == 1:
                index += 1
            #print start, index
            
            if index - start < 2:
                _sel = ','.join(map(str,sorted_list[start:index+1]))
            else:
                _sel = '%s~%s'%(sorted_list[start],sorted_list[index])

            if len(sel) == 0:
                sel = _sel
            else:
                sel = ','.join([sel,_sel])

            #print '\'%s\''%(sel)
            index += 1

    return sel

def intersection(sel1, sel2):
    if sel1 is None or len(sel1) == 0:
        return '' if sel2 is None else sel2
    elif sel2 is None or len(sel2) == 0:
        return sel1
    else:
        set1 = set(selection_to_list(sel1))
        set2 = set(selection_to_list(sel2))
        l = list(set1 & set2)
        if len(l) == 0:
            return None
        else:
            return list_to_selection(l)
    
def pol_filter(group_desc, filter_func):
    for m in group_desc:
        filter = filter_func(m.antenna)
        if len(filter) == 0:
            yield m.pols
        else:
            yield list(set(filter) & set(m.pols))

def polstring(pols):
    if pols == [0, 1]:
        polstr = 'XXYY'
    elif pols == [0] or pols == 0:
        polstr = 'XX'
    elif pols == [1] or pols == 1:
        polstr = 'YY'
    else:
        polstr = 'I'
    return polstr

@contextlib.contextmanager
def TableSelector(name, query):
    with casatools.TableReader(name) as tb:
        tsel = tb.query(query)
        yield tsel
        tsel.close()
    
def nonscience_spw(spectral_windows):
    # detect spws for WVR and square-law detector (SQLD)
    pattern = '(SQLD|WVR)'
    for (spwid, spw) in spectral_windows.items():
        spw_name = spw.name
        match_by_name = re.search(pattern, spw_name) is not None
        match_by_intents = spw.is_target is False
        if match_by_name or match_by_intents:
            yield spwid

def science_spw(spectral_windows):
    # exclude spws for WVR and square-law detector (SQLD)
    pattern = '(SQLD|WVR|CH_AVG)'
    for (spwid, spw) in spectral_windows.items():
        spw_name = spw.name
        match_by_name = re.search(pattern, spw_name) is None
        match_by_intents = spw.is_target 
        if match_by_name and match_by_intents:
            yield spwid
    
# dictionary that always returns key         
class EchoDictionary(dict):
    def __getitem__(self, x):
        return x
    
def make_row_map_for_baselined_ms(ms):
    """
    Make row mapping between calibrated MS and baselined MS.
    Return value is a dictionary whose key is row number for calibrated MS and 
    its corresponding value is the one for baselined MS.
    
    ms: measurement set domain object
    
    returns: row mapping dictionary
    """
    work_data = ms.work_data
    
    return make_row_map(ms, work_data)

#@profiler
def make_row_map_by_taql(src_ms, derived_vis):
    """
    Make row mapping between source MS and associating MS
    
    src_ms: measurement set domain object for source MS 
    derived_vis: name of the MS that derives from source MS
    
    returns: row mapping dictionary
    """
    ms = src_ms
    vis0 = ms.name
    vis1 = derived_vis
    
    generate_taql_item = lambda column, value: \
        '{column} == {value}'.format(column=column, value=value)
    
    def generate_taql(processor_id=None, observation_id=None,
                      field_id=None, antenna1=None, antenna2=None,
                      data_desc_id=None, scan_number=None,
                      state_id=None, time=None):
        if antenna2 is None:
            antenna2 = antenna1
            
        taql_items = []
        taql_list = [('PROCESSOR_ID', processor_id),
                     ('OBSERVATION_ID', observation_id),
                     ('FIELD_ID', field_id),
                     ('ANTENNA1', antenna1),
                     ('ANTENNA2', antenna2),
                     ('DATA_DESC_ID', data_desc_id),
                     ('SCAN_NUMBER', scan_number),
                     ('STATE_ID', state_id),
                     ('TIME', time)]
        for (column, value) in taql_list:
            if value is not None:
                taql_items.append(generate_taql_item(column, value))
                
        taql = ' && '.join(taql_items)
        return taql

    rowmap = {}

    if vis0 == vis1:
        return EchoDictionary()
    
    start_time = time.time()
    LOG.debug('START processing "%s" and "%s"'%(vis0, vis1))
    
    # make polarization map between src MS and derived MS
    to_derived_polid = make_polid_map(vis0, vis1)
    LOG.trace('to_derived_polid=%s'%(to_derived_polid))
    
    # make spw map between src MS and derived MS
    to_derived_spwid = make_spwid_map(vis0, vis1)
    LOG.trace('to_derived_spwid=%s'%(to_derived_spwid))
    
    # make a map between (polid, spwid) pair and ddid for derived MS
    derived_ddid_map = make_ddid_map(vis1)
    LOG.trace('derived_ddid_map=%s'%(derived_ddid_map))
    
    with casatools.TableReader(vis0) as tb:
        observation_ids = set(tb.getcol('OBSERVATION_ID'))
        processor_ids = set(tb.getcol('PROCESSOR_ID'))
    scans = ms.get_scans(scan_intent='TARGET')
    for processor_id in processor_ids:
        LOG.trace('PROCESSOR_ID %s'%(processor_id))
        for observation_id in observation_ids:
            LOG.trace('OBSERVATION_ID %s'%(observation_id))
            for scan in scans:
                scan_number = scan.id
                LOG.trace('SCAN_NUMBER %s'%(scan_number))
                states = [s for s in scan.states if 'TARGET' in s.intents]

                if len(states) == 0:
                    LOG.trace('No target states in SCAN %s'%(scan_number))
                    continue
                
                for field in scan.fields:
                    field_id = field.id
                    LOG.trace('FIELD_ID %s'%(field_id))
                    for antenna in ms.antennas:
                        antenna_id = antenna.id
                        LOG.trace('ANTENNA_ID %s'%(antenna_id))
                        for spw in ms.get_spectral_windows(science_windows_only=True):
                            data_desc = ms.get_data_description(spw=spw)
                            data_desc_id = data_desc.id
                            pol_id = data_desc.pol_id
                            spw_id = spw.id
                            derived_pol_id = to_derived_polid[pol_id]
                            derived_spw_id = to_derived_spwid[spw_id]
                            derived_dd_id = derived_ddid_map[(derived_pol_id, derived_spw_id)]
                            LOG.trace('SRC DATA_DESC_ID %s (SPW %s)'%(data_desc_id, spw.id))
                            LOG.trace('DERIVED DATA_DESC_ID %s (SPW %s)'%(derived_dd_id, derived_spw_id))
                            for state in states:
                                state_id = state.id
                                LOG.trace('STATE_ID %s'%(state_id))

                                # get time stamp and row numbers for selected rows of vis0
                                taql0 = generate_taql(processor_id=processor_id,
													observation_id=observation_id,
													field_id=field_id,
													antenna1=antenna_id,
													antenna2=antenna_id,
													data_desc_id=data_desc_id,
													scan_number=scan_number,
													state_id=state_id)
                                with casatools.TableReader(vis0) as ti:
                                    tisel = ti.query(taql0, sortlist='TIME')
                                    try:
                                        LOG.trace('NROW = %s'%(tisel.nrows()))
                                        if tisel.nrows() > 0:
                                            time_in = tisel.getcol('TIME')
                                            row_in = tisel.rownumbers()
                                        else:
                                            time_in = None
                                            row_in = None
                                    finally:
                                        tisel.close()
                                        
                                LOG.trace('obtained time and row numbers for state %s'%(state.id))

                                if time_in is not None:
                                    # get time stamp and row numbers for selected rows of vis1
                                    taql1 = generate_taql(processor_id=processor_id,
                                                         observation_id=observation_id,
                                                         field_id=field_id,
                                                         antenna1=antenna_id,
                                                         antenna2=antenna_id,
                                                         data_desc_id=derived_dd_id,
                                                         scan_number=scan_number,
                                                         state_id=state_id)
                                    with casatools.TableReader(vis1) as to:
                                        tosel = to.query(taql1, sortlist='TIME')
                                        try:
                                            time_out = tosel.getcol('TIME')
                                            row_out = tosel.rownumbers()
                                        finally:
                                            tosel.close()
                                            
                                    assert numpy.all(time_in == time_out)

                                    for (rin, rout) in zip(row_in, row_out):
                                        rowmap[rin] = rout
                                else:
                                    LOG.trace('NOTE: no rows')
                                    
                                LOG.trace('DONE State %s'%(state.id))

    end_time = time.time()
    LOG.debug('Elapsed %s sec'%(end_time - start_time))
    return rowmap

#@profiler
def make_row_map(src_ms, derived_vis):
    """
    Make row mapping between source MS and associating MS
     
    src_ms: measurement set domain object for source MS 
    derived_vis: name of the MS that derives from source MS
     
    returns: row mapping dictionary
    """
    ms = src_ms
    vis0 = ms.name
    vis1 = derived_vis
 
    rowmap = {}
 
    if vis0 == vis1:
        return EchoDictionary()
          
    # make polarization map between src MS and derived MS
    to_derived_polid = make_polid_map(vis0, vis1)
    LOG.trace('to_derived_polid=%s'%(to_derived_polid))
     
    # make spw map between src MS and derived MS
    to_derived_spwid = make_spwid_map(vis0, vis1)
    LOG.trace('to_derived_spwid=%s'%(to_derived_spwid))
     
    # make a map between (polid, spwid) pair and ddid for derived MS
    derived_ddid_map = make_ddid_map(vis1)
    LOG.trace('derived_ddid_map=%s'%(derived_ddid_map))
     
    scans = ms.get_scans(scan_intent='TARGET')
    scan_numbers = [s.id for s in scans]
    fields = {}
    states = {}
    for scan in scans:
        fields[scan.id] = [f.id for f in scan.fields if 'TARGET' in f.intents]
        states[scan.id] = [s.id for s in scan.states if 'TARGET' in s.intents]
    field_values = fields.values()
    is_unique_field_set = True
    for v in field_values:
        if v != field_values[0]:
            is_unique_field_set = False
    state_values = states.values()
    is_unique_state_set = True
    for v in state_values:
        if v != state_values[0]:
            is_unique_state_set = False
    if is_unique_field_set and is_unique_state_set:
        taql = 'ANTENNA1 == ANTENNA2 && SCAN_NUMBER IN %s && FIELD_ID IN %s && STATE_ID IN %s'%(scan_numbers, field_values[0], state_values[0])
    else:
        taql = 'ANTENNA1 == ANTENNA2 && (%s)'%(' || '.join(['(SCAN_NUMBER == %s && FIELD_ID IN %s && STATE_ID IN %s)'%(scan.id, map(lambda x: x.id, scan.fields), map(lambda x: x.id, scan.states)) for scan in scans]))
    print taql
     
     
    with casatools.TableReader(os.path.join(vis0, 'OBSERVATION')) as tb:
        nrow_obs0 = tb.nrows()
    with casatools.TableReader(os.path.join(vis0, 'PROCESSOR')) as tb:
        nrow_proc0 = tb.nrows()
    with casatools.TableReader(os.path.join(vis1, 'OBSERVATION')) as tb:
        nrow_obs1 = tb.nrows()
    with casatools.TableReader(os.path.join(vis1, 'PROCESSOR')) as tb:
        nrow_proc1 = tb.nrows()
        
    assert nrow_obs0 == nrow_obs1
    assert nrow_proc0 == nrow_proc1
    
    is_unique_observation_id = nrow_obs0 == 1
    is_unique_processor_id = nrow_proc0 == 1
        
    with casatools.TableReader(vis0) as tb:
        tsel = tb.query(taql)
        try:
            if is_unique_observation_id:
                observation_id_list0 = None
                observation_id_set = set([0])
            else:
                observation_id_list0 = tsel.getcol('OBSERVATION_ID')
                observation_id_set = set(observation_id_list0)
            if is_unique_processor_id:
                processor_id_list0 = None
                processor_id_set = set([0])
            else:
                processor_id_list0 = tsel.getcol('PROCESSOR_ID')
                processor_id_set = set(processor_id_list0)
            scan_number_list0 = tsel.getcol('SCAN_NUMBER')
            field_id_list0 = tsel.getcol('FIELD_ID')
            antenna1_list0 = tsel.getcol('ANTENNA1')
            state_id_list0 = tsel.getcol('STATE_ID')
            data_desc_id_list0 = tsel.getcol('DATA_DESC_ID')
            time_list0 = tsel.getcol('TIME')
            rownumber_list0 = tsel.rownumbers()
        finally:
            tsel.close()
     
    with casatools.TableReader(vis1) as tb:
        tsel = tb.query(taql)
        try:
            if is_unique_observation_id:
                observation_id_list1 = None
            else:
                observation_id_list1 = tsel.getcol('OBSERVATION_ID')
            if is_unique_processor_id:
                processor_id_list1 = None
            else:
                processor_id_list1 = tsel.getcol('PROCESSOR_ID')
            scan_number_list1 = tsel.getcol('SCAN_NUMBER')
            field_id_list1 = tsel.getcol('FIELD_ID')
            antenna1_list1 = tsel.getcol('ANTENNA1')
            state_id_list1 = tsel.getcol('STATE_ID')
            data_desc_id_list1 = tsel.getcol('DATA_DESC_ID')
            time_list1 = tsel.getcol('TIME')
            rownumber_list1 = tsel.rownumbers()
        finally:
            tsel.close()
 
    for processor_id in processor_id_set:
         
        LOG.trace('PROCESSOR_ID %s'%(processor_id))
                      
        for observation_id in observation_id_set:
            LOG.trace('OBSERVATION_ID %s'%(observation_id))
             
            for scan_number in scan_numbers:
                LOG.trace('SCAN_NUMBER %s'%(scan_number))
 
                if len(states[scan_number]) == 0:
                    LOG.trace('No target states in SCAN %s'%(scan_number))
                    continue
                 
                for field_id in fields[scan_number]:
                    LOG.trace('FIELD_ID %s'%(field_id))
                                         
                    for antenna in ms.antennas:
                        antenna_id = antenna.id
                        LOG.trace('ANTENNA_ID %s'%(antenna_id))
                         
                        for spw in ms.get_spectral_windows(science_windows_only=True):
                            data_desc = ms.get_data_description(spw=spw)
                            data_desc_id = data_desc.id
                            pol_id = data_desc.pol_id
                            spw_id = spw.id
                            derived_pol_id = to_derived_polid[pol_id]
                            derived_spw_id = to_derived_spwid[spw_id]
                            derived_dd_id = derived_ddid_map[(derived_pol_id, derived_spw_id)]
                            LOG.trace('SRC DATA_DESC_ID %s (SPW %s)'%(data_desc_id, spw.id))
                            LOG.trace('DERIVED DATA_DESC_ID %s (SPW %s)'%(derived_dd_id, derived_spw_id))
                             
                            for state_id in states[scan_number]:
                                LOG.trace('STATE_ID %s'%(state_id))
                                 
                                mask0 = numpy.logical_and(state_id_list0 == state_id, 
                                            numpy.logical_and(data_desc_id_list0 == data_desc_id,
                                                            numpy.logical_and(antenna1_list0 == antenna_id,
                                                                    numpy.logical_and(field_id_list0 == field_id,                                         
                                                                            scan_number_list0 == scan_number))))
#                                 mask0 = numpy.logical_and.reduce((state_id_list0 == state_id,
#                                                                   data_desc_id_list0 == data_desc_id,
#                                                                   antenna2_list0 == antenna_id,
#                                                                   antenna1_list0 == antenna_id,
#                                                                   field_id_list0 == field_id,
#                                                                   scan_number_list0 == scan_number),
#                                                                  axis=0)
                                if not is_unique_processor_id:
                                    numpy.logical_and(mask0, processor_id_list0 == processor_id, out=mask0)
                                if not is_unique_observation_id:
                                    numpy.logical_and(mask0, observation_id_list0 == observation_id, out=mask0)

                               
                                LOG.trace('obtained time and row numbers for state %s'%(state_id))
 
                                if numpy.any(mask0 == True):
                                    # get time stamp and row numbers for selected rows of vis0
                                    time_in0 = time_list0[mask0]
                                    row_in0 = rownumber_list0[mask0]
                                    argsort0 = numpy.argsort(time_in0)
                                    time_in = time_in0[argsort0]
                                    row_in = row_in0[argsort0]
                                     
                                    # get time stamp and row numbers for selected rows of vis1                                             
                                    mask1 = numpy.logical_and(state_id_list1 == state_id, 
                                                    numpy.logical_and(data_desc_id_list1 == derived_dd_id,
                                                                    numpy.logical_and(antenna1_list1 == antenna_id,
                                                                            numpy.logical_and(field_id_list1 == field_id,                                         
                                                                                    scan_number_list1 == scan_number))))

                                    if not is_unique_processor_id:
                                        numpy.logical_and(mask1, processor_id_list1 == processor_id, out=mask1)
                                    if not is_unique_observation_id:
                                        numpy.logical_and(mask1, observation_id_list1 == observation_id, out=mask1)
                                      
                                    time_out1 = time_list1[mask1]
                                    row_out1 = rownumber_list1[mask1]
                                    argsort1 = numpy.argsort(time_out1)
                                    time_out = time_out1[argsort1]
                                    row_out = row_out1[argsort1]

                                    assert numpy.all(time_in == time_out)
 
                                    for (rin, rout) in zip(row_in, row_out):
                                        rowmap[rin] = rout
                                else:
                                    assert numpy.all(mask0 == False)
                                    LOG.trace('NOTE: no rows')
                                     
                                LOG.trace('DONE State %s'%(state_id))
 
    return rowmap

def __read_table(reader, method, vis):
    if reader is None:
        result = method(vis)
    else:
        with reader(vis) as readerobj:
            result = method(readerobj)
    return result

def _read_table(reader, table, vis):
    rows = __read_table(reader, table._read_table, vis)
    return rows

def make_spwid_map(srcvis, dstvis):
    src_spws = __read_table(casatools.MSMDReader, 
                            tablereader.SpectralWindowTable.get_spectral_windows,
                            srcvis)
    dst_spws = __read_table(casatools.MSMDReader, 
                            tablereader.SpectralWindowTable.get_spectral_windows,
                            dstvis)
    for spw in src_spws:
        LOG.trace('SRC SPWID %s NAME %s'%(spw.id,spw.name))
    for spw in dst_spws:
        LOG.trace('DST SPWID %s NAME %s'%(spw.id,spw.name))
        
    map_byname = collections.defaultdict(list)
    for src_spw in src_spws:
        for dst_spw in dst_spws:
            if src_spw.name == dst_spw.name:
                map_byname[src_spw].append(dst_spw)
    
    spwid_map = {}
    for (src,dst) in map_byname.items():
        LOG.trace('map_byname src spw %s: dst spws %s'%(src.id, [spw.id for spw in dst]))    
        if len(dst) == 0:
            continue
        elif len(dst) == 1:
            # mapping by name worked
            spwid_map[src.id] = dst[0].id
        else:
            # need more investigation
            for spw in dst:
                if src.num_channels == spw.num_channels \
                    and src.ref_frequency == spw.ref_frequency \
                    and src.min_frequency == spw.min_frequency \
                    and src.max_frequency == spw.max_frequency:
                    if spwid_map.has_key(src.id):
                        raise RuntimeError('Failed to create spw map for MSs \'%s\' and \'%s\''%(vis0,vis1))
                    spwid_map[src.id] = spw.id
    return spwid_map
    

def make_polid_map(srcvis, dstvis):
    src_rows = _read_table(None, tablereader.PolarizationTable, srcvis)
    dst_rows = _read_table(None, tablereader.PolarizationTable, dstvis)
    for (src_polid, src_numpol, src_poltype, _, _) in src_rows:
        LOG.trace('SRC: POLID %s NPOL %s POLTYPE %s'%(src_polid, src_numpol, src_poltype))
    for (dst_polid, dst_numpol, dst_poltype, _, _) in dst_rows:
        LOG.trace('DST: POLID %s NPOL %s POLTYPE %s'%(dst_polid, dst_numpol, dst_poltype))
    polid_map = {}
    for (src_polid, src_numpol, src_poltype, _, _) in src_rows:
        for (dst_polid, dst_numpol, dst_poltype, _, _) in dst_rows:
            if src_numpol == dst_numpol and numpy.all(src_poltype == dst_poltype):
                polid_map[src_polid] = dst_polid
    LOG.trace('polid_map = %s'%(polid_map))
    return polid_map

def make_ddid_map(vis):
    table_rows = _read_table(casatools.MSMDReader, tablereader.DataDescriptionTable, vis)
    return dict((((polid,spwid),ddid) for ddid,spwid,polid in table_rows))

def get_datacolumn_name(vis):
    colname_candidates = ['CORRECTED_DATA', 'FLOAT_DATA', 'DATA']
    with casatools.TableReader(vis) as tb:
        colnames = tb.colnames()
    colname = None
    for name in colname_candidates:
        if name in colnames:
            colname = name
            break
    assert colname is not None
    return colname

@contextlib.contextmanager
def asap_force_storage(storage='disk'):
    import asap as sd
    key = 'scantable.storage'
    storage_org = sd.rcParams[key]
    sd.rcParams[key] = storage  
    yield None
    sd.rcParams[key] = storage_org
    
# helper functions for parallel execution
def create_serial_job(task_cls, task_args, context):
    inputs = task_cls.Inputs(context, **task_args)
    task = task_cls(inputs)
    job = mpihelpers.SyncTask(task)
    LOG.debug('Serial Job: %s'%(task))
    return job

def create_parallel_job(task_cls, task_args, context):
    context_path = os.path.join(context.output_dir, context.name + '.context')
    if not os.path.exists(context_path):
        context.save(context_path)
    task = mpihelpers.Tier0PipelineTask(task_cls, task_args, context_path)
    job = mpihelpers.AsyncTask(task)
    LOG.debug('Parallel Job: %s'%(task))
    return job
