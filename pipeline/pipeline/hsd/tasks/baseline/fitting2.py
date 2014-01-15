from __future__ import absolute_import

import os
import time
import re
import numpy

import asap as sd

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.sdfilenamer as filenamer
from pipeline.hsd.heuristics import fitorder, fragmentation
from .. import common
from . import utils
from .baselinetable import SplineBaselineTableGenerator
from .baselinetable import PolynomialBaselineTableGenerator

LOG = infrastructure.get_logger(__name__)

class FittingFactory(object):
    @staticmethod
    def get_fitting_class(fitfunc='spline'):
        if re.match('^C?SPLINE$', fitfunc.upper()):
            return CubicSplineFitting
#         elif re.match('^POLYNOMIAL$', fitfunc.upper()):
#             return PolynomialFitting
        else:
            return None
        
class FittingInputs(common.SingleDishInputs):
    def __init__(self, context, antennaid, spwid, pollist, iteration, 
                 fit_order=None, edge=None, outfile=None):
        self._init_properties(vars())
        self._bltable = None
        
    @property
    def edge(self):
        return (0,0) if self._edge is None else self._edge
    
    @edge.setter
    def edge(self, value):
        self._edge = value
        
    @property
    def fit_order(self):
        return 'automatic' if self._fit_order is None else self._fit_order
    
    @fit_order.setter
    def fit_order(self, value):
        self._fit_order = value
        
    @property
    def data_object(self):
        return self.context.observing_run[self.antennaid]
        
    @property
    def infile(self):
        return self.data_object.name
    
#     @property
#     def outfile(self):
#         return self.data_object.name + self.outfile_suffix
# #         return self.data_object.baselined_name
    
    @property
    def bltable(self):
        if self._bltable is None:
            namer = filenamer.BaselineSubtractedTable()
            namer.spectral_window(self.spwid)
            st = self.data_object
            asdm = common.asdm_name(st)
            namer.asdm(asdm)
            namer.antenna_name(st.antenna.name)
            self._bltable = namer.get_filename()
        return self._bltable
    
    @property
    def srctype(self):
        return self.data_object.calibration_strategy['srctype']
    
    @property
    def nchan(self):
        return self.data_object.spectral_window[self.spwid].nchan
                
class FittingResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(FittingResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(FittingResults, self).merge_with_context(context)
        
    def _outcome_name(self):
        return self.outcome


class FittingBase(common.SingleDishTaskTemplate):
    Inputs = FittingInputs

#class FittingBase(object):
    ApplicableDuration = 'raster' # 'raster' | 'subscan'
    MaxPolynomialOrder = 'none' # 'none', 0, 1, 2,...
    PolynomialOrder = 'automatic' # 'automatic', 0, 1, 2, ...
    ClipCycle = 1

    def __init__(self, inputs):
        super(FittingBase, self).__init__(inputs)
        self.fragmentation_heuristic = fragmentation.FragmentationHeuristics()
        
        # fitting order
        fit_order = self.inputs.fit_order
        if fit_order == 'automatic':
            # fit order heuristics
            LOG.info('Baseline-Fitting order was automatically determined')
            self.fitorder_heuristic = fitorder.FitOrderHeuristics()
        else:
            LOG.info('Baseline-Fitting order was fixed to be %d'%(fit_order))
            self.fitorder_heuristic = lambda *args, **kwargs: self.inputs.fit_order


    def prepare(self):
        """
        """
        datatable = self.datatable
        filename_in = self.inputs.infile
        filename_out = self.inputs.outfile
        iteration = self.inputs.iteration
        bltable_name = self.inputs.bltable
#         if iteration == 0:
#             utils.createExportTable(bltable_name)
            
        if not filename_out or len(filename_out) == 0:
            self.outfile = self.data_object.baselined_name
            LOG.debug("Using default output scantable name, %s" % self.outfile)
        
        if not os.path.exists(filename_out):
            raise RuntimeError, "Output scantable '%s' does not exist. It should be exist before you run this method." % filename_out
#         if iteration == 0 or not os.path.exists(filename_out):
#             with casatools.TableReader(filename_in) as tb:
#                 copied = tb.copy(filename_out, deep=True, valuecopy=True, returnobject=True)
#                 copied.close()    

        #time_table = self.time_table
        #index_list = self.index_list
        antennaid = self.inputs.antennaid
        spwid = self.inputs.spwid
        pollist = self.inputs.pollist
        nchan = self.inputs.nchan
        srctype = self.inputs.srctype
        edge = common.parseEdge(self.inputs.edge)
        fit_order = self.inputs.fit_order

        if self.ApplicableDuration == 'subscan':
            timetable_index = 1
        else:
            timetable_index = 0
            
        ifnos = numpy.array(datatable.getcol('IF'))
        polnos = numpy.array(datatable.getcol('POL'))
        srctypes = numpy.array(datatable.getcol('SRCTYPE'))
        antennas = numpy.array(datatable.getcol('ANTENNA'))
        
        rows_to_process = numpy.where(numpy.logical_and(antennas == antennaid, 
                                                   numpy.logical_and(ifnos == spwid, srctypes==srctype)))[0]

#         blfile = filename_in.rstrip('/')+'.baseline.table'
#         os.system('rm -rf %s'%(blfile))

        # dummy scantable for baseline subtraction
#         dummy_scan = utils.create_dummy_scan(filename_in, datatable, rows_to_process)
        
        _polnos = polnos.take(rows_to_process)
        index_list_total = []
        row_list_total = []
        blinfo = []

        for pol in pollist:
            time_table = datatable.get_timetable(antennaid, spwid, pol)
            member_list = time_table[timetable_index]

            index_list_per_pol = rows_to_process.take(numpy.where(_polnos==pol)[0])

            # working with spectral data in scantable
            nrow_total = len(index_list_per_pol)
                
#             # Create progress timer
#             Timer = common.ProgressTimer(80, nrow_total, LOG.level)
    
            LOG.info('Calculating Baseline Fitting Parameter...')
            LOG.info('Baseline Fit: background subtraction...')
            LOG.info('Processing %d spectra...'%(nrow_total))
    
            mask_array = numpy.ones(nchan, dtype=int)
            mask_array[:edge[0]] = 0
            mask_array[nchan-edge[1]:] = 0
    
            for y in xrange(len(member_list)):
                rows = member_list[y][0]
                idxs = member_list[y][1]
                #LOG.debug('rows=%s'%(rows))
                with casatools.TableReader(filename_in) as tb:
                    spectra = numpy.array([tb.getcell('SPECTRA',row)
                                           for row in rows])
                spectra[:,:edge[0]] = 0.0
                spectra[:,nchan-edge[1]:] = 0.0 
                masklist = [datatable.tb2.getcell('MASKLIST',idx)
                            for idx in idxs]
    
                # fit order determination
                polyorder = self.fitorder_heuristic(spectra, masklist, edge)
                if fit_order == 'automatic' and self.MaxPolynomialOrder != 'none':
                    polyorder = min(polyorder, self.MaxPolynomialOrder)
                LOG.info('time group %d: fitting order=%s'%(y,polyorder))
    
                # calculate fragmentation
                (fragment, nwindow, win_polyorder) = self.fragmentation_heuristic(polyorder, nchan, edge)
    
                nrow = len(rows)
                LOG.debug('nrow = %s'%(nrow))
                LOG.debug('len(idxs) = %s'%(len(idxs)))
#                 updated = []
                
                index_list = []
                row_list = []

                for i in xrange(nrow):
                    row = rows[i]
                    idx = idxs[i]
                    LOG.trace('===== Processing at row = %s ====='%(row))
                    nochange = datatable.tb2.getcell('NOCHANGE',idx)
                    LOG.trace('row = %s, Flag = %s'%(row, nochange))
    
#                     # skip if no update on line window
#                     if nochange > 0:
#                         continue
    
                    # data to be fitted
#                     sp = spectra[i]
    
                    # mask lines
                    maxwidth = 1
                    #masklist = datatable.getcell('MASKLIST',idx)
                    _masklist = masklist[i] 
                    for [chan0, chan1] in _masklist:
                        if chan1 - chan0 >= maxwidth:
                            maxwidth = int((chan1 - chan0 + 1) / 1.4)
                            # allowance in Process3 is 1/5:
                            #    (1 + 1/5 + 1/5)^(-1) = (5/7)^(-1)
                            #                         = 7/5 = 1.4
                    max_polyorder = int((nchan - sum(edge)) / maxwidth + 1)
                    LOG.trace('Masked Region from previous processes = %s'%(_masklist))
                    LOG.trace('edge parameters= (%s,%s)'%(edge))
                    LOG.trace('Polynomial order = %d  Max Polynomial order = %d'%(polyorder, max_polyorder))
    
                    # fitting
                    polyorder = min(polyorder, max_polyorder)
                    mask_array[edge[0]:nchan-edge[1]] = 1
                    irow = len(row_list_total)+len(row_list)
                    param = self._calc_baseline_param(irow, polyorder, nchan, 0, edge, _masklist, win_polyorder, fragment, nwindow, mask_array)
                    blinfo.append(param)
                    index_list.append(idx)
                    row_list.append(row)

                index_list_total.extend(index_list)
                row_list_total.extend(row_list)

        # subtract baseline
        LOG.info('Baseline Fit: background subtraction...')
        LOG.info('Processing %d spectra...'%(len(row_list_total)))
        LOG.info('rows = %s' % str(row_list_total))
        st_out = sd.scantable(filename_out, average=False)
        LOG.info('number of rows in scantable = %d' % st_out.nrow())
        st_out.set_selection(rows=row_list_total)
        LOG.info('number of rows in selected = %d' % st_out.nrow())
        LOG.info('BLINFO=%s' % (str(blinfo)))
        st_out.sub_baseline(insitu=True, retfitres=False, blinfo=blinfo, bltable=bltable_name, overwrite=True)
        st_out.set_selection()
        st_out.save(filename_out, format='ASAP', overwrite=True)
        
        outcome = {'bltable': bltable_name,
                   'index_list': index_list_total,
                   'outtable': filename_out}
        result = FittingResults(task=self.__class__,
                                success=True,
                                outcome=outcome)
                
        if self.inputs.context.subtask_counter is 0: 
            result.stage_number = self.inputs.context.task_counter - 1
        else:
            result.stage_number = self.inputs.context.task_counter 
                
        return result

                
    def analyse(self, result):
        bltable = result.outcome['bltable']
        index_list = result.outcome['index_list']
        stname = self.inputs.data_object.name
        spwid = self.inputs.spwid
        iteration = self.inputs.iteration
        edge = common.parseEdge(self.inputs.edge)
        datatable = self.datatable
        nchan = self.inputs.nchan
        FittingSummary.summary(bltable, stname, spwid, iteration, edge, datatable, index_list, nchan)
        return result

    def _calc_baseline_param(self, row_idx, polyorder, nchan, modification, edge, masklist, win_polyorder, fragment, nwindow, mask):
        # set edge mask
        #data[:edge[0]] = 0.0
        #data[nchan-edge[1]:] = 0.0
        
        # Create mask for line protection
        nchan_without_edge = nchan - sum(edge)
        #mask = numpy.ones(nchan, dtype=int)
        if type(masklist) == list or type(masklist) == numpy.ndarray:
            for [m0, m1] in masklist:
                mask[m0:m1] = 0
        else:
            LOG.critical('Invalid masklist')
        #if edge[1] > 0:
        #    nmask = int(nchan_without_edge - numpy.sum(mask[edge[0]:-edge[1]] * 1.0))
        #else:
        #    nmask = int(nchan_without_edge - numpy.sum(mask[edge[0]:] * 1.0))
        num_mask = int(nchan_without_edge - numpy.sum(mask[edge[0]:nchan-edge[1]] * 1.0))
        masklist_all = self._mask_to_masklist(mask)

        LOG.trace('nchan_without_edge, num_mask, diff=%s, %s'%(nchan_without_edge, num_mask))

        outdata = self._get_param(row_idx, polyorder, nchan, mask, edge, nchan_without_edge, num_mask, fragment, nwindow, win_polyorder, masklist_all)

        return outdata
    
    def _mask_to_masklist(self, mask):
        """
        Converts mask array to masklist
        
        Argument
            mask : an array of channel mask in values 0 (rejected) or 1 (adopted)
        """
        nchan = len(mask)
        istart = []
        iend = []
        if mask[0] == 1:
            istart = [0]
        for ichan in range(1, nchan):
            switch = mask[ichan] - mask[ichan-1]
            if switch == 0:
                continue
            elif switch == 1:
                # start of mask channels (0 -> 1)
                istart.append(ichan)
            elif switch == -1:
                # end of mask channels (1 -> 0)
                iend.append(ichan-1)
        if mask[nchan-1] == 1:
            iend.append(nchan-1)
        if len(istart) != len(iend):
            raise RuntimeError, "Failed to get mask ranges. The lenght of start channels and end channels do not match."
        masklist = []
        for irange in range(len(istart)):
            if istart[irange] > iend[irange]:
                raise RuntimeError, "Failed to get mask ranges. A start channel index is larger than end channel."
            masklist.append([istart[irange], iend[irange]])
        return masklist

class CubicSplineFitting(FittingBase):
    def _get_param(self, idx, polyorder, nchan, mask, edge, nchan_without_edge, nchan_masked, fragment, nwindow, win_polyorder, masklist):
        num_nomask = nchan_without_edge - nchan_masked
        num_pieces = max(int(min(polyorder * num_nomask / float(nchan_without_edge) + 0.5, 0.1 * num_nomask)), 1)
        LOG.trace('Cubic Spline Fit: Number of Sections = %d' % num_pieces)
        return {'row': idx, 'masklist': masklist, 'npiece': num_pieces, 'blfunc': 'cspline', 'clipthresh': 5.0, 'clipniter': self.ClipCycle}


class FittingSummary(object):
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
            
        FittingSummary.cspline_summary(tablename)

        footer = separator
        LOG.info(footer)

    @staticmethod
    def cspline_summary(tablename):
        # number of segments for cspline_baseline
        with casatools.TableReader(tablename) as tb:
            nrow = tb.nrows()
#             num_segments = [tb.getcell('Sections', irow).shape[0] \
#                             for irow in xrange(nrow)]
            num_segments = [( len(tb.getcell('FUNC_PARAM', irow)) - 1 ) \
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
