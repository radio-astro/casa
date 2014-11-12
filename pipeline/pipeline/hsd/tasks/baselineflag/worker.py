from __future__ import absolute_import

import os
import math
import numpy
import time
import copy

import asap as sd
from taskinit import gentools

import pipeline.infrastructure as infrastructure
# import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils
from pipeline.hsd.tasks.common import utils as sdutils

from .flagsummary import _get_iteration
from .. import common

LOG = infrastructure.get_logger(__name__)

from .SDFlagRule import INVALID_STAT

class SDBLFlagWorker(object):
    '''
    The worker class of single dish flagging task.
    This class defines per spwid flagging operation.
    '''


    def __init__(self, context, datatable, clip_niteration, spwid_list, nchan, pols_list, file_index, flagRule, userFlag=[], edge=(0,0)):
        '''
        Constructor of worker class
        '''
        self.context = context
        self.datatable = datatable
        self.clip_niteration = clip_niteration
        self.spwid_list = spwid_list
        self.nchan = nchan
        self.pols_list = pols_list
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
        clip_niteration = self.clip_niteration
        spwid_list = self.spwid_list
        nchan = self.nchan
        pols_list = self.pols_list
        file_index = self.file_index
        flagRule = self.flagRule
        userFlag = self.userFlag
        edge = self.edge
        
        assert len(file_index) == len(spwid_list)
        LOG.debug('Members to be processed:')
        for (a,s,p) in zip(file_index, spwid_list, pols_list):
            LOG.debug('\tAntenna %s Spw %s Pol %s'%(a,s,p))


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
#         # out table name
#         namer = filenamer.BaselineSubtractedTable()
#         namer.spectral_window(spwid)

        flagSummary = []
        # loop over file
        for (idx,spwid,pollist) in zip(file_index, spwid_list, pols_list):
            LOG.debug('Performing flag for Antenna %s Spw %s'%(idx,spwid))
            st = self.context.observing_run[idx]
            filename_in = st.name
            filename_out = st.baselined_name
#             asdm = common.asdm_name(st)
#             namer.asdm(asdm)
#             namer.antenna_name(st.antenna.name)
#             out_table_name = namer.get_filename()
            
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
                
                # Set is_baselined flag when processing not yet baselined data.
                is_baselined = (_get_iteration(self.context.observing_run.reduction_group,idx,spwid,pol) > 0)
                if not is_baselined:
                    LOG.debug("No baseline subtraction operated to data. Skipping flag by post fit spectra.")
                # Reset MASKLIST for the non-baselined DataTable
                if not is_baselined: self.ResetDataTableMaskList(TimeTable)
                flagRule_local = copy.deepcopy(flagRule)
                if not is_baselined: # force disable post fit flagging (not really effective except for flagSummary)
                    flagRule_local['RmsPostFitFlag']['isActive'] = False
                    flagRule_local['RunMeanPostFitFlag']['isActive'] = False
                    flagRule_local['RmsExpectedPostFitFlag']['isActive'] = False
                LOG.debug("FLAGRULE = %s" % str(flagRule_local))
                
                # Calculate Standard Deviation and Diff from running mean
                t0 = time.time()
                data = self.calcStatistics(datatable, filename_in, filename_out, nchan, nmean, TimeTable, edge, is_baselined)
                t1 = time.time()
                LOG.info('Standard Deviation and diff calculation End: Elapse time = %.1f sec' % (t1 - t0))
                
                t0 = time.time()
                tmpdata = numpy.transpose(data)
                dt_idx = numpy.array(tmpdata[0], numpy.int)
                LOG.info('Calculating the thresholds by Standard Deviation and Diff from running mean of Pre/Post fit. (Iterate %d times)' % (clip_niteration))
                stat_flag, final_thres = self._get_flag_from_stats(tmpdata[1:6], Threshold, clip_niteration, is_baselined)
                LOG.debug('final threshold shape = %d' % len(final_thres))
                LOG.info('Final thresholds: StdDev (pre-/post-fit) = %.2f / %.2f , Diff StdDev (pre-/post-fit) = %.2f / %.2f , Tsys=%.2f' % tuple([final_thres[i][1] for i in (1,0,3,2,4)]))
                
                self._apply_stat_flag(datatable, dt_idx, stat_flag)

                # flag by Expected RMS
                self.flagExpectedRMS(datatable, spwid, dt_idx, idx, FlagRule=flagRule_local, rawFileIdx=idx, is_baselined=is_baselined)
  
                # flag by scantable row ID defined by user
                self.flagUser(datatable, dt_idx, UserFlag=userFlag)
                # Check every flags to create summary flag
                self.flagSummary(datatable, dt_idx, flagRule_local) 
                t1 = time.time()
                LOG.info('Apply flags End: Elapse time = %.1f sec' % (t1 - t0))
                
