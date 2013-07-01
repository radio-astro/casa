'''
Created on 2013/06/30

@author: kana
'''
from __future__ import absolute_import

import os
#import math
import numpy
import time

import pipeline.infrastructure as infrastructure
#import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.casatools as casatools
#import pipeline.infrastructure.jobrequest as jobrequest

LOG = infrastructure.get_logger(__name__)


class SDFlagDataWorker(object):
    '''
    classdocs
    '''


    def __init__(self, context):
        '''
        Constructor
        '''
        self.context = context
    
    def execute(self, datatable, iteration, spwid, nchan, pollist, file_index, flagRule):
        
        start_time = time.time()

        # TODO: make sure baseline subtraction is already done
        # filename for before/after baseline
        filenames_work = [self.context.observing_run[idx].work_data
                          for idx in file_index]

        ThreNewRMS = flagRule['RmsPostFitFlag']['Threshold']
        ThreOldRMS = flagRule['RmsPreFitFlag']['Threshold']
        ThreNewDiff = flagRule['RunMeanPostFitFlag']['Threshold']
        ThreOldDiff = flagRule['RunMeanPreFitFlag']['Threshold']
        ThreTsys = flagRule['TsysFlag']['Threshold']
        Threshold = [ThreNewRMS, ThreOldRMS, ThreNewDiff, ThreOldDiff, ThreTsys]
        #ThreExpectedRMSPreFit = flagRule['RmsExpectedPreFitFlag']['Threshold']
        #ThreExpectedRMSPostFit = flagRule['RmsExpectedPostFitFlag']['Threshold']
        # WARN: ignoring the value set as flagRule['RunMeanPostFitFlag']['Nmean']
        nmean = flagRule['RunMeanPreFitFlag']['Nmean']

        ifnos = numpy.array(datatable.getcol('IF'))
        polnos = numpy.array(datatable.getcol('POL'))
        antennas = numpy.array(datatable.getcol('ANTENNA'))
        index_list = numpy.where(ifnos == spwid)[0]
        LOG.debug('index_list=%s'%(list(index_list)))
        LOG.debug('len(index_list)=%s'%(len(index_list)))
    
        # loop over file
        for idx in file_index:
            st = self.context.observing_run[idx]
            filename_in = st.name
            filename_out = st.baselined_name
            if not os.path.exists(filename_out):
                with casatools.TableReader(filename_in) as tb:
                    copied = tb.copy(filename_out, deep=True, valuecopy=True, returnobject=True)
                    copied.close()
            ant_indices = numpy.where(antennas.take(index_list)==idx)[0]
            ant_indices = index_list.take(ant_indices)
            for pol in pollist:
                time_table = datatable.get_timetable(idx, spwid, pol)
                # Select time gap list: 'subscan': large gap; 'raster': small gap
                if flagRule['Flagging']['ApplicableDuration'] == "subscan":
                    TimeTable = time_table[1]
                else:
                    TimeTable = time_table[0]
                LOG.info('Applied time bin for the running mean calculation: %s' % flagRule['Flagging']['ApplicableDuration'])

                pol_indices = numpy.where(polnos.take(ant_indices)==pol)[0]
                pol_indices = ant_indices.take(pol_indices)
                LOG.debug('pol_indices=%s'%(list(pol_indices)))
                
                # Calculate RMS and Diff from running mean
                t0 = time.time()
                data = self.calcStatistics(datatable, filename_in, filename_out, rows, nchan, nmean, TimeTable, edge)
                t1 = time.time()
                LOG.info('RMS and diff caluculation End: Elapse time = %.1f sec' % (t1 - t0))
                tmpdata = numpy.transpose(data)
                flag = self.get_flag_from_stats(tmpdata[1:6], Threshold, iteration)

        end_time = time.time()
        LOG.info('PROFILE execute: elapsed time is %s sec'%(end_time-start_time))

    def calcStatistics(self, DataTable, DataIn, DataOut, rows, NCHAN, Nmean, TimeTable, edge):

        # Calculate RMS and Diff from running mean
        NROW = len(rows)
        (edgeL, edgeR) = parseEdge(edge)

        LOG.info('Calculate RMS and Diff from running mean for Pre/Post fit...')
        LOG.info('Processing %d spectra...' % NROW)
        LOG.info('Nchan for running mean=%s' % Nmean)
        data = []

        ProcStartTime = time.time()

        LOG.info('RMS and diff caluculation Start')

        tbIn,tbOut = gentools(['tb','tb'])
        tbIn.open(DataIn)
        tbOut.open(DataOut)

        # Create progress timer
        Timer = ProgressTimer(80, NROW, LogLevel)
        for chunks in TimeTable:
            # chunks[0]: row, chunks[1]: index
            chunk = chunks[0]
            LOG.debug('Before Fit: Processing spectra = %s' % chunk)
            LOG.debug('chunks[0]= %s' % chunks[0])
            nrow = len(chunks[0])
            START = 0
            ### 2011/05/26 shrink the size of data on memory
            SpIn = numpy.zeros((nrow, NCHAN), dtype=numpy.float32)
            SpOut = numpy.zeros((nrow, NCHAN), dtype=numpy.float32)
            for index in range(len(chunks[0])):
                SpIn[index] = tbIn.getcell('SPECTRA', chunks[0][index])
                SpOut[index] = tbOut.getcell('SPECTRA', chunks[0][index])
            SpIn[index][:edgeL] = 0
            SpOut[index][:edgeL] = 0
            if edgeR > 0:
                SpIn[index][-edgeR:] = 0
                SpOut[index][-edgeR:] = 0
            ### loading of the data for one chunk is done

            for index in range(len(chunks[0])):
                row = chunks[0][index]
                idx = chunks[1][index]
                # Countup progress timer
                Timer.count()
                START += 1
                mask = numpy.ones(NCHAN, numpy.int)
