from __future__ import absolute_import

import copy
import math
import numpy
import os
import time

from taskinit import gentools

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils
from pipeline.hsd.tasks.common import utils as sdutils
from pipeline.domain import DataTable
from pipeline.domain.datatable import OnlineFlagIndex

from .flagsummary import _get_iteration
from .. import common
from .SDFlagRule import INVALID_STAT


LOG = infrastructure.get_logger(__name__)


class SDBLFlagWorkerInputs(basetask.StandardInputs):
    """
    Inputs for imaging worker
    NOTE: infile should be a complete list of MSes 
    """

    def __init__(self, context, clip_niteration, ms_list, antenna_list, fieldid_list, spwid_list, pols_list, nchan,
                 flagRule, userFlag=None, edge=None, rowmap=None):
        super(SDBLFlagWorkerInputs, self).__init__(context, vis=None, output_dir=None)

        if userFlag is None:
            userFlag = []
        if edge is None:
            edge = (0, 0)

        self.clip_niteration = clip_niteration
        self.ms_list = ms_list
        self.antenna_list = antenna_list
        self.fieldid_list = fieldid_list
        self.spwid_list = spwid_list
        self.pols_list = pols_list
        self.flagRule = flagRule
        self.nchan = nchan
        self.userFlag = userFlag
        self.edge = edge
        self.rowmap = rowmap


class SDBLFlagWorkerResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDBLFlagWorkerResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDBLFlagWorkerResults, self).merge_with_context(context)       

    def _outcome_name(self):
        return ''


class SDBLFlagWorker(basetask.StandardTaskTemplate):
    """
    The worker class of single dish flagging task.
    This class defines per spwid flagging operation.
    """
    Inputs = SDBLFlagWorkerInputs
    
    def is_multi_vis_task(self):
        return True
    
    def _search_datacol(self, table):
        """
        Returns data column name to process. Returns None if not found.
        The search order is ['CORRECTED_DATA', 'FLOAT_DATA', 'DATA']
        
        Argument: table tool object of MS to search a data column for.
        """
        col_found = None
        col_list = table.colnames()
        for col in ['CORRECTED_DATA', 'FLOAT_DATA', 'DATA']:
            if col in col_list:
                col_found = col
                break
        return col_found
            
    
