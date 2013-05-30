from __future__ import absolute_import

import os
import math
import numpy
import time

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from .. import common 

LOG = infrastructure.get_logger(__name__)

NoData = common.NoData

def gridding_factory(obs_pattern):
    if obs_pattern.upper() == 'RASTER':
        return RasterGridding
    elif obs_pattern.upper() == 'SINGLE-POINT':
        return SinglePointGridding
    elif obs_pattern.upper() == 'MULTI-POINT':
        return MultiPointGridding
    else:
        raise ValueError('obs_pattern \'%s\' is invalid.'%(obs_pattern))

class GriddingBase(object):
    rule = {'WeightDistance': 'Gauss', \
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

    def execute(self):
        start = time.time()
        combine_size = self.grid_ra
        allowance = self.grid_ra * 0.1
        spacing = self.grid_ra / 3.0
        index_list, grid_table = self.GroupForGrid(combine_size, allowance, spacing, 'RASTER')
        DataIn = self.files
        LOG.info('DataIn=%s'%(DataIn))
        LOG.info('index_list.shape=%s'%(list(index_list.shape)))
        spstorage, grid_table2 = self.Process8(DataIn, index_list, self.nchan, grid_table, 0.5 * combine_size, self.rule)
        end = time.time()
        LOG.info('execute: elapsed time %s sec'%(end-start))
        return spstorage, grid_table2
        
    #def make_grid_table(self):
    def GroupForGrid(self, CombineRadius, Allowance, GridSpacing, ObsPattern):
        """
        Gridding by RA/DEC position
        """
        start = time.time()
        
        # need opened table to call taql, so we use table object that 
        # datatable holds.
        table = self.datatable.tb1

        # TaQL command to make a view from DataTable that is physically 
        # separated to two tables named RO and RW, respectively. 
        # Note that TaQL accesses DataTable on disk, not on memory. 
        taqlstring = 'USING STYLE PYTHON SELECT ROWNUMBER() AS ID, RO.ROW, RO.ANTENNA, RO.RA, RO.DEC, RW.STATISTICS[0] AS STAT FROM "%s" RO, "%s" RW WHERE IF==%s && POL == %s && ANTENNA IN %s'%(os.path.join(self.datatable_name,'RO'),os.path.join(self.datatable_name,'RW'),self.spw,self.pol,list(self.antenna))
        if self.srctype is not None:
            taqlstring += ' && SRCTYPE==%s'%(self.srctype)

        LOG.debug('taqlstring="%s"'%(taqlstring))
        tx = table.taql(taqlstring)
        index_list = tx.getcol('ID')
        rows = tx.getcol('ROW')
        ants = tx.getcol('ANTENNA')
        ras = tx.getcol('RA')
        decs = tx.getcol('DEC')
        stats = tx.getcol('STAT')
        tx.close()
        del tx

        start1 = time.time()

        # Re-Gridding

        NROW = len(index_list)
        LOG.info('ObsPattern = %s' % ObsPattern)
        LOG.info('Processing %d spectra...' % NROW)

        GridTable = []
        # 2008/09/20 Spacing should be identical between RA and DEC direction
        # Curvature has not been taken account
        DecCorrection = 1.0 / math.cos(self.datatable.getcell('DEC',0) / 180.0 * 3.141592653)

        #### GridSpacingRA = GridSpacing * DecCorrection
        index_listTmp=[]
        if type(index_list[0]) == list:
            index_list = [x for y in index_list for x in y]
        
        GridTable = self._group(index_list, rows, ants, ras, decs, stats, CombineRadius, Allowance, GridSpacing, DecCorrection)

        end = time.time()
        LOG.info('GroupForGrid: elapsed time %s sec (except table access %s sec)'%(end-start,end-start1))
        return index_list, GridTable

    def Process8(self, DataIn, index_list, NCHAN, GridTable, CombineRadius, GridRule, LogLevel=2):
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
        
        # create storage
        num_spectra = len(index_list)
        num_data = len(DataIn)
        num_spectra_per_data = num_spectra / num_data
        SpStorage = numpy.zeros((num_spectra, NCHAN), dtype=numpy.float32)

        if NCHAN != 1:
            clip = GridRule['Clipping']
            rms_weight = GridRule['WeightRMS']
            tsys_weight = GridRule['WeightTsysExptime']
        else:
            clip = 'none'
            rms_weight = False
            tsys_weight = True
        weight = GridRule['WeightDistance']

        NROW = len(GridTable)
        LOG.info('Accumulate nearby spectrum for each Grid position...')
        LOG.info('Processing %d spectra...' % (NROW))
        OutputTable = []
        # create storage for output
        StorageOut = numpy.ones((NROW, NCHAN), dtype=numpy.float32) * NoData
        ID = 0

        tROW = self.datatable.getcol('ROW')
        tSFLAG = self.datatable.getcol('FLAG_SUMMARY')
        tTSYS = self.datatable.getcol('TSYS')
        tEXPT = self.datatable.getcol('EXPOSURE')

        rows = numpy.take(tROW, index_list)
        net_flag = numpy.take(tSFLAG, index_list)
        tsys = numpy.take(tTSYS, index_list)
        exposure = numpy.take(tEXPT, index_list)

        # 2011/11/12 DataIn and rowsSel are [list]
        IDX2StorageID = {}
        StorageID = 0
        for i in xrange(len(DataIn)):
            # read data to SpStorage
            with casatools.TableReader(DataIn[i]) as tb:
                for j in xrange(num_spectra_per_data):
                    x = index_list[StorageID]
                    SpStorage[StorageID] = tb.getcell('SPECTRA', rows[StorageID])
                    IDX2StorageID[x] = StorageID
                    StorageID += 1
                LOG.debug('Data Stored in SpStorage')

        # Create progress timer
        Timer = common.ProgressTimer(80, NROW, LogLevel)
        for [IF, POL, X, Y, RAcent, DECcent, RowDelta] in GridTable:
            rowlist = []
            indexlist = []
            deltalist = []
            rmslist = []
            flagged = 0
            for [row, delta, rms, index, ant] in RowDelta:
                # Check Summary Flag
                _index = IDX2StorageID[int(index)]
                if net_flag[_index] == 1:
                    rowlist.append(int(row))
                    indexlist.append(_index)
                    deltalist.append(delta)
                    rmslist.append(rms)
                else: flagged += 1
            num_valid_row = len(rowlist)
            if num_valid_row == 0:
                # No valid Spectra at the position
                RMS = 0.0
                pass
            elif num_valid_row == 1:
                # One valid Spectrum at the position
                StorageOut[ID] = SpStorage[0]
                RMS = rmslist[0]
            else:
                # More than one valid Spectra at the position
                data = SpStorage[indexlist]
                w = numpy.ones(numpy.shape(data), numpy.float32)
                weightlist = numpy.ones(num_valid_row, numpy.float32)
                # Clipping
                if clip.upper() == 'MINMAXREJECT' and num_valid_row > 2:
                    w[numpy.argmin(data, axis=0), range(len(data[0]))] = 0.0
                    w[numpy.argmax(data, axis=0), range(len(data[0]))] = 0.0
                # Weight by RMS
                # Weight = 1/(RMS**2)
                weight_factor = 0.0
                if rms_weight == True:
                    for m in xrange(num_valid_row):
                        if rmslist[m] != 0.0:
                            weight_factor = rmslist[m] * rmslist[m]
                            w[m] /= weight_factor
                            weightlist[m] /= weight_factor
                        else:
                            w[m] = 0.0
                            weightlist[m] = 0.0
                # Weight by Exptime & Tsys
                # RMS = n * Tsys/math.sqrt(Exptime)
                # Weight = 1/(RMS**2) = (Exptime/(Tsys**2))
                if tsys_weight == True:
                    for m in xrange(num_valid_row):
                        # 2008/9/21 Bug fix
                        # 2013/05/30 TN Bug fix
                        # using rowlist as index is invalid
                        if tsys[indexlist[m]] > 0.5:
                        #if tTSYS[rowlist[m]] > 0.5:
                            #weight_factor = (tEXPT[rowlist[m]]/(tTSYS[rowlist[m]]*tTSYS[rowlist[m]]))
                            weight_factor = (exposure[indexlist[m]]/(tsys[indexlist[m]]*tsys[indexlist[m]]))
                            LOG.debug('tEXPT[rowlist[m]]=%s, tTSYS[rowlist[m]]=%s'%(tEXPT[rowlist[m]], tTSYS[rowlist[m]]))
                            LOG.debug('exposure[indexlist[m]]=%s, tsys[indexlist[m]]=%s'%(exposure[indexlist[m]], tsys[indexlist[m]]))
                            w[m] *= weight_factor
                            weightlist[m] *= weight_factor
                            #if self.datatable[m][DT_TSYS] > 0.5:
                            #    w[m] *= (self.datatable[m][DT_EXPT]/(self.datatable[m][DT_TSYS]**2))
                            #    weightlist[m] *= (self.datatable[m][DT_EXPT]/(self.datatable[m][DT_TSYS]**2))
                        else:
                            w[m] = 0.0
                            weightlist[m] = 0.0
                # Weight by radius
                if weight.upper() == 'GAUSS':
                    # weight = exp(-ln2*((r/hwhm)**2))
                    for m in xrange(num_valid_row):
                        weight_factor = (math.exp(-0.69314718055994529*((deltalist[m]*deltalist[m]/(CombineRadius*CombineRadius)))))
                        w[m] *= weight_factor
                        weightlist[m] *= weight_factor
                elif weight.upper() == 'LINEAR':
                    # weight = 0.5 + (hwhm - r)/2/hwhm = 1.0 - r/2/hwhm
                    for m in xrange(num_valid_row):
                        weight_factor = (1.0 - 0.5*deltalist[m]/CombineRadius)
                        w[m] *= weight_factor
                        weightlist[m] *= weight_factor
                # Combine Spectra
                if w.sum() != 0: StorageOut[ID] = (numpy.sum(data * w, axis=0) / numpy.sum(w,axis=0))
                # Calculate RMS of the spectrum
                r0 = 0.0
                r1 = 0.0
                for m in xrange(num_valid_row):
                    r0 += (rmslist[m] * weightlist[m]) * (rmslist[m] * weightlist[m])
                    r1 += weightlist[m]
                RMS = math.sqrt(r0) / r1

            OutputTable.append([IF, POL, X, Y, RAcent, DECcent, num_valid_row, flagged, RMS])
            ID += 1
            del indexlist, deltalist, rmslist

            # Countup progress timer
            Timer.count()

        end = time.time()
        LOG.info('Process8: elapsed time %s sec'%(end-start))
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

        tot = 0.0
        tot2 = 0.0
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
        LOG.info('test: accumulated elapsed time %s sec vs. %s sec'%(tot,tot2))
        LOG.info('_group: elapsed time %s sec'%(end-start))
        return GridTable

    def _grid_table(self, index_list, rows, ants, ras, decs, stats, CombineRadius, Allowance, GridSpacing, DecCorrection):
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

        LOG.info('NGridRA = %s  NGridDEC = %s' % (NGridRA, NGridDEC))
        
        MinRA = (MinRA + MaxRA) / 2.0 - (NGridRA - 1) / 2.0 * GridSpacing * DecCorrection
        MinDEC = (MinDEC + MaxDEC) / 2.0 - (NGridDEC - 1) / 2.0 * GridSpacing

        tot = 0.0
        tot2 = 0.0
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
                #GridTable.append(line)
                yield line
                #LOG.debug("GridTable: %s" % line)

        end = time.time()
        LOG.info('test: accumulated elapsed time %s sec vs. %s sec'%(tot,tot2))
        LOG.info('_group: elapsed time %s sec'%(end-start))
        #return GridTable

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
        GridTable.append(line)
        #LOG.debug("GridTable: %s" % line)
        end = time.time()

        LOG.info('NGridRA = %s  NGridDEC = %s' % (NGridRA, NGridDEC))

        LOG.info('_group: elapsed time %s sec'%(end-start))
        return GridTable

    def _grid_table(self, index_list, rows, ants, ras, decs, stats, CombineRadius, Allowance, GridSpacing, DecCorrection):
        """
        Gridding by RA/DEC position
        """
        start = time.time()

        GridTable = []
        NROW = len(index_list)

        NGridRA = 1
        NGridDEC = 1
        LOG.info('NGridRA = %s  NGridDEC = %s' % (NGridRA, NGridDEC))
        CenterRA = ras.mean()
        CenterDEC = decs.mean()
        line = [self.spw, self.pol, 0, 0, CenterRA, CenterDEC, []]
        for x in range(len(index_list)):
            Delta = math.sqrt((ras[x] - CenterRA) ** 2.0 + (decs[x] - CenterDEC) ** 2.0)
            if Delta <= Allowance:
                line[6].append([index_list[x], Delta, stats[x], index_list[x], ants[x]])
        #GridTable.append(line)
        yield line
        #LOG.debug("GridTable: %s" % line)
        end = time.time()

        LOG.info('_group: elapsed time %s sec'%(end-start))
        #return GridTable

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
                    GridTable.append(line)
                    #LOG.debug("GridTable: %s" % line)
        del Flag

        LOG.info('NGridRA = %s  NGridDEC = %s' % (NGridRA, NGridDEC))

        end = time.time()
        LOG.info('_group: elapsed time %s sec'%(end-start))
        return GridTable
        
    def _grid_table(self, index_list, rows, ants, ras, decs, stats, CombineRadius, Allowance, GridSpacing, DecCorrection):
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
                    #GridTable.append(line)
                    yield line
                    #LOG.debug("GridTable: %s" % line)
        del Flag

        LOG.info('NGridRA = %s  NGridDEC = %s' % (NGridRA, NGridDEC))

        end = time.time()
        LOG.info('_group: elapsed time %s sec'%(end-start))
        #return GridTable
        
