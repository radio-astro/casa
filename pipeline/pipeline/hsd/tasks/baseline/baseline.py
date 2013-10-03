from __future__ import absolute_import

import os
import math
#from math import cos, sqrt, exp
import numpy
import time

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.casatools as casatools
from pipeline.domain import DataTable
from .. import common
from . import maskline
from .fitting import FittingFactory
from . import utils

LOG = infrastructure.get_logger(__name__)

class SDBaselineInputs(common.SingleDishInputs):
    """
    Inputs for imaging
    """
    def __init__(self, context, infiles=None, iflist=None, pollist=None,
                 linewindow=None, edge=None, broadline=None, fitorder=None,
                 fitfunc=None):
        self._init_properties(vars())
        self._to_list(['infiles', 'iflist', 'pollist', 'edge', 'linewindow'])
        self._to_bool('broadline')
        self._to_numeric('fitorder')
        if isinstance(self.fitorder, float):
            self.fitorder = int(self.fitorder)

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

        # replace and export datatable to merge updated data with context
        datatable = self.outcome.pop('datatable')
        datatable.exportdata(minimal=True)

        context.observing_run.datatable_instance = datatable
        
        # increment iteration counter
        # register detected lines to reduction group member
        reduction_group = context.observing_run.reduction_group
        for b in self.outcome['baselined']:
            spw = b['spw']
            antenna = b['index']
            pols = b['pols']
            lines = b['lines']
            for _ant in antenna:
                group_id = -1
                for (idx,desc) in reduction_group.items():
                    if desc[0].spw == spw:
                        group_id = idx
                        break
                if group_id >= 0:
                    reduction_group[group_id].iter_countup(_ant, spw, pols)
                    reduction_group[group_id].add_linelist(lines, _ant, spw, pols)
                st = context.observing_run[_ant]
                st.work_data = st.baselined_name

    def _outcome_name(self):
        return ['%s: %s (spw=%s, pol=%s)'%(idx, name, b['spw'], b['pols'])
                for b in self.outcome['baselined']
                for (idx,name) in zip(b['index'], b['name'])]

class SDBaseline(common.SingleDishTaskTemplate):
    Inputs = SDBaselineInputs

    def prepare(self):
        inputs = self.inputs
        context = inputs.context
        datatable = context.observing_run.datatable_instance
        reduction_group = context.observing_run.reduction_group
        infiles = inputs.infiles
        iflist = inputs.iflist
        #antennalist = inputs.antennalist
        pollist = inputs.pollist
        st_names = context.observing_run.st_names
        file_index = [st_names.index(infile) for infile in infiles]

        window = [] if inputs.linewindow is None else inputs.linewindow
        edge = (0,0) if inputs.edge is None else inputs.edge
        broadline = False if inputs.broadline is None else inputs.broadline
        fitorder = 'automatic' if inputs.fitorder is None or inputs.fitorder < 0 else inputs.fitorder
        fitfunc = 'spline' if inputs.fitfunc is None else inputs.fitfunc
        
        baselined = []

        ifnos = numpy.array(datatable.getcol('IF'))
        polnos = numpy.array(datatable.getcol('POL'))
        srctypes = numpy.array(datatable.getcol('SRCTYPE'))
        antennas = numpy.array(datatable.getcol('ANTENNA'))

        # loop over reduction group
        files = set()
        for (group_id,group_desc) in reduction_group.items():            
            # assume all members have same spw and pollist
            first_member = group_desc[0]
            spwid = first_member.spw
            LOG.debug('spwid=%s'%(spwid))
            pols = first_member.pols
            iteration = first_member.iteration[0]
            if pollist is not None:
                pols = list(set(pollist) & set(pols))

            # skip spw not included in iflist
            if iflist is not None and spwid not in iflist:
                LOG.debug('Skip spw %s'%(spwid))
                continue

            # reference data is first scantable 
            st = context.observing_run[first_member.antenna]

            # skip channel averaged spw
            nchan = group_desc.nchan
            if nchan == 1:
                LOG.info('Skip channel averaged spw %s.'%(spwid))
                continue

                
            #beam_size = st.beam_size[spwid]
            srctype = st.calibration_strategy['srctype']
            _file_index = set(file_index) & set([m.antenna for m in group_desc])
            files = files | _file_index
            #pattern = st.pattern[spwid][pols[0]]
            index_list = numpy.where(numpy.logical_and(ifnos == spwid, srctypes==srctype))[0]
            maskline_inputs = maskline.MaskLine.Inputs(context, list(_file_index), spwid, iteration, 
                                                       index_list, window, edge, broadline)
            maskline_task = maskline.MaskLine(maskline_inputs)
            maskline_result = self._executor.execute(maskline_task, merge=True)
            #worker = SDBaselineWorker(context, iteration, spwid, nchan, beam_size, srctype, list(_file_index), index_list, window, edge, broadline, pattern)
            #(detected_lines, cluster_info) = self._executor.execute(worker, merge=False)
            detected_lines = maskline_result.outcome['detected_lines']
            cluster_info = maskline_result.outcome['cluster_info']
            #datatable.importdata(datatable.plaintable, minimal=False)
            datatable = DataTable(datatable.plaintable)

            #LOG.info('detected_lines=%s'%(detected_lines))
            #LOG.info('cluster_info=%s'%(cluster_info))

            # filenamer
            namer = filenamer.BaselineSubtractedTable()
            namer.spectral_window(spwid)

            # fit order determination and fitting
            fitter_cls = FittingFactory.get_fitting_class(fitfunc)
            fitter = fitter_cls(datatable)

            # loop over file
            for idx in _file_index:
                st = context.observing_run[idx]
                filename_in = st.name
                filename_out = st.baselined_name
                if not os.path.exists(filename_out):
                    with casatools.TableReader(filename_in) as tb:
                        copied = tb.copy(filename_out, deep=True, valuecopy=True, returnobject=True)
                        copied.close()
                asdm = common.asdm_name(st)
                namer.asdm(asdm)
                namer.antenna_name(st.antenna.name)
                bltable_name = namer.get_filename()
                iteration = group_desc.get_iteration(idx, spwid)
                LOG.debug('iteration (antenna %s, spw %s): %s'%(idx,spwid,iteration))
                if iteration == 0:
                    utils.createExportTable(bltable_name)
                ant_indices = numpy.where(antennas.take(index_list)==idx)[0]
                ant_indices = index_list.take(ant_indices)
                for pol in pols:
                    time_table = datatable.get_timetable(idx, spwid, pol)
                    pol_indices = numpy.where(polnos.take(ant_indices)==pol)[0]
                    pol_indices = ant_indices.take(pol_indices)
                    #LOG.debug('pol_indices=%s'%(list(pol_indices)))
                    t0 = time.time()
                    fitter.setup(filename_in, filename_out, bltable_name, time_table, pol_indices, nchan, edge, fitorder)
                    self._executor.execute(fitter, merge=False)
                    t1 = time.time()
                    LOG.debug('PROFILE baseline ant%s spw%s pol%s: elapsed time is %s sec'%(idx,spwid,pol,t1-t0))

                # output summary of baseline fitting
                BaselineSummary.summary(bltable_name, st.basename, spwid, iteration, edge, datatable, ant_indices, nchan)


            #for f in _file_index:
            #    name = context.observing_run[f].baselined_name
            name_list = [context.observing_run[f].baselined_name
                         for f in _file_index]
            baselined.append({'name': name_list, 'index': list(_file_index),
                              'spw': spwid, 'pols': pols,
                              'lines': detected_lines,
                              'clusters': cluster_info})

        outcome = {'datatable': datatable,
                   'baselined': baselined,
                   'edge': edge}
        results = SDBaselineResults(task=self.__class__,
                                    success=True,
                                    outcome=outcome)
                
        if self.inputs.context.subtask_counter is 0: 
            results.stage_number = self.inputs.context.task_counter - 1
        else:
            results.stage_number = self.inputs.context.task_counter 
                
        return results

    def analyse(self, result):
        return result