#     def execute(self, dry_run=True):
    def prepare(self):
        """
        Invoke single dish flagging based on statistics of spectra.
        Iterates over antenna and polarization for a certain spw ID
        """
        start_time = time.time()

        context = self.inputs.context
        clip_niteration = self.inputs.clip_niteration
        ms_list = self.inputs.ms_list
        antid_list = self.inputs.antenna_list
        fieldid_list = self.inputs.fieldid_list
        spwid_list = self.inputs.spwid_list
        pols_list = self.inputs.pols_list
        nchan = self.inputs.nchan
        flagRule = self.inputs.flagRule
        userFlag = self.inputs.userFlag
        edge = self.inputs.edge
        datatable = DataTable(name=context.observing_run.ms_datatable_name, readonly=False)
        rowmap = self.inputs.rowmap
        
        LOG.debug('Members to be processed in worker class:')
        for (m,a,f,s,p) in zip(ms_list, antid_list, fieldid_list, spwid_list, pols_list):
            LOG.debug('\t%s: Antenna %s Field %d Spw %d Pol %s'%(m.basename,a,f,s,p))

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
        # loop over members (practically, per antenna loop in an MS)
        for (msobj,antid,fieldid,spwid,pollist) in zip(ms_list, antid_list, fieldid_list, spwid_list, pols_list):
            LOG.debug('Performing flag for %s Antenna %d Field %d Spw %d'%(msobj.basename,antid,fieldid,spwid))
            filename_in = msobj.name
            filename_out = msobj.work_data
            
            LOG.info("*** Processing: %s ***" % (os.path.basename(msobj.name)))
            LOG.info("\tpre-fit table: %s (Ant %d)" % (os.path.basename(filename_in), antid))
            LOG.info("\tpost-fit table: %s (Ant %d)" % (os.path.basename(filename_out), antid))
            
            # deviation mask
            deviation_mask = msobj.deviation_mask[(fieldid,antid,spwid)] \
                if (hasattr(msobj, 'deviation_mask') and msobj.deviation_mask.has_key((fieldid,antid,spwid))) else None
            LOG.debug('deviation mask for %s antenna %d field %d spw %d is %s' % \
                      (msobj.basename, antid, fieldid, spwid, deviation_mask))
            
            time_table = datatable.get_timetable(antid, spwid, None, msobj.basename, fieldid)
            # Select time gap list: 'subscan': large gap; 'raster': small gap
            if flagRule['Flagging']['ApplicableDuration'] == "subscan":
                TimeTable = time_table[1]
            else:
                TimeTable = time_table[0]
            LOG.info('Applied time bin for the running mean calculation: %s' % flagRule['Flagging']['ApplicableDuration'])
            flagRule_local = copy.deepcopy(flagRule)
            # Set is_baselined flag when processing not yet baselined data.
            is_baselined = (_get_iteration(context.observing_run.ms_reduction_group,msobj,antid, fieldid,spwid) > 0)
            if not is_baselined:
                LOG.debug("No baseline subtraction operated to data. Skipping flag by post fit spectra.")
                # Reset MASKLIST for the non-baselined DataTable
                self.ResetDataTableMaskList(datatable,TimeTable)
                # force disable post fit flagging (not really effective except for flagSummary)
                flagRule_local['RmsPostFitFlag']['isActive'] = False
                flagRule_local['RunMeanPostFitFlag']['isActive'] = False
                flagRule_local['RmsExpectedPostFitFlag']['isActive'] = False
            elif rowmap is None:
                rowmap = sdutils.make_row_map_for_baselined_ms(msobj)
            LOG.debug("FLAGRULE = %s" % str(flagRule_local))
            
            for pol in pollist:
                LOG.info("[ POL=%s ]" % (pol))
                ddobj = msobj.get_data_description(spw=spwid)
                polid = ddobj.get_polarization_id(pol)
                # Calculate Standard Deviation and Diff from running mean
                t0 = time.time()
                dt_idx, tmpdata, _ = self.calcStatistics(datatable, msobj, nchan, nmean,
                                                         TimeTable, polid, edge,
                                                         is_baselined, rowmap, deviation_mask)
                t1 = time.time()
                LOG.info('Standard Deviation and diff calculation End: Elapse time = %.1f sec' % (t1 - t0))
                
                t0 = time.time()
                LOG.debug('tmpdata.shape=%s, len(Threshold)=%s'%(str(tmpdata.shape),len(Threshold)))
                LOG.info('Calculating the thresholds by Standard Deviation and Diff from running mean of Pre/Post fit. (Iterate %d times)' % (clip_niteration))
                stat_flag, final_thres = self._get_flag_from_stats(tmpdata, Threshold, clip_niteration, is_baselined)
                LOG.debug('final threshold shape = %d' % len(final_thres))
                LOG.info('Final thresholds: StdDev (pre-/post-fit) = %.2f / %.2f , Diff StdDev (pre-/post-fit) = %.2f / %.2f , Tsys=%.2f' % tuple([final_thres[i][1] for i in (1,0,3,2,4)]))
                del tmpdata, _
                
                self._apply_stat_flag(datatable, dt_idx, polid, stat_flag)

                # flag by Expected RMS
                self.flagExpectedRMS(datatable, dt_idx, msobj.name, spwid, polid,
                                     FlagRule=flagRule_local, is_baselined=is_baselined)
  
                # flag by scantable row ID defined by user
                self.flagUser(datatable, dt_idx, polid, UserFlag=userFlag)
                # Check every flags to create summary flag
                self.flagSummary(datatable, dt_idx, polid, flagRule_local)
                t1 = time.time()
                LOG.info('Apply flags End: Elapse time = %.1f sec' % (t1 - t0))
                
