from __future__ import absolute_import
import collections
import contextlib
import copy
import datetime
import decimal
import itertools
import math
import operator
import types

import pipeline.extern.pyparsing as pyparsing

from . import casatools
from . import logging

LOG = logging.get_logger(__name__)


def context_manager_factory(tool):
    '''
    Create a context manager function that wraps the given CASA tool.
    
    The returned context manager function takes one argument: a filename. The
    function opens the file using the CASA tool, returning the tool so that it
    may be used for queries or other operations pertaining to the tool. The 
    tool is closed once it falls out of scope or an exception is raised.
    '''
    tool_name = tool.__class__.__name__
    
    @contextlib.contextmanager    
    def f(filename):
        LOG.trace('%s tool: opening \'%s\'' % (tool_name, filename))
        tool.open(filename)
        yield tool
        LOG.trace('%s tool: closing \'%s\'' % (tool_name, filename))
        tool.close()
    return f

# context manager for CASA table tool
open_table = context_manager_factory(casatools.table)
# context manager for CASA image tool
open_image = context_manager_factory(casatools.image)
# context manager for CASA ms tool
open_ms = context_manager_factory(casatools.ms)


def commafy(l, quotes=True):
    '''
    Return the textual description of the given list.
    
    For example: commafy(['a','b','c']) = "'a', 'b' and 'c'"     
    '''
    if type(l) is not types.ListType and isinstance(l, collections.Iterable):
        l = [i for i in l]
    
    length = len(l)
    if length is 0:
        return ''
    if length is 1:
        if quotes:
            return '\'%s\'' % l[0] 
        else:
            return '%s' % l[0]
    if length is 2:
        if quotes:
            return '\'%s\' and \'%s\'' % (l[0], l[1])
        else:
            return '%s and %s' % (l[0], l[1])
    else:
        if quotes:
            return '\'%s\', %s' % (l[0], commafy(l[1:], quotes))
        else: 
            return '%s, %s' % (l[0], commafy(l[1:], quotes))

def find_ranges(data):
    try:
        integers = [int(d) for d in data.split(',')]
    except ValueError:
        return data

    s = sorted(integers)
    ranges = []
    for _, g in itertools.groupby(enumerate(s), lambda (i,x):i-x):
        rng = map(operator.itemgetter(1), g)
        if len(rng) is 1:
            ranges.append('%s' % rng[0])
        else:
            ranges.append('%s~%s' % (rng[0], rng[-1]))
    return ','.join(ranges)

def get_epoch_as_datetime(epoch):
    mt = casatools.measures
    qt = casatools.quanta

    # calculate UTC standard offset
    datetime_base = mt.epoch('UTC', '40587.0d')
    base_time = mt.getvalue(datetime_base)['m0']
    base_time = qt.convert(base_time, 'd')
    base_time = qt.floor(base_time)

    # subtract offset from UTC equivalent time
    epoch_utc = mt.measure(epoch, 'UTC')
    t = mt.getvalue(epoch_utc)['m0']
    t = qt.sub(t, base_time)  
    t = qt.convert(t, 's')
    t = datetime.datetime.utcfromtimestamp(qt.getvalue(t))

    return t

def total_time_on_source(scans):
    '''
    Return the total time on source for the given Scans.
    
    scans -- a collection of Scan domain objects
    return -- a datetime.timedelta object set to the total time on source
    '''
    times_on_source = [scan.time_on_source for scan in scans]
    return reduce(operator.add, times_on_source)

def format_datetime(dt):
    '''
    Return a formatted string representation for the given datetime
    '''
    # Ignore microseconds
    return dt.strftime('%d/%m/%Y %H:%M:%S')

def format_timedelta(td, dp=0):
    '''
    Return a formatted string representation for the given timedelta
    '''
    # 
    secs = decimal.Decimal(td.seconds) 
    microsecs = decimal.Decimal(td.microseconds) / decimal.Decimal('1e6')
    rounded_secs = (secs + microsecs).quantize(decimal.Decimal(10) ** -dp)
    rounded = datetime.timedelta(days=td.days, 
                                 seconds=math.floor(rounded_secs))
    # get rounded number of microseconds as an integer
    rounded_microsecs = int((rounded_secs % 1).shift(6))
    # .. which we can pad with zeroes..
    str_microsecs = '{0:06d}'.format(rounded_microsecs)
    # .. which we can append onto the end of the default timedelta string
    # representation
    if dp:
        fraction = str_microsecs[0:dp]
        return str(rounded) + '.' + str(fraction)
    else:
        return str(rounded)

def dict_merge(a, b):
    '''
    Recursively merge dictionaries.
    '''
    if not isinstance(b, dict):
        return b
    result = copy.deepcopy(a)
    for k, v in b.iteritems():
        if k in result and isinstance(result[k], dict):
                result[k] = dict_merge(result[k], v)
        else:
            result[k] = copy.deepcopy(v)
    return result

def safe_split(fields):
    '''
    Split a string containing field names into a list, taking account of 
    field names within quotes.
    ''' 
    return pyparsing.commaSeparatedList.parseString(str(fields))
