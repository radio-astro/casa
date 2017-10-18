from __future__ import absolute_import
import copy_reg
import contextlib
import inspect
import os
import platform
import sys

from casac import casac
from taskinit import casalog

from . import logging

# standard logger that emits messages to stdout and CASA logger
LOG = logging.get_logger(__name__)

# logger for keeping a trace of CASA task and CASA tool calls.
# The filename incorporates the hostname to keep MPI client files distinct
CASACALLS_LOG = logging.get_logger('CASACALLS', stream=None, format='%(message)s', addToCasaLog=False,
                                   filename='casacalls-{!s}.txt'.format(platform.node().split('.')[0]))


def log_call(fn, logger, level):
    """
    Decorate a function or method so that all invocations of that function or
    method are logged.

    :param fn: function to decorate
    :param logger: logger to record messages to
    :param level: log level (e.g., logging.INFO, logging.WARNING, etc.)
    :return: decorated function
    """

    def f(*args, **kwargs):
        # remove any keyword arguments that have a value of None or an empty
        # string, letting CASA use the default value for that argument
        kwargs = {k: v for k, v in kwargs.iteritems() if v not in (None, '')}

        # get the argument names and default argument values for the given
        # function
        code = fn.func_code
        argcount = code.co_argcount
        argnames = code.co_varnames[:argcount]

        positional = {k: v for k, v in zip(argnames, args)}

        def format_arg_value(arg_val):
            arg, val = arg_val
            return '%s=%r' % (arg, val)

        nameless = map(repr, args[argcount:])
        positional = map(format_arg_value, positional.iteritems())
        keyword = map(format_arg_value, kwargs.iteritems())

        # don't want self in message as it is an object memory reference
        msg_args = [v for v in positional + nameless + keyword if not v.startswith('self=')]

        msg = '{!s}.{!s}({!s})'.format(fn.im_class.__name__, fn.__name__, ', '.join(msg_args))
        logger.log(level, msg)

        return fn(*args, **kwargs)

    return f


def log_tool_invocations(cls, level=logging.TRACE):
    """
    Return an instance of a class, with all class methods decorated to log
    method calls.

    :param cls: class to wrap
    :param level: log level for emitted messages
    :return: an instance of the decorated class
    """
    bound_methods = {name: method for (name, method) in inspect.getmembers(cls, inspect.ismethod)
                     if not name.startswith('__') and not name.endswith('__')}
    logging_override_methods = {name: log_call(method, CASACALLS_LOG, level)
                                for name, method in bound_methods.iteritems()}

    cls_name = 'Logging{!s}'.format(cls.__name__.capitalize())
    new_cls = type(cls_name, (cls,), logging_override_methods)

    return new_cls()

# log messages from CASA tool X with log level Y.
# The assigned log level for tools should be DEBUG or lower, otherwise the log
# file is created and written to even on non-debug pipeline runs, where
# loglevel=INFO. The default log level is TRACE.
#
# Example:
# imager = log_tool_invocations(casac.imager, logging.DEBUG)
imager = log_tool_invocations(casac.imager)
measures = log_tool_invocations(casac.measures)
quanta = log_tool_invocations(casac.quanta)
table = log_tool_invocations(casac.table)
ms = log_tool_invocations(casac.ms)
tableplot = log_tool_invocations(casac.table)
calibrater = log_tool_invocations(casac.calibrater)
calanalysis = log_tool_invocations(casac.calanalysis)
msplot = log_tool_invocations(casac.msplot)
calplot = log_tool_invocations(casac.calplot)
agentflagger = log_tool_invocations(casac.agentflagger)
image = log_tool_invocations(casac.image)
imagepol = log_tool_invocations(casac.imagepol)
simulator = log_tool_invocations(casac.simulator)
componentlist = log_tool_invocations(casac.componentlist)
coordsys = log_tool_invocations(casac.coordsys)
regionmanager = log_tool_invocations(casac.regionmanager)
spectralline = log_tool_invocations(casac.spectralline)
utils = log_tool_invocations(casac.utils)
deconvolver = log_tool_invocations(casac.deconvolver)
vpmanager = log_tool_invocations(casac.vpmanager)
vlafillertask = log_tool_invocations(casac.vlafillertask)
atmosphere = log_tool_invocations(casac.atmosphere)
msmd = log_tool_invocations(casac.msmetadata)

log = casalog


def post_to_log(comment='', echo_to_screen=True):
    log.post(comment)
    if echo_to_screen:
        sys.stdout.write('{0}\n'.format(comment))


def set_log_origin(fromwhere=''):
    log.origin(fromwhere)


def context_manager_factory(tool):
    """
    Create a context manager function that wraps the given CASA tool.

    The returned context manager function takes one argument: a filename. The
    function opens the file using the CASA tool, returning the tool so that it
    may be used for queries or other operations pertaining to the tool. The
    tool is closed once it falls out of scope or an exception is raised.
    """
    tool_name = tool.__class__.__name__

    @contextlib.contextmanager
    def f(filename, **kwargs):
        if not os.path.exists(filename):
            raise IOError('No such file or directory: {!r}'.format(filename))
        LOG.trace('%s tool: opening %r', tool_name, filename)
        tool.open(filename, **kwargs)
        try:
            yield tool
        finally:
            LOG.trace('%s tool: closing %r', tool_name, filename)
            tool.close()
    return f

# context managers for frequently used CASA tools
CalAnalysis = context_manager_factory(calanalysis)
ImageReader = context_manager_factory(image)
ImagerReader = context_manager_factory(imager)
MSReader = context_manager_factory(ms)
TableReader = context_manager_factory(table)
MSMDReader = context_manager_factory(msmd)

# C extensions cannot be pickled, so ignore the CASA logger on pickle and
# replace with it with the current CASA logger on unpickle
__tools = ['imager', 'measures', 'quanta', 'table', 'ms', 'tableplot', 
           'calibrater', 'msplot', 'calplot', 'agentflagger',
           'image', 'imagepol', 'simulator', 'componentlist', 'coordsys',
           'regionmanager', 'spectralline', 'utils', 'deconvolver',
           'vpmanager', 'vlafillertask', 'atmosphere', 'log', 'utils']

for tool in __tools:
    tool_type = type(globals()[tool])
    unpickler = lambda data: globals()[tool]
    pickler = lambda _: (unpickler, (tool, ))
    copy_reg.pickle(tool_type, pickler, unpickler)
