import abc
import copy_reg
import os
try:
    import cPickle as pickle
except:
    import pickle
import types

from mpi4casa.MPICommandClient import MPICommandClient
from mpi4casa.MPIEnvironment import MPIEnvironment

from . import api
from . import logging

# global variable for toggling MPI usage
USE_MPI = True

LOG = logging.get_logger(__name__)


class PipelineError(Exception):
    """
    Warning! This class is here temporarily and could disappear at any time!

    References to this class should be replaced with references to the real
    Pipeline exception classes, once that module has been created.
    """
    pass


class AsyncTask(object):
    def __init__(self, executable):
        """
        Create a new AsyncTask.

        The referenced task will be immediately queued for asynchronous
        execution on an MPI server upon creation of an object.

        :param task_cls: the class of a pipeline task
        :param task_args: a dict of arguments to be given to the task Inputs
        :param context_path: the path to the pickled context
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
        :except PipelineError: if the task did not complete successfully.
        """
        response = mpiclient.get_command_response(self.__pid,
                                                  block=True,
                                                  verbose=True)
        response = response[0]
        if response['successful']:
            return response['ret']
        else:
            # TODO add traceback to exception
            raise PipelineError('Failure executing job')


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
        :except PipelineError: if the task did not complete successfully.
        """
        try:
            if self.__executor:
                return self.__executor.execute(self.__task)
            else:
                return self.__task.execute(dry_run=False)
        except Exception:
            # TODO add exception to error
            raise PipelineError('Failure executing job')


class Tier1Executable(object):
    @abc.abstractmethod
    def get_executable(self):
        """
        Recreate and return the executable object. The executable object
        should have an .execute() function.
        """
        raise NotImplementedError


class Tier0PipelineTask(Tier1Executable):
    def __init__(self, task_cls, task_args, context_path):
        """
        Create a new Tier0PipelineTask representing a pipeline task to be
        executed on an MPI server.

        :param task_cls: the class of the pipeline task to execute
        :param task_args: any arguments to passed to the task Inputs
        :param context_path: the filesystem path to the pickled Context
        """
        self.__task_cls = task_cls
        self.__task_args = task_args
        self.__context_path = context_path

    def get_executable(self):
        with open(self.__context_path, 'rb') as context_file:
            context = pickle.load(context_file)
        inputs = self.__task_cls.Inputs(context, **self.__task_args)
        task = self.__task_cls(inputs)
        return task

    def __str__(self):
        return 'Tier0PipelineTask(%s, %s, %s)' % (self.__task_cls,
                                                  self.__task_args,
                                                  self.__context_path)


class Tier0CASATask(Tier1Executable):
    def __init__(self, task_cls, task_args):
        """
        Create a new Tier0CASATask representing a JobRequest CASA task to be
        executed on an MPI server.

        :param task_cls: the class of the CASA task to execute
        :param task_args: any arguments to passed to the task Inputs
        """
        self.__task_cls = task_cls
        self.__task_args = task_args

    def get_executable(self):
        return self.__task_cls(**self.__task_args)

    def __str__(self):
        return 'Tier0CASATask(%s, %s)' % (self.__task_cls, self.__task_args)


def mpiexec(tier0_executable):
    """
    Execute a pipeline task.

    This function is used to recreate and execute tasks on cluster nodes.

    :param executable: the Tier0Executable task to execute
    :return: the Result returned by executing the task
    """
    LOG.trace('rank%s@%s: mpiexec(%s)', MPIEnvironment.mpi_processor_rank,
              MPIEnvironment.hostname, tier0_executable)

    executable = tier0_executable.get_executable()
    LOG.info('Executing %s on rank%s@%s', tier0_executable,
             MPIEnvironment.mpi_processor_rank, MPIEnvironment.hostname)
    return executable.execute(dry_run=False)


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


def _pickle_method(method):
    func_name = method.im_func.__name__
    obj = method.im_self
    cls = method.im_class
    return _unpickle_method, (func_name, obj, cls)


def _unpickle_method(func_name, obj, cls):
    for cls in cls.mro():
        try:
            func = cls.__dict__[func_name]
        except KeyError:
            pass
        else:
            break
    return func.__get__(obj, cls)

copy_reg.pickle(types.MethodType, _pickle_method, _unpickle_method)


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
