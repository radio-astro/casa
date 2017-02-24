from __future__ import absolute_import 

import sys
import os
import numpy
import contextlib
import re
import time
import collections
import functools

import pipeline.infrastructure.mpihelpers as mpihelpers
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.casatools as casatools
from pipeline.domain.datatable import OnlineFlagIndex
import pipeline.infrastructure.tablereader as tablereader

_LOG = infrastructure.get_logger(__name__)

class OnDemandStringParseLogger(object):
    PRIORITY_MAP = {'warn': 'warning'}
    
    def __init__(self, logger):
        self.logger = logger
        self._func_list = []

    @staticmethod
    def parse(msg_template, *args, **kwargs):
        if len(args) == 0 and len(kwargs) == 0:
            return msg_template
        else:
            return msg_template.format(*args, **kwargs)

    def _post(self, priority, msg_template, *args, **kwargs):
        key_for_level = self.PRIORITY_MAP.get(priority, priority)
        if self.logger.isEnabledFor(logging.LOGGING_LEVELS[key_for_level]):
            getattr(self.logger, priority)(OnDemandStringParseLogger.parse(msg_template, *args, **kwargs))

    def critical(self, msg_template, *args, **kwargs):
        self._post('critical', msg_template, *args, **kwargs)

    def error(self, msg_template, *args, **kwargs):
        self._post('error', msg_template, *args, **kwargs)

    def warn(self, msg_template, *args, **kwargs):
        self._post('warning', msg_template, *args, **kwargs)

    def info(self, msg_template, *args, **kwargs):
        self._post('info', msg_template, *args, **kwargs)

    def debug(self, msg_template, *args, **kwargs):
        self._post('debug', msg_template, *args, **kwargs)

    def todo(self, msg_template, *args, **kwargs):
        self._post('todo', msg_template, *args, **kwargs)

    def trace(self, msg_template, *args, **kwargs):
        self._post('trace', msg_template, *args, **kwargs)
        
LOG = OnDemandStringParseLogger(_LOG)

def profiler(func):
    @functools.wraps(func)
    def wrapper(*args, **kw):
        start = time.time()
#        LOG.info('#TIMING# Begin {} at {}', func.__name__, start)
        result = func(*args, **kw)
        end = time.time()
#        LOG.info('#TIMING# End {} at {}', func.__name__, end)
        
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
            self.LogLevel = logging.LOGGING_LEVELS[LogLevel] if logging.LOGGING_LEVELS.has_key(LogLevel) else logging.INFO
        else:
            # should be integer
            self.LogLevel = LogLevel
        if self.LogLevel >= logging.INFO:
            print '\n|' + '='*((length-8)/2) + ' 100% ' + '='*((length-8)/2) + '|'

    def __del__(self):
        if self.LogLevel >= logging.INFO:
            print '\n'

    def count(self, increment=1):
        if self.LogLevel >= logging.INFO:
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
               
def get_valid_ms_members(group_desc, msname_filter, ant_selection, field_selection, spw_selection):
    for member_id in xrange(len(group_desc)):
        member = group_desc[member_id]
        spw_id = member.spw_id
        field_id = member.field_id
        ant_id = member.antenna_id
        msobj = member.ms
        if msobj.name in [os.path.abspath(name) for name in msname_filter]:
            try:
                mssel = casatools.ms.msseltoindex(vis=msobj.name, spw=spw_selection,
                                                  field=field_selection, baseline=ant_selection)
            except RuntimeError, e:
                LOG.trace('RuntimeError: {0}'.format(str(e)))
                LOG.trace('vis="{0}" field_selection: "{1}"'.format(msobj.name, field_selection))
                continue
            spwsel = mssel['spw']
            fieldsel = mssel['field']
            antsel = mssel['antenna1']
            if (len(spwsel) == 0 or spw_id in spwsel) and \
            (len(fieldsel) == 0 or field_id in fieldsel) and \
            (len(antsel) == 0 or ant_id in antsel):
                yield member_id

def _collect_logrecords(logger):
    capture_handlers = [h for h in logger.handlers if h.__class__.__name__ == 'CapturingHandler']
    logrecords = []
    for handler in capture_handlers:
        logrecords.extend(handler.buffer[:])
    return logrecords

