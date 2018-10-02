from __future__ import absolute_import
from __future__ import print_function

import os
import numpy
import time
import copy
import itertools

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
from pipeline.domain.datatable import OnlineFlagIndex
from pipeline.domain import DataTable

from . import SDFlagPlotter as SDP
from .. import common

LOG = infrastructure.get_logger(__name__)


class SDBLFlagSummary(object):
    """
    A class of single dish flagging task.
    This class defines per spwid flagging operation.
    """

    def __init__(self, context, ms, antid_list, fieldid_list,
                 spwid_list, pols_list, thresholds, flagRule, userFlag=[]):
        """
        Constructor of worker class
        """
        self.context = context
        self.ms = ms
        datatable_name = os.path.join(self.context.observing_run.ms_datatable_name, self.ms.basename)
        self.datatable = DataTable(name=datatable_name, readonly=True)
        self.antid_list = antid_list
        self.fieldid_list = fieldid_list
        self.spwid_list = spwid_list
        self.pols_list = pols_list
        self.thres_value = thresholds
        self.flagRule = flagRule
        self.userFlag = userFlag
    
    def execute(self, dry_run=True):
        """
        Summarizes flagging results.
        Iterates over antenna and polarization for a certain spw ID
        """
        start_time = time.time()

        datatable = self.datatable
        ms = self.ms
        antid_list = self.antid_list
        fieldid_list = self.fieldid_list
        spwid_list = self.spwid_list
        pols_list = self.pols_list
        thresholds = self.thres_value
        flagRule = self.flagRule
        #userFlag = self.userFlag

        LOG.debug('Members to be processed in worker class:')
        for (a,f,s,p) in itertools.izip(antid_list, fieldid_list, spwid_list, pols_list):
            LOG.debug('\t%s: Antenna %s Field %d Spw %d Pol %s'%(ms.basename,a,f,s,p))
        
        # output directory
        stage_number = self.context.task_counter
        FigFileDir = (self.context.report_dir+"/stage%d" % stage_number)
        ### WORKAROUND to GENERATE stage# dir manually
        if not os.path.exists(FigFileDir):
            os.mkdir(FigFileDir)
        FigFileDir += "/"

        flagSummary = []
        # loop over members (practically, per antenna loop in an MS)
        for (antid,fieldid,spwid,pollist) in itertools.izip(antid_list, fieldid_list, spwid_list, pols_list):
            LOG.debug('Performing flagging for %s Antenna %d Field %d Spw %d'%(ms.basename,antid,fieldid,spwid))
            filename_in = ms.name
            ant_name = ms.get_antenna(antid)[0].name
            asdm = common.asdm_name_from_ms(ms)
            field_name = ms.get_fields(field_id=fieldid)[0].name
            LOG.info("*** Summarizing table: %s ***" % (os.path.basename(filename_in)))
            time_table = datatable.get_timetable(antid, spwid, None, ms.basename, fieldid)
            # Select time gap list: 'subscan': large gap; 'raster': small gap
            if flagRule['Flagging']['ApplicableDuration'] == "subscan":
                TimeTable = time_table[1]
            else:        
                TimeTable = time_table[0]
            flatiter = utils.flatten([chunks[1] for chunks in TimeTable])
            dt_idx = [chunk for chunk in flatiter]
            iteration = _get_iteration(self.context.observing_run.ms_reduction_group,
                                       ms, antid, fieldid, spwid)
            for pol in pollist:
                ddobj = ms.get_data_description(spw=spwid)
                polid = ddobj.get_polarization_id(pol)
                # generate summary plot
                FigFileRoot = ("FlagStat_%s_ant%d_field%d_spw%d_pol%d_iter%d" %
                               (asdm, antid, fieldid, spwid, polid, iteration))
                time_gap = datatable.get_timegap(antid, spwid, None,
                                                 ms=ms, field_id=fieldid)
                # time_gap[0]: PosGap, time_gap[1]: TimeGap
                for i in range(len(thresholds)):
                    thres = thresholds[i]
                    if (thres['msname'] == ms.basename and thres['antenna'] == antid and
                            thres['field'] == fieldid and thres['spw'] == spwid and
                            thres['pol'] == pol):
                        final_thres = thres['result_threshold']
                        is_baselined = thres['baselined'] if 'baselined' in thres else False
                        thresholds.pop(i)
                        break
                if (not is_baselined) and not iteration == 0:
                    raise Exception, "Internal error: is_baselined flag is set to False for baselined data."
                t0 = time.time()
                htmlName, nflags = self.plot_flag(datatable, dt_idx, polid,
                                                  time_gap[0], time_gap[1],
                                                  final_thres, flagRule, FigFileDir,
                                                  FigFileRoot, is_baselined)
                t1 = time.time()
                LOG.info('Plot flags End: Elapsed time = %.1f sec' % (t1 - t0) )
                flagSummary.append({'html': htmlName, 'name': asdm,
                                    'antenna': ant_name, 'field': field_name,
                                    'spw': spwid, 'pol': pol,
                                    'nrow': len(dt_idx), 'nflags': nflags,
                                    'baselined': is_baselined})

        end_time = time.time()
        LOG.info('PROFILE execute: elapsed time is %s sec'%(end_time-start_time))

        return flagSummary

    def _get_parmanent_flag_summary(self, pflag, FlagRule):
        # FLAG_PERMANENT[0] --- 'WeatherFlag'
        # FLAG_PERMANENT[1] --- 'TsysFlag'
        # FLAG_PERMANENT[2] --- 'UserFlag'
        # FLAG_PERMANENT[3] --- 'OnlineFlag' (fixed)
        if pflag[OnlineFlagIndex] == 0:
            return 0
        
        types = ['WeatherFlag', 'TsysFlag', 'UserFlag']
        mask = 1
        for idx in xrange(len(types)):
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
        for idx in xrange(len(types)):
            if FlagRule[types[idx]]['isActive'] and tflag[idx+1] == 0:
                mask = 0
                break
        return mask

    def plot_flag(self, DataTable, ids, polid, PosGap, TimeGap, threshold, FlagRule, FigFileDir, FigFileRoot, is_baselined):
        FlagRule_local = copy.deepcopy(FlagRule)
        if not is_baselined:
            FlagRule_local['RmsPostFitFlag']['isActive'] = False
            FlagRule_local['RunMeanPostFitFlag']['isActive'] = False
            FlagRule_local['RmsExpectedPostFitFlag']['isActive'] = False

        # Plot statistics
        NROW = len(ids)
        # Store data for plotting
        FlaggedRowsCategory = [[],[],[],[],[],[],[],[],[],[]]
        FlaggedRows = []
        PermanentFlag = []
        NPpdata = numpy.zeros((7,NROW), numpy.float)
        NPpflag = numpy.zeros((7,NROW), numpy.int)
        NPprows = numpy.zeros((2,NROW), numpy.int)
        N = 0
        for ID in ids:
            row = DataTable.getcell('ROW', ID)
            # Check every flags to create summary flag
            tFLAG = DataTable.getcell('FLAG',ID)[polid]
            tPFLAG = DataTable.getcell('FLAG_PERMANENT',ID)[polid]
            tTSYS = DataTable.getcell('TSYS',ID)[polid]
            tSTAT = DataTable.getcell('STATISTICS',ID)[polid]

            # permanent flag
            Flag = self._get_parmanent_flag_summary(tPFLAG, FlagRule_local)
            PermanentFlag.append(Flag)
            # FLAG_SUMMARY
            Flag *= self._get_stat_flag_summary(tFLAG, FlagRule_local)
            if Flag == 0:
                FlaggedRows.append(row)
            # Tsys flag
            NPpdata[0][N] = tTSYS
            NPpflag[0][N] = tPFLAG[1]
            NPprows[0][N] = row
            if FlagRule_local['TsysFlag']['isActive'] and tPFLAG[1] == 0:
                FlaggedRowsCategory[0].append(row)
            # Weather flag
            if FlagRule_local['WeatherFlag']['isActive'] and tPFLAG[0] == 0:
                FlaggedRowsCategory[1].append(row)
            # User flag
            if FlagRule_local['UserFlag']['isActive'] and tPFLAG[2] == 0:
                FlaggedRowsCategory[2].append(row)
            # Online flag
            if tPFLAG[3] == 0:
                FlaggedRowsCategory[3].append(row)

            NPprows[1][N] = row
            # RMS flag before baseline fit
            NPpdata[1][N] = tSTAT[2]
            NPpflag[1][N] = tFLAG[2]
            if FlagRule_local['RmsPreFitFlag']['isActive'] and tFLAG[2] == 0:
                FlaggedRowsCategory[5].append(row)
            NPpdata[2][N] = tSTAT[1]
            NPpflag[2][N] = tFLAG[1]
            if FlagRule_local['RmsPostFitFlag']['isActive'] and tFLAG[1] == 0:
                FlaggedRowsCategory[4].append(row)
            # Running mean flag before baseline fit
            NPpdata[3][N] = tSTAT[4]
            NPpflag[3][N] = tFLAG[4]
            if FlagRule_local['RunMeanPreFitFlag']['isActive'] and tFLAG[4] == 0:
                FlaggedRowsCategory[7].append(row)
            # Running mean flag after baseline fit
            NPpdata[4][N] = tSTAT[3]
            NPpflag[4][N] = tFLAG[3]
            if FlagRule_local['RunMeanPostFitFlag']['isActive'] and tFLAG[3] == 0:
                FlaggedRowsCategory[6].append(row)
            # Expected RMS flag before baseline fit
            NPpdata[5][N] = tSTAT[6]
            NPpflag[5][N] = tFLAG[6]
            if FlagRule_local['RmsExpectedPreFitFlag']['isActive'] and tFLAG[6] == 0:
                FlaggedRowsCategory[9].append(row)
            # Expected RMS flag after baseline fit
            NPpdata[6][N] = tSTAT[5]
            NPpflag[6][N] = tFLAG[5]
            if FlagRule_local['RmsExpectedPostFitFlag']['isActive'] and tFLAG[5] == 0:
                FlaggedRowsCategory[8].append(row)
            N += 1
        # data store finished

        ThreExpectedRMSPreFit = FlagRule_local['RmsExpectedPreFitFlag']['Threshold']
        ThreExpectedRMSPostFit = FlagRule_local['RmsExpectedPostFitFlag']['Threshold']
        plots = []
        # Tsys flag
        PlotData = {'row': NPprows[0],
                    'data': NPpdata[0],
                    'flag': NPpflag[0],
                    'thre': [threshold[4][1], 0.0],
                    'gap': [PosGap, TimeGap],
                    'title': "Tsys (K)\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % FlagRule_local['TsysFlag']['Threshold'],
                    'xlabel': "row (spectrum)",
                    'ylabel': "Tsys (K)",
                    'permanentflag': PermanentFlag,
                    'isActive': FlagRule_local['TsysFlag']['isActive'],
                    'threType': "line"}
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_0')
        plots.append(FigFileRoot+'_0.png')

        # RMS flag before baseline fit
        PlotData['row'] = NPprows[1]
        PlotData['data'] = NPpdata[1]
        PlotData['flag'] = NPpflag[1]
        PlotData['thre'] = [threshold[1][1]]
        PlotData['title'] = "Baseline RMS (Jy/beam) before baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % FlagRule_local['RmsPreFitFlag']['Threshold']
        PlotData['ylabel'] = "Baseline RMS (Jy/beam)"
        PlotData['isActive'] = FlagRule_local['RmsPreFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_1')
        plots.append(FigFileRoot+'_1.png')

        # RMS flag after baseline fit
        PlotData['data'] = NPpdata[2] if is_baselined else None
        PlotData['flag'] = NPpflag[2]
        PlotData['thre'] = [threshold[0][1]]
        PlotData['title'] = "Baseline RMS (Jy/beam) after baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % FlagRule_local['RmsPostFitFlag']['Threshold']
        PlotData['isActive'] = FlagRule_local['RmsPostFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_2')
        plots.append(FigFileRoot+'_2.png')

        # Running mean flag before baseline fit
        PlotData['data'] = NPpdata[3]
        PlotData['flag'] = NPpflag[3]
        PlotData['thre'] = [threshold[3][1]]
        PlotData['title'] = "RMS (Jy/beam) for Baseline Deviation from the running mean (Nmean=%d) before baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % (FlagRule_local['RunMeanPreFitFlag']['Nmean'], FlagRule_local['RunMeanPreFitFlag']['Threshold'])
        PlotData['isActive'] = FlagRule_local['RunMeanPreFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_3')
        plots.append(FigFileRoot+'_3.png')

        # Running mean flag after baseline fit
        PlotData['data'] = NPpdata[4] if is_baselined else None
        PlotData['flag'] = NPpflag[4]
        PlotData['thre'] = [threshold[2][1]]
        PlotData['title'] = "RMS (Jy/beam) for Baseline Deviation from the running mean (Nmean=%d) after baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % (FlagRule_local['RunMeanPostFitFlag']['Nmean'], FlagRule_local['RunMeanPostFitFlag']['Threshold'])
        PlotData['isActive'] = FlagRule_local['RunMeanPostFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_4')
        plots.append(FigFileRoot+'_4.png')

        # Expected RMS flag before baseline fit
        PlotData['data'] = NPpdata[1]
        PlotData['flag'] = NPpflag[5]
        PlotData['thre'] = [NPpdata[5]]
        PlotData['title'] = "Baseline RMS (Jy/beam) compared with the expected RMS calculated from Tsys before baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: threshold with the scaling factor of %.1f, Red H-line(s): out of vertical scale limit(s)" % ThreExpectedRMSPreFit
        PlotData['isActive'] = FlagRule_local['RmsExpectedPreFitFlag']['isActive']
        PlotData['threType'] = "plot"
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_5')
        plots.append(FigFileRoot+'_5.png')

        # Expected RMS flag after baseline fit
        PlotData['data'] = NPpdata[2] if is_baselined else None
        PlotData['flag'] = NPpflag[6]
        PlotData['thre'] = [NPpdata[6]]
        PlotData['title'] = "Baseline RMS (Jy/beam) compared with the expected RMS calculated from Tsys after baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: threshold with the scaling factor of %.1f" % ThreExpectedRMSPostFit
        PlotData['isActive'] = FlagRule_local['RmsExpectedPostFitFlag']['isActive']
        PlotData['threType'] = "plot"
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_6')
        plots.append(FigFileRoot+'_6.png')

        # ugly restore for summary table
        if not is_baselined:
            FlagRule_local['RmsPostFitFlag']['isActive'] = FlagRule['RmsPostFitFlag']['isActive']
            FlagRule_local['RmsPostFitFlag']['Threshold'] = "SKIPPED"
            FlagRule_local['RunMeanPostFitFlag']['isActive'] = FlagRule['RunMeanPostFitFlag']['isActive']
            FlagRule_local['RunMeanPostFitFlag']['Threshold'] = "SKIPPED"
            FlagRule_local['RmsExpectedPostFitFlag']['isActive'] = FlagRule['RmsExpectedPostFitFlag']['isActive']
            FlagRule_local['RmsExpectedPostFitFlag']['Threshold'] = "SKIPPED"

        # Create Flagging Summary Page
        if FigFileDir != False:
            Filename = FigFileDir+FigFileRoot+'.html'
            relpath = os.path.basename(FigFileDir.rstrip("/")) ### stage#
            if os.access(Filename, os.F_OK):
                os.remove(Filename)
            # Assuming single MS, antenna, field, spw, and polid
            ID0 = ids[0]
            msobj = self.ms
            antid = DataTable.getcell('ANTENNA', ID0)
            fieldid = DataTable.getcell('FIELD_ID', ID0)
            spwid = DataTable.getcell('IF', ID0)
            asdm = asdm = common.asdm_name_from_ms(msobj)
            ant_name = msobj.get_antenna(antid)[0].name
            field_name = msobj.get_fields(field_id=fieldid)[0].name
            ddobj = msobj.get_data_description(spw=spwid)
            pol_name = ddobj.corr_axis[polid]
            
            Out = open(Filename, 'w')
            print('<body>', file=Out)
            print('<p class="ttl">Data Summary</p>', file=Out)
            # A table of data summary
            print('<table border="0"  cellpadding="3">', file=Out)
            print('<tr align="left" class="stp"><th>%s</th><th>:</th><th>%s</th></tr>' % ('Name', asdm), file=Out)
            print('<tr align="left" class="stp"><th>%s</th><th>:</th><th>%s</th></tr>' % ('Antenna', ant_name), file=Out)
            print('<tr align="left" class="stp"><th>%s</th><th>:</th><th>%s</th></tr>' % ('Field', field_name), file=Out)
            print('<tr align="left" class="stp"><th>%s</th><th>:</th><th>%s</th></tr>' % ('Spw ID', spwid), file=Out)
            print('<tr align="left" class="stp"><th>%s</th><th>:</th><th>%s</th></tr>' % ('Pol', pol_name), file=Out)
            print('</table>\n', file=Out)
            
            print('<HR><p class="ttl">Flagging Status</p>', file=Out)
            # A table of flag statistics summary
            print('<table border="1">', file=Out)
            print('<tr align="center" class="stt"><th>&nbsp</th><th>isActive?</th><th>SigmaThreshold<th>Flagged spectra</th><th>Flagged ratio(%)</th></tr>', file=Out)
            print(_format_table_row_html('User', FlagRule_local['UserFlag']['isActive'], FlagRule_local['UserFlag']['Threshold'], len(FlaggedRowsCategory[2]), NROW), file=Out)
            print(_format_table_row_html('Weather', FlagRule_local['WeatherFlag']['isActive'], FlagRule_local['WeatherFlag']['Threshold'], len(FlaggedRowsCategory[1]), NROW), file=Out)
            print(_format_table_row_html('Tsys', FlagRule_local['TsysFlag']['isActive'], FlagRule_local['TsysFlag']['Threshold'], len(FlaggedRowsCategory[0]), NROW), file=Out)
            print(_format_table_row_html('Online', True, "-", len(FlaggedRowsCategory[3]), NROW), file=Out)
            print(_format_table_row_html('RMS baseline (pre-fit)', FlagRule_local['RmsPreFitFlag']['isActive'], FlagRule_local['RmsPreFitFlag']['Threshold'], len(FlaggedRowsCategory[5]), NROW), file=Out)
            print(_format_table_row_html('RMS baseline (post-fit)', FlagRule_local['RmsPostFitFlag']['isActive'], FlagRule_local['RmsPostFitFlag']['Threshold'], len(FlaggedRowsCategory[4]), NROW), file=Out)
            print(_format_table_row_html('Running Mean (pre-fit)', FlagRule_local['RunMeanPreFitFlag']['isActive'], FlagRule_local['RunMeanPreFitFlag']['Threshold'], len(FlaggedRowsCategory[7]), NROW), file=Out)
            print(_format_table_row_html('Running Mean (post-fit)', FlagRule_local['RunMeanPostFitFlag']['isActive'], FlagRule_local['RunMeanPostFitFlag']['Threshold'], len(FlaggedRowsCategory[6]), NROW), file=Out)
            print(_format_table_row_html('Expected RMS (pre-fit)', FlagRule_local['RmsExpectedPreFitFlag']['isActive'], FlagRule_local['RmsExpectedPreFitFlag']['Threshold'], len(FlaggedRowsCategory[9]), NROW), file=Out)
            print(_format_table_row_html('Expected RMS (post-fit)', FlagRule_local['RmsExpectedPostFitFlag']['isActive'], FlagRule_local['RmsExpectedPostFitFlag']['Threshold'], len(FlaggedRowsCategory[8]), NROW), file=Out)
            print('<tr align="center" class="stt"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Total Flagged', '-', '-', len(FlaggedRows), len(FlaggedRows)*100.0/NROW), file=Out)
            print('<tr><td colspan=4>%s</td></tr>' % ("Note: flags in grey background are permanent, <br> which are not reverted or changed during the iteration cycles."), file=Out) 
            print('</table>\n', file=Out)
            # NOTE for not is_baselined
            if not is_baselined: print('ATTENTION: flag by post-fit spectra are skipped due to absence of baseline-fitting in previous stages.\n', file=Out)
            # Plot figures
            print('<HR>\nNote to all the plots below: short green vertical lines indicate position gaps; short cyan vertical lines indicate time gaps\n<HR>', file=Out)
            for name in plots:
                print('<img src="%s/%s">\n<HR>' % (relpath, name), file=Out)
            print('</body>', file=Out)
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
        # Tsys
        LOG.info('Number of rows flagged by on-line flag = %d /%d' % (len(FlaggedRowsCategory[3]), NROW))
        if len(FlaggedRowsCategory[3]) > 0:
            LOG.debug('Flagged rows by Tsys =%s ' % FlaggedRowsCategory[3])
        # Pre-fit RMS
        LOG.info('Number of rows flagged by the baseline fluctuation (pre-fit) = %d /%d' % (len(FlaggedRowsCategory[5]), NROW))
        if len(FlaggedRowsCategory[5]) > 0:
            LOG.debug('Flagged rows by the baseline fluctuation (pre-fit) =%s ' % FlaggedRowsCategory[5])
        # Post-fit RMS
        if is_baselined: LOG.info('Number of rows flagged by the baseline fluctuation (post-fit) = %d /%d' % (len(FlaggedRowsCategory[4]), NROW))
        if len(FlaggedRowsCategory[4]) > 0:
            LOG.debug('Flagged rows by the baseline fluctuation (post-fit) =%s ' % FlaggedRowsCategory[4])
        # Pre-fit running mean
        LOG.info('Number of rows flagged by the difference from running mean (pre-fit) = %d /%d' % (len(FlaggedRowsCategory[7]), NROW))
        if len(FlaggedRowsCategory[7]) > 0:
            LOG.debug('Flagged rows by the difference from running mean (pre-fit) =%s ' % FlaggedRowsCategory[7])
        # Post-fit running mean
        if is_baselined: LOG.info('Number of rows flagged by the difference from running mean (post-fit) = %d /%d' % (len(FlaggedRowsCategory[6]), NROW))
        if len(FlaggedRowsCategory[6]) > 0:
            LOG.debug('Flagged rows by the difference from running mean (post-fit) =%s ' % FlaggedRowsCategory[6])
        # Pre-fit expected RMS
        LOG.info('Number of rows flagged by the expected RMS (pre-fit) = %d /%d' % (len(FlaggedRowsCategory[9]), NROW))
        if len(FlaggedRowsCategory[9]) > 0:
            LOG.debug('Flagged rows by the expected RMS (pre-fit) =%s ' % FlaggedRowsCategory[9])
        # Post-fit expected RMS
        if is_baselined: LOG.info('Number of rows flagged by the expected RMS (post-fit) = %d /%d' % (len(FlaggedRowsCategory[8]), NROW))
        if len(FlaggedRowsCategory[8]) > 0:
            LOG.debug('Flagged rows by the expected RMS (post-fit) =%s ' % FlaggedRowsCategory[8])
        # All categories
        LOG.info('Number of rows flagged by all active categories = %d /%d' % (len(FlaggedRows), NROW))
        if len(FlaggedRows) > 0:
            LOG.debug('Final Flagged rows by all active categories =%s ' % FlaggedRows)

        flag_nums = [len(FlaggedRows)] + [len(frows) for frows in FlaggedRowsCategory]

        del threshold, NPpdata, NPpflag, NPprows, PlotData, FlaggedRows, FlaggedRowsCategory
        return os.path.basename(Filename), flag_nums


