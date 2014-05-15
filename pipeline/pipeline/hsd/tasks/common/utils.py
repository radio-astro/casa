from __future__ import absolute_import 

import sys
import os
import numpy
import contextlib

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

def get_index_list(datatable, antenna, spw, srctype):
    assert len(antenna) == len(spw)
    table = datatable.tb1
    antenna_column = table.getcol('ANTENNA')
    spw_column = table.getcol('IF')
    if srctype is None:
        srctype_column = None
        f = lambda i, j: antenna_column[i] == antenna[j] and spw_column[i] == spw[j] 
    else:
        srctype_column = table.getcol('SRCTYPE')
        f = lambda i, j: antenna_column[i] == antenna[j] and spw_column[i] == spw[j] and srctype_column[i] == srctype

    nrow = table.nrows()
    nval = len(antenna)
    for irow in xrange(nrow):
        for ival in xrange(nval):
            if f(irow, ival):
                yield irow

def get_valid_members(group_desc, antenna_filter, spwid_filter):
    for i in xrange(len(group_desc)):
        member = group_desc[i]
        antenna = member.antenna
        spwid = member.spw
        if antenna in antenna_filter:
            if spwid_filter is None or spwid in spwid_filter:
                yield i
