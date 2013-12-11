from __future__ import absolute_import

import os
import math
import numpy
import time

import asap as sd
from taskinit import gentools

#from SDTool import ProgressTimer

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.casatools as casatools

from . import SDFlagPlotter as SDP
from .. import common

LOG = infrastructure.get_logger(__name__)


class SDFlagDataWorker(object):
    '''
    The worker class of single dish flagging task.
    This class defines per spwid flagging operation.
    '''


    def __init__(self, context, datatable, iteration, spwid, nchan, pollist, file_index, flagRule, userFlag=[], edge=(0,0)):
        '''
        Constructor of worker class
        '''
        self.context = context
        self.datatable = datatable
        self.iteration = iteration
        self.spwid = spwid
        self.nchan = nchan
        self.pollist = pollist
        self.file_index = file_index
        self.flagRule = flagRule
        self.userFlag = userFlag
        self.edge = edge
    
    def execute(self, dry_run=True):
        """
        Invoke single dish flagging based on statistics of spectra.
        Iterates over antenna and polarization for a certain spw ID
        """
        start_time = time.time()

        datatable = self.datatable
        iteration = self.iteration
        spwid = self.spwid
        nchan = self.nchan
        pollist = self.pollist
        file_index = self.file_index
        flagRule = self.flagRule
        userFlag = self.userFlag
        edge = self.edge


        # TODO: make sure baseline subtraction is already done
        # filename for before/after baseline
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
        # out table name
        namer = filenamer.BaselineSubtractedTable()
        namer.spectral_window(spwid)
        # output directory
        if self.context.subtask_counter is 0: 
            stage_number = self.context.task_counter - 1
        else:
            stage_number = self.context.task_counter 
        FigFileDir = (self.context.report_dir+"/stage%d" % stage_number)
        ### WORKAROUND to GENERATE stage# dir manually
        if not os.path.exists(FigFileDir):
            os.mkdir(FigFileDir)
        FigFileDir += "/"

        flagSummary = []
        # loop over file
        for idx in file_index:
            st = self.context.observing_run[idx]
            filename_in = st.name
            filename_out = st.baselined_name
            asdm = common.asdm_name(st)
            namer.asdm(asdm)
            namer.antenna_name(st.antenna.name)
            out_table_name = namer.get_filename()
            # Baseline is not yet done
            if not os.path.exists(filename_out):
                #with casatools.TableReader(filename_in) as tb:
                #    copied = tb.copy(filename_out, deep=True, valuecopy=True, returnobject=True)
                #    copied.close()
                raise Exception, "Flagging should be done after baseline-subtraction."
            
            LOG.info("*** Processing table: %s ***" % (os.path.basename(filename_in)))
            for pol in pollist:
                LOG.info("[ POL=%d ]" % (pol))
                # time_table should only list on scans
                time_table = datatable.get_timetable(idx, spwid, pol)               
                # Select time gap list: 'subscan': large gap; 'raster': small gap
                if flagRule['Flagging']['ApplicableDuration'] == "subscan":
                    TimeTable = time_table[1]
                else:
                    TimeTable = time_table[0]
                LOG.info('Applied time bin for the running mean calculation: %s' % flagRule['Flagging']['ApplicableDuration'])
                
                # Calculate RMS and Diff from running mean
                t0 = time.time()
                data = self.calcStatistics(datatable, filename_in, filename_out, nchan, nmean, TimeTable, edge)
                t1 = time.time()
                LOG.info('RMS and diff calculation End: Elapse time = %.1f sec' % (t1 - t0))
                
                t0 = time.time()
                tmpdata = numpy.transpose(data)
                dt_idx = numpy.array(tmpdata[0], numpy.int)
                LOG.info('Calculating the thresholds by RMS and Diff from running mean of Pre/Post fit. (Iterate %d times)' % (iteration))
                stat_flag, final_thres = self._get_flag_from_stats(tmpdata[1:6], Threshold, iteration)
                LOG.debug('final threshold shape = %d' % len(final_thres))
                LOG.info('Final thresholds: RMS (pre-/post-fit) = %.2f / %.2f , Diff RMS (pre-/post-fit) = %.2f / %.2f , Tsys=%.2f' % tuple([final_thres[i][1] for i in (1,0,3,2,4)]))
                
                self._apply_stat_flag(datatable, dt_idx, stat_flag)

                # flag by Expected RMS
                self.flagExpectedRMS(datatable, spwid, dt_idx, idx, FlagRule=flagRule, rawFileIdx=idx)
  
                # flag by scantable row ID defined by user
                self.flagUser(datatable, dt_idx, UserFlag=userFlag)
                # Check every flags to create summary flag
                self.flagSummary(datatable, dt_idx, flagRule) 
                t1 = time.time()
                LOG.info('Apply flags End: Elapse time = %.1f sec' % (t1 - t0))
                
                # generate summary plot
                #FigFileRoot = ("FlagStat_%s_spw%d_pol%d_it%d" % (st.antenna.name, spwid, pol, iteration))
                st_prefix = st.name.rstrip('/').split('/')[-1].rstrip('\.asap').replace('\.', '_')
                FigFileRoot = ("FlagStat_%s_spw%d_pol%d" % (st_prefix, spwid, pol))
                time_gap = datatable.get_timegap(idx, spwid, pol)
                # time_gap[0]: PosGap, time_gap[1]: TimeGap
                t0 = time.time()
                htmlName = self.plot_flag(datatable, dt_idx, time_gap[0], time_gap[1], final_thres, flagRule, FigFileDir, FigFileRoot)
                t1 = time.time()
                LOG.info('Plot flags End: Elapsed time = %.1f sec' % (t1 - t0) )
                self.save_outtable(datatable, dt_idx, out_table_name)
                flagSummary.append({'name': htmlName, 'index': idx, 'spw': spwid, 'pol': pol})

        end_time = time.time()
        LOG.info('PROFILE execute: elapsed time is %s sec'%(end_time-start_time))

        return flagSummary


    def calcStatistics(self, DataTable, DataIn, DataOut, NCHAN, Nmean, TimeTable, edge):

        # Calculate RMS and Diff from running mean
        NROW = len(numpy.array(TimeTable).flatten())/2
        # parse edge
        if len(edge) == 2:
            (edgeL, edgeR) = edge
        else:
            edgeL = edge[0]
            edgeR = edge[0]

        LOG.info('Calculate RMS and Diff from running mean for Pre/Post fit...')
        LOG.info('Processing %d spectra...' % NROW)
        LOG.info('Nchan for running mean=%s' % Nmean)
        data = []

        ProcStartTime = time.time()

        LOG.info('RMS and diff calculation Start')

        tbIn, tbOut = gentools(['tb','tb'])
        tbIn.open(DataIn)
        tbOut.open(DataOut)

        # Create progress timer
        #Timer = ProgressTimer(80, NROW, LogLevel)
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
                #Timer.count()
                START += 1
                mask = numpy.ones(NCHAN, numpy.int)
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
                        for [m0, m1] in DataTable.getcell('MASKLIST',chunks[0][START - 2]): mask0[m0:m1] = 0
                        Lmask += mask0
                    else:
                        LdataOld0 += (SpIn[START - 2] - SpIn[START - 2 - Nmean])
                        LdataNew0 += (SpOut[START - 2] - SpOut[START - 2 - Nmean])
                        mask0 = numpy.ones(NCHAN, numpy.int)
                        for [m0, m1] in DataTable.getcell('MASKLIST',chunks[0][START - 2]): mask0[m0:m1] = 0
                        Lmask += mask0
                        mask0 = numpy.ones(NCHAN, numpy.int)
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

                DataTable.putcell('STATISTICS',idx,stats)
                DataTable.putcell('NMASK',idx,Nmask)
                LOG.debug('Row=%d, pre-fit RMS= %.2f pre-fit diff RMS= %.2f' % (row, OldRMS, OldRMSdiff))
                LOG.debug('Row=%d, post-fit RMS= %.2f post-fit diff RMS= %.2f' % (row, NewRMS, NewRMSdiff))
                data.append([idx, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, DataTable.getcell('TSYS',idx), Nmask])
            del SpIn, SpOut
        return data

    def _get_flag_from_stats(self, stat, Threshold, iteration):
        Ndata = len(stat[0])
        Nflag = len(stat)
        mask = numpy.ones((Nflag, Ndata), numpy.int)
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
        return mask, threshold

    def _apply_stat_flag(self, DataTable, ids, stat_flag):
        LOG.info("Updating flags in data table")
        N = 0
        for ID in ids:
            flags = DataTable.getcell('FLAG', ID)
            pflags = DataTable.getcell('FLAG_PERMANENT', ID)
            flags[1] = stat_flag[0][N]
            flags[2] = stat_flag[1][N]
            flags[3] = stat_flag[2][N]
            flags[4] = stat_flag[3][N]
            pflags[1] = stat_flag[4][N]
            DataTable.putcell('FLAG', ID, flags)
            DataTable.putcell('FLAG_PERMANENT', ID, pflags)
            N += 1

    def flagExpectedRMS(self, DataTable, vIF, ids, vAnt, FlagRule=None, rawFileIdx=0):
        # FLagging based on expected RMS
        # TODO: Include in normal flags scheme

        # The expected RMS according to the radiometer formula sometimes needs
        # special scaling factors to account for meta data conventions (e.g.
        # whether Tsys is given for DSB or SSB mode) and for backend specific
        # setups (e.g. correlator, AOS, etc. noise scaling). These factors are
        # not saved in the data sets' meta data. Thus we have to read them from
        # a special file. TODO: This needs to be changed for ALMA later on.

        LOG.info("Flagging spectra by Expected RMS")
        try:
            fd = open('%s.exp_rms_factors' % (DataTable.getkeyword['FILENAMES'][rawFileIdx]), 'r')
            sc_fact_list = fd.readlines()
            fd.close()
            sc_fact_dict = {}
            for sc_fact in sc_fact_list:
                sc_fact_key, sc_fact_value = sc_fact.replace('\n','').split()
                sc_fact_dict[sc_fact_key] = float(sc_fact_value)
            tsys_fact = sc_fact_dict['tsys_fact']
            nebw_fact = sc_fact_dict['nebw_fact']
            integ_time_fact = sc_fact_dict['integ_time_fact']
            LOG.info("Using scaling factors tsys_fact=%f, nebw_fact=%f and integ_time_fact=%f for flagging based on expected RMS." % (tsys_fact, nebw_fact, integ_time_fact))
        except:
            LOG.warn("Cannot read scaling factors for flagging based on expected RMS. Using 1.0.")
            tsys_fact = 1.0
            nebw_fact = 1.0
            integ_time_fact = 1.0

        # TODO: Make threshold a parameter
        # This needs to be quite strict to catch the ripples in the bad Orion
        # data. Maybe this is due to underestimating the total integration time.
        # Check again later.
        # 2008/10/31 divided the category into two
        ThreExpectedRMSPreFit = FlagRule['RmsExpectedPreFitFlag']['Threshold']
        ThreExpectedRMSPostFit = FlagRule['RmsExpectedPostFitFlag']['Threshold']

        # The noise equivalent bandwidth is proportional to the channel width
        # but may need a scaling factor. This factor was read above.
        st_name = DataTable.getkeyword('FILENAMES')[vAnt]
        s = sd.scantable(st_name, average=False)
        s.set_selection(ifs=[vIF])
        s.set_unit('GHz')
        Abcissa = s.get_abcissa()[0]
        noiseEquivBW = abs(Abcissa[1]-Abcissa[0]) * 1e9 * nebw_fact

        tEXPT = DataTable.getcol('EXPOSURE')
        tTSYS = DataTable.getcol('TSYS')

        for ID in ids:
            row = DataTable.getcell('ROW',ID)
            # The HHT and APEX test data show the "on" time only in the CLASS
            # header. To get the total time, at least a factor of 2 is needed,
            # for OTFs and rasters with several on per off even higher, but this
            # cannot be automatically determined due to lacking meta data. We
            # thus use a manually supplied scaling factor.
            integTimeSec = tEXPT[ID] * integ_time_fact
            # The Tsys value can be saved for DSB or SSB mode. A scaling factor
            # may be needed. This factor was read above.
            currentTsys = tTSYS[ID] * tsys_fact
            if ((noiseEquivBW * integTimeSec) > 0.0):
                expectedRMS = currentTsys / math.sqrt(noiseEquivBW * integTimeSec)
                # 2008/10/31
                # Comparison with both pre- and post-BaselineFit RMS
                stats = DataTable.getcell('STATISTICS',ID)
                PostFitRMS = stats[1]
                PreFitRMS = stats[2]
                LOG.debug('DEBUG_DM: Row: %d Expected RMS: %f PostFit RMS: %f PreFit RMS: %f' % (row, expectedRMS, PostFitRMS, PreFitRMS))
                stats[5] = expectedRMS * ThreExpectedRMSPostFit
                stats[6] = expectedRMS * ThreExpectedRMSPreFit
                DataTable.putcell('STATISTICS',ID,stats)
                flags = DataTable.getcell('FLAG',ID)
                if (PostFitRMS > ThreExpectedRMSPostFit * expectedRMS):
                    flags[5] = 0
                else:
                    flags[5] = 1
                if (PreFitRMS > ThreExpectedRMSPreFit * expectedRMS):
                    flags[6] = 0
                else:
                    flags[6] = 1
                DataTable.putcell('FLAG',ID,flags)


    def flagUser(self, DataTable, ids, UserFlag=[]):
        # flag by scantable row ID.
        for ID in ids:
            row = DataTable.getcell('ROW', ID)
            # Update User Flag 2008/6/4
            try:
                Index = UserFlag.index(row)
                tPFLAG = DataTable.getcell('FLAG_PERMANENT', ID)
                tPFLAG[2] = 0
                DataTable.putcell('FLAG_PERMANENT', ID, tPFLAG)
            except ValueError:
                tPFLAG = DataTable.getcell('FLAG_PERMANENT', ID)
                tPFLAG[2] = 1
                DataTable.putcell('FLAG_PERMANENT', ID, tPFLAG)


    def flagSummary(self, DataTable, ids, FlagRule):
        for ID in ids:
            # Check every flags to create summary flag
            tFLAG = DataTable.getcell('FLAG', ID)
            tPFLAG = DataTable.getcell('FLAG_PERMANENT', ID)
            Flag = 1
            pflag = self._get_parmanent_flag_summary(tPFLAG, FlagRule)
            sflag = self._get_stat_flag_summary(tFLAG, FlagRule)
            Flag = pflag*sflag
            DataTable.putcell('FLAG_SUMMARY', ID, Flag)

    def _get_parmanent_flag_summary(self, pflag, FlagRule):
        # FLAG_PERMANENT[0] --- 'WeatherFlag'
        # FLAG_PERMANENT[1] --- 'TsysFlag'
        # FLAG_PERMANENT[2] --- 'UserFlag'
        types = ['WeatherFlag', 'TsysFlag', 'UserFlag']
        mask = 1
        for idx in range(len(types)):
            if FlagRule[types[idx]]['isActive'] and pflag[idx] == 0:
                mask = 0
                break
        return mask

    def _get_stat_flag_summary(self, tflag, FlagRule):
        # FLAG[0] --- 'LowFrRMSFlag' (OBSOLETE)
        # FLAG[1] --- 'RmsPostFitFlag'
        # FLAG[2] --- 'RmsPreFitFlag'
        # FLAG[3] --- 'RunMeanPostFitFlag'
        # FLAG[4] --- 'RunMeanPreFitFlag'
        # FLAG[5] --- 'RmsExpectedPostFitFlag'
        # FLAG[6] --- 'RmsExpectedPreFitFlag'
        types = ['RmsPostFitFlag', 'RmsPreFitFlag', 'RunMeanPostFitFlag', 'RunMeanPreFitFlag',
                 'RmsExpectedPostFitFlag', 'RmsExpectedPreFitFlag']
        mask = 1
        for idx in range(len(types)):
            if FlagRule[types[idx]]['isActive'] and tflag[idx+1] == 0:
                mask = 0
                break
        return mask

    
    def plot_flag(self, DataTable, ids, PosGap, TimeGap, threshold, FlagRule, FigFileDir, FigFileRoot):
        # Plot statistics
        NROW = len(ids)
        # Store data for plotting
        FlaggedRowsCategory = [[],[],[],[],[],[],[],[],[]]
        FlaggedRows = []
        PermanentFlag = []
        NPpdata = numpy.zeros((7,NROW), numpy.float)
        NPpflag = numpy.zeros((7,NROW), numpy.int)
        NPprows = numpy.zeros((2,NROW), numpy.int)
        N = 0
        for ID in ids:
            row = DataTable.getcell('ROW', ID)
            # Check every flags to create summary flag
            tFLAG = DataTable.getcell('FLAG',ID)
            tPFLAG = DataTable.getcell('FLAG_PERMANENT',ID)
            tTSYS = DataTable.getcell('TSYS',ID)
            tSTAT = DataTable.getcell('STATISTICS',ID)

            # permanent flag
            Flag = self._get_parmanent_flag_summary(tPFLAG, FlagRule)
            PermanentFlag.append(Flag)
            # FLAG_SUMMARY
            Flag *= self._get_stat_flag_summary(tFLAG, FlagRule)
            if Flag == 0:
                FlaggedRows.append( row )
           # Tsys flag
            NPpdata[0][N] = tTSYS
            NPpflag[0][N] = tPFLAG[1]
            NPprows[0][N] = row
            Valid = True
            if tPFLAG[1] == 0:
                FlaggedRowsCategory[0].append(row)
                Valid = False
            # Weather flag
            if tPFLAG[0] == 0:
                FlaggedRowsCategory[1].append(row)
                Valid = False
            # User flag
            if tPFLAG[2] == 0:
                FlaggedRowsCategory[2].append(row)
                Valid = False

            # commented out not to remove permanent flagged data to show
            #if Valid:
            NPprows[1][N] = row
            # RMS flag before baseline fit
            NPpdata[1][N] = tSTAT[2]
            NPpflag[1][N] = tFLAG[2]
            if tFLAG[2] == 0:
                FlaggedRowsCategory[4].append(row)
            NPpdata[2][N] = tSTAT[1]
            NPpflag[2][N] = tFLAG[1]
            if tFLAG[1] == 0:
                FlaggedRowsCategory[3].append(row)
            # Running mean flag before baseline fit
            NPpdata[3][N] = tSTAT[4]
            NPpflag[3][N] = tFLAG[4]
            if tFLAG[4] == 0:
                FlaggedRowsCategory[6].append(row)
            # Running mean flag after baseline fit
            NPpdata[4][N] = tSTAT[3]
            NPpflag[4][N] = tFLAG[3]
            if tFLAG[3] == 0:
                FlaggedRowsCategory[5].append(row)
            # Expected RMS flag before baseline fit
            NPpdata[5][N] = tSTAT[6]
            NPpflag[5][N] = tFLAG[6]
            if tFLAG[6] == 0:
                FlaggedRowsCategory[8].append(row)
            # Expected RMS flag after baseline fit
            NPpdata[6][N] = tSTAT[5]
            NPpflag[6][N] = tFLAG[5]
            if tFLAG[5] == 0:
                FlaggedRowsCategory[7].append(row)
            N += 1
        # data store finished

        ThreExpectedRMSPreFit = FlagRule['RmsExpectedPreFitFlag']['Threshold']
        ThreExpectedRMSPostFit = FlagRule['RmsExpectedPostFitFlag']['Threshold']
        plots = []
        # Tsys flag
        PlotData = {'row': NPprows[0], 'data': NPpdata[0], 'flag': NPpflag[0], \
                    'thre': [threshold[4][1], 0.0], \
                    'gap': [PosGap, TimeGap], \
                            'title': "Tsys (K)\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % FlagRule['TsysFlag']['Threshold'], \
                    'xlabel': "row (spectrum)", \
                    'ylabel': "Tsys (K)", \
                    'permanentflag': PermanentFlag, \
                    'isActive': FlagRule['TsysFlag']['isActive'], \
                    'threType': "line"}
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_0')
        plots.append(FigFileRoot+'_0.png')

        # RMS flag before baseline fit
        PlotData['row'] = NPprows[1]
        PlotData['data'] = NPpdata[1]
        PlotData['flag'] = NPpflag[1]
        PlotData['thre'] = [threshold[1][1]]
        PlotData['title'] = "Baseline RMS (K) before baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % FlagRule['RmsPreFitFlag']['Threshold']
        PlotData['ylabel'] = "Baseline RMS (K)"
        PlotData['isActive'] = FlagRule['RmsPreFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_1')
        plots.append(FigFileRoot+'_1.png')

        # RMS flag after baseline fit
        PlotData['data'] = NPpdata[2]
        PlotData['flag'] = NPpflag[2]
        PlotData['thre'] = [threshold[0][1]]
        PlotData['title'] = "Baseline RMS (K) after baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % FlagRule['RmsPostFitFlag']['Threshold']
        PlotData['isActive'] = FlagRule['RmsPostFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_2')
        plots.append(FigFileRoot+'_2.png')

        # Running mean flag before baseline fit
        PlotData['data'] = NPpdata[3]
        PlotData['flag'] = NPpflag[3]
        PlotData['thre'] = [threshold[3][1]]
        PlotData['title'] = "RMS (K) for Baseline Deviation from the running mean (Nmean=%d) before baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % (FlagRule['RunMeanPreFitFlag']['Nmean'], FlagRule['RunMeanPreFitFlag']['Threshold'])
        PlotData['isActive'] = FlagRule['RunMeanPreFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_3')
        plots.append(FigFileRoot+'_3.png')

        # Running mean flag after baseline fit
        PlotData['data'] = NPpdata[4]
        PlotData['flag'] = NPpflag[4]
        PlotData['thre'] = [threshold[2][1]]
        PlotData['title'] = "RMS (K) for Baseline Deviation from the running mean (Nmean=%d) after baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % (FlagRule['RunMeanPostFitFlag']['Nmean'], FlagRule['RunMeanPostFitFlag']['Threshold'])
        PlotData['isActive'] = FlagRule['RunMeanPostFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_4')
        plots.append(FigFileRoot+'_4.png')

        # Expected RMS flag before baseline fit
        PlotData['data'] = NPpdata[1]
        PlotData['flag'] = NPpflag[5]
        PlotData['thre'] = [NPpdata[5]]
        PlotData['title'] = "Baseline RMS (K) compared with the expected RMS calculated from Tsys before baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: threshold with the scaling factor of %.1f, Red H-line(s): out of vertical scale limit(s)" % ThreExpectedRMSPreFit
        PlotData['isActive'] = FlagRule['RmsExpectedPreFitFlag']['isActive']
        PlotData['threType'] = "plot"
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_5')
        plots.append(FigFileRoot+'_5.png')

        # Expected RMS flag after baseline fit
        PlotData['data'] = NPpdata[2]
        PlotData['flag'] = NPpflag[6]
        PlotData['thre'] = [NPpdata[6]]
        PlotData['title'] = "Baseline RMS (K) compared with the expected RMS calculated from Tsys after baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: threshold with the scaling factor of %.1f" % ThreExpectedRMSPostFit
        PlotData['isActive'] = FlagRule['RmsExpectedPostFitFlag']['isActive']
        PlotData['threType'] = "plot"
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_6')
        plots.append(FigFileRoot+'_6.png')

        # Create Flagging Summary Page
        if FigFileDir != False:
            Filename = FigFileDir+FigFileRoot+'.html'
            relpath = os.path.basename(FigFileDir.rstrip("/")) ### stage#
            if os.access(Filename, os.F_OK): os.remove(Filename)
            # Assuming single scantable, antenna, spw, and pol
            ID = ids[0]
            ant_id = DataTable.getcell('ANTENNA',ID)
            st_row = DataTable.getcell('ROW',ID)
            st_name = DataTable.getkeyword('FILENAMES')[ant_id]
            st = self.context.observing_run[ant_id]
            ant_name = st.antenna.name
            pol = DataTable.getcell('POL',ID)
            spw = DataTable.getcell('IF',ID)
            
            Out = open(Filename, 'w')
            #print >> Out, '<html>\n<head>\n<style>'
            #print >> Out, '.ttl{font-size:20px;font-weight:bold;}'
            #print >> Out, '.stt{font-size:18px;font-weight:bold;color:white;background-color:navy;}'
            #print >> Out, '.stp{font-size:18px;font-weight:bold;color:black;background-color:gray;}'
            #print >> Out, '.stc{font-size:16px;font-weight:normal;}'
            #print >> Out, '</style>\n</head>\n<body>'
            print >> Out, '<body>'
            print >> Out, '<p class="ttl">Data Summary</p>'
            # A table of data summary
            print >> Out, '<table border="0"  cellpadding="3">'
            print >> Out, '<tr align="left" class="stp"><th>%s</th><th>:</th><th>%s</th></tr>' % ('Name', st_name)
            print >> Out, '<tr align="left" class="stp"><th>%s</th><th>:</th><th>%s</th></tr>' % ('Antenna', ant_name)
            print >> Out, '<tr align="left" class="stp"><th>%s</th><th>:</th><th>%s</th></tr>' % ('Spw ID', spw)
            print >> Out, '<tr align="left" class="stp"><th>%s</th><th>:</th><th>%s</th></tr>' % ('Pol', pol)
            print >> Out, '</table>\n'
            
            print >> Out, '<HR><p class="ttl">Flagging Status</p>'
            # A table of flag statistics summary
            print >> Out, '<table border="1">'
            print >> Out, '<tr align="center" class="stt"><th>&nbsp</th><th>isActive?</th><th>SigmaThreshold<th>Flagged spectra</th><th>Flagged ratio(%)</th></tr>'
            print >> Out, '<tr align="center" class="stp"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('User', FlagRule['UserFlag']['isActive'], FlagRule['UserFlag']['Threshold'], len(FlaggedRowsCategory[2]), len(FlaggedRowsCategory[2])*100.0/NROW)
            print >> Out, '<tr align="center" class="stp"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Weather', FlagRule['WeatherFlag']['isActive'], FlagRule['WeatherFlag']['Threshold'], len(FlaggedRowsCategory[1]), len(FlaggedRowsCategory[1])*100.0/NROW)
            print >> Out, '<tr align="center" class="stp"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Tsys', FlagRule['TsysFlag']['isActive'], FlagRule['TsysFlag']['Threshold'], len(FlaggedRowsCategory[0]), len(FlaggedRowsCategory[0])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('RMS baseline (pre-fit)', FlagRule['RmsPreFitFlag']['isActive'], FlagRule['RmsPreFitFlag']['Threshold'], len(FlaggedRowsCategory[4]), len(FlaggedRowsCategory[4])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('RMS baseline (post-fit)', FlagRule['RmsPostFitFlag']['isActive'], FlagRule['RmsPostFitFlag']['Threshold'], len(FlaggedRowsCategory[3]), len(FlaggedRowsCategory[3])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Running Mean (pre-fit)', FlagRule['RunMeanPreFitFlag']['isActive'], FlagRule['RunMeanPreFitFlag']['Threshold'], len(FlaggedRowsCategory[6]), len(FlaggedRowsCategory[6])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Running Mean (post-fit)', FlagRule['RunMeanPostFitFlag']['isActive'], FlagRule['RunMeanPostFitFlag']['Threshold'], len(FlaggedRowsCategory[5]), len(FlaggedRowsCategory[5])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Expected RMS (pre-fit)', FlagRule['RmsExpectedPreFitFlag']['isActive'], FlagRule['RmsExpectedPreFitFlag']['Threshold'], len(FlaggedRowsCategory[8]), len(FlaggedRowsCategory[8])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Expected RMS (post-fit)', FlagRule['RmsExpectedPostFitFlag']['isActive'], FlagRule['RmsExpectedPostFitFlag']['Threshold'], len(FlaggedRowsCategory[7]), len(FlaggedRowsCategory[7])*100.0/NROW)
            print >> Out, '<tr align="center" class="stt"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Total Flagged', '-', '-', len(FlaggedRows), len(FlaggedRows)*100.0/NROW)
            print >> Out, '<tr><td colspan=4>%s</td></tr>' % ("Note: flags in grey background are permanent, <br> which are not reverted or changed during the iteration cycles.") 
            #print >> Out, '</table>\n</body>\n</html>'
            print >> Out, '</table>\n'
            # Plot figures
            print >> Out, '<HR>\nNote to all the plots below: short green vertical lines indicate position gaps; short cyan vertical lines indicate time gaps\n<HR>'
            for name in plots:
                print >> Out, '<img src="%s/%s">\n<HR>' % (relpath, name)
            #print >> Out, '</body>\n</html>'
            print >> Out, '</body>'
            Out.close()

        # User flag
        LOG.info('Number of rows flagged by User = %d /%d' % (len(FlaggedRowsCategory[2]), NROW))
        if len(FlaggedRowsCategory[2]) > 0:
            LOG.debug('Flagged rows by User =%s ' % FlaggedRowsCategory[2])
        # Weather
        LOG.info('Number of rows flagged by Weather = %d /%d' % (len(FlaggedRowsCategory[1]), NROW))
        if len(FlaggedRowsCategory[1]) > 0:
            LOG.debug('Flagged rows by Weather =%s ' % FlaggedRowsCategory[1])
        # Tsys
        LOG.info('Number of rows flagged by Tsys = %d /%d' % (len(FlaggedRowsCategory[0]), NROW))
        if len(FlaggedRowsCategory[0]) > 0:
            LOG.debug('Flagged rows by Tsys =%s ' % FlaggedRowsCategory[0])
        # Pre-fit RMS
        LOG.info('Number of rows flagged by the baseline fluctuation (pre-fit) = %d /%d' % (len(FlaggedRowsCategory[4]), NROW))
        if len(FlaggedRowsCategory[4]) > 0:
            LOG.debug('Flagged rows by the baseline fluctuation (pre-fit) =%s ' % FlaggedRowsCategory[4])
        # Post-fit RMS
        LOG.info('Number of rows flagged by the baseline fluctuation (post-fit) = %d /%d' % (len(FlaggedRowsCategory[3]), NROW))
        if len(FlaggedRowsCategory[3]) > 0:
            LOG.debug('Flagged rows by the baseline fluctuation (post-fit) =%s ' % FlaggedRowsCategory[3])
        # Pre-fit running mean
        LOG.info('Number of rows flagged by the difference from running mean (pre-fit) = %d /%d' % (len(FlaggedRowsCategory[6]), NROW))
        if len(FlaggedRowsCategory[6]) > 0:
            LOG.debug('Flagged rows by the difference from running mean (pre-fit) =%s ' % FlaggedRowsCategory[6])
        # Post-fit running mean
        LOG.info('Number of rows flagged by the difference from running mean (post-fit) = %d /%d' % (len(FlaggedRowsCategory[5]), NROW))
        if len(FlaggedRowsCategory[5]) > 0:
            LOG.debug('Flagged rows by the difference from running mean (post-fit) =%s ' % FlaggedRowsCategory[5])
        # Pre-fit expected RMS
        LOG.info('Number of rows flagged by the expected RMS (pre-fit) = %d /%d' % (len(FlaggedRowsCategory[8]), NROW))
        if len(FlaggedRowsCategory[8]) > 0:
            LOG.debug('Flagged rows by the expected RMS (pre-fit) =%s ' % FlaggedRowsCategory[8])
        # Post-fit expected RMS
        LOG.info('Number of rows flagged by the expected RMS (post-fit) = %d /%d' % (len(FlaggedRowsCategory[7]), NROW))
        if len(FlaggedRowsCategory[7]) > 0:
            LOG.debug('Flagged rows by the expected RMS (post-fit) =%s ' % FlaggedRowsCategory[7])
        # All categories
        LOG.info('Number of rows flagged by all active categories = %d /%d' % (len(FlaggedRows), NROW))
        if len(FlaggedRows) > 0:
            LOG.debug('Final Flagged rows by all active categories =%s ' % FlaggedRows)

        del threshold, NPpdata, NPpflag, NPprows, PlotData, FlaggedRows, FlaggedRowsCategory
        return os.path.basename(Filename)

    def save_outtable(self, DataTable, ids, out_table_name):
        # 2012/09/01 for Table Output
        #StartTime = time.time()
        tbTBL = gentools(['tb'])[0]
        tbTBL.open(out_table_name, nomodify=False)
        st_rows = list(tbTBL.getcol('Row'))
        LOG.info('Filling flag output in table: %s' % out_table_name)
        LOG.debug('Number of rows in output table = %d' % tbTBL.nrows())
        LOG.debug('Number of rows to be filled = %d' % len(ids))
        for ID in ids:
            strow = DataTable.getcell('ROW', ID)
            try:
                row = st_rows.index(strow)
            except ValueError:
                raise ValueError, "Index search failed for column Row = %d in BL table, %s (Corresponding DataTable ID=%d)" % (strow, out_table_name, ID)
            #LOG.debug('filling %d-th data to ROW=%d' % (ID, row))
            tflaglist = DataTable.getcell('FLAG',ID)[1:7]
            tpflaglist = DataTable.getcell('FLAG_PERMANENT',ID)[:3]
            tstatisticslist = DataTable.getcell('STATISTICS',ID)[1:7]
            flaglist, pflaglist, statisticslist = [], [], []
            for i in range(6):
                flaglist.append([tflaglist[i]])
            for i in range(3):
                pflaglist.append([tpflaglist[i]])
            for i in range(6):
                statisticslist.append([tstatisticslist[i]])

            tbTBL.putcol('StatisticsFlags',flaglist,row,1,1)
            tbTBL.putcol('PermanentFlags',pflaglist,row,1,1)
            tbTBL.putcol('Statistics',statisticslist,row,1,1)
            tbTBL.putcol('SummaryFlag',bool(DataTable.getcell('FLAG_SUMMARY',ID)),row,1,1)
        tbTBL.close()

        


