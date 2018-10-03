"""
The sorting module contains utility functions used by the pipeline web log.
"""
import collections
import datetime
import itertools
import operator
import os
from functools import reduce

import numpy

from .conversion import flatten, to_pipeline_intent, spw_arg_to_id
from .. import casatools
from .. import logging

__all__ = ['OrderedDefaultdict', 'merge_td_columns', 'get_vis_from_plots', 'total_time_on_source',
           'total_time_on_target_on_source', 'get_logrecords', 'get_intervals']

LOG = logging.get_logger(__name__)


class OrderedDefaultdict(collections.OrderedDict):
    def __init__(self, *args, **kwargs):
        if not args:
            self.default_factory = None
        else:
            if not (args[0] is None or callable(args[0])):
                raise TypeError('first argument must be callable or None')
            self.default_factory = args[0]
            args = args[1:]
        super(OrderedDefaultdict, self).__init__(*args, **kwargs)

    def __missing__(self, key):
        if self.default_factory is None:
            raise KeyError(key)
        self[key] = default = self.default_factory()
        return default

    def __reduce__(self):  # optional, for pickle support
        args = (self.default_factory,) if self.default_factory else ()
        return self.__class__, args, None, None, self.iteritems()


def merge_td_columns(rows, num_to_merge=None, vertical_align=False):
    """
    Merge HTML TD columns with identical values using rowspan.

    Arguments:
    rows -- a list of tuples, one tuple per row, containing n elements for the
            n columns.
    num_to_merge -- the number of columns to merge, starting from the left
                    hand column. Leave as None to merge all columns.
    vertical_align -- Set to True to vertically centre any merged cells.

    Output:
    A list of strings, one string per row, containing TD elements.
    """
    transposed = zip(*rows)
    if num_to_merge is None:
        num_to_merge = len(transposed)
    valign = ' style="vertical-align:middle;"' if vertical_align else ''

    new_cols = []
    for col_idx, col in enumerate(transposed):
        if col_idx > num_to_merge - 1:
            new_cols.append(['<td>%s</td>' % v for v in col])
            continue

        merged = []
        start = 0
        while start < len(col):
            l = col[start:]
            same_vals = list(itertools.takewhile(lambda x: x == col[start], l))
            rowspan = len(same_vals)
            start += rowspan

            if rowspan > 1:
                new_td = ['<td rowspan="%s"%s>%s</td>' % (rowspan,
                                                          valign,
                                                          same_vals[0])]
                blanks = [''] * (rowspan - 1)
                merged.extend(new_td + blanks)
            else:
                td = '<td>%s</td>' % (same_vals[0])
                merged.append(td)

        new_cols.append(merged)

    return zip(*new_cols)


def get_vis_from_plots(plots):
    """
    Get the name to be used for the MS from the given plots.

    :param plots:
    :return:
    """
    vis = {p.parameters['vis'] for p in plots}
    vis = vis.pop() if len(vis) is 1 else 'all data'
    return vis


def total_time_on_target_on_source(ms, autocorr_only=False):
    """
    Return the nominal total time on target source for the given MeasurementSet
    excluding OFF-source integrations (REFERENCE). The flag is not taken into account.

    Background of development: ALMA-TP observations have integrations of both TARGET
    and REFERENCE intents in one scan. Scan.time_on_source does not return appropriate
    exposure time in the case.

    :param ms: MeasurementSet domain object to examine
    :param autocorr_only:
    :return: a datetime.timedelta object set to the total time on source
    """
    science_spws = ms.get_spectral_windows(science_windows_only=True)
    state_ids = [s.id for s in ms.states if 'TARGET' in s.intents]
    max_time = 0.0
    ant_ids = [a.id for a in ms.antennas]
    dds = [ms.get_data_description(spw=spw) for spw in science_spws]
    science_dds = numpy.unique([dd.id for dd in dds])
    with casatools.TableReader(ms.name) as tb:
        for dd in science_dds:
            for a1 in ant_ids:
                for a2 in ant_ids:
                    if autocorr_only and a1 != a2:
                        continue
                    seltb = tb.query('DATA_DESC_ID == %d AND ANTENNA1 == %d AND ANTENNA2 == %d AND STATE_ID IN %s' % (
                        dd, a1, a2, state_ids))
                    try:
                        if seltb.nrows() == 0:
                            continue
                        target_exposures = seltb.getcol('EXPOSURE').sum()
                        LOG.debug(
                            "Selected %d ON-source rows for DD=%d, Ant1=%d, Ant2=%d: total exposure time = %f sec" % (
                                seltb.nrows(), dd, a1, a2, target_exposures))
                        max_time = max(max_time, target_exposures)
                    finally:
                        seltb.close()
    LOG.debug("Max ON-source exposure time = %f sec" % max_time)
    return datetime.timedelta(int(max_time / 86400), int(max_time % 86400), int((max_time % 1) * 1e6))