#                if self.USE_CASA_TABLE:    
#                    for [m0, m1] in DataTable.getcell('MASKLIST',idx): mask[m0:m1] = 0
#                else:
#                    for [m0, m1] in DataTable[idx][DT_MASKLIST]: mask[m0:m1] = 0
                for [m0, m1] in DataTable.getcell('MASKLIST',idx): mask[m0:m1] = 0
                mask[:edgeL] = 0
                if edgeR > 0: mask[-edgeR:] = 0
                Nmask = int(NCHAN - numpy.sum(mask * 1.0))

                # Calculate RMS
                ### 2011/05/26 shrink the size of data on memory
                MaskedData = SpIn[index] * mask
                StddevMasked = MaskedData.std()
                MeanMasked = MaskedData.mean()
                OldRMS = math.sqrt(abs(NCHAN * StddevMasked ** 2 / (NCHAN - Nmask) - \
                                NCHAN * Nmask * MeanMasked ** 2 / ((NCHAN - Nmask) ** 2)))
#                if self.USE_CASA_TABLE:
#                    stats = DataTable.getcell('STATISTICS',idx)
#                else:
#                    stats = DataTable[idx][DT_STAT]
                stats = DataTable.getcell('STATISTICS',idx)
                stats[2] = OldRMS

                MaskedData = SpOut[index] * mask
                StddevMasked = MaskedData.std()
                MeanMasked = MaskedData.mean()
                NewRMS = math.sqrt(abs(NCHAN * StddevMasked ** 2 / (NCHAN - Nmask) - \
                                NCHAN * Nmask * MeanMasked ** 2 / ((NCHAN - Nmask) ** 2)))
                stats[1] = NewRMS

                # Calculate Diff from the running mean
                ### 2011/05/26 shrink the size of data on memory
                ### modified to calculate Old and New statistics in a single cycle
                if nrow == 1:
                    OldRMSdiff = 0.0
                    stats[4] = OldRMSdiff
                    NewRMSdiff = 0.0
                    stats[3] = NewRMSdiff
                else:
                    if START == 1:
                        Rmask = numpy.zeros(NCHAN, numpy.int)
                        RdataOld0 = numpy.zeros(NCHAN, numpy.float64)
                        RdataNew0 = numpy.zeros(NCHAN, numpy.float64)
                        NR = 0
                        for x in range(1, min(Nmean + 1, nrow)):
                            NR += 1
                            RdataOld0 += SpIn[x]
                            RdataNew0 += SpOut[x]
                            mask0 = numpy.ones(NCHAN, numpy.int)
