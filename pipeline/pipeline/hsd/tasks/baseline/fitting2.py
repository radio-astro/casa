from __future__ import absolute_import

import os
import time
import re
import numpy

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
                 fit_order=None, edge=None):
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
    
    @property
    def outfile(self):
        return self.data_object.baselined_name
    
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
        if iteration == 0:
            utils.createExportTable(bltable_name)
            
        if iteration == 0 or not os.path.exists(filename_out):
            with casatools.TableReader(filename_in) as tb:
                copied = tb.copy(filename_out, deep=True, valuecopy=True, returnobject=True)
                copied.close()    

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

        blfile = filename_in.rstrip('/')+'.baseline.table'
        os.system('rm -rf %s'%(blfile))

        # dummy scantable for baseline subtraction
        dummy_scan = utils.create_dummy_scan(filename_in, datatable, rows_to_process)
        
        _polnos = polnos.take(rows_to_process)
        index_list_total = []

        for pol in pollist:
            time_table = datatable.get_timetable(antennaid, spwid, pol)
            member_list = time_table[timetable_index]

            index_list_per_pol = rows_to_process.take(numpy.where(_polnos==pol)[0])

            # working with spectral data in scantable
            nrow_total = len(index_list_per_pol)
                
            # Create progress timer
            Timer = common.ProgressTimer(80, nrow_total, LOG.level)
    
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
                updated = []
                
                index_list = []

                for i in xrange(nrow):
                    row = rows[i]
                    idx = idxs[i]
                    LOG.trace('===== Processing at row = %s ====='%(row))
                    nochange = datatable.tb2.getcell('NOCHANGE',idx)
                    LOG.trace('row = %s, Flag = %s'%(row, nochange))
    
                    # skip if no update on line window
                    if nochange > 0:
                        continue
    
                    # data to be fitted
                    sp = spectra[i]
    
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
                    (result, nmask) = self._calc_baseline_fit(dummy_scan, sp, polyorder, nchan, 0, edge, _masklist, blfile, win_polyorder, fragment, nwindow, mask_array)
                    spectra[i] = result
                    index_list.append(idx)
                    updated.append(i)
    
                # write data
                with casatools.TableReader(filename_out, nomodify=False) as tb:
                    for i in updated:
                        tb.putcell('SPECTRA', rows[i], spectra[i])
    
                # update baseline table
                if os.path.exists(blfile):
                    self._update_bltable(bltable_name, blfile, datatable, index_list, nchan, edge, nwindow, fragment)
    
                    # cleanup blfile
                    os.system('rm -rf %s'%(blfile))
                
                index_list_total.extend(index_list)
                
        outcome = {'bltable': bltable_name,
                   'index_list': index_list_total}
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

    def _calc_baseline_fit(self, scan, data, polyorder, nchan, modification, edge, masklist, blfile, win_polyorder, fragment, nwindow, mask):
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

        LOG.trace('nchan_without_edge, num_mask, diff=%s, %s'%(nchan_without_edge, num_mask))

        outdata = self._fit(data, scan, polyorder, nchan, mask, edge, nchan_without_edge, num_mask, fragment, nwindow, win_polyorder, masklist, blfile)
        outdata[:edge[0]] = 0.0
        outdata[nchan-edge[1]:] = 0.0

        return (outdata, num_mask)


class CubicSplineFitting(FittingBase):
    def _fit(self, data, scan, polyorder, nchan, mask, edge, nchan_without_edge, nchan_masked, fragment, nwindow, win_polyorder, masklist, blfile):
        #mask[:edge[0]] = 0
        #mask[nchan-edge[1]:] = 0
        num_nomask = nchan_without_edge - nchan_masked
        num_pieces = max(int(min(polyorder * num_nomask / float(nchan_without_edge) + 0.5, 0.1 * num_nomask)), 1)
        LOG.trace('Cubic Spline Fit: Number of Sections = %d' % num_pieces)
        scan._setspectrum(data, 0)
        # 2013/05/08 TN
        # insitu=False is slower since it needs to create scantable instance
        # for output.
        #outdata = numpy.array(scan.cspline_baseline(insitu=False, mask=mask, npiece=num_pieces, clipthresh=5.0, clipniter=self.ClipCycle, blfile=blfile)._getspectrum(0))
        scan.cspline_baseline(insitu=True, mask=mask, npiece=num_pieces, clipthresh=5.0, clipniter=self.ClipCycle, blfile=blfile, csvformat=True)
        outdata = numpy.array(scan._getspectrum(0))
        return outdata

    def _update_bltable(self, table_name, csvfile, datatable, index_list, nchan, edge, nwindow, fragment):
        blgen = SplineBaselineTableGenerator()
        blgen.import_csv(csvfile)
        blgen.update_table(table_name, datatable, index_list)


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

