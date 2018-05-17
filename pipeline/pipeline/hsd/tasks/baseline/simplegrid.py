from __future__ import absolute_import

import os
import numpy
import collections
import itertools
from math import cos

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
import pipeline.infrastructure.casatools as casatools
from pipeline.domain.datatable import DataTableImpl as DataTable
from .. import common
from ..common import utils

_LOG = infrastructure.get_logger(__name__)
LOG = utils.OnDemandStringParseLogger(_LOG)

NoData = common.NoData
DO_TEST = False

class SDSimpleGriddingInputs(vdp.StandardInputs):
    nplane = vdp.VisDependentProperty(default=3)
    
    @property
    def group_desc(self):
        return self.context.observing_run.ms_reduction_group[self.group_id]
    
    @property
    def reference_member(self):
        return self.group_desc[self.member_list[0]]

    def __init__(self, context, group_id, member_list, 
                 nplane=None):
        super(SDSimpleGriddingInputs, self).__init__()
        
        self.context = context
        self.group_id = group_id
        self.member_list = member_list
        self.nplane = nplane

class SDSimpleGriddingResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDSimpleGriddingResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDSimpleGriddingResults, self).merge_with_context(context)
    
    def _outcome_name(self):
        return ''


class SDSimpleGridding(basetask.StandardTaskTemplate):
    Inputs = SDSimpleGriddingInputs

    def prepare(self, datatable=None, index_list=None):
        if datatable is None:
            LOG.debug('#PNP# instantiate local datatable')
            datatable = DataTable(self.inputs.context.observing_run.ms_datatable_name)
        else:
            LOG.debug('datatable is propagated from parent task')
            
        assert index_list is not None

        grid_table = self.make_grid_table(datatable, index_list)
        # LOG.debug('work_dir=%s'%(work_dir))
        import time
        start = time.time()
        retval = self.grid(grid_table=grid_table, datatable=datatable)
        end = time.time()
        LOG.debug('Elapsed time: {} sec', (end - start))
        
        outcome = {'spectral_data': retval[0],
                   'meta_data': retval[1],
                   'grid_table': grid_table}
        result = SDSimpleGriddingResults(task=self.__class__,
                                       success=True,
                                       outcome=outcome)
        result.task = self.__class__
                       
        return result
    
    def analyse(self, result):
        return result

    def make_grid_table(self, datatable, index_list):
        """
        Calculate Parameters for grid by RA/DEC positions
        """
        reference_data = self.inputs.reference_member.ms
        reference_antenna = self.inputs.reference_member.antenna_id
        reference_spw = self.inputs.reference_member.spw_id
        beam_size = reference_data.beam_sizes[reference_antenna][reference_spw]
        grid_size = casatools.quanta.convert(beam_size, 'deg')['value']
        
        ras = datatable.getcol('RA').take(index_list)
        decs = datatable.getcol('DEC').take(index_list)
        
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
        LOG.info(' RA range: [{}, {}]', min_ra, max_ra)
        LOG.info('DEC range: [{}, {}]', min_dec, max_dec)
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
        for (ira, idec) in itertools.izip(index_ra, index_dec):
            combine_list[counter[ira][idec] % nplane][ira][idec].append(index)
            counter[ira][idec] += 1
            index += 1
        del index, index_ra, index_dec, counter

        # Create grid_table for output
        grid_table = []
        # vIF, vPOL: dummy (not necessary)
        vIF = reference_spw
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
                        #Delta = sqrt((ras[index] - RA) * (ras[index] - RA)
                        #             * dec_corr * dec_corr 
                        #             + (decs[index] - DEC) * (decs[index] - DEC))
                        datatable_index = index_list[index]
                        row = datatable.getcell('ROW', datatable_index)
                        #stat = datatable.getcell('STATISTICS', datatable_index)[0] 
                        ant = datatable.getcell('ANTENNA', datatable_index)
                        msid = datatable.getcell('MS', datatable_index)
                        line[6].append([row, None, None, datatable_index, ant, msid])
                    line[6] = numpy.array(line[6])
                    grid_table.append(line)
        del ras, decs, combine_list

        LOG.info('ngrid_ra = {}  ngrid_dec = {}', ngrid_ra, ngrid_dec)
        return grid_table

    def grid(self, grid_table, datatable):
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
        LOG.info('SimpleGrid: Processing {} spectra...', nrow)

        reference_data = self.inputs.reference_member.ms
        reference_spw = self.inputs.reference_member.spw_id 
        nchan = reference_data.spectral_windows[reference_spw].num_channels
        npol = reference_data.get_data_description(spw=reference_spw).num_polarizations
        LOG.debug('nrow={} nchan={} npol={}', nrow,nchan,npol)
        
        #tTSYS = datatable.getcol('TSYS')
        tEXPT = datatable.getcol('EXPOSURE')
        #tSFLAG = datatable.getcol('FLAG_SUMMARY')
            
        # loop for all ROWs in grid_table to make dictionary that 
        # associates spectra in data_in and weights with grids.
        # bind_to_grid = dict([(k,[]) for k in self.data_in.keys()])
        bind_to_grid = collections.defaultdict(list)
        for grid_table_row in xrange(nrow):
            [IF, POL, X, Y, RAcent, DECcent, RowDelta] = grid_table[grid_table_row]
            for [_data_row, _, _, _index, _ant, _msid] in RowDelta:
                index = int(_index)
                msid = int(_msid)
                ms = self.inputs.context.observing_run.measurement_sets[msid]
                data_row = int(_data_row)
                tSFLAG = datatable.getcell('FLAG_SUMMARY', index)
                tTSYS = datatable.getcell('TSYS', index)
                #tEXPT = datatable.getcell('EXPOSURE', index)
                exposure = tEXPT[index]
                pols = []
                flags = []
                weights = []
                for (pol, flag_summary) in enumerate(tSFLAG):
                    if flag_summary == 1:
                        if tTSYS[pol] > 0.5 and exposure > 0.0:
                            Weight = exposure / (tTSYS[pol] ** 2.0)
                        else:
                            Weight = 1.0
                        pols.append(pol)
                        flags.append(flag_summary)
                        weights.append(Weight)
                bind_to_grid[ms].append([data_row, grid_table_row, pols, weights, flags])
