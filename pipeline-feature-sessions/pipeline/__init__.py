from __future__ import absolute_import
import imp
import os
import pkg_resources
import webbrowser

from . import environment
from . import infrastructure

from . import h
from . import hco
from . import hif
from . import hsd
from . import hifv
from . import hifa

from .infrastructure import Pipeline, Context

from .domain import measures
from casa_stack_manip import stack_frame_find

LOG = infrastructure.get_logger(__name__)

__pipeline_documentation_weblink_alma__ = "http://almascience.org/documents-and-tools/pipeline-documentation-archive"

# create a fake module containing all the tasks defined in 


def _all_subclasses(cls):
    """
    Return a list of all subclasses that inherit directly or indirectly from
    the given class.
    """
    return cls.__subclasses__() + [g for s in cls.__subclasses__()
                                    for g in _all_subclasses(s)]


def _get_unified_task_module(packages):
    """
    Create a new module containing all tasks in the given packages.
    """
    module = imp.new_module('pipeline.tasks')

    task_classes = _all_subclasses(infrastructure.api.Task)
    for pkg in packages:
        tasks = dict((k, v) for k, v in pkg.__dict__.items()
                     if v in task_classes)
        for k, v in tasks.items():
            LOG.trace('Importing %s from %s' % (k, pkg.__name__))
            module.__dict__[k] = v
    return module

tasks = _get_unified_task_module([h.tasks, hif.tasks, hco.tasks, hsd.tasks, hifv.tasks, hifa.tasks])


def show_weblog(context):
    if context is None:
        return

    index_html = os.path.join(context.report_dir, 't1-1.html')
    webbrowser.open(index_html)


def initcli():
    LOG.info('Initializing cli...')
    my_globals = stack_frame_find()
    for package in ['h', 'hif', 'hifa', 'hifv', 'hsd']:
        abs_package = 'pipeline.{package}.cli.{package}'.format(package=package)
        try:
            # buildmytasks writes output to packagename.py
            path_to_cli = pkg_resources.resource_filename(abs_package, '{!s}.py'.format(package))
        except ImportError as e:
            LOG.exception('Import error: {!s}'.format(e))
            LOG.info('No tasks found for package: {!s}'.format(package))
        else:
            execfile(path_to_cli, my_globals)
            LOG.info('Loaded CASA tasks from package: {!s}'.format(package))


# def initcli() :
#     print "Initializing cli..."
#     mypath = pkg_resources.resource_filename(__name__, '')
#     hifpath = mypath+"/hif/cli/hif.py"
#     hpath = mypath+"/h/cli/h.py"
#     hsdpath = mypath+"/hsd/cli/hsd.py"
#     hifapath = mypath+"/hifa/cli/hifa.py"
#     hifvpath = mypath+"/hifv/cli/hifv.py"
#     myglobals = stack_frame_find()
#
#     execfile(hpath, myglobals)
#     execfile(hifpath, myglobals)
#     execfile(hsdpath, myglobals)
#     execfile(hifapath, myglobals)
#     execfile(hifvpath, myglobals)
#     #exec('import pipeline.infrastructure.executeppr', myglobals)

revision = environment.pipeline_revision


def log_host_environment():
    LOG.info('Pipeline version {!s} running on {!s}'.format(environment.pipeline_revision, environment.hostname))
    try:
        host_summary = '{!s} memory, {!s} x {!s} running {!s}'.format(
            measures.FileSize(environment.memory_size, measures.FileSizeUnits.BYTES),
            environment.logical_cpu_cores,
            environment.cpu_type,
            environment.host_distribution)

        LOG.info('Host environment: {!s}'.format(host_summary))
    except NotImplemented:
        pass

log_host_environment()

# FINALLY import executeppr. Do so as late as possible in pipeline module
# because executeppr make use of a part of pipeline module.
import pipeline.infrastructure.executeppr
