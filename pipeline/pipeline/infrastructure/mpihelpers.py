import os
try:
    import cPickle as pickle
except:
    import pickle

from mpi4casa.MPICommandClient import MPICommandClient
from mpi4casa.MPIEnvironment import MPIEnvironment

from . import api
from . import logging

# global variable for toggling MPI usage
USE_MPI = True

LOG = logging.get_logger(__name__)


class PipelineError(Exception):
    pass


class AsyncTask(object):
    def __init__(self, task_cls, task_args, context_path=None):
        """
        Create a new AsyncTask.

        The referenced task will be immediately queued for asynchronous
        execution on an MPI server upon creation of an object.

        :param task_cls: the class of a pipeline task
        :param task_args: a dict of arguments to be given to the task Inputs
        :param context_path: the path to the pickled context
        :return: an AsyncTask object
        """
        job_args = {'task_cls': task_cls,
                    'task_args': task_args,
                    'context_path': context_path}

        self.__pid = mpiclient.push_command_request(
                'pipeline.infrastructure.mpihelpers.mpiexec(task_cls, task_args, context_path)',
                block=False,
                parameters=job_args)

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

def mpiexec(task_cls=None, task_args=None, context_path=None):
    """
    Create and execute a pipeline task.

    This function is used to recreate and execute pipeline tasks on cluster
    nodes.

    :param task_cls: the class of the pipeline task to execute
    :param task_args: any arguments to passed to the task Inputs
    :param context_path: the filesystem path to the pickled Context
    :return: the Result returned by executing the task
    """
    LOG.debug('%s@%s: mpiexec(%s, %s, %r)', MPIEnvironment.mpi_processor_rank,
              MPIEnvironment.hostname, task_cls, task_args, context_path)
    if issubclass(task_cls, api.Task):
        with open(context_path, 'rb') as context_file:
            context = pickle.load(context_file)
        inputs = task_cls.Inputs(context, **task_args)
        task = task_cls(inputs)
    else:
        # assuming this is a CASA task call, which is created without any
        # reference to the context
        task = task_cls(**task_args)

    return task.execute(dry_run=False)

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