#                 # store statistics and flag information to bl.tbl
#                 self.save_outtable(datatable, dt_idx, out_table_name)
                flagSummary.append({'msname': msobj.basename, 'antenna': antid,
                                    'field': fieldid, 'spw': spwid, 'pol': pol,
                                    'result_threshold': final_thres,
                                    'baselined': is_baselined})
            # Generate flag command file
            filename = ("%s_ant%d_field%d_spw%d_blflag.txt" % \
                        (os.path.basename(msobj.work_data), antid, fieldid, spwid))
            do_flag = self.generateFlagCommandFile(datatable, msobj, antid, fieldid, spwid,
                                         pollist, filename)
            if not os.path.exists(filename):
                raise RuntimeError, 'Failed to create flag command file %s' % filename
            if do_flag:
                flagdata_apply_job = casa_tasks.flagdata(vis=filename_out, mode='list',
                                                         inpfile=filename, action='apply')
                self._executor.execute(flagdata_apply_job)
            else:
                LOG.info("No flag command in %s. Skip flagging." % filename)


        end_time = time.time()
        LOG.info('PROFILE execute: elapsed time is %s sec'%(end_time-start_time))
        # Need to flush changes to disk
        datatable.exportdata(minimal=True)

        result = SDBLFlagWorkerResults(task=self.__class__,
                                       success=True,
                                       outcome=flagSummary)
