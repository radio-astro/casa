from __future__ import absolute_import
import collections
import copy
import datetime
import decimal
import errno
import glob
import inspect
import itertools
import math
import operator
import os

try:
    import cPickle as pickle
except:
    import pickle
import platform
import re
import string
try:
    import cStringIO as StringIO
except:
    import StringIO
import types
import uuid
import numpy as np

import cachetools

# for is_top_level_task
from mpi4casa.MPIEnvironment import MPIEnvironment

import pipeline.extern.pyparsing as pyparsing
import pipeline.extern.ps_mem as ps_mem
from . import casatools
from . import logging
from . import mpihelpers
from . import pipelineqa

LOG = logging.get_logger(__name__)


def commafy(l, quotes=True, multi_prefix='', separator=', ', conjunction='and'):
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
            return '%s\'%s\'%s%s' % (multi_prefix, l[0], separator, commafy(l[1:], separator=separator, quotes=quotes, conjunction=conjunction))
        else:
            return '%s%s%s%s' % (multi_prefix, l[0], separator, commafy(l[1:], separator=separator, quotes=quotes, conjunction=conjunction))


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
    for _, g in itertools.groupby(enumerate(s), lambda (i, x):i - x):
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


def total_time_on_source(scans):
    '''
    Return the total time on source for the given Scans.

    scans -- a collection of Scan domain objects
    return -- a datetime.timedelta object set to the total time on source
    '''
    times_on_source = [scan.time_on_source for scan in scans]
    if times_on_source:
        return reduce(operator.add, times_on_source)
    else:
        # could potentially be zero matching scans, such as when the
        # measurement set is missing scans with science intent
        return datetime.timedelta(0)


def format_datetime(dt, dp=0):
    '''
    Return a formatted string representation for the given datetime
    '''
    if dp > 6:
        raise ValueError('Cannot exceed 6 decimal places as datetime stores '
                         'to microsecond precision')

    s = dt.strftime('%Y-%m-%d %H:%M:%S')    
    if dp <= 0:
        # Ignore microseconds
        return s

    microsecs = dt.microsecond / 1e6
    f = '{0:.%sf}' % dp
    return s + f.format(microsecs)[1:]


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


def to_pipeline_intent(ms, intent):
    """
    Convert CASA intents back into obs modes, then into a pipeline intent.
    """
    casa_intents = set([i.strip('*') for i in intent.split(',') if i is not None])

    state = ms.states[0]

    pipeline_intents = set()
    for casa_intent in casa_intents:
        matching = [pipeline_intent for mode, pipeline_intent in state.obs_mode_mapping.items()
                    if casa_intent in mode]
        pipeline_intents.update(set(matching))

    return ','.join(pipeline_intents)


def stringSplitByNumbers(x):
    r = re.compile('(\d+)')
    l = r.split(x)
    return [int(y) if y.isdigit() else y for y in l]


