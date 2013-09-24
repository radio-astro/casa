from __future__ import absolute_import
import collections
import contextlib
import copy
import datetime
import decimal
import functools
import itertools
import math
import operator
import os
import platform
import re
import sys
import StringIO
import types

import pipeline.extern.pyparsing as pyparsing
import pipeline.extern.ps_mem as ps_mem

from . import casatools
from . import logging
from . import jobrequest

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


def commafy(l, quotes=True, multi_prefix='', separator=', '):
    '''
    Return the textual description of the given list.
    
    For example: commafy(['a','b','c']) = "'a', 'b' and 'c'"     
    '''
    if type(l) is not types.ListType and isinstance(l, collections.Iterable):
        l = [i for i in l]
    
    # turn 's' into 's '
    if multi_prefix:
        multi_prefix += ' '
    
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
            return '%s\'%s\' and \'%s\'' % (multi_prefix, l[0], l[1])
        else:
            return '%s%s and %s' % (multi_prefix, l[0], l[1])
    else:
        if quotes:
            return '%s\'%s\'%s%s' % (multi_prefix, l[0], separator, commafy(l[1:], separator=separator, quotes=quotes))
        else: 
            return '%s%s%s%s' % (multi_prefix, l[0], separator, commafy(l[1:], separator=separator, quotes=quotes))

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

def unix_seconds_to_datetime(unix_secs):
    """
    Return the input list, specified in seconds elapsed since 1970-01-01,
    converted to the equivalent Python datetimes.
    
    If given a list, a list is returned. If given a scalar, a scalar is 
    returned.
    """   
    datetimes = [datetime.datetime.fromtimestamp(s) for s in unix_secs]
    return datetimes if len(unix_secs) > 1 else datetimes[0] 

def mjd_seconds_to_datetime(mjd_secs):
    """
    Return the input list, specified in MJD seconds, converted to the 
    equivalent Python datetimes.
    
    If given a list, a list is returned. If given a scalar, a scalar is 
    returned.
    """   
    # 1970-01-01 is JD 40587. 86400 = seconds in a day
    unix_offset = 40587 * 86400
    return unix_seconds_to_datetime(mjd_secs - unix_offset)

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

def to_CASA_intent(ms, intents):
    '''
    Convert pipeline intents back to obs modes, and then to an intent CASA will
    understand.
    '''
    obs_modes = ms.get_original_intent(intents)
    if obs_modes:
        r = re.compile('\W*_([a-zA-Z]*)[#\._]\W*')
        intents = [r.findall(obs_mode) for obs_mode in obs_modes]
        # convert the list of lists back to a 1-D list
        intents = set(itertools.chain(*intents))
        # replace the CASA arg with *INTENT1*,*INTENT2*, etc.
        return ','.join(['*{0}*'.format(intent) for intent in intents])

def stringSplitByNumbers(x):
    r = re.compile('(\d+)')
    l = r.split(x)
    return [int(y) if y.isdigit() else y for y in l]

def numericSort(l):
    """
    Sort a list numerically, eg. 
    
    ['9,11,13,15', '11,13', '9'] -> ['9', '9,11,13,15', '11,13']
    """    
    return sorted(l, key = stringSplitByNumbers)

def truncate_floats(s, precision=3):
    """
    Return a copy of the input string with all floating point numbers
    truncated to the given precision.

    Example:
    truncate_float('a,2,123.456789', 3) => 'a,2,123.456'

    :param s: the string to modify
    :param precision: the maximum floating point precision
    :return: a string
    """
    # we need fixed-width terms to so a positive look-behind search,
    # so instead of finding the offending digits and removing them,
    # we have to capture the good digits and cut off the remainder
    # during replacement.
    pattern = '(\d+\.\d{%s})\d+' % precision
    return re.sub(pattern, '\\1', s)

def enable_memstats():
    if platform.system() == 'Darwin':
        LOG.error('Cannot measure memory on OS X.')
        return
    
    import pipeline.domain.measures as measures
    def get_hook_fn(msg):
        pid = os.getpid()
    
        def log_mem_usage(jobrequest):
            sorted_cmds, shareds, _, _ = ps_mem.get_memory_usage([pid,], False, True)
            for cmd in sorted_cmds:
                private = measures.FileSize(cmd[1]-shareds[cmd[0]], 
                                            measures.FileSizeUnits.KILOBYTES)
                shared = measures.FileSize(shareds[cmd[0]], 
                                           measures.FileSizeUnits.KILOBYTES)
                total = measures.FileSize(cmd[1], measures.FileSizeUnits.KILOBYTES)

                LOG.info('%s%s: private=%s shared=%s total=%s' % (
                        msg, jobrequest.fn.__name__, str(private),
                        str(shared), str(total)))
                    
            vm_accuracy = ps_mem.shared_val_accuracy()
            if vm_accuracy is -1:
                LOG.warning("Shared memory is not reported by this system. "
                            "Values reported will be too large, and totals "
                            "are not reported")
            elif vm_accuracy is 0:
                LOG.warning("Shared memory is not reported accurately by "
                            "this system. Values reported could be too "
                            "large, and totals are not reported.")
            elif vm_accuracy is 1:
                LOG.warning("Shared memory is slightly over-estimated by "
                            "this system for each program, so totals are "
                            "not reported.")
                
        return log_mem_usage

    jobrequest.PREHOOKS.append(get_hook_fn('Memory usage before '))
    jobrequest.POSTHOOKS.append(get_hook_fn('Memory usage after '))


def get_calfroms(inputs, caltypes=None):
    """
    Get the CalFroms of the requested type from the callibrary.
    
    This function assumes that 'vis' is a property of the given inputs. 
    """
    import pipeline.infrastructure.callibrary as callibrary
    if caltypes is None:
        caltypes = callibrary.CalFrom.CALTYPES.keys()

    # check that the 
    if type(caltypes) is types.StringType:
        caltypes = (caltypes,)
            
    for c in caltypes:
        assert c in callibrary.CalFrom.CALTYPES

    # get the CalState for the ms - no field/spw/antenna selection (for now..)
    calto = callibrary.CalTo(vis=inputs.vis)
    calstate = inputs.context.callibrary.get_calstate(calto)

    calfroms = (itertools.chain(*calstate.merged().values()))
    
    return [cf for cf in calfroms if cf.caltype in caltypes]
    
    
class memoized(object):
    '''
    Function decorator. Caches a function's return value each time it is 
    called. If called later with the same arguments, the cached value is 
    returned (not re-evaluated).
    '''
    def __init__(self, func):
        self.func = func
        self.cache = {}
    def __call__(self, *args):
        if not isinstance(args, collections.Hashable):
            # uncacheable. a list, for instance.
            # better to not cache than blow up.
            return self.func(*args)
        if args in self.cache:
            return self.cache[args]
        else:
            value = self.func(*args)
            self.cache[args] = value
            return value
    def __repr__(self):
        '''Return the function's docstring.'''
        return self.func.__doc__
    def __get__(self, obj, objtype):
        '''Support instance methods.'''
        return functools.partial(self.__call__, obj)
    