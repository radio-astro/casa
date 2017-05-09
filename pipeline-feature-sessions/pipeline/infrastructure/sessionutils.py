from __future__ import absolute_import
import abc
import collections
import itertools
import os
import tempfile

from pipeline.infrastructure import basetask
from . import mpihelpers
from . import utils
from . import vdp

__all__ = [
    'as_list'
    'group_into_sessions',
    'parallel_inputs_impl',
    'ParallelTemplate'
    'remap_spw_int',
    'remap_spw_str',
    'VDPTaskFactory',
    'VisResultTuple'
]

# VisResultTuple is a data structure used by VDPTaskFactor to group
# inputs and results.
VisResultTuple = collections.namedtuple('VisResultTuple', 'vis inputs result')


def parallel_inputs_impl():
    """
    Get a vis-independent property implementation for a parallel
    Inputs argument.

    :return: Inputs property implementation
    :rtype: property
    """

    def fget(self):
        return self._parallel

    def fset(self, value):
        if value is None:
            value = 'automatic'
        else:
            allowed = ('true', 'false', 'automatic', True, False)
            if value not in allowed:
                m = ', '.join(('{!r}'.format(i) for i in allowed))
                raise ValueError('Value not in allowed value set ({!s}): {!r}'.format(m, value))
        self._parallel = value

    return property(fget, fset)


def as_list(o):
    return o if isinstance(o, list) else [o]


def group_into_sessions(context, all_results):
    """
    Return results grouped into lists by session.

    :param context: pipeline context
    :type context: Context
    :param all_results: result to be grouped
    :type all_results: list
    :return: dict of sessions to results for that session
    :rtype: dict {session name: [result, result, ...]
    """
    session_map = {ms.basename: ms.session
                   for ms in context.observing_run.measurement_sets}

    ms_start_times = {ms.basename: utils.get_epoch_as_datetime(ms.start_time)
                      for ms in context.observing_run.measurement_sets}

    def get_session(r):
        basename = os.path.basename(r[0])
        return session_map.get(basename, 'Shared')

    def get_start_time(r):
        basename = os.path.basename(r[0])
        return ms_start_times.get(basename, None)

    results_by_session = sorted(all_results, key=get_session)
    return {session_id: sorted(results_for_session, key=get_start_time)
            for session_id, results_for_session in itertools.groupby(results_by_session, get_session)}


class VDPTaskFactory(object):
    """
    VDPTaskFactory is a class that implements the Factory design
    pattern, returning tasks that execute on an MPI client or locally
    as appropriate.

    The correctness of this task is dependent on the correct mapping of
    Inputs arguments to measurement set, hence it is dependent on
    Inputs objects that sub-class VDP StandardInputs.
    """

    def __init__(self, inputs, executor, task):
        """
        Create a new VDPTaskFactory.

        :param inputs: inputs for the task
        :type inputs: class that extends vdp.StandardInputs
        :param executor: pipeline task executor
        :type executor: basetask.Executor
        :param task: task to execute
        """
        self.__inputs = inputs
        self.__context = inputs.context
        self.__executor = executor
        self.__task = task
        self.__context_path = None

    def __enter__(self):
        # If there's a possibility that we'll submit MPI jobs, save the context
        # to disk ready for import by the MPI servers.
        if mpihelpers.mpiclient:
            # Use the tempfile module to generate a unique temporary filename,
            # which we use as the output path for our pickled context
            tmpfile = tempfile.NamedTemporaryFile(suffix='.context',
                                                  dir=self.__context.output_dir,
                                                  delete=True)
            self.__context_path = tmpfile.name
            tmpfile.close()

            self.__context.save(self.__context_path)

        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.__context_path and os.path.exists(self.__context_path):
            os.unlink(self.__context_path)

    def get_task(self, vis):
        """
        Create and return a SyncTask or AsyncTask for the job.

        :param vis: measurement set to create a job for
        :type vis: str
        :return: task object ready for execution
        :rtype: a tuple of (task arguments, (SyncTask|AsyncTask)
        """
        # get the task arguments for the targetted MS
        task_args = self.__get_task_args(vis)

        # Validate the task arguments against the signature expected by
        # the task. This is necessary as self.__inputs could be a
        # different type from the Inputs instance associated with the
        # task, and hence and hence have a different signature. For
        # instance, the session-aware Inputs classes accept a
        # 'parallel' argument, which the non-session tasks do not.
        #
        # To make things complicated, ModeInputs are a special case.
        # Here we shouldn't inspect the constructor signature of the
        # task as it doesn't reflect that of the active task, which is
        # the one that we would want to filter on. So, if the task's
        # Inputs are ModeInputs, dereference the task's Inputs class.
        if issubclass(self.__task.Inputs, vdp.ModeInputs):
            active_mode_task = self.__task.Inputs._modes[self.__inputs.mode]
            task_inputs_cls = active_mode_task.Inputs
        else:
            task_inputs_cls = self.__task.Inputs
        valid_args = validate_args(task_inputs_cls, task_args)

        is_mpi_ready = mpihelpers.is_mpi_ready()
        is_tier0_job = is_mpi_ready

        parallel_wanted = mpihelpers.parse_mpi_input_parameter(self.__inputs.parallel)

        if is_tier0_job and parallel_wanted:
            executable = mpihelpers.Tier0PipelineTask(self.__task, valid_args, self.__context_path)
            return valid_args, mpihelpers.AsyncTask(executable)
        else:
            inputs = self.__task.Inputs(self.__context, **valid_args)
            task = self.__task(inputs)
            return valid_args, mpihelpers.SyncTask(task, self.__executor)

    def __get_task_args(self, vis):
        inputs = self.__inputs

        original_vis = inputs.vis
        try:
            inputs.vis = vis
            task_args = inputs.as_dict()
        finally:
            inputs.vis = original_vis

        return task_args


