from __future__ import absolute_import

import re

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.domain.datatable import DataTableImpl as DataTable
from .. import common
from . import reader
from . import analyser

LOG = infrastructure.get_logger(__name__)

import os
import asap as sd
import numpy 

class SDInspectDataInputs(common.SingleDishInputs):
    """
    Inputs for single dish calibration
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, infiles=None):
        self._init_properties(vars())
        self._to_list(['infiles'])
        

class SDInspectDataResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDInspectDataResults,self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDInspectDataResults,self).merge_with_context(context)
        
        # merge outcome with datatable
        datatable = DataTable(self.outcome['name'])
        datatable.putcol('POSGRP', self.outcome.pop('position_group'))
        datatable.putkeyword('POSGRP_REP', self.outcome.pop('position_group_rep'))
        datatable.putkeyword('POSGRP_LIST', self.outcome.pop('position_group_list'))
        datatable.putkeyword('TIMEGRP_LIST', self.outcome.pop('time_group_list'))
        time_group = self.outcome.pop('time_group')
        time_gap = self.outcome.pop('time_gap')
        datatable.putcol('TIMEGRP_S', time_group[0])
        datatable.putcol('TIMEGRP_L', time_group[1])
        datatable.putkeyword('TIMEGAP_S', time_gap[0])
        datatable.putkeyword('TIMEGAP_L', time_gap[1])

        # export datatable (both RO and RW)
        datatable.exportdata(minimal=False)

        # merge
        context.observing_run.merge_inspection(instance=datatable, **self.outcome)

    def _outcome_name(self):
        name = self.outcome['name']
        return os.path.abspath(os.path.expanduser(os.path.expandvars(name)))


class SDInspectData(common.SingleDishTaskTemplate):
    Inputs = SDInspectDataInputs

    @common.datatable_setter
    def prepare(self):
        # use plain table for scantable
        storage_save = sd.rcParams['scantable.storage']
        sd.rcParams['scantable.storage'] = 'disk'

        # inputs 
        inputs = self.inputs

        # create DataTable under context directory
        table_name = os.path.join(inputs.context.name,'DataTable.tbl')

        if os.path.exists(table_name):
            # if DataTable already exists, remove it
            LOG.info('remove existing DataTable...')
            os.system('rm -rf %s'%(table_name))
        

        # create DataTableReader instance
        worker = reader.DataTableReader(context=inputs.context, table_name=table_name)
        LOG.debug('table_name=%s'%(table_name))


        # loop over infiles
        infiles = inputs.infiles
        if isinstance(infiles, list):
            for f in infiles:
                worker.set_name(f)
                self._executor.execute(worker, merge=False)
        else:
            worker.set_name(infiles)
            self._executor.execute(worker, merge=False)

        # done, restore scantable.storage
        sd.rcParams['scantable.storage'] = storage_save

        outcome = {}
        outcome['name'] = table_name
        datatable = worker.get_datatable()
        # export datatable (both RO and RW)
        datatable.exportdata(minimal=False)

        result = SDInspectDataResults(task=self.__class__,
                                      success=True,
                                      outcome=outcome)
        result.task = self.__class__

#         #stage_number is taken care of by basetask.result_finaliser
#         if inputs.context.subtask_counter is 0: 
#             result.stage_number = inputs.context.task_counter - 1
#         else:
#             result.stage_number = inputs.context.task_counter               

        return result

    def analyse(self, result):

        datatable = DataTable(result.outcome['name'])
        scantablelist = self.inputs.context.observing_run
        worker = analyser.DataTableAnalyser(scantablelist,
                                            datatable)

        # analyse datatable with observing_run
        self._executor.execute(worker, merge=False)

        # update result
        result.outcome['position_group'] = worker.posgrp
        result.outcome['time_group'] = worker.timegrp
        result.outcome['position_group_rep'] = worker.posgrp_rep
        result.outcome['position_group_list'] = worker.posgrp_list
        result.outcome['time_group_list'] = worker.timegrp_list
        result.outcome['time_gap'] = worker.timegap
        result.outcome['reduction_group'] = worker.reduction_group
        result.outcome['calibration_strategy'] = worker.calibration_strategy
        result.outcome['beam_size'] = worker.beam_size
        #result.outcome['grid_position'] = worker.grid_position
        result.outcome['observing_pattern'] = worker.observing_pattern
        
        return result



