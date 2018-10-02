"""
The utils module contains general-purpose uncategorised utility functions and
classes.
"""
import copy
import itertools
import operator
import re

import numpy as np

from .conversion import range_to_list
from .. import casatools
from .. import logging

LOG = logging.get_logger(__name__)

__all__ = ['find_ranges', 'dict_merge', 'are_equal', 'approx_equal', 'get_num_caltable_polarizations',
           'flagged_intervals']


def find_ranges(data):
    if isinstance(data, str):
        # barf if channel ranges are also in data, eg. 23:1~10,24
        if ':' in data:
            return data

        data = range_to_list(data)
        if len(data) is 0:
            return ''

    try:
        integers = [int(d) for d in data]
    except ValueError:
        return ','.join(data)

    s = sorted(integers)
    ranges = []
    for _, g in itertools.groupby(enumerate(s), lambda (i, x): i - x):
        rng = map(operator.itemgetter(1), g)
        if len(rng) is 1:
            ranges.append('%s' % rng[0])
        else:
            ranges.append('%s~%s' % (rng[0], rng[-1]))
    return ','.join(ranges)


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


def are_equal(a, b):
    """
    Return True if the contents of the given arrays are equal.
    """
    return len(a) == len(b) and len(a) == sum([1 for i, j in zip(a, b) if i == j])


def approx_equal(x, y, tol=1e-15):
    """
    Return True if two numbers are equal within the given tolerance.
    """
    lo = min(x, y)
    hi = max(x, y)
    return (lo + 0.5 * tol) >= (hi - 0.5 * tol)


def get_num_caltable_polarizations(caltable):
    # it seems that the number of QA ID does not map directly to the number
    # of polarisations for the spw in the MS, but the number of polarisations
    # for the spw as held in the caltable.

    with casatools.TableReader(caltable) as tb:
        col_shapes = set(tb.getcolshapestring('CPARAM'))

    # get the number of pols stored in the caltable, checking that this
    # is consistent across all rows
    fmt = re.compile(r'\[(?P<num_pols>\d+), (?P<num_rows>\d+)\]')
    col_pols = set()
    for shape in col_shapes:
        m = fmt.match(shape)
        if m:
            col_pols.add(int(m.group('num_pols')))
        else:
            raise ValueError('Could not find shape of polarisation from %s' % shape)

    if len(col_pols) is not 1:
        raise ValueError('Got %s polarisations from %s' % (len(col_pols), col_shapes))

    return int(col_pols.pop())


def flagged_intervals(vec):
    """
    Find islands of non-zeros in the vector vec
    Used to find contiguous flagged channels in a given spw.  Returns a list of tuples with the start and end channels.

    :param vec:
    :return:
    """
    if len(vec) == 0:
        return []
    elif not isinstance(vec, np.ndarray):
        vec = np.array(vec)

    edges, = np.nonzero(np.diff((vec == True) * 1))
    edge_vec = [edges + 1]
    if vec[0] != 0:
        edge_vec.insert(0, [0])
    if vec[-1] != 0:
        edge_vec.append([len(vec)])
    edges = np.concatenate(edge_vec)
    return zip(edges[::2], edges[1::2] - 1)