def remove_unexpected_args(fn, fn_args):
    # get the argument names for the function
    code = fn.func_code
    arg_count = code.co_argcount
    arg_names = code.co_varnames[:arg_count]

    # identify arguments that are not expected by the function
    unexpected = [k for k in fn_args if k not in arg_names]

    # return the fn args purged of any unexpected items
    x = {k:v for k, v in fn_args.iteritems() if k not in unexpected}

    # LOG.info('Arg names: {!s}'.format(arg_names))
    # LOG.info('Unexpected: {!s}'.format(unexpected))
    # LOG.info('Valid: {!s}'.format(x))

    return x


def validate_args(inputs_cls, task_args):
    inputs_constructor_fn = inputs_cls.__init__.__func__
    valid_args = remove_unexpected_args(inputs_constructor_fn, task_args)
    return valid_args


def get_spwmap(source_ms, target_ms):
    """
    Get a map of spectral windows IDs that map from a source spw ID in
    the source MS to its equivalent spw in the target MS.

    :param source_ms: the MS to map spws from
    :type source_ms: domain.MeasurementSet
    :param target_ms: the MS to map spws to
    :type target_ms: domain.MeasurementSet
    :param spws: the spw argument to convert
    :return: dict of integer spw IDs
    """
    # spw names are not guaranteed to be unique. They seem to be unique
    # across science intents, but they could be repeated for other
    # scans (pointing, sideband, etc.) in non-science spectral windows.
    # if not eliminated, these non-science window names collide with
    # the science windows and you end up having science windows map to
    # non-science windows and vice versa. Not what we want! This set
    # will be used to filter for the spectral windows we want to
    # consider.
    science_intents = {'AMPLITUDE', 'BANDPASS', 'PHASE', 'TARGET', 'CHECK'}

    # map spw id to spw name for source MS - just for science intents
    id_to_name = {spw.id: spw.name
                  for spw in source_ms.spectral_windows
                  if not science_intents.isdisjoint(spw.intents)}

    # map spw name to spw id for target MS - just for science intents
    name_to_id = {spw.name: spw.id
                  for spw in target_ms.spectral_windows
                  if not science_intents.isdisjoint(spw.intents)}

    # note the get(v, k) here. This says that for non-science windows,
    # which have been filtered from the maps, use the original spw ID -
    # hence non-science spws are not remapped.
    return {k: name_to_id.get(v, k)
            for k, v in id_to_name.iteritems()
            if v in name_to_id}


def remap_spw_int(source_ms, target_ms, spws):
    """
    Map integer spw arguments from one MS to their equivalent spw in
    the target ms.

    :param source_ms: the MS to map spws from
    :type source_ms: domain.MeasurementSet
    :param target_ms: the MS to map spws to
    :type target_ms: domain.MeasurementSet
    :param spws: the spw argument to convert
    :return: a list of remapped integer spw IDs
    :rtype: list
    """
    int_spw_map = get_spwmap(source_ms, target_ms)
    return [int_spw_map[spw_id] for spw_id in spws]


def remap_spw_str(source_ms, target_ms, spws):
    """
    Remap a string spw argument, e.g., '16,18,20,22', from one MS to
    the equivalent map in the target ms.

    :param source_ms: the MS to map spws from
    :type source_ms: domain.MeasurementSet
    :param target_ms: the MS to map spws to
    :type target_ms: domain.MeasurementSet
    :param spws: the spw argument to convert
    :return: a list of remapped integer spw IDs
    :rtype: str
    """
    spw_ints = [int(i) for i in spws.split(',')]
    l = remap_spw_int(source_ms, target_ms, spw_ints)
    return ','.join([str(i) for i in l])


class ParallelTemplate(basetask.StandardTaskTemplate):
    @abc.abstractproperty
    def Task(self):
        """
        A reference to the :class:`Task` class containing the implementation
        for this pipeline stage.
        """
        raise NotImplementedError

    def __init__(self, inputs):
        super(ParallelTemplate, self).__init__(inputs)

    def is_multi_vis_task(self):
        return True

    def get_result_for_exception(self, vis, result):
        raise NotImplementedError

    def prepare(self):
        inputs = self.inputs

        # this will hold the tuples of ms, jobs and results
        assessed = []

        vis_list = as_list(inputs.vis)
        with VDPTaskFactory(inputs, self._executor, self.Task) as factory:
            task_queue = [(vis, factory.get_task(vis)) for vis in vis_list]

            # Jobs must complete within the scope of the VDPTaskFactory as the
            # context copies used by the MPI clients are removed on __exit__.
            for (vis, (task_args, task)) in task_queue:
                try:
                    worker_result = task.get_result()
                except mpihelpers.PipelineError as e:
                    assessed.append((vis, task_args, e))
                else:
                    assessed.append((vis, task_args, worker_result))

        return assessed

    def analyse(self, assessed):
        # all results will be added to this object
        final_result = basetask.ResultsList()

        context = self.inputs.context
        session_groups = group_into_sessions(context, assessed)
        for session_id, session_results in session_groups.iteritems():
            for vis, task_args, vis_result in session_results:
                if isinstance(vis_result, Exception):
                    fake_result = self.get_result_for_exception(vis, vis_result)
                    fake_result.inputs = task_args
                    final_result.append(fake_result)

                else:
                    if isinstance(vis_result, collections.Iterable):
                        final_result.extend(vis_result)
                    else:
                        final_result.append(vis_result)

        return final_result