#                 # store statistics and flag information to bl.tbl
#                 self.save_outtable(datatable, dt_idx, out_table_name)
                flagSummary.append({'index': idx, 'spw': spwid, 'pol': pol, 'result_threshold': final_thres, 'baselined': is_baselined})

        end_time = time.time()
        LOG.info('PROFILE execute: elapsed time is %s sec'%(end_time-start_time))

        return flagSummary


    def calcStatistics(self, DataTable, DataIn, DataOut, NCHAN, Nmean, TimeTable, edge, is_baselined):
        # Calculate Standard Deviation and Diff from running mean
        NROW = len([ series for series in utils.flatten(TimeTable) ])/2
        # parse edge
        if len(edge) == 2:
            (edgeL, edgeR) = edge
        else:
            edgeL = edge[0]
            edgeR = edge[0]

        LOG.info('Calculate Standard Deviation and Diff from running mean for Pre/Post fit...')
        LOG.info('Processing %d spectra...' % NROW)
        LOG.info('Nchan for running mean=%s' % Nmean)
        data = []

        ProcStartTime = time.time()

        LOG.info('Standard deviation and diff calculation Start')

        tbIn, tbOut = gentools(['tb','tb'])
        tbIn.open(DataIn)
        if is_baselined:
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
            FlIn = numpy.zeros((nrow, NCHAN), dtype=int)
            FlOut = numpy.zeros((nrow,NCHAN), dtype=int)
            for index in range(len(chunks[0])):
                data_row = chunks[0][index]
                SpIn[index] = tbIn.getcell('SPECTRA', data_row)
                FlIn[index] = tbIn.getcell('FLAGTRA', data_row)
                if is_baselined: 
                    SpOut[index] = tbOut.getcell('SPECTRA', data_row)
                    FlOut[index] = tbOut.getcell('FLAGTRA', data_row)
                SpIn[index][:edgeL] = 0
                SpOut[index][:edgeL] = 0
                FlIn[index][:edgeL] = 128
                FlOut[index][:edgeL] = 128
                if edgeR > 0:
                    SpIn[index][-edgeR:] = 0
                    SpOut[index][-edgeR:] = 0
                    FlIn[index][-edgeR:] = 128
                    FlOut[index][-edgeR:] = 128
            ### loading of the data for one chunk is done

            # list of valid rows in this chunk
            valid_indices = numpy.where(numpy.any(FlIn == 0, axis=1))[0]
            valid_nrow = len(valid_indices)
            
            for index in range(len(chunks[0])):
                row = chunks[0][index]
                idx = chunks[1][index]
                
                # check if current row is valid or not
                isvalid = index in valid_indices
                
                # Countup progress timer
                #Timer.count()

                # Mask out line and edge channels
                masklist = DataTable.getcell('MASKLIST',idx)

                stats = DataTable.getcell('STATISTICS',idx)
                # Calculate Standard Deviation (NOT RMS)
                ### 2011/05/26 shrink the size of data on memory
                mask_in = self._get_mask_array(masklist, (edgeL, edgeR), FlIn[index])
                mask_out = numpy.zeros(NCHAN)
                if isvalid:
                    #mask_in = self._get_mask_array(masklist, (edgeL, edgeR), FlIn[index])
                    OldRMS, Nmask = self._calculate_masked_stddev(SpIn[index], mask_in)
                    #stats[2] = OldRMS
                    del Nmask
    
                    NewRMS = -1
                    if is_baselined:
                        mask_out = self._get_mask_array(masklist, (edgeL, edgeR), FlOut[index])
                        NewRMS, Nmask = self._calculate_masked_stddev(SpOut[index], mask_out)
                        del Nmask
                    #stats[1] = NewRMS
                else:
                    OldRMS = INVALID_STAT
                    NewRMS = INVALID_STAT
                stats[2] = OldRMS
                stats[1] = NewRMS

                
                # Calculate Diff from the running mean
                ### 2011/05/26 shrink the size of data on memory
                ### modified to calculate Old and New statistics in a single cycle
                if isvalid:
                    START += 1

                if nrow == 1:
                    OldRMSdiff = 0.0
                    stats[4] = OldRMSdiff
                    NewRMSdiff = 0.0
                    stats[3] = NewRMSdiff
                elif isvalid:
                    # Mean spectra of row = row+1 ~ row+Nmean
                    if START == 1:
                        RmaskOld = numpy.zeros(NCHAN, numpy.int)
                        RdataOld0 = numpy.zeros(NCHAN, numpy.float64)
                        RmaskNew = numpy.zeros(NCHAN, numpy.int)
                        RdataNew0 = numpy.zeros(NCHAN, numpy.float64)
                        NR = 0
                        for _x in range(1, min(Nmean + 1, valid_nrow)):
                            x = valid_indices[_x]
                            NR += 1
                            RdataOld0 += SpIn[x]
                            masklist = DataTable.getcell('MASKLIST',chunks[1][x])
                            mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlIn[x])
                            RmaskOld += mask0
                            RdataNew0 += SpOut[x]
                            mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlOut[x]) if is_baselined else numpy.zeros(NCHAN)
                            RmaskNew += mask0
                    elif START > (valid_nrow - Nmean):
                        NR -= 1
                        RdataOld0 -= SpIn[index]
                        RmaskOld -= mask_in
                        RdataNew0 -= SpOut[index]
                        RmaskNew -= mask_out
                    else:
                        box_edge = valid_indices[START + Nmean - 1]
                        masklist = DataTable.getcell('MASKLIST',chunks[1][box_edge])
                        RdataOld0 -= (SpIn[index] - SpIn[box_edge])
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlIn[box_edge])
                        RmaskOld += (mask0 - mask_in)
                        RdataNew0 -= (SpOut[index] - SpOut[box_edge])
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlOut[box_edge]) if is_baselined else numpy.zeros(NCHAN)
                        RmaskNew += (mask0 - mask_out)
                    # Mean spectra of row = row-Nmean ~ row-1
                    if START == 1:
                        LmaskOld = numpy.zeros(NCHAN, numpy.int)
                        LdataOld0 = numpy.zeros(NCHAN, numpy.float64)
                        LmaskNew = numpy.zeros(NCHAN, numpy.int)
                        LdataNew0 = numpy.zeros(NCHAN, numpy.float64)
                        NL = 0
                    elif START <= (Nmean + 1):
                        NL += 1
                        box_edge = valid_indices[START - 2]
                        masklist = DataTable.getcell('MASKLIST',chunks[1][box_edge])
                        LdataOld0 += SpIn[box_edge]
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlIn[box_edge])
                        LmaskOld += mask0
                        LdataNew0 += SpOut[box_edge]
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlOut[box_edge]) if is_baselined else numpy.zeros(NCHAN)
                        LmaskNew += mask0
                    else:
                        box_edge_right = valid_indices[START - 2]
                        box_edge_left = valid_indices[START - 2 - Nmean]
                        masklist = DataTable.getcell('MASKLIST',chunks[1][box_edge_right])
                        LdataOld0 += (SpIn[box_edge_right] - SpIn[box_edge_left])
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlIn[box_edge_right])
                        LmaskOld += mask0
                        LdataNew0 += (SpOut[box_edge_right] - SpOut[box_edge_left])
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlOut[box_edge_right]) if is_baselined else numpy.zeros(NCHAN)
                        LmaskNew += mask0
                        masklist = DataTable.getcell('MASKLIST',chunks[1][box_edge_left])
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlIn[box_edge_left])
                        LmaskOld -= mask0
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlOut[box_edge_left]) if is_baselined else numpy.zeros(NCHAN)
                        LmaskNew -= mask0

                    diffOld0 = (LdataOld0 + RdataOld0) / float(NL + NR) - SpIn[index]
                    diffNew0 = (LdataNew0 + RdataNew0) / float(NL + NR) - SpOut[index]

                    # Calculate Standard Deviation (NOT RMS)
                    mask0 = (RmaskOld + LmaskOld + mask_in) / (NL + NR + 1)
                    OldRMSdiff, Nmask = self._calculate_masked_stddev(diffOld0, mask0)
                    stats[4] = OldRMSdiff

                    NewRMSdiff = -1
                    if is_baselined: 
                        mask0 = (RmaskNew + LmaskNew + mask_out) / (NL + NR + 1)
                        NewRMSdiff, Nmask = self._calculate_masked_stddev(diffNew0, mask0)
                    stats[3] = NewRMSdiff
                else:
                    # invalid data
                    OldRMSdiff = INVALID_STAT
                    NewRMSdiff = INVALID_STAT
                    stats[3] = NewRMSdiff
                    stats[4] = OldRMSdiff
                    Nmask = NCHAN

                # Fit STATISTICS and NMASK columns in DataTable (post-Fit statistics will be -1 when is_baselined=F)
                DataTable.putcell('STATISTICS',idx,stats)
                DataTable.putcell('NMASK',idx,Nmask)
                LOG.debug('Row=%d, pre-fit StdDev= %.2f pre-fit diff StdDev= %.2f' % (row, OldRMS, OldRMSdiff))
                if is_baselined: LOG.debug('Row=%d, post-fit StdDev= %.2f post-fit diff StdDev= %.2f' % (row, NewRMS, NewRMSdiff))
                data.append([idx, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, DataTable.getcell('TSYS',idx), Nmask])
            del SpIn, SpOut
        return data

    def _calculate_masked_stddev(self, data, mask):
        """Calculated standard deviation of data array with mask array (1=valid, 0=flagged)"""
        Ndata = len(data)
        Nmask = int(Ndata - numpy.sum(mask))
        MaskedData = data * mask
        StddevMasked = MaskedData.std()
        MeanMasked = MaskedData.mean()
        if Ndata == Nmask:
            # all channels are masked
            RMS = INVALID_STAT
        else:
            RMS = math.sqrt(abs(Ndata * StddevMasked ** 2 / (Ndata - Nmask) - \
                            Ndata * Nmask * MeanMasked ** 2 / ((Ndata - Nmask) ** 2)))
        return RMS, Nmask

        
    def _get_mask_array(self, masklist, edge, flagtra, flagrow=False):
        """Get a list of channel mask (1=valid 0=flagged)"""
        array_type = [list, tuple, numpy.ndarray]
        if type(flagtra) not in array_type:
            raise Exception, "flagtra should be an array"
        if flagrow:
            return [0]*len(flagtra)
        # Not row flagged
        if type(masklist) not in array_type:
            raise Exception, "masklist should be an array"
        if len(masklist) > 0 and type(masklist[0]) not in array_type:
            raise Exception, "masklist should be an array of array"
        if type(edge) not in array_type:
            edge = (edge, edge)
        elif len(edge) == 1:
            edge = (edge[0], edge[0])
        # convert FLAGTRA to mask (1=valid channel, 0=flagged channel)
        mask = numpy.array(sdutils.get_mask_from_flagtra(flagtra))
        # masklist
        for [m0, m1] in masklist: mask[m0:m1] = 0
        # edge channels
        mask[0:edge[0]] = 0
        mask[len(flagtra)-edge[1]:] = 0
        return mask

    def _get_flag_from_stats(self, stat, Threshold, clip_niteration, is_baselined):
        skip_flag = [] if is_baselined else [0, 2]
        Ndata = len(stat[0])
        Nflag = len(stat)
        mask = numpy.ones((Nflag, Ndata), numpy.int)
        for cycle in range(clip_niteration + 1):
            threshold = []
            for x in range(Nflag):
                if x in skip_flag: # for not baselined data
                    threshold.append([-1, -1])
                    # Leave mask all 1 (no need to modify)
                    continue
                valid_data_index = numpy.where(stat[x] != INVALID_STAT)
                LOG.debug('valid_data_index=%s'%(valid_data_index))
                mask[x][numpy.where(stat[x] == INVALID_STAT)] = 0
                Unflag = int(numpy.sum(mask[x] * 1.0))
                if Unflag == 0:
                    # all data are invalid
                    threshold.append([-1, -1])
                    continue
                FlaggedData = (stat[x] * mask[x]).take(valid_data_index)
                StddevFlagged = FlaggedData.std()
                if StddevFlagged == 0: StddevFlagged = FlaggedData[0] / 100.0
                MeanFlagged = FlaggedData.mean()
                AVE = MeanFlagged / float(Unflag) * float(Ndata)
                RMS = math.sqrt(abs( Ndata * StddevFlagged ** 2 / Unflag - \
                                Ndata * (Ndata - Unflag) * MeanFlagged ** 2 / (Unflag ** 2) ))
                #print('x=%d, AVE=%f, RMS=%f, Thres=%s' % (x, AVE, RMS, str(Threshold[x])))
                ThreP = AVE + RMS * Threshold[x]
                if x == 4:
                    # Tsys case
                    ThreM = 0.0
                else: ThreM = -1.0
                threshold.append([ThreM, ThreP])
                #for y in range(Ndata):
                for y in valid_data_index[0]:
                    if ThreM < stat[x][y] <= ThreP: mask[x][y] = 1
                    else: mask[x][y] = 0
                LOG.debug('threshold=%s'%(threshold))
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

    def flagExpectedRMS(self, DataTable, vIF, ids, vAnt, FlagRule=None, rawFileIdx=0, is_baselined=True):
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
                stats[5] = expectedRMS * ThreExpectedRMSPostFit if is_baselined else -1
                stats[6] = expectedRMS * ThreExpectedRMSPreFit
                DataTable.putcell('STATISTICS',ID,stats)
                flags = DataTable.getcell('FLAG',ID)
                if (PostFitRMS > ThreExpectedRMSPostFit * expectedRMS) or PostFitRMS == INVALID_STAT:
                    flags[5] = 0
                else:
                    flags[5] = 1
                if is_baselined and (PreFitRMS == INVALID_STAT or PreFitRMS > ThreExpectedRMSPreFit * expectedRMS):
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

    def ResetDataTableMaskList(self,TimeTable):
        """Reset MASKLIST column of DataTable for row indices in TimeTable"""
        for chunks in TimeTable:
            for index in range(len(chunks[0])):
                idx = chunks[1][index]
                self.datatable.putcell("MASKLIST", idx, [])
    
