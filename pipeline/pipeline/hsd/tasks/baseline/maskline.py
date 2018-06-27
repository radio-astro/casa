from __future__ import absolute_import

import os
import time
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
import pipeline.infrastructure.casatools as casatools
from pipeline.domain.datatable import DataTableImpl as DataTable
from pipeline.domain.datatable import DataTableIndexer
from . import simplegrid
from . import detection
from . import validation
from .. import common
from ..common import utils

_LOG = infrastructure.get_logger(__name__)
LOG = utils.OnDemandStringParseLogger(_LOG)

NoData = common.NoData

class MaskLineInputs(vdp.StandardInputs):
    window = vdp.VisDependentProperty(default=[])
    edge = vdp.VisDependentProperty(default=(0,0))
    broadline = vdp.VisDependentProperty(default=True)
    clusteringalgorithm = vdp.VisDependentProperty(default='kmean')
    
    @property
    def group_desc(self):
        return self.context.observing_run.ms_reduction_group[self.group_id]
    
    @property
    def reference_member(self):
        return self.group_desc[self.member_list[0]]

    def __init__(self, context, iteration, group_id, member_list, #vis_list, field_list, antenna_list, spwid_list,
                 window=None, edge=None, broadline=None, clusteringalgorithm=None):
        super(MaskLineInputs, self).__init__()
        
        self.context = context
        self.iteration = iteration
        self.group_id = group_id
        self.member_list = member_list
        self.window = window
        self.edge = edge
        self.broadline = broadline
        self.clusteringalgorithm = clusteringalgorithm
        

class MaskLineResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(MaskLineResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(MaskLineResults, self).merge_with_context(context)
        
    def _outcome_name(self):
        return ''


class MaskLine(basetask.StandardTaskTemplate):
    Inputs = MaskLineInputs

    def prepare(self):
        context = self.inputs.context

        start_time = time.time()

        iteration = self.inputs.iteration
        group_id = self.inputs.group_id
        member_list = self.inputs.member_list
        group_desc = self.inputs.group_desc 
        reference_member = self.inputs.reference_member
        reference_data = reference_member.ms
        reference_antenna = reference_member.antenna_id
        reference_field = reference_member.field_id
        reference_spw = reference_member.spw_id
        mses = context.observing_run.measurement_sets
        dt_dict = dict((ms.basename, DataTable(os.path.join(context.observing_run.ms_datatable_name, ms.basename))) 
                       for ms in mses)
        srctype = 0  # reference_data.calibration_strategy['srctype']
        t0 = time.time()
        index_dict = utils.get_index_list_for_ms3(dt_dict, group_desc, member_list, srctype)
        t1 = time.time()
        LOG.info('Elapsed time for generating index_dict: {0} sec'.format(t1 - t0))
        

        LOG.debug('index_dict={}', index_dict)
        # debugging
        t0 = time.time()
        indexer = DataTableIndexer(context)
        def _g():
            for ms in mses:
                if ms.basename in index_dict:
                    for i in index_dict[ms.basename]:
                        yield indexer.perms2serial(ms.basename, i)
        index_list = numpy.fromiter(_g(), dtype=numpy.int64)
        LOG.info('index_list={}', index_list)
        t1 = time.time()
        LOG.info('Elapsed time for generating index_list: {0} sec'.format(t1 - t0))
        # LOG.trace('all(spwid == {}) ? {}', spwid_list[0], numpy.all(dt.getcol('IF').take(index_list) == spwid_list[0]))
        # LOG.trace('all(fieldid == {}) ? {}', field_list[0], numpy.all(dt.getcol('FIELD_ID').take(index_list) == field_list[0]))
        if len(index_list) == 0:
            # No valid data 
            outcome = {'detected_lines': [],
                       'cluster_info': {},
                       'grid_table': None}
            result = MaskLineResults(task=self.__class__,
                                     success=True,
                                     outcome=outcome)
            result.task = self.__class__
                
            if self.inputs.context.subtask_counter is 0: 
                result.stage_number = self.inputs.context.task_counter - 1
            else:
                result.stage_number = self.inputs.context.task_counter 
            return result

        window = self.inputs.window
        edge = self.inputs.edge
        broadline = self.inputs.broadline
        clusteringalgorithm = self.inputs.clusteringalgorithm
        beam_size = casatools.quanta.convert(reference_data.beam_sizes[reference_antenna][reference_spw], 'deg')['value']
        observing_pattern = reference_data.observing_pattern[reference_antenna][reference_spw][reference_field]
         
        LOG.debug('Members to be processed:')
        for (m, f, a, s) in utils.iterate_group_member(group_desc, member_list):#itertools.izip(vis_list, field_list, antenna_list, spwid_list):
            v = m.name
            LOG.debug('MS "{}" Field {} Antenna {} Spw {}', os.path.basename(v), f, a, s)
             
        # filename for input/output
        # ms_list = [context.observing_run.get_ms(vis) for vis in vis_list]
        # filenames_work = [ms.work_data for ms in ms_list]
        # files_to_grid = dict(zip(file_index, filenames_work))
 
        # LOG.debug('files_to_grid=%s'%(files_to_grid))
                 
        # gridding size
        grid_size = beam_size
 
        # simple gridding
        t0 = time.time()
        gridding_inputs = simplegrid.SDSimpleGridding.Inputs(context, group_id, member_list, window)
        gridding_task = simplegrid.SDSimpleGridding(gridding_inputs)
        job = common.ParameterContainerJob(gridding_task, datatable_dict=dt_dict, index_list=index_list)
        gridding_result = self._executor.execute(job, merge=False)
        spectra = gridding_result.outcome['spectral_data']
        grid_table = gridding_result.outcome['grid_table']
        t1 = time.time()
 
        # return empty result if grid_table is empty
        if len(grid_table) == 0: # or len(spectra) == 0:
            LOG.warn('Line detection/validation will not be done since grid table is empty. Maybe all the data are flagged out in the previous step.')
            outcome = {'detected_lines': [],
                       'cluster_info': {},
                       'grid_table': None}
            result = MaskLineResults(task=self.__class__,
                                     success=True,
                                     outcome=outcome)
            result.task = self.__class__
                 
            return result
         
        LOG.trace('len(grid_table)={}, spectra.shape={}', len(grid_table), numpy.asarray(spectra).shape)
        LOG.trace('grid_table={}', grid_table)
        LOG.debug('PROFILE simplegrid: elapsed time is {} sec', t1 - t0)
 
        # line finding
        t0 = time.time()
        detection_inputs = detection.DetectLine.Inputs(context, group_id, window, edge, broadline)
        line_finder = detection.DetectLine(detection_inputs)
        job = common.ParameterContainerJob(line_finder, datatable_dict=dt_dict, grid_table=grid_table, 
                                           spectral_data=spectra)
        detection_result = self._executor.execute(job, merge=False)
        detect_signal = detection_result.signals
        t1 = time.time()
 
        LOG.trace('detect_signal={}', detect_signal)
        LOG.debug('PROFILE detection: elapsed time is {} sec', t1-t0)
 
        # line validation
        t0 = time.time()
        validator_cls = validation.ValidationFactory(observing_pattern)
        validation_inputs = validator_cls.Inputs(context, group_id, member_list, 
                                                 iteration, grid_size, 
                                                 grid_size, window, edge, 
                                                 clusteringalgorithm=clusteringalgorithm)
        line_validator = validator_cls(validation_inputs)
        LOG.trace('len(index_list)={}', len(index_list))
        job = common.ParameterContainerJob(line_validator, datatable_dict=dt_dict, index_list=index_list, 
                                           grid_table=grid_table, detect_signal=detect_signal)
        validation_result = self._executor.execute(job, merge=False)
        lines = validation_result.outcome['lines']
        if 'channelmap_range' in validation_result.outcome:
            channelmap_range = validation_result.outcome['channelmap_range']
        else:
            channelmap_range = validation_result.outcome['lines']
        cluster_info = validation_result.outcome['cluster_info']

        # export datatables
        for datatable in dt_dict.itervalues():
            datatable.exportdata(minimal=True)
        t1 = time.time()
 
        # LOG.debug('lines=%s'%(lines))
        LOG.debug('PROFILE validation: elapsed time is {} sec', t1-t0)
         
        # LOG.debug('cluster_info=%s'%(cluster_info))
 
        end_time = time.time()
        LOG.debug('PROFILE execute: elapsed time is {} sec', end_time-start_time)
 
        outcome = {'detected_lines': lines,
                   'channelmap_range': channelmap_range,
                   'cluster_info': cluster_info,
                   'grid_table': grid_table}
        result = MaskLineResults(task=self.__class__,
                                 success=True,
                                 outcome=outcome)
        result.task = self.__class__

        return result
        
    def analyse(self, result):
        return result
