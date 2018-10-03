"""
The conversion module contains utility functions that convert between data
types and assist in formatting objects as strings for presentation to the
user.
"""
import collections
import datetime
import decimal
import math
import string

import cachetools
import pyparsing

from .. import casatools
from .. import logging
from .. import pipelineqa

LOG = logging.get_logger(__name__)

__all__ = ['commafy', 'flatten', 'mjd_seconds_to_datetime', 'get_epoch_as_datetime', 'range_to_list', 'to_CASA_intent',
           'to_pipeline_intent', 'field_arg_to_id', 'spw_arg_to_id', 'ant_arg_to_id', 'safe_split', 'dequote',
           'format_datetime', 'format_timedelta']


def commafy(l, quotes=True, multi_prefix='', separator=', ', conjunction='and'):
    """
    Return the textual description of the given list.

    For example: commafy(['a','b','c']) = "'a', 'b' and 'c'"

    :param l:
    :param quotes:
    :param multi_prefix:
    :param separator:
    :param conjunction:
    :return:
    """
    if not isinstance(l, list) and isinstance(l, collections.Iterable):
        l = [i for i in l]

    # turn 's' into 's '
    if multi_prefix:
        multi_prefix += ' '

    length = len(l)
    if length is 0:
        return ''
    if length is 1:
        if multi_prefix:
            prefix = ' '
        else:
            prefix = ''

        if quotes:
            return '%s\'%s\'' % (prefix, l[0])
        else:
            return '%s%s' % (prefix, l[0])
    if length is 2:
        if quotes:
            return '%s\'%s\' %s \'%s\'' % (multi_prefix, l[0], conjunction, l[1])
        else:
            return '%s%s %s %s' % (multi_prefix, l[0], conjunction, l[1])
    else:
        if quotes:
            return '%s\'%s\'%s%s' % (
                multi_prefix, l[0], separator,
                commafy(l[1:], separator=separator, quotes=quotes, conjunction=conjunction))
        else:
            return '%s%s%s%s' % (
                multi_prefix, l[0], separator,
                commafy(l[1:], separator=separator, quotes=quotes, conjunction=conjunction))


def flatten(l):
    """
    Flatten a list of lists into a single list
    """
    for el in l:
        if isinstance(el, collections.Iterable) and not isinstance(el, (basestring, pipelineqa.QAScore)):
            for sub in flatten(el):
                yield sub
        else:
            yield el


def unix_seconds_to_datetime(unix_secs):
    """
    Return the input list, specified in seconds elapsed since 1970-01-01,
    converted to the equivalent Python datetimes.

    If given a list, a list is returned. If given a scalar, a scalar is
    returned.
    """
    datetimes = [datetime.datetime.utcfromtimestamp(s) for s in unix_secs]
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


def get_epoch_as_datetime(epoch):
    """
    Convert a CASA epoch measure into a Python datetime.

    :param epoch: CASA epoch to convert
    :return: equivalent Python datetime
    """
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


def range_to_list(arg):
    """
    Expand a numeric range expressed in CASA syntax to the equivalent Python
    list of integers.

    Example:
    >>> range_to_list('1~5,7~9')
    [1,2,3,4,5,7,8,9]

    :param arg: CASA range to expand
    :type arg: str
    :return: list of integers
    """
    if arg == '':
        return []

    # recognise but suppress the mode-switching tokens
    TILDE = pyparsing.Suppress('~')

    # recognise '123' as a number, converting to an integer
    number = pyparsing.Word(pyparsing.nums).setParseAction(lambda tokens: int(tokens[0]))

    # convert '1~10' to a range
    rangeExpr = number('start') + TILDE + number('end')
    rangeExpr.setParseAction(lambda tokens: range(tokens.start, tokens.end + 1))

    casa_chars = ''.join([c for c in string.printable
                          if c not in ',;"/' + string.whitespace])
    textExpr = pyparsing.Word(casa_chars)

    # numbers can be expressed as ranges or single numbers
    atomExpr = rangeExpr | number | textExpr

    # we can have multiple items separated by commas
    atoms = pyparsing.delimitedList(atomExpr, delim=',')('atoms')

    return list(atoms.parseString(str(arg)))


def to_CASA_intent(ms, intents):
    """
    Convert pipeline intents back to the equivalent intents recorded in the
    measurement set.

    Example:
    >>> to_CASA_intent(ms, 'PHASE,BANDPASS')
    'CALIBRATE_PHASE_ON_SOURCE,CALIBRATE_BANDPASS_ON_SOURCE'

    :param ms: MeasurementSet object
    :param intents: pipeline intents to convert
    :return: CASA intents
    """
    obs_modes = ms.get_original_intent(intents)
    return ','.join(obs_modes)


