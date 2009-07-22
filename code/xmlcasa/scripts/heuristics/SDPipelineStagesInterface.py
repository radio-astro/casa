#
# ALMA - Atacama Large Milliiter Array (c) European Southern Observatory, 2002
# Copyright by ESO (in the framework of the ALMA collaboration), All rights
# reserved
#
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2.1 of the License, or (at your option)
# any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA


from asap import *
import time
import datetime
import os
import pylab as PL
import numpy as NP
    
import SDDataPreparation as DP
import SDBaselineFit as BF
import SDTool as SDT
import SDPlotter as SDP
#import SDjava2html as SDJ
import SDHtmlProduct as SDH
import SDBookKeeper as SDB
import SDCalibration as SDC
# CASA Imager output
import SDImaging as SDI
import re

# 0:DebugPlot 1:TPlotRADEC 2:TPlotAzEl 3:TPlotCluster 4:TplotFit 5:TPlotMultiSP 6:TPlotSparseSP 7:TPlotChannelMap 8:TPlotFlag 9:TPlotIntermediate
MATPLOTLIB_FIGURE_ID = [8904, 8905, 8906, 8907, 8908, 8909, 8910, 8911, 8912, 8013]

MaxPanels4SparseSP = 8

BF_DATA_DIR = 'Summary'
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
BF_LOG_DIR = 'Logs'


