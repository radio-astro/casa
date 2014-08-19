from __future__ import absolute_import
import copy
import os
import sys
import types

import applycal_cli
import bandpass_cli
import calstat_cli
import clean_cli
import clearcal_cli
import exportfits_cli
import flagcmd_cli
import flagdata_cli
import flagmanager_cli
import fluxscale_cli
import gaincal_cli
import gencal_cli
import hanningsmooth_cli
import immath_cli
import immoments_cli
import importasdm_cli
import importevla_cli
import listobs_cli
import plotants_cli
import plotbandpass_cli
import plotcal_cli
import plotms_cli
import plotweather_cli
import polcal_cli
import setjy_cli
import split_cli
import statwt_cli
import wvrgcal_cli
import visstat_cli

import sdcal2_cli
import sdflag_cli 
import sdimaging_cli 
import sdsave_cli 

from . import casatools
from . import logging
from . import utils

LOG = logging.get_logger(__name__)

# functions to be executed just prior to and immediately after execution of the
# CASA task, providing a way to collect metrics on task execution.
PREHOOKS = []
POSTHOOKS = []

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
            LOG.warning('Removing unexpected keywords from JobRequest: '
                        '%s' % utils.commafy(unexpected_kw, quotes=False))
            map(lambda key: kw.pop(key), unexpected_kw)

        self.args = args
        self.kw = kw

        def format_arg_value(arg_val):
            arg, val = arg_val
            return '%s=%r' % (arg, val)
        
        self._positional = map(format_arg_value, zip(argnames, args))
        self._defaulted = [format_arg_value((a, argdefs[a]))
                           for a in argnames[len(args):] 
                           if a not in kw and a is not 'self']
        self._nameless = map(repr, args[argcount:])
        self._keyword = map(format_arg_value, kw.items())

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
        msg = self._get_fn_msg(verbose)
        if dry_run:
            sys.stdout.write('Dry run: %s\n' % msg)                                
        else:
            for hook in PREHOOKS:
                hook(self)
            LOG.info('Executing %s' % msg)
            try:
                return self.fn(*self.args, **self.kw)
            finally:
                for hook in POSTHOOKS:
                    hook(self)

    def _recur_map(self, f, data):
        return [type(x) is types.StringType and f(x) or self._recur_map(f, x) for x in data]

    def _get_fn_msg(self, verbose=False):
        kw = dict(self.kw)
        for path_arg in ('vis', 'caltable', 'gaintable', 'asdm', 'outfile'):
            if path_arg in kw:
                # wrap value in a tuple so that strings can be interpreted by
                # the recursive map function
                val = (kw[path_arg],)
                kw[path_arg] = self._recur_map(os.path.basename, val)[0]

        def format_arg_value(arg_val):
            arg, val = arg_val
            return '%s=%r' % (arg, val)
    
        basename_kw = map(format_arg_value, kw.items())            
        if verbose:
            args = self._positional + self._defaulted + self._nameless \
                + basename_kw
        else:
            args = self._positional + self._nameless + basename_kw
        
        msg = '%s(%s)' % (self.fn.__name__, ', '.join(args))
        return msg
    
    def __repr__(self):
        return 'JobRequest({0})'.format(str(self))

    def __str__(self):
        args = self._positional + self._nameless + self._keyword
        call = "%s(%s)" % (self.fn.__name__, ", ".join(args))
        return '{0}'.format(call)

    def hash_code(self, ignore=[]):
        """
        Get the numerical hash code for this JobRequest.
        
        This code should - but is not guaranteed - to be unique.
        """
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
        
    def comment(self, comment='', echo_to_screen=True):
        """
        Schedule a job that posts the given comment to the CASA log. This may
        be useful if you wish to explain why a job is being executed.
        
        :param comment: the comment to post
        :type comment: string
        :rtype: :class:`JobRequest`
        """
        job = JobRequest(casatools.post_to_log, comment=comment, 
                         echo_to_screen=echo_to_screen)
        self._jobs.append(job)
        return job

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
        return self._get_job(applycal_cli.applycal_cli, *v, **k)

    def bandpass(self, *v, **k):
        return self._get_job(bandpass_cli.bandpass_cli, *v, **k)

    def calstat(self, *v, **k):
        return self._get_job(calstat_cli.calstat_cli, *v, **k)

    def clean(self, *v, **k):
        return self._get_job(clean_cli.clean_cli, *v, **k)

    def clearcal(self, *v, **k):
        return self._get_job(clearcal_cli.clearcal_cli, *v, **k)

    def exportfits(self, *v, **k):
        return self._get_job(exportfits_cli.exportfits_cli, *v, **k)

    def gaincal(self, *v, **k):
        return self._get_job(gaincal_cli.gaincal_cli, *v, **k)

    def flagcmd(self, *v, **k):
        return self._get_job(flagcmd_cli.flagcmd_cli, *v, **k)

    def flagdata(self, *v, **k):
        return self._get_job(flagdata_cli.flagdata_cli, *v, **k)

    def flagmanager(self, *v, **k):
        return self._get_job(flagmanager_cli.flagmanager_cli, *v, **k)

    def fluxscale(self, *v, **k):
        return self._get_job(fluxscale_cli.fluxscale_cli, *v, **k)

    def gencal(self, *v, **k):
        return self._get_job(gencal_cli.gencal_cli, *v, **k)
    
    def hanningsmooth(self, *v, **k):
        return self._get_job(hanningsmooth_cli.hanningsmooth_cli, *v, **k)

    def immath(self, *v, **k):
        return self._get_job(immath_cli.immath_cli, *v, **k)    

    def immoments(self, *v, **k):
        return self._get_job(immoments_cli.immoments_cli, *v, **k)    

    def importasdm(self, *v, **k):
        return self._get_job(importasdm_cli.importasdm_cli, *v, **k)
    
    def importevla(self, *v, **k):
        return self._get_job(importevla_cli.importevla_cli, *v, **k)

    def listobs(self, *v, **k):
        return self._get_job(listobs_cli.listobs_cli, *v, **k)

    def plotants(self, *v, **k):
        return self._get_job(plotants_cli.plotants_cli, *v, **k)

    def plotbandpass(self, *v, **k):
        return self._get_job(plotbandpass_cli.plotbandpass_cli, *v, **k)

    def plotcal(self, *v, **k):
        return self._get_job(plotcal_cli.plotcal_cli, *v, **k)

    def plotms(self, *v, **k):
        return self._get_job(plotms_cli.plotms_cli, *v, **k)

    def plotweather(self, *v, **k):
        return self._get_job(plotweather_cli.plotweather_cli, *v, **k)

    def polcal(self, *v, **k):
        return self._get_job(polcal_cli.polcal_cli, *v, **k)

    def setjy(self, *v, **k):
        return self._get_job(setjy_cli.setjy_cli, *v, **k)
    
    def split(self, *v, **k):
        return self._get_job(split_cli.split_cli, *v, **k)
    
    def statwt(self, *v, **k):
        return self._get_job(statwt_cli.statwt_cli, *v, **k)

    def wvrgcal(self, *v, **k):
        return self._get_job(wvrgcal_cli.wvrgcal_cli, *v, **k)
        
    def visstat(self, *v, **k):
        return self._get_job(visstat_cli.visstat_cli, *v, **k)

    def sdcal2(self, *v, **k):
        return self._get_job(sdcal2_cli.sdcal2_cli, *v, **k)
    
    def sdflag(self, *v, **k):
        return self._get_job(sdflag_cli.sdflag_cli, *v, **k)
    
    def sdimaging(self, *v, **k):
        return self._get_job(sdimaging_cli.sdimaging_cli, *v, **k)
    
    def sdsave(self, *v, **k):
        return self._get_job(sdsave_cli.sdsave_cli, *v, **k) 

    def _get_job(self, task, *v, **k):
        job = JobRequest(task, *v, **k)
        self._jobs.append(job)
        return job


casa_tasks = CASATaskJobGenerator()