def numericSort(l):
    """
    Sort a list numerically, eg.

    ['9,11,13,15', '11,13', '9'] -> ['9', '9,11,13,15', '11,13']
    """
    return sorted(l, key=stringSplitByNumbers)


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
    import pipeline.infrastructure.jobrequest as jobrequest
    def get_hook_fn(msg):
        pid = os.getpid()

        def log_mem_usage(jobrequest):
            sorted_cmds, shareds, _, _ = ps_mem.get_memory_usage([pid, ], False, True)
            for cmd in sorted_cmds:
                private = measures.FileSize(cmd[1] - shareds[cmd[0]],
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

    try:
        # old dict-based callibrary implementation
        calfroms = (itertools.chain(*calstate.merged().values()))
        return [cf for cf in calfroms if cf.caltype in caltypes]
    except AttributeError:
        # it's a new IntervalTree-based callibrary
        return [calfrom for _, calfroms in calstate.merged()
                for calfrom in calfroms
                if calfrom.caltype in caltypes]


def areEqual(a, b):
    """
    Return True if the contents of the given arrays are equal.
    """
    return len(a) == len(b) and len(a) == sum([1 for i, j in zip(a, b) if i == j])


def pickle_copy(original):
    stream = StringIO.StringIO()
    pickle.dump(original, stream, -1)
    # rewind to the start of the 'file', allowing it to be read in its
    # entirety - otherwise we get an EOFError
    stream.seek(0)
    return pickle_load(stream)


def pickle_load(fileobj):
    unpickled = pickle.load(fileobj)
    if hasattr(unpickled, 'reconnect_weakrefs'):
        unpickled.reconnect_weakrefs()
    return unpickled


def gen_hash(o):
    """
    Makes a hash from a dictionary, list, tuple or set to any level, that
    contains only other hashable types (including any lists, tuples, sets,
    and dictionaries).
    """
    LOG.trace('gen_hash(%s)' % str(o))
    if isinstance(o, set) or isinstance(o, tuple) or isinstance(o, list):
        return tuple([gen_hash(e) for e in o])

    elif not isinstance(o, dict):
        h = hash(o)
        LOG.trace('Hash: %s=%s' % (o, h))
        return hash(o)

    new_o = copy.deepcopy(o)
    for k, v in new_o.items():
        new_o[k] = gen_hash(v)

    return hash(tuple(frozenset(new_o.items())))


def range_to_list(arg):
    if arg == '':
        return []

    # recognise but suppress the mode-switching tokens
    TILDE = pyparsing.Suppress('~')

    # recognise '123' as a number, converting to an integer
    number = pyparsing.Word(pyparsing.nums).setParseAction(lambda tokens : int(tokens[0]))

    # convert '1~10' to a range
    rangeExpr = number('start') + TILDE + number('end')
    rangeExpr.setParseAction(lambda tokens : range(tokens.start, tokens.end + 1))

    casa_chars = ''.join([c for c in string.printable
                          if c not in ',;"/' + string.whitespace])
    textExpr = pyparsing.Word(casa_chars)

    # numbers can be expressed as ranges or single numbers
    atomExpr = rangeExpr | number | textExpr

    # we can have multiple items separated by commas
    atoms = pyparsing.delimitedList(atomExpr, delim=',')('atoms')

    return list(atoms.parseString(str(arg)))


def collect_properties(instance, ignore=[]):
    """
    Return the public properties of an object as a dictionary
    """
    skip = ['context', 'ms']
    skip.extend(ignore)
    properties = {}
    for dd_name, dd in inspect.getmembers(instance.__class__, inspect.isdatadescriptor):
        if dd_name.startswith('_') or dd_name in skip:
            continue
        try:
            properties[dd_name] = dd.fget(instance)
        except:
            LOG.debug('Could not get input property %s' % dd_name)
    return properties


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


def mkdir_p(path):
    """
    Emulate mkdir -p functionality.
    """
    try:
        os.makedirs(path)
    except OSError as exc:
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise


def task_depth():
    """
    Get the number of executing tasks currently on the stack. If the depth is
    1, the calling function is the top-level task.
    """
    stack = [frame_obj for (frame_obj, _, _, _, _, _) in inspect.stack()
             if frame_obj.f_code.co_name == 'execute'
             and frame_obj.f_code.co_filename.endswith('pipeline/infrastructure/basetask.py')]
    stack_count = len(stack)
    return stack_count


def is_top_level_task():
    """
    Return True if the callee if executing as part of a top-level task.
    """
    # If this code is executed on an MPI server, it must have been invoked
    # from a sub-task running on an MPI server, which itself must have been
    # called from a pipeline task running on the MPI client. In this case, we
    # know this is not a top-level task without examining the stack.
    if all((MPIEnvironment.is_mpi_enabled,       # running on MPI cluster
           not MPIEnvironment.is_mpi_client)):   # running as MPI server
        return False

    return task_depth() is  1


def get_logrecords(result, loglevel):
    """
    Get the logrecords for the result, removing any duplicates

    :param result: a result containing logrecords
    :param loglevel: the loglevel to match
    :return:
    """
    if isinstance(result, list):
        # note that flatten returns a generator, which empties after
        # traversal. we convert to a list to allow multiple traversals
        g = flatten([get_logrecords(r, loglevel) for r in result])
        records = list(g)
    else:
        records = [l for l in result.logrecords if l.levelno is loglevel]

    # append the message target to the LogRecord so we can link to the
    # matching page in the web log
    try: 
        target = os.path.basename(result.inputs['vis'])
        for r in records:
            r.target = {'vis':target}
    except:
        pass

    dset = set()
    # relies on the fact that dset.add() always returns None.
    return [r for r in records if
            r.msg not in dset and not dset.add(r.msg)]


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
    taql = {arg_name : str(arg_val)}
    with casatools.MSReader(ms_path) as ms:
        ms.msselect(taql, onlyparse=True)
        return ms.msselectedindices()


def get_qascores(result, lo=None, hi=None):
    if isinstance(result, list):
        scores = flatten([get_qascores(r, lo, hi) for r in result])
    else:
        scores = [s for s in result.qa.pool
                  if s.score not in ('', 'N/A', None)]
    
    if lo is None and hi is None:
        matches = lambda score: True
    elif lo is not None and hi is None:
        matches = lambda score: s.score > lo
    elif lo is None and hi is not None:
        matches = lambda score: s.score <= hi
    else:
        matches = lambda score: s.score > lo and s.score <= hi
      
    return [s for s in scores if matches(s)]


def natural_sort(l): 
    convert = lambda text: int(text) if text.isdigit() else text.lower() 
    alphanum_key = lambda key: [convert(c) for c in re.split('([0-9]+)', key)] 
    return sorted(l, key = alphanum_key)


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

    def __missing__ (self, key):
        if self.default_factory is None:
            raise KeyError(key)
        self[key] = default = self.default_factory()
        return default

    def __reduce__(self):  # optional, for pickle support
        args = (self.default_factory,) if self.default_factory else ()
        return self.__class__, args, None, None, self.iteritems()


def get_intervals(context, calapp, spw_ids=None):
    """
    Get the integration intervals for scans processed by a calibration.
    
    The scan and spw selection is formed through inspection of the 
    CalApplication representing the calibration. 

    :param ms: the MeasurementSet domain object
    :param calapp: the CalApplication representing the calibration
    :param spw_ids: a set of spw IDs to get intervals for. Leave as None to
        use all spws specified in the CalApplication.
    :return: a list of datetime objects representing the unique scan intervals 
    """
    ms = context.observing_run.get_ms(calapp.vis)

    from_intent = calapp.origin.inputs['intent']
    # from_intent is given in CASA intents, ie. *AMPLI*, *PHASE*
    # etc. We need this in pipeline intents.
    pipeline_intent = to_pipeline_intent(ms, from_intent)
    scans = ms.get_scans(scan_intent=pipeline_intent)

    # let CASA parse spw arg in case it contains channel spec
    if not spw_ids:
        spw_ids = set([spw_id for (spw_id, _, _, _) 
                       in spw_arg_to_id(calapp.vis, calapp.spw)])
    
    all_solints = set()
    for scan in scans:
        scan_spw_ids = set([spw.id for spw in scan.spws])
        # scan with intent may not have data for the spws used in
        # the gaincal call, eg. X20fb, so only get the solint for 
        # the intersection
        solints = [scan.mean_interval(spw_id) 
                   for spw_id in spw_ids.intersection(scan_spw_ids)]
        all_solints.update(set(solints))
    
    return all_solints


def merge_jobs(jobs, task, merge=None, ignore=None):
    """
    Merge jobs that are identical apart from the arguments named in
    ignore. These jobs will be recreated with  

    Identical tasks are identified by creating a hash of the dictionary
    of task keyword arguments, ignoring keywords specified in the 
    'ignore' argument. Jobs with the same hash can be merged; this is done
    by appending the spw argument of job X to the spw argument of memoed
    job Y, whereafter job X can be discarded.  

    :param jobs: the job requests to merge
    :type jobs: a list of\ 
        :class:`~pipeline.infrastructure.jobrequest.JobRequest`
    :param task: the CASA task to recreate
    :type task: a reference to a function on \
        :class:`~pipeline.infrastructure.jobrequest.casa_tasks'
    :param ignore: the task arguments to ignore during hash creation
    :type ignore: an iterable containing strings

    :rtype: a list of \
        :class:`~pipeline.infrastructure.jobrequest.JobRequest`
    """
    if merge is None:
        merge = ()
    if ignore is None:
        ignore = ()

    # union holds the property names to ignore while calculating the job hash
    union = frozenset(itertools.chain.from_iterable((merge, ignore)))

    # mapping of job hash to merged job 
    merged_jobs = collections.OrderedDict()
    # mapping of job hash to all the jobs that were merged to create it
    component_jobs = collections.OrderedDict()

    for job in jobs:
        job_hash = job.hash_code(ignore=union)
        # either first job or a unique job, so add to dicts..
        if job_hash not in merged_jobs:
            merged_jobs[job_hash] = job
            component_jobs[job_hash] = [job]
            continue

        # .. otherwise this job looks identical to one we have already. Merge
        # this job's arguments with those of the job we've already got.
        hashed_job_args = merged_jobs[job_hash].kw
        new_job_args = dict(hashed_job_args)
        for prop in merge:
            if job.kw[prop] not in safe_split(hashed_job_args[prop]):
                merged_value = ','.join((hashed_job_args[prop], job.kw[prop]))
                new_job_args[prop] = merged_value
            merged_jobs[job_hash] = task(**new_job_args)

        # add this to the record of jobs we merged
        component_jobs[job_hash].append(job)

    return zip(merged_jobs.values(), component_jobs.values())


def plotms_iterate(jobs_and_wrappers, iteraxis):
    # component jobs containing a comma should be executed as they are. An
    # example situation is calling plotms for all spws in a baseband. Here,
    # the multiple spw argument should be left as-is and iteration left
    # disabled.
    mergeable = [j for j,_ in jobs_and_wrappers if ',' not in j.kw[iteraxis]]
    non_mergeable = [j for j,_ in jobs_and_wrappers if j not in mergeable]

    from pipeline.infrastructure import casa_tasks
    merged_results = merge_jobs(mergeable,
                                casa_tasks.plotms,
                                merge=(iteraxis,),
                                ignore=('plotfile',))

    # add the unmergable jobs back into the merged results structure so we can
    # treat all jobs the same
    merged_results.extend([(j, [j]) for j in non_mergeable])

    jobs_and_callbacks = []
    for merged_job, component_jobs in merged_results:
        # filename components are only inserted if there is more than one plot
        # generated by the iterator. If there's only one job, queue the
        # original job for execution rather than creating a replacement with
        # iteration enabled.

        if len(component_jobs) is 1:
            job_to_execute = component_jobs[0]
            src_filenames = [job_to_execute.kw['plotfile']]
        else:
            # generate random filename to make it easier to identify when things go wrong
            iter_filename = '%s.png' % uuid.uuid4()
            root, ext = os.path.splitext(iter_filename)

            # activate plotms iteration in the merged job arguments to activate
            merged_job.kw['plotfile'] = iter_filename
            merged_job.kw['clearplots'] = True
            merged_job.kw['overwrite'] = True
            merged_job.kw['exprange'] = 'all'
            job_to_execute = casa_tasks.plotms(iteraxis=iteraxis,
                                               **merged_job.kw)
            LOG.trace('Component jobs:\n%s\nReplacement job:\n%s',
                      '\n'.join([str(j) for j in component_jobs]),
                      job_to_execute)

            iter_indexes = ['_%s' % (n+1) for n in range(len(component_jobs))]
            iter_indexes[0] = ''

            if iteraxis == 'antenna':
                src_filenames = ['%s_Antenna%s@*%s%s' % (root, job.kw['antenna'], idx, ext)
                                 for idx, job in zip(iter_indexes, component_jobs)]
            elif iteraxis == 'spw':
                src_filenames = ['%s_Spw%s%s%s' % (root, job.kw['spw'], idx, ext)
                                 for idx, job in zip(iter_indexes, component_jobs)]
            else:
                raise NotImplementedError('Plotms mapping not known for '
                                          'iteraxis=%s' % iteraxis)

        dest_filenames = [job.kw['plotfile'] for job in component_jobs]

        # execute merged job if some of the output files are missing
        if not all([os.path.exists(dest) for dest in dest_filenames]):
            if mpihelpers.is_mpi_ready():
                executable = mpihelpers.Tier0CASATask(casa_tasks.plotms,
                                                      job_to_execute.kw)
                queued_job = mpihelpers.AsyncTask(executable)
            else:
                queued_job = mpihelpers.SyncTask(job_to_execute)

            # variables within functions and lambdas are late binding, so we
            # supply them as default arguments to get the values at function
            # definition time into the closure scope
            def callback(src_filenames=src_filenames,
                         dest_filenames=dest_filenames,
                         component_jobs=component_jobs):
                # move the plotms output into place, renaming to the expected
                # filename containing ant, spw, field components.
                for src, dst, cjob in zip(src_filenames, dest_filenames,
                                          component_jobs):
                    matching_files = glob.glob(src)
                    if len(matching_files) is 1:
                        os.rename(matching_files[0], dst)
                    else:
                        LOG.info('%s not found. plotms iterator did not '
                                 'generate any output for equivalent of %s',
                                 src, cjob)

            jobs_and_callbacks.append((queued_job, callback))
        else:
            LOG.trace('Skipping unnecessary job: %s' % job_to_execute)

    # now execute all the callbacks, which will rename the output files
    LOG.info('Compressed %s plotms jobs to %s jobs',
             len(jobs_and_wrappers), len(jobs_and_callbacks))
    for (queued_job, callback) in jobs_and_callbacks:
        queued_job.get_result()
        callback()

    # at this point, the sequentially-named plots from the merged job have
    # been renamed match that of the unmerged job, so we can simply check
    # whether the plot (with the original filename) exists or not.
    wrappers = [w for _,w in jobs_and_wrappers]
    return filter(lambda w: os.path.exists(w.abspath), wrappers)


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
        if col_idx > num_to_merge-1:
            new_cols.append(['<td>%s</td>' % v for v in col])
            continue
            
        merged = []
        start = 0
        while start < len(col):
            l = col[start:]
            same_vals = list(itertools.takewhile(lambda x: x==col[start], l))
            rowspan = len(same_vals)
            start += rowspan
            
            if rowspan > 1:
                new_td = ['<td rowspan="%s"%s>%s</td>' % (rowspan, 
                                                          valign,
                                                          same_vals[0])]
                blanks = [''] * (rowspan-1)
                merged.extend(new_td + blanks)
            else:
                td = '<td>%s</td>' % (same_vals[0])
                merged.append(td)
            
        new_cols.append(merged)
    
    return zip(*new_cols)


def get_cube_freq_axis(img):

    '''Get image cube frequency axis.'''

    iaTool = casatools.image

    # Get frequency axis
    iaTool.open(img)
    imInfo = iaTool.summary()
    iaTool.close()

    fIndex = imInfo['axisnames'].tolist().index('Frequency')
    refFreq = imInfo['refval'][fIndex]
    deltaFreq = imInfo['incr'][fIndex]
    freqUnit = imInfo['axisunits'][fIndex]
    refPix = imInfo['refpix'][fIndex]
    numPix = imInfo['shape'][fIndex]

    return refFreq, deltaFreq, freqUnit, refPix, numPix


def selection_to_frequencies(img, selection, unit='GHz'):

    '''Convert channel selection to frequency tuples.'''

    frequencies = []
    if (selection != ''):
        qaTool = casatools.quanta


        # Get frequency axis
        try:
            refFreq, deltaFreq, freqUnit, refPix, numPix = get_cube_freq_axis(img)
        except:
            LOG.error('No frequency axis found in %s.' % (img))
            return ['NONE']

        for crange in selection.split(';'):
            c0, c1 = map(float, crange.split('~'))
            f0 = qaTool.convert({'value': refFreq + (c0 - refPix) * deltaFreq, 'unit': freqUnit}, unit)
            f1 = qaTool.convert({'value': refFreq + (c1 - refPix) * deltaFreq, 'unit': freqUnit}, unit)
            if (qaTool.lt(f0, f1)):
                #print '%.9f~%.9f%s' % (f0['value'], f1['value'], unit)
                frequencies.append((f0['value'], f1['value']))
            else:
                #print '%.9f~%.9f%s' % (f1['value'], f0['value'], unit)
                frequencies.append((f1['value'], f0['value']))
    else:
        frequencies = ['NONE']
        #print 'None'

    return frequencies


def get_vis_from_plots(plots):
    """
    Get the name to be used for the MS from the given plots.

    :param plots:
    :return:
    """
    vis = {p.parameters['vis'] for p in plots}
    vis = vis.pop() if len(vis) is 1 else 'all data'
    return vis


def merge_ranges(ranges):

    """
    Merge overlapping and adjacent ranges and yield the merged ranges
    in order. The argument must be an iterable of pairs (start, stop).

    >>> list(merge_ranges([(5,7), (3,5), (-1,3)]))
    [(-1, 7)]
    >>> list(merge_ranges([(5,6), (3,4), (1,2)]))
    [(1, 2), (3, 4), (5, 6)]
    >>> list(merge_ranges([]))
    []

    (c) Gareth Rees 02/2013

    """
    ranges = iter(sorted(ranges))
    current_start, current_stop = next(ranges)
    for start, stop in ranges:
        if start > current_stop:
            # Gap between segments: output current segment and start a new one.
            yield current_start, current_stop
            current_start, current_stop = start, stop
        else:
            # Segments adjacent or overlapping: merge.
            current_stop = max(current_stop, stop)
    yield current_start, current_stop


def spw_intersect(spw_range, line_regions):

    """
    Compute intersect between SPW frequency range and line frequency
    ranges to be excluded.
    """

    spw_sel_intervals = []
    for line_region in line_regions:
        if (line_region[0] <= spw_range[0]) and (line_region[1] >= spw_range[1]):
            spw_sel_intervals = []
            spw_range = []
            break
        elif (line_region[0] <= spw_range[0]) and (line_region[1] >= spw_range[0]):
            spw_range = [line_region[1], spw_range[1]]
        elif (line_region[0] >= spw_range[0]) and (line_region[1] < spw_range[1]):
            spw_sel_intervals.append([spw_range[0], line_region[0]])
            spw_range = [line_region[1], spw_range[1]]
        elif (line_region[0] >= spw_range[1]):
            spw_sel_intervals.append(spw_range)
            spw_range = []
            break
        elif (line_region[0] >= spw_range[0]) and (line_region[1] >= spw_range[1]):
            spw_sel_intervals.append([spw_range[0], line_region[0]])
            spw_range = []
            break
    if spw_range != []:
        spw_sel_intervals.append(spw_range)

    return spw_sel_intervals


def flatten_dict(d, join=operator.add, lift=lambda x: (x,)):
    FLAG_FIRST = object()

    def visit(subdict, results, partial_key):
        for k, v in subdict.iteritems():
            new_key = lift(k) if partial_key is FLAG_FIRST else join(partial_key, lift(k))
            if isinstance(v, collections.Mapping):
                visit(v, results, new_key)
            else:
                results.append((new_key, v))

    results = []
    visit(d, results, FLAG_FIRST)
    return results


def flagged_intervals(vec):
    '''
    Find islands of non-zeros in the vector vec
    Used to find contiguous flagged channels in a given spw.  Returns a list of tuples with the start and end channels.
    '''
    if len(vec)==0:
        return []
    elif not isinstance(vec, np.ndarray):
        vec = np.array(vec)

    edges, = np.nonzero(np.diff((vec==True)*1))
    edge_vec = [edges+1]
    if vec[0] != 0:
        edge_vec.insert(0, [0])
    if vec[-1] != 0:
        edge_vec.append([len(vec)])
    edges = np.concatenate(edge_vec)
    return zip(edges[::2], edges[1::2]-1)


def dequote(s):
    '''Remove any kind of quotes from a string to faciliate comparisons.'''

    return s.replace('"','').replace("'","")
