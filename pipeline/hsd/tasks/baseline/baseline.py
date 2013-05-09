from __future__ import absolute_import

import os
import math
#from math import cos, sqrt, exp
import numpy
import time

from taskinit import gentools

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.jobrequest as jobrequest
import pipeline.infrastructure.imagelibrary as imagelibrary
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
from .. import common
from .simplegrid import SimpleGridding
from .detection import DetectLine
from .validation import ValidateLine
from .fitting import FittingFactory

LOG = infrastructure.get_logger(__name__)
logging.set_logging_level('trace')
#logging.set_logging_level('info')

NoData = common.NoData

class SDBaselineInputs(common.SingleDishInputs):
    """
    Inputs for imaging
    """
    def __init__(self, context, infiles=None, iflist=None, pollist=None):
        self._init_properties(vars())

    @property
    def antennalist(self):
        if type(self.infiles) == list:
            antennas = [self.context.observing_run.get_scantable(f).antenna.name 
                        for f in self.infiles]
            return list(set(antennas))
        else:
            return [self.context.observing_run.get_scantable(self.infiles).antenna.name]

class SDBaselineResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDBaselineResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDBaselineResults, self).merge_with_context(context)
        LOG.todo('need to decide what is done in SDBaselineResults.merge_with_context')

    def _outcome_name(self):
        #return [image.imagename for image in self.outcome]
        return self.outcome.imagename

class SDBaseline(common.SingleDishTaskTemplate):
    Inputs = SDBaselineInputs

    def prepare(self):
        context = self.inputs.context
        datatable = context.observing_run.datatable_instance
        reduction_group = context.observing_run.reduction_group
        infiles = self.inputs.infiles
        iflist = self.inputs.iflist
        antennalist = self.inputs.antennalist
        pollist = self.inputs.pollist
        st_names = context.observing_run.st_names
        file_index = [st_names.index(infile) for infile in infiles]

        # task returns ResultsList
        results = basetask.ResultsList()

        # loop over reduction group
        for (group_id,group_desc) in reduction_group.items():
            # assume all members have same spw and pollist
            spwid = group_desc['member'][0][1]
            LOG.debug('spwid=%s'%(spwid))
            pols = group_desc['member'][0][2]
            if pollist is not None:
                pols = list(set(pollist).intersection(pols))

            # skip spw not included in iflist
            if iflist is not None and spwid not in iflist:
                LOG.debug('Skip spw %s'%(spwid))
                continue
                
            # reference data is first scantable 
            st = context.observing_run[indices[0]]

            # skip channel averaged spw
            nchan = group_desc['nchan']
            if nchan == 1:
                LOG.info('Skip channel averaged spw %s.'%(spwid))
                continue


        return results

    def analyse(self, result):
        return result


class SDBaselineWorker(object):
    SRCTYPE = {'ps': 0,
               'otf': 0,
               'otfraster': 0}

    def __init__(self):
        pass

    def execute(self, datatable, reference_data, spwid, pollist, srctype, file_index, window, edge, broadline, fitorder, fitfunc, observing_pattern, work_dir):
        # spectral window
        spw = reference_data.spectral_window[spwid]
        nchan = spw.nchan
        
        # beam size
        grid_size = casatools.quanta.convert(reference_data.beam_size[spwid], 'deg')['value']

        # simple gridding
        simple_grid = SimpleGridding(datatable, spwid, srctype, grid_size, 
                                     file_index, nplane=3)
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
        LOG.info('index_list=%s'%(list(index_list)))
        LOG.info('len(index_list)=%s'%(len(index_list)))
        lines = line_validator.execute(grid_table, detect_signal, spwid, nchan, index_list, window, observing_pattern, grid_size, grid_size, iteration)


        # fit order determination and fitting
        fitter_cls = FittingFactory.get_fitting_class(fitfunc)
        fitter = fitter_cls()
        
        # loop over file
        filenames = datatable.getkeyword('FILENAMES')
        for idx in file_index:
            #filename = os.path.join(context.output_dir, filenames[idx])
            filename = os.path.join(work_dir, filenames[idx])
            filename_out = filename.rstrip('/') + '_work'
            bltable_name = filename.rstrip('/') + '.product.tbl'
            createExportTable(bltable_name)
            with casatools.TableReader(filename) as tb:
                copied = tb.copy(filename_out, deep=True, valuecopy=True, returnobject=True)
                copied.close()
            ant_indices = numpy.where(antennas.take(index_list)==idx)[0]
            ant_indices = index_list.take(ant_indices)
            for pol in pollist:
                time_table = datatable.get_timetable(idx, spwid, pol)
                pol_indices = numpy.where(polnos.take(ant_indices)==pol)[0]
                pol_indices = ant_indices.take(pol_indices)
                LOG.info('pol_indices=%s'%(list(pol_indices)))
                fitter.execute(datatable, filename, filename_out, bltable_name, time_table, pol_indices, nchan, edge, fitorder)
        
        return lines
        
def createExportTable(table_name):
    table = gentools(['tb'])[0]

    desc = dict()

    desc['Row'] = {
        'comment': 'Row number',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'integer'
        }

    desc['Ant'] = {
        'comment': 'Antenna IDr',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'integer'
        }

    desc['FitFunc'] = {
        'comment': 'Baseline Fitting Function',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'string'
        }

    desc['SummaryFlag'] = {
        'comment': 'Summary Flag applied',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'boolean'
        }

    desc['Sections'] = {
        'comment': 'Spectral baseline section coefficients',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'integer',
        'ndim': 2
        }

    desc['LineMask'] = {
        'comment': 'Line detected region',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'integer',
        'ndim': 2
        }

    desc['SectionCoefficients'] = {
        'comment': 'Spectral baseline section coefficients',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'double',
        'ndim': 2
        }

    desc['Statistics'] = {
        'comment': 'Spectral baseline RMS',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'double',
        'ndim': 1
        }

    desc['StatisticsFlags'] = {
        'comment': 'Statistics Flags by category',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'boolean',
        'ndim': 1
        }

    desc['PermanentFlags'] = {
        'comment': 'Permanent Flags by category',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'boolean',
        'ndim': 1
        }

    table.create(table_name, tabledesc=desc)
    table.close()
    #return table




