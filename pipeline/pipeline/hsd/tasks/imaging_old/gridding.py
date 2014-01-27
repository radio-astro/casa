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

NoData = common.NoData

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

class GriddingBase(object):
    Rule = {'WeightDistance': 'Gauss', \
            'Clipping': 'MinMaxReject', \
            'WeightRMS': True, \
            'WeightTsysExptime': False} 

    def __init__(self, datatable, antenna, files, spw, pol, srctype, nchan, grid_size):
        self.datatable = datatable
        self.datatable_name = self.datatable.plaintable
        if type(antenna) == int:
            self.antenna = [antenna]
        else:
            self.antenna = antenna
        self.files = files
        self.spw = spw
        self.pol = pol
        self.nchan = nchan
        self.srctype = srctype
        if isinstance(grid_size, list):
            if len(grid_size) > 1:
                self.grid_ra = grid_size[0]
                self.grid_dec = grid_size[1]
            elif len(grid_size) > 0:
                self.grid_ra = grid_size[0]
                self.grid_dec = grid_size[0]
            else:
                self.grid_ra = -1
                self.grid_dec = -1
        else:
            self.grid_ra = grid_size
            self.grid_dec = grid_size

    def execute(self, dry_run=True):
        if dry_run:
            return [], []
        start = time.time()
        combine_radius = self.grid_ra
        kernel_width = 0.5 * combine_radius
        allowance = self.grid_ra * 0.1
        spacing = self.grid_ra / 3.0
        DataIn = self.files
        LOG.info('DataIn=%s'%(DataIn))
        spstorage, grid_table = self.dogrid(DataIn, kernel_width, combine_radius, allowance, spacing)
        end = time.time()
        LOG.info('execute: elapsed time %s sec'%(end-start))
        return spstorage, grid_table
        
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

        # need opened table to call taql, so we use table object that 
        # datatable holds.
        table = self.datatable.tb1

        # TaQL command to make a view from DataTable that is physically 
        # separated to two tables named RO and RW, respectively. 
        # Note that TaQL accesses DataTable on disk, not on memory. 
        #taqlstring = 'USING STYLE PYTHON SELECT ROWNUMBER() AS ID, RO.ROW, RO.ANTENNA, RO.RA, RO.DEC, RO.TSYS, RO.EXPOSURE, RW.STATISTICS[0] AS STAT, RW.FLAG_SUMMARY FROM "%s" RO, "%s" RW WHERE IF==%s && POL == %s && ANTENNA IN %s'%(os.path.join(self.datatable_name,'RO'),os.path.join(self.datatable_name,'RW'),self.spw,self.pol,list(self.antenna))
        taqlstring = 'USING STYLE PYTHON SELECT ROWNUMBER() AS ID FROM "%s" WHERE IF==%s && POL == %s && ANTENNA IN %s'%(os.path.join(self.datatable_name,'RO'),self.spw,self.pol,list(self.antenna))
        if self.srctype is not None:
            taqlstring += ' && SRCTYPE==%s'%(self.srctype)

        LOG.debug('taqlstring="%s"'%(taqlstring))
        tx = table.taql(taqlstring)
        index_list = tx.getcol('ID')
        tx.close()
        del tx
        rows = table.getcol('ROW').take(index_list)
        ants = table.getcol('ANTENNA').take(index_list)
        ras = table.getcol('RA').take(index_list)
        decs = table.getcol('DEC').take(index_list)
        stats = self.datatable.tb2.getcol('STATISTICS').take(index_list)
        tsys = table.getcol('TSYS').take(index_list)
        exposure = table.getcol('EXPOSURE').take(index_list)
        net_flag = self.datatable.tb2.getcol('FLAG_SUMMARY').take(index_list)

        # Re-Gridding
        # 2008/09/20 Spacing should be identical between RA and DEC direction
        # Curvature has not been taken account
        dec_corr = 1.0 / math.cos(self.datatable.getcell('DEC',0) / 180.0 * 3.141592653)
        #dec_corr = 1.0 / math.cos(decs[0] / 180.0 * 3.141592653)

        GridTable = self._group(index_list, rows, ants, ras, decs, stats, combine_radius, allowance_radius, grid_spacing, dec_corr)
        
        # create storage
        num_spectra = len(index_list)
        num_data = len(DataIn)
        #num_spectra_per_data = num_spectra / num_data
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
        SpStorage = numpy.zeros((num_spectra, self.nchan), dtype=numpy.float32)

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
        StorageOut = numpy.ones((num_grid, self.nchan), dtype=numpy.float32) * NoData
        # 2011/11/12 DataIn and rowsSel are [list]
        IDX2StorageID = {}
        StorageID = 0
        for i in xrange(len(DataIn)):
            # read data to SpStorage
            with casatools.TableReader(DataIn[i]) as tb:
                #for j in xrange(num_spectra_per_data):
                for j in xrange(num_spectra_per_data[i]):
                    x = index_list[StorageID]
                    SpStorage[StorageID] = tb.getcell('SPECTRA', rows[StorageID])
                    IDX2StorageID[x] = StorageID
                    StorageID += 1
                LOG.debug('Data Stored in SpStorage')

        # Create progress timer
        Timer = common.ProgressTimer(80, num_grid, loglevel)
        ID = 0
        for [IF, POL, X, Y, RAcent, DECcent, RowDelta] in GridTable:
            # RowDelta is numpy array
            if len(RowDelta) == 0:
                #rowlist = []
                indexlist = []
                deltalist = []
                rmslist = []
            else:
                indexlist = numpy.array([IDX2StorageID[int(idx)] for idx in RowDelta[:,3]])
                valid_index = numpy.where(net_flag[indexlist] == 1)[0]
                #rowlist = numpy.array(RowDelta[:,0].take(valid_index),dtype=int)
                indexlist = indexlist.take(valid_index)
                deltalist = RowDelta[:,1].take(valid_index)
                rmslist = RowDelta[:,2].take(valid_index)
            num_valid = len(indexlist)
            num_flagged = len(RowDelta) - num_valid
            if num_valid == 0:
                # No valid Spectra at the position
                RMS = 0.0
                pass
            elif num_valid == 1:
                # One valid Spectrum at the position
                StorageOut[ID] = SpStorage[0]
                RMS = rmslist[0]
            else:
                # More than one valid Spectra at the position
                data = SpStorage[indexlist]

                # Data accumulation by Accumulator
                #accum = Accumulator(clip.upper()=='MINMAXREJECT',
                #                    rms_weight,
                #                    tsys_weight)
                accum.init(data)
                accum.accumulate(indexlist, rmslist, deltalist, tsys, exposure)
                
                StorageOut[ID] = accum.accumulated
                RMS = accum.rms

            OutputTable.append([IF, POL, X, Y, RAcent, DECcent, num_valid, num_flagged, RMS])
            ID += 1
            del indexlist, deltalist, rmslist

            # Countup progress timer
            Timer.count()

        end = time.time()
        LOG.info('dogrid: elapsed time %s sec'%(end-start))
        return (StorageOut, OutputTable)