#         return flagSummary
        return result

    def analyse(self, result):
        return result

    def calcStatistics(self, DataTable, msobj, NCHAN, Nmean, TimeTable, polid, edge, is_baselined, rowmap, deviation_mask=None):
        DataIn = msobj.name
        DataOut = msobj.work_data
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

        LOG.info('Standard deviation and diff calculation Start')

        tbIn, tbOut = gentools(['tb','tb'])
        tbIn.open(DataIn)
        datacolIn = self._search_datacol(tbIn)
        if not datacolIn:
            raise RuntimeError, 'Could not find any data column in %s' % DataIn
        if is_baselined:
            tbOut.open(DataOut)
            datacolOut = self._search_datacol(tbOut)
            if not datacolOut:
                raise RuntimeError, 'Could not find any data column in %s' % DataOut
            

        # Create progress timer
        #Timer = ProgressTimer(80, NROW, LogLevel)
        
        # A priori evaluation of output array size
        output_array_size = sum((len(c[0]) for c in TimeTable))
        output_array_index = 0
        datatable_index = numpy.zeros(output_array_size, dtype=int)
        statistics_array = numpy.zeros((5,output_array_size), dtype=numpy.float)
        num_masked_array = numpy.zeros(output_array_size, dtype=int)
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
            FlIn = numpy.zeros((nrow, NCHAN), dtype=numpy.int16)
            FlOut = numpy.zeros((nrow,NCHAN), dtype=numpy.int16)
            for index in range(len(chunks[0])):
                data_row_in = chunks[0][index]
                SpIn[index] = tbIn.getcell(datacolIn, data_row_in)[polid]
                FlIn[index] = tbIn.getcell('FLAG', data_row_in)[polid]
                if is_baselined: 
                    data_row_out = rowmap[data_row_in]
                    SpOut[index] = tbOut.getcell(datacolOut, data_row_out)[polid]
                    FlOut[index] = tbOut.getcell('FLAG', data_row_out)[polid]
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
            
            datatable_index[output_array_index:output_array_index+nrow] = chunks[1]
            for index in xrange(len(chunks[0])):
                row = chunks[0][index]
                idx = chunks[1][index]
                
                # check if current row is valid or not
                isvalid = index in valid_indices
                
                # Countup progress timer
                #Timer.count()

                # Mask out line and edge channels
                masklist = DataTable.tb2.getcell('MASKLIST',idx)
                tStats = DataTable.getcell('STATISTICS',idx)
                stats = tStats[polid]
                # Calculate Standard Deviation (NOT RMS)
                ### 2011/05/26 shrink the size of data on memory
                mask_in = self._get_mask_array(masklist, (edgeL, edgeR), FlIn[index], deviation_mask=deviation_mask)
                mask_out = numpy.zeros(NCHAN)
                if isvalid:
                    #mask_in = self._get_mask_array(masklist, (edgeL, edgeR), FlIn[index])
                    OldRMS, Nmask = self._calculate_masked_stddev(SpIn[index], mask_in)
                    #stats[2] = OldRMS
                    del Nmask
    
                    NewRMS = -1
                    if is_baselined:
                        mask_out = self._get_mask_array(masklist, (edgeL, edgeR), FlOut[index], deviation_mask=deviation_mask)
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
                    Nmask = NCHAN - numpy.sum(mask_out)
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
                            masklist = DataTable.tb2.getcell('MASKLIST',chunks[1][x])
                            mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlIn[x], deviation_mask=deviation_mask)
                            RmaskOld += mask0
                            RdataNew0 += SpOut[x]
                            mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlOut[x], deviation_mask=deviation_mask) if is_baselined else numpy.zeros(NCHAN)
                            RmaskNew += mask0
                    elif START > (valid_nrow - Nmean):
                        NR -= 1
                        RdataOld0 -= SpIn[index]
                        RmaskOld -= mask_in
                        RdataNew0 -= SpOut[index]
                        RmaskNew -= mask_out
                    else:
                        box_edge = valid_indices[START + Nmean - 1]
                        masklist = DataTable.tb2.getcell('MASKLIST',chunks[1][box_edge])
                        RdataOld0 -= (SpIn[index] - SpIn[box_edge])
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlIn[box_edge], deviation_mask=deviation_mask)
                        RmaskOld += (mask0 - mask_in)
                        RdataNew0 -= (SpOut[index] - SpOut[box_edge])
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlOut[box_edge], deviation_mask=deviation_mask) if is_baselined else numpy.zeros(NCHAN)
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
                        masklist = DataTable.tb2.getcell('MASKLIST',chunks[1][box_edge])
                        LdataOld0 += SpIn[box_edge]
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlIn[box_edge], deviation_mask=deviation_mask)
                        LmaskOld += mask0
                        LdataNew0 += SpOut[box_edge]
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlOut[box_edge], deviation_mask=deviation_mask) if is_baselined else numpy.zeros(NCHAN)
                        LmaskNew += mask0
                    else:
                        box_edge_right = valid_indices[START - 2]
                        box_edge_left = valid_indices[START - 2 - Nmean]
                        masklist = DataTable.tb2.getcell('MASKLIST',chunks[1][box_edge_right])
                        LdataOld0 += (SpIn[box_edge_right] - SpIn[box_edge_left])
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlIn[box_edge_right], deviation_mask=deviation_mask)
                        LmaskOld += mask0
                        LdataNew0 += (SpOut[box_edge_right] - SpOut[box_edge_left])
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlOut[box_edge_right], deviation_mask=deviation_mask) if is_baselined else numpy.zeros(NCHAN)
                        LmaskNew += mask0
                        masklist = DataTable.tb2.getcell('MASKLIST',chunks[1][box_edge_left])
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlIn[box_edge_left], deviation_mask=deviation_mask)
                        LmaskOld -= mask0
                        mask0 = self._get_mask_array(masklist, (edgeL, edgeR), FlOut[box_edge_left], deviation_mask=deviation_mask) if is_baselined else numpy.zeros(NCHAN)
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
                tStats[polid] = stats
                DataTable.putcell('STATISTICS',idx,tStats)
                DataTable.putcell('NMASK',idx,Nmask)
                LOG.debug('Row=%d, pre-fit StdDev= %.2f pre-fit diff StdDev= %.2f' % (row, OldRMS, OldRMSdiff))
                if is_baselined: LOG.debug('Row=%d, post-fit StdDev= %.2f post-fit diff StdDev= %.2f' % (row, NewRMS, NewRMSdiff))
                output_serial_index = output_array_index + index
                statistics_array[0,output_serial_index] = NewRMS
                statistics_array[1,output_serial_index] = OldRMS
                statistics_array[2,output_serial_index] = NewRMSdiff
                statistics_array[3,output_serial_index] = OldRMSdiff
                statistics_array[4,output_serial_index] = DataTable.tb1.getcell('TSYS', idx)[polid]
                num_masked_array[output_serial_index] = Nmask
            del SpIn, SpOut
            output_array_index += nrow
        tbIn.close()
        tbOut.close()
        return datatable_index, statistics_array, num_masked_array

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

        
    def _get_mask_array(self, masklist, edge, flagchan, flagrow=False, deviation_mask=None):
        """Get a list of channel mask (1=valid 0=flagged)"""
        array_type = [list, tuple, numpy.ndarray]
        if type(flagchan) not in array_type:
            raise Exception, "flagchan should be an array"
        if flagrow:
            return [0]*len(flagchan)
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
        mask = numpy.array(sdutils.get_mask_from_flagtra(flagchan))
        # masklist
        for [m0, m1] in masklist: 
            mask[m0:m1] = 0
        
        # deviation mask
        if deviation_mask is not None:
            if type(deviation_mask) not in array_type:
                raise Exception, "deviation_mask should be an array or None"
            if len(deviation_mask) > 0 and type(deviation_mask[0]) not in array_type:
                raise Exception, "deviation_mask should be an array of array or None"
            for m0, m1 in deviation_mask:
                mask[m0:m1] = 0
                
        # edge channels
        mask[0:edge[0]] = 0
        mask[len(flagchan)-edge[1]:] = 0
        return mask

    def _get_flag_from_stats(self, stat, Threshold, clip_niteration, is_baselined):
        skip_flag = [] if is_baselined else [0, 2]
        Ndata = len(stat[0])
        Nflag = len(stat)
        mask = numpy.ones((Nflag, Ndata), numpy.int)
        for cycle in xrange(clip_niteration + 1):
            threshold = []
            for x in xrange(Nflag):
                if x in skip_flag: # for not baselined data
                    threshold.append([-1, -1])
                    # Leave mask all 1 (no need to modify)
                    continue
                valid_data_index = numpy.where(stat[x] != INVALID_STAT)[0]
                LOG.debug('valid_data_index=%s'%(valid_data_index))
                #mask[x][numpy.where(stat[x] == INVALID_STAT)] = 0
                Unflag = int(numpy.sum(mask[x][valid_data_index] * 1.0))
                if Unflag == 0:
                    # all data are invalid
                    threshold.append([-1, -1])
                    continue
                FlaggedData = (stat[x] * mask[x]).take(valid_data_index)
                StddevFlagged = FlaggedData.std()
                if StddevFlagged == 0: StddevFlagged = FlaggedData[0] / 100.0
                MeanFlagged = FlaggedData.mean()
                #LOG.debug("Ndata = %s, Unflag = %s, shape(FlaggedData) = %s, Std = %s, mean = %s" \
                #      % (str(Ndata), str(Unflag), str(FlaggedData.shape), str(StddevFlagged), str(MeanFlagged)))
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
                for y in valid_data_index:
                    if ThreM < stat[x][y] <= ThreP: mask[x][y] = 1
                    else: mask[x][y] = 0
                LOG.debug('threshold=%s'%(threshold))
        return mask, threshold

    def _apply_stat_flag(self, DataTable, ids, polid, stat_flag):
        LOG.info("Updating flags in data table")
        N = 0
        for ID in ids:
            flags = DataTable.tb2.getcell('FLAG', ID)
            pflags = DataTable.tb2.getcell('FLAG_PERMANENT', ID)
            flags[polid,1] = stat_flag[0][N]
            flags[polid,2] = stat_flag[1][N]
            flags[polid,3] = stat_flag[2][N]
            flags[polid,4] = stat_flag[3][N]
            pflags[polid,1] = stat_flag[4][N]
            DataTable.putcell('FLAG', ID, flags)
            DataTable.putcell('FLAG_PERMANENT', ID, pflags)
            N += 1

    def flagExpectedRMS(self, DataTable, ids, msname, spwid, polid, FlagRule=None, rawFileIdx=0, is_baselined=True):
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
            fd = open('%s.exp_rms_factors' % (os.path.basename(msname)), 'r')
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
            LOG.info("Cannot read scaling factors for flagging based on expected RMS. Using 1.0.")
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
        msobj = self.inputs.context.observing_run.get_ms(name=msname)
        spw = msobj.get_spectral_window(spwid)
        noiseEquivBW = abs(numpy.mean(spw.channels.chan_effbws)) * nebw_fact

        #tEXPT = DataTable.tb1.getcol('EXPOSURE')
        #tTSYS = DataTable.tb1.getcol('TSYS')

        for ID in ids:
            row = DataTable.tb1.getcell('ROW',ID)
            # The HHT and APEX test data show the "on" time only in the CLASS
            # header. To get the total time, at least a factor of 2 is needed,
            # for OTFs and rasters with several on per off even higher, but this
            # cannot be automatically determined due to lacking meta data. We
            # thus use a manually supplied scaling factor.
            tEXPT = DataTable.tb1.getcell('EXPOSURE', ID)
            integTimeSec = tEXPT * integ_time_fact
            # The Tsys value can be saved for DSB or SSB mode. A scaling factor
            # may be needed. This factor was read above.
            tTSYS = DataTable.tb1.getcell('TSYS', ID)[polid]
            # K->Jy factor
            tAnt = DataTable.tb1.getcell('ANTENNA',ID)
            antname = msobj.get_antenna(tAnt)[0].name
            polname = msobj.get_data_description(spw=spwid).get_polarization_label(polid)
            k2jy_fact = msobj.k2jy_factor[(spwid, antname, polname)] if (hasattr(msobj, 'k2jy_factor') and msobj.k2jy_factor.has_key((spwid, antname, polname))) else 1.0

            currentTsys = tTSYS * tsys_fact * k2jy_fact
            if ((noiseEquivBW * integTimeSec) > 0.0):
                expectedRMS = currentTsys / math.sqrt(noiseEquivBW * integTimeSec)
                # 2008/10/31
                # Comparison with both pre- and post-BaselineFit RMS
                stats = DataTable.tb2.getcell('STATISTICS',ID)
                PostFitRMS = stats[polid, 1]
                PreFitRMS = stats[polid, 2]
                LOG.debug('DEBUG_DM: Row: %d Expected RMS: %f PostFit RMS: %f PreFit RMS: %f' % (row, expectedRMS, PostFitRMS, PreFitRMS))
                stats[polid, 5] = expectedRMS * ThreExpectedRMSPostFit if is_baselined else -1
                stats[polid, 6] = expectedRMS * ThreExpectedRMSPreFit
                DataTable.putcell('STATISTICS',ID,stats)
                flags = DataTable.tb2.getcell('FLAG',ID)
                #if (PostFitRMS > ThreExpectedRMSPostFit * expectedRMS) or PostFitRMS == INVALID_STAT:
                if PostFitRMS != INVALID_STAT and (PostFitRMS > ThreExpectedRMSPostFit * expectedRMS):
                    #LOG.debug("Row=%d flagged by expected RMS postfit: %f > %f (expected)" %(ID, PostFitRMS, ThreExpectedRMSPostFit * expectedRMS))
                    flags[polid, 5] = 0
                else:
                    flags[polid, 5] = 1
                #if is_baselined and (PreFitRMS == INVALID_STAT or PreFitRMS > ThreExpectedRMSPreFit * expectedRMS):
                if is_baselined and PreFitRMS != INVALID_STAT and (PreFitRMS > ThreExpectedRMSPreFit * expectedRMS):
                    #LOG.debug("Row=%d flagged by expected RMS postfit: %f > %f (expected)" %(ID, PreFitRMS, ThreExpectedRMSPreFit * expectedRMS))
                    flags[polid, 6] = 0
                else:
                    flags[polid, 6] = 1
                DataTable.putcell('FLAG',ID,flags)

    def flagUser(self, DataTable, ids, polid, UserFlag=[]):
        # flag by scantable row ID.
        for ID in ids:
            row = DataTable.getcell('ROW', ID)
            # Update User Flag 2008/6/4
            try:
                Index = UserFlag.index(row)
                tPFLAG = DataTable.tb2.getcell('FLAG_PERMANENT', ID)
                tPFLAG[polid, 2] = 0
                DataTable.putcell('FLAG_PERMANENT', ID, tPFLAG)
            except ValueError:
                tPFLAG = DataTable.tb2.getcell('FLAG_PERMANENT', ID)
                tPFLAG[polid, 2] = 1
                DataTable.putcell('FLAG_PERMANENT', ID, tPFLAG)

    def flagSummary(self, DataTable, ids, polid, FlagRule):
        for ID in ids:
            # Check every flags to create summary flag
            tFLAG = DataTable.tb2.getcell('FLAG', ID)[polid]
            tPFLAG = DataTable.tb2.getcell('FLAG_PERMANENT', ID)[polid]
            tSFLAG = DataTable.getcell('FLAG_SUMMARY', ID)
            pflag = self._get_parmanent_flag_summary(tPFLAG, FlagRule)
            sflag = self._get_stat_flag_summary(tFLAG, FlagRule)
            tSFLAG[polid] = pflag*sflag
            DataTable.putcell('FLAG_SUMMARY', ID, tSFLAG)

    def _get_parmanent_flag_summary(self, pflag, FlagRule):
        # FLAG_PERMANENT[0] --- 'WeatherFlag'
        # FLAG_PERMANENT[1] --- 'TsysFlag'
        # FLAG_PERMANENT[2] --- 'UserFlag'
        # FLAG_PERMANENT[3] --- 'OnlineFlag' (fixed)
        
        # OnlineFlag is always active
        if pflag[OnlineFlagIndex] == 0:
            return 0
        
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

    def ResetDataTableMaskList(self,datatable,TimeTable):
        """Reset MASKLIST column of DataTable for row indices in TimeTable"""
        for chunks in TimeTable:
            for index in range(len(chunks[0])):
                idx = chunks[1][index]
                datatable.putcell("MASKLIST", idx, []) # OR more precisely, [[-1,-1]]
    
    def generateFlagCommandFile(self, datatable, msobj, antid, fieldid, spwid, pollist, filename):
        """
        Summarize FLAG status in DataTable and generate flag command file

        Arguments:
            datatable: DataTable instance
            msobj: MS instance to summarize flag
            antid, fieldid, spwid: ANTENNA, FIELD_ID and IF to summarize
            filename: output flag command file name
        Returns if there is any valid flag command in file.
        """
        dt_ids = common.get_index_list_for_ms(datatable, [msobj.name],
                                              [antid], [fieldid], [spwid])
        ant_name = msobj.get_antenna(antid)[0].name
        ddobj = msobj.get_data_description(spw=spwid)
        polids = [ddobj.get_polarization_id(pol) for pol in pollist]
        base_selection = "antenna='%s&&&' spw='%d' field='%d'" % (ant_name, spwid, fieldid)
        time_unit = datatable.tb1.getcolkeyword('TIME', 'UNIT')
        valid_flag_commands = False
        with open(filename, "w") as fout:
            # header part
            fout.write("#"*60+"\n")
            fout.write("# Flag command file for Statistic Flags\n")
            fout.write("# Filename: %s\n" % filename)
            fout.write("# MS name: %s\n" % os.path.basename(msobj.work_data))
            fout.write("# Antenna: %s\n" % ant_name)
            fout.write("# Field ID: %d\n" % fieldid)
            fout.write("# SPW: %d\n" % spwid)
            fout.write("#"*60+"\n")
            # data part
            # Flag status by Active flag type is summarized in FLAG_SUMMARY column
            # NOTE Elements in FLAG and FLAG_PERMANENT have 0 (flagged) even if the
            # flag category is inactive.
            # We want avoid generating flag commands if online flag is the only reason for the flag.
            for i in xrange(len(dt_ids)):
                line = [base_selection]
                ID = dt_ids[i]
                tSFLAG = datatable.tb2.getcell('FLAG_SUMMARY', ID)
                tFLAG = datatable.tb2.getcell('FLAG', ID)
                tPFLAG = datatable.tb2.getcell('FLAG_PERMANENT', ID)
                flag_sum = tFLAG.sum(axis=1) + tPFLAG.sum(axis=1)
                online = tPFLAG[:,OnlineFlagIndex]
                # num_flag: the number of flag types.
                # data are valid (unflagged) only if all elements of flags are 1.
                # Hence sum of flag == num_flag
                num_flag = len(tFLAG[0])+len(tPFLAG[0])
                # Ignore the case only online flag is active (0).
                # in that case, flag_sum = num_flag (if online=1)
                #                          num_flag-1 (if online=0)
                # if online = 1 (valid) => sflag = flag_sum == num_flag
                # if online = 0 (invalid)
                #     => sflag = flag_sum+1 == num_flag if no other flag
                #        is active
                sflag = flag_sum + numpy.ones(flag_sum.shape)-online
                flagged_pols = []
                for idx in range(len(polids)):
                    if tSFLAG[polids[idx]]==0 and sflag[polids[idx]] != num_flag:
                        flagged_pols.append(pollist[idx])
                if (len(flagged_pols)==0): # no flag in selcted pols
                    continue
                valid_flag_commands = True
                if len(flagged_pols)!=len(pollist):
                    line.append("correlation='%s'" % ','.join(flagged_pols))
                timeval = datatable.getcell('TIME', ID)
                tbuff = datatable.getcell('EXPOSURE', ID)*0.5/86400.0
                qtime_s = casatools.quanta.quantity(timeval-tbuff, time_unit)
                qtime_e = casatools.quanta.quantity(timeval+tbuff, time_unit)
                line += ["timerange='%s~%s'" % (casatools.quanta.time(qtime_s, prec=9, form="ymd")[0],
                                                casatools.quanta.time(qtime_e, prec=9, form="ymd")[0]),
                         "reason='blflag'"]
                fout.write(str(" ").join(line)+"\n")
        return valid_flag_commands