from __future__ import absolute_import

import re

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.basetask as basetask
from pipeline.domain.datatable import DataTableImpl as DataTable
import pipeline.domain as domain
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
        time_group_list = self.outcome.pop('time_group_list')
        time_group = self.outcome.pop('time_group')
        time_gap = self.outcome.pop('time_gap')
        datatable.putkeyword('TIMEGAP_S', time_gap[0])
        datatable.putkeyword('TIMEGAP_L', time_gap[1])
        
        # put time group to DataTable keyword
        reduction_group = self.outcome['reduction_group']
        for (group_id, member_list) in reduction_group.items():
            for member in member_list:
                ant = member.antenna
                spw = member.spw
                pols = member.pols
                for pol in pols:
                    LOG.info('Adding time table for Reduction Group %s (antenna %s spw %s pol %s)'%(group_id,ant,spw,pol))
                    datatable.set_timetable(ant, spw, pol, time_group_list[ant][spw][pol], numpy.array(time_group[0]), numpy.array(time_group[1]))

        # export datatable (both RO and RW)
        datatable.exportdata(minimal=False)
        
        LOG.info('DataTable keys: %s'%(datatable.tb2.keywordnames()))

        # merge to observing_run
        context.observing_run.merge_inspection(instance=datatable, **self.outcome)
        
        # update callibrary
        #if isinstance(context.observing_run, domain.ScantableList):
        #    self.update_callibrary(context)

    def update_callibrary(self, context):
        callib = context.callibrary
        apply_list = []
        for st in context.observing_run:
            ms = st.ms
            vis = ms.name
            basename = ms.basename
            antenna = st.antenna.name
            LOG.debug('Processing %s antenna %s'%(basename, antenna))
            tsys_strategy = st.calibration_strategy['tsys_strategy']
            spwmap = {}
            for pairs in tsys_strategy:
                tsysspw = pairs[0]
                targetspw = pairs[1]
                if not spwmap.has_key(tsysspw):
                    spwmap[tsysspw] = []
                spwmap[tsysspw].append(targetspw)
            calto = callibrary.CalTo(vis=vis,
                                     spw=','.join(map(str,[v[1] for v in tsys_strategy])),
                                     antenna=antenna,
                                     intent='TARGET,REFERENCE')
            calstate = callib.get_calstate(calto)
            calfroms = calstate.merged().values()[0]
            LOG.debug('There are %s calfrom objects %s'%(len(calfroms),map(lambda x: x.caltype, calfroms)))
            for calfrom in calfroms:
                if calfrom.caltype == 'tsys':
                    newcalfrom = callibrary.SDCalFrom(gaintable=calfrom.gaintable,
                                                      interp='',
                                                      spwmap=spwmap,
                                                      caltype=calfrom.caltype)
                    apply_list.append([calto, newcalfrom])
                else:
                    apply_list.append([calto, calfrom])

        context.callibrary.clear()
        for (calto, calfrom) in apply_list:
            context.callibrary.add(calto, calfrom)
            
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



