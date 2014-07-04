from __future__ import absolute_import

import os
import math
import numpy
import time

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from .. import common 

from .accumulator import Accumulator

LOG = infrastructure.get_logger(__name__)
DO_TEST = True

def gridding_factory(observing_pattern):
    LOG.info('ObsPattern = %s' % observing_pattern)
    if observing_pattern.upper() == 'RASTER':
        return RasterGridding
    elif observing_pattern.upper() == 'SINGLE-POINT':
        return SinglePointGridding
    elif observing_pattern.upper() == 'MULTI-POINT':
        return MultiPointGridding
    else:
        raise ValueError('observing_pattern \'%s\' is invalid.'%(observing_pattern))

class GriddingInputs(common.SingleDishInputs):
    def __init__(self, context, antennaid, spwid, polid, nx=None, ny=None):
        self._init_properties(vars())
        
class GriddingResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(GriddingResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(GriddingResults, self).merge_with_context(context)

    def _outcome_name(self):
        # return [image.imagename for image in self.outcome]
        return ''


class GriddingBase(common.SingleDishTaskTemplate):
    Inputs = GriddingInputs
    Rule = {'WeightDistance': 'Gauss', \
            'Clipping': 'MinMaxReject', \
            'WeightRMS': True, \
            'WeightTsysExptime': False} 

    @common.datatable_setter
    def prepare(self):
        start = time.time()
        inputs = self.inputs
        context = self.context
        if type(inputs.antennaid) == int:
            self.antenna = [inputs.antennaid]
        else:
            self.antenna = inputs.antennaid
        self.files = [context.observing_run[i].name for i in self.antenna]
        if type(inputs.spwid) == int:
            self.spw = [inputs.spwid]
        else:
            self.spw = inputs.spwid
        self.spwmap = dict([(a,s) for (a,s) in zip(self.antenna,self.spw)])
        if type(inputs.polid) == int:
            self.pol = [inputs.polid]
        else:
            self.pol = inputs.polid
        LOG.debug('self.antenna=%s'%(self.antenna))
        LOG.debug('self.spw=%s'%(self.spw))
        LOG.debug('self.pol=%s'%(self.pol))
            
        LOG.debug('Members to be processed:')
        for (a,s) in zip(self.antenna, self.spw):
            LOG.debug('\tAntenna %s Spw %s Pol %s'%(a,s,self.pol))
            
        reference_data = context.observing_run[self.antenna[0]]
        reference_spw = reference_data.spectral_window[self.spw[0]]
        self.nchan = reference_spw.nchan
        self.srctype = reference_data.calibration_strategy['srctype']
        # beam size
        grid_size = casatools.quanta.convert(reference_data.beam_size[self.spw[0]], 'deg')['value']
        self.grid_ra = grid_size
        self.grid_dec = grid_size       
        
        combine_radius = self.grid_ra
        kernel_width = 0.5 * combine_radius
        allowance = self.grid_ra * 0.1
        spacing = self.grid_ra / 3.0
        DataIn = self.files
        LOG.info('DataIn=%s'%(DataIn))
        grid_table = self.dogrid(DataIn, kernel_width, combine_radius, allowance, spacing)
        end = time.time()
        LOG.info('execute: elapsed time %s sec'%(end-start))
        result = GriddingResults(task=self.__class__,
                                 success=True,
                                 outcome=grid_table)
        result.task = self.__class__

        if self.inputs.context.subtask_counter is 0: 
            result.stage_number = self.inputs.context.task_counter - 1
        else:
            result.stage_number = self.inputs.context.task_counter 

        return result
    
    def analyse(self, result):
        return result
        
    def dogrid(self, DataIn, kernel_width, combine_radius, allowance_radius, grid_spacing, loglevel=2):
        """
        The process does re-map and combine spectrum for each position
        GridTable format:
          [[IF,POL,0,0,RAcent,DECcent,[[row0,r0,RMS0,index0,ant0],[row1,r1,RMS1,index1,ant1],..,[rowN,rN,RMSN,indexN,antn]]]
           [IF,POL,0,1,RAcent,DECcent,[[row0,r0,RMS0,index0,ant0],[row1,r1,RMS1,index1,ant1],..,[rowN,rN,RMSN,indexN,antn]]]
                        ......
           [IF,POL,M,N,RAcent,DECcent,[[row0,r0,RMS0,index0,ant0],[row1,r1,RMS1,index1,ant1],..,[rowN,rN,RMSN,indexN,antn]]]]
         where row0,row1,...,rowN should be combined to one for better S/N spectra
               'r' is a distance from grid position
        'weight' can be 'CONST', 'GAUSS', or 'LINEAR'
        'clip' can be 'none' or 'minmaxreject' 
        'rms_weight' is either True or False. If True, NewRMS is used for additional weight
          Number of spectra output is len(GridTable)
        OutputTable format:
           [[IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]
            [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]
                    ......
            [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]]

        DataOut is not used 2010/10/25 GK
        """
        start = time.time()

        table = self.datatable.tb1
        index_list = list(common.get_index_list(self.datatable, self.antenna, self.spw, self.pol, self.srctype))
        index_list.sort()
        #pols = table.getcol('POL').take(index_list)
        #index_list = numpy.take(index_list, numpy.where(pols == self.pol)[0])
        #del pols
        
        rows = table.getcol('ROW').take(index_list)
        ants = table.getcol('ANTENNA').take(index_list)
        ras = table.getcol('RA').take(index_list)
        decs = table.getcol('DEC').take(index_list)
        stats = self.datatable.tb2.getcol('STATISTICS').take(index_list)
        tsys = table.getcol('TSYS').take(index_list)
        exposure = table.getcol('EXPOSURE').take(index_list)
        net_flag = self.datatable.tb2.getcol('FLAG_SUMMARY').take(index_list)

        ### test code
        if DO_TEST:
            ifnos = self.datatable.tb1.getcol('IF').take(index_list)
            polnos = self.datatable.tb1.getcol('POL').take(index_list)
            for _i in xrange(len(ants)):
                _ant = ants[_i]
                _spw = ifnos[_i]
                _pol = polnos[_i]
                _index = numpy.where(self.antenna == _ant)[0]
                _j = list(self.antenna).index(_ant)
                assert _pol in self.pol[_j], 'row %s is bad selection: POLNO doesn\'t match (actual %s expected %s)'%(index_list[_i], _pol, self.pol[_j])
                assert _spw == self.spw[_j], 'row %s is bad selection: IFNO not in process list (actual %s expected %s)'%(index_list[_i], _spw, self.spw[_j])
        ###

        # Re-Gridding
        # 2008/09/20 Spacing should be identical between RA and DEC direction
        # Curvature has not been taken account
        dec_corr = 1.0 / math.cos(self.datatable.getcell('DEC',0) / 180.0 * 3.141592653)

        GridTable = self._group(index_list, rows, ants, ras, decs, stats, combine_radius, allowance_radius, grid_spacing, dec_corr)
        
        # create storage
        num_spectra = len(index_list)
        _counter = 0
        _index = ants[0]
        num_spectra_per_data = []
        for i in xrange(num_spectra):
            if _index == ants[i]:
                _counter += 1
            else:
                num_spectra_per_data.append(_counter)
                _counter = 1
                _index = ants[i]
        num_spectra_per_data.append(num_spectra - sum(num_spectra_per_data))
        LOG.trace('num_spectra_per_data=%s'%(num_spectra_per_data))

        LOG.info('Processing %d spectra...' % num_spectra)
        
        if self.nchan != 1:
            accum = Accumulator(minmaxclip=(self.Rule['Clipping'].upper()=='MINMAXREJECT'),
                                weight_rms=self.Rule['WeightRMS'],
                                weight_tintsys=self.Rule['WeightTsysExptime'],
                                kernel_type=self.Rule['WeightDistance'],
                                kernel_width=kernel_width)
        else:
            accum = Accumulator(minmaxclip=False,
                                weight_rms=False,
                                weight_tintsys=True,
                                kernel_type=self.Rule['WeightDistance'],
                                kernel_width=kernel_width)

        num_grid = len(GridTable)
        LOG.info('Accumulate nearby spectrum for each Grid position...')
        LOG.info('Processing %d spectra...' % (num_grid))
        OutputTable = []
        
        # create storage for output
        # 2011/11/12 DataIn and rowsSel are [list]
        IDX2StorageID = {}
        StorageID = 0
        for i in xrange(len(DataIn)):
            # read data to SpStorage
            for j in xrange(num_spectra_per_data[i]):
                x = index_list[StorageID]
                IDX2StorageID[x] = StorageID
                StorageID += 1

        # Create progress timer
        Timer = common.ProgressTimer(80, num_grid, loglevel)
        ID = 0
        for [IFS, POL, X, Y, RAcent, DECcent, RowDelta] in GridTable:
            IF = IFS[0]
            # RowDelta is numpy array
            if len(RowDelta) == 0:
                indexlist = []
                deltalist = []
                rmslist = []
            else:
                indexlist = numpy.array([IDX2StorageID[int(idx)] for idx in RowDelta[:,3]])
                valid_index = numpy.where(net_flag[indexlist] == 1)[0]
                indexlist = indexlist.take(valid_index)
                deltalist = RowDelta[:,1].take(valid_index)
                rmslist = RowDelta[:,2].take(valid_index)
            num_valid = len(indexlist)
            num_flagged = len(RowDelta) - num_valid
            if num_valid == 0:
                # No valid Spectra at the position
                RMS = 0.0
            elif num_valid == 1:
                # One valid Spectrum at the position
                RMS = rmslist[0]
            else:
                # More than one valid Spectra at the position
                # Data accumulation by Accumulator
                accum.init(len(indexlist))
                accum.accumulate(indexlist, rmslist, deltalist, tsys, exposure)
                
                RMS = accum.rms

            OutputTable.append([IF, POL, X, Y, RAcent, DECcent, num_valid, num_flagged, RMS])
            ID += 1
            del indexlist, deltalist, rmslist

            # Countup progress timer
            Timer.count()

        end = time.time()
        LOG.info('dogrid: elapsed time %s sec'%(end-start))
        return OutputTable

class RasterGridding(GriddingBase):
    def _group(self, index_list, rows, ants, ras, decs, stats, CombineRadius, Allowance, GridSpacing, DecCorrection):
        """
        Gridding by RA/DEC position
        """
        start = time.time()

        GridTable = []
        NROW = len(index_list)

        ThresholdR = CombineRadius * CombineRadius
        MinRA = ras.min()
        MaxRA = ras.max()
        MinDEC = decs.min()
        MaxDEC = decs.max()
        # Check if the distribution crosses over the RA=0
        if MinRA < 10 and MaxRA > 350:
            ras = ras + numpy.less_equal(ras, 180) * 360.0
            MinRA = ras.min()
            MaxRA = ras.max()
        # (RAcenter, DECcenter) to be the center of the grid
        nx = self.inputs.nx
        ny = self.inputs.ny
        if nx is None:
            NGridRA = int((MaxRA - MinRA) / (GridSpacing * DecCorrection)) + 1
            if NGridRA % 2 == 0:
                NGridRA += 2
            else:
                NGridRA += 1
        else:
            NGridRA = nx
        if ny is None:
            NGridDEC = int((MaxDEC - MinDEC) / GridSpacing) + 1
            if NGridDEC % 2 == 0:
                NGridDEC += 2
            else:
                NGridDEC += 1
        else:
            NGridDEC = ny
        MinRA = (MinRA + MaxRA) / 2.0 - (NGridRA - 1) / 2.0 * GridSpacing * DecCorrection
        MinDEC = (MinDEC + MaxDEC) / 2.0 - (NGridDEC - 1) / 2.0 * GridSpacing

        for y in xrange(NGridDEC):
            if NROW > 10000: print 'Progress:', y, '/', NGridDEC, '(', NGridRA, ')', ' : ', time.ctime()
            DEC = MinDEC + GridSpacing * y
            DeltaDEC = decs - DEC
            SelectD = numpy.where(numpy.logical_and(DeltaDEC < CombineRadius, DeltaDEC > -CombineRadius))[0]
            #SelectD = numpy.nonzero(numpy.less_equal(DeltaDEC, CombineRadius) * numpy.greater_equal(DeltaDEC, -CombineRadius))[0]
            sDeltaDEC = numpy.take(DeltaDEC, SelectD)
            sRA = numpy.take(ras, SelectD)
            sROW = numpy.take(rows, SelectD)
            sIDX = numpy.take(index_list, SelectD)
            sRMS = numpy.take(stats, SelectD)
            sANT = numpy.take(ants, SelectD)
            #LOG.debug('Combine Spectra: %s' % len(sRMS))
            LOG.debug('Combine Spectra: %s' % len(sRA))
            for x in xrange(NGridRA):
                RA = MinRA + GridSpacing * DecCorrection * x
                sDeltaRA = (sRA - RA) / DecCorrection
                Delta = sDeltaDEC * sDeltaDEC + sDeltaRA * sDeltaRA
                #Select = numpy.less_equal(Delta, ThresholdR)
                #SelectR = numpy.nonzero(numpy.less_equal(Delta, ThresholdR))[0]
                SelectR = numpy.where(Delta < ThresholdR)[0]
                if len(SelectR > 0):
                    ssROW = numpy.take(sROW, SelectR)
                    ssRMS = numpy.take(sRMS, SelectR)
                    ssIDX = numpy.take(sIDX, SelectR)
                    ssANT = numpy.take(sANT, SelectR)
                    ssDelta = numpy.sqrt(numpy.take(Delta, SelectR))
                    line = [map(lambda x: self.spwmap[x], ssANT), self.pol, x, y, RA, DEC, numpy.transpose([ssROW, ssDelta, ssRMS, ssIDX, ssANT])]
                else:
                    line = [self.spw, self.pol, x, y, RA, DEC, []]
                GridTable.append(line)
                #LOG.debug("GridTable: %s" % line)

        LOG.info('NGridRA = %s  NGridDEC = %s' % (NGridRA, NGridDEC))

        end = time.time()
        LOG.info('_group: elapsed time %s sec'%(end-start))
        return GridTable

class SinglePointGridding(GriddingBase):
    def _group(self, index_list, rows, ants, ras, decs, stats, CombineRadius, Allowance, GridSpacing, DecCorrection):
        """
        Gridding by RA/DEC position
        """
        start = time.time()

        GridTable = []

        NGridRA = 1
        NGridDEC = 1
        CenterRA = ras.mean()
        CenterDEC = decs.mean()
        line = [[], self.pol, 0, 0, CenterRA, CenterDEC, []]
        for x in range(len(index_list)):
            Delta = math.sqrt((ras[x] - CenterRA) ** 2.0 + (decs[x] - CenterDEC) ** 2.0)
            if Delta <= Allowance:
                line[0].append(self.spwmap[ants[x]])
                line[6].append([index_list[x], Delta, stats[x], index_list[x], ants[x]])
        line[6] = numpy.array(line[6], dtype=float)
        GridTable.append(line)
        #LOG.debug("GridTable: %s" % line)
        end = time.time()

        LOG.info('NGridRA = %s  NGridDEC = %s' % (NGridRA, NGridDEC))

        LOG.info('_group: elapsed time %s sec'%(end-start))
        return GridTable


class MultiPointGridding(GriddingBase):
    def _group(self, index_list, rows, ants, ras, decs, stats, CombineRadius, Allowance, GridSpacing, DecCorrection):
        """
        Gridding by RA/DEC position
        """
        start = time.time()

        GridTable = []

        NGridRA = 0
        NGridDEC = 1
        Flag = numpy.ones(len(index_list))
        while Flag.sum() > 0:
            for x in range(len(index_list)):
                if Flag[x] == 1:
                    RA = ras[x]
                    DEC = decs[x]
                    RAtmp = [RA]
                    DECtmp = [DEC]
                    for y in range(x + 1, len(index_list)):
                        if Flag[y] == 1:
                            Delta = math.sqrt((RA - ras[y]) ** 2.0 + (DEC - decs[y]) ** 2.0)
                            if Delta <= Allowance:
                                RAtmp.append(ras[y])
                                DECtmp.append(decs[y])
                    CenterRA = (numpy.array(RAtmp)).mean()
                    CenterDEC = (numpy.array(DECtmp)).mean()
                    line = [[], self.pol, 0, NGridRA, CenterRA, CenterDEC, []]
                    NGridRA += 1
                    for x in range(len(index_list)):
                        if Flag[x] == 1:
                            Delta = math.sqrt((ras[x] - CenterRA) ** 2.0 + (decs[x] - CenterDEC) ** 2.0)
                            if Delta <= Allowance:
                                line[0].append(self.spwmap[ants[x]])
                                line[6].append([index_list[x], Delta, stats[x], index_list[x], ants[x]])
                                Flag[x] = 0
                    line[6] = numpy.array(line[6])
                    GridTable.append(line)
                    #LOG.debug("GridTable: %s" % line)
        del Flag

        LOG.info('NGridRA = %s  NGridDEC = %s' % (NGridRA, NGridDEC))

        end = time.time()
        LOG.info('_group: elapsed time %s sec'%(end-start))
        return GridTable
        
