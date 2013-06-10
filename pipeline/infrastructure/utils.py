from __future__ import absolute_import
import collections
import contextlib
import datetime
import itertools
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


def commafy(l):
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
        return '\'%s\'' % l[0]
    if length is 2:
        return '\'%s\' and \'%s\'' % (l[0], l[1])
    else:
        return '\'%s\', %s' % (l[0], commafy(l[1:])) 

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

    time_ref = mt.getref(epoch)
    datetime_base = mt.epoch(time_ref, '40587.0d')
    base_time = mt.getvalue(datetime_base)['m0']
    base_time = qt.convert(base_time, 'd')
    base_time = qt.floor(base_time)
    
    t = mt.getvalue(epoch)['m0']
    t = qt.sub(t, base_time)  
    t = qt.convert(t, 's')
    t = datetime.datetime.utcfromtimestamp(qt.getvalue(t))

    return t

def safe_split(fields):
    '''
    Split a string containing field names into a list, taking account of 
    field names within quotes.
    ''' 
    return pyparsing.commaSeparatedList.parseString(str(fields))
