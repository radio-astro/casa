from asap import *
import time
import os
import pylab as PL
import numarray as NA
    
import SDDataPreparation as DP
import SDBaselineFit as BF
import SDTool as SDT
import SDPlotter as SDP

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

#BF_DATA_DIR = 'BF_Data'
BF_DATA_DIR = 'measurementset'
#BF_CLUSTER_DIR = 'BF_Clstr'
BF_CLUSTER_DIR = 'measurementset'
#BF_FIT_DIR = 'BF_Fit'
BF_FIT_DIR = 'calibrater'
#BF_STAT_DIR = 'BF_Stat'
BF_STAT_DIR = 'flagger'
#BF_GRID_DIR = 'Gridding'
BF_GRID_DIR = 'imager'

#SD_PIPELINE_LOG = 'PIPELINE.log'
SD_PIPELINE_LOG = 'reducer.log'
#BF_DATA_LOG = 'Data.log'
BF_DATA_LOG = 'checkMS.log'
#BF_GROUP_LOG = 'BF_Grouping.log'
BF_GROUP_LOG = 'calibrater.log'
#BF_DETECT_LOG = 'BF_DetectLine.log'
BF_DETECT_LOG = 'plotter.log'
#BF_CLUSTER_LOG = '/BF_Cluster.log'
BF_CLUSTER_LOG = 'plotter.log'
#BF_FITORDER_LOG = 'BF_FitOrder.log'
BF_FITORDER_LOG = 'plotter.log'
#BF_FIT_LOG = 'BF_Fit.log'
BF_FIT_LOG = 'plotter.log'
#BF_STAT_LOG = 'Flagger.log'
BF_STAT_LOG = 'flagger.log'
#BF_GRID_LOG = 'Gridding.log'
BF_GRID_LOG = 'imager.log'

