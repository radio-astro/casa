from __future__ import absolute_import
import collections
import datetime
import itertools
import operator
import types

import pipeline.infrastructure.casatools as casatools

def commafy(l=[]):
    if type(l) is not types.ListType and isinstance(l, collections.Iterable):
        l = [i for i in l]
    
    if len(l) is 0:
        return ''
    if len(l) is 1:
        return '\'%s\'' % l[0]
    if len(l) is 2:
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
