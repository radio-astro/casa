import abc
import os

try:
    import cPickle as pickle
except:
    import pickle
import tempfile

from mpi4casa.MPICommandClient import MPICommandClient
from mpi4casa.MPIEnvironment import MPIEnvironment

from pipeline.infrastructure import exceptions
from pipeline.infrastructure import logging

# global variable for toggling MPI usage
USE_MPI = True

LOG = logging.get_logger(__name__)


class AsyncTask(object):
    def __init__(self, executable):
        """
        Create a new AsyncTask.

        The referenced task will be immediately queued for asynchronous
        execution on an MPI server upon creation of an object.

        :param executable: the TierN executable class to run
        :return: an AsyncTask object
        """
        self.__pid = mpiclient.push_command_request(
            'pipeline.infrastructure.mpihelpers.mpiexec(tier0_executable)',
            block=False,
            parameters={'tier0_executable': executable})

    def get_result(self):
        """
        Get the result from the executed task.

        This method blocks until execution of the asynchronous task is
        complete.

        :return: the Result returned by the executing task
        :rtype: pipeline.infrastructure.api.Result
        :except PipelineException: if the task did not complete successfully.
        """
        response = mpiclient.get_command_response(self.__pid,
                                                  block=True,
                                                  verbose=True)
        response = response[0]
        if response['successful']:
            return response['ret']
        else:
            err_msg = "Failure executing job on MPI server {}, " \
                      "with traceback\n {}".format(response['server'], response['traceback'])
            raise exceptions.PipelineException(err_msg)


class SyncTask(object):
    def __init__(self, task, executor=None):
        """
        Create a new SyncTask object.

        Creation of a SyncTask does not result in immediate execution of the
        given task. Execution is delayed until the result is requested.

        :param task: a pipeline task or JobRequest
        :param executor: a pipeline Executor (optional)
        :return: a SyncTask object
        :rtype: SyncTask
        """
        self.__task = task
        self.__executor = executor

    def get_result(self):
        """
        Get the result from the executed task.

        This method starts execution of the wrapped task and blocks until
        execution is complete.

        :return: the Result returned by the executing task
        :rtype: pipeline.infrastructure.api.Result
        :except pipeline.infrastructure.exceptions.PipelineException: if the
        task did not complete successfully.
        """
        try:
            if self.__executor:
                return self.__executor.execute(self.__task)
            else:
                return self.__task.execute(dry_run=False)
        except Exception, e:
            import traceback
            err_msg = "Failure executing job by an exception {} " \
                      "with the following traceback\n {}".format(e.__class__.__name__, traceback.format_exc())
            raise exceptions.PipelineException(err_msg)


class Executable(object):
    @abc.abstractmethod
    def get_executable(self):
        """
        Recreate and return the executable object. The executable object
        should have an .execute() function.
        """
        raise NotImplementedError


class Tier0PipelineTask(Executable):
    def __init__(self, task_cls, task_args, context_path):
        """
        Create a new Tier0PipelineTask representing a pipeline task to be
        executed on an MPI server.

        :param task_cls: the class of the pipeline task to execute
        :param task_args: any arguments to passed to the task Inputs
        :param context_path: the filesystem path to the pickled Context
        """
        self.__task_cls = task_cls
        self.__context_path = context_path

        # Assume that the path to the context pickle is safe to write the task
        # argument pickle too
        context_dir = os.path.dirname(context_path)
        # Use the tempfile module to generate a unique temporary filename,
        # which we use as the output path for our pickled context
        tmpfile = tempfile.NamedTemporaryFile(suffix='.task_args', dir=context_dir, delete=True)
        self.__task_args_path = tmpfile.name
        tmpfile.close()

        # write task args object to pickle
        with open(self.__task_args_path, 'wb') as pickle_file:
            LOG.info('Saving task arguments to {!s}'.format(self.__task_args_path))
            pickle.dump(task_args, pickle_file, protocol=-1)

    def get_executable(self):
        # the constructor runs on the MPI server, this runs on the MPI client
        try:
            with open(self.__context_path, 'rb') as context_file:
                context = pickle.load(context_file)

            with open(self.__task_args_path, 'rb') as task_args_file:
                task_args = pickle.load(task_args_file)

            inputs = self.__task_cls.Inputs(context, **task_args)
            task = self.__task_cls(inputs)

            return lambda: task.execute(dry_run=False)

        finally:
            if self.__task_args_path and os.path.exists(self.__task_args_path):
                os.unlink(self.__task_args_path)

    def __str__(self):
        return 'Tier0PipelineTask(%s, %s, %s)' % (self.__task_cls,
                                                  self.__task_args_path,
                                                  self.__context_path)