#                            if self.USE_CASA_TABLE:
#                                for [m0, m1] in DataTable.getcell('MASKLIST',chunks[0][x]): mask0[m0:m1] = 0
#                            else:
#                                for [m0, m1] in DataTable[chunks[0][x]][DT_MASKLIST]: mask0[m0:m1] = 0
                            for [m0, m1] in DataTable.getcell('MASKLIST',chunks[0][x]): mask0[m0:m1] = 0
                            Rmask += mask0
                    elif START > (nrow - Nmean):
                        NR -= 1
                        RdataOld0 -= SpIn[index]
                        RdataNew0 -= SpOut[index]
                        Rmask -= mask
                    else:
                        RdataOld0 -= (SpIn[index] - SpIn[START + Nmean - 1])
                        RdataNew0 -= (SpOut[index] - SpOut[START + Nmean - 1])
                        mask0 = numpy.ones(NCHAN, numpy.int)
#                        if self.USE_CASA_TABLE:
#                            for [m0, m1] in DataTable.getcell('MASKLIST',chunks[0][START + Nmean - 1]): mask0[m0:m1] = 0
#                        else:
#                            for [m0, m1] in DataTable[chunks[0][START + Nmean - 1]][DT_MASKLIST]: mask0[m0:m1] = 0
                        for [m0, m1] in DataTable.getcell('MASKLIST',chunks[0][START + Nmean - 1]): mask0[m0:m1] = 0
                        Rmask += (mask0 - mask)
                    if START == 1:
                        Lmask = numpy.zeros(NCHAN, numpy.int)
                        LdataOld0 = numpy.zeros(NCHAN, numpy.float64)
                        LdataNew0 = numpy.zeros(NCHAN, numpy.float64)
                        NL = 0
                    elif START <= (Nmean + 1):
                        NL += 1
                        LdataOld0 += SpIn[START - 2]
                        LdataNew0 += SpOut[START - 2]
                        mask0 = numpy.ones(NCHAN, numpy.int)
#                        if self.USE_CASA_TABLE:
#                            for [m0, m1] in DataTable.getcell('MASKLIST',chunks[0][START - 2]): mask0[m0:m1] = 0
#                        else:
#                            for [m0, m1] in DataTable[chunks[0][START - 2]][DT_MASKLIST]: mask0[m0:m1] = 0
                        for [m0, m1] in DataTable.getcell('MASKLIST',chunks[0][START - 2]): mask0[m0:m1] = 0
                        Lmask += mask0
                    else:
                        LdataOld0 += (SpIn[START - 2] - SpIn[START - 2 - Nmean])
                        LdataNew0 += (SpOut[START - 2] - SpOut[START - 2 - Nmean])
                        mask0 = numpy.ones(NCHAN, numpy.int)
#                        if self.USE_CASA_TABLE:
#                            for [m0, m1] in DataTable.getcell('MASKLIST',chunks[0][START - 2]): mask0[m0:m1] = 0
#                        else:
#                            for [m0, m1] in DataTable[chunks[0][START - 2]][DT_MASKLIST]: mask0[m0:m1] = 0
                        for [m0, m1] in DataTable.getcell('MASKLIST',chunks[0][START - 2]): mask0[m0:m1] = 0
                        Lmask += mask0
                        mask0 = numpy.ones(NCHAN, numpy.int)