def _format_table_row_html(label, isactive, threshold, nflag, ntotal):
    valid_flag = isactive and (threshold != 'SKIPPED')
    typestr = "%.1f"
    if not valid_flag: typestr="%s"
    html_str = '<tr align="center" class="stp"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>'+typestr+'</th></tr>'
    return html_str % (label, isactive, threshold, (nflag if valid_flag else "N/A"), (nflag*100.0/ntotal if valid_flag else "N/A"))

# validity check in _get_iteration is not necessary since group_member 
# has already been validated at upper level (baselineflag.py)
def _get_iteration(reduction_group, msobj, antid, fieldid, spwid):
    members = []
    for group_desc in reduction_group.values():
        #memids = common.get_valid_ms_members(group_desc, [msobj.name], antid, fieldid, spwid)
        #members.extend([group_desc[i] for i in memids])
        member_id = group_desc._search_member(msobj, antid, spwid, fieldid)
        if member_id is not None:
            members.append(group_desc[member_id])
    if len(members) == 1:
        return members[0].iteration
    elif len(members) == 0:
        raise RuntimeError('Given (%s, %s, %s) is not in reduction group.' % (antid, fieldid, spwid))
    raise RuntimeError('Given (%s, %s, %s) is in more than one reduction groups.' % (antid, fieldid, spwid))
