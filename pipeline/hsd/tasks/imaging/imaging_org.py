from __future__ import absolute_import

import os
import math
#from math import cos, sqrt, exp
import numpy
import time

import pipeline.infrastructure.api as api
import pipeline.heuristics as heuristics
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.jobrequest as jobrequest
import pipeline.infrastructure.logging as logging
from . import common
from . import utils
#from . import constants

LOG = logging.get_logger(__name__)
#logging.set_logging_level('trace')
logging.set_logging_level('info')

subsqr = lambda x, y: (x - y) * (x - y)
#NoData = constants.NoData
NoData = -32767.0

class SDImagingInputs(common.SingleDishInputs):
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

class SDImagingResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDImagingResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDImagingResults, self).merge_with_context(context)
        LOG.todo('need to decide what is done in SDImagingResults.merge_with_context')

    def _outcome_name(self):
        return self.outcome.__str__()

class SDImaging(common.SingleDishTaskTemplate):
    Inputs = SDImagingInputs
    SRCTYPE = {'ps': 0,
               'otf': 0,
               'otfraster': 0}

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
            spw = group_desc['member'][0][1]
            LOG.debug('spw=%s'%(spw))

            # skip spw that is not included in iflist
            if iflist is not None and spw not in iflist:
                LOG.debug('Skip spw %s'%(spw))
                continue

            # assume all members have same calmode
            ant = group_desc['member'][0][0]
            st = context.observing_run[ant]
            calmode = st.calibration_strategy['calmode']
            srctype = self.SRCTYPE[calmode] if self.SRCTYPE.has_key(calmode) else None
            
            # beam size
            grid_size = qa.convert(st.beam_size[spw], 'deg')['value']

            worker = SimpleGridding(datatable, spw, srctype, grid_size, file_index)
            
            # create job
            job = jobrequest.JobRequest(worker.execute) 
            
            # execute job
            results = self._executor.execute(job)

            grid_tables.append(results)

        #return grid_tables
        result = SDImagingResults(task = self.__class__,
                                     success = True, 
                                     outcome = grid_tables)
        result.task = self.__class__
        return result

    def analyse(self, result):
        return result

