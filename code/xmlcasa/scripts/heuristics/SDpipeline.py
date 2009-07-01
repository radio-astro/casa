from asap import *
import time
import datetime
import os
import pylab as PL
#import numarray as NA
import numpy as NP
    
import SDDataPreparation as DP
import SDBaselineFit as BF
import SDTool as SDT
import SDPlotter as SDP
import SDjava2html as SDJ
import SDBookKeeper as SDB
import SDCalibration as SDC
# CASA Imager output
import SDImaging as SDI
import re

# for Statistics plot
MATPLOTLIB_FIGURE_ID_1 = 8904
# for Fit spectrum plot
MATPLOTLIB_FIGURE_ID_2 = 8905
# for debug baseline plot
MATPLOTLIB_FIGURE_ID_3 = 8906
# for spectrum result
MATPLOTLIB_FIGURE_ID_4 = 8907
# for clustering result
MATPLOTLIB_FIGURE_ID_5 = 8908

MaxPanels4SparseSP = 8

#BF_DATA_DIR = 'BF_Data'
BF_DATA_DIR = 'measurementset'
BF_CLUSTER_PDIR = 'BF_Clstr'
BF_CLUSTER_DIR = 'measurementset'
BF_FIT_PDIR = 'BF_Fit'
BF_FIT_DIR = 'calibrater'
BF_STAT_PDIR = 'BF_Stat'
BF_STAT_DIR = 'flagger'
BF_GRID_PDIR = 'Gridding'
BF_GRID_DIR = 'imager'
BF_CHANNELMAP_PDIR = 'ChannelMap'
BF_SPARSESP_PDIR = 'SparseSpMap'

SD_PIPELINE_LOG = 'PIPELINE.log'
#SD_PIPELINE_LOG = 'reducer.log'
BF_DATA_LOG = 'Data.log'
#BF_DATA_LOG = 'checkMS.log'
BF_GROUP_LOG = 'BF_Grouping.log'
#BF_GROUP_LOG = 'calibrater.log'
BF_DETECT_LOG = 'BF_DetectLine.log'
#BF_DETECT_LOG = 'plotter.log'
BF_CLUSTER_LOG = '/BF_Cluster.log'
#BF_CLUSTER_LOG = 'plotter.log'
BF_FITORDER_LOG = 'BF_FitOrder.log'
#BF_FITORDER_LOG = 'plotter.log'
BF_FIT_LOG = 'BF_Fit.log'
#BF_FIT_LOG = 'plotter.log'
BF_STAT_LOG = 'Flagger.log'
#BF_STAT_LOG = 'flagger.log'
BF_GRID_LOG = 'Gridding.log'
#BF_GRID_LOG = 'imager.log'

#for debug
StopNow = False 