#     def save_outtable(self, DataTable, ids, out_table_name):
#         # 2012/09/01 for Table Output
#         #StartTime = time.time()
#         tbTBL = gentools(['tb'])[0]
#         tbTBL.open(out_table_name, nomodify=False)
#         st_rows = list(tbTBL.getcol('Row'))
#         LOG.info('Filling flag output in table: %s' % out_table_name)
#         LOG.debug('Number of rows in output table = %d' % tbTBL.nrows()
#         LOG.info('Filling flag output in DataTable')
#         LOG.debug('Number of rows to be filled = %d' % len(ids))
#         for ID in ids:
#             strow = DataTable.getcell('ROW', ID)
#             try:
#                 row = st_rows.index(strow)
#             except ValueError:
#                 raise ValueError, "Index search failed for column Row = %d in BL table, %s (Corresponding DataTable ID=%d)" % (strow, out_table_name, ID)
#             #LOG.debug('filling %d-th data to ROW=%d' % (ID, row))
#             tflaglist = DataTable.getcell('FLAG',ID)[1:7]
#             tpflaglist = DataTable.getcell('FLAG_PERMANENT',ID)[:3]
#             tstatisticslist = DataTable.getcell('STATISTICS',ID)[1:7]
#             flaglist, pflaglist, statisticslist = [], [], []
#             for i in range(6):
#                 flaglist.append([tflaglist[i]])
#             for i in range(3):
#                 pflaglist.append([tpflaglist[i]])
#             for i in range(6):
#                 statisticslist.append([tstatisticslist[i]])
#  
#             tbTBL.putcol('StatisticsFlags',flaglist,row,1,1)
#             tbTBL.putcol('PermanentFlags',pflaglist,row,1,1)
#             tbTBL.putcol('Statistics',statisticslist,row,1,1)
#             tbTBL.putcol('SummaryFlag',bool(DataTable.getcell('FLAG_SUMMARY',ID)),row,1,1)
#         tbTBL.close()