class Gridding(object):
    rule = {'WeightDistance': 'Gauss', \
                'Clipping': 'MinMaxReject', \
                'WeightRMS': True, \
                'WeightTsysExptime': False} 

    def __init__(self, datatable, antenna, spw, pol, srctype, nchan, grid_size):
        self.datatable = datatable
        self.datatable_name = self.datatable.plaintable
        if type(antenna) == int:
            self.antenna = [antenna]
        else:
            self.antenna = antenna
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
        combine_radius = self.grid_ra
        allowance = self.grid_ra * 0.1
        spacing = self.grid_ra / 3.0
        index_list, grid_table = self.GroupForGrid(combine_radius, allowance, spacing, 'RASTER')
        #return index_list, grid_table
        work_dir = os.path.join('/',*(self.datatable.plaintable.split('/')[:-2]))
        filenames = [os.path.join(work_dir,f) for f in self.datatable.getkeyword('FILENAMES')]
        #DataIn = filenames[self.antenna]
        #DataIn = [filenames[i] for i in self.antenna]
        ### FOR TESTING ###
        DataIn = ['uid___A002_X316307_X6f.CM02.asap_work/',
                  'uid___A002_X3178c7_X31.CM02.asap_work/']
        ###################
        LOG.info('DataIn=%s'%(DataIn))
        LOG.info('index_list.shape=%s'%(list(index_list.shape)))
        spstorage, grid_table2 = self.Process8(DataIn, index_list, self.nchan, grid_table, 0.5 * combine_radius, self.rule)
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
        #taqlstring = 'USING STYLE PYTHON SELECT ROWNUMBER() AS ID, RO.ROW, RO.ANTENNA, RO.RA, RO.DEC, RW.STATISTICS[0] AS STAT FROM "%s" RO, "%s" RW WHERE IF==%s && POL == %s && ANTENNA IN %s'%(os.path.join(self.datatable_name,'RO'),os.path.join(self.datatable_name,'RW'),self.spw,self.pol,list(self.antenna))
        #if self.srctype is not None:
        #    taqlstring += ' && SRCTYPE==%s'%(self.srctype)

        ### FOR TESTING ###
        taqlstring = 'USING STYLE PYTHON SELECT ROWNUMBER() AS ID, RO.ROW, RO.ANTENNA, RO.RA, RO.DEC, RW.STATISTICS[0] AS STAT FROM "%s" RO, "%s" RW WHERE IF==%s && POL == %s && ANTENNA IN %s'%(os.path.join('ThisIsExportedDataTable.tbl/RO'),os.path.join('ThisIsExportedDataTable.tbl/RW'),self.spw,self.pol,list(self.antenna))        
        ###################
            
        LOG.debug('taqlstring="%s"'%(taqlstring))
        #NpRAs = numpy.take(self.datatable.getcol('RA'),Idx)
        #NpDECs = numpy.take(self.datatable.getcol('DEC'),Idx)
        #NpRMSs = numpy.take(self.datatable.getcol('STATISTICS'),Idx,axis=1)[1,:]
        #NpROWs = numpy.take(self.datatable.getcol('ROW'),Idx)
        #NpIDXs = numpy.array(index_list)
        #NpANTs = numpy.take(self.datatable.getcol('ANTENNA'),index_list)
        tx = table.taql(taqlstring)
        index_list = tx.getcol('ID')
        rows = tx.getcol('ROW')
        ants = tx.getcol('ANTENNA')
        ras = tx.getcol('RA')
        decs = tx.getcol('DEC')
        stats = tx.getcol('STAT')
        tx.close()
        del tx


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
            index_listTmp = index_list[0][:]
            for i in range(len(index_list)-1):
                index_listTmp += index_list[i+1]
            index_list = index_listTmp

        

        if ObsPattern.upper() == 'RASTER':
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
            #NGridRA = int((max(RAs) - MinRA) / (GridSpacing * DecCorrection)) + 1
            #NGridDEC = int((max(DECs) - MinDEC) / GridSpacing) + 1
            # (RAcenter, DECcenter) to be the center of the grid
            NGridRA = int((MaxRA - MinRA) / (GridSpacing * DecCorrection)) + 1
            NGridDEC = int((MaxDEC - MinDEC) / GridSpacing) + 1
            MinRA = (MinRA + MaxRA) / 2.0 - (NGridRA - 1) / 2.0 * GridSpacing * DecCorrection
            MinDEC = (MinDEC + MaxDEC) / 2.0 - (NGridDEC - 1) / 2.0 * GridSpacing

            for y in range(NGridDEC):
                if NROW > 10000: print 'Progress:', y, '/', NGridDEC, '(', NGridRA, ')', ' : ', time.ctime()
                DEC = MinDEC + GridSpacing * y
                DeltaDEC = decs - DEC
                SelectD = numpy.nonzero(numpy.less_equal(DeltaDEC, CombineRadius) * numpy.greater_equal(DeltaDEC, -CombineRadius))[0]
                sDeltaDEC = numpy.take(DeltaDEC, SelectD)
                sRA = numpy.take(ras, SelectD)
                sROW = numpy.take(rows, SelectD)
                sIDX = numpy.take(index_list, SelectD)
                sRMS = numpy.take(stats, SelectD)
                sANT = numpy.take(ants, SelectD)
                LOG.debug('Combine Spectra: %s' % len(sRMS))
                for x in range(NGridRA):
                    RA = MinRA + GridSpacing * DecCorrection * x
                    sDeltaRA = (sRA - RA) / DecCorrection
                    Delta = sDeltaDEC * sDeltaDEC + sDeltaRA * sDeltaRA
                    #Select = numpy.less_equal(Delta, ThresholdR)
                    SelectR = numpy.nonzero(numpy.less_equal(Delta, ThresholdR))[0]
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
                    LOG.debug("GridTable: %s" % line)
            del DEC, DeltaDEC, SelectD, sDeltaDEC, sRA, sROW, sIDX, sRMS, sANT, sDeltaRA, RA, Delta, line

        elif ObsPattern.upper() == 'SINGLE-POINT':
            NGridRA = 1
            NGridDEC = 1
            CenterRA = ras.mean()
            CenterDEC = decs.mean()
            #CenterRA = (numpy.array(RAs)).mean()
            #CenterDEC = (numpy.array(DECs)).mean()
            line = [self.spw, self.pol, 0, 0, CenterRA, CenterDEC, []]
            for x in range(len(index_list)):
                Delta = math.sqrt((ras[x] - CenterRA) ** 2.0 + (decs[x] - CenterDEC) ** 2.0)
                #Delta = math.math.sqrt((RAs[x] - CenterRA) ** 2.0 + (DECs[x] - CenterDEC) ** 2.0)
                if Delta <= Allowance:
                    line[6].append([index_list[x], Delta, stats[x], index_list[x], ants[x]])
                    #line[6].append([rows[x], Delta, RMSs[x], IDXs[x]])
            GridTable.append(line)
            LOG.debug("GridTable: %s" % line)

        elif ObsPattern.upper() == 'MULTI-POINT':
            NGridRA = 0
            NGridDEC = 1
            Flag = numpy.ones(len(index_list))
            while Flag.sum() > 0:
                for x in range(len(index_list)):
                    if Flag[x] == 1:
                        RA = ras[x]
                        DEC = decs[x]
                        #RA = RAs[x]
                        #DEC = DECs[x]
                        RAtmp = [RA]
                        DECtmp = [DEC]
                        for y in range(x + 1, len(index_list)):
                            if Flag[y] == 1:
                                Delta = math.sqrt((RA - ras[y]) ** 2.0 + (DEC - decs[y]) ** 2.0)
                                #Delta = math.math.sqrt((RA - RAs[y]) ** 2.0 + (DEC - DECs[y]) ** 2.0)
                                if Delta <= Allowance:
                                    RAtmp.append(ras[y])
                                    DECtmp.append(decs[y])
                                    #RAtmp.append(RAs[y])
                                    #DECtmp.append(DECs[y])
                        CenterRA = (numpy.array(RAtmp)).mean()
                        CenterDEC = (numpy.array(DECtmp)).mean()
                        line = [self.spw, self.pol, 0, NGridRA, CenterRA, CenterDEC, []]
                        NGridRA += 1
                        for x in range(len(index_list)):
                            if Flag[x] == 1:
                                Delta = math.sqrt((ras[x] - CenterRA) ** 2.0 + (decs[x] - CenterDEC) ** 2.0)
                                #Delta = math.math.sqrt((RAs[x] - CenterRA) ** 2.0 + (DECs[x] - CenterDEC) ** 2.0)
                                if Delta <= Allowance:
                                    line[6].append([index_list[x], Delta, stats[x], index_list[x], ants[x]])
                                    #line[6].append([rows[x], Delta, RMSs[x], IDXs[x]])
                                    Flag[x] = 0
                        GridTable.append(line)
                        LOG.debug("GridTable: %s" % line)
            del Flag
        else:
            LOG.error('Error: ObsPattern not defined...')
            return False


        LOG.info('NGridRA = %s  NGridDEC = %s' % (NGridRA, NGridDEC))

        index_list = index_list.reshape((len(self.antenna),len(index_list)/len(self.antenna)))

        end = time.time()
        LOG.info('execute: elapsed time %s sec'%(end-start))
        return index_list, GridTable

    def Process8(self, DataIn, IDX, NCHAN, GridTable, CombineRadius, GridRule, LogLevel=2):
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
        # 2011/11/12 DataIn and rowsSel are [list]
        NSpIn = len(IDX[0])
        for i in range(len(IDX)-1): NSpIn += len(IDX[i+1])
        SpStorage = numpy.zeros((NSpIn, NCHAN), dtype=numpy.float32)
        #SpStorage = numpy.zeros((len(IDX), NCHAN), dtype=numpy.float32)

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

        #tROW = self.datatable.getcol('ROW')
        #tSFLAG = self.datatable.getcol('FLAG_SUMMARY')
        #tTSYS = self.datatable.getcol('TSYS')
        #tEXPT = self.datatable.getcol('EXPOSURE')
        ### FOR TESTING ###
        with casatools.TableReader('ThisIsExportedDataTable.tbl/RO') as tb:
            tROW = tb.getcol('ROW')
            tTSYS = tb.getcol('TSYS')
            tEXPT = tb.getcol('EXPOSURE')
        with casatools.TableReader('ThisIsExportedDataTable.tbl/RW') as tb:
            tSFLAG = tb.getcol('FLAG_SUMMARY')
        ###################

        # 2011/11/12 DataIn and rowsSel are [list]
        IDX2StorageID = {}
        StorageID = 0
        for i in range(len(DataIn)):
            # read data to SpStorage
            with casatools.TableReader(DataIn[i]) as tb:
                #self._tb.open(DataIn[i])
                for x in range(len(IDX[i])):
                    #SpStorage[x] = self._tb.getcell('SPECTRA', self.datatable[IDX[x]][DT_ROW])
                    SpStorage[StorageID] = tb.getcell('SPECTRA', tROW[IDX[i][x]])
                    IDX2StorageID[IDX[i][x]] = StorageID
                    #IDX2StorageID[IDX[x]] = x
                    StorageID += 1
                LOG.debug('Data Stored in SpStorage')
                #self._tb.close()

        # Create progress timer
        Timer = utils.ProgressTimer(80, NROW, LogLevel)
        for [IF, POL, X, Y, RAcent, DECcent, RowDelta] in GridTable:
            rowlist = []
            indexlist = []
            deltalist = []
            rmslist = []
            flagged = 0
            for [row, delta, rms, index, ant] in RowDelta:
                # Check Summary Flag
                if tSFLAG[int(index)] == 1:
                    #if self.datatable[row][DT_SFLAG] == 1:
                    rowlist.append(int(row))
                    indexlist.append(IDX2StorageID[int(index)])
                    deltalist.append(delta)
                    rmslist.append(rms)
                else: flagged += 1
            if len(rowlist) == 0:
                # No valid Spectra at the position
                RMS = 0.0
                pass
            elif len(rowlist) == 1:
                # One valid Spectrum at the position
                #StorageOut[ID] = SpStorage[rowlist[0]]
                StorageOut[ID] = SpStorage[indexlist[0]]
                RMS = rmslist[0]
            else:
                # More than one valid Spectra at the position
                #data = SpStorage[rowlist].copy()
                data = SpStorage[indexlist].copy()
                #print data
                #w = numpy.ones(numpy.shape(data), numpy.float64)
                #weightlist = numpy.ones(len(rowlist), numpy.float64)
                w = numpy.ones(numpy.shape(data), numpy.float32)
                weightlist = numpy.ones(len(rowlist), numpy.float32)
                # Clipping
                if clip.upper() == 'MINMAXREJECT' and len(rowlist) > 2:
                    w[numpy.argmin(data, axis=0), range(len(data[0]))] = 0.0
                    w[numpy.argmax(data, axis=0), range(len(data[0]))] = 0.0
                # Weight by RMS
                # Weight = 1/(RMS**2)
                if rms_weight == True:
                    for m in range(len(rowlist)):
                        if rmslist[m] != 0.0:
                            w[m] /= (rmslist[m]**2)
                            weightlist[m] /= (rmslist[m]**2)
                        else:
                            w[m] *= 0.0
                            weightlist[m] *= 0.0
                # Weight by Exptime & Tsys
                # RMS = n * Tsys/math.sqrt(Exptime)
                # Weight = 1/(RMS**2) = (Exptime/(Tsys**2))
                if tsys_weight == True:
                    for m in range(len(rowlist)):
                        # 2008/9/21 Bug fix
                        if tTSYS[rowlist[m]] > 0.5:
                            w[m] *= (tEXPT[rowlist[m]]/(tTSYS[rowlist[m]]**2))
                            weightlist[m] *= (tEXPT[rowlist[m]]/(tTSYS[rowlist[m]]**2))
                            #if self.datatable[m][DT_TSYS] > 0.5:
                            #    w[m] *= (self.datatable[m][DT_EXPT]/(self.datatable[m][DT_TSYS]**2))
                            #    weightlist[m] *= (self.datatable[m][DT_EXPT]/(self.datatable[m][DT_TSYS]**2))
                        else:
                            w[m] *= 0.0
                            weightlist[m] = 0.0
                # Weight by radius
                if weight.upper() == 'GAUSS':
                    # weight = exp(-ln2*((r/hwhm)**2))
                    for m in range(len(rowlist)):
                        w[m] *= (math.exp(-0.69314718055994529*((deltalist[m]/CombineRadius)**2)))
                        weightlist[m] *= (math.exp(-0.69314718055994529*((deltalist[m]/CombineRadius)**2)))
                elif weight.upper() == 'LINEAR':
                    # weight = 0.5 + (hwhm - r)/2/hwhm = 1.0 - r/2/hwhm
                    for m in range(len(rowlist)):
                        w[m] *= (1.0 - deltalist[m]/2.0/CombineRadius)
                        weightlist[m] *= (1.0 - deltalist[m]/2.0/CombineRadius)
                # Combine Spectra
                if w.sum() != 0: StorageOut[ID] = (numpy.sum(data * w, axis=0) / numpy.sum(w,axis=0))
                # Calculate RMS of the spectrum
                r0 = 0.0
                r1 = 0.0
                for m in range(len(rowlist)):
                    r0 += (rmslist[m] * weightlist[m]) ** 2
                    r1 += weightlist[m]
                RMS = (r0**0.5) / r1
                del data, w, weightlist

            OutputTable.append([IF, POL, X, Y, RAcent, DECcent, len(rowlist), flagged, RMS])
            ID += 1
            del rowlist, indexlist, deltalist, rmslist

            # Countup progress timer
            Timer.count()

        del SpStorage, Timer, IDX2StorageID

        end = time.time()
        LOG.info('execute: elapsed time %s sec'%(end-start))
        return (StorageOut, OutputTable)
