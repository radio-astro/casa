from __future__ import absolute_import
import inspect
import imp
import os
import string
import subprocess
import sys
import webbrowser

from . import infrastructure
from . import recipes

from . import h
from . import hco
from . import hif
from . import hsd
from . import hifv
from . import hifa

from .infrastructure import Pipeline, Context

LOG = infrastructure.get_logger(__name__)

# create a fake module containing all the tasks defined in 

def _all_subclasses(cls):
    '''
    Return a list of all subclasses that inherit directly or indirectly from
    the given class.
    '''
    return cls.__subclasses__() + [g for s in cls.__subclasses__()
                                    for g in _all_subclasses(s)]

def _get_unified_task_module(packages):
    '''
    Create a new module containing all tasks in the given packages.
    '''
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


def initcli() :
    print "Initializing cli..."
    mypath = os.path.dirname(__file__);
    hifpath = mypath+"/hif/cli/hif.py"
    hpath = mypath+"/h/cli/h.py"
    hsdpath = mypath+"/hsd/cli/hsd.py"
    hifapath = mypath+"/hifa/cli/hifa.py"
    hifvpath = mypath+"/hifv/cli/hifv.py"
    myglobals = sys._getframe(len(inspect.stack())-1).f_globals

    execfile(hpath, myglobals)
    execfile(hifpath, myglobals)
    execfile(hsdpath, myglobals)
    execfile(hifapath, myglobals)
    execfile(hifvpath, myglobals)


# find pipeline revision using svnversion
def _get_revision():
    try:
        # get SVN revision using svnversion as it gives information when the
        # directory has been modified
        args = ['svnversion', '.']
        p = subprocess.Popen(args, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, shell=True,
                cwd=os.path.dirname(__file__))
        (stdout, _) = p.communicate()

        if p.returncode is not 0:
            return 'Unknown'

        revision = string.strip(stdout)

        # get SVN branch using svn info
        args = ['svn info .']
        myenv = os.environ.copy()
        myenv['LC_MESSAGES'] = 'en_US.UTF_8'
        p = subprocess.Popen(args, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, shell=True,
                cwd=os.path.dirname(__file__),
                env=myenv)
        (stdout, _) = p.communicate()

        if p.returncode is not 0:
            return revision
        
        kv = [s.split(':', 1) for s in stdout.splitlines()]
        # subindex kv as last item in splitlines is []
        d = {k:v.strip() for (k, v) in kv[0:-1]}

        url = d['URL']
        # pipeline trunk and branches live within this common directory
        common_svn_prefix = '/branches/project/'
        root = d['Repository Root'] + common_svn_prefix

        branch = os.path.split(os.path.relpath(url, root))[0]
        if branch == 'pipeline':
            branch = 'trunk'

        return '%s (%s)' % (revision, branch)
    except:
        return 'Unknown'


revision = _get_revision()
