from __future__ import absolute_import

import copy
import itertools
import operator
import os
import platform
import re
import sys
import types

from . import logging

LOG = logging.get_logger(__name__)

# logger for keeping a trace of CASA task and CASA tool calls.
# The filename incorporates the hostname to keep MPI client files distinct
CASACALLS_LOG = logging.get_logger('CASACALLS', stream=None, format='%(message)s', addToCasaLog=False,
                                   filename='casacalls-{!s}.txt'.format(platform.node().split('.')[0]))

# functions to be executed just prior to and immediately after execution of the
# CASA task, providing a way to collect metrics on task execution.
PREHOOKS = []
POSTHOOKS = []


class FunctionArg(object):
    """
    Class to hold named function or method arguments
    """
    def __init__(self, name, value):
        self.name = name
        self.value = value

    def __str__(self):
        return '{!s}={!r}'.format(self.name, self.value)

    def __repr__(self):
        return 'FunctionArg({!r}, {!r})'.format(self.name, self.value)


class NamelessArg(object):
    """
    Class to hold unnamed arguments
    """
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return str(self.value)

    def __repr__(self):
        return 'NamelessArg({!r})'.format(self.value)


def alphasort(argument):
    """
    Return an argument with values sorted so that the log record is easier to
    compare to other pipeline executions.

    :param argument: the FunctionArg or NamelessArg to sort
    :return: a value-sorted argument
    """
    if isinstance(argument, NamelessArg):
        return argument

    # holds a map of argument name to separators for argument values
    attrs_and_separators = {
        'asis': ' ',
        'spw': ',',
        'field': ',',
        'intent': ','
    }

    # deepcopy as we sort in place and don't want to modify the original
    argument = copy.deepcopy(argument)
    name = argument.name
    value = argument.value

    if name == 'inpfile' and type(value) == list:
        # get the indices of commands that are not summaries.
        apply_cmd_idxs = [idx for idx, val in enumerate(value) if "mode='summary'" not in val]

        # group the indices into consecutive ranges, i.e., between
        # flagdata summaries. Commands within these ranges can be
        # sorted.
        for _, g in itertools.groupby(enumerate(apply_cmd_idxs), lambda (i, x): i - x):
            idxs = map(operator.itemgetter(1), g)
            start_idx = idxs[0]
            end_idx = idxs[-1] + 1
            value[start_idx:end_idx] = sorted(value[start_idx:end_idx], key=natural_sort)

    else:
        for attr_name, separator in attrs_and_separators.iteritems():
            if name == attr_name and isinstance(value, str) and separator in value:
                value = separator.join(sorted(value.split(separator), key=natural_sort))

    return FunctionArg(name, value)


_uuid_regex = re.compile('[a-f0-9]{8}-?[a-f0-9]{4}-?4[a-f0-9]{3}-?[89ab][a-f0-9]{3}-?[a-f0-9]{12}', re.I)


def UUID_to_underscore(argument):
    """
    Return an argument with UUIDs converted to underscores.

    :param argument: the FunctionArg or NamelessArg to sort
    :return: a value-sorted argument
    """
    if isinstance(argument, NamelessArg):
        return argument
    if not isinstance(argument.value, str):
        return argument

    # deepcopy as we sort in place and don't want to modify the original
    argument = copy.deepcopy(argument)

    value = _uuid_regex.sub('<UUID>', argument.value)

    return FunctionArg(argument.name, value)


def truncate_paths(arg):
    # Path arguments are kw args with specific identifiers. Exit early if this
    # is not a path argument
    if isinstance(arg, NamelessArg):
        return arg
    if arg.name not in ('vis', 'caltable', 'gaintable', 'asdm', 'outfile', 'figfile', 'listfile', 'inpfile', 'plotfile',
                        'fluxtable', 'infile', 'infiles', 'mask', 'imagename', 'fitsimage', 'outputvis'):
        return arg

    # wrap value in a tuple so that strings can be interpreted by
    # the recursive map function
    basename_value = _recur_map(os.path.basename, (arg.value,))[0]
    return FunctionArg(arg.name, basename_value)


def _recur_map(fn, data):
    return [type(x) is types.StringType and fn(x) or _recur_map(fn, x) for x in data]


