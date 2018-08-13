from __future__ import absolute_import

import os
import re
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.domain.datatable as datatable
from . import utils

LOG = infrastructure.get_logger(__name__)

def absolute_path(name):
    return os.path.abspath(os.path.expanduser(os.path.expandvars(name)))

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

class SingleDishTask(basetask.StandardTaskTemplate):
    def __init__(self, inputs):
        super(SingleDishTask, self).__init__(inputs)

    @property
    def datatable_name(self):
        if hasattr(self.inputs.context.observing_run, 'ms_datatable_name'):
            return self.inputs.context.observing_run.ms_datatable_name
        else:
            return None
         
    @property
    def datatable_instance(self):
        if hasattr(self, '_datatable_instance'):
            if self._datatable_instance is None and self._datatable_name is not None:
                LOG.info('SingleDishTask: Creating DataTable instance...')
                self._datatable_instance = datatable.DataTableImpl(name=self.datatable_name, 
                                                                   readonly=False)
            return self._datatable_instance
        else:
            if self.datatable_name is not None:
                LOG.info('SingleDishTask: Creating DataTable instance...')
                self._datatable_instance = datatable.DataTableImpl(name=self.datatable_name, 
                                                                   readonly=False)
            else:
                self._datatable_instance = None
            return self._datatable_instance
    

class ParameterContainerJob(object):
    def __init__(self, task, **parameters):
        self.task = task
        self.parameters = parameters
    
    def execute(self, dry_run=True):
        result = self.task.execute(dry_run, **self.parameters)
        return result