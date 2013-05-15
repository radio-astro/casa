from __future__ import absolute_import

import os
import math
#from math import cos, sqrt, exp
import numpy
import time

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.jobrequest as jobrequest
import pipeline.infrastructure.imagelibrary as imagelibrary
import pipeline.infrastructure.basetask as basetask
#import pipeline.infrastructure.logging as logging
from .simplegrid import SimpleGridding
from .detection import DetectLine
from .validation import ValidateLine
from .fitting import FittingFactory
from . import utils

LOG = infrastructure.get_logger(__name__)
#logging.set_logging_level('trace')
#logging.set_logging_level('info')

class SDBaselineWorker(object):
    def __init__(self, context):
        self.context = context

    def execute(self, datatable, spwid, nchan, beam_size, pollist, srctype, file_index, window, edge, broadline, fitorder, fitfunc, observing_pattern):

        # filename for input/output
        filenames_out = [self.context.observing_run[idx].baselined_name
                         for idx in file_index]
        filenames_in = [self.context.observing_run[idx].name
                        for idx in file_index]
        filenames_out = dict(zip(file_index, filenames_out))
        filenames_in = dict(zip(file_index, filenames_in))
        filenames_grd = [filenames_out[idx] if os.path.exists(filenames_out[idx])
                        else filenames_in[idx] for idx in file_index]
        filenames_grd = dict(zip(file_index, filenames_grd))

        LOG.debug('filenames_grd=%s'%(filenames_grd))
        
        # spectral window
        #spw = reference_data.spectral_window[spwid]
        #nchan = spw.nchan
        
        # beam size
        #grid_size = casatools.quanta.convert(reference_data.beam_size[spwid], 'deg')['value']
        grid_size = casatools.quanta.convert(beam_size, 'deg')['value']

        # simple gridding
        simple_grid = SimpleGridding(datatable, spwid, srctype, grid_size, 
                                     filenames_grd, nplane=3)
        (spectra, grid_table) = simple_grid.execute()

        LOG.info('len(grid_table)=%s, spectra.shape=%s'%(len(grid_table),list(spectra.shape)))
        LOG.info('grid_table=%s'%(grid_table))
        #LOG.info('spectra=%s'%(spectra.tolist()))

        # line finding
        line_finder = DetectLine(datatable)
        detect_signal = line_finder.execute(grid_table, spectra, window, edge, broadline)

        LOG.info('detect_signal=%s'%(detect_signal))

        # line validation
        line_validator = ValidateLine(datatable)
        iteration = 1
        ifnos = numpy.array(datatable.getcol('IF'))
        polnos = numpy.array(datatable.getcol('POL'))
        srctypes = numpy.array(datatable.getcol('SRCTYPE'))
        antennas = numpy.array(datatable.getcol('ANTENNA'))
        index_list = numpy.where(numpy.logical_and(ifnos == spwid, srctypes==srctype))[0]
        LOG.debug('index_list=%s'%(list(index_list)))
        LOG.debug('len(index_list)=%s'%(len(index_list)))
        lines = line_validator.execute(grid_table, detect_signal, spwid, nchan, index_list, window, observing_pattern, grid_size, grid_size, iteration)


        # fit order determination and fitting
        fitter_cls = FittingFactory.get_fitting_class(fitfunc)
        fitter = fitter_cls()
        
        # loop over file
        for idx in file_index:
            filename_in = filenames_in[idx]
            filename_out = filenames_out[idx]
            if not os.path.exists(filename_out):
                with casatools.TableReader(filename_in) as tb:
                    copied = tb.copy(filename_out, deep=True, valuecopy=True, returnobject=True)
                    copied.close()
            bltable_name = self.context.observing_run[idx].name.rstrip('/') + '.product.tbl'
            #if not os.path.exists(bltable_name):
            utils.createExportTable(bltable_name)
            ant_indices = numpy.where(antennas.take(index_list)==idx)[0]
            ant_indices = index_list.take(ant_indices)
            for pol in pollist:
                time_table = datatable.get_timetable(idx, spwid, pol)
                pol_indices = numpy.where(polnos.take(ant_indices)==pol)[0]
                pol_indices = ant_indices.take(pol_indices)
                LOG.info('pol_indices=%s'%(list(pol_indices)))
                fitter.execute(datatable, filename_in, filename_out, bltable_name, time_table, pol_indices, nchan, edge, fitorder)

        
