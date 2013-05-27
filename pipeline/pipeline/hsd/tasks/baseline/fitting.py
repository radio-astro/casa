from __future__ import absolute_import

import os
import time
import re
from math import sqrt
import numpy
import contextlib

import asap as sd

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.jobrequest as jobrequest
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils
from pipeline.hsd.heuristics import fitorder, fragmentation
from .. import common
from . import rules
from . import utils
from .baselinetable import SplineBaselineTableGenerator
from .baselinetable import PolynomialBaselineTableGenerator

LOG = infrastructure.get_logger(__name__)

class FittingBase(object):
    ApplicableDuration = 'raster' # 'raster' | 'subscan'
    MaxPolynomialOrder = 'none' # 'none', 0, 1, 2,...
    PolynomialOrder = 'automatic' # 'automatic', 0, 1, 2, ...
    ClipCycle = 1
    
    def execute(self, datatable, filename, filename_out, bltable_name, time_table, index_list, nchan, edge, fit_order='automatic', LogLevel='info'):
        """
        """

        # fitting order
        if fit_order == 'automatic':
            # fit order heuristics
            LOG.info('Baseline-Fitting order was automatically determined')
            h_polyorder = fitorder.FitOrderHeuristics()
        else:
            LOG.info('Baseline-Fitting order was fixed to be %d'%(fit_order))
            h_polyorder = lambda *args, **kwargs: fitorder

        if self.ApplicableDuration == 'subscan':
            member_list = time_table[1]
        else:
            member_list = time_table[0]

        _edge = common.parseEdge(edge)

        blfile = filename.rstrip('/')+'.baseline.table'
        os.system('rm -rf %s'%(blfile))

        # dummy scantable for baseline subtraction
        dummy_scan = utils.create_dummy_scan(filename, datatable, index_list)
        LOG.debug('dummy_scan.nrow()=%s'%(dummy_scan.nrow()))
        LOG.debug('nchan for dummy_scan=%s'%(len(dummy_scan._getspectrum())))

        # working with spectral data in scantable
        nrow_total = len(index_list)

        # Create progress timer
        Timer = common.ProgressTimer(80, nrow_total, LogLevel)

        nwin = []
        LOG.info('Baseline Fit: background subtraction...')
        LOG.info('Processing %d spectra...'%(nrow_total))

        for y in xrange(len(member_list)):
            rows = member_list[y][0]
            idxs = member_list[y][1]
            LOG.debug('rows=%s'%(rows))
            with casatools.TableReader(filename) as tb:
                spectra = numpy.array([tb.getcell('SPECTRA',row)
                                       for row in rows])
            LOG.debug('spectra.shape=%s'%(list(spectra.shape)))
            masklist = [datatable.tb2.getcell('MASKLIST',idx)
                        for idx in idxs]

            # fit order determination
            polyorder = h_polyorder(spectra, masklist, _edge)
            if fit_order == 'automatic' and self.MaxPolynomialOrder != 'none':
                polyorder = min(polyorder, self.MaxPolynomialOrder)
            LOG.info('group %d: fitting order=%s'%(y,polyorder))

            # calculate fragmentation
            h_fragmentation = fragmentation.FragmentationHeuristics()
            (fragment, nwindow, win_polyorder) = h_fragmentation(polyorder, nchan, edge)

            nrow = len(rows)
            LOG.debug('nrow = %s'%(nrow))
            LOG.debug('len(idxs) = %s'%(len(idxs)))
            index_list = []
            for i in xrange(nrow):
                row = rows[i]
                idx = idxs[i]
                LOG.info('===== Processing at row = %s ====='%(row))
                nochange = datatable.tb2.getcell('NOCHANGE',idx)
                LOG.debug('row = %s, Flag = %s'%(row, nochange))

                # skip if no update on line window
                if nochange > 0:
                    continue

                # data to be fitted
                sp = spectra[i]

                # mask lines
                maxwidth = 1
                masklist = datatable.getcell('MASKLIST',idx)
                for [chan0, chan1] in masklist:
                    if chan1 - chan0 >= maxwidth:
                        maxwidth = int((chan1 - chan0 + 1) / 1.4)
                        # allowance in Process3 is 1/5:
                        #    (1 + 1/5 + 1/5)^(-1) = (5/7)^(-1)
                        #                         = 7/5 = 1.4
                max_polyorder = int((nchan - sum(_edge)) / maxwidth + 1)
                LOG.debug('Masked Region from previous processes = %s'%(masklist))
                LOG.debug('edge parameters= (%s,%s)'%(_edge))
                LOG.debug('Polynomial order = %d  Max Polynomial order = %d'%(polyorder, max_polyorder))

                # fitting
                polyorder = min(polyorder, max_polyorder)
                start_time = time.time()
                (result, nmask) = self._calc_baseline_fit(dummy_scan, sp, polyorder, nchan, 0, _edge, masklist, blfile, win_polyorder, fragment, nwindow)
                end_time = time.time()
                LOG.info('fitting: elapsed time=%s'%(end_time-start_time))
                nwin.append(nwindow)
                spectra[i] = result
                index_list.append(idx)

            # write data
            with casatools.TableReader(filename, nomodify=False) as tb:
                for i in xrange(nrow):
                    tb.putcell('SPECTRA', rows[i], spectra[i])

            # update baseline table
            self._update_bltable(bltable_name, blfile, datatable, index_list, nchan, _edge, nwindow, fragment)

            # cleanup blfile
            os.system('rm -rf %s'%(blfile))
                        
    def _calc_baseline_fit(self, scan, data, polyorder, nchan, modification, edge, masklist, blfile, win_polyorder, fragment, nwindow):
        LOG.debug('masklist=%s'%(masklist))

        # set edge mask
        data[:edge[0]] = 0.0
        data[nchan-edge[1]:] = 0.0
        
        # Create mask for line protection
        nchan_without_edge = nchan - sum(edge)
        mask = numpy.ones(nchan, dtype=int)
        if type(masklist) == list:
            for [m0, m1] in masklist:
                mask[m0:m1] = 0
        else:
            LOG.critical('Invalid masklist')
        #if edge[1] > 0:
        #    nmask = int(nchan_without_edge - numpy.sum(mask[edge[0]:-edge[1]] * 1.0))
        #else:
        #    nmask = int(nchan_without_edge - numpy.sum(mask[edge[0]:] * 1.0))
        num_mask = int(nchan_without_edge - numpy.sum(mask[edge[0]:nchan-edge[1]] * 1.0))
        num_nomask = nchan_without_edge - num_mask

        LOG.debug('nchan_without_edge, num_mask, diff=%s, %s, %s'%(nchan_without_edge, num_mask, num_nomask))

        outdata = self._fit(data, scan, polyorder, nchan, mask, edge, nchan_without_edge, num_mask, fragment, nwindow, win_polyorder, masklist, blfile)
        outdata[:edge[0]] = 0.0
        outdata[nchan-edge[1]:] = 0.0

        return (outdata, num_mask)