@contextlib.contextmanager
def TableSelector(name, query):
    with casatools.TableReader(name) as tb:
        tsel = tb.query(query)
        yield tsel
        tsel.close()
    
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
        taql = 'ANTENNA1 == ANTENNA2 && (%s)'%(' || '.join(['(SCAN_NUMBER == %s && FIELD_ID IN %s && STATE_ID IN %s)'%(scan, fields[scan], states[scan]) for scan in scan_numbers]))
    LOG.trace('taql=\'%s\''%(taql))
     
     
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
 
                if not states.has_key(scan_number): 
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
                            LOG.trace('START PROCESSOR %s SCAN %s DATA_DESC_ID %s ANTENNA %s FIELD %s'%(processor_id, scan_number,data_desc_id,antenna_id,field_id))
                            derived_pol_id = to_derived_polid[pol_id]
                            derived_spw_id = to_derived_spwid[spw_id]
                            derived_dd_id = derived_ddid_map[(derived_pol_id, derived_spw_id)]
                            LOG.trace('SRC DATA_DESC_ID %s (SPW %s)'%(data_desc_id, spw_id))
                            LOG.trace('DERIVED DATA_DESC_ID %s (SPW %s)'%(derived_dd_id, derived_spw_id))
                             
                             
                            tmask0 = numpy.logical_and(data_desc_id_list0 == data_desc_id,
                                                numpy.logical_and(antenna1_list0 == antenna_id,
                                                    numpy.logical_and(field_id_list0 == field_id,                                         
                                                                      scan_number_list0 == scan_number)))
                            if not is_unique_processor_id:
                                numpy.logical_and(tmask0, processor_id_list0 == processor_id, out=tmask0)
                            if not is_unique_observation_id:
                                numpy.logical_and(tmask0, observation_id_list0 == observation_id, out=tmask0)

                            tmask1 = numpy.logical_and(data_desc_id_list1 == derived_dd_id,
                                                numpy.logical_and(antenna1_list1 == antenna_id,
                                                    numpy.logical_and(field_id_list1 == field_id,                                         
                                                                      scan_number_list1 == scan_number)))
                            if not is_unique_processor_id:
                                numpy.logical_and(tmask1, processor_id_list1 == processor_id, out=tmask1)
                            if not is_unique_observation_id:
                                numpy.logical_and(tmask1, observation_id_list1 == observation_id, out=tmask1)
                            
                            if numpy.all(tmask0 == False) and numpy.all(tmask1 == False):
                                # no corresponding data (probably due to PROCESSOR_ID for non-science windows)
                                LOG.trace('SKIP PROCESSOR %s SCAN %s DATA_DESC_ID %s ANTENNA %s FIELD %s'%(processor_id, scan_number,data_desc_id,antenna_id,field_id))
                                continue
                            
                            tstate0 = state_id_list0[tmask0]
                            tstate1 = state_id_list1[tmask1]
                            ttime0 = time_list0[tmask0]
                            ttime1 = time_list1[tmask1]
                            trow0 = rownumber_list0[tmask0]
                            trow1 = rownumber_list1[tmask1]
                            sort_index0 = numpy.lexsort((tstate0, ttime0))
                            sort_index1 = numpy.lexsort((tstate1, ttime1))
                            LOG.trace('scan %s'%(scan_number)
                                     + ' actual %s'%(list(set(tstate0))) 
                                     + ' expected %s'%(states[scan_number]))
                            assert numpy.all(ttime0[sort_index0] == ttime1[sort_index1])
                            assert numpy.all(tstate0[sort_index0] == tstate1[sort_index1])
                            #assert set(tstate0) == set(states[scan_number])
                            assert set(tstate0).issubset(set(states[scan_number]))
                            
                            for (i0, i1) in zip(sort_index0, sort_index1):
                                r0 = trow0[i0]
                                r1 = trow1[i1]
                                rowmap[r0] = r1

                            LOG.trace('END PROCESSOR %s SCAN %s DATA_DESC_ID %s ANTENNA %s FIELD %s'%(processor_id, scan_number,data_desc_id,antenna_id,field_id))

    return rowmap

class SpwSimpleView(object):
    def __init__(self, spwid, name):
        self.id = spwid
        self.name = name
        
class SpwDetailedView(object):
    def __init__(self, spwid, name, num_channels, ref_frequency, min_frequency, max_frequency):
        self.id = spwid
        self.name = name
        self.num_channels = num_channels
        self.ref_frequency = ref_frequency
        self.min_frequency = min_frequency
        self.max_frequency = max_frequency

def get_spw_names(vis):
    with casatools.TableReader(os.path.join(vis, 'SPECTRAL_WINDOW')) as tb:
        gen = (SpwSimpleView(i, tb.getcell('NAME', i)) for i in xrange(tb.nrows()))
        spws = list(gen)
    return spws

def get_spw_properties(vis):
    with casatools.TableReader(os.path.join(vis, 'SPECTRAL_WINDOW')) as tb:
        spws = []
        for irow in xrange(tb.nrows()):
            name = tb.getcell('NAME', irow)
            nchan = tb.getcell('NUM_CHAN', irow)
            ref_freq = tb.getcell('REF_FREQUENCY', irow)
            chan_freq = tb.getcell('CHAN_FREQ', irow)
            chan_width = tb.getcell('CHAN_WIDTH', irow)
            min_freq = chan_freq.min() - abs(chan_width[0]) / 2
            max_freq = chan_freq.max() + abs(chan_width[0]) / 2
            spws.append(SpwDetailedView(irow, name, nchan, ref_freq, min_freq, max_freq))
    return spws

#@profiler
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

#@profiler
def make_spwid_map(srcvis, dstvis):
#     src_spws = __read_table(casatools.MSMDReader, 
#                             tablereader.SpectralWindowTable.get_spectral_windows,
#                             srcvis)
#     dst_spws = __read_table(casatools.MSMDReader, 
#                             tablereader.SpectralWindowTable.get_spectral_windows,
#                             dstvis)

    src_spws = __read_table(None, get_spw_properties, srcvis)
    dst_spws = __read_table(None, get_spw_properties, dstvis)
    
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
                        raise RuntimeError('Failed to create spw map for MSs \'%s\' and \'%s\''%(srcvis,dstvis))
                    spwid_map[src.id] = spw.id
    return spwid_map
    

#@profiler
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

#@profiler
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