class SDPipelineStagesInterface:
    def __init__(self,rawDir='default',outDir='default'):


        self.rawDir      = None
        self.outDir      = None
        self.workingDir  = None
        self.rawFile     = None
        self.workingFile = None
        self.outFile     = None
        self.HtmlDir     = None
        self.LogDir      = None

        if (rawDir != 'default'):
            self.rawDir = rawDir
        if (outDir != 'default'):
            self.outDir = outDir
            self.workingDir = outDir
            

        ### Default Plot control
        # Plot on terminal (either True, False, or 'Last')
        self.TPlotDebug = False
        self.TPlotRADEC = False
        self.TPlotAzEl = False
        self.TPlotCluster = False
        self.TPlotFit = False
        self.TPlotMultiSP = False
        self.TPlotSparseSP = False
        self.TPlotChannelMap = False
        self.TPlotFlag = False
        self.TPlotIntermediate = False
        
        # Plot on browser (either True, False, or 'Last')
        self.BPlotRADEC = True
        self.BPlotAzEl = True
        self.BPlotCluster = True
        self.BPlotFit = True
        self.BPlotMultiSP = True
        self.BPlotSparseSP = True
        self.BPlotChannelMap = True
        self.BPlotFlag = True

        self.colormap='color'       

        ### Default Flag rule
        import SDFlagRule
        reload(SDFlagRule)
        self.FlagRuleDictionary = SDFlagRule.SDFlagRule

        ### Default Clustering rule
        self.ClusterRuleDictionary = {'ThresholdValid': 0.5, \
                                      'ThresholdMarginal': 0.4, \
                                      'ThresholdQuestionable': 0.2, \
                                      'MaxCluster': 10, \
                                      'BlurRatio': 0.1}

        ### Default Gridding rule
        # Gridding['WeightDistance']: 'Gauss' | 'Linear' | 'Const'
        # Gridding['Clipping']: 'MinMaxReject' | 'none'
        # Gridding['WeightRMS']: True | False
        self.GridRuleDictionary = {'WeightDistance': 'Gauss', \
                                   'Clipping': 'MinMaxReject', \
                                   'WeightRMS': True, \
                                   'WeightTsysExptime': False} 
        
        ### Default FitOrder rule
        # ApplicableDuration: 'raster' | 'subscan'
        # MaxPolynomialOrder: 'none' | 1 | 2 | ...
        self.FitOrderRuleDictionary = {'ApplicableDuration': 'raster', \
                                       'MaxPolynomialOrder': 'none'}

        ### Log Filenames
        self.SD_PIPELINE_LOG = 'PIPELINE.txt'
        self.BF_DATA_LOG = 'Data.txt'
        self.BF_GROUP_LOG = 'BF_Grouping.txt'
        self.BF_DETECT_LOG = 'BF_DetectLine.txt'
        self.BF_CLUSTER_LOG = '/BF_Cluster.txt'
        self.BF_FITORDER_LOG = 'BF_FitOrder.txt'
        self.BF_FIT_LOG = 'BF_Fit.txt'
        self.BF_STAT_LOG = 'Flagger.txt'
        self.BF_GRID_LOG = 'Gridding.txt'
        self.CALIBRATION_LOG = 'Calibration.txt'

        # BookKeeper
        self.BK = SDB.SDbookkeeper('./.SDPipelineStagesInterface.History')
        
        self.ReStartID = 0
                
        import casac
        loghome =  casac.homefinder.find_home_by_name('logsinkHome')
        
        self.casalog = loghome.create()

        self.casalog.setglobal(True) 

 
        # internal processing parameters

        self.radius = 0.0025
        self.spacing = self.radius / 3.0 * 2.0
        self.clusterGridSize = self.spacing * 3.0        
        self.edge = []        
        self.broadComponent = False
        self.imageCube = True
        self.iteration = 0
        self.initial = 0

        self.LogLevel = 2
        self.History = False

        # internally stored data currently processed

        self.DataTable = None
        self.listall   = None
        self.SpStorage = None
        self.Abcissa   = None
        self.Pattern   = None
        self.TimeTable = None
        self.TimeGap   = None
        self.GridTable = None
        self.ResultTable = None
        self.NewSP = None
        self.PosDict = None
        self.PosDictAll = None
        self.scan = None
        self.Lines=[]

        self.SpectrumWindow = []
        self.ROW = []
        self.ROWbase = []
        self.SCAN = []
        self.SCANbase = []
        self.IF = []
        self.POL = []
        self.processedIFPOL = []

        PL.ion()
        if self.TPlotDebug != False: PL.figure(MATPLOTLIB_FIGURE_ID[0])
        if self.TPlotRADEC != False: PL.figure(MATPLOTLIB_FIGURE_ID[1])
        if self.TPlotAzEl != False: PL.figure(MATPLOTLIB_FIGURE_ID[2])
        if self.TPlotCluster != False: PL.figure(MATPLOTLIB_FIGURE_ID[3])
        if self.TPlotFit != False: PL.figure(MATPLOTLIB_FIGURE_ID[4])
        if self.TPlotMultiSP != False: PL.figure(MATPLOTLIB_FIGURE_ID[5])
        if self.TPlotSparseSP != False: PL.figure(MATPLOTLIB_FIGURE_ID[6])
        if self.TPlotChannelMap != False: PL.figure(MATPLOTLIB_FIGURE_ID[7])
        if self.TPlotFlag != False: PL.figure(MATPLOTLIB_FIGURE_ID[8])
        if self.TPlotIntermediate != False: PL.figure(MATPLOTLIB_FIGURE_ID[9])
        PL.ioff()

    def resetIntermediateData(self):
        self.Pattern   = None
        self.TimeTable = None
        self.TimeGap   = None
        self.GridTable = None
        self.ResultTable = None
        self.PosDict = None
        self.PosDictAll = None
        self.NewSP = None
        self.iteration = 0
        self.initial = 0

    def resetResultData(self):
        self.ResultTable = None

    def restoreFromHistory(self,historyDir):
        self.BK = SDB.SDbookkeeper(historyDir)        
        self.ReStartID = self.BK.Counter

    def showHistory(self):
        self.BK.show()
               
    def enableHistory(self):
        self.History = False

    def disableHistory(self):
        self.History = True

    def enableBookKeeping(self):
        #self.casalog.post('enableBookKeeping',origin='SDPipeline/Engines')
        print 'enableBookKeeping'

    def disableBookKeeping(self):
        #casalog.post('disableBookKeeping',orign='SDPipeline/Engines')
        print 'disableBookKeeping'

    def setBeamRadius(self,radius):
        self.radius=radius
        self.spacing = self.radius / 3.0 * 2.0
        
    def getBeamRadius(self):
        return self.radius

    def setBaselineEdge(self,edge):
        self.edge=edge

    def getBaselineEdge(self):
        return self.edge

    def setLogLevel(self,LogLevel):
        self.LogLevel=LogLevel

    def getLogLevel(self):
        return self.LogLevel

    def setTsysFlagThreshold(self,threshold):
        self.FlagRuleDictionary['TsysFlag']['Threshold']=threshold

    def setRmsPreFitFlagThreshold(self,threshold):
        self.FlagRuleDictionary['RmsPreFitFlag']['Threshold']=threshold

    def activateFlagRule(self,key):
        if(key in self.FlagRuleDictionary.keys()):
           self.FlagRuleDictionary[key]['isActive']=True
        else:
            print 'Error not in predefined Flagging Rules'

    def deactivateFlagRule(self,key):
        if(key in self.FlagRuleDictionary.keys()):
           self.FlagRuleDictionary[key]['isActive']=False
        else:
            print 'Error not in predefined Flagging Rules'

    def setFlagRuleThreshold(self,key,threshold):
        if(key in self.FlagRuleDictionary.keys()):
           self.FlagRuleDictionary[key]['Threshold']=Threshold
        else:
            print 'Error not in predefined Flagging Rules'
            
    def setRunMeanPreFitFlagThreshold(self,threshold):
        self.FlagRuleDictionary['RunMeanPreFitFlag']['Threshold']=threshold

    def showFlagRulesActive(self):
        for rules in self.FlagRuleDictionary.keys():
            try:
                if(self.FlagRuleDictionary[rules]['isActive']==True):
                    print"%s is active for flagging" % (rules)
            except:
                pass

    def setInteractiveMode(self,showPlots=1):
        if(showPlots==1):
            self.TPlotDebug = True
            self.TPlotRADEC = True
            self.TPlotAzEl = True
            self.TPlotCluster = True
            self.TPlotFit = True
            self.TPlotMultiSP = True
            self.TPlotSparseSP = True
            self.TPlotChannelMap = True
            self.TPlotFlag = True
            self.TPlotIntermediate = True
        else:
            self.TPlotDebug = False
            self.TPlotRADEC = False
            self.TPlotAzEl = False
            self.TPlotCluster = False
            self.TPlotFit = False
            self.TPlotMultiSP = False
            self.TPlotSparseSP = False
            self.TPlotChannelMap = False
            self.TPlotFlag = False
            self.TPlotIntermediate = False
            
        
    def setOutDirectory(self,outDir):

        """
        Sets the output directory. If none is set data is written to ./
        """
        
        self.outDir = outDir+'/'
        self.workingDir = self.outDir
        if not os.access(self.outDir, os.F_OK):
            os.mkdir(self.outDir)

    def getOutDirectory(self):
        return self.outDir        

    def setRawDirectory(self,rawDir):

        """
        Sets the input directory containing the raw ASAP file/directory. If none is set data is read from ./
        """        
 
        self.rawDir = rawDir+'/'        
        
    def getRawDirectory(self):
        return self.rawDir        

    def setRawFile(self,rawFile):

        """
        Sets the input raw ASAP file/directory in ./ respectively the input directory.
        """
        
        self.rawFile=rawFile

    def setOutFile(self,outFile):
        """
        Sets the output file/directory structure in ./ respectively the output directory.
        If this is not set a directory rawFile_out is created in ./ respectively the output directory.
        """
        self.outFile=outFile

    def getRawFile(self):
        return self.rawFile
    
    def getOutFile(self):
        return self.outFile
        
    
    def setBroadLineSpectralComponent(self,broadComponent=True):
        if((broadComponent==True)or(broadComponent==False)):
            self.broadComponent=broadComponent

    def getBroadLineSpectralomponent(self):
        return self.broadComponent

    def setImageCube(self,imageCube):
        if((imageCube==True)or(imageCube==False)):
            self.imageCube=imageCube

    def getIF(self):
        return self.IF
    
    def getPOL(self):
        return self.POL

    def setFlagRules(self):
        import SDFlagRule
        reload(SDFlagRule)
        self.FlagRuleDictionary = SDFlagRule.SDFlagRule

    def getFlagRules(self):
        return self.FlagRuleDictionary

    def showFlagRules(self):
        return self.FlagRuleDictionary.keys()

    def getGridRules(self):
        return self.GridRuleDictionary

    def showGridRules(self):
        return self.GridRuleDictionary.keys()

    def getClusterRules(self):
        return self.ClusterRuleDictionary

    def showClusterRules(self):
        return self.ClusterRuleDictionary.keys()

    def getFitOrderRules(self):
        return self.FitOrderRuleDictionary

    def showFitOrderRules(self):
        return self.FitOrderRuleDictionary.keys()

    def getFinalData(self):
         return (self.SpStorage, self.FinalSP, self.DataTable, self.FinalTable)

    def getImageCube(self):
        return self.broadComponent

    def calibrate(self):

        # if already calibrated, return
        filename=self.rawDir+self.rawFile
        self.workingFile = self.rawFile+'_cal'
        self.workingDir = self.outDir
        if ( os.path.exists( self.workingDir+self.workingFile ) ):
             # already calibrated
             return
        
        # log file
        logfile = self.outDir + '/' + self.CALIBRATION_LOG
        BFlog0 = open( logfile, 'a' )

        # Calibration using CASA single-dish tools (ASAP)
        #
        # -- 18/08/2008 Takeshi Nakazato
        
        ProcStartTime = time.time()
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='Calibration Start: %s' % time.ctime(ProcStartTime))
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='Calibration mode is ps')
        #casalog.post('Calibration Start: %s' % time.ctime(ProcStartTime),origin='SDPipeline/Engines')
        #casalog.post('Calibration mode is ps',origin='SDPipeline/Engines')

        # tool level processing using asap
        scal = SDC.SDCalibration( filename=filename, caltype='none', LogLevel=self.LogLevel, LogFile=BFlog0 )
        scal.summary()
        scal.calibrate()
        if ( not scal.isCalibrated() ):
            self.logger.logWarning('Data could not calibrated. Abort.',origin='SDPipeline/Engines')
            del scal
            return
        scal.average()
        scal.save(self.workingDir+self.workingFile)
        del scal
        if os.path.exists( filename+'.exp_rms_factors' ):
            os.system( 'ln -s '+filename+'.exp_rms_factors '+self.workingDir+self.workingFile+'.exp_rms_factors' )
        #casalog.post('Raw Data: %s' % filename,origin='SDPipeline/Engines' )
        #casalog.post('Calibrated Data: %s' % filename,origin='SDPipeline/Engines' )
        #casalog.post('Calibration End: %s' % time.ctime(ProcStartTime),origin='SDPipeline/Engines')
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='Raw Data: %s' % filename )
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='Calibrated Data: %s' % self.workingDir+self.workingFile )
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='Calibration End: %s' % time.ctime(ProcStartTime))
        BFlog0.close()


    def readData(self,IF='all',POL='all',ROW='all',ROWbase='all',SCAN='all',SCANbase='all'):

        """
        Extract Information from the raw file into a data tableand setup a directory structure
        in the output ditexctory in which the dataproducts and figures are written.
        
        Additional Parameters
        IF='all',
        POL='all',
        ROW='all',
        ROWbase='all',
        SCAN='all',
        SCANbase='all'
        to select individual scans. SCANbase: scans used only for selecting emission line channels
        SCANbase must be equal to or includes SCAN.

        """

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

        # Setup lists
        # setup input output directories and files
        

        if(self.outFile==None):
            self.outFile=self.rawFile+'_out'

        if(self.rawDir==None):
            self.rawDir='./'
        
        if(self.outDir==None):
            self.outDir='./'

        if os.path.exists( self.outDir+self.rawFile+'_cal' ):
            self.workingFile=self.rawFile+'_cal'
            self.workingDir = self.outDir
        else:
            os.system( 'cp -r '+self.rawDir+self.rawFile+' '+self.outDir)
            os.system( 'cp -r '+self.rawDir+self.rawFile+'.exp_rms_factors '+self.outDir)
            self.workingFile=self.rawFile
            self.workingDir=self.outDir
            
        # 2009/6/18 for html output
        self.HtmlDir = self.outDir+self.outFile+'_html'
        self.LogDir = self.outDir+self.outFile+'_html'+'/'+BF_LOG_DIR
            
        if os.access(self.HtmlDir, os.F_OK):
            for root, dirs, files in os.walk(self.HtmlDir, topdown=False):
                for name in files:
                    os.remove(os.path.join(root, name))
                for name in dirs:
                    os.rmdir(os.path.join(root, name))
        else: os.mkdir(self.HtmlDir)
        os.mkdir(self.HtmlDir+'/'+BF_LOG_DIR)
        os.mkdir(self.HtmlDir+'/'+BF_DATA_DIR)
        os.mkdir(self.HtmlDir+'/'+BF_CLUSTER_PDIR)
        os.mkdir(self.HtmlDir+'/'+BF_FIT_PDIR)
        os.mkdir(self.HtmlDir+'/'+BF_STAT_PDIR)
        os.mkdir(self.HtmlDir+'/'+BF_GRID_PDIR)
        os.mkdir(self.HtmlDir+'/'+BF_CHANNELMAP_PDIR)
        os.mkdir(self.HtmlDir+'/'+BF_SPARSESP_PDIR)
        if ( self.workingFile == self.rawFile+'_cal' ):
            os.system('cp '+self.workingDir+self.CALIBRATION_LOG+' '+self.HtmlDir+'/'+BF_LOG_DIR+'/'+self.CALIBRATION_LOG)
        SDH.HtmlInit(self.HtmlDir)
        

        ProcStartTime = time.time()
        BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='readData Start: %s' % time.ctime(ProcStartTime))

        TableOutFile=self.LogDir+'/' + self.BF_DATA_LOG
        #casalog.post('ReadData Start: %s' % time.ctime(ProcStartTime,origin='SDPipeline/Engines'))
        (DataTable, listall, SpStorage, Abcissa) = DP.ReadData(self.workingDir+self.workingFile,TableOutFile, self.HtmlDir+'/'+BF_DATA_DIR+'/index.html')

        # Setup lists
        if ROW == 'all': self.ROW = listall[1]
        if ROWbase == 'all': self.ROWbase = listall[0]
        if SCAN == 'all': self.SCAN = listall[2]
        if SCANbase == 'all': self.SCANbase = listall[2]
        if IF == 'all': self.IF = listall[3]
        if POL == 'all': self.POL = listall[4]

        self.DataTable=DataTable
        self.listall=listall
        self.SpStorage=SpStorage
        self.Abcissa=Abcissa
        self.iteration = 0
        self.initial = 0

        self.processedIFPOL = []
        
        # Prepare dummy scantable
        #scan=scantable(filename, average=True)
        sc=DP.RenumASAPData(self.workingDir+self.workingFile)
        self.scan=sc.get_scan(0)
        del sc
        self.GridTable = []

        ProcEndTime = time.time()
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='readData End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), (ProcEndTime-ProcStartTime)))
        BFlog0.close()
        #casalog.post('ReadData End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), (ProcEndTime-ProcStartTime)),origin='SDPipeline/Engines')

    def createPositionOverviewPlots(self):

        """
        this method creates RADEC and AzEl overview plots to show the observing pattern
        and time/position gaps in the processed observation set.
        """

        ProcStartTime = time.time()
        BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='createPositionOverviewPlots Start: %s' % time.ctime(ProcStartTime))

        (tmpdict, PosGap) = BF.GroupByPosition(self.DataTable, self.ROWbase, self.radius * 1.0, self.radius / 10.0)
        (tmptable, TimeGap) = BF.GroupByTime(self.DataTable, self.ROWbase, self.ROWbase)
        (tmptable, TimeGap) = BF.MergeGapTables(self.DataTable, TimeGap, tmptable, PosGap, self.LogLevel, BFlog0)
        #tmpPattern = BF.ObsPatternAnalysis(tmpdict, ROWbase)
        tmpPattern = BF.ObsPatternAnalysis(tmpdict, self.ROW)
        del tmpdict, tmptable
        
        if self.BPlotRADEC != False: figFile = self.HtmlDir+'/'+BF_DATA_DIR+'/RADEC.png'
        else: figFile = False

        #SDP.DrawRADEC(self.DataTable, ROW, connect=True, circle=[radius], ObsPattern=tmpPattern, ShowPlot=TPlotRADEC, FigFile=FigFile)
        SDP.DrawRADEC(self.DataTable, self.ROWbase, connect=True, circle=[self.radius], ObsPattern=tmpPattern, ShowPlot=self.TPlotRADEC, FigFile=figFile)
        if self.BPlotAzEl != False: figFile = self.HtmlDir+'/'+BF_DATA_DIR+'/AzEl.png'
        else: FigFile = False
        #SDP.DrawAzEl(self.DataTable, TimeGap, ROW, ShowPlot=TPlotAzEl, FigFile=FigFile)
        SDP.DrawAzEl(self.DataTable, TimeGap, self.ROWbase, ShowPlot=self.TPlotAzEl, FigFile=figFile)

        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='ObsPattern = %s' % tmpPattern)
        ProcEndTime = time.time()
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='createPositionOverviewPlots End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
        BFlog0.close()


    def writeMSDataCube(self,outFile='default'):

        """
        writeMSData: Write baselined data to disk.
        Additional Parameters
        outFile='default'
        it writes the data cube to the default file name taken from the raw file name or a
        user defined one given as a string here
        """


        # WriteMSData: Write baselined data to Disk
        # DataTable is not used yet. Flags, RMS, and etc should be added as sub-table.
        ProcStartTime = time.time()
        BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='writeMSDataCube Start: %s' % time.ctime(ProcStartTime))

        if(outFile!='default'):
            self.outFile=outFile

        fileBaselined = self.outDir+self.outFile+'.baseline.MS'

        DP.WriteMSData(self.workingDir+self.workingFile, fileBaselined, self.SpStorage, self.DataTable, outform='MS2', LogLevel=self.LogLevel, LogFile=BFlog0)
        nchan = self.DataTable[0][13]
        
        #if ImageCube:
        if self.imageCube and self.Pattern.upper() == 'RASTER':
            # CASA Imager output
            #radius; OK
            #iteration; OK
            #spacing; OK
            #Lines; OK
            #print 'Lines=', Lines
            # singleimage =True to make a single data cube for all the lines
            # will use max channel width and ignores NChannelMap
            singleimage=False
            #singleimage=True
            (mapCenter, ngridx, ngridy, cellsize, startchans, nchans, chansteps)=SDI.getImParams(fileBaselined, self.Lines, self.radius, self.spacing, singleimage)
            cellx=cellsize
            celly=cellsize
            outImagename=self.outDir+self.outFile+'.bs'
            field=0
            convsupport=3
            spw=0
            moments=[0]
            SDI.MakeImages(fileBaselined, outImagename, ngridx, ngridy, cellx, celly, mapCenter, nchans, startchans, chansteps,field, convsupport, spw, moments, showImage=False)
            SDI.MakeImage(fileBaselined, outImagename+'.image', ngridx, ngridy, cellx, celly, mapCenter, (nchan-self.edge[0]-self.edge[1]), self.edge[0], 1, field, convsupport, spw, moments, showImage=False)

        # WriteNewMSData: Write final output to Disk
        FileReGrid = self.outDir+self.outFile+'.grid.MS'
        
        DP.WriteNewMSData(self.workingDir+self.workingFile, FileReGrid, self.FinalSP, self.FinalTable, outform='MS2', LogLevel=self.LogLevel, LogFile=BFlog0)

        #casalog.post('WriteNewMSData End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime),origin='SDPipeline/Engines' )
        #if ImageCube:
        if self.imageCube and self.Pattern.upper() == 'RASTER':
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
            (mapCenter, ngridx, ngridy, cellsize, startchans, nchans, chansteps)=SDI.getImParams(FileReGrid, self.Lines, self.radius, self.spacing, singleimage)
            cellx=cellsize
            celly=cellsize
            outImagename=self.outDir+self.outFile+'.gd'
            field=0
            convsupport=-1
            spw=0
            moments=[0]
            SDI.MakeImages(FileReGrid, outImagename, ngridx, ngridy, cellx, celly, mapCenter, nchans, startchans, chansteps,field, convsupport, spw, moments, showImage=False)
            SDI.MakeImage(FileReGrid, outImagename+'.image', ngridx, ngridy, cellx, celly, mapCenter, (nchan-self.edge[0]-self.edge[1]), self.edge[0], 1, field, convsupport, spw, moments, showImage=False)

        ProcEndTime = time.time()
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='writeMSDataCube End %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
        BFlog0.close()

        
    def groupData(self,vIF,vPOL):

        """
        groupData : groups the data by position. It works for a given IF and Polarization.
        parameter : vIF, vPOL
        """

        # GroupByPosition: grouping the data by their positions.
        #  PosTable: output table consists of information i.e.,
        #   ID, IDs associated to the ID (within the radius)
        self.iteration = 0
        rows = []
        rowsid = []
        basedata = []
        basedataid = []
        for ID in self.listall[0]:
            if self.DataTable[ID][2] == vIF and \
                   self.DataTable[ID][3] == vPOL:
                if self.DataTable[ID][1] in self.SCANbase and \
                       ID in self.ROWbase:
                    basedata.append(self.DataTable[ID][0])
                    basedataid.append(ID)
                if self.DataTable[ID][1] in self.SCAN and \
                       ID in self.ROW:
                    rows.append(self.DataTable[ID][0])
                    rowsid.append(ID)
        if self.History:
            self.BK.countup()
            if self.BK.Counter ==self.ReStartID:
                (self.DataTable) = self.BK.read(ReStartID, [], 'DataTable')

        ProcStartTime = time.time()
        BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='groupData Start: %s' % time.ctime(ProcStartTime))
        #casalog.post('GroupByPostion Start: %s' % time.ctime(ProcStartTime),origin='SDPipeline/Engines')

        (self.PosDictAll, PosGap) = BF.GroupByPosition(self.DataTable, basedataid, self.radius * 1.0, self.radius / 10.0)

        (self.PosDict, PosGapTmp) = BF.GroupByPosition(self.DataTable, rowsid, self.radius * 1.0, self.radius / 10.0)

        #ProcEndTime = time.time()
        #casalog.post('GroupByPostion End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime),origin='SDPipeline/Engines')

        #ProcStartTime = time.time()
        #casalog.post('ObsPatternAnalysis Start: %s' % time.ctime(ProcStartTime),origin='SDPipeline/Engines',origin='SDPipeline/Engines')
        self.Pattern = BF.ObsPatternAnalysis(self.PosDictAll, basedata)
        #Pattern = BF.ObsPatternAnalysis(PosDict, rows, LogLevel=LogLevel, LogFile=BFlog)
        #ProcEndTime = time.time()
        #casalog.post('ObsPattern = %s' % self.Pattern,origin='SDPipeline/Engines')
        #casalog.post('ObsPatternAnalysis End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime),origin='SDPipeline/Engines')
                
        # GroupByTime: grouping the data by time lag to the next integration
        #ProcStartTime = time.time()

        #casalog.post('GroupByTime Start: %s' % time.ctime(ProcStartTime),origin='SDPipeline/Engines')

        (TimeTable, TimeGap) = BF.GroupByTime(self.DataTable, basedataid, rowsid)

        # Merge Gap Tables
        
        (self.TimeTable, self.TimeGap) = BF.MergeGapTables(self.DataTable, TimeGap, TimeTable, PosGap)

        if self.History: self.BK.write('groupData:%d:%d' % (vIF, vPOL), [0], [self.SpStorage[0]], DataTable=self.DataTable, PosDict=self.PosDict, PosDictAll=self.PosDictAll, TimeTable=self.TimeTable, TimeGap=self.TimeGap, Pattern=self.Pattern)

        ProcEndTime = time.time()
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='groupData End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
        BFlog0.close()
        #casalog.post('GroupByTime End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime),origin='SDPipeline/Engines')
                       

    def remove1stOrderBaseline(self,vIF,vPOL):

        """
        remove1stOrderBaseline: removes a tentative 1st order baseline via a reverse fft analysis.
        It works for a given IF and Polarization.
        parameter : vIF, vPOL
        
        the method utilizes Process1: tentative lower order baseline subtraction.
        """

        rows = []
        rowsid = []
        basedata = []
        basedataid = []
        for ID in self.listall[0]:
            if self.DataTable[ID][2] == vIF and \
                   self.DataTable[ID][3] == vPOL:
                if self.DataTable[ID][1] in self.SCANbase and \
                       ID in self.ROWbase:
                    basedata.append(self.DataTable[ID][0])
                    basedataid.append(ID)
                if self.DataTable[ID][1] in self.SCAN and \
                       ID in self.ROW:
                    rows.append(self.DataTable[ID][0])
                    rowsid.append(ID)
        
        if self.History:
            self.BK.countup()
            if self.BK.Counter ==self.ReStartID:
                (self.SpStorage[0], self.DataTable) = self.BK.read(ReStartID, [0], 'DataTable')

        ProcStartTime = time.time()
        BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
        BFlog = open(self.LogDir+'/'+self.BF_GROUP_LOG, 'a')
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='remove1stOrderBaseline Start: %s: DC and 1st order baseline gradient removal' % time.ctime(ProcStartTime))
        #casalog.post('Process1 Start: %s: DC and 1st order baseline gradient removal' % time.ctime(ProcStartTime),origin='SDPipeline/Engines')
                
        BF.Process1(self.SpStorage, self.DataTable,basedataid, FFTORDER=1, edge=self.edge, LogLevel=self.LogLevel, LogFile=BFlog)
        ProcEndTime = time.time()

        if self.History: self.BK.write('remove1stOrderBaseline:%d:%d' % (vIF, vPOL), [1], [self.SpStorage[1]], DataTable=self.DataTable)

        SDT.LogMessage('LONG', self.LogLevel, BFlog0, Msg='SpStorage[2][0] after the Process= %s' % self.SpStorage[2][0])
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='remove1stOrderBaseline End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
        BFlog0.close()
        BFlog.close()
        #casalog.post('SpStorage[2][0] after Proc1= %s' % self.SpStorage[2][0],origin='SDPipeline/Engines')
        #casalog.post('Process1 End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime),origin='SDPipeline/Engines')
                 
    def accumulateSpectra(self,vIF, vPOL):

        """
        accumulateSpectra: accumulates spectra within specified radius
        It works for a given IF and Polarization.

        parameter: vIF, vPOL

        the method utilizes Process2: accumulate spectra within specified radius
        centered at GroupByPosition

        """

        rows = []
        rowsid = []
        basedata = []
        basedataid = []
        for ID in self.listall[0]:
            if self.DataTable[ID][2] == vIF and \
                   self.DataTable[ID][3] == vPOL:
                if self.DataTable[ID][1] in self.SCANbase and \
                       ID in self.ROWbase:
                    basedata.append(self.DataTable[ID][0])
                    basedataid.append(ID)
                if self.DataTable[ID][1] in self.SCAN and \
                       ID in self.ROW:
                    rows.append(self.DataTable[ID][0])
                    rowsid.append(ID)

        # Process2: accumulate spectra within specified radius
        #  centered at GroupByPosition
        if self.History:
            self.BK.countup()
            if self.BK.Counter ==self.ReStartID:
                (self.SpStorage[0], self.SpStorage[1], self.DataTable, self.PosDictAll) = self.BK.read(ReStartID, [0,1], 'DataTable', 'PosDictAll')

        ProcStartTime = time.time()
        BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
        BFlog = open(self.LogDir+'/'+self.BF_GROUP_LOG, 'a')
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='accumulateSpectra Start: %s: Accumulate nearby spectra' % time.ctime(ProcStartTime))

        #casalog.post('Process2 Start: %s: Accumulate nearby spectra' % time.ctime(ProcStartTime),origin='SDPipeline/Engines')
        #BF.Process2(SpStorage, DataTable, PosDictAll, basedata, LogLevel=LogLevel, LogFile=BFlog)
        BF.Process2(self.SpStorage, self.DataTable, self.PosDictAll, basedataid, LogLevel=self.LogLevel, LogFile=BFlog)

        if self.History: self.BK.write('accumulateSpectra:%d:%d' % (vIF, vPOL), [2], [self.SpStorage[2]], DataTable=self.DataTable)

        ProcEndTime = time.time()
        SDT.LogMessage('LONG', self.LogLevel, BFlog0, Msg='SpStorage[2][0] after the process= %s' % self.SpStorage[2][0])
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='accumulateSpectra End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcStartTime), ProcEndTime - ProcStartTime) )
        BFlog0.close()
        BFlog.close()
        #casalog.post('SpStorage[2][0] after Proc2= %s' % self.SpStorage[2][0])
        #casalog.post('Process2 End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcStartTime), ProcEndTime - ProcStartTime),origin='SDPipeline/Engines')
                                
    def spectralLineDetect(self,vIF, vPOL):

        """
        accumulateSpectra: detects all possible line features in the selected spec 
        It works for a given IF and Polarization.

        parameter: vIF, vPOL
        the method utilizes Process3, which detects all possible line features

        """
        
        # Process3: detects all possible line features
        #  scan: dummy scantable
        #  Threshold: detection threshold
        #  DetectSignal = {ID1,[RA,DEC,[[LineStrChan1, LineEndChan1],
        #                               [LineStrChan2, LineEndChan2],
        #                               [LineStrChanN, LineEndChanN]]],
        #                  IDn,[RA,DEC,[[LineStrChan1, LineEndChan1],
        #                               [LineStrChanN, LineEndChanN]]]}

        self.iteration += 1
        rows = []
        rowsid = []
        basedata = []
        basedataid = []
        for ID in self.listall[0]:
            if self.DataTable[ID][2] == vIF and \
                   self.DataTable[ID][3] == vPOL:
                if self.DataTable[ID][1] in self.SCANbase and \
                       ID in self.ROWbase:
                    basedata.append(self.DataTable[ID][0])
                    basedataid.append(ID)
                if self.DataTable[ID][1] in self.SCAN and \
                       ID in self.ROW:
                    rows.append(self.DataTable[ID][0])
                    rowsid.append(ID)

        # for Pre-Defined Spectrum Window
        # SpectrumWindow format: [[CenterFreq(GHz), minVel(Km/s), maxVel(Km/s)],[CF,,],,,]
        # SpWin format: [CenterChannel, FullWidthChannel]
        # SpWinRange format: [channel0, channel1]
        (SpWin, SpWinRange) = SDT.FreqWindow2Channel(self.Abcissa[vIF], self.SpectrumWindow)

        if ( [vIF,vPOL] not in self.processedIFPOL ):
            self.ResultTable = None
            self.processedIFPOL.append( [vIF,vPOL] )
    
        if self.ResultTable == None:
            self.ResultTable = []
            if self.History:
                self.BK.countup()
                if self.BK.Counter ==self.ReStartID:
                    (self.SpStorage[0], self.SpStorage[1], self.SpStorage[2], self.DataTable, self.PosDict) = self.BK.read(ReStartID, [0,1,2], 'DataTable', 'PosDict')

            ProcStartTime = time.time()
            BFlog = open(self.LogDir+'/' + self.BF_DETECT_LOG, 'a')
            BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
            SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='spectralLineDetect Start: %s: Line detection' % time.ctime(ProcStartTime))
            #casalog.post('Process3 Start: %s: Line detection' % time.ctime(ProcStartTime),origin='SDPipeline/Engines')
            self.DetectSignal = BF.Process3(self.SpStorage[2], self.scan, self.DataTable, self.ResultTable, self.PosDict,rows, SpWinRange, Threshold=3, edge=self.edge, BroadComponent=self.broadComponent, LogLevel=self.LogLevel, LogFile=BFlog)
            ProcEndTime = time.time()
            SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='spectralLineDetect End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
            SDT.LogMessage('DEBUG', self.LogLevel, BFlog0, Msg='DetectSignal = %s' % self.DetectSignal)
            BFlog0.close()
            BFlog.close()
            #casalog.post('Process3 End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime),origin='SDPipeline/Engines')
            #casalog.post('DetectSignal = %s' % DetectSignal,origin='SDPipeline/Engines')
                
        else:
            if self.History:
                self.BK.countup()
                if self.BK.Counter ==self.ReStartID:
                    (self.SpStorage[0], self.SpStorage[1], self.SpStorage[2], self.NewSP, self.DataTable, self.ResultTable, self.PosDict) = self.BK.read(ReStartID, [0,1,2,3], 'DataTable', 'ResultTable', 'PosDict')
            
            ProcStartTime = time.time()
            BFlog = open(self.LogDir+'/' + self.BF_DETECT_LOG, 'a')
            BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
            SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='spectralLineDetect Start: %s: Line detection' % time.ctime(ProcStartTime))
            #casalog.post('Process3 Start: %s: Line detection' % time.ctime(ProcStartTime),origin='SDPipeline/Engines')
            self.DetectSignal = BF.Process3(self.NewSP, self.scan, self.DataTable, self.ResultTable, self.PosDict, range(len(self.NewSP)), Threshold=3, edge=self.edge, BroadComponent=self.broadComponent, LogLevel=self.LogLevel, LogFile=BFlog)

            if self.History: self.BK.write('spectralLineDetect:%d:%d' % (vIF, vPOL), [], [], DetectSignal=self.DetectSignal, DataTable=self.DataTable)
            ProcEndTime = time.time()
            SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='spectralLineDetect End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
            SDT.LogMessage('DEBUG', self.LogLevel, BFlog0, Msg='DetectSignal = %s' % self.DetectSignal)
            BFlog0.close()
            BFlog.close()
            #casalog.post('Process3 End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime),origin='SDPipeline/Engines')
            #casalog.post('DetectSignal = %s' % DetectSignal,origin='SDPipeline/Engines')
                        

    def spatialLineDetect(self,vIF, vPOL,createResultPlots=False):

        """
        spatialLineDetect: evaluate lines detected in the previous process by
        the clustering-analysis.
        It works for a given IF and Polarization.

        parameter: vIF, vPOL
        optional parameter: createResultPlots=False

        the method utilizes Process4

        """
        
        
        rows = []
        rowsid = []
        basedata = []
        basedataid = []
        for ID in self.listall[0]:
            if self.DataTable[ID][2] == vIF and \
                   self.DataTable[ID][3] == vPOL:
                if self.DataTable[ID][1] in self.SCANbase and \
                       ID in self.ROWbase:
                    basedata.append(self.DataTable[ID][0])
                    basedataid.append(ID)
                if self.DataTable[ID][1] in self.SCAN and \
                       ID in self.ROW:
                    rows.append(self.DataTable[ID][0])
                    rowsid.append(ID)

        # for Pre-Defined Spectrum Window
        # SpectrumWindow format: [[CenterFreq(GHz), minVel(Km/s), maxVel(Km/s)],[CF,,],,,]
        # SpWin format: [CenterChannel, FullWidthChannel]
        # SpWinRange format: [channel0, channel1]
        (SpWin, SpWinRange) = SDT.FreqWindow2Channel(self.Abcissa[vIF], self.SpectrumWindow)

        if self.History:
            self.BK.countup()
            if self.BK.Counter ==self.ReStartID:
                if self.DetectSignal==None:
                    (self.SpStorage[0], self.SpStorage[1], self.SpStorage[2], self.DetectSignal, self.DataTable, self.PosDict, self.Pattern) = self.BK.read(ReStartID, [0,1,2], 'DetectSignal', 'DataTable', 'PosDict', 'Pattern')
                else:
                    (self.SpStorage[0], self.SpStorage[1], self.SpStorage[2], self.DetectSignal, self.ResultTable, self.DataTable, self.PosDict,self.Pattern) = self.BK.read(ReStartID, [0,1,2], 'DetectSignal', 'ResultTable', 'DataTable', 'PosDict', 'Pattern')

        ProcStartTime = time.time()
        BFlog = open(self.LogDir+'/' + self.BF_CLUSTER_LOG, 'a')
        BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='spatialLineDetect Start: %s: Cluster analysis' % time.ctime(ProcStartTime))
        #casalog.post('Process4 Start: %s: Cluster analysis' % time.ctime(ProcStartTime),origin='SDPipeline/Engines')
             
        figFileDir = False
        if createResultPlots != False: figFileDir = self.HtmlDir+'/'+BF_CLUSTER_PDIR+'/'

        self.Lines = BF.Process4(self.DetectSignal, self.DataTable, self.ResultTable, self.GridTable, self.PosDict, self.Abcissa[vIF], rows, SpWin, self.Pattern, self.clusterGridSize, self.clusterGridSize,1, Nsigma=4.0, Xorder=-1, Yorder=-1, ClusterRule=self.ClusterRuleDictionary, ShowPlot=self.TPlotCluster, FigFileDir=figFileDir, FigFileRoot='Clstr_%s_%s_%s' % (vIF, vPOL,self.iteration), LogLevel=self.LogLevel, LogFile=BFlog)
        SDH.HtmlClustering(self.HtmlDir)
        if self.History: self.BK.write('spatialLineDetect:%d:%d' % (vIF, vPOL), [], [], Lines=self.Lines, DataTable=self.DataTable)
        ProcEndTime = time.time()
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='spatialLineDetect End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime) )
        BFlog0.close()
        BFlog.close()
        #casalog.post('Process4 End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime),origin='SDPipeline/Engines')


    def removeBaseline(self,vIF, vPOL,createResultPlots=False,fitOrder='automatic'):

        """
        removeBaseline: determin fitting order for polynomial baseline fit from
        the dominated frequency determined by FFT and remove a baseline using this fit order.
        Optional one can set the fit order using an optional parameter.
        
        evaluate lines detected in the previous process by
        the clustering-analysis.
        It works for a given IF and Polarization.

        parameter: vIF, vPOL
        optional parameter: createResultPlots=False
                            fitOrder='automatic' (otherwise it expects an integer)

        the method utilizes Process5 and Process6

        """
 
        rows = []
        rowsid = []
        basedata = []
        basedataid = []
        for ID in self.listall[0]:
            if self.DataTable[ID][2] == vIF and \
                   self.DataTable[ID][3] == vPOL:
                if self.DataTable[ID][1] in self.SCANbase and \
                       ID in self.ROWbase:
                    basedata.append(self.DataTable[ID][0])
                    basedataid.append(ID)
                if self.DataTable[ID][1] in self.SCAN and \
                       ID in self.ROW:
                    rows.append(self.DataTable[ID][0])
                    rowsid.append(ID)

        if self.History:
            self.BK.countup()
            if self.BK.Counter ==self.ReStartID:
                (SpStorage[0], SpStorage[1], SpStorage[2], self.Lines, self.DataTable, TimeTable) = self.BK.read(ReStartID, [0,1,2], 'Lines', 'DataTable', 'TimeTable')
        BFlog = open(self.LogDir+'/' + self.BF_FITORDER_LOG, 'a')
        BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
        ProcStartTime = time.time()
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='removeBaseline Start: %s: Fit order determination and baseline subtraction' % time.ctime(ProcStartTime))
        if(fitOrder!='automatic'):
            self.FitOrderRuleDictionary['MaxPolynomialOrder'] = fitOrder
            SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='fit order preset to %i' %(fitOrder))
        #casalog.post('determination of fit order  Start: %s: Fit order determination' % time.ctime(ProcStartTime),origin='SDPipeline/Engines')
        fitOrder = BF.Process5(self.SpStorage, self.DataTable, self.TimeTable, rows, edge=self.edge, FitOrderRule=self.FitOrderRuleDictionary, LogLevel=self.LogLevel, LogFile=BFlog)
        #print 'fitOrder calculated ',fitOrder,type(fitOrder)
        #ProcEndTime = time.time()
        #casalog.post('determination of fit order End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime),origin='SDPipeline/Engines')
        BFlog.close()
        
        FitTableOut = self.outDir+self.outFile+'.fit.tbl'
        #ProcStartTime = time.time()

        #casalog.post('Process6 Start: %s: Baseline fitting' % time.ctime(ProcStartTime),origin='SDPipeline/Engines')
            
        if createResultPlots != False: FigFileDir = self.HtmlDir+'/'+BF_FIT_PDIR+'/'
        else: FigFileDir = False
        BFlog = open(self.LogDir+'/' + self.BF_FIT_LOG, 'a')
        BF.Process6(self.SpStorage, self.DataTable, rows, self.scan, fitOrder, self.Abcissa[vIF], self.TimeGap, showevery=100, savefile=FitTableOut, edge=self.edge, FitOrderRule=self.FitOrderRuleDictionary, ShowRMS=self.TPlotIntermediate, ShowPlot=self.TPlotFit, DebugPlot=self.TPlotDebug, FigFileDir=FigFileDir, FigFileRoot='Fit_%s_%s_%s' % (vIF, vPOL,self.iteration), LogLevel=self.LogLevel, LogFile=BFlog)
        SDH.HtmlBaselineFit(self.HtmlDir)
        if self.History: self.BK.write('removeBaseline:%d:%d' % (vIF, vPOL), [1], [self.SpStorage[1]], DataTable=self.DataTable)            
        ProcEndTime = time.time()
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='removeBaseline End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
        BFlog0.close()
        BFlog.close()
        #casalog.post('Process6 End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime),origin='SDPipeline/Engines')
            
        
    def flagData(self,vIF, vPOL,UserFlag=[],createResultPlots=False,FlagRule=[]):

        """
        flagData: flagging by RMS (pre/post baseline, difference from running mean)
        Optional flagg parameters can be given as a list.
        It works for a given IF and Polarization.

        parameter: vIF, vPOL
        optional parameter: createResultPlots=False
                            UserFlag=[] list of optional flag parameters

        the method utilizes Process7

        """
        
        # Process7: flagging by RMS (pre/post baseline, difference from running mean)
        rows = []
        rowsid = []
        basedata = []
        basedataid = []
        for ID in self.listall[0]:
            if self.DataTable[ID][2] == vIF and \
                   self.DataTable[ID][3] == vPOL:
                if self.DataTable[ID][1] in self.SCANbase and \
                       ID in self.ROWbase:
                    basedata.append(self.DataTable[ID][0])
                    basedataid.append(ID)
                if self.DataTable[ID][1] in self.SCAN and \
                       ID in self.ROW:
                    rows.append(self.DataTable[ID][0])
                    rowsid.append(ID)

        if self.History:
            self.BK.countup()
            if self.BK.Counter ==self.ReStartID:
                (self.SpStorage[0], self.SpStorage[1], self.SpStorage[2], self.Lines, self.DataTable, self.TimeTable, self.TimeGap) = self.BK.read(ReStartID, [0,1,2], 'Lines', 'DataTable', 'TimeTable', 'TimeGap')
            
        ProcStartTime = time.time()
        BFlog = open(self.LogDir+'/' + self.BF_STAT_LOG, 'a')
        BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='flagData Start: %s: Flagging' % time.ctime(ProcStartTime))
        #casalog.post('Process7 Start: %s: Flagging' % time.ctime(ProcStartTime),origin='SDPipeline/Engines')
            
        figFileDir = False
        if createResultPlots != False: figFileDir = self.HtmlDir+'/'+BF_STAT_PDIR+'/'

        if len(FlagRule) == 0:
            self.setFlagRules()
        else:
            for item in self.showFlagRules():
                self.FlagRuleDictionary[item]['isActive'] = False
                for Flag in FlagRule:
                    if Flag.upper() in item.upper():
                        self.FlagRuleDictionary[item]['isActive'] = True

        BF.Process7(self.SpStorage, self.DataTable, self.Abcissa[vIF], rows, self.TimeGap, self.TimeTable, Iteration=self.iteration, edge=self.edge, UserFlag=UserFlag, FlagRule=self.FlagRuleDictionary, ShowPlot=self.TPlotFlag, FigFileDir=figFileDir, FigFileRoot='Stat_%s_%s_%s' % (vIF, vPOL,self.iteration), LogLevel=self.LogLevel, LogFile=BFlog)
        SDH.HtmlFitStatistics(self.HtmlDir)

        if self.History: self.BK.write('flagData:%d:%d' % (vIF, vPOL), [], [], DataTable=self.DataTable)
        ProcEndTime = time.time()
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='flagData End: %s (Elapse Time=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
        BFlog0.close()
        BFlog.close()
        #casalog.post('Process7 End: %s (Elapse Time=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime),origin='SDPipeline/Engines')
            

    def regridData(self,vIF, vPOL):

        """
        regridDataaccumulateSpectra: combinea spectra for new grid
        It works for a given IF and Polarization.

        parameter: vIF, vPOL
        the method utilizes Process8
        """
        rows = []
        rowsid = []
        basedata = []
        basedataid = []
        for ID in self.listall[0]:
            if self.DataTable[ID][2] == vIF and \
                   self.DataTable[ID][3] == vPOL:
                if self.DataTable[ID][1] in self.SCANbase and \
                       ID in self.ROWbase:
                    basedata.append(self.DataTable[ID][0])
                    basedataid.append(ID)
                if self.DataTable[ID][1] in self.SCAN and \
                       ID in self.ROW:
                    rows.append(self.DataTable[ID][0])
                    rowsid.append(ID)
       
        if self.History:
            self.BK.countup()
            if self.BK.Counter ==self.ReStartID:
                (self.SpStorage[0], self.SpStorage[1], self.SpStorage[2], self.Lines, self.DataTable, self.Pattern) = self.BK.read(ReStartID, [0,1,2], 'Lines', 'DataTable', 'Pattern')
            
        ProcStartTime = time.time()
        BFlog = open(self.LogDir+'/' + self.BF_GRID_LOG, 'a')
        BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='regridData Start: %s ' % time.ctime(ProcStartTime))
        #casalog.post('GroupForGrid: Re-gridding Start: %s ' % time.ctime(ProcStartTime,origin='SDPipeline/Engines'))
        # GroupForGrid: re-gridding
        # 2008/09/24 extend the radius for the convolution to 2*hwhm
        self.GridTable = BF.GroupForGrid(self.DataTable, rows, vIF, vPOL, self.radius*2.0, self.radius/10.0, self.spacing, self.Pattern, LogLevel=self.LogLevel, LogFile=BFlog)

        #ProcEndTime = time.time()
        #casalog.post('GroupForGrid End: %s (Elapse time = %.1f sec) ' % (time.ctime(ProcStartTime), (ProcEndTime - ProcStartTime)),origin='SDPipeline/Engines')
        #self.logger.logDebug('GridTable = %s' % self.GridTable)
                        
        # Process8: Combine spectra for new grid
        NewTable = self.outDir+self.outFile+'.new.tbl'
        #NewTable = filename+'.new.tbl'
        if self.Pattern.upper() == 'SINGLE-POINT' or \
               self.Pattern.upper() == 'MULTI-POINT':
            weight = 'CONST'
        else:
            weight = 'GAUSS'
        ProcStartTime = time.time()
        #casalog.post('Process8 Start: %s: Gridding' % time.ctime(ProcStartTime),origin='SDPipeline/Engines')
        (self.NewSP, self.ResultTable) = BF.Process8(self.SpStorage, self.DataTable, self.GridTable, self.radius, self.GridRuleDictionary, LogLevel=self.LogLevel, LogFile=BFlog)

        Nline = len(self.ResultTable)
        if self.initial == 0:
            self.initial += 1
            self.FinalSP = self.NewSP.copy()
            self.FinalTable = self.ResultTable[:]
        elif self.iteration == 0:
                
            #FinalSP = NA.concatenate((FinalSP, NewSP))
            self.FinalSP = NP.concatenate((self.FinalSP, self.NewSP))
            for x in range(Nline):
                self.FinalTable.append(self.ResultTable[x])
        else:
            self.FinalSP[-Nline:] = self.NewSP
            self.FinalTable[-Nline:] = self.ResultTable
        #self.iteration += 1

        ProcEndTime = time.time()
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='regridData End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
        BFlog0.close()
        BFlog.close()
        #casalog.post('Process8 End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime),origin='SDPipeline/Engines')


    def plotResultSpectra(self, vIF, vPOL):

        """
        plotResultSpectra: method creates plots which show the resulting Spectra after the conducted steps of reduction
        It works for a given IF and Polarization.

        parameter: vIF, vPOL
        """

        # Show Result Spectra
        ProcStartTime = time.time()
        BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='plotResultSpectra Start: %s' % time.ctime(ProcStartTime))
        
        FigFileDir = self.HtmlDir+'/'+BF_GRID_PDIR+'/'
        SDP.DrawMultiSpectra(self.NewSP, self.ResultTable, self.Abcissa[vIF], range(len(self.NewSP)), self.Pattern, chan0=self.edge[0], chan1=(len(self.NewSP[0]) - self.edge[1] - 1), ShowPlot=self.TPlotMultiSP, FigFileDir=FigFileDir, FigFileRoot='Result_%s_%s_%s' % (vIF, vPOL,self.iteration))
        SDH.HtmlMultiSpectra(self.HtmlDir)

        ProcEndTime = time.time()
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='plotResultSpectra End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime) )
        BFlog0.close()


    def plotChannelMaps(self, vIF, vPOL):

        """
        plotChannelMaps: method creates Channel Maps for each Cluster detected in prvious steps of the reduction
        It works for a given IF and Polarization.

        parameter: vIF, vPOL
        """

        # Draw Channel Maps for each Cluster
        ProcStartTime = time.time()
        BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='plotChannelMaps Start: %s' % time.ctime(ProcStartTime))

        if self.Pattern.upper() == 'RASTER':
            FigFileDir = self.HtmlDir+'/'+BF_CHANNELMAP_PDIR+'/'
            SDP.DrawImage(self.NewSP, self.ResultTable, self.Abcissa[vIF], self.Lines, self.radius, self.spacing, scale_max=False, scale_min=False, colormap=self.colormap, ShowPlot=self.TPlotChannelMap, FigFileDir=FigFileDir, FigFileRoot='ChannelMap_%s_%s_%s' % (vIF, vPOL,self.iteration))
            SDH.HtmlGridding(self.HtmlDir)

        ProcEndTime = time.time()
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='plotChannelMaps End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime) )
        BFlog0.close()


    def plotSparseSpectraMap(self, vIF, vPOL):

        """
        plotSparseSpectraMap: method creates Sparse Spectra Maps
        It works for a given IF and Polarization.

        parameter: vIF, vPOL
        """
        ProcStartTime = time.time()
        BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='plotSparseSpectraMap Start: %s' % time.ctime(ProcStartTime))

        FigFileDir = self.HtmlDir+'/'+BF_SPARSESP_PDIR+'/'
        SDP.DrawSparseSpectra(self.NewSP, self.ResultTable, self.Abcissa[vIF], MaxPanels4SparseSP, chan0=self.edge[0], chan1=(len(self.NewSP[0]) - self.edge[1] - 1), ShowPlot=self.TPlotSparseSP, FigFileDir=FigFileDir, FigFileRoot='SparseSpMap_%s_%s_%s' % (vIF, vPOL,self.iteration))
        SDH.HtmlSparseSpectraMap(self.HtmlDir)

        ProcEndTime = time.time()
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='plotSparseSpectraMap End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime) )
        BFlog0.close()


    def createHTML(self): 

        """
        createHTML: compiles the retrieved results as HTML pages for further inspection
        """
        ProcStartTime = time.time()
        BFlog0 = open(self.LogDir+'/' + self.SD_PIPELINE_LOG, 'a')
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='createHTML Start: %s' % time.ctime(ProcStartTime))

        SDJ.SDjava2html(self.outDir+self.outFile)

        ProcEndTime = time.time()
        SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='createHTML End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime) )
        BFlog0.close()