class CubicSplineFitting(FittingBase):
    def _fit(self, data, scan, polyorder, nchan, mask, edge, nchan_without_edge, nchan_masked, fragment, nwindow, win_polyorder, masklist, blfile):
        mask[:edge[0]] = 0
        mask[nchan-edge[1]:] = 0
        num_nomask = nchan_without_edge - nchan_masked
        num_pieces = max(int(min(polyorder * num_nomask / float(nchan_without_edge) + 0.5, 0.1 * num_nomask)), 1)
        LOG.info('Cubic Spline Fit: Number of Sections = %d' % num_pieces)
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


class PolynomialFitting(FittingBase):
    def _fit(self, data, scan, polyorder, nchan, mask, edge, nchan_without_edge, nchan_masked, fragment, nwindow, win_polyorder, masklist, blfile):
        LOG.info('fragment, nwindow, win_polyorder = %s, %s, %s' % (fragment, nwindow, win_polyorder))
        LOG.info('Number of subdivided segments = %s'%(nwindow))

        # fit per fragments
        resultdata = []
        for win in xrange(nwindow):
            ledge = int(win * nchan_without_edge / (fragment * 2) + edge[0])
            redge = nchan - edge[1] - int(nchan_without_edge * (nwindow - 1 - win) / (fragment * 2))

            # check new edges are inside mask region or not
            masked = 0
            for [m0, m1] in masklist:
                new_ledge = ledge
                new_redge = redge
                # all masked
                if m0 <= ledge and redge <= m1:
                    new_ledge = m0 - (redge - ledge) / 2
                    new_redge = m1 + (redge - ledge) / 2
                    masked += redge - ledge
                # mask inside windows
                elif ledge < m0 and m1 < redge:
                    masked += (m1 - m0)
                    if m1 <= (redge + ledge) / 2:
                        new_ledge = ledge - (m1 - m0)
                        new_redge = redge
                    elif (ledge + redge) / 2 <= m0:
                        new_ledge = ledge
                        new_redge = redge + (m1 - m0)
                    else:
                        new_ledge = ledge - ((ledge + redge) / 2 - m0)
                        new_redge = redge - (m1 - (ledge + redge) / 2)
                # left edge inside mask
                elif m0 <= ledge and ledge < m1:
                    masked += (m1 - ledge)
                    if m1 <= (ledge + redge) / 2:
                        new_ledge = ledge - (m1 - m0)
                        new_redge = redge
                    else:
                        new_ledge = m0 - (redge - ledge) / 2
                        new_redge = redge + (m1 - (ledge + redge) / 2)
                # right edge inside mask
                elif m0 < redge and redge <= m1:
                    masked += (redge - m0)
                    if (ledge + redge) / 2 <= m0:
                        new_ledge = ledge
                        new_redge = redge + (m1 - m0)
                    else:
                        new_ledge = ledge - ((ledge + redge) / 2 - m0)
                        new_redge = m1 + (redge - ledge) / 2
                ledge = new_ledge
                redge = new_redge
            ledge = max(ledge, edge[0])
            redge = min(redge, nchan - edge[1])

            # Calculate positions for combining fragmented spectrum
            LOG.debug('nchan_without_edge=%s, ledge=%s, redge=%s'%(nchan_without_edge, ledge, redge))
            win_edge_ignore_l = int(nchan_without_edge / (fragment * win_polyorder))
            win_edge_ignore_r = win_edge_ignore_l
            pos_l0 = int(win * nchan_without_edge / (fragment * 2)) + nchan_without_edge / win_polyorder + ledge
            pos_l1 = int((win + 1) * nchan_without_edge / (fragment * 2)) - 1 - nchan_without_edge / fragment / win_polyorder + ledge
            pos_r0 = int((win + 1) * nchan_without_edge / (fragment * 2)) + nchan_without_edge / fragment / win_polyorder + ledge
            pos_r1 = int((win + 2) * nchan_without_edge / (fragment * 2)) - 1 - nchan_without_edge / fragment / win_polyorder + ledge
            dl = float(pos_l1 - pos_l0)
            dr = float(pos_r1 - pos_r0)
            if win == 0:
                win_edge_ignore_l = 0
                pos_l0 = edge[0]
                pos_l1 = edge[0]
                dl = 1.0
            if win == (nwindow - 1):
                win_edge_ignore_r = 0
                pos_r0 = nchan - edge[1]
                pos_r1 = nchan - edge[1]
                dr = 1.0

            nn_mask = float((pos_r1 - pos_l0) - mask[pos_l0:pos_r1].sum())
            dorder = int(max(1, ((pos_r1 - pos_l0 - nn_mask * 0.5) * win_polyorder / (pos_r1 - pos_l0) + 0.5)))
            LOG.debug('Revised edgemask = %s:%s  Adjust polyorder = %s' % (ledge, redge, dorder))
            LOG.debug('Segment %d: Revised edgemask = %s:%s  Adjust polyorder used in individual fit= %s' % (win, ledge, redge, dorder))

            start_time = time.time()
            LOG.debug('Fitting Start')
            edge_mask = scan.create_mask([0, redge-ledge])
            # 0 and (redge-ledge) are included in the fitting range
            scan._setspectrum(numpy.concatenate((data[ledge:redge],numpy.zeros(nchan+ledge-redge, dtype=numpy.float64))))
            tmp_mask = numpy.concatenate((mask[ledge:redge], numpy.zeros(nchan+ledge-redge, dtype=int)))
            # 2013/05/08 TN
            # insitu=False is slower since it needs to create scantable instance
            # for output.
            #tmpfit0 = scan.poly_baseline(order=dorder, mask=(tmp_mask & edge_mask), insitu=False, clipthresh=5.0, clipniter=self.ClipCycle, blfile=blfile)._getspectrum(0)
            #tmpfit = numpy.array(tmpfit0, dtype=numpy.float32)[:redge-ledge]
            scan.poly_baseline(order=dorder, mask=(tmp_mask & edge_mask), insitu=True, clipthresh=5.0, clipniter=self.ClipCycle, blfile=blfile, csvformat=True)
            tmpfit = numpy.array(scan._getspectrum(0), dtype=numpy.float32)[:redge-ledge]

            # Restore scan to the original position
            # 0 -> EdgeL
            resultdata.append(list(numpy.concatenate((numpy.zeros(ledge), tmpfit, numpy.zeros(nchan-redge)))))
            end_time = time.time()
            LOG.debug('Fitting End: Elapsed Time=%.1f'%(end_time-start_time))

            # window function: f(x) = -2x^3 + 2x^2 (0 <= x <= 1)
            for i in xrange(nchan):
                if i < pos_l0:
                    resultdata[win][i] = 0.0
                elif i <= pos_l1:
                    x = (i - pos_l0) / dl
                    resultdata[win][i] *= (-2.0 * x ** 3.0 + 3.0 * x ** 2.0)
                elif i > pos_r1:
                    resultdata[win][i] = 0.0
                elif i >= pos_r0:
                    x = (i - pos_r0) / dr
                    resultdata[win][i] *= (2.0 * x ** 3.0 - 3.0 * x ** 2.0 + 1.0)
        outdata = numpy.sum(resultdata, axis=0)
        outdata[:edge[0]] = 0.0
        outdata[nchan-edge[1]:] = 0.0

        return outdata

    def _update_bltable(self, table_name, csvfile, datatable, index_list, nchan, edge, nwindow, fragment):
        segments = []
        nchan_without_edge = nchan - sum(edge)
        for win in xrange(nwindow):
            ledge = int(win *  nchan_without_edge / (fragment * 2) + edge[0])
            redge = nchan - edge[1] - int(nchan_without_edge * (nwindow - 1 - win) / (fragment * 2))
            redge = min(redge, nchan-1)
            segments.append([ledge,redge])
        blgen = PolynomialBaselineTableGenerator(segments)
        blgen.import_csv(csvfile)
        blgen.update_table(table_name, datatable, index_list)

class FittingFactory(object):
    @staticmethod
    def get_fitting_class(fitfunc='spline'):
        if re.match('^C?SPLINE$', fitfunc.upper()):
            return CubicSplineFitting
        elif re.match('^POLYNOMIAL$', fitfunc.upper()):
            return PolynomialFitting
        else:
            return None
            

