from __future__ import absolute_import
import os
import pkg_resources
import webbrowser

from . import environment
from . import infrastructure

from .infrastructure import Pipeline, Context

import pipeline.h
import pipeline.hif
import pipeline.hifa
import pipeline.hsd
import pipeline.hifv
import pipeline.hsdn

from .domain import measures
from casa_stack_manip import stack_frame_find

LOG = infrastructure.get_logger(__name__)

__pipeline_documentation_weblink_alma__ = "http://almascience.org/documents-and-tools/pipeline-documentation-archive"


def show_weblog(context):
    if context is None:
        return

    index_html = os.path.join(context.report_dir, 't1-1.html')
    webbrowser.open('file://' + index_html)


# def initcli():
#     print "Initializing cli..."
#     mypath = pkg_resources.resource_filename(__name__, '')
#     hifpath = mypath + "/hif/cli/hif.py"
#     hpath = mypath + "/h/cli/h.py"
#     hsdpath = mypath + "/hsd/cli/hsd.py"
#     hifapath = mypath + "/hifa/cli/hifa.py"
#     hifvpath = mypath + "/hifv/cli/hifv.py"
#     hsdnpath = mypath + "/hsdn/cli/hsdn.py"
#     myglobals = stack_frame_find()
#
#     execfile(hpath, myglobals)
#     execfile(hifpath, myglobals)
#     execfile(hsdpath, myglobals)
#     execfile(hifapath, myglobals)
#     execfile(hifvpath, myglobals)
#     execfile(hsdnpath, myglobals)
#     # exec('import pipeline.infrastructure.executeppr', myglobals)


def initcli():
    LOG.info('Initializing cli...')
    my_globals = stack_frame_find()
    for package in ['h', 'hif', 'hifa', 'hifv', 'hsd', 'hsdn']:
        abs_package = 'pipeline.{package}.cli.{package}'.format(package=package)
        try:
            # buildmytasks writes output to packagename.py
            path_to_cli = pkg_resources.resource_filename(abs_package, '{!s}.py'.format(package))
        except ImportError as e:
            LOG.debug('Import error: {!s}'.format(e))
            LOG.info('No tasks found for package: {!s}'.format(package))
        else:
            execfile(path_to_cli, my_globals)
            LOG.info('Loaded CASA tasks from package: {!s}'.format(package))


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