class JobRequest(object):
    def __init__(self, fn, *args, **kw):
        """
        Create a new JobRequest that encapsulates a function call and its
        associated arguments and keywords.
        """
        # remove any keyword arguments that have a value of None or an empty
        # string, letting CASA use the default value for that argument
        null_keywords = [k for k, v in kw.iteritems() if v in (None, '')]
        map(lambda key: kw.pop(key), null_keywords)

        self.fn = fn

        # CASA tasks are instances rather than functions, whose execution
        # begins at __call__
        if type(fn) is types.InstanceType:
            fn = fn.__call__

        # the next piece of code does some introspection on the given function
        # so that we can find out the complete invocation, adding any implicit
        # or defaulted argument values to those arguments explicitly given. We
        # use this information if execute(verbose=True) is specified.

        # get the argument names and default argument values for the given
        # function
        code = fn.func_code
        argcount = code.co_argcount
        argnames = code.co_varnames[:argcount]
        fn_defaults = fn.func_defaults or list()
        argdefs = dict(zip(argnames[-len(fn_defaults):], fn_defaults))

        # remove arguments that are not expected by the function, such as
        # pipeline variables that the CASA task is not expecting.
        unexpected_kw = [k for k, v in kw.iteritems() if k not in argnames]
        if unexpected_kw:
            LOG.warning('Removing unexpected keywords from JobRequest: {!s}'.format(unexpected_kw))
            map(lambda key: kw.pop(key), unexpected_kw)

        self.args = args
        self.kw = kw

        self._positional = [FunctionArg(name, arg) for name, arg in zip(argnames, args)]
        self._defaulted = [FunctionArg(name, argdefs[name])
                           for name in argnames[len(args):]
                           if name not in kw and name is not 'self']
        self._keyword = [FunctionArg(name, kw[name]) for name in argnames if name in kw]
        self._nameless = [NamelessArg(a) for a in args[argcount:]]

    def execute(self, dry_run=False, verbose=False):
        """
        Execute this job, returning any result to the caller.

        :param dry_run: True if the job should be logged rather than executed\
            (default: False)
        :type dry_run: boolean
        :param verbose: True if the complete invocation, including all default\
            variables and arguments, should be logged instead of just those\
            explicitly given (default: False)
        :type verbose: boolean
        """
        msg = self._get_fn_msg(verbose, sort_args=False)
        if dry_run:
            sys.stdout.write('Dry run: %s\n' % msg)
            return

        for hook in PREHOOKS:
            hook(self)
        LOG.info('Executing %s' % msg)

        # log sorted arguments to facilitate easier comparisons between
        # pipeline executions
        sorted_msg = self._get_fn_msg(verbose=False, sort_args=True)
        CASACALLS_LOG.debug(sorted_msg)

        try:
            return self.fn(*self.args, **self.kw)
        finally:
            for hook in POSTHOOKS:
                hook(self)

    def _get_fn_msg(self, verbose=False, sort_args=False):
        if verbose:
            args = self._positional + self._defaulted + self._nameless + self._keyword
        else:
            args = self._positional + self._nameless + self._keyword

        processed = [truncate_paths(arg) for arg in args]
        if sort_args:
            processed = [alphasort(arg) for arg in processed]
            processed = [UUID_to_underscore(arg) for arg in processed]

        string_args = [str(arg) for arg in processed]
        return '{!s}({!s})'.format(self.fn.__name__, ', '.join(string_args))

    def __repr__(self):
        return 'JobRequest({!r}, {!r})'.format(self.args, self.kw)

    def __str__(self):
        return self._get_fn_msg(verbose=False, sort_args=False)

    def hash_code(self, ignore=None):
        """
        Get the numerical hash code for this JobRequest.

        This code should - but is not guaranteed - to be unique.
        """
        if ignore is None:
            ignore = []

        to_match = dict(self.kw)
        for key in ignore:
            if key in to_match:
                del to_match[key]
        return self._gen_hash(to_match)

    def _gen_hash(self, o):
        """
        Makes a hash from a dictionary, list, tuple or set to any level, that
        contains only other hashable types (including any lists, tuples, sets,
        and dictionaries).
        """
        if isinstance(o, set) or isinstance(o, tuple) or isinstance(o, list):
            return tuple([self._gen_hash(e) for e in o])

        elif not isinstance(o, dict):
            return hash(o)

        new_o = copy.deepcopy(o)
        for k, v in new_o.items():
            new_o[k] = self._gen_hash(v)

        return hash(tuple(frozenset(new_o.items())))

    # JobRequests hold CASA functions as part of their __dict__; pickling the
    # JobRequest thus attempts to pickle the CASA function, which essentially
    # tries to pickle all of CASA. We circumvent this by replacing the
    # function with the module name on serialisation, replacing the module
    # name with the real function when the pickled state is unmarshalled.
    def __getstate__(self):
        odict = self.__dict__.copy()
        fn = odict['fn']
        module = fn.__module__
        odict['fn'] = '{0}'.format(module)
        return odict

    def __setstate__(self, d):
        odict = d.copy()
        name = odict['fn']
        module = __import__(name)
        odict['fn'] = getattr(module, name)
        self.__dict__.update(odict)


def natural_sort(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