#                        if self.USE_CASA_TABLE:
#                            for [m0, m1] in DataTable.getcell('MASKLIST',chunks[0][START - 2 - Nmean]): mask0[m0:m1] = 0
#                        else:
#                            for [m0, m1] in DataTable[chunks[0][START - 2 - Nmean]][DT_MASKLIST]: mask0[m0:m1] = 0
                        for [m0, m1] in DataTable.getcell('MASKLIST',chunks[0][START - 2 - Nmean]): mask0[m0:m1] = 0
                        Lmask -= mask0

                    diffOld0 = (LdataOld0 + RdataOld0) / float(NL + NR) - SpIn[index]
                    diffNew0 = (LdataNew0 + RdataNew0) / float(NL + NR) - SpOut[index]
                    mask0 = (Rmask + Lmask + mask) / (NL + NR + 1)

                    # Calculate RMS
                    MaskedDataOld = diffOld0 * mask0
                    StddevMasked = MaskedDataOld.std()
                    MeanMasked = MaskedDataOld.mean()
                    OldRMSdiff = math.sqrt(abs((NCHAN * StddevMasked ** 2 - Nmask * MeanMasked ** 2 )/ (NCHAN - Nmask)))
                    stats[4] = OldRMSdiff
                    MaskedDataNew = diffNew0 * mask0
                    StddevMasked = MaskedDataNew.std()
                    MeanMasked = MaskedData.mean()
                    NewRMSdiff = math.sqrt(abs((NCHAN * StddevMasked ** 2 - Nmask * MeanMasked ** 2 )/ (NCHAN - Nmask)))
                    stats[3] = NewRMSdiff
#                if self.USE_CASA_TABLE:
#                    DataTable.putcell('STATISTICS',idx,stats)
#                    DataTable.putcell('NMASK',idx,Nmask)
#                else:
#                    DataTable[idx][DT_STAT] = stats
#                    DataTable[idx][DT_NMASK] = Nmask
                DataTable.putcell('STATISTICS',idx,stats)
                DataTable.putcell('NMASK',idx,Nmask)
                LOG.info('Row=%d, pre-fit RMS= %.2f pre-fit diff RMS= %.2f' % (row, OldRMS, OldRMSdiff))
                LOG.info('Row=%d, post-fit RMS= %.2f post-fit diff RMS= %.2f' % (row, NewRMS, NewRMSdiff))
#                if self.USE_CASA_TABLE:
#                    data.append([row, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, DataTable.getcell('TSYS',idx), Nmask])
#                else:
#                    data.append([row, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, DataTable[idx][DT_TSYS], Nmask])
                data.append([row, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, DataTable.getcell('TSYS',idx), Nmask])
            del SpIn, SpOut
        return data

    def _get_flag_from_stats(self, stat, Threshold, iteration):
        Ndata = len(stat[0])
        Nflag = len(stat)
        mask = numpy.ones(Nflag, Ndata, numpy.int)
        for cycle in range(iteration + 1):
            threshold = []
            for x in range(Nflag):
                Unflag = int(numpy.sum(mask[x] * 1.0))
                FlaggedData = stat[x] * mask[x]
                StddevFlagged = FlaggedData.std()
                if StddevFlagged == 0: StddevFlagged = stat[x][0] / 100.0
                MeanFlagged = FlaggedData.mean()
                AVE = MeanFlagged / float(Unflag) * float(Ndata)
                RMS = math.sqrt(Ndata * StddevFlagged ** 2 / Unflag - \
                                Ndata * (Ndata - Unflag) * MeanFlagged ** 2 / (Unflag ** 2))
                ThreP = AVE + RMS * Threshold[x]
                if x == 4:
                    # Tsys case
                    ThreM = 0.0
                else: ThreM = -1.0
                threshold.append([ThreM, ThreP])
                for y in range(Ndata):
                    if ThreM < stat[x][y] <= ThreP: mask[x][y] = 1
                    else: mask[x][y] = 0
        return mask
    