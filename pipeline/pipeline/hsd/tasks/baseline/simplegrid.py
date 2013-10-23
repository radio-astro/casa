from __future__ import absolute_import

import os
#import math
from math import cos, sqrt
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from .. import common

LOG = infrastructure.get_logger(__name__)

NoData = common.NoData

class SimpleGriddingInputs(common.SingleDishInputs):
    def __init__(self, context, antennalist, spwid, nplane=None):
        self._init_properties(vars())
        
    @property
    def nplane(self):
        return 3 if self._nplane is None else self._nplane
    
    @nplane.setter
    def nplane(self, value):
        self._nplane = value
        
class SimpleGriddingResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SimpleGriddingResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        pass
    
    def _outcome_name(self):
        return ''

class SimpleGridding(common.SingleDishTaskTemplate):
    Inputs = SimpleGriddingInputs
    def prepare(self):
        grid_table = self.make_grid_table()
        # LOG.debug('work_dir=%s'%(work_dir))
        import time
        start = time.time()
        retval = self.grid(grid_table=grid_table)
        end = time.time()
        LOG.debug('Elapsed time: %s sec' % (end - start))
        
        outcome = {'spectral_data': retval[0],
                   'grid_table': retval[1]}
        result = SimpleGriddingResults(task=self.__class__,
                                       success=True,
                                       outcome=outcome)
        result.task = self.__class__
                       
        if self.context.subtask_counter is 0: 
            result.stage_number = self.context.task_counter - 1
        else:
            result.stage_number = self.context.task_counter 
                
        return result
    
    def analyse(self, result):
        return result

    def make_grid_table(self):
        """
        Calculate Parameters for grid by RA/DEC positions
        """
        spwid = self.inputs.spwid
        index_list = self.inputs.antennalist
        reference_data = self.context.observing_run[index_list[0]]
        srctype = reference_data.calibration_strategy['srctype']
        beam_size = reference_data.beam_size[spwid]
        grid_size = casatools.quanta.convert(beam_size, 'deg')['value']
        
        # need opened table to call taql, so we use table object that 
        # datatable holds.
        table = self.datatable.tb1

        # TaQL command to make a view from DataTable that is physically 
        # separated to two tables named RO and RW, respectively. 
        # Note that TaQL accesses DataTable on disk, not on memory. 
        datatable_name = self.datatable.plaintable
        taqlstring = 'USING STYLE PYTHON SELECT ROWNUMBER() AS ID, RO.ROW, RO.ANTENNA, RO.RA, RO.DEC, RW.STATISTICS[0] AS STAT FROM "%s" RO, "%s" RW WHERE IF==%s'\
            % (os.path.join(datatable_name, 'RO'), os.path.join(datatable_name, 'RW'), spwid)
        if srctype is not None:
            taqlstring += ' && SRCTYPE==%s' % (srctype)
        tx = table.taql(taqlstring)
        index_list = tx.getcol('ID')
        rows = tx.getcol('ROW')
        ants = tx.getcol('ANTENNA')
        ras = tx.getcol('RA')
        decs = tx.getcol('DEC')
        stats = tx.getcol('STAT')
        tx.close()
        del tx

        # Curvature has not been taken account
        dec_corr = 1.0 / cos(decs[0] / 180.0 * 3.141592653)
        grid_ra_corr = grid_size * dec_corr
        grid_dec = grid_size

        min_ra = ras.min()
        max_ra = ras.max()
        min_dec = decs.min()
        max_dec = decs.max()
        # Check if the distribution crosses over the RA=0
        if min_ra < 10 and max_ra > 350:
            ras = ras + numpy.less_equal(ras, 180) * 360.0
            min_ra = ras.min()
            max_ra = ras.max()
        LOG.info(' RA range: [%s, %s]' % (min_ra, max_ra))
        LOG.info('DEC range: [%s, %s]' % (min_dec, max_dec))
        ngrid_ra = int(int((max_ra - min_ra + grid_ra_corr) / (2.0 * grid_ra_corr)) * 2 + 1)
        ngrid_dec = int(int((max_dec - min_dec + grid_dec) / (2.0 * grid_dec)) * 2 + 1)
        min_ra = (min_ra + max_ra - ngrid_ra * grid_ra_corr) / 2.0
        min_dec = (min_dec + max_dec - ngrid_dec * grid_dec) / 2.0

        # Calculate Grid index for each position
        igrid_ra_corr = 1.0 / grid_ra_corr
        igrid_dec = 1.0 / grid_dec
        index_ra = numpy.array((ras - min_ra) * igrid_ra_corr, dtype=numpy.int)
        index_dec = numpy.array((decs - min_dec) * igrid_dec, dtype=numpy.int)

        # Counter for distributing spectrum into several planes (nplane)
        counter = numpy.zeros((ngrid_ra, ngrid_dec), dtype=numpy.int)

        # Make lists to store indexes for combine spectrum
        nplane = self.inputs.nplane
        combine_list = []
        for p in range(nplane):
            combine_list.append([])
            for x in range(ngrid_ra):
                combine_list[p].append([])
                for y in range(ngrid_dec):
                    combine_list[p][x].append([])

        # Store indexes
        index = 0
        for (ira, idec) in zip(index_ra, index_dec):
            combine_list[counter[ira][idec] % nplane][ira][idec].append(index)
            counter[ira][idec] += 1
            index += 1
        del index, index_ra, index_dec, counter

        # Create grid_table for output
        grid_table = []
        # vIF, vPOL: dummy (not necessary)
        vIF = spwid
        vPOL = 0

        for y in range(ngrid_dec):
            DEC = min_dec + grid_dec * (y + 0.5)
            for x in range(ngrid_ra):
                RA = min_ra + grid_ra_corr * (x + 0.5)
                for p in range(nplane):
                    line = [vIF, vPOL, x, y, RA, DEC, []]
                    for index in combine_list[p][x][y]:
                        # math.sqrt is twice as fast as ** 0.5 according to 
                        # the measurement on alma-cluster-proto03 in NAOJ 
                        # (3.5GHz CPU 8 Cores, 16GB RAM). 
                        # Furthermore, direct import of sqrt from math is 
                        # slightly (~10%) faster than calling sqrt using 
                        # 'math.sqrt'.
                        # Also, x * x is ~30% faster than x ** 2.0.
                        # Delta = (((ras[index] - RA) * dec_corr) ** 2.0 + \
                        #         (decs[index] - DEC) ** 2.0) ** 0.5
                        Delta = sqrt((ras[index] - RA) * (ras[index] - RA)
                                     * dec_corr * dec_corr 
                                     + (decs[index] - DEC) * (decs[index] - DEC))
                        line[6].append([rows[index], Delta, stats[index], index_list[index], ants[index]])
                    grid_table.append(line)
                    # LOG.info("grid_table: %s" % line)
        del ras, decs, combine_list

        LOG.info('ngrid_ra = %s  ngrid_dec = %s' % (ngrid_ra, ngrid_dec))
        return grid_table


    def grid(self, grid_table):
        """
        The process does re-map and combine spectrum for each position
        grid_table format:
          [[IF,POL,0,0,RAcent,DECcent,[[row0,r0,RMS0,index0,ant0],[row1,r1,RMS1,index1,ant1],..,[rowN,rN,RMSN,indexN,antn]]]
           [IF,POL,0,1,RAcent,DECcent,[[row0,r0,RMS0,index0,ant0],[row1,r1,RMS1,index1,ant1],..,[rowN,rN,RMSN,indexN,antn]]]
                        ......
           [IF,POL,M,N,RAcent,DECcent,[[row0,r0,RMS0,index0,ant0],[row1,r1,RMS1,index1,ant1],..,[rowN,rN,RMSN,indexN,antn]]]]
         where row0,row1,...,rowN should be combined to one for better S/N spectra
               'r' is a distance from grid position
          Number of spectra output is len(grid_table)
        OutputTable format:
           [[IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]
            [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]
                    ......
            [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]]

        """
        nrow = len(grid_table)
        LOG.info('SimpleGrid: Processing %d spectra...' % (nrow))

        spwid = self.inputs.spwid
        index_list = self.inputs.antennalist
        infiles = [self.context.observing_run[i].work_data for i in index_list]
        reference_data = self.context.observing_run[index_list[0]]
        nchan = reference_data.spectral_window[spwid].nchan
        
        # create storage for output
        StorageOut = numpy.zeros((nrow, nchan), dtype=numpy.float32)
        StorageWeight = numpy.zeros((nrow), dtype=numpy.float32)
        StorageNumSp = numpy.zeros((nrow), dtype=numpy.int)
        StorageNumFlag = numpy.zeros((nrow), dtype=numpy.int)
        StorageNoData = numpy.ones((nchan), dtype=numpy.float32) * NoData
        OutputTable = []

        tROW = self.datatable.getcol('ROW')
        tTSYS = self.datatable.getcol('TSYS')
        tEXPT = self.datatable.getcol('EXPOSURE')
        tSFLAG = self.datatable.getcol('FLAG_SUMMARY')
            
        # loop for all ROWs in grid_table to make dictionary that 
        # associates spectra in data_in and weights with grids.
        # bind_to_grid = dict([(k,[]) for k in self.data_in.keys()])
        bind_to_grid = dict([(k, []) for k in index_list])
        for ROW in xrange(nrow):
            [IF, POL, X, Y, RAcent, DECcent, RowDelta] = grid_table[ROW]
            for [row, delta, rms, index, ant] in RowDelta:
                if tSFLAG[index] == 1:
                    if tTSYS[index] > 0.5 and tEXPT[index] > 0.0:
                        Weight = tEXPT[index] / (tTSYS[index] ** 2.0)
                    else: Weight = 1.0
                    bind_to_grid[ant].append([tROW[index], ROW, Weight, tSFLAG[index]])

        # Return empty result if all the spectra are flagged out
        number_of_spectra = sum(map(len, bind_to_grid.values()))
        if number_of_spectra == 0:
            LOG.warn('Empty grid table, maybe all the data are flagged out in the previous step.')
            return ([], [])
        
        # Create progress timer
        Timer = common.ProgressTimer(80, sum(map(len, bind_to_grid.values())), LOG.level)

        # loop for antennas
        # for AntID in index_list:
        for i in xrange(len(index_list)):
            AntID = index_list[i]
            with casatools.TableReader(infiles[i]) as tb:
                for entry in bind_to_grid[AntID]:
                    [tROW, ROW, Weight, tSFLAG] = entry
                    if tSFLAG == 1:
                        Sp = tb.getcell('SPECTRA', tROW)
                        StorageOut[ROW] += Sp * Weight
                        StorageWeight[ROW] += Weight
                        StorageNumSp[ROW] += 1
                    else:
                        StorageNumFlag[ROW] += 1
                    Timer.count()

        # Calculate Tsys-ExpTime weighted average
        # RMS = n * Tsys/sqrt(Exptime)
        # Weight = 1/(RMS**2) = (Exptime/(Tsys**2))
        for ROW in range(nrow):
            [IF, POL, X, Y, RAcent, DECcent, RowDelta] = grid_table[ROW]
            if StorageNumSp[ROW] == 0:
                StorageOut[ROW] = StorageNoData
                RMS = 0.0
            else:
                StorageOut[ROW] /= StorageWeight[ROW]
                RMS = 1.0
            OutputTable.append([IF, POL, X, Y, RAcent, DECcent, StorageNumSp[ROW], StorageNumFlag[ROW], RMS])

        del StorageWeight, StorageNumSp, StorageNumFlag, StorageNoData, Timer
        return (StorageOut, OutputTable)
