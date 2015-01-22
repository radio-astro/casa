from __future__ import absolute_import

import os
import re

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from . import utils

LOG = infrastructure.get_logger(__name__)

def absolute_path(name):
    return os.path.abspath(os.path.expanduser(os.path.expandvars(name)))

class SingleDishInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None,
                 infiles=None):
        self._init_properties(vars())

    def to_casa_args(self):
        args = self._get_task_args(ignore=('infiles','vis','output_dir'))
        if isinstance(self.infiles, list):
            args['infile'] = self.infiles[0]
        else:
            args['infile'] = self.infiles
        keys = ('iflist','pollist','scanlist')
        for (k,v) in args.items():
            if k in keys and v is None:
                args[k] = []
        keys = ('spw','scan','pol')
        for (k,v) in args.items():
            if k in keys and v is None:
                args[k] = ''
            
        return args

    @property
    def infiles(self):
        if self._infiles is not None:
            return self._infiles

        st_names = self.context.observing_run.st_names
        return st_names[0] if len(st_names) == 1 else st_names

    @infiles.setter
    def infiles(self, value):
        if isinstance(value, list):
            for v in value:
                self.context.observing_run.st_names.index(v)
        elif isinstance(value, str):
            if value.startswith('['):
                # may be PPR input (string list as string)
                _value = utils.to_list(value)
                for v in _value:
                    self.context.observing_run.st_names.index(v)
            else:
                self.context.observing_run.st_names.index(value)
            
        LOG.debug('Setting Input._infiles to %s'%(value))
        self._infiles = value

    @property
    def output_dir(self):
        if self._output_dir is None:
            return self.context.output_dir
        else:
            return self._output_dir

    @output_dir.setter
    def output_dir(self, value):
        self._output_dir = value

    # This is dummy
    @property
    def vis(self):
        return None

    def _to_list(self, attributes):
        if isinstance(attributes, str):
            new_value = utils.to_list(getattr(self, attributes))
            setattr(self, attributes, new_value)
        else:
            for attr in attributes:
                new_value = utils.to_list(getattr(self, attr))
                setattr(self, attr, new_value)

    def _to_bool(self, attributes):
        if isinstance(attributes, str):
            new_value = utils.to_bool(getattr(self, attributes))
            setattr(self, attributes, new_value)
        else:
            for attr in attributes:
                new_value = utils.to_bool(getattr(self, attr))
                setattr(self, attr, new_value)

    def _to_numeric(self, attributes):
        if isinstance(attributes, str):
            new_value = utils.to_numeric(getattr(self, attributes))
            setattr(self, attributes, new_value)
        else:
            for attr in attributes:
                new_value = utils.to_numeric(getattr(self, attr))
                setattr(self, attr, new_value)

    def _to_casa_arg(self, arg_list, file_id):
        if isinstance(arg_list, list):
            if file_id < len(arg_list):
                casa_arg = arg_list[file_id]
            else:
                casa_arg = arg_list[0]
        else:
            casa_arg = arg_list
        if casa_arg is None:
            casa_arg = ''
        return casa_arg

    def get_iflist(self, index):
        spw = '' if self.spw is None else self.spw
        return self._get_arg(spw, index)

    def get_pollist(self, index):
        pol = '' if self.pol is None else self.pol
        return self._get_arg(pol, index)

    def _get_arg(self, arg_list, index):
        sel = self._to_casa_arg(arg_list, index)
        return utils.selection_to_list(sel)
        

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

    def __repr__(self):
        #taskname = self.task if hasattr(self,'task') else 'none'
        s = '%s:\n\toutcome is %s'%(self.__class__.__name__,self._outcome_name())
        return s

class SingleDishTaskTemplate(basetask.StandardTaskTemplate):
    Inputs = SingleDishInputs

    def __init__(self, inputs):
        super(SingleDishTaskTemplate,self).__init__(inputs)
    
    @basetask.timestamp
    @basetask.capture_log
    @basetask.result_finaliser
    def execute(self, dry_run=True, **parameters):
        try:
            # Empty a reference to DataTable instance to avoid an additional instantiation 
            # of DataTable by copying context
            context = self.inputs.context
            self._disconnect_datatable(context)
                        
            # execute
            aresult = super(SingleDishTaskTemplate,self).execute(dry_run=dry_run, **parameters)
            
        finally:
            # Put DataTable back to context
            # This is necessary because datatable_setter handles a copy of context, which 
            # is passed to prepare/analyse while here we handle original context
            LOG.debug('%s: Clean up after execute...'%(self.__class__.__name__))
            self._reconnect_datatable(context   )
        
        # Don't convert results to ResultsList if it is not
        # the top-level task.
        if self.inputs.context.subtask_counter > 0:
            return aresult
        
        if type(aresult) is basetask.ResultsList:
            results = aresult
        else:
            results = basetask.ResultsList()
            results.append(aresult)

        # Delete the capture log for subtasks as the log will be attached to the
        # outer ResultList.
        for r in results:
            if hasattr(r, 'casalog'):
                del r.casalog
        return results


    @property
    def context(self):
        return self.inputs.context
    
    @property
    def datatable(self):
        observing_run = self.context.observing_run
        if hasattr(observing_run, 'datatable_instance'):
            return observing_run.datatable_instance
        else:
            return None
        
    def _disconnect_datatable(self, context):
        self._datatable_instance = None
        if hasattr(context.observing_run, 'datatable_instance') \
            and context.observing_run.datatable_instance is not None:
            self._datatable_instance = context.observing_run.datatable_instance
            LOG.debug('Empty reference to datatable (address 0x%x)'%(id(self._datatable_instance)))
            context.observing_run.datatable_instance = None
            context.observing_run.datatable_name = None
    
    def _reconnect_datatable(self, context):
        if hasattr(context.observing_run, 'datatable_instance') \
            and hasattr(self, '_datatable_instance') \
            and self._datatable_instance is not None:
            datatable = self._datatable_instance
            if context.observing_run.datatable_instance is None:
                LOG.debug('Set reference to DataTable instance (address 0x%x)'%(id(datatable)))
                context.observing_run.datatable_instance = datatable
                context.observing_run.datatable_name = os.path.relpath(datatable.name, context.output_dir)

def datatable_setter(func):
    import functools
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        task = args[0]
        context = task.inputs.context
        LOG.debug('%s.%s'%(task.__class__.__name__,func.__name__))
        task._reconnect_datatable(context)
        return func(*args, **kwargs)
    return wrapper
            