def SDpipeline(filename, outname='default', calib=False, IF='all', POL='all', BEAM='all', SCAN='all', SCANbase='all', ROW='all', ROWbase='all', SpectrumWindow=[], radius=0.0025, edge=(0, 0), BroadComponent=False, iteration=0, UserFlag=[], ImageCube=True, History=True, LogLevel=1, colormap='color', PlotControl='SDpipeline.plotcontrol.txt'):
    """
    Single dish pipeline heuristics script

    Parameters:
    	filename:	name of data set (currently, ASAP scantable format is assumed)
        outname:	root name of the output data set. if omitted, filename is used instead.
        calib:          if True, do calibration before reading data.
    	IF:		a list of IF ids to be processed (default = 'all')
        POL:		a list of Polarization ids to be processed (default = 'all')
        BEAM:           a list of BEAM to be processed (default = 'all')
        SCAN:
        SCANbase:		
        ROW:
        ROWbase:
        SpectrumWindow:			
        radius:		
        edge:		numbers of channels to be dropped for each edge of spectrum for baseline fitting ( e.g. (100:50)) 
        BroadComponent:	
        iteration:	
        UserFlag:       user specified flag [row0, row1, ..., rowM]
        ImageCube:      if True, make image cube(s)
        LogLevel:	logging level (0:mininal, 1:include warning, etc, 2:standard, 3:include DEBUG messages, 4: most verobse)
        colormap:	if set to 'color' use colormap
        PlotControl:	name of file containing plot control parameters

    Notes:
    	SpectrumWindow: [[CenterFreq(GHz), MinVelocity(Km/s), MaxVelocity(Km/s)],[CF,MinV,MaxV],,,]
        if one velocity is set for the CenterFreq (e.g., [CenterFreq, Vel]), SpectrumWindow is
        set as [CenterFreq, -Vel, +Vel]
    """
    RunParams = vars()
    ParamKeys = ['filename', 'outname', 'calib', 'IF', 'POL', 'BEAM', 'SCAN', 'SCANbase', 'ROW', 'ROWbase', 'SpectrumWindow', 'radius', 'edge', 'BroadComponent', 'iteration', 'UserFlag', 'ImageCube', 'LogLevel', 'colormap', 'PlotControl']
    if ( calib ):
        rawdata = filename
        filename = filename.rstrip('/')+'_cal'
    else:
        filename = filename.rstrip('/')
    if outname == 'default': outname = filename

    ### Default Plot control (Overridden if there is 'SDpipeline.plotcontrol.txt' file in the current directory)
    # Plot on terminal (either True, False, or 'Last')
    TPlotRADEC = False
    TPlotAzEl = False
    TPlotCluster = False
    TPlotFit = False
    TPlotMultiSP = False
    TPlotSparseSP = False
    TPlotChannelMap = False
    TPlotFlag = False
    # Plot on browser (either True, False, or 'Last')
    BPlotRADEC = True
    BPlotAzEl = True
    BPlotCluster = True
    BPlotFit = True
    BPlotMultiSP = True
    BPlotSparseSP = True
    BPlotChannelMap = True
    BPlotFlag = True

    # BookKeeper
    if History:
        #BK = SDB.SDbookkeeper(filename+'.History')
        BK = SDB.SDbookkeeper(outname+'.History')
        ReStartID = -1
        while ReStartID < 0:
            if BK.ID != 0:
                line = raw_input('Input Line Number to Re-start (Start All: 0) : ')
                if line.isdigit():
                    if 0 <= int(line) <= BK.ID:
                        ReStartID = int(line)
                if ReStartID == 0: BK.clean()
            else:
                ReStartID = 0

    StartTime = time.time()

    # saved the input parameters used
    InParamstr = ''
    for x in ParamKeys:
        #if x == 'radius' or x == 'spacing':
        if x == 'radius':
            InParamstr += x + '=' + str(RunParams[x])+ '[deg]\n'
        elif x == 'edge':
            InParamstr += x + '=' + str(RunParams[x])+ '[no. channels]\n'
        else:
            InParamstr += x + '=' + str(RunParams[x])+ '\n'
    # radius: beam size in degree

    # Check and apply Plot Control File
    Contents = ['RADEC', 'AZEL', 'CLUSTER', 'FIT', 'MULTISP', 'SPARSESP', 'CHANNELMAP', 'FLAG']
    if os.access(PlotControl, os.F_OK):
        File = open(PlotControl, 'r')
        TPcontrol = [TPlotRADEC, TPlotAzEl, TPlotCluster, TPlotFit, TPlotMultiSP, TPlotSparseSP, TPlotChannelMap, TPlotFlag]
        BPcontrol = [BPlotRADEC, BPlotAzEl, BPlotCluster, BPlotFit, BPlotMultiSP, BPlotSparseSP, BPlotChannelMap, BPlotFlag]
        while 1:
            line = File.readline()
            if not line: break
            item = line[:-1].upper().replace(' ', '').replace('\'', '').split('=')
            if item[0][0] != '#':
                for ID in range(len(Contents)):
                    if item[0] == ('TPLOT' + Contents[ID]):
                        if item[1] == 'TRUE': val = True
                        elif item[1] == 'FALSE': val = False
                        elif item[1] == 'LAST': val = 'Last'
                        else: break
                        TPcontrol[ID] = val
                    if item[0] == ('BPLOT' + Contents[ID]):
                        if item[1] == 'TRUE': val = True
                        elif item[1] == 'FALSE': val = False
                        elif item[1] == 'LAST': val = 'Last'
                        else: break
                        BPcontrol[ID] = val
        [TPlotRADEC, TPlotAzEl, TPlotCluster, TPlotFit, TPlotMultiSP, TPlotSparseSP, TPlotChannelMap, TPlotFlag] = TPcontrol
        [BPlotRADEC, BPlotAzEl, BPlotCluster, BPlotFit, BPlotMultiSP, BPlotSparseSP, BPlotChannelMap, BPlotFlag] = BPcontrol
        File.close()
        #print 'TPcontrol = %s' % TPcontrol
        #print 'BPcontrol = %s' % BPcontrol

    # Setup LogFile
    LogDir = outname+'.logs'
    PlotDir = outname+'.plots'
    SummaryDir = outname+'.summary'
    #LogDir = filename+'.logs'
    #PlotDir = filename+'.plots'
    #SummaryDir = filename+'.summary'
    # Remove Old Log Directories at start-all
    if History == False or ReStartID == 0:
        if os.access(LogDir, os.F_OK):
            for root, dirs, files in os.walk(LogDir, topdown=False):
                for name in files:
                    os.remove(os.path.join(root, name))
                for name in dirs:
                    os.rmdir(os.path.join(root, name))
        else: os.mkdir(LogDir)
        if os.access(PlotDir, os.F_OK):
            for root, dirs, files in os.walk(PlotDir, topdown=False):
                for name in files:
                    os.remove(os.path.join(root, name))
                for name in dirs:
                    os.rmdir(os.path.join(root, name))
        else: os.mkdir(PlotDir)
        os.mkdir(PlotDir+'/'+BF_CLUSTER_PDIR)
        os.mkdir(PlotDir+'/'+BF_FIT_PDIR)
        os.mkdir(PlotDir+'/'+BF_STAT_PDIR)
        os.mkdir(PlotDir+'/'+BF_GRID_PDIR)
        os.mkdir(PlotDir+'/'+BF_CHANNELMAP_PDIR)
        os.mkdir(PlotDir+'/'+BF_SPARSESP_PDIR)
        if os.access(SummaryDir, os.F_OK):
            for root, dirs, files in os.walk(SummaryDir, topdown=False):
                for name in files:
                    os.remove(os.path.join(root, name))
                for name in dirs:
                    os.rmdir(os.path.join(root, name))
        else: os.mkdir(SummaryDir)
        os.mkdir(SummaryDir+'/'+BF_CLUSTER_DIR)
        os.mkdir(SummaryDir+'/'+BF_FIT_DIR)
        os.mkdir(SummaryDir+'/'+BF_STAT_DIR)
        os.mkdir(SummaryDir+'/'+BF_GRID_DIR)
        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'w')
        BFlog0.close()

    BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
    SDT.LogMessage('TITLE', LogLevel, BFlog0, Msg='Pipeline process start: %s' % time.ctime(StartTime))
    SDT.LogMessage('TITLE', LogLevel, BFlog0, Msg='Input parameters to SDpipeline')
    SDT.LogMessage('TITLE', LogLevel, BFlog0, Msg=InParamstr)
    BFlog0.close()

    # Calibration using CASA single-dish tools (ASAP)
    # Only position-switch (ps) mode is supported at the moment
    #
    # -- 18/08/2008 Takeshi Nakazato
    if ( calib ):
        ProcStartTime = time.time()
        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Calibration Start: %s' % time.ctime(ProcStartTime))
        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Calibration mode is ps')
        # position-switch calibration is assumed
        # tool level processing using asap
        scal = SDC.SDCalibration( filename=rawdata, caltype='none', LogLevel=LogLevel, LogFile=BFlog0 )
        scal.summary()
        scal.calibrate()
        if ( not scal.isCalibrated() ):
            SDT.LogMessage( 'INFO', LogLevel, BFlog0, Msg='Data could not calibrated. Abort.' )
            del scal
            return
        scal.save()
        del scal
        if os.path.exists( rawdata+'.exp_rms_factors' ):
            os.system( 'ln -s '+rawdata+'.exp_rms_factors '+filename+'.exp_rms_factors' )
        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Raw Data: %s' % rawdata )
        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Calibrated Data: %s' % filename )
        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Calibration End: %s' % time.ctime(ProcStartTime))
        BFlog0.close()

    # Extract Information to make DataTable file & Dictionary
    # Spectra selected by IF/POL/SCAN/ROW are processed
    # SCANbase: scans used only for selecting emission line channels
    #         SCANbase must be equal to or includes SCAN
    #
    #   DataTable: [Row, Scan, IF, Pol, Beam, Date, Time, ElapsedTime,
    #                 0     1   2    3     4     5     6            7
    #               Exptime, RA, DEC, Az, El, nchan, Tsys, TargetName,
    #                     8   9   10  11  12     13    14          15
    #               Statistics, Flags, PermanentFlags, SummaryFlag, Nmask, MaskList, NoChange]
    #                       16,    17,             18,          19,    20,       21,       22
    #   Statistics: DataTable[ID][16] =
    #               [LowFreqRMS, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, ExpectedRMSThreshold]
    #                         0       1       2           3           4            5
    #   Flags: DataTable[ID][17] =
    #               [LowFrRMSFlag, NewRMSFlag, OldRMSFlag, NewRMSdiff, OldRMSdiff, ExpRMSFlag]
    #                           0           1           2           3           4           5
    #   PermanentFlags: DataTable[ID][18] =
    #               [WeatherFlag, TsysFlag, UserFlag]
    #                          0         1         2
    # listall = [[IDs], [ROWs], [SCANs], [IFs], [POLs], [BEAMs]]
    #           [    0,      1,       2,     3,      4,       5]
    ProcStartTime = time.time()
    BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='ReadData Start: %s' % time.ctime(ProcStartTime))
    BFlog0.close()
    (DataTable, listall, SpStorage, Abcissa) = DP.ReadData(filename, LogDir+'/'+BF_DATA_LOG, SummaryDir+'/'+BF_DATA_DIR+'/index.html')
    ProcEndTime = time.time()
    BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='ReadData End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), (ProcEndTime-ProcStartTime)))
    BFlog0.close()

    # for ASDM data
    if ( os.path.isdir(filename) and os.path.exists(filename+'/ASDM.xml') ):
        filename = filename + '.ms'

    # Setup lists
    if ROW == 'all': ROW = listall[1]
    #if ROWbase == 'all': ROWbase = listall[1]
    if ROWbase == 'all': ROWbase = listall[0]
    if SCAN == 'all': SCAN = listall[2]
    if SCANbase == 'all': SCANbase = listall[2]
    if IF == 'all': IF = listall[3]
    if POL == 'all': POL = listall[4]
    if BEAM == 'all': BEAM = listall[5]

    PL.ion()
    if LogLevel > 3:
        PL.figure(MATPLOTLIB_FIGURE_ID_3)
    PL.figure(MATPLOTLIB_FIGURE_ID_4)
    PL.figure(MATPLOTLIB_FIGURE_ID_5)
    PL.figure(MATPLOTLIB_FIGURE_ID_2)
    PL.figure(MATPLOTLIB_FIGURE_ID_1)
    PL.ioff()

    #spacing = radius / 2.0
    #spacing = radius
    spacing = radius / 3.0 * 2.0
    #ClusterGridSize = spacing * 4.0
    ClusterGridSize = spacing * 3.0
    #ClusterGridSize = spacing * 2.0
    
    if History:
        BK.countup()
        if BK.Counter == ReStartID:
            (DataTable) = BK.read(ReStartID, [], 'DataTable')
        elif BK.Counter > ReStartID:
            BK.write('PreGrouping', DataTable=DataTable)
    if History == False or BK.Counter >= ReStartID:
        (tmpdict, PosGap) = BF.GroupByPosition(DataTable, ROWbase, radius * 1.0, radius / 10.0, False)
        (tmptable, TimeGap) = BF.GroupByTime(DataTable, ROWbase, ROWbase)
        (tmptable, TimeGap) = BF.MergeGapTables(DataTable, TimeGap, tmptable, PosGap)
        # to speed-up, commented out
        #tmpPattern = BF.ObsPatternAnalysis(tmpdict, ROW)
        del tmpdict, tmptable
    
        if BPlotRADEC != False: FigFile = SummaryDir+'/'+BF_DATA_DIR+'/RADEC.png'
        else: FigFile = False
    
        #SDP.DrawRADEC(DataTable, ROW, connect=True, circle=[radius], ObsPattern=tmpPattern, ShowPlot=TPlotRADEC, FigFile=FigFile)
        #SDP.DrawRADEC(DataTable, ROWbase, connect=True, circle=[radius], ObsPattern=tmpPattern, ShowPlot=TPlotRADEC, FigFile=FigFile)
        # to speed-up, don't calculate ObsPattern here
        SDP.DrawRADEC(DataTable, ROWbase, connect=True, circle=[radius], ObsPattern=False, ShowPlot=TPlotRADEC, FigFile=FigFile)
        if BPlotAzEl != False: FigFile = SummaryDir+'/'+BF_DATA_DIR+'/AzEl.png'
        else: FigFile = False
        #SDP.DrawAzEl(DataTable, TimeGap, ROW, ShowPlot=TPlotAzEl, FigFile=FigFile)
        SDP.DrawAzEl(DataTable, TimeGap, ROWbase, ShowPlot=TPlotAzEl, FigFile=FigFile)

    #### for debugging/development
    if StopNow: return 0 

    Initial = 0
    if type(edge) is int or type(edge) is float: edge = (int(edge), int(edge))
    elif len(edge) == 1: edge = (int(edge[0]), int(edge[0]))
    else: edge = (int(edge[0]), int(edge[1]))
    nchan = DataTable[0][13]
    #BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'w')
    #BFlog0.close()
    #BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
    #SDT.LogMessage('TITLE', LogLevel, BFlog0, Msg='Pipeline process start: %s' % time.ctime(StartTime))
    #SDT.LogMessage('TITLE', LogLevel, BFlog0, Msg='Input parameters to SDpipeline')
    #SDT.LogMessage('TITLE', LogLevel, BFlog0, Msg=InParamstr)
    #BFlog0.close()
    for vIF in IF:
        for vPOL in POL:
            # rows: Final output will be calculated for list of rows
            # basedata: used only for selecting emission line channels
            rows = []
            rowsid = []
            basedata = []
            basedataid = []
            #beam = []
            #beamid = {}
            for ID in listall[0]:
                if DataTable[ID][2] == vIF and \
                   DataTable[ID][3] == vPOL and \
                   DataTable[ID][4] in BEAM:
                    if DataTable[ID][1] in SCANbase and \
                       ID in ROWbase:
                            basedata.append(DataTable[ID][0])
                            basedataid.append(ID)
                    if DataTable[ID][1] in SCAN and \
                       ID in ROW:
                            rows.append(DataTable[ID][0])
                            rowsid.append(ID)

            # Do Pipeline for selected IF & POL
            if len(rows) > 0:
                BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                SDT.LogMessage('PROGRESS', LogLevel, BFlog0, Msg='IF = %s  POL = %s' % (vIF, vPOL))
                SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='nrows = %s  nbasedata = %s' % (len(rows), len(basedata)))
                SDT.LogMessage('LONG', LogLevel, BFlog0, Msg='rows = %s' % rows)
                SDT.LogMessage('LONG', LogLevel, BFlog0, Msg='basedata = %s' % basedata)
                BFlog0.close()

                BFlog = open(LogDir+'/'+BF_GROUP_LOG, 'a')

                # GroupByPosition: grouping the data by their positions.
                #  PosTable: output table consists of information i.e.,
                #   ID, IDs associated to the ID (within the radius)
                if History:
                    BK.countup()
                    if BK.Counter == ReStartID:
                        (DataTable) = BK.read(ReStartID, [], 'DataTable')
                    elif BK.Counter > ReStartID:
                        BK.write('Grouping', DataTable=DataTable)
                if History == False or BK.Counter >= ReStartID:
                    ProcStartTime = time.time()
                    BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='GroupByPostion Start: %s' % time.ctime(ProcStartTime))
                    #(PosDictAll, PosGap) = BF.GroupByPosition(DataTable, basedata, radius * 1.0, radius / 10.0, LogLevel=LogLevel, LogFile=BFlog)
                    (PosDictAll, PosGap) = BF.GroupByPosition(DataTable, basedataid, radius * 1.0, radius / 10.0, LogLevel=LogLevel, LogFile=BFlog)
                    if len(basedataid) == len(rowsid):
                        PosDict = PosDictAll
                        PosGapTmp = PosGap
                    else:
                        #(PosDict, PosGapTmp) = BF.GroupByPosition(DataTable, rows, radius * 1.0, radius / 10.0, LogLevel=LogLevel, LogFile=BFlog)
                        (PosDict, PosGapTmp) = BF.GroupByPosition(DataTable, rowsid, radius * 1.0, radius / 10.0, LogLevel=LogLevel, LogFile=BFlog)
                    ProcEndTime = time.time()
                    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='GroupByPostion End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
                    BFlog0.close()
                    ProcStartTime = time.time()
                    BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='ObsPatternAnalysis Start: %s' % time.ctime(ProcStartTime))
                    Pattern = BF.ObsPatternAnalysis(PosDictAll, basedata, LogLevel=LogLevel, LogFile=BFlog)
                    #Pattern = BF.ObsPatternAnalysis(PosDict, rows, LogLevel=LogLevel, LogFile=BFlog)
                    ProcEndTime = time.time()
                    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='ObsPattern = %s' % Pattern)
                    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='ObsPatternAnalysis End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
                    BFlog0.close()

                    # GroupByTime: grouping the data by time lag to the next integration
                    ProcStartTime = time.time()
                    BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='GroupByTime Start: %s' % time.ctime(ProcStartTime))
                    #(TimeTable, TimeGap) = BF.GroupByTime(DataTable, basedata, rows, LogLevel=LogLevel, LogFile=BFlog)
                    (TimeTable, TimeGap) = BF.GroupByTime(DataTable, basedataid, rowsid, LogLevel=LogLevel, LogFile=BFlog)
                    # Merge Gap Tables
                    (TimeTable, TimeGap) = BF.MergeGapTables(DataTable, TimeGap, TimeTable, PosGap, LogLevel=LogLevel, LogFile=BFlog)
                    ProcEndTime = time.time()
                    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='GroupByTime End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
                    BFlog0.close()

                # for Pre-Defined Spectrum Window
                # SpectrumWindow format: [[CenterFreq(GHz), minVel(Km/s), maxVel(Km/s)],[CF,,],,,]
                # SpWin format: [CenterChannel, FullWidthChannel]
                # SpWinRange format: [channel0, channel1]
                (SpWin, SpWinRange) = SDT.FreqWindow2Channel(Abcissa[vIF], SpectrumWindow)
                #print '\n***********************************************************************'
                #print Abcissa[vIF][1]
                #print 'SpWin = ', SpWin
                #print 'SpectrumWindow =', SpectrumWindow
                #print 'SpWinRange =', SpWinRange
                #print '***********************************************************************\n'

                # Process1: tentative lower order baseline subtraction for
                #  tentative accumuration of spectra in the vicinity of each
                #  position on the sky.
                if History:
                    BK.countup()
                    if BK.Counter == ReStartID:
                        (SpStorage[0], DataTable, PosDict, PosDictAll, TimeTable, TimeGap, Pattern) = BK.read(ReStartID, [0], 'DataTable', 'PosDict', 'PosDictAll', 'TimeTable', 'TimeGap', 'Pattern')
                    elif BK.Counter > ReStartID:
                        BK.write('Process1:%d:%d' % (vIF, vPOL), [0], [SpStorage[0]], DataTable=DataTable, PosDict=PosDict, PosDictAll=PosDictAll, TimeTable=TimeTable, TimeGap=TimeGap, Pattern=Pattern)
                if History == False or BK.Counter >= ReStartID:
                    ProcStartTime = time.time()
                    BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process1 Start: %s: DC and 1st order baseline gradient removal' % time.ctime(ProcStartTime))
                    BFlog0.close()
                    #BF.Process1(SpStorage, DataTable, basedata, FFTORDER=1, edge=edge, LogLevel=LogLevel, LogFile=BFlog)
                    BF.Process1(SpStorage, DataTable, basedataid, FFTORDER=1, edge=edge, LogLevel=LogLevel, LogFile=BFlog)
                    ProcEndTime = time.time()
                    BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                    SDT.LogMessage('LONG', LogLevel, BFlog0, Msg='SpStorage[2][0] after Proc1= %s' % SpStorage[2][0])
                    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process1 End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
                    BFlog0.close()

                # Process2: accumurate spectra within specified radius
                #  centered at GroupByPosition
                if History:
                    BK.countup()
                    if BK.Counter == ReStartID:
                        (SpStorage[0], SpStorage[1], SpStorage[2], DataTable, TimeTable, TimeGap, PosDict, PosDictAll, Pattern) = BK.read(ReStartID, [0,1,2], 'DataTable', 'TimeTable', 'TimeGap', 'PosDict', 'PosDictAll', 'Pattern')
                        #(SpStorage[0], SpStorage[1], DataTable, PosDictAll) = BK.read(ReStartID, [0,1], 'DataTable', 'PosDictAll')
                    elif BK.Counter > ReStartID:
                        BK.write('Process2:%d:%d' % (vIF, vPOL), [1], [SpStorage[1]], DataTable=DataTable)
                if History == False or BK.Counter >= ReStartID:
                    ProcStartTime = time.time()
                    BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process2 Start: %s: Accumulate nearby spectra' % time.ctime(ProcStartTime))
                    BFlog0.close()

                    # 2009/2/7 add parameter SpWinRange to check whether it has pre-defined windows
                    #BF.Process2(SpStorage, DataTable, PosDictAll, basedata, LogLevel=LogLevel, LogFile=BFlog)
                    #BF.Process2(SpStorage, DataTable, PosDictAll, basedataid, LogLevel=LogLevel, LogFile=BFlog)
                    BF.Process2(SpStorage, DataTable, PosDictAll, basedataid, SpWinRange, LogLevel=LogLevel, LogFile=BFlog)
                    ProcEndTime = time.time()
                    BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                    SDT.LogMessage('LONG', LogLevel, BFlog0, Msg='SpStorage[2][0] after Proc2= %s' % SpStorage[2][0])
                    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process2 End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcStartTime), ProcEndTime - ProcStartTime) )
                    BFlog0.close()

                BFlog.close()

                # Prepare dummy scantable
                #scan=scantable(filename, average=True)
                sc=DP.RenumASAPData(filename)
                # 2009/2/11 spectrum have several different length
                #scan=sc.get_scan(0)
                scan=sc.get_scan(rows[0])
                del sc
                GridTable = []

                # 2009/2/7 Moved prior to Process1
                # for Pre-Defined Spectrum Window
                # SpectrumWindow format: [[CenterFreq(GHz), minVel(Km/s), maxVel(Km/s)],[CF,,],,,]
                # SpWin format: [CenterChannel, FullWidthChannel]
                # SpWinRange format: [channel0, channel1]
                #(SpWin, SpWinRange) = SDT.FreqWindow2Channel(Abcissa[vIF], SpectrumWindow)
                #if len(SpectrumWindow) != 0: iteration = 0
                if len(SpWinRange) != 0: iteration = 0

                for ITER in range(iteration + 1):
                    BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                    SDT.LogMessage('PROGRESS', LogLevel, BFlog0, Msg="Iteration Cycle (%d/%d)" % ((ITER + 1), (iteration + 1)))
                    BFlog0.close()


                    # Process3: detects all possible line features
                    #  scan: dummy scantable
                    #  Threshold: detection threshold
                    #  DetectSignal = {ID1,[RA,DEC,[[LineStrChan1, LineEndChan1],
                    #                               [LineStrChan2, LineEndChan2],
                    #                               [LineStrChanN, LineEndChanN]]],
                    #                  IDn,[RA,DEC,[[LineStrChan1, LineEndChan1],
                    #                               [LineStrChanN, LineEndChanN]]]}
                    if ITER == 0:
                        ResultTable = []
                        if History:
                            BK.countup()
                            if BK.Counter == ReStartID:
                                (SpStorage[0], SpStorage[1], SpStorage[2], DataTable, TimeTable, TimeGap, PosDict, Pattern) = BK.read(ReStartID, [0,1,2], 'DataTable', 'TimeTable', 'TimeGap', 'PosDict', 'Pattern')
                            elif BK.Counter > ReStartID:
                                BK.write('Process3:%d:%d:%d' % (vIF, vPOL, ITER), [2], [SpStorage[2]], DataTable=DataTable)
                        if History == False or BK.Counter >= ReStartID:
                            BFlog = open(LogDir+'/'+BF_DETECT_LOG, 'a')
                            ProcStartTime = time.time()
                            BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                            SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process3 Start: %s: Line detection' % time.ctime(ProcStartTime))
                            BFlog0.close()
                            DetectSignal = BF.Process3(SpStorage[2], scan, DataTable, ResultTable, PosDict, rows, SpWinRange, Threshold=3, edge=edge, BroadComponent=BroadComponent, LogLevel=LogLevel, LogFile=BFlog)
                            ProcEndTime = time.time()
                            BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                            SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process3 End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
                            SDT.LogMessage('DEBUG', LogLevel, BFlog0, Msg='DetectSignal = %s' % DetectSignal)
                            BFlog0.close()
                            BFlog.close()
                    else:
                        if History:
                            BK.countup()
                            if BK.Counter == ReStartID:
                                (SpStorage[0], SpStorage[1], SpStorage[2], NewSP, DataTable, TimeTable, TimeGap, ResultTable, PosDict, Pattern) = BK.read(ReStartID, [0,1,2,3], 'DataTable', 'TimeTable', 'TimeGap', 'ResultTable', 'PosDict', 'Pattern')
                            elif BK.Counter > ReStartID:
                                BK.write('Process3:%d:%d:%d' % (vIF, vPOL, ITER), [3], [NewSP], DataTable=DataTable, ResultTable=ResultTable)
                        if History == False or BK.Counter >= ReStartID:
                            BFlog = open(LogDir+'/'+BF_DETECT_LOG, 'a')
                            ProcStartTime = time.time()
                            BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                            SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process3 Start: %s: Line detection' % time.ctime(ProcStartTime))
                            BFlog0.close()
                            DetectSignal = BF.Process3(NewSP, scan, DataTable, ResultTable, PosDict, range(len(NewSP)), Threshold=3, edge=edge, BroadComponent=BroadComponent, LogLevel=LogLevel, LogFile=BFlog)
                            ProcEndTime = time.time()
                            BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                            SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process3 End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
                            SDT.LogMessage('DEBUG', LogLevel, BFlog0, Msg='DetectSignal = %s' % DetectSignal)
                            BFlog0.close()
                            BFlog.close()

                    # Process4: evaluate lines detected in the previous process by
                    #  the clustering-analysis.
                    if History:
                        BK.countup()
                        if BK.Counter == ReStartID:
                            if ITER == 0:
                                (SpStorage[0], SpStorage[1], SpStorage[2], DetectSignal, DataTable, TimeTable, TimeGap, PosDict, Pattern) = BK.read(ReStartID, [0,1,2], 'DetectSignal', 'DataTable', 'TimeTable', 'TimeGap', 'PosDict', 'Pattern')
                            else:
                                (SpStorage[0], SpStorage[1], SpStorage[2], DetectSignal, ResultTable, DataTable, TimeTable, PosDict, Pattern) = BK.read(ReStartID, [0,1,2], 'DetectSignal', 'ResultTable', 'DataTable', 'TimeTable', 'PosDict', 'Pattern')
                        elif BK.Counter > ReStartID:
                            BK.write('Process4:%d:%d:%d' % (vIF, vPOL, ITER), [], [], DetectSignal=DetectSignal, DataTable=DataTable)
                    if History == False or BK.Counter >= ReStartID:
                        BFlog = open(LogDir+'/'+BF_CLUSTER_LOG, 'a')
                        ProcStartTime = time.time()
                        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process4 Start: %s: Cluster analysis' % time.ctime(ProcStartTime))
                        BFlog0.close()
                        if SDT.PlotCheck(BPlotCluster, ITER, iteration) != False: FigFileDir = PlotDir+'/'+BF_CLUSTER_PDIR+'/'
                        else: FigFileDir = False
                        Lines = BF.Process4(DetectSignal, DataTable, ResultTable, GridTable, PosDict, Abcissa[vIF], rows, SpWin, Pattern, ClusterGridSize, ClusterGridSize, ITER, Nsigma=4.0, Xorder=-1, Yorder=-1, LogLevel=LogLevel, LogFile=BFlog, ShowPlot=SDT.PlotCheck(TPlotCluster, ITER, iteration), FigFileDir=FigFileDir, FigFileRoot='Clstr_%s_%s_%s' % (vIF, vPOL, ITER))
                        ProcEndTime = time.time()
                        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process4 End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime) )
                        BFlog0.close()
                        BFlog.close()

                    # for Clustering Test
                    #return(DetectSignal, DataTable, ResultTable, GridTable, PosDict, Abcissa[vIF], rows, SpWin, Pattern, radius*2.0)


                    # Process5: determin fitting order for polynomial baseline fit from
                    #  the dominated frequency determined by FFT
                    if History:
                        BK.countup()
                        if BK.Counter == ReStartID:
                            (SpStorage[0], SpStorage[1], SpStorage[2], Lines, DataTable, TimeTable, TimeGap, Pattern) = BK.read(ReStartID, [0,1,2], 'Lines', 'DataTable', 'TimeTable', 'TimeGap', 'Pattern')
                        elif BK.Counter > ReStartID:
                            BK.write('Process5:%d:%d:%d' % (vIF, vPOL, ITER), [], [], Lines=Lines, DataTable=DataTable)
                    if History == False or BK.Counter >= ReStartID:
                        BFlog = open(LogDir+'/'+BF_FITORDER_LOG, 'a')
                        ProcStartTime = time.time()
                        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process5 Start: %s: Fit order determination' % time.ctime(ProcStartTime))
                        BFlog0.close()
                        FitOrder = BF.Process5(SpStorage, DataTable, TimeTable, rows, edge=edge, LogLevel=LogLevel, LogFile=BFlog)
                        ProcEndTime = time.time()
                        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process5 End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
                        BFlog0.close()
                        BFlog.close()

                    # Process6: baseline fit by polynomial
                    if History:
                        BK.countup()
                        if BK.Counter == ReStartID:
                            (SpStorage[0], SpStorage[1], SpStorage[2], Lines, FitOrder, DataTable, TimeTable, TimeGap, Pattern) = BK.read(ReStartID, [0,1,2], 'Lines', 'FitOrder', 'DataTable', 'TimeTable', 'TimeGap', 'Pattern')
                        elif BK.Counter > ReStartID:
                            BK.write('Process6:%d:%d:%d' % (vIF, vPOL, ITER), [], [], FitOrder=FitOrder)
                    if History == False or BK.Counter >= ReStartID:
                        BFlog = open(LogDir+'/'+BF_FIT_LOG, 'a')
                        FitTableOut = outname+'.fit.tbl'
                        #FitTableOut = filename+'.fit.tbl'
                        ProcStartTime = time.time()
                        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process6 Start: %s: Baseline fitting' % time.ctime(ProcStartTime))
                        BFlog0.close()
                        if SDT.PlotCheck(BPlotFit, ITER, iteration) != False: FigFileDir = PlotDir+'/'+BF_FIT_PDIR+'/'
                        else: FigFileDir = False
                        BF.Process6(SpStorage, DataTable, rows, scan, FitOrder, Abcissa[vIF], TimeGap, showevery=100, savefile=FitTableOut, edge=edge, LogLevel=LogLevel, LogFile=BFlog, ShowRMS=SDT.PlotCheck(TPlotFlag, ITER, iteration), ShowPlot=SDT.PlotCheck(TPlotFit, ITER, iteration), FigFileDir=FigFileDir, FigFileRoot='Fit_%s_%s_%s' % (vIF, vPOL, ITER))
                        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                        ProcEndTime = time.time()
                        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process6 End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
                        BFlog0.close()
                        BFlog.close()
                   
                    # Process7: flagging by RMS (pre/post baseline, difference from running mean)
                    if History:
                        BK.countup()
                        if BK.Counter == ReStartID:
                            (SpStorage[0], SpStorage[1], SpStorage[2], Lines, DataTable, TimeTable, TimeGap, Pattern) = BK.read(ReStartID, [0,1,2], 'Lines', 'DataTable', 'TimeTable', 'TimeGap', 'Pattern')
                        elif BK.Counter > ReStartID:
                            BK.write('Process7:%d:%d:%d' % (vIF, vPOL, ITER), [1], [SpStorage[1]], DataTable=DataTable)
                    if History == False or BK.Counter >= ReStartID:
                        BFlog = open(LogDir+'/'+BF_STAT_LOG, 'a')
                        ProcStartTime = time.time()
                        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process7 Start: %s: Flagging' % time.ctime(ProcStartTime))
                        BFlog0.close()
                        if SDT.PlotCheck(BPlotFlag, ITER, iteration) != False: FigFileDir = PlotDir+'/'+BF_STAT_PDIR+'/'
                        else: FigFileDir = False
                        BF.Process7(SpStorage, DataTable, Abcissa[vIF], rows, TimeGap, TimeTable, Iteration=10, interactive=False, edge=edge, UserFlag=UserFlag, LogLevel=LogLevel, LogFile=BFlog, ShowPlot=SDT.PlotCheck(TPlotFlag, ITER, iteration), FigFileDir=FigFileDir, FigFileRoot='Stat_%s_%s_%s' % (vIF, vPOL, ITER))
                        ProcEndTime = time.time()
                        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process7 End: %s (Elapse Time=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
                        BFlog0.close()
                        BFlog.close()

                    if History:
                        BK.countup()
                        if BK.Counter == ReStartID:
                            (SpStorage[0], SpStorage[1], SpStorage[2], Lines, DataTable, Pattern) = BK.read(ReStartID, [0,1,2], 'Lines', 'DataTable', 'Pattern')
                        elif BK.Counter > ReStartID:
                            BK.write('Process8:%d:%d:%d' % (vIF, vPOL, ITER), [], [], DataTable=DataTable)
                    if History == False or BK.Counter >= ReStartID:
                        BFlog = open(LogDir+'/'+BF_GRID_LOG, 'a')
                        ProcStartTime = time.time()
                        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='GroupForGrid: Re-gridding Start: %s ' % time.ctime(ProcStartTime))
                        BFlog0.close()
                        # GroupForGrid: re-gridding
                        # 2008/09/24 extend the radius for the convolution to 2*hwhm
                        GridTable = BF.GroupForGrid(DataTable, rows, vIF, vPOL, radius*2.0, radius/10.0, spacing, Pattern, LogLevel=LogLevel, LogFile=BFlog)
                        #GridTable = BF.GroupForGrid(DataTable, rows, vIF, vPOL, radius, radius/10.0, spacing, Pattern, LogLevel=LogLevel, LogFile=BFlog)
                        ProcEndTime = time.time()
                        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='GroupForGrid End: %s (Elapse time = %.1f sec) ' % (time.ctime(ProcStartTime), (ProcEndTime - ProcStartTime)))
                        SDT.LogMessage('LONG', LogLevel, BFlog0, Msg='GridTable = %s' % GridTable)
                        BFlog0.close()

                    # Process8: Combine spectra for new grid
                        NewTable = outname+'.new.tbl'
                        #NewTable = filename+'.new.tbl'
                        if Pattern.upper() == 'SINGLE-POINT' or \
                           Pattern.upper() == 'MULTI-POINT':
                            weight = 'CONST'
                        else:
                            weight = 'GAUSS'
                        ProcStartTime = time.time()
                        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process8 Start: %s: Gridding' % time.ctime(ProcStartTime))
                        BFlog0.close()
                        (NewSP, ResultTable) = BF.Process8(SpStorage, DataTable, GridTable, radius, weight=weight, LogLevel=LogLevel, LogFile=BFlog)
                        ProcEndTime = time.time()
                        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Process8 End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
                        BFlog0.close()
                        BFlog.close()

                    # Show Result Spectra
                        ProcStartTime = time.time()
                        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='DrawMultiSpectra Start: %s' % time.ctime(ProcStartTime))
                        BFlog0.close()
                        if SDT.PlotCheck(BPlotMultiSP, ITER, iteration) != False: FigFileDir = PlotDir+'/'+BF_GRID_PDIR+'/'
                        else: FigFileDir = False
                        SDP.DrawMultiSpectra(NewSP, ResultTable, Abcissa[vIF], range(len(NewSP)), Pattern, chan0=edge[0], chan1=(len(NewSP[0]) - edge[1] - 1), ShowPlot=SDT.PlotCheck(TPlotMultiSP, ITER, iteration), FigFileDir=FigFileDir, FigFileRoot='Result_%s_%s_%s' % (vIF, vPOL, ITER))
                        ProcEndTime = time.time()
                        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                        SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='DrawMultiSpectra End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime) )
                        BFlog0.close()

                    # Draw Channel Maps for each Cluster
                        if Pattern.upper() == 'RASTER':
                            ProcStartTime = time.time()
                            BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                            SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='DrawImage Start: %s' % time.ctime(ProcStartTime))
                            BFlog0.close()
                            if SDT.PlotCheck(BPlotChannelMap, ITER, iteration) != False: FigFileDir = PlotDir+'/'+BF_CHANNELMAP_PDIR+'/'
                            else: FigFileDir = False
                            SDP.DrawImage(NewSP, ResultTable, Abcissa[vIF], Lines, radius, spacing, scale_max=False, scale_min=False, colormap=colormap, ShowPlot=SDT.PlotCheck(TPlotChannelMap, ITER, iteration), FigFileDir=FigFileDir, FigFileRoot='ChannelMap_%s_%s_%s' % (vIF, vPOL, ITER))
                            ProcEndTime = time.time()
                            BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                            SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='DrawImage End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
                            BFlog0.close()
                    # Show Sparse Spectra Map
                            ProcStartTime = time.time()
                            BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                            SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='DrawSparseSpectra Start: %s' % time.ctime(ProcStartTime))
                            BFlog0.close()
                            if SDT.PlotCheck(BPlotSparseSP, ITER, iteration) != False: FigFileDir = PlotDir+'/'+BF_SPARSESP_PDIR+'/'
                            else: FigFileDir = False
                            SDP.DrawSparseSpectra(NewSP, ResultTable, Abcissa[vIF], MaxPanels4SparseSP, chan0=edge[0], chan1=(len(NewSP[0]) - edge[1] - 1), ShowPlot=SDT.PlotCheck(TPlotSparseSP, ITER, iteration), FigFileDir=FigFileDir, FigFileRoot='SparseSpMap_%s_%s_%s' % (vIF, vPOL, ITER))
                            ProcEndTime = time.time()
                            BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                            SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='DrawSparseSpectra End: %s (ElapseTime=%.1f sec) ' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime) )
                            BFlog0.close()

                        Nline = len(ResultTable)
                        if Initial == 0:
                            Initial += 1
                            FinalSP = NewSP.copy()
                            FinalTable = ResultTable[:]
                        elif ITER == 0:
                            #FinalSP = NA.concatenate((FinalSP, NewSP))
                            FinalSP = NP.concatenate((FinalSP, NewSP))
                            for x in range(Nline):
                                FinalTable.append(ResultTable[x])
                        else:
                            FinalSP[-Nline:] = NewSP
                            FinalTable[-Nline:] = ResultTable

                        BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                        SDT.LogMessage('DEBUG', LogLevel, BFlog0, Msg='Len(SP), Len(Table) = %s, %s' % (len(FinalSP), len(FinalTable)))
                        BFlog0.close()
                    # Show All Result Spectra
                        if Pattern.upper() != 'RASTER':
                            ProcStartTime = time.time()
                            BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                            SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='DrawMultiSpectra Start: %s' % time.ctime(ProcStartTime))
                            BFlog0.close()
                            SDP.DrawMultiSpectra(FinalSP, FinalTable, Abcissa[vIF], range(len(FinalSP)), chan0=edge[0], chan1=(len(FinalSP[0]) - edge[1] - 1), ShowPlot=SDT.PlotCheck(TPlotMultiSP, ITER, iteration), FigFileDir=False, FigFileRoot=False)
                            ProcEndTime =  time.time()
                            BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                            SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='DrawMultiSpectra End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
                            BFlog0.close()

                BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
                SDT.LogMessage('PROGRESS', LogLevel, BFlog0, Msg="Iteration Cycle End")
                BFlog0.close()
    BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
    SDT.LogMessage('PROGRESS', LogLevel, BFlog0, Msg="Iteration over IF and POL End")
    BFlog0.close()


    # HTML Output
    BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'a')
    ProcStartTime = time.time()
    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='HTML Output Start: %s' % time.ctime(ProcStartTime))
    SDJ.SDjava2html(outname)
    #SDJ.SDjava2html(filename)
    ProcEndTime = time.time()
    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='HTML Output End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime) )


    # WriteMSData: Write baselined data to Disk
    # DataTable is not used yet. Flags, RMS, and etc should be added as sub-table.
    ProcStartTime = time.time()
    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='WriteMSData Start: %s' % time.ctime(ProcStartTime))
    FileBaselined = outname+'.baseline.MS'
    #FileBaselined = filename+'.baseline.MS'
    DP.WriteMSData(filename, FileBaselined, SpStorage, DataTable, outform='MS2', LogLevel=LogLevel, LogFile=BFlog0)
    ProcEndTime = time.time()
    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='WriteMSData End %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))

    #if ImageCube:
    if ImageCube and Pattern.upper() == 'RASTER':
        # CASA Imager output
        #radius; OK
        #iteration; OK
        #spacing; OK
        #Lines; OK
        #print 'Lines=', Lines
        # singleimage =True to make a single data cube for all the lines
        # willl use max channel width and ignores NChannelMap
        singleimage=False
        #singleimage=True
        (mapCenter, ngridx, ngridy, cellsize, startchans, nchans, chansteps)=SDI.getImParams(FileBaselined, Lines, radius, spacing, singleimage)
        cellx=cellsize
        celly=cellsize
        outImagename=outname+'.bs'
        field=0
        convsupport=3
        spw=0
        moments=[0]
        SDI.MakeImages(FileBaselined, outImagename, ngridx, ngridy, cellx, celly, mapCenter, nchans, startchans, chansteps,field, convsupport, spw, moments, showImage=False)
        SDI.MakeImage(FileBaselined, outImagename+'.image', ngridx, ngridy, cellx, celly, mapCenter, (nchan-edge[0]-edge[1]), edge[0], 1, field, convsupport, spw, moments, showImage=False)

    # WriteNewMSData: Write final output to Disk
    ProcStartTime = time.time()
    FileReGrid = outname+'.grid.MS'
    #FileReGrid = filename+'.grid.MS'
    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='WriteNewMSData Start: %s' % time.ctime(ProcStartTime))
    #DP.WriteNewMSData(FileBaselined, FileReGrid, FinalSP, FinalTable, outform='MS2', LogLevel=LogLevel, LogFile=BFlog0)
    DP.WriteNewMSData(filename, FileReGrid, FinalSP, FinalTable, outform='MS2', LogLevel=LogLevel, LogFile=BFlog0)
    ProcEndTime = time.time()
    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='WriteNewMSData End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime) )

    #if ImageCube:
    if ImageCube and Pattern.upper() == 'RASTER':
        # CASA Imager output
        #radius; OK
        #iteration; OK
        #spacing; OK
        #Lines; OK
        #print 'Lines=', Lines
        # singleimage =True to make a single data cube for all the lines
        # willl use max channel width and ignores NChannelMap
        singleimage=False
        #singleimage=True
        (mapCenter, ngridx, ngridy, cellsize, startchans, nchans, chansteps)=SDI.getImParams(FileReGrid, Lines, radius, spacing, singleimage)
        cellx=cellsize
        celly=cellsize
        outImagename=outname+'.gd'
        field=0
        convsupport=-1
        spw=0
        moments=[0]
        SDI.MakeImages(FileReGrid, outImagename, ngridx, ngridy, cellx, celly, mapCenter, nchans, startchans, chansteps,field, convsupport, spw, moments, showImage=False)
        SDI.MakeImage(FileReGrid, outImagename+'.image', ngridx, ngridy, cellx, celly, mapCenter, (nchan-edge[0]-edge[1]), edge[0], 1, field, convsupport, spw, moments, showImage=False)

    EndTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, BFlog0, Msg='Done: %s, Elapsed time = %.1f sec in total' % (time.ctime(EndTime), (EndTime - StartTime)))
    BFlog0.close()

    return (SpStorage, FinalSP, DataTable, FinalTable)


