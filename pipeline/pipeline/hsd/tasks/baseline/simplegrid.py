from __future__ import absolute_import

import os
#import math
from math import cos, sqrt
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from pipeline.infrastructure import JobRequest
from .. import common

LOG = infrastructure.get_logger(__name__)

subsqr = lambda x, y: (x - y) * (x - y)
NoData = common.NoData

class SDSimpleGridInputs(common.SingleDishInputs):
    """
    Inputs for simple gridding
    """
    def __init__(self, context, infiles=None, iflist=None):
        self._init_properties(vars())

    @property
    def infiles(self):
        if self._infiles is not None:
            return self._infiles
        else:
            return self.context.observing_run.st_names

    @infiles.setter
    def infiles(self, value):
        if isinstance(value, list):
            for v in value:
                self.context.observing_run.st_names.index(v)
        elif isinstance(value, str):
            self.context.observing_run.st_names.index(value)
            
        LOG.info('Setting Input._infiles to %s'%(value))
        self._infiles = value        

class SDSimpleGridResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDSimpleGridResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDSimpleGridResults, self).merge_with_context(context)
        LOG.todo('need to decide what is done in SDSimpleGridResults.merge_with_context')

    def _outcome_name(self):
        return self.outcome.__str__()

class SDSimpleGrid(common.SingleDishTaskTemplate):
    Inputs = SDSimpleGridInputs

    def prepare(self):
        context = self.inputs.context
        datatable = context.observing_run.datatable_instance
        reduction_group = context.observing_run.reduction_group
        qa = casatools.quanta
        grid_tables = []
        infiles = self.inputs.infiles
        iflist = self.inputs.iflist
        st_names = context.observing_run.st_names
        file_index = [st_names.index(infile) for infile in infiles]
        for (group_id,group_desc) in reduction_group.items():
            # assume all members have same spw
            spw = group_desc.menber_list[0].spw
            LOG.debug('spw=%s'%(spw))

            # skip spw that is not included in iflist
            if iflist is not None and spw not in iflist:
                LOG.debug('Skip spw %s'%(spw))
                continue

            # assume all members have same calmode
            ant = group_desc.member_list[0].antenna
            st = context.observing_run[ant]
            calmode = st.calibration_strategy['calmode']
            srctype = common.SrcTypeMap(calmode)
            
            # beam size
            grid_size = qa.convert(st.beam_size[spw], 'deg')['value']

            worker = SimpleGridding(datatable, spw, srctype, grid_size, file_index)
            
            # create job
            job = JobRequest(worker.execute) 
            
            # execute job
            results = self._executor.execute(job)

            grid_tables.append(results)

        #return grid_tables
        result = SDSimpleGridResults(task = self.__class__,
                                     success = True, 
                                     outcome = grid_tables)
        result.task = self.__class__
        return result

    def analyse(self, result):
        return result