# class PolynomialFitting(FittingBase):
#     def _fit(self, data, scan, polyorder, nchan, mask, edge, nchan_without_edge, nchan_masked, fragment, nwindow, win_polyorder, masklist, blfile):
#         LOG.info('fragment, nwindow, win_polyorder = %s, %s, %s' % (fragment, nwindow, win_polyorder))
#         LOG.info('Number of subdivided segments = %s'%(nwindow))
# 
#         # fit per fragments
#         resultdata = []
#         for win in xrange(nwindow):
#             ledge = int(win * nchan_without_edge / (fragment * 2) + edge[0])
#             redge = nchan - edge[1] - int(nchan_without_edge * (nwindow - 1 - win) / (fragment * 2))
# 
#             # check new edges are inside mask region or not
#             masked = 0
#             for [m0, m1] in masklist:
#                 new_ledge = ledge
#                 new_redge = redge
#                 # all masked
#                 if m0 <= ledge and redge <= m1:
#                     new_ledge = m0 - (redge - ledge) / 2
#                     new_redge = m1 + (redge - ledge) / 2
#                     masked += redge - ledge
#                 # mask inside windows
#                 elif ledge < m0 and m1 < redge:
#                     masked += (m1 - m0)
#                     if m1 <= (redge + ledge) / 2:
#                         new_ledge = ledge - (m1 - m0)
#                         new_redge = redge
#                     elif (ledge + redge) / 2 <= m0:
#                         new_ledge = ledge
#                         new_redge = redge + (m1 - m0)
#                     else:
#                         new_ledge = ledge - ((ledge + redge) / 2 - m0)
#                         new_redge = redge - (m1 - (ledge + redge) / 2)
#                 # left edge inside mask
#                 elif m0 <= ledge and ledge < m1:
#                     masked += (m1 - ledge)
#                     if m1 <= (ledge + redge) / 2:
#                         new_ledge = ledge - (m1 - m0)
#                         new_redge = redge
#                     else:
#                         new_ledge = m0 - (redge - ledge) / 2
#                         new_redge = redge + (m1 - (ledge + redge) / 2)
#                 # right edge inside mask
#                 elif m0 < redge and redge <= m1:
#                     masked += (redge - m0)
#                     if (ledge + redge) / 2 <= m0:
#                         new_ledge = ledge
#                         new_redge = redge + (m1 - m0)
#                     else:
#                         new_ledge = ledge - ((ledge + redge) / 2 - m0)
#                         new_redge = m1 + (redge - ledge) / 2
#                 ledge = new_ledge
#                 redge = new_redge
#             ledge = max(ledge, edge[0])
#             redge = min(redge, nchan - edge[1])
# 
#             # Calculate positions for combining fragmented spectrum
#             LOG.debug('nchan_without_edge=%s, ledge=%s, redge=%s'%(nchan_without_edge, ledge, redge))
#             win_edge_ignore_l = int(nchan_without_edge / (fragment * win_polyorder))
#             win_edge_ignore_r = win_edge_ignore_l
#             pos_l0 = int(win * nchan_without_edge / (fragment * 2)) + nchan_without_edge / win_polyorder + ledge
#             pos_l1 = int((win + 1) * nchan_without_edge / (fragment * 2)) - 1 - nchan_without_edge / fragment / win_polyorder + ledge
#             pos_r0 = int((win + 1) * nchan_without_edge / (fragment * 2)) + nchan_without_edge / fragment / win_polyorder + ledge
#             pos_r1 = int((win + 2) * nchan_without_edge / (fragment * 2)) - 1 - nchan_without_edge / fragment / win_polyorder + ledge
#             dl = float(pos_l1 - pos_l0)
#             dr = float(pos_r1 - pos_r0)
#             if win == 0:
#                 win_edge_ignore_l = 0
#                 pos_l0 = edge[0]
#                 pos_l1 = edge[0]
#                 dl = 1.0
#             if win == (nwindow - 1):
#                 win_edge_ignore_r = 0
#                 pos_r0 = nchan - edge[1]
#                 pos_r1 = nchan - edge[1]
#                 dr = 1.0
# 
#             nn_mask = float((pos_r1 - pos_l0) - mask[pos_l0:pos_r1].sum())
#             dorder = int(max(1, ((pos_r1 - pos_l0 - nn_mask * 0.5) * win_polyorder / (pos_r1 - pos_l0) + 0.5)))
#             LOG.debug('Revised edgemask = %s:%s  Adjust polyorder = %s' % (ledge, redge, dorder))
#             LOG.debug('Segment %d: Revised edgemask = %s:%s  Adjust polyorder used in individual fit= %s' % (win, ledge, redge, dorder))
# 
#             start_time = time.time()
#             LOG.debug('Fitting Start')
#             edge_mask = scan.create_mask([0, redge-ledge])
#             # 0 and (redge-ledge) are included in the fitting range
#             scan._setspectrum(numpy.concatenate((data[ledge:redge],numpy.zeros(nchan+ledge-redge, dtype=numpy.float64))))
#             tmp_mask = numpy.concatenate((mask[ledge:redge], numpy.zeros(nchan+ledge-redge, dtype=int)))
#             # 2013/05/08 TN
#             # insitu=False is slower since it needs to create scantable instance
#             # for output.
#             #tmpfit0 = scan.poly_baseline(order=dorder, mask=(tmp_mask & edge_mask), insitu=False, clipthresh=5.0, clipniter=self.ClipCycle, blfile=blfile)._getspectrum(0)
#             #tmpfit = numpy.array(tmpfit0, dtype=numpy.float32)[:redge-ledge]
#             scan.poly_baseline(order=dorder, mask=(tmp_mask & edge_mask), insitu=True, clipthresh=5.0, clipniter=self.ClipCycle, blfile=blfile, csvformat=True)
#             tmpfit = numpy.array(scan._getspectrum(0), dtype=numpy.float32)[:redge-ledge]
# 
#             # Restore scan to the original position
#             # 0 -> EdgeL
#             resultdata.append(list(numpy.concatenate((numpy.zeros(ledge), tmpfit, numpy.zeros(nchan-redge)))))
#             end_time = time.time()
#             LOG.debug('Fitting End: Elapsed Time=%.1f'%(end_time-start_time))
# 
#             # window function: f(x) = -2x^3 + 2x^2 (0 <= x <= 1)
#             for i in xrange(nchan):
#                 if i < pos_l0:
#                     resultdata[win][i] = 0.0
#                 elif i <= pos_l1:
#                     x = (i - pos_l0) / dl
#                     resultdata[win][i] *= (-2.0 * x ** 3.0 + 3.0 * x ** 2.0)
#                 elif i > pos_r1:
#                     resultdata[win][i] = 0.0
#                 elif i >= pos_r0:
#                     x = (i - pos_r0) / dr
#                     resultdata[win][i] *= (2.0 * x ** 3.0 - 3.0 * x ** 2.0 + 1.0)
#         outdata = numpy.sum(resultdata, axis=0)
#         outdata[:edge[0]] = 0.0
#         outdata[nchan-edge[1]:] = 0.0
# 
#         return outdata
# 
#     def _update_bltable(self, table_name, csvfile, datatable, index_list, nchan, edge, nwindow, fragment):
#         segments = []
#         nchan_without_edge = nchan - sum(edge)
#         for win in xrange(nwindow):
#             ledge = int(win *  nchan_without_edge / (fragment * 2) + edge[0])
#             redge = nchan - edge[1] - int(nchan_without_edge * (nwindow - 1 - win) / (fragment * 2))
#             redge = min(redge, nchan-1)
#             segments.append([ledge,redge])
#         blgen = PolynomialBaselineTableGenerator(segments)
#         blgen.import_csv(csvfile)
#         blgen.update_table(table_name, datatable, index_list)
#             