def to_pipeline_intent(ms, intents):
    """
    Convert CASA intents to pipeline intents.

    :param ms: MeasurementSet object
    :param intents: CASA intents to convert
    :return: pipeline intents
    """
    casa_intents = {i.strip('*') for i in intents.split(',') if i is not None}

    state = ms.states[0]

    pipeline_intents = {pipeline_intent for casa_intent in casa_intents
                        for obsmode, pipeline_intent in state.obs_mode_mapping.iteritems()
                        if casa_intent in obsmode}

    return ','.join(pipeline_intents)


def field_arg_to_id(ms_path, field_arg):
    """
    Convert a string to the corresponding field IDs.

    :param ms_path: the path to the measurement set
    :param field_arg: the field selection in CASA format
    :return: a list of field IDs
    """
    try:
        all_indices = _convert_arg_to_id('field', ms_path, str(field_arg))
        return all_indices['field'].tolist()
    except RuntimeError:
        # SCOPS-1666
        # msselect throws exceptions with numeric field names beginning with
        # zero. Try again, encapsulating the argument in quotes.
        quoted_arg = '"%s"' % str(field_arg)
        all_indices = _convert_arg_to_id('field', ms_path, quoted_arg)
        return all_indices['field'].tolist()


def spw_arg_to_id(ms_path, spw_arg):
    """
    Convert a string to spectral window IDs and channels.

    :param ms_path: the path to the measurement set
    :param spw_arg: the spw selection in CASA format
    :return: a list of (spw, chan_start, chan_end, step) lists
    """
    all_indices = _convert_arg_to_id('spw', ms_path, str(spw_arg))
    # filter out channel tuples whose spw is not in the spw entry
    return [(spw, start, end, step)
            for (spw, start, end, step) in all_indices['channel']
            if spw in all_indices['spw']]


def ant_arg_to_id(ms_path, ant_arg):
    """
    Convert a string to the corresponding antenna IDs.

    :param ms_path: the path to the measurement set
    :param ant_arg: the antenna selection in CASA format
    :return: a list of antenna IDs
    """
    all_indices = _convert_arg_to_id('baseline', ms_path, str(ant_arg))
    return all_indices['antenna1'].tolist()


@cachetools.lru_cache(maxsize=1000)
def _convert_arg_to_id(arg_name, ms_path, arg_val):
    """
    Parse the CASA input argument and return the matching IDs.

    :param arg_name:
    :param ms_path: the path to the measurement set
    :param field_arg: the field argument formatted with CASA syntax.
    :return: a set of field IDs
    """
    # due to memoized decorator we'll only see this log message when a new
    # msselect is executed
    LOG.trace('Executing msselect({%r:%r} on %s', arg_name, arg_val, ms_path)
    taql = {arg_name: str(arg_val)}
    with casatools.MSReader(ms_path) as ms:
        ms.msselect(taql, onlyparse=True)
        return ms.msselectedindices()


def safe_split(fields):
    """
    Split a string containing field names into a list, taking account of
    field names within quotes.

    :param fields:
    :return:
    """
    return pyparsing.commaSeparatedList.parseString(str(fields))


def dequote(s):
    """
    Remove any kind of quotes from a string to faciliate comparisons.

    :param s:
    :return:
    """
    return s.replace('"', '').replace("'", "")


def format_datetime(dt, dp=0):
    """
    Return a string representation of a Python datetime, including microseconds
    to the requested precision.

    :param dt: Python datetime to convert
    :param dp: number of decimal places for microseconds (0=do not show)
    :return: string date
    """
    if dp > 6:
        raise ValueError('Cannot exceed 6 decimal places as datetime stores to microsecond precision')

    s = dt.strftime('%Y-%m-%d %H:%M:%S')
    if dp <= 0:
        # Ignore microseconds
        return s

    microsecs = dt.microsecond / 1e6
    f = '{0:.%sf}' % dp
    return s + f.format(microsecs)[1:]


def format_timedelta(td, dp=0):
    """
    Return a formatted string representation for the given timedelta

    :param td:
    :param dp:
    :return:
    """
    secs = decimal.Decimal(td.seconds)
    microsecs = decimal.Decimal(td.microseconds) / decimal.Decimal('1e6')
    rounded_secs = (secs + microsecs).quantize(decimal.Decimal(10) ** -dp)
    rounded = datetime.timedelta(days=td.days, seconds=math.floor(rounded_secs))
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