def total_time_on_source(scans):
    """
    Return the total time on source for the given Scans.

    :param scans: collection of Scan domain objects
    :return: a datetime.timedelta object set to the total time on source
    """
    times_on_source = [scan.time_on_source for scan in scans]
    if times_on_source:
        return reduce(operator.add, times_on_source)
    else:
        # could potentially be zero matching scans, such as when the
        # measurement set is missing scans with science intent
        return datetime.timedelta(0)


def get_logrecords(result, loglevel):
    """
    Get the logrecords for the result, removing any duplicates

    :param result: a result containing logrecords
    :param loglevel: the loglevel to match
    :return:
    """
    try:
        # WeakProxy is registered as an Iterable (and a Container, Hashable, etc.)
        # so we can't check for isinstance(result, collections.Iterable)
        # see https://bugs.python.org/issue24067
        _ = iter(result)
    except TypeError:
        if not hasattr(result, 'logrecords'):
            return []
        records = [l for l in result.logrecords if l.levelno is loglevel]
    else:
        # note that flatten returns a generator, which empties after
        # traversal. we convert to a list to allow multiple traversals
        g = flatten([get_logrecords(r, loglevel) for r in result])
        records = list(g)

    # append the message target to the LogRecord so we can link to the
    # matching page in the web log
    try:
        target = os.path.basename(result.inputs['vis'])
        for r in records:
            r.target = {'vis': target}
    except:
        pass

    dset = set()
    # relies on the fact that dset.add() always returns None.
    return [r for r in records if
            r.msg not in dset and not dset.add(r.msg)]


def get_intervals(context, calapp, spw_ids=None):
    """
    Get the integration intervals for scans processed by a calibration.

    The scan and spw selection is formed through inspection of the
    CalApplication representing the calibration.

    :param context: the pipeline context
    :param calapp: the CalApplication representing the calibration
    :param spw_ids: a set of spw IDs to get intervals for. Leave as None to
        use all spws specified in the CalApplication.
    :return: a list of datetime objects representing the unique scan intervals
    """
    # With the advent of session calibrations, the target MS for the
    # calibration may be different from the MS used to calculate the
    # calibration. Therefore, we must look to the calapp.origin, which
    # refers to the originating calls, to calculate the true values.
    vis = {o.inputs['vis'] for o in calapp.origin}
    assert (len(vis) == 1)
    vis = vis.pop()

    from_intent = {o.inputs['intent'] for o in calapp.origin}
    assert (len(from_intent) == 1)
    from_intent = from_intent.pop()

    # let CASA parse spw arg in case it contains channel spec
    if not spw_ids:
        task_spw_args = {o.inputs['spw'] for o in calapp.origin}
        spw_arg = ','.join(task_spw_args)
        spw_ids = {spw_id for (spw_id, _, _, _) in spw_arg_to_id(vis, spw_arg)}

    # from_intent is given in CASA intents, ie. *AMPLI*, *PHASE*
    # etc. We need this in pipeline intents.
    ms = context.observing_run.get_ms(vis)
    pipeline_intent = to_pipeline_intent(ms, from_intent)
    scans = ms.get_scans(scan_intent=pipeline_intent)

    # scan with intent may not have data for the spws used in the
    # gaincal call, eg. X20fb. Only get the solint for spws in the call
    # by using the intersection.
    all_solints = {scan.mean_interval(spw_id)
                   for scan in scans
                   for spw_id in spw_ids.intersection({spw.id for spw in scan.spws})}

    return all_solints

    # ms = context.observing_run.get_ms(calapp.vis)
    #
    # from_intent = calapp.origin.inputs['intent']
    # # from_intent is given in CASA intents, ie. *AMPLI*, *PHASE*
    # # etc. We need this in pipeline intents.
    # pipeline_intent = to_pipeline_intent(ms, from_intent)
    # scans = ms.get_scans(scan_intent=pipeline_intent)
    #
    # # let CASA parse spw arg in case it contains channel spec
    # if not spw_ids:
    #     spw_ids = set([spw_id for (spw_id, _, _, _)
    #                    in spw_arg_to_id(calapp.vis, calapp.spw)])
    #
    # all_solints = set()
    # for scan in scans:
    #     scan_spw_ids = set([spw.id for spw in scan.spws])
    #     # scan with intent may not have data for the spws used in
    #     # the gaincal call, eg. X20fb, so only get the solint for
    #     # the intersection
    #     solints = [scan.mean_interval(spw_id)
    #                for spw_id in spw_ids.intersection(scan_spw_ids)]
    #     all_solints.update(set(solints))
    #
    # return all_solints
