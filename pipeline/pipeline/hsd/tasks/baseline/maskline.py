from __future__ import absolute_import

import time

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from . import simplegrid
from . import detection
from . import validation
from .. import common

LOG = infrastructure.get_logger(__name__)

class MaskLineInputs(common.SingleDishInputs):
    def __init__(self, context, antennalist, spwid, iteration, index_list, 
                 window=None, edge=None, broadline=None):
        self._init_properties(vars())
        
    @property
    def window(self):
        return [] if self._window is None else self._window
    
    @window.setter
    def window(self, value):
        self._window = value
        
    @property
    def edge(self):
        return (0,0) if self._edge is None else self._edge
    
    @edge.setter
    def edge(self, value):
        self._edge = value
        
    @property
    def broadline(self):
        return False if self._broadline is None else self._broadline
    
    @broadline.setter
    def broadline(self, value):
        self._broadline = value
        
class MaskLineResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(MaskLineResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(MaskLineResults, self).merge_with_context(context)
        
    def _outcome_name(self):
        return ''


class MaskLine(common.SingleDishTaskTemplate):
    Inputs = MaskLineInputs

    def prepare(self):
        context = self.context

        start_time = time.time()

        iteration = self.inputs.iteration
        spwid = self.inputs.spwid
        file_index = self.inputs.antennalist
        index_list = self.inputs.index_list
        window = self.inputs.window
        edge = self.inputs.edge
        broadline = self.inputs.broadline
        reference_data = context.observing_run[file_index[0]]
        beam_size = casatools.quanta.convert(reference_data.beam_size[spwid], 'deg')['value']
        observing_pattern = reference_data.pattern[spwid][0]
        
        # filename for input/output
        filenames_work = [self.context.observing_run[idx].work_data
                          for idx in file_index]
        files_to_grid = dict(zip(file_index, filenames_work))

        LOG.debug('files_to_grid=%s'%(files_to_grid))
                
        # beam size
        grid_size = beam_size

        # simple gridding
        t0 = time.time()
        gridding_inputs = simplegrid.SimpleGridding.Inputs(context, file_index, spwid)
        gridding_task = simplegrid.SimpleGridding(gridding_inputs)
        gridding_result = self._executor.execute(gridding_task, merge=True)
        spectra = gridding_result.outcome['spectral_data']
        grid_table = gridding_result.outcome['grid_table']
        t1 = time.time()

        # return empty result if grid_table is empty
        if len(grid_table) == 0:
            LOG.warn('Line detection/validation will not be done since grid table is empty. Maybe all the data are flagged out in the previous step.')
            outcome = {'detected_lines': [],
                       'cluster_info': {}}
            result = MaskLineResults(task=self.__class__,
                                     success=True,
                                     outcome=outcome)
            result.task = self.__class__
                
            if self.inputs.context.subtask_counter is 0: 
                result.stage_number = self.inputs.context.task_counter - 1
            else:
                result.stage_number = self.inputs.context.task_counter 
            return result
        
        LOG.trace('len(grid_table)=%s, spectra.shape=%s'%(len(grid_table),list(spectra.shape)))
        LOG.trace('grid_table=%s'%(grid_table))
        LOG.debug('PROFILE simplegrid: elapsed time is %s sec'%(t1-t0))

        # line finding
        t0 = time.time()
        detection_inputs = detection.DetectLine.Inputs(context, grid_table, spectra, window, edge, broadline)
        line_finder = detection.DetectLine(detection_inputs)
        detection_result = self._executor.execute(line_finder, merge=True)
        detect_signal = detection_result.outcome
        t1 = time.time()

        LOG.trace('detect_signal=%s'%(detect_signal))
        LOG.debug('PROFILE detection: elapsed time is %s sec'%(t1-t0))

        # line validation
        t0 = time.time()
        validator_cls = validation.ValidationFactory(observing_pattern)
        validation_inputs = validator_cls.Inputs(context, grid_table, detect_signal, spwid, index_list, iteration, grid_size, grid_size, window, edge)
        line_validator = validator_cls(validation_inputs)
        LOG.trace('len(index_list)=%s'%(len(index_list)))
        validation_result = self._executor.execute(line_validator, merge=True)
        lines = validation_result.outcome['lines']
        cluster_info = validation_result.outcome['cluster_info']
        t1 = time.time()

        #LOG.debug('lines=%s'%(lines))
        LOG.debug('PROFILE validation: elapsed time is %s sec'%(t1-t0))
        
        #LOG.debug('cluster_info=%s'%(cluster_info))

        end_time = time.time()
        LOG.debug('PROFILE execute: elapsed time is %s sec'%(end_time-start_time))

        outcome = {'detected_lines': lines,
                   'cluster_info': cluster_info}
        result = MaskLineResults(task=self.__class__,
                                  success=True,
                                  outcome=outcome)
        result.task = self.__class__
                
        if self.inputs.context.subtask_counter is 0: 
            result.stage_number = self.inputs.context.task_counter - 1
        else:
            result.stage_number = self.inputs.context.task_counter 
                
        return result
    
    def analyse(self, result):
        return result