#                 if tSFLAG[index] == 1:
#                     if tTSYS[index] > 0.5 and tEXPT[index] > 0.0:
#                         Weight = tEXPT[index] / (tTSYS[index] ** 2.0)
#                     else: Weight = 1.0
#                     bind_to_grid[vis].append([data_row, grid_table_row, Weight, tSFLAG[index]])
#         del tTSYS, tEXPT, tSFLAG
        LOG.debug('bind_to_grid.keys() = %s'%(map(lambda x: x.name, bind_to_grid.keys())))
        LOG.debug('bind_to_grid={}', bind_to_grid)
        
        def cmp(x, y):
            if x[0] < y[0]:
                return -1
            elif x[0] > y[0]:
                return 1
            elif x[1] < y[1]:
                return -1
            elif x[1] > y[1]:
                return 1
            return 0
        for (k,v) in bind_to_grid.iteritems():
            v.sort(cmp=cmp)
        LOG.debug('sorted bind_to_grid={}', bind_to_grid)

        # create storage for output
        StorageOut = numpy.zeros((nrow, nchan), dtype=numpy.complex)
        #FlagOut = numpy.zeros((nrow, nchan), dtype=numpy.int)
        StorageWeight = numpy.zeros((nrow, nchan), dtype=numpy.float32)
        #StorageNumSp = numpy.zeros((nrow, nchan), dtype=numpy.int)
        StorageNumSp = numpy.zeros((nrow), dtype=numpy.int)
        StorageNumFlag = numpy.zeros((nrow), dtype=numpy.int)
        OutputTable = []

        # Return empty result if all the spectra are flagged out
        number_of_spectra = sum(map(len, bind_to_grid.values()))
        if number_of_spectra == 0:
            LOG.warn('Empty grid table, maybe all the data are flagged out in the previous step.')
            return ([], [])
        
        # Create progress timer
        Timer = common.ProgressTimer(80, sum(map(len, bind_to_grid.values())), LOG.logger.level)

        # loop for antennas
        # for AntID in index_list:
        #for vis in infiles:
        #for i in xrange(len(antenna_list)):
        #query = lambda condition: 1 if condition else 0
        #vquery = numpy.vectorize(query)
        for (ms, entries) in bind_to_grid.iteritems():
            #AntID = antenna_list[i]
            #with casatools.TableReader(infiles[i]) as tb:
            vis = ms.work_data
            ms_colname = utils.get_datacolumn_name(vis)
            rowmap = utils.make_row_map_for_baselined_ms(ms)
            LOG.debug('Start reading data from "{}"', os.path.basename(vis))
            LOG.debug('There are {} entries', len(entries))
            with casatools.TableReader(vis) as tb:
                #get = lambda col, row: tb.getcell(col, row)
                #for entry in bind_to_grid[AntID]:
                for entry in entries:
                    [tROW, ROW, pols, weights, flags] = entry
                    Sp = None
                    Mask = None
                    mapped_row = rowmap[tROW]
                    LOG.debug('tROW {}: mapped_row {}', tROW, mapped_row)
                    for (Weight, Pol, SFLAG) in itertools.izip(weights, pols, flags):
                        if SFLAG == 1:
                            if Sp is None:
                                Sp = tb.getcell(ms_colname, mapped_row)
                            if numpy.any(numpy.isnan(Sp[Pol])):
                                LOG.debug('vis "{}" row {} pol {} contains NaN', os.path.basename(vis),tROW,Pol)
                            if Mask is None:
                                Mask = numpy.asarray(numpy.logical_not(tb.getcell('FLAG', mapped_row)),
                                                     dtype=int)#vquery(tb.getcell('FLAG', mapped_row) == False)
                            #LOG.debug('Mask.shape = {shape}'.format(shape=Mask.shape))
                            #collapsed_mask = numpy.any(Mask == 1, axis=1)
                            #LOG.debug('collapsed_mask = %s'%(collapsed_mask))
                            StorageOut[ROW] += (Sp[Pol] * Mask[Pol] * Weight)
                            StorageWeight[ROW] += (Mask[Pol] * Weight)
                            StorageNumSp[ROW] += 1 if numpy.any(Mask[Pol] == 1) else 0#query(any(Mask[Pol] == 1))
                            #StorageOut[ROW] += (Sp * Mask * Weight).sum(axis=0)
                            #StorageWeight[ROW] += (Mask * Weight).sum(axis=0)
                            #StorageNumSp[ROW] += numpy.sum(vquery(numpy.any(Mask == 1, axis=1)))#query(any(Mask == 1))
                        else:
                            StorageNumFlag[ROW] += 1
                    Timer.count()
            LOG.debug('DONE')

        # Calculate Tsys-ExpTime weighted average
        # RMS = n * Tsys/sqrt(Exptime)
        # Weight = 1/(RMS**2) = (Exptime/(Tsys**2))
        for ROW in range(nrow):
            LOG.debug('Calculate weighed average for row {}', ROW)
            [IF, POL, X, Y, RAcent, DECcent, RowDelta] = grid_table[ROW]
            if StorageNumSp[ROW] == 0 or all(StorageWeight[ROW] == 0.0):
                StorageOut[ROW,:] = NoData
                #FlagOut[ROW,:] = 1
                RMS = 0.0
            else:
                for ichan in xrange(nchan):
                    if StorageWeight[ROW,ichan] == 0.0:
                        StorageOut[ROW,ichan] = NoData
                        #FlagOut[ROW,ichan] = 1
                    else:
                        StorageOut[ROW,ichan] /= StorageWeight[ROW,ichan]
                RMS = 1.0
            OutputTable.append([IF, POL, X, Y, RAcent, DECcent, StorageNumSp[ROW], StorageNumFlag[ROW], RMS])

        del StorageWeight, StorageNumSp, StorageNumFlag, Timer 
        #return (StorageOut, FlagOut, OutputTable)
        #LOG.debug('StorageOut=%s'%(numpy.real(StorageOut).tolist()))
        return (numpy.real(StorageOut), OutputTable)
