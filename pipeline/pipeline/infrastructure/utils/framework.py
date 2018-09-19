"""
The framework module contains:

 1. utility functions used by the pipeline framework;
 2. utility functions used by pipeline tasks to help process framework
    objects (Results, CalLibrary objects, etc.).
"""
import collections
import copy
import errno
import glob
import inspect
import itertools
import operator
import os
import types
import uuid

from mpi4casa.MPIEnvironment import MPIEnvironment

from .conversion import flatten, safe_split
from .. import jobrequest
from .. import logging
from .. import mpihelpers

try:
    import cPickle as pickle
except ImportError:
    import pickle
try:
    import cStringIO as StringIO
except ImportError:
    import StringIO


LOG = logging.get_logger(__name__)

__all__ = ['task_depth', 'is_top_level_task', 'get_calfroms', 'pickle_copy', 'pickle_load', 'gen_hash',
           'collect_properties', 'mkdir_p', 'get_tracebacks', 'get_qascores', 'merge_jobs', 'get_origin_input_arg',
           'contains_single_dish', 'plotms_iterate']


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
    if all((MPIEnvironment.is_mpi_enabled,  # running on MPI cluster
            not MPIEnvironment.is_mpi_client)):  # running as MPI server
        return False

    return task_depth() is 1


def get_calfroms(context, vis, caltypes=None):
    """
    Get the CalFroms of the requested type from the callibrary.
    """
    # TODO remove circular imports. This function can live in the callibrary module itself.
    from .. import callibrary
    if caltypes is None:
        caltypes = callibrary.CalFrom.CALTYPES.keys()

    # check that the
    if type(caltypes) is types.StringType:
        caltypes = (caltypes,)

    for c in caltypes:
        assert c in callibrary.CalFrom.CALTYPES

    # get the CalState for the ms - no field/spw/antenna selection (for now..)
    calto = callibrary.CalTo(vis=vis)
    calstate = context.callibrary.get_calstate(calto)

    try:
        # old dict-based callibrary implementation
        calfroms = (itertools.chain(*calstate.merged().values()))
        return [cf for cf in calfroms if cf.caltype in caltypes]
    except AttributeError:
        # it's a new IntervalTree-based callibrary
        return [calfrom for _, calfroms in calstate.merged()
                for calfrom in calfroms
                if calfrom.caltype in caltypes]


def pickle_copy(original):
    stream = StringIO.StringIO()
    pickle.dump(original, stream, -1)
    # rewind to the start of the 'file', allowing it to be read in its
    # entirety - otherwise we get an EOFError
    stream.seek(0)
    return pickle_load(stream)


def pickle_load(fileobj):
    return pickle.load(fileobj)


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


def collect_properties(instance, ignore=None):
    """
    Return the public properties of an object as a dictionary
    """
    if ignore is None:
        ignore = []
    skip = ['context', 'ms']
    skip.extend(ignore)
    properties = {}
    for dd_name, dd in inspect.getmembers(instance.__class__, inspect.isdatadescriptor):
        if dd_name.startswith('_') or dd_name in skip:
            continue
        # Hidden VDP properties should not be included
        if getattr(dd, 'hidden', False) is True:
            continue
        try:
            properties[dd_name] = getattr(instance, dd_name)
            # properties[dd_name] = dd.fget(instance)
        except TypeError:
            # TODO can we always use this instead of the data descriptor fget?
            properties[dd_name] = getattr(instance, dd_name)
        except AttributeError:
            LOG.debug('Could not get input property: {!s}.{!s}'.format(instance.__class__, dd_name))

    return properties


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


def get_tracebacks(result):
    """
    Get the tracebacks for the result, from any failures that may have
    occurred during the task.

    :param result: a result or result list.
    :return: list of tracebacks as strings.
    """
    if isinstance(result, collections.Iterable):
        tracebacks = [get_tracebacks(r) for r in result]
    else:
        tracebacks = [getattr(result, "tb", [])]
    return list(flatten(tracebacks))


def get_qascores(result, lo=None, hi=None):
    if isinstance(result, collections.Iterable):
        scores = flatten([get_qascores(r, lo, hi) for r in result])
    else:
        scores = [s for s in result.qa.pool if s.score not in ('', 'N/A', None)]

    if lo is None and hi is None:
        matches = lambda score: True
    elif lo is not None and hi is None:
        matches = lambda score: s.score > lo
    elif lo is None and hi is not None:
        matches = lambda score: s.score <= hi
    else:
        matches = lambda score: lo < s.score <= hi

    return [s for s in scores if matches(s)]


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


def flatten_dict(d, join=operator.add, lift=lambda x: (x,)):
    flag_first = object()

    def visit(subdict, results, partial_key):
        for k, v in subdict.iteritems():
            new_key = lift(k) if partial_key is flag_first else join(partial_key, lift(k))
            if isinstance(v, collections.Mapping):
                visit(v, results, new_key)
            else:
                results.append((new_key, v))

    results = []
    visit(d, results, flag_first)
    return results