class BaselineSummary(object):
    @staticmethod
    def summary(tablename, stname, spw, iteration, edge, datatable, index_list, nchan):
        header = 'Summary of cspline_baseline for %s (spw%s, iter%s)'%(stname, spw, iteration)
        separator = '=' * len(header)
        LOG.info(separator)
        LOG.info(header)
        LOG.info(separator)

        # edge channels dropped
        LOG.info('1) Number of edge channels dropped')
        LOG.info('')
        LOG.info('\t left edge: %s channels'%(edge[0]))
        LOG.info('\tright edge: %s channels'%(edge[1]))
        LOG.info('')

        # line masks
        LOG.info('2) Masked fraction on each channel')
        LOG.info('')
        histogram = numpy.zeros(nchan, dtype=float)
        nrow = len(index_list)
        for idx in index_list:
            masklist = datatable.getcell('MASKLIST', idx)
            for mask in masklist:
                start = mask[0]
                end = mask[1] + 1
                for ichan in xrange(start, end):
                    histogram[ichan] += 1.0
        nonzero_channels = histogram.nonzero()[0]
        if len(nonzero_channels) > 0:
            dnz = nonzero_channels[1:] - nonzero_channels[:-1]
            mask_edges = numpy.where(dnz > 1)[0]
            start_chan = nonzero_channels.take([0]+(mask_edges+1).tolist())
            end_chan = nonzero_channels.take(mask_edges.tolist()+[-1])
            merged_start_chan = [start_chan[0]]
            merged_end_chan = []
            for i in xrange(1, len(start_chan)):
                if start_chan[i] - end_chan[i-1] > 4:
                    merged_start_chan.append(start_chan[i])
                    merged_end_chan.append(end_chan[i-1])
            merged_end_chan.append(end_chan[-1])
            LOG.info('channel|fraction')
            LOG.info('-------|---------')
            if merged_start_chan[0] > 0:
                LOG.info('%7d|%9.1f%%'%(0, 0))
                LOG.info('       ~')
                LOG.info('       ~')
            #for ichan in xrange(len(histogram)):
            for i in xrange(len(merged_start_chan)):
                for j in xrange(max(0,merged_start_chan[i]-1), min(nchan,merged_end_chan[i]+2)):
                    LOG.info('%7d|%9.1f%%'%(j, histogram[j]/nrow*100.0))
                if merged_end_chan[i] < nchan-2:
                    LOG.info('       ~')
                    LOG.info('       ~')
            if merged_end_chan[-1] < nchan-2:
                LOG.info('%7d|%9.1f%%'%(nchan-1, 0))
        else:
            LOG.info('\tNo line mask')
        LOG.info('')
            
        BaselineSummary.cspline_summary(tablename)

        footer = separator
        LOG.info(footer)

    @staticmethod
    def cspline_summary(tablename):
        # number of segments for cspline_baseline
        with casatools.TableReader(tablename) as tb:
            nrow = tb.nrows()
            num_segments = [tb.getcell('Sections', irow).shape[0] \
                            for irow in xrange(nrow)]
        unique_values = numpy.unique(num_segments)
        max_segments = max(unique_values) + 2
        LOG.info('3) Frequency distribution for number of segments')
        LOG.info('')
        LOG.info('# of segments|frequency')
        LOG.info('-------------|---------')
        #for val in unique_values:
        for val in xrange(1, max_segments):
            count = num_segments.count(val)
            LOG.info('%13d|%9d'%(val, count))
        LOG.info('')

