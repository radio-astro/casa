from __future__ import absolute_import 

import sys
import os
import numpy
import contextlib
import re

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

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)

def asdm_name(scantable_object):
    """
    Return ASDM name that target scantable belongs to.
    Assumptions are:
       - scantable is generated from MS
       - MS is generated from ASDM
       - MS name is <uid>.ms
    """
    ms_basename = scantable_object.ms.basename
    index_for_suffix = ms_basename.rfind('.')
    asdm = ms_basename[:index_for_suffix] if index_for_suffix > 0 \
           else ms_basename
    return asdm
    

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
    return [ 0 if int(flg)!=0 else 1 for flg in flagtra ]


@contextlib.contextmanager
def temporary_filename(name='_heuristics.temporary.table'):
    yield name
    os.system('rm -rf %s'%(name))

def get_index_list(datatable, antenna, spw, pols=None, srctype=None):
    assert len(antenna) == len(spw)
    table = datatable.tb1
    antenna_column = table.getcol('ANTENNA')
    spw_column = table.getcol('IF')
    pol_column = table.getcol('POL')
    srctype_column = table.getcol('SRCTYPE')
    f = lambda i, j: antenna_column[i] == antenna[j] and spw_column[i] == spw[j]
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

    nrow = table.nrows()
    nval = len(antenna)
    for irow in xrange(nrow):
        for ival in xrange(nval):
            if sel(irow, ival):
                yield irow

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

def selection_to_list(sel):
    def _selection_to_list(sel):
        elements = sel.split(',')
        for elem in elements:
            if elem.isdigit():
                yield int(elem)
            elif re.match('^[0-9]+~[0-9]+$', elem):
                s = [int(e) for e in elem.split('~')]
                for i in xrange(s[0], s[1]+1):
                    yield i
    l = set(_selection_to_list(sel))
    return list(l)

def list_to_selection(rows):
    unique_list = numpy.unique(rows)
    sorted_list = numpy.sort(unique_list)
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
    