class RasterGridding(GriddingBase):
    def __init__(self, datatable, antenna, files, spw, pol, srctype, nchan, grid_size):

        super(RasterGridding, self).__init__(datatable, antenna, files, spw, pol, srctype, nchan, grid_size)

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
        NGridRA = int((MaxRA - MinRA) / (GridSpacing * DecCorrection)) + 1
        NGridDEC = int((MaxDEC - MinDEC) / GridSpacing) + 1
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
                    line = [self.spw, self.pol, x, y, RA, DEC, numpy.transpose([ssROW, ssDelta, ssRMS, ssIDX, ssANT])]
                else:
                    line = [self.spw, self.pol, x, y, RA, DEC, []]
                GridTable.append(line)
                #LOG.debug("GridTable: %s" % line)

        LOG.info('NGridRA = %s  NGridDEC = %s' % (NGridRA, NGridDEC))

        end = time.time()
        LOG.info('_group: elapsed time %s sec'%(end-start))
        return GridTable

class SinglePointGridding(GriddingBase):
    def __init__(self, datatable, antenna, files, spw, pol, srctype, nchan, grid_size):

        super(SinglePointGridding, self).__init__(datatable, antenna, files, spw, pol, srctype, nchan, grid_size)

    def _group(self, index_list, rows, ants, ras, decs, stats, CombineRadius, Allowance, GridSpacing, DecCorrection):
        """
        Gridding by RA/DEC position
        """
        start = time.time()

        GridTable = []
        NROW = len(index_list)

        NGridRA = 1
        NGridDEC = 1
        CenterRA = ras.mean()
        CenterDEC = decs.mean()
        line = [self.spw, self.pol, 0, 0, CenterRA, CenterDEC, []]
        for x in range(len(index_list)):
            Delta = math.sqrt((ras[x] - CenterRA) ** 2.0 + (decs[x] - CenterDEC) ** 2.0)
            if Delta <= Allowance:
                line[6].append([index_list[x], Delta, stats[x], index_list[x], ants[x]])
        line[6] = numpy.array(line[6], dtype=float)
        GridTable.append(line)
        #LOG.debug("GridTable: %s" % line)
        end = time.time()

        LOG.info('NGridRA = %s  NGridDEC = %s' % (NGridRA, NGridDEC))

        LOG.info('_group: elapsed time %s sec'%(end-start))
        return GridTable


class MultiPointGridding(GriddingBase):
    def __init__(self, datatable, antenna, files, spw, pol, srctype, nchan, grid_size):

        super(MultiPointGridding, self).__init__(datatable, antenna, files, spw, pol, srctype, nchan, grid_size)

    def _group(self, index_list, rows, ants, ras, decs, stats, CombineRadius, Allowance, GridSpacing, DecCorrection):
        """
        Gridding by RA/DEC position
        """
        start = time.time()

        GridTable = []
        NROW = len(index_list)

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
                    line = [self.spw, self.pol, 0, NGridRA, CenterRA, CenterDEC, []]
                    NGridRA += 1
                    for x in range(len(index_list)):
                        if Flag[x] == 1:
                            Delta = math.sqrt((ras[x] - CenterRA) ** 2.0 + (decs[x] - CenterDEC) ** 2.0)
                            if Delta <= Allowance:
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
        
