from __future__ import absolute_import
import copy
import collections
import itertools
import operator
import os
import operator
import platform
import re
import sys
import types

import casadef

import applycal_pg
import bandpass_pg
import calstat_cli
import clean_cli
import clearcal_cli
import delmod_cli
import exportfits_cli
import flagcmd_cli
import flagdata_pg
import flagmanager_cli
import fluxscale_pg
import gaincal_pg
import gencal_cli
import hanningsmooth_cli
import imdev_cli
import imhead_cli
import immath_cli
import immoments_cli
import imregrid_pg
import impbcor_pg
import importasdm_cli
import importevla_cli
import imstat_cli
import imsubimage_cli
import initweights_cli
import listobs_cli
import mstransform_cli
import partition_cli
import plotants_cli
import plotbandpass_cli
import plotcal_cli
import plotms_pg
import plotweather_cli
import polcal_cli
import setjy_pg
import split_cli
import statwt_cli
import statwt2_cli
import tclean_pg
import wvrgcal_cli
import visstat_cli
import sdbaseline_cli
import sdcal_cli
import sdimaging_cli

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


class CASATaskJobGenerator(object):
    """
    CASATaskJobGenerator is a JobRequest-generating proxy for CASA tasks.

    CASATaskJobGenerator has instance methods for every CASA task. The
    signature of each methods exactly matches that of the CASA task it
    mirrors. However, rather than executing the task directly when these
    methods are called, CASATaskJobGenerator returns a JobRequest for every
    invocation; these jobs then be examined and executed at a later date.

    In addition to directly collecting the JobRequest generated by an instance
    method call, jobs can also be retrieved with a call to get_jobs() (which
    clears the job queue between calls) or with a call to get_job_history(),
    which returns every JobRequest logged by this CASATaskJobGenerator.

    The CASA task implementations are located at run-time and proxies for each
    task attached to this class at runtime. The name and signature of each
    method will match those of the tasks in the CASA environment when this
    module was imported.
    """
    def __init__(self):
        """
        Create a new CASATaskJobGenerator instance.
        """
        self._jobs = []
        self._current_job = 0

    def flush_jobs(self):
        """
        Get the list of jobs added to this queue since the last time
        flush_jobs was called.

        :rtype: list of :class:`JobRequest`
        """
        # get the job history from the current pointer onwards..
        current_queue = self._jobs[self._current_job:]
        # .. and move the current job pointer to the end of the list
        self._current_job = len(self._jobs)
        return current_queue

    def get_job_history(self):
        """
        Get the list of all jobs invoked via this instance.

        :rtype: list of :class:`JobRequest`
        """
        return self._jobs

    def applycal(self, *v, **k):
        return self._get_job(applycal_pg.applycal_pg, *v, **k)

    def bandpass(self, *v, **k):
        return self._get_job(bandpass_pg.bandpass_pg, *v, **k)

    def calstat(self, *v, **k):
        return self._get_job(calstat_cli.calstat_cli, *v, **k)

    def clean(self, *v, **k):
        return self._get_job(clean_cli.clean_cli, *v, **k)

    def clearcal(self, *v, **k):
        return self._get_job(clearcal_cli.clearcal_cli, *v, **k)

    def delmod(self, *v, **k):
        return self._get_job(delmod_cli.delmod_cli, *v, **k)

    def exportfits(self, *v, **k):
        return self._get_job(exportfits_cli.exportfits_cli, *v, **k)

    def gaincal(self, *v, **k):
        return self._get_job(gaincal_pg.gaincal_pg, *v, **k)

    def flagcmd(self, *v, **k):
        return self._get_job(flagcmd_cli.flagcmd_cli, *v, **k)

    def flagdata(self, *v, **k):
        return self._get_job(flagdata_pg.flagdata_pg, *v, **k)

    def flagmanager(self, *v, **k):
        return self._get_job(flagmanager_cli.flagmanager_cli, *v, **k)

    def fluxscale(self, *v, **k):
        # work around problem with fluxscale_pg that says:
        # An error occurred running task fluxscale: in method 'calibrater_fluxscale', argument 15 of type 'bool'
        if 'display' not in k:
            k['display'] = False
        return self._get_job(fluxscale_pg.fluxscale_pg, *v, **k)

    def gencal(self, *v, **k):
        return self._get_job(gencal_cli.gencal_cli, *v, **k)

    def hanningsmooth(self, *v, **k):
        return self._get_job(hanningsmooth_cli.hanningsmooth_cli, *v, **k)

    def imdev(self, *v, **k):
        return self._get_job(imdev_cli.imdev_cli, *v, **k)

    def imhead(self, *v, **k):
        return self._get_job(imhead_cli.imhead_cli, *v, **k)

    def immath(self, *v, **k):
        return self._get_job(immath_cli.immath_cli, *v, **k)

    def immoments(self, *v, **k):
        return self._get_job(immoments_cli.immoments_cli, *v, **k)

    def imregrid(self, *v, **k):
        return self._get_job(imregrid_pg.imregrid_pg, *v, **k)

    def impbcor(self, *v, **k):
        return self._get_job(impbcor_pg.impbcor_pg, *v, **k)

    def importasdm(self, *v, **k):
        return self._get_job(importasdm_cli.importasdm_cli, *v, **k)

    def importevla(self, *v, **k):
        return self._get_job(importevla_cli.importevla_cli, *v, **k)

    def imstat(self, *v, **k):
        return self._get_job(imstat_cli.imstat_cli, *v, **k)

    def imsubimage(self, *v, **k):
        return self._get_job(imsubimage_cli.imsubimage_cli, *v, **k)

    def initweights(self, *v, **k):
        return self._get_job(initweights_cli.initweights_cli, *v, **k)

    def listobs(self, *v, **k):
        return self._get_job(listobs_cli.listobs_cli, *v, **k)

    def mstransform(self, *v, **k):
        return self._get_job(mstransform_cli.mstransform_cli, *v, **k)

    def partition(self, *v, **k):
        return self._get_job(partition_cli.partition_cli, *v, **k)

    def plotants(self, *v, **k):
        return self._get_job(plotants_cli.plotants_cli, *v, **k)

    def plotbandpass(self, *v, **k):
        return self._get_job(plotbandpass_cli.plotbandpass_cli, *v, **k)

    def plotcal(self, *v, **k):
        return self._get_job(plotcal_cli.plotcal_cli, *v, **k)

    def plotms(self, *v, **k):
        return self._get_job(plotms_pg.plotms_pg, *v, **k)

    def plotweather(self, *v, **k):
        return self._get_job(plotweather_cli.plotweather_cli, *v, **k)

    def polcal(self, *v, **k):
        return self._get_job(polcal_cli.polcal_cli, *v, **k)

    def setjy(self, *v, **k):
        return self._get_job(setjy_pg.setjy_pg, *v, **k)

    def split(self, *v, **k):
        return self._get_job(split_cli.split_cli, *v, **k)

    def statwt(self, *v, **k):
        return self._get_job(statwt_cli.statwt_cli, *v, **k)

    def statwt2(self, *v, **k):
        return self._get_job(statwt2_cli.statwt2_cli, *v, **k)

    def tclean(self, *v, **k):
        return self._get_job(tclean_pg.tclean_pg, *v, **k)

    def wvrgcal(self, *v, **k):
        return self._get_job(wvrgcal_cli.wvrgcal_cli, *v, **k)

    def visstat(self, *v, **k):
        return self._get_job(visstat_cli.visstat_cli, *v, **k)

    def uvcontfit(self, *v, **k):
        # Note this is pipeline CASA style task not a CASA task
        import pipeline.hif.cli.task_uvcontfit as task_uvcontfit
        return self._get_job(task_uvcontfit.uvcontfit, *v, **k)

    def sdimaging(self, *v, **k):
        return self._get_job(sdimaging_cli.sdimaging_cli, *v, **k)

    def sdcal(self, *v, **k):
        return self._get_job(sdcal_cli.sdcal_cli, *v, **k)

    def sdbaseline(self, *v, **k):
        return self._get_job(sdbaseline_cli.sdbaseline_cli, *v, **k)
        # return self._get_job(sdbaseline_pg.sdbaseline_pg, *v, **k)

    def _get_job(self, task, *v, **k):
        job = JobRequest(task, *v, **k)
        self._jobs.append(job)
        return job


casa_tasks = CASATaskJobGenerator()


def natural_sort(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