def get_origin_input_arg(calapp, attr):
    """
    Get a single-valued task input argument from a CalApp.

    If more than one value is present, for instance, asking for solint
    when the originating jobs have different solint arguments, an
    assertion error will be raised.

    :param calapp: CalApplication to inspect
    :type calapp: callibrary.CalApplication
    :param attr: name of input argument
    :type attr: str
    :return:
    """
    values = {o.inputs[attr] for o in calapp.origin}
    assert (len(values) == 1)
    return values.pop()


def contains_single_dish(context):
    """
    Return True if the context contains single-dish data.

    :param context: the pipeline context
    :return: True if SD data is present
    """
    return any([hasattr(context.observing_run, 'ms_reduction_group'),
                hasattr(context.observing_run, 'ms_datatable_name')])


def plotms_iterate(jobs_and_wrappers, iteraxis=None):
    # CAS-11220: Some Pipeline Plots Do Not Contain Spw Number
    # fix: replace job, adding iteraxis='spw' so that spw is always in title
    if (  # iteraxis must be spw,...
            iteraxis == 'spw' and
            # .. only add for single jobs that wouldn't otherwise use iteraxis...
            len(jobs_and_wrappers) == 1 and
            # .. when we're not plotting basebands, i.e., comma-separated spws
            ',' not in jobs_and_wrappers[0][0].kw.get(iteraxis, '')):
        jobs_and_wrappers = [(jobrequest.JobRequest(job.fn, *job.args, iteraxis='spw', **job.kw), wrappers)
                             for job, wrappers in jobs_and_wrappers]

    # component jobs containing a comma should be executed as they are. An
    # example situation is calling plotms for all spws in a baseband. Here,
    # the multiple spw argument should be left as-is and iteration left
    # disabled.
    mergeable = [j for j, _ in jobs_and_wrappers if ',' not in j.kw.get(iteraxis, '')]
    non_mergeable = [j for j, _ in jobs_and_wrappers if j not in mergeable]

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
        # holds final name of png after any move operation
        dest_filenames = [job.kw['plotfile'] for job in component_jobs]
        # holds expected filenames after plotms has done any filename mangling
        src_filenames = []

        # If there's only one job, queue the original job for execution
        if len(component_jobs) is 1:
            job_to_execute = component_jobs[0]

        else:
            # set the final kwargs for the iteraxis-enabled job
            final_kwargs = dict(merged_job.kw)
            final_kwargs['iteraxis'] = iteraxis
            final_kwargs['clearplots'] = True
            final_kwargs['overwrite'] = True
            final_kwargs['exprange'] = 'all'

            # generate random filename to make it easier to identify when things go wrong
            iter_filename = '%s.png' % uuid.uuid4()
            final_kwargs['plotfile'] = iter_filename

            job_to_execute = casa_tasks.plotms(**final_kwargs)

            LOG.trace('Component jobs:\n%s\nReplacement job:\n%s',
                      '\n'.join([str(j) for j in component_jobs]),
                      job_to_execute)

        # plotms iteraxis mode appends an iteration index to the filenames.
        # This code calculates what the output filenames would be so that we
        # can look for them on disk
        if 'iteraxis' in job_to_execute.kw:
            # calculate the iteration-specific part of the suffix. Filename
            # components are only inserted if there is more than one plot
            # is generated by the plotms iterator.
            iter_indexes = ['_%s' % (n + 1) for n in range(len(component_jobs))]
            iter_indexes[0] = ''

            root, ext = os.path.splitext(job_to_execute.kw['plotfile'])

            if iteraxis == 'antenna':
                src_filenames.extend(['%s_Antenna%s@*%s%s' % (root, job.kw['antenna'], idx, ext)
                                      for idx, job in zip(iter_indexes, component_jobs)])
            elif iteraxis == 'spw':
                src_filenames.extend(['%s_Spw%s%s%s' % (root, job.kw['spw'], idx, ext)
                                      for idx, job in zip(iter_indexes, component_jobs)])
            else:
                raise NotImplementedError('Plotms mapping not known for iteraxis={!s}'.format(iteraxis))

        else:
            # iteraxis is not enabled, so output should be written to the
            # filename we provide without any modification by plotms
            src_filenames.append(job_to_execute.kw['plotfile'])

        # execute merged job if any of the output files are missing
        if not all([os.path.exists(dest) for dest in dest_filenames]):
            # Cycle 6 fallback: revert to serial plotting until CAS-11660,
            # CAS-11578, etc. are fixed.
            tier0_plots_enabled = 'ENABLE_TIER0_PLOTMS' in os.environ or mpihelpers.ENABLE_TIER0_PLOTMS
            if tier0_plots_enabled and mpihelpers.is_mpi_ready():
                executable = mpihelpers.Tier0JobRequest(casa_tasks.plotms, job_to_execute.kw)
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
                for src, dst, cjob in zip(src_filenames, dest_filenames, component_jobs):
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
    wrappers = [w for _, w in jobs_and_wrappers]
    return filter(lambda w: os.path.exists(w.abspath), wrappers)
