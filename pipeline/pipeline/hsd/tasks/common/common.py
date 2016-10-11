from __future__ import absolute_import

import os
import re
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from . import utils

LOG = infrastructure.get_logger(__name__)

def absolute_path(name):
    return os.path.abspath(os.path.expanduser(os.path.expandvars(name)))

class SingleDishInputs(basetask.StandardInputs):
    # This is dummy
    @property
    def vis(self):
        return None
        

class SingleDishResults(basetask.Results):
    def __init__(self, task=None, success=None, outcome=None):
        super(SingleDishResults, self).__init__()
        self.task = task
        self.success = success
        self.outcome = outcome
        self.error = set()
        
    def merge_with_context(self, context):
        self.error.clear()

    def _outcome_name(self):
        # usually, outcome is a name of the file
        return absolute_path(self.outcome)

    def _get_outcome(self, key):
        if type(self.outcome) is types.DictType:
            return self.outcome.get(key, None)
        else:
            return None
        
    def __repr__(self):
        #taskname = self.task if hasattr(self,'task') else 'none'
        s = '%s:\n\toutcome is %s'%(self.__class__.__name__,self._outcome_name())
        return s

class ParameterContainerJob(object):
    def __init__(self, task, **parameters):
        self.task = task
        self.parameters = parameters
    
    def execute(self, dry_run=True):
        result = self.task.execute(dry_run, **self.parameters)
        return result