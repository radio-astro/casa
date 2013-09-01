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
from .simplegrid import SimpleGridding
from .detection import DetectLine
from .validation import ValidateLine
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

        start_time = time.time()

        datatable = self.datatable
        iteration = self.iteration
        spwid = self.spwid
        nchan = self.nchan
        beam_size = self.beam_size
        srctype = self.srctype
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
        simple_grid = SimpleGridding(datatable, spwid, srctype, grid_size, 
                                     files_to_grid, nplane=3)
        (spectra, grid_table) = simple_grid.execute()
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
        line_finder = DetectLine(datatable)
        detect_signal = line_finder.execute(grid_table, spectra, window, edge, broadline)
        t1 = time.time()

        LOG.trace('detect_signal=%s'%(detect_signal))
        LOG.debug('PROFILE detection: elapsed time is %s sec'%(t1-t0))

        # line validation
        t0 = time.time()
        line_validator = ValidateLine(datatable)
        #validator_class = ValidationFactory(observing_pattern)
        #line_validator = validator_class(datatable)
        LOG.trace('len(index_list)=%s'%(len(index_list)))
        lines, cluster_info = line_validator.execute(grid_table, detect_signal, spwid, nchan, index_list, window, observing_pattern, grid_size, grid_size, iteration)
        t1 = time.time()

        #LOG.debug('lines=%s'%(lines))
        LOG.debug('PROFILE validation: elapsed time is %s sec'%(t1-t0))
        
        #LOG.debug('cluster_info=%s'%(cluster_info))

        end_time = time.time()
        LOG.debug('PROFILE execute: elapsed time is %s sec'%(end_time-start_time))

        return lines, cluster_info