def SDpipeline(filename, IF='all', POL='all', SCAN='all', SCANbase='all', ROW='all', ROWbase='all', ShowSpectrum=False, radius=0.0025, spacing=0.00125, edge=(0, 0), BroadComponent=False, iteration=0, LogLevel=2):
    StartTime = time.time()
    # radius: beam size in degree

    # Setup LogFile
    LogDir = filename+'.logs'
    PlotDir = filename+'.plots'
    SummaryDir = filename+'.summary'
    # Rename Old Log Directories
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
    os.mkdir(PlotDir+'/'+BF_CLUSTER_DIR)
    os.mkdir(PlotDir+'/'+BF_FIT_DIR)
    os.mkdir(PlotDir+'/'+BF_STAT_DIR)
    os.mkdir(PlotDir+'/'+BF_GRID_DIR)
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

    # Extract Information to make DataTable file & Dictionary
    # Spectra selected by IF/POL/SCAN/ROW are processed
    # SCANbase: scans used only for selecting emission line channels
    #         SCANbase must be equal to or includes SCAN
    #
    # DataTable[ID] = [Row,Scan,IF,Pol,Beam,Date,MJD,ElapsedTime,
    #                  ExpTime,RA,Dec,Az,El,Nchan,Tsys,TargetName,
    #                  LowFreqPower, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff,
    #                  flag1, flag2, flag3, flag4, Nmask]
    #                 [  0,   1, 2,  3,   4,   5,  6,          7,
    #                        8, 9, 10,11,12,   13,  14,        15,
    #                            16,     17,     18,         19,         20,
    #                     21,    22,    23,    24,    25]
    # listall = [[IDs], [ROWs], [SCANs], [IFs], [POLs], [BEAMs]]
    #           [    0,      1,       2,     3,      4,       5]
    (DataTable, listall, SpStorage) = DP.ReadData(filename, LogDir+'/'+BF_DATA_LOG, SummaryDir+'/'+BF_DATA_DIR+'/index.html')

    # Setup lists
    if ROW == 'all': ROW = listall[1]
    if ROWbase == 'all': ROWbase = listall[1]
    if SCAN == 'all': SCAN = listall[2]
    if SCANbase == 'all': SCANbase = listall[2]
    if IF == 'all': IF = listall[3]
    if POL == 'all': POL = listall[4]

    PL.ion()
    if LogLevel > 3:
        PL.figure(MATPLOTLIB_FIGURE_ID_3)
    PL.figure(MATPLOTLIB_FIGURE_ID_4)
    PL.figure(MATPLOTLIB_FIGURE_ID_5)
    PL.figure(MATPLOTLIB_FIGURE_ID_2)
    PL.figure(MATPLOTLIB_FIGURE_ID_1)
    PL.ioff()

    (tmptable, TimeGap) = BF.GroupByTime(DataTable, ROWbase, ROWbase)
    (tmpdict, PosGap) = BF.GroupByPosition(DataTable, ROWbase, radius * 1.0, radius / 10.0)
    del tmpdict, tmptable
    SDP.DrawRADEC(DataTable, ROW, connect=True, circle=[radius], FigFile=SummaryDir+'/'+BF_DATA_DIR+'/RADEC.png')
    SDP.DrawAzEl(DataTable, TimeGap, PosGap, ROW, FigFile=SummaryDir+'/'+BF_DATA_DIR+'/AzEl.png')

    Initial = 0
    BFlog0 = open(LogDir+'/' + SD_PIPELINE_LOG, 'w')
    for vIF in IF:
        for vPOL in POL:
            # rows: Final output will be calculated for list of rows
            # basedata: used only for selecting emission line channels
            rows = []
            basedata = []
            for ID in listall[0]:
                if DataTable[ID][2] == vIF and \
                   DataTable[ID][3] == vPOL:
                    if DataTable[ID][1] in SCANbase and \
                       ID in ROWbase:
                            basedata.append(DataTable[ID][0])
                    if DataTable[ID][1] in SCAN and \
                       ID in ROW:
                            rows.append(DataTable[ID][0])
            # Do Pipeline for selected IF & POL
            if len(rows) > 0:
                SDT.LogMessage('PROGRESS', LogLevel, BFlog0, Msg='IF = %s  POL = %s' % (vIF, vPOL))
                SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='nrows = %s  nbasedata = %s' % (len(rows), len(basedata)))
                SDT.LogMessage('LONG', LogLevel, BFlog0, Msg='rows = %s' % rows)

                BFlog = open(LogDir+'/'+BF_GROUP_LOG, 'a')

                # GroupByPosition: grouping the data by their positions.
                #  PosTable: output table consists of information i.e.,
                #   ID, IDs associated to the ID (within the radius)
                (PosDict, PosGap) = BF.GroupByPosition(DataTable, basedata, radius * 1.0, radius / 10.0, LogLevel=LogLevel, LogFile=BFlog)
                SDT.LogMessage('LONG', LogLevel, BFlog0, Msg='PosDict = %s' % PosDict)
                Pattern = BF.ObsPatternAnalysis(PosDict, basedata, LogLevel=LogLevel, LogFile=BFlog)
                SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='ObsPattern = %s' % Pattern)

                # GroupByTime: grouping the data by time lag to the next integration
                (TimeTable, TimeGap) = BF.GroupByTime(DataTable, basedata, rows, LogLevel=LogLevel, LogFile=BFlog)

                # Process1: tentative lower order baseline subtraction for
                #  tentative accumuration of spectra in the vicinity of each
                #  position on the sky.
                BF.Process1(SpStorage, DataTable, basedata, FFTORDER=1, edge=edge, LogLevel=LogLevel, LogFile=BFlog)

                # Process2: accumurate spectra within specified radius
                #  centered at GroupByPosition
                BF.Process2(SpStorage, DataTable, PosDict, basedata, LogLevel=LogLevel, LogFile=BFlog)

                BFlog.close()

                # Prepare dummy scantable
                #scan=scantable(filename, average=True)
                sc=DP.RenumASAPData(filename)
                scan=sc.get_scan(0)
                del sc
                GridTable = []

                for ITER in range(iteration + 1):
                    SDT.LogMessage('PROGRESS', LogLevel, BFlog0, Msg="Iteration Cycle (%d/%d)" % ((ITER + 1), (iteration + 1)))

                    BFlog = open(LogDir+'/'+BF_DETECT_LOG, 'a')

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
                        DetectSignal = BF.Process3(SpStorage[2], scan, DataTable, ResultTable, PosDict, rows, Threshold=3, edge=edge, BroadComponent=BroadComponent, LogLevel=LogLevel, LogFile=BFlog)
                    else:
                        DetectSignalOrg = DetectSignal.copy()
                        DetectSignal = BF.Process3(NewSP, scan, DataTable, ResultTable, PosDict, range(len(NewSP)), Threshold=3, edge=edge, BroadComponent=BroadComponent, LogLevel=LogLevel, LogFile=BFlog)
                        if Pattern.upper() == 'SINGLE-POINT' and \
                           DetectSignalOrg.values()[0][2] == DetectSignal.values()[0][2]:
                            SDT.LogMessage('INFO', LogLevel, BFlog0, Msg="Line Detection Converged")
                            break
                    SDT.LogMessage('DEBUG', LogLevel, BFlog0, Msg='DetectSignal = %s' % DetectSignal)

                    BFlog.close()
                    BFlog = open(LogDir+'/'+BF_CLUSTER_LOG, 'a')

                    # Process4: evaluate lines detected in the previous process by
                    #  the clustering-analysis.
                    #  access TableIn to get position information
                    #  RealSignal = {ID1,[RA,DEC,[[LineStrChan1, LineEndChan1],
                    #                             [LineStrChan2, LineEndChan2],
                    #                             [LineStrChanN, LineEndChanN]]],
                    #                IDn,[RA,DEC,[[LineStrChan1, LineEndChan1],
                    #                             [LineStrChanN, LineEndChanN]]]}
                    (RealSignal, Lines) = BF.Process4(DetectSignal, DataTable, ResultTable, GridTable, PosDict, rows, Pattern, radius, radius, Nsigma=3.0, Xorder=-1, Yorder=-1, LogLevel=3, LogFile=BFlog, FigFileDir=PlotDir+'/'+BF_CLUSTER_DIR+'/', FigFileRoot='Clstr_%s_%s_%s' % (ITER, vIF, vPOL))
                    SDT.LogMessage('DEBUG', LogLevel, BFlog0, Msg='RealSignal = %s' % RealSignal)

                    BFlog.close()
                    BFlog = open(LogDir+'/'+BF_FITORDER_LOG, 'a')

                    # Process5: determin fitting order for polynomial baseline fit from
                    #  the dominated frequency determined by FFT
                    FitOrder = BF.Process5(SpStorage, TimeTable, rows, RealSignal, edge=edge, LogLevel=LogLevel, LogFile=BFlog)

                    BFlog.close()
                    BFlog = open(LogDir+'/'+BF_FIT_LOG, 'a')

                    # Process6: baseline fit by polynomial
                    FitTableOut = filename+'.fit.tbl'
                   
                    BF.Process6(SpStorage, DataTable, rows, scan, FitOrder, RealSignal, TimeGap, PosGap, showplot=True, showevery=100, showspectrum=ShowSpectrum, savefile=FitTableOut, edge=edge, LogLevel=LogLevel, LogFile=BFlog, FigFileDir=PlotDir+'/'+BF_FIT_DIR+'/', FigFileRoot='Fit_%s_%s_%s' % (ITER, vIF, vPOL))

                    BFlog.close()
                    BFlog = open(LogDir+'/'+BF_STAT_LOG, 'a')
                   
                    # Process7: flagging by RMS (pre/post baseline, difference from running mean)
                    BF.Process7(SpStorage, DataTable, rows, RealSignal,  TimeGap, PosGap,Nmean=5, ThreNewRMS=4.0, ThreOldRMS=4.5, ThreNewDiff=5.5, ThreOldDiff=6.0, Iteration=10, showplot=True, interactive=False, edge=edge, LogLevel=LogLevel, LogFile=BFlog, FigFileDir=PlotDir+'/'+BF_STAT_DIR+'/', FigFileRoot='Stat_%s_%s_%s' % (ITER, vIF, vPOL))

                    BFlog.close()
                    BFlog = open(LogDir+'/'+BF_GRID_LOG, 'a')

                    # GroupForGrid: re-gridding
                    GridTable = BF.GroupForGrid(DataTable, rows, vIF, vPOL, radius, radius/10.0, spacing, Pattern, LogLevel=LogLevel, LogFile=BFlog)
                    SDT.LogMessage('LONG', LogLevel, BFlog0, Msg='GridTable = %s' % GridTable)

                    # Process8: Combine spectra for new grid
                    NewTable = filename+'.new.tbl'
                    if Pattern.upper() == 'SINGLE-POINT' or \
                       Pattern.upper() == 'MULTI-POINT':
                        weight = 'CONST'
                    else:
                        weight = 'GAUSS'
                    (NewSP, ResultTable) = BF.Process8(SpStorage, DataTable, GridTable, radius, weight=weight, clip='minmaxrej', rms_weight=True, LogLevel=LogLevel, LogFile=BFlog)

                    BFlog.close()

                    # Show Result Spectra
                    SDP.DrawMultiSpectra(NewSP, ResultTable, range(len(NewSP)), Pattern, chan0=edge[0], chan1=(len(NewSP[0]) - edge[1] - 1), FigFileDir=PlotDir+'/'+BF_GRID_DIR+'/', FigFileRoot='Result_%s_%s_%s' % (ITER, vIF, vPOL))

                    # Draw Channel Maps for each Cluster
                    if Pattern.upper() == 'RASTER':
                        SDP.DrawImage(NewSP, ResultTable, Lines, radius, spacing, scale_max=False, scale_min=False, FigFileDir=PlotDir+'/'+BF_GRID_DIR+'/', FigFileRoot='ChannelMap_%s_%s_%s' % (ITER, vIF, vPOL))

                    Nline = len(ResultTable)
                    if Initial == 0:
                        Initial += 1
                        FinalSP = NewSP.copy()
                        FinalTable = ResultTable[:]
                    elif ITER == 0:
                        FinalSP = NA.concatenate((FinalSP, NewSP))
                        for x in range(Nline):
                            FinalTable.append(ResultTable[x])
                    else:
                        FinalSP[-Nline:] = NewSP
                        FinalTable[-Nline:] = ResultTable

                    SDT.LogMessage('INFO', LogLevel, BFlog0, Msg='Len(SP), Len(Table) = %s, %s' % (len(FinalSP), len(FinalTable)))
                    # Show All Result Spectra
                    if Pattern.upper() != 'RASTER':
                        SDP.DrawMultiSpectra(FinalSP, FinalTable, range(len(FinalSP)), chan0=edge[0], chan1=(len(FinalSP[0]) - edge[1] - 1), FigFileDir=False, FigFileRoot=False)

    # WriteMSData: Write baselined data to Disk
    # DataTable is not used yet. Flags, RMS, and etc should be added as sub-table.
    FileBaselined = filename+'.baseline.MS'
    DP.WriteMSData(filename, FileBaselined, SpStorage, DataTable, LogLevel=LogLevel, LogFile=BFlog0)

    # WriteNewMSData: Write final output to Disk
    FileReGrid = filename+'.grid.MS'
    DP.WriteNewMSData(filename, FileReGrid, FinalSP, FinalTable, LogLevel=LogLevel, LogFile=BFlog0)

    EndTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, BFlog0, Msg='Done: Elapsed time = %.1f sec in total' % (EndTime - StartTime))
    BFlog0.close()

    return (SpStorage, FinalSP, DataTable, FinalTable)