class Tier0JobRequest(Executable):
    def __init__(self, creator_fn, job_args):
        """
        Create a new Tier0JobRequest representing a JobRequest to be executed
        on an MPI server.

        :param creator_fn: the class of the CASA task to execute
        :param job_args: any arguments to passed to the task Inputs
        """
        self.__creator_fn = creator_fn
        self.__job_args = job_args

    def get_executable(self):
        job_request = self.__creator_fn(**self.__job_args)
        return lambda: job_request.execute(dry_run=False)

    def __str__(self):
        return 'Tier0JobRequest({}, {})'.format(self.__creator_fn, self.__job_args)


class Tier0FunctionCall(object):
    def __init__(self, fn, *args, **kwargs):
        """
        Create a new Tier0FunctionCall for a function to be executed on an MPI
        server.
        """
        self.__fn = fn
        self.__args = args
        self.__kwargs = kwargs

        # the following code is used to get a nice repr format
        code = fn.func_code
        arg_count = code.co_argcount
        arg_names = code.co_varnames[:arg_count]

        def format_arg_value(arg_val):
            arg, val = arg_val
            return '%s=%r' % (arg, val)

        self.__positional = map(format_arg_value, zip(arg_names, args))
        self.__nameless = map(repr, args[arg_count:])
        self.__keyword = map(format_arg_value, kwargs.items())

    def get_executable(self):
        return lambda: self.__fn(*self.__args, **self.__kwargs)

    def __repr__(self):
        args = self.__positional + self.__nameless + self.__keyword
        args.insert(0, self.__fn.__name__)
        return 'Tier0FunctionCall({!s})'.format(', '.join(args))


def mpiexec(tier0_executable):
    """
    Execute a pipeline task.

    This function is used to recreate and execute tasks on cluster nodes.

    :param tier0_executable: the Tier0Executable task to execute
    :return: the Result returned by executing the task
    """
    LOG.trace('rank%s@%s: mpiexec(%s)', MPIEnvironment.mpi_processor_rank,
              MPIEnvironment.hostname, tier0_executable)

    executable = tier0_executable.get_executable()
    LOG.info('Executing %s on rank%s@%s', tier0_executable,
             MPIEnvironment.mpi_processor_rank, MPIEnvironment.hostname)
    return executable()


def is_mpi_ready():
    # to allow MPI jobs, the executing pipeline code must be running as the
    # client node on an MPI cluster, with the MPICommandClient ready and the
    # no user override specified.
    return all([USE_MPI,  # User has not disabled MPI globally
                MPIEnvironment.is_mpi_enabled,  # running on MPI cluster
                MPIEnvironment.is_mpi_client,  # running as MPI client
                mpiclient])  # MPICommandClient ready


def _splitall(path):
    allparts = []
    while 1:
        parts = os.path.split(path)
        if parts[0] == path:  # sentinel for absolute paths
            allparts.insert(0, parts[0])
            break
        elif parts[1] == path:  # sentinel for relative paths
            allparts.insert(0, parts[1])
            break
        else:
            path = parts[0]
            allparts.insert(0, parts[1])
    return allparts


def parse_mpi_input_parameter(input_arg):
    lowercase = str(input_arg).lower()
    if lowercase == 'automatic':
        return is_mpi_ready()
    elif lowercase == 'true':
        return True
    elif lowercase == 'false':
        return False
    else:
        raise ValueError('Arg must be one of true, false or automatic. Got %s' % input_arg)


mpiclient = None
mpi_server_list = None

if MPIEnvironment.is_mpi_enabled:
    try:
        if MPIEnvironment.is_mpi_client:
            __client = MPICommandClient()
            __client.start_services()

            mpi_server_list = MPIEnvironment.mpi_server_rank_list()

            # get path to pipeline code and import it on the cluster nodes
            __client.push_command_request('import sys', block=True, target_server=mpi_server_list)
            __codepath = os.path.join(*_splitall(__file__)[0:-3])
            __client.push_command_request('sys.path.insert(0, %r)' % __codepath, block=True, target_server=mpi_server_list)

            # LOG.warning('Executing tclean_pg import on MPI servers to work around import bug in CASA stable')
            # __client.push_command_request('from tclean_pg import tclean_pg as tclean', block=True, target_server=mpi_server_list)

            __client.push_command_request('import pipeline', block=True, target_server=mpi_server_list)
            # __client.push_command_request('pipeline.infrastructure.logging.set_logging_level(level="trace")', block=True, target_server=mpi_server_list)

            mpiclient = __client
            LOG.info('MPI environment detected. Pipeline operating in cluster'
                     ' mode.')
    except:
        LOG.warning('Problem initialising MPI. Pipeline falling back to single'
                    ' host mode.')
        mpiclient = None
else:
    LOG.info('Environment is not MPI enabled. Pipeline operating in single '
             'host mode')
    mpiclient = None
