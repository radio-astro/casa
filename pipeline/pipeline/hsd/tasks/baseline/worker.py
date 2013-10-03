from __future__ import absolute_import

import os
import math
#from math import cos, sqrt, exp
import numpy
import time

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.imagelibrary as imagelibrary
import pipeline.infrastructure.basetask as basetask
from . import simplegrid
from . import detection
from . import validation
from .fitting import FittingFactory
from . import utils
from .. import common

LOG = infrastructure.get_logger(__name__)

class SDBaselineWorker(object):
    def __init__(self, context, iteration, spwid, nchan, beam_size, srctype, file_index, index_list, window, edge, broadline, observing_pattern):
        self.context = context
        self.datatable = self.context.observing_run.datatable_instance
        self.iteration = iteration
        self.spwid = spwid
        self.nchan = nchan
        self.beam_size = beam_size
        self.srctype = srctype
        self.file_index = file_index
        self.index_list = index_list
        self.window = window
        self.edge = edge
        self.broadline = broadline
        self.observing_pattern = observing_pattern

    def execute(self, dry_run=True):

        if dry_run:
            return [], {}
        
        context = self.context

        self._executor = basetask.Executor(context, dry_run)

        start_time = time.time()

        datatable = self.datatable
        iteration = self.iteration
        spwid = self.spwid
        nchan = self.nchan
        beam_size = self.beam_size
        file_index = self.file_index
        index_list = self.index_list
        window = self.window
        edge = self.edge
        broadline = self.broadline
        observing_pattern = self.observing_pattern
        
        # filename for input/output
        filenames_work = [self.context.observing_run[idx].work_data
                          for idx in file_index]
        files_to_grid = dict(zip(file_index, filenames_work))

        LOG.debug('files_to_grid=%s'%(files_to_grid))
        
        # spectral window
        #spw = reference_data.spectral_window[spwid]
        #nchan = spw.nchan
        
        # beam size
        #grid_size = casatools.quanta.convert(reference_data.beam_size[spwid], 'deg')['value']
        grid_size = casatools.quanta.convert(beam_size, 'deg')['value']

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
            return [], {}
        
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

        return lines, cluster_info