class SimpleGridding(object):
    def __init__(self, datatable, spw, srctype, grid_size, data_in, nplane=3):
        self.datatable = datatable
        self.datatable_name = self.datatable.plaintable
        self.spw = spw
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
        self.nplane = nplane
        self.data_in = data_in

    def execute(self):
        grid_table = self.make_grid_table()
        work_dir = os.path.join('/',*(self.datatable.plaintable.split('/')[:-2]))
        #LOG.debug('work_dir=%s'%(work_dir))
        filenames = [os.path.join(work_dir,f) for f in self.datatable.getkeyword('FILENAMES')]
        import time
        start = time.time()
        retval = self.grid(grid_table=grid_table)
        end = time.time()
        LOG.debug('Elapsed time: %s sec'%(end-start))
        return retval

    def make_grid_table(self):
        """
        Calculate Parameters for grid by RA/DEC positions
        """
        # need opened table to call taql, so we use table object that 
        # datatable holds.
        table = self.datatable.tb1

        # TaQL command to make a view from DataTable that is physically 
        # separated to two tables named RO and RW, respectively. 
        # Note that TaQL accesses DataTable on disk, not on memory. 
        taqlstring = 'USING STYLE PYTHON SELECT ROWNUMBER() AS ID, RO.ROW, RO.ANTENNA, RO.RA, RO.DEC, RW.STATISTICS[0] AS STAT FROM "%s" RO, "%s" RW WHERE IF==%s'%(os.path.join(self.datatable_name,'RO'),os.path.join(self.datatable_name,'RW'),self.spw)
        if self.srctype is not None:
            taqlstring += ' && SRCTYPE==%s'%(self.srctype)
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
        grid_ra_corr = self.grid_ra * dec_corr

        min_ra = ras.min()
        max_ra = ras.max()
        min_dec = decs.min()
        max_dec = decs.max()
        # Check if the distribution crosses over the RA=0
        if min_ra < 10 and max_ra > 350:
            ras = ras + numpy.less_equal(ras, 180) * 360.0
            min_ra = ras.min()
            max_ra = ras.max()
        LOG.info(' RA: [%s, %s]'%(min_ra, max_ra))
        LOG.info('DEC: [%s, %s]'%(min_dec, max_dec))
        ngrid_ra = int(int((max_ra - min_ra + grid_ra_corr)  / (2.0 * grid_ra_corr)) * 2 + 1)
        ngrid_dec = int(int((max_dec - min_dec + self.grid_dec) / (2.0 * self.grid_dec)) * 2 + 1)
        min_ra = (min_ra + max_ra - ngrid_ra * grid_ra_corr) / 2.0
        min_dec = (min_dec + max_dec - ngrid_dec * self.grid_dec) / 2.0

        # Calculate Grid index for each position
        igrid_ra_corr = 1.0 / grid_ra_corr
        igrid_dec = 1.0 / self.grid_dec
        index_ra = numpy.array((ras - min_ra) * igrid_ra_corr, dtype=numpy.int)
        index_dec = numpy.array((decs - min_dec) * igrid_dec, dtype=numpy.int)

        # Counter for distributing spectrum into several planes (self.nplane)
        counter = numpy.zeros((ngrid_ra, ngrid_dec), dtype=numpy.int)

        # Make lists to store indexes for combine spectrum
        combine_list = []
        for p in range(self.nplane):
            combine_list.append([])
            for x in range(ngrid_ra):
                combine_list[p].append([])
                for y in range(ngrid_dec):
                    combine_list[p][x].append([])

        # Store indexes
        index = 0
        for (ira,idec) in zip(index_ra,index_dec):
            combine_list[counter[ira][idec] % self.nplane][ira][idec].append(index)
            counter[ira][idec] += 1
            index += 1
        del index, index_ra, index_dec, counter

        # Create grid_table for output
        grid_table = []
        # vIF, vPOL: dummy (not necessary)
        vIF = self.spw
        vPOL = 0

        for y in range(ngrid_dec):
            DEC = min_dec + self.grid_dec * (y + 0.5)
            for x in range(ngrid_ra):
                RA = min_ra + grid_ra_corr * (x + 0.5)
                for p in range(self.nplane):
                    line = [vIF, vPOL, x, y, RA, DEC, []]
                    for index in combine_list[p][x][y]:
                        # math.sqrt is twice as fast as ** 0.5 according to 
                        # the measurement on alma-cluster-proto03 in NAOJ 
                        # (3.5GHz CPU 8 Cores, 16GB RAM). 
                        # Furthermore, direct import of sqrt from math is 
                        # slightly (~10%) faster than calling sqrt using 
                        # 'math.sqrt'.
                        # Also, x * x is ~30% faster than x ** 2.0.
                        #Delta = (((ras[index] - RA) * dec_corr) ** 2.0 + \
                        #         (decs[index] - DEC) ** 2.0) ** 0.5
                        Delta = sqrt(subsqr(ras[index], RA) * dec_corr * dec_corr 
                                     + subsqr(decs[index], DEC))
                        line[6].append([rows[index], Delta, stats[index], index_list[index], ants[index]])
                    grid_table.append(line)
                    #LOG.info("grid_table: %s" % line)
        del ras, decs, combine_list

        LOG.info('ngrid_ra = %s  ngrid_dec = %s' % (ngrid_ra, ngrid_dec))
        return grid_table


    def grid(self, grid_table, LogLevel='info'):
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
        NROW = len(grid_table)
        LOG.info('grid() start...')
        LOG.info('Processing %d spectra...' % (NROW))

        # any opened table object is needed, so we use the one 
        # that datatable holds.
        tb = self.datatable.tb1
        tx = tb.taql('SELECT NELEMENTS(FLAGTRA) AS NCHAN FROM "%s" WHERE IFNO==%s LIMIT 1'%(self.data_in.values()[0], self.spw))
        NCHAN = tx.getcell('NCHAN',0)
        tx.close()
        del tx
        
        # create storage for output
        StorageOut = numpy.zeros((NROW, NCHAN), dtype=numpy.float32)
        StorageWeight = numpy.zeros((NROW), dtype=numpy.float32)
        StorageNumSp = numpy.zeros((NROW), dtype=numpy.int)
        StorageNumFlag = numpy.zeros((NROW), dtype=numpy.int)
        StorageNoData = numpy.ones((NCHAN), dtype=numpy.float32) * NoData
        OutputTable = []

        tROW = self.datatable.getcol('ROW')
        tTSYS = self.datatable.getcol('TSYS')
        tEXPT = self.datatable.getcol('EXPOSURE')
        tSFLAG = self.datatable.getcol('FLAG_SUMMARY')
            
        # loop for all ROWs in grid_table to make dictionary that 
        # associates spectra in data_in and weights with grids.
        bind_to_grid = dict([(k,[]) for k in self.data_in.keys()])
        for ROW in xrange(NROW):
            [IF, POL, X, Y, RAcent, DECcent, RowDelta] = grid_table[ROW]
            for [row, delta, rms, index, ant] in RowDelta:
                if tSFLAG[index] == 1:
                    if tTSYS[index] > 0.5 and tEXPT[index] > 0.0:
                        Weight = tEXPT[index]/(tTSYS[index]**2.0)
                    else: Weight = 1.0
                    bind_to_grid[ant].append([tROW[index], ROW, Weight, tSFLAG[index]])
        # Create progress timer
        Timer = common.ProgressTimer(80, sum(map(len,bind_to_grid.values())), LogLevel)

        # loop for antennas
        for AntID in self.data_in.keys():
            with casatools.TableReader(self.data_in[AntID]) as tb:
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
        for ROW in range(NROW):
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
