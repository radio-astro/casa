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

#
# $Revision: 1.108.2.4.2.11 $
# $Date: 2012/09/27 12:32:07 $
# $Author: tnakazat $
#

from asap import *
import time
import datetime
import os
import pylab as PL
import numpy as NP
import pickle
#import casac
from taskinit import gentools
    
#import SDDataPreparation as DP
#import SDBaselineFit as BF
## import SDTool as SDT
import SDPlotter as SDP
import SDHtmlProduct as SDH
import SDCalibration as SDC
import SDDataProductName as SDN
# CASA Imager output
#import SDImaging as SDI
import re
from SDTool import SDLogger, osfdatautils, is_scantable, is_ms, dec_stages_interface_logging, dec_stages_interface_logfile, getHeuristicsVersion, getSummary, beamsize
from SDEngine import SDEngine
import filltsys

#from pipeline.heuristics import SingleDishBeamSizeFromName
from pipeline.hsd.heuristics import SingleDishBeamSizeFromName

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


class SDPipelineStagesInterface( SDLogger ):
    """
    Interface class for single-dish pipeline heuristics.
    """
    def __init__(self,rawDir='default',outDir='default',LogLevel=2,ConsoleLevel=3,LogFile='casapy.log',htmlDir=None):

        # Start time (=the time when the instance is created)
        self.startTime = time.gmtime()

        # status of the process
        self.status = ['Succeeded','green']

        # local tool
        #self._tbtool = casac.homefinder.find_home_by_name('tableHome')
        #self._tb = self._tbtool.create()
        self._tb = gentools(['tb'])[0]

        self.rawDir      = None
        self.outDir      = None
        self.workDir     = None
        self.rawFile     = None
        self.rawFileList = None
        self.rawFileIdx  = -1
        self.outFile     = None
        self.workFile    = None
        self.contFile    = None
        self.HtmlDir     = None
        self.HtmlTopDir  = None
        self.frontPage   = None
        self.profilePage = None
        self.LogDir      = None
        self.GlobalLogDir = None
        self.initData    = None
        self.dtctData    = None
        self.contData    = None
        self.lastData    = None
        self.dataFormat  = None
        self.antFile     = {}
        self.outFiles    = {}
        self.contFiles   = {}
        self.gridInfo    = {}
        self.outGridComb = {}
        self.contGridComb = {}

        # parameter dictionary
        self.paramdic = {}

        # engine
        self.engine = SDEngine()

        # logger setup
        SDLogger.__init__( self, level=LogLevel, consolelevel=ConsoleLevel, origin='SDPipeline', Global=True )

        # number of antenna loop
        self.numAnt = 0
        
        # antenna name
        self.antName = {}

        # data product name
        self.productName = None

        # for WVR data
        self.WVRScan = None
        #self.WVRFile = None
        self.WVRFile = {}

        if (rawDir != 'default'):
            self.rawDir = os.path.abspath(rawDir)+'/'
        if (outDir != 'default'):
            self.outDir = os.path.abspath(outDir)+'/'
            self.workDir = self.outDir
        if htmlDir is not None:
            self.HtmlTopDir = os.path.abspath(htmlDir)+'/'
            
        ### Default Plot control
        # Plot on terminal (either True, False, or 'Last')
        self.TPlotDebug = False
        self.TPlotRADEC = False
        self.TPlotAzEl = False
        self.TPlotWeather = False
        self.TPlotWVR = False
        self.TPlotCluster = False
        self.TPlotFit = False
        self.TPlotMultiSP = False
        self.TPlotSparseSP = False
        self.TPlotChannelMap = False
        self.TPlotFlag = False
        self.TPlotIntermediate = False
        self.addparam(interactive=False)
        
        # Plot on browser (either True, False, or 'Last')
        self.BPlotRADEC = True
        self.BPlotAzEl = True
        self.BPlotWeather = True
        self.BPlotWVR = True
        self.BPlotCluster = True
        self.BPlotFit = True
        #self.BPlotFit = False
        self.BPlotMultiSP = True
        #self.BPlotMultiSP = False
        self.BPlotSparseSP = True
        self.BPlotChannelMap = True
        self.BPlotFlag = True

        self.colormap='color'       

        ### Default Flag rule
        #import SDFlagRule
        from pipeline.hsd.tasks.flagdata import SDFlagRule
        reload(SDFlagRule)
        self.FlagRuleDictionary = SDFlagRule.SDFlagRule

        self.addflagparam()

        ### Default Clustering rule
        ### 2011/05/16 Marginal 0.4 -> 0.35 to save more lines
        ### 2011/05/16 BlurRatio 0.1 -> 0.3
        self.ClusterRuleDictionary = {'ThresholdValid': 0.5, \
                                      'ThresholdMarginal': 0.35, \
                                      'ThresholdQuestionable': 0.2, \
                                      'MaxCluster': 100, \
                                      'BlurRatio': 0.3}

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
        # PolynomialOrder: 'automatic' | 1 | 2 | ...
        self.FitOrderRuleDictionary = {'ApplicableDuration': 'raster', \
                                       'MaxPolynomialOrder': 'none', \
                                       'PolynomialOrder': 'automatic'}

        ### Default Line Finder rule
        self.LineFinderRuleDictionary = {'MaxFWHM': 900, \
                                         'MinFWHM': 5, \
                                         #'MinFWHM': 3, \
                                         'Threshold': 3}
                                         #'Threshold': 2.5}
                                         #'Threshold': 2}

        ### Default Switching rule
        # CalibrationFormula: 'On-Off' | 'On-Off/Off'
        self.SwitchingRuleDictionary = {'CalibrationFormula': 'On-Off', \
                                        'NumOffSpectra': 3}
        #self.SwitchingRuleDictionary = {'CalibrationFormula': 'On-Off/Off'}

        ### Log Filenames
        self.SD_PIPELINE_LOG = 'PIPELINE.txt'
        self.SD_RECIPE_LOG = 'Recipe.txt'
        self.BF_DATA_LOG = 'Data.txt'
        self.BF_GROUP_LOG = 'BF_Grouping.txt'
        self.BF_DETECT_LOG = 'BF_DetectLine.txt'
        self.BF_CLUSTER_LOG = 'BF_Cluster.txt'
        self.BF_FITORDER_LOG = 'BF_FitOrder.txt'
        self.BF_FIT_LOG = 'BF_Fit.txt'
        self.BF_STAT_LOG = 'Flagger.txt'
        self.BF_GRID_LOG = 'Gridding.txt'
        self.CALIBRATION_LOG = 'Calibration.txt'

        self.ReStartID = 0

        # temporary log file
        self.tmpLogFile = './heuristics_tmp.log-%s'%(time.asctime(time.gmtime()).replace(' ','_')).replace(':','_')
        #self.calLogFile = './heuristics_cal.log-%s'%(time.asctime(time.gmtime()).replace(' ','_')).replace(':','_')
        self.calLogFile = {}
        self.setFileOut(self.tmpLogFile)
        self.LogMessage('INFO',Origin='__init__()',Msg=self.tmpLogFile,ConsoleOut=False)

        # internal processing parameters
        self.radius = 0.0025
        self.spacing = self.radius / 3.0 * 2.0
        self.clusterGridSize = self.radius * 2.0        
        self.edge = [0, 0]        
        self.broadComponent = False
        self.continuumOutput = False
        self.imageCube = True
        self.iteration = 0
        self.isTP = False

        #self.LogLevel = 2
        # 2010/06/15 TN
        # Depth of the call stack depends on how scripts are run
        # (if task interface is used or not)
        import inspect
        a = inspect.stack()
        stacklevel = 0
        for i in range(len(a)):
            if a[i][1].find( 'ipython console' ) != -1:
                stacklevel = i-1
                break
        self.recipeName = sys._getframe(stacklevel).f_code.co_filename
        casadict = sys._getframe(stacklevel+1).f_globals['casa']
        try:
            self.casarevision = int( casadict['build']['number'] )
        except:
            self.casarevision = -1
        self.casaverstr = casadict['build']['version']
        self.casaversion = int( self.casaverstr.replace('.','') )
        self.LogMessage('INFO',Origin='__init__()',Msg='CASA version: %s (revision %s)'%(self.casaverstr,self.casarevision))

        # heuristics version
        self.hsdversion=getHeuristicsVersion()

        # internally stored data currently processed
##         self.DataTable = None
##         self.listall   = None
        self.Abcissa   = None
        self.Pattern   = None
        self.TimeTable = None
        self.TimeGap   = None
        self.GridTable = None
        self.ResultTable = None
        self.DetectSignal = None
        self.PosDict = None
        self.PosDictAll = None
        self.Lines=[]
        self.LinesSPW={}
        self.GridResult = {}
        self.GridTableComb = None

        self.SpectrumWindow = []
        self.ChannelWindow = []
        self.addparam(linewindow=[])
        self.ROW = []
        self.ROWbase = []
        self.SCAN = []
        self.SCANbase = []
        self.IF = []
        self.POL = []
        self.processedIFPOL = []
        self.processedStages = []
        self.NCHAN = 1
        self.vIF = 0
        self.vPOL = 0
        self.poltype = 'none'
        # integrated intensity map is created by default
##         self.moments = [0]
##         self.addparam(moments=self.moments)

        # list of selected rows
        self.rowsSel = []
        self.rowsidSel = []
        self.basedataSel = []
        self.basedataidSel = []

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

        # for calibration
        self.addparam(calmode='none')

    def __del__( self, base=SDLogger ):
        """
        """
##         if self.frontPage is not None:
##             endtime=time.gmtime()
##             timestr=time.asctime(endtime)+' UTC'
##             elapsed=time.mktime(endtime)-time.mktime(self.startTime)
##             self.frontPage.postInfo(endtime=timestr,elapsedtime=elapsed)
        #del self._tbtool
        del self._tb
##         self.cleanup()
##         base.__del__( self )
        

    def resetIntermediateData(self):
        self.Pattern   = None
        self.TimeTable = None
        self.TimeGap   = None
        self.GridTable = None
        self.ResultTable = None
        self.PosDict = None
        self.PosDictAll = None
        self.iteration = 0

    def resetResultData(self):
        self.ResultTable = None

    def setBeamRadius(self,radius):
        self.radius=radius
        self.addparam(beamsize='%s arcsec'%(radius*7200.0))
        self.spacing = self.radius / 3.0 * 2.0
        self.clusterGridSize = self.radius * 2.0        
        #self.clusterGridSize = self.spacing * 3.0        
        
    def getBeamRadius(self):
        return self.radius

    def setSpectrumWindow(self,SpectrumWindow):
        self.SpectrumWindow = SpectrumWindow
        self.addparam(linewindow='%s (spectral window)'%(self.SpectrumWindow))
        
    def getSpectrumWindow(self):
        return self.SpectrumWindow

    def setChannelWindow(self,ChannelWindow):
        self.ChannelWindow = ChannelWindow
        self.addparam(linewindow='%s (channel window)'%(self.ChannelWindow))

    def getChannelWindow(self):
        return self.ChannelWindow

    def setBaselineEdge(self,edge):
        if type(edge) == int:
            self.edge = [edge, edge]
        elif len(edge) != 2:
            self.edge = [edge[0], edge[0]]
        else:
            self.edge=edge
        self.addparam(edge=self.edge)

    def getBaselineEdge(self):
        return self.edge

    def setLogLevel(self,LogLevel,ConsoleLevel=3):
        if self.engine is not None:
            self.engine.setLogLevel( LogLevel, ConsoleLevel )
        SDLogger.setLogLevel( self, LogLevel, ConsoleLevel )
        self.addparam(loglevel=LogLevel)

    def getLogLevel(self):
        return self.LogLevel

    def disableConsoleOutput( self ):
        if self.engine is not None:
            self.engine.disableConsoleOutput()
        SDLogger.disableConsoleOutput( self )

    def enableConsoleOutput( self ):
        if self.engine is not None:
            self.engine.enableConsoleOutput()
        SDLogger.enableConsoleOutput( self ) 

    def setTsysFlagThreshold(self,threshold):
        self.FlagRuleDictionary['TsysFlag']['Threshold']=threshold

    def setRmsPreFitFlagThreshold(self,threshold):
        self.FlagRuleDictionary['RmsPreFitFlag']['Threshold']=threshold

    def activateFlagRule(self,key):
        if(key in self.FlagRuleDictionary.keys()):
           self.FlagRuleDictionary[key]['isActive']=True
           self.addflagparam()
        else:
            print 'Error not in predefined Flagging Rules'

    def deactivateFlagRule(self,key):
        if(key in self.FlagRuleDictionary.keys()):
           self.FlagRuleDictionary[key]['isActive']=False
           self.addflagparam()
        else:
            print 'Error not in predefined Flagging Rules'

    def setFlagRuleThreshold(self,key,threshold):
        if(key in self.FlagRuleDictionary.keys()):
           self.FlagRuleDictionary[key]['Threshold']=threshold
           self.addflagparam()
        else:
            print 'Error not in predefined Flagging Rules'
            
    def setRunMeanPreFitFlagThreshold(self,threshold):
        self.FlagRuleDictionary['RunMeanPreFitFlag']['Threshold']=threshold
        self.addflagparam()

    def setRunMeanPreFitFlagNmean(self,nmean):
        self.FlagRuleDictionary['RunMeanPreFitFlag']['Nmean']=nmean
        self.addflagparam()
            
    def setRunMeanPostFitFlagNmean(self,nmean):
        self.FlagRuleDictionary['RunMeanPostFitFlag']['Nmean']=nmean
        self.addflagparam()

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
            self.addparam(interactive='True')
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
            self.addparam(interactive='False')

            
        
    def setOutDirectory(self,outDir):

        """
        Sets the output directory. If none is set data is written to ./
        """
        
        self.outDir = outDir+'/'
        self.addparam(outdir=outDir)
        self.workDir = self.outDir
        if not os.access(self.outDir, os.F_OK):
            os.mkdir(self.outDir)

    def getOutDirectory(self):
        return self.outDir        

    def setRawDirectory(self,rawDir):

        """
        Sets the input directory containing the raw ASAP file/directory. If none is set data is read from ./
        """        
 
        self.rawDir = rawDir+'/'        
        self.addparam(rawdir=rawDir)
        
    def getRawDirectory(self):
        return self.rawDir

    def setRawFile(self,rawFile):

        """
        Sets the input raw ASAP file/directory in ./ respectively the input directory.
        """
        
        if type(rawFile) is str:
            self.rawFile = rawFile
            self.rawFileList = [rawFile]
        else:
            self.rawFile = rawFile[0]
            self.rawFileList = rawFile
        self.addparam(rawfile=self.rawFileList)

    def setOutFile(self,outFile):
        """
        Sets the output file/directory structure in ./ respectively the output directory.
        If this is not set a directory rawFile_out is created in ./ respectively the output directory.
        """
        self.outFile=outFile

    def getRawFile(self):
        return self.rawFileList
    
    def getOutFile(self):
        return self.outFile

    def setRecipeName(self, RecipeName):
        """
        Sets the name of the Recipe invoked
        """
        self.recipeName = RecipeName

    def getRecipeName(self):
        return self.recipeName
    
    def setBroadLineSpectralComponent(self, broadComponent=True):
        if((broadComponent==True)or(broadComponent==False)):
            self.broadComponent=broadComponent
        self.addparam(broadline=self.broadComponent)

    def getBroadLineSpectralomponent(self):
        return self.broadComponent

    def setContinuumOutput(self, continuumOutput=True):
        if((continuumOutput==True)or(continuumOutput==False)):
            self.continuumOutput=continuumOutput
        self.addparam(continuum=self.continuumOutput)

    def getContinuumOutput(self):
        return self.continuumOutput

    def setImageCube(self, imageCube):
        if((imageCube==True)or(imageCube==False)):
            self.imageCube=imageCube

    def getIF(self):
        return self.IF
    
    def getPOL(self):
        return self.POL

    def setFlagRules(self):
        #import SDFlagRule
        from pipeline.hsd.tasks.flagdata import SDFlagRule
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

    def getImageCube(self):
        return 1

##     def setMoments(self,moments):
##         if type(moments)==list:
##             self.moments = moments
##         else:
##             self.moments = [moments]
##         self.addparam(moments=self.moments)

##     def getMoments(self):
##         return self.moments

    def saveMetaData(self, MetaFileName):
        M_file = open(MetaFileName, 'w')
        # Dump USE_CASA_TABLE variable
        pickle.dump((self.engine.USE_CASA_TABLE),M_file)
        # Dump meta data created at initial stage
##         pickle.dump((self.ROW, self.ROWbase, self.SCAN, self.SCANbase, self.IF, self.POL, \
##                      self.listall, self.Abcissa, self.DataTable), M_file)
        if self.engine.USE_CASA_TABLE:
            # pickle abspath to DataTable
            dtname = os.path.expandvars(self.workDir+'ThisIsExportedDataTable.tbl')
            pickle.dump((os.path.abspath(dtname)),M_file)
            pickle.dump((self.ROW, self.ROWbase, self.SCAN, self.SCANbase, self.IF, self.POL, \
                         ##                      self.iterAnt, self.rawFileIdx, \
                         self.rawFileIdx, \
                         ##                     self.engine.listall, self.Abcissa, self.engine.DataTable), M_file)
                         self.engine.listall, self.Abcissa), M_file)
            self.engine.DataTable.exportdata(dtname,overwrite=True)
##             try:
##                 self.engine.DataTable.exportdataminimal()
##             except StandardError, e:
##                 if str(e).find('You have to call') >= 0:
##                     self.engine.DataTable.exportdata(dtname,overwrite=True)
##                 else:
##                     raise e
##             except Exception, e:
##                 raise e
        else:
            pickle.dump((self.ROW, self.ROWbase, self.SCAN, self.SCANbase, self.IF, self.POL, \
                         ##                      self.iterAnt, self.rawFileIdx, \
                         self.rawFileIdx, \
                         self.engine.listall, self.Abcissa, self.engine.DataTable), M_file)
            
        # Dump file and directory names
        pickle.dump((self.rawDir, self.outDir, self.workDir, self.HtmlDir, self.LogDir, \
                     self.rawFile, self.outFile, self.workFile, \
                     self.rawFileList, self.outFiles, self.antFile, \
                     self.initData, self.dtctData, self.lastData), M_file)
        # Dump meta related to the selected IF/POL
        pickle.dump((self.rowsSel, self.rowsidSel, self.basedataSel, self.basedataidSel, \
                     self.PosDict, self.PosDictAll, self.TimeTable, self.TimeGap, \
                     self.vIF, self.vPOL, self.isTP, self.Pattern, self.NCHAN, \
                     self.iteration, self.Lines, self.GridTable, self.ResultTable, \
                     self.DetectSignal, self.processedIFPOL, self.processedStages), M_file)
        # Dump rule books
        pickle.dump((self.FlagRuleDictionary, self.ClusterRuleDictionary, \
                     self.GridRuleDictionary, self.FitOrderRuleDictionary, \
                     self.SwitchingRuleDictionary), M_file)
        # Dump other parameters
        pickle.dump((self.radius, self.spacing, self.clusterGridSize, self.broadComponent, \
                     self.imageCube, self.LogLevel, self.recipeName, self.edge), M_file)
        M_file.close()
        return

    def loadMetaData(self, MetaFileName):
        M_file = open(MetaFileName, 'r')
        # Unpack USE_CASA_DATA variable
        self.engine.USE_CASA_TABLE=pickle.load(M_file)
        # UnPack meta data created at initial stage
        if self.engine.USE_CASA_TABLE:
            #import heuristics.hsd.SDDataTable as SDDataTable
            #import pipeline.hsd.heuristics.SDDataTable as SDDataTable
            import SDDataTable
            # load path to DataTable
            dtname = pickle.load(M_file)
            (self.ROW, self.ROWbase, self.SCAN, self.SCANbase, self.IF, self.POL, \
             ##          self.iterAnt, self.rawFileIdx, \
             self.rawFileIdx, \
             self.engine.listall, self.Abcissa) = pickle.load(M_file)
            self.engine.DataTable = SDDataTable.DataTableAdapter()
            self.engine.DataTable.importdata( dtname )
        else:
##         (self.ROW, self.ROWbase, self.SCAN, self.SCANbase, self.IF, self.POL, \
##          self.listall, self.Abcissa, self.DataTable) = pickle.load(M_file)
            (self.ROW, self.ROWbase, self.SCAN, self.SCANbase, self.IF, self.POL, \
##          self.iterAnt, self.rawFileIdx, \
             self.rawFileIdx, \
             self.engine.listall, self.Abcissa, self.engine.DataTable) = pickle.load(M_file)
        # Unpack file and directory names
        (self.rawDir, self.outDir, self.workDir, self.HtmlDir, self.LogDir, \
         self.rawFile, self.outFile, self.workFile, \
         self.rawFileList, self.outFiles, self.antFile, \
         self.initData, self.dtctData, self.lastData) = pickle.load(M_file)
        # Unpack meta related to the selected IF/POL
        (self.rowsSel, self.rowsidSel, self.basedataSel, self.basedataidSel, \
         self.PosDict, self.PosDictAll, self.TimeTable, self.TimeGap, \
         self.vIF, self.vPOL, self.isTP, self.Pattern, self.NCHAN, \
         self.iteration, self.Lines, self.GridTable, self.ResultTable, \
         self.DetectSignal, self.processedIFPOL, self.processedStages) = pickle.load(M_file)
        # Unpack rule books
        (self.FlagRuleDictionary, self.ClusterRuleDictionary, \
         self.GridRuleDictionary, self.FitOrderRuleDictionary, \
         self.SwitchingRuleDictionary) = pickle.load(M_file)
        # Unpack other parameters
        (self.radius, self.spacing, self.clusterGridSize, self.broadComponent, \
         self.imageCube, self.LogLevel, self.recipeName, self.edge) = pickle.load(M_file)
        M_file.close()
        return

    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    #def calibrate(self,Antenna=-1,calMode='auto'):
    def calibrate(self,calMode='auto'):

        origin = 'calibrate()'

        self.addparam( calmode=calMode )

##         # if Antenna < 0, it should be while loop, otherwise set self.iterAnt
##         if Antenna >= 0:
##             if Antenna not in self.antFile.keys():
##                 # Error
##                 raise Exception('Invalid Antenna ID')
##             elif Antenna in self.outFiles.keys():
##                 self.LogMessage( 'INFO', Origin=origin, Msg='Calibration already done. Skip.' )
##                 return
##             else:
##                 self.iterAnt = Antenna
##                 idx = self.rawFileIdx[self.iterAnt]
##                 self.rawFile = self.rawFileList[idx]
##                 self.productName = self.productNameList[idx]

                
        # run fillTsys if necessary
        for key in self.antKeys:
            idx = self.rawFileIdx[key]
            if self.dataSummary[idx]['filltsys']:
                self.LogMessage( 'INFO', Origin=origin, Msg='Running filltsys on %s'%(self.antFile[key]) )
                spwspec=self.dataSummary[idx]['spwspec']
                spwtsys=self.dataSummary[idx]['spwtsys']
                for i in xrange(len(spwspec)):
                    if (type(self.IF) is str and self.IF == 'all') \
                           or (type(self.IF) is list and spwspec[i] in self.IF):
                        self.LogMessage('INFO',Origin=origin,Msg='Transfer Tsys from SPW %s to SPW %s'%(spwtsys[i],spwspec[i]) )
                        filltsys.fillTsys( self.workDir+self.antFile[key],
                                           specif=spwspec[i],
                                           tsysif=spwtsys[i] )

            # setup
            filename = self.workDir+self.antFile[key]

            # log files
            #logfile = self.calLogFile
            #logfile = self.LogDir+'/'+self.CALIBRATION_LOG
            self.calLogFile[key] = './heuristics_cal.log-%s'%(time.asctime(time.gmtime()).replace(' ','_')).replace(':','_')
            logfile = self.calLogFile[key]

            #self.setFileOut( self.tmpLogFile )
            #self.LogMessage('INFO',Origin=origin,Msg=self.tmpLogFile,ConsoleOut=False)
            self.LogMessage( 'INFO', Origin=origin, Msg='Start Calibration' )

            # clean-up work files
            if os.path.exists( self.workDir+self.workFile[key] ):
                os.system( '\\rm -rf '+self.workDir+self.workFile[key] )

            # detect WVR scan, create WVR data if necessary
            self.detectWVRScan()
            if not self.WVRFile.has_key(key):
                self.saveWVRData(key)

            # prepare working data, calibrate if necessary
            if calMode == 'auto':
                needCal = self.checkCal( self.workDir+self.antFile[key] )
            elif calMode == 'skip':
                needCal = [False]
            else:
                needCal = [True]
            if needCal[0] == False:
                self.LogMessage('INFO', Origin=origin, Msg='Skip Calibration')
                self.createWorkFileWithoutWVRScan(key)
                return 

            # Calibration using CASA single-dish tools (ASAP)
            #
            # -- 18/08/2008 Takeshi Nakazato
            # tool level processing using asap
            if calMode == 'auto':
                calType = 'none'
            else:
                calType = calMode
            scal = SDC.SDCalibration( filename=filename, caltype=calType, casaversion=self.casaversion, LogLevel=self.LogLevel, LogFile=logfile)
            scal.summary(LogLevel=self.LogLevel,LogFile=logfile)
            scal.calibrate(LogLevel=self.LogLevel,LogFile=logfile)
            if ( not scal.isCalibrated() ):
                #self.setFileOut( self.GlobalLogDir+self.SD_PIPELINE_LOG )
                #self.setFileOut( self.tmpLogFile )
                self.LogMessage('ERROR', Origin=origin, Msg='Data could not be calibrated. Abort.')
                del scal
                raise Exception('calibrate(): Data could not be calibrated. Abort.')
                return
            scal.average(LogLevel=self.LogLevel,LogFile=logfile)
            scal.save(self.workDir+self.workFile[key],LogLevel=self.LogLevel,LogFile=logfile)
            del scal
            self.setFileOut(self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG)
            #self.setFileOut( self.tmpLogFile )
            self.LogMessage('INFO', Origin=origin, Msg='Raw Data: %s' % filename )
            self.LogMessage('INFO', Origin=origin, Msg='Calibrated Data: %s' % self.workDir+self.workFile[key] )


    #def setupSDReducer(self,IF='all',POL='all',ROW='all',ROWbase='all',SCAN='all',SCANbase='all',doCal='auto',overwrite=False):
    #def setupSDReducer(self,overwrite=False):
    #def setupSDReducer(self,IF='all',POL='all',ROW='all',ROWbase='all',SCAN='all',SCANbase='all',overwrite=False):
    #@dec_stages_interface_logging
    def setupSDReducer(self,ANTENNA='all',IF='all',POL='all',ROW='all',ROWbase='all',SCAN='all',SCANbase='all',overwrite=False):
        """
        Setup a directory structure for working space.
        Convert input data into several Scantables by antenna if ASDM or MS data is specified.
        
        Additional Parameters
        overwrite=False
        overwrite existing converted Scantables
        """

        origin = 'setupSDReducer()'

        # Data Product Naming Convention
        # 2011/05/23 TN
        self.productNameList = SDN.SDDataProductNameList( rawdir=self.rawDir, rawfile=self.rawFileList, workdir=self.outDir )

        # HTML front page
        # 2011/05/20 TN
        #self.HtmlTopDir = self.productName.getGlobalHtmlName()
        if self.HtmlTopDir is None:
            self.HtmlTopDir = self.productNameList.getGlobalHtmlName()
        # clean-up existing html directory
        if os.access(self.HtmlTopDir, os.F_OK):
            for root, dirs, files in os.walk(self.HtmlTopDir, topdown=False):
                for name in files:
                    os.remove(os.path.join(root, name))
                for name in dirs:
                    os.rmdir(os.path.join(root, name))
        else:
            os.makedirs(self.HtmlTopDir)        
        self.frontPage = SDH.HtmlFrontPage(self.HtmlTopDir,rawFile=self.rawFileList)

        # for Profiling page
        # 2011/05/24 TN
        self.profilePage = SDH.HtmlProfile(self.HtmlTopDir)

        # Global log (PIPELINE.txt)
        # 2011/05/23 TN
        self.GlobalLogDir = self.HtmlTopDir+'/'+self.frontPage.logDir
        if os.path.exists( self.tmpLogFile ):
            f=open(self.tmpLogFile,'r')
            lines=f.readlines()
            f.close()
            idx = -1
            for iline in xrange(len(lines)):
                if lines[iline].find( self.tmpLogFile ) != -1:
                    idx = iline+1
                    break
            if idx != -1:
                #f=open(self.LogDir+'/'+self.SD_PIPELINE_LOG,'a')
                f=open(self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG,'a')
                for iline in xrange(idx,len(lines)):
                    f.write(lines[iline])
                f.close()
            os.remove( self.tmpLogFile )
        #self.setFileOut(self.LogDir+'/' + self.SD_PIPELINE_LOG )
        self.setFileOut(self.GlobalLogDir+'/' + self.SD_PIPELINE_LOG )


        self.addparam( antlist=ANTENNA,
                       iflist=IF,
                       pollist=POL,
                       rowlist=ROW,
                       rowbase=ROWbase,
                       scanlist=SCAN,
                       scanbase=SCANbase )

        # setup input output directories and files
        if(self.rawDir==None): self.rawDir='./'
        if(self.outDir==None): self.outDir='./'
        self.workDir = self.outDir
        #self.workFile = self.rawFile+'_work'
        #self.initData = self.workDir+self.workFile
        #self.dtctData = self.workDir+self.workFile
        #self.lastData = self.workDir+self.workFile

        self.ANTENNA = ANTENNA
        if type(self.ANTENNA) == int:
            self.ANTENNA = [ANTENNA]
        elif type(self.ANTENNA) == str:
            self.ANTENNA = [ANTENNA]
        self.IF = IF
        if type(self.IF) == int:
            self.IF = [IF]
        self.POL = POL
        if type(self.POL) == int:
            self.POL = [POL]
        self.ROW = ROW
        if type(self.ROW) == int:
            self.ROW = [ROW]
        self.ROWbase = ROWbase
        if type(self.ROWbase) == int:
            self.ROWbase = [ROWbase]
        self.SCAN = SCAN
        if type(self.SCAN) == int:
            self.SCAN = [SCAN]
        self.SCANbase = SCANbase
        if type(self.SCANbase) == int:
            self.SCANbase = [SCANbase]

        # create data summary for subsequent process
        numRawFile = len(self.rawFileList)
        self.dataSummary = [{} for i in xrange(numRawFile)]
        self.dataFormat = ['' for i in xrange(numRawFile)]
        for i in xrange(numRawFile):
            self.dataSummary[i] = getSummary( self.rawDir+self.rawFileList[i] )
            self.dataFormat[i] = self.dataSummary[i]['type']

        # prepare working data
        numAnt = 0
        self.rawFileIdx = {}
        for i in xrange(numRawFile):
            dform = self.dataFormat[i]
            currentFile = self.rawFileList[i].rstrip('/')
            if dform == 'ASDM' or dform == 'MS2':
                osfdconv = osfdatautils( self.rawDir, currentFile, dform )
                convfiles = osfdconv.domod(outdir=self.workDir,outprefix=currentFile,overwrite=overwrite)
                self.dataSummary[i] = getSummary( osfdconv.filems )
                for ifile in xrange(len(convfiles)):
                    self.antFile[numAnt] = convfiles[ifile].split('/')[-1]
                    self.rawFileIdx[numAnt] = i
                    numAnt += 1
            elif dform == 'ASAP':
                self.antFile[numAnt] = currentFile
                self.rawFileIdx[numAnt] = i
                numAnt += 1
                os.system('cp -r ' + self.rawDir+currentFile+ ' ' + self.workDir)
            elif dform == 'FITS':
                stmp = scantable( self.rawDir+currentFile, False )
                convfile = currentFile+'.asap'
                self.antFile[numAnt] = convfile
                self.rawFileIdx[numAnt] = i
                numAnt += 1
                stmp.save( self.workDir+convfile, 'ASAP' )
                del stmp
        
        self.selectAntenna()
        self.numAnt = len(self.antFile)
        self.antKeys = self.antFile.keys()
        self.LogMessage('INFO',Origin=origin,Msg='self.antKeys=%s'%(self.antKeys))

        # beam size
        # self.beamsize = { antKey0: [v_spw0_file0, v_spw1_file0, ...],
        #                   antKey1: [v_spw0_file0, v_spw1_file0, ...],
        #                   ... }
        self.beamsize = {}
        aid = 0
        beamHeuristics = SingleDishBeamSizeFromName()
        for i in xrange(numRawFile):
            antList = self.dataSummary[i]['antenna']
            spwList = self.dataSummary[i]['spw']
            for (akey,aitem) in antList.items():
                a = aitem['name']
                if a not in self.antName .values():
                    continue
                l = []
                for (skey,sitem) in spwList.items():
                    f = 0.5*sum(sitem['freq'])*1.0e-9 # Hz->GHz
                    #b = beamsize(a, f)
                    b = beamHeuristics(a,f)
                    l.append(b)
                self.beamsize[self.antKeys[aid]] = l
                aid += 1
        self.LogMessage('INFO',Origin=origin,Msg='self.beamsize=%s'%(self.beamsize))

        # prepare working data
        self.workFile = {}
        self.initData = {}
        #self.dtctData = {}
        self.dtctData = self.workDir+self.antFile[self.antKeys[0]]+'_dtct'
        self.lastData = {}
        self.gridData = {}
        self.gridCont = {}
        for key in self.antKeys:
            self.workFile[key] = self.antFile[key]+'_work'
            self.initData[key] = self.workDir+self.workFile[key]
            #self.dtctData[key] = self.workDir+self.workFile[key]
            self.lastData[key] = self.workDir+self.workFile[key]
            self.gridData[key] = {}
            self.gridCont[key] = {}
            self.GridResult[key] = {}

        # 2011/05/23 TN
        # set attributes for data product name
        self.productNameList.setFormat( self.dataFormat )
        self.productNameList.setSource() 

        # fill in front page
        import string
        self.frontPage.setNumAnt(self.numAnt)
        self.frontPage.postInfo(casaver=self.casaverstr,
                                casarev=self.casarevision,
                                hver=self.hsdversion)
        for i in xrange(numRawFile):
            self.frontPage.addFile(self.rawFileList[i])
            timestr = time.asctime(self.startTime)+' UTC'
            summary = self.dataSummary[i]
            observer = summary['observer']
            obsdate = summary['date'][0]+' - '+summary['date'][1]
            sourcename = summary['source'][0]['name']
            srcdir = summary['source'][0]['dir']
            if sourcename is None or len(sourcename) == 0:
                sourcename = 'Undefined'
            antDic = summary['antenna']
            antList = []
            for key in antDic.keys():
                antList.append(antDic[key]['name'])
            antennas = string.join(antList,' ')
            self.frontPage.postInfo(starttime=timestr,
                                    who=observer,
                                    when=obsdate,
                                    what=sourcename,
                                    antenna=antennas)
            spwSummary = summary['spw']
            for key in spwSummary.keys():
                val = spwSummary[key]
                freq = val['freq']
                freqStr = '%7.4f-%7.4f'%(freq[0]/1.0e9,freq[1]/1.0e9)
                self.frontPage.addSpectralWindow(key,
                                                 freqStr,
                                                 val['nchan'],
                                                 val['bw']/1.0e6,
                                                 val['frame'],
                                                 val['intent'].split(':')[0],
                                                 val['type'])
            polSummary = summary['pol']
            for key in polSummary.keys():
                val = polSummary[key]
                ptype=val['type']
                corr=string.join(val['str'])
                assoc=val['spwid']
                self.frontPage.addPolarization(key,
                                               ptype,
                                               corr,
                                               assoc)
            self.frontPage.setSourceDirection(srcdir)
            self.frontPage.finishFile()
        self.frontPage.finishObs()

        # copy recipe file to log directory
        if os.path.exists(self.recipeName):
            os.system('cp '+self.recipeName+' '+self.GlobalLogDir+'/'+self.SD_RECIPE_LOG)
            os.system('chmod 664 '+self.GlobalLogDir+'/'+self.SD_RECIPE_LOG)
            self.frontPage.postInfo(recipename=self.recipeName.split('/')[-1])
        else:
            self.recipeName = None
            self.frontPage.postInfo(recipename='Recipe name')


    #def readData(self,IF='all',POL='all',ROW='all',ROWbase='all',SCAN='all',SCANbase='all',doCal='auto',overwrite=False):
    #def readData(self,IF='all',POL='all',ROW='all',ROWbase='all',SCAN='all',SCANbase='all',doCal='auto'):
    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def readData(self):
    #def readData(self,Antenna=-1):
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
        doCal='auto'
        to specify calibration mode
           'auto': calibrate if neccesary, skip calibration if _cal file exists
           'force': calibrate forcibly, overwrite exsisting _cal file
           'fold': no frequeycy switch calibration, but do folding
           'skip': no calibration
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
        #               Statistics, Flags, PermanentFlags, SummaryFlag, Nmask, MaskList, NoChange, Ant]
        #                       16,    17,             18,          19,    20,       21,       22,  23
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

        origin = 'readData()'

        # if Antenna < 0, it should be while loop, otherwise set self.iterAnt
##         if Antenna >= 0:
##             if Antenna not in self.antFile.keys():
##                 # Error
##                 raise Exception('Invalid Antenna ID')
##             elif Antenna in self.outFiles.keys():
##                 self.LogMessage( 'INFO', Origin=origin, Msg='Data already read. Skip.' )
##                 return
##             else:
##                 self.iterAnt = Antenna
##                 idx = self.rawFileIdx[self.iterAnt]
##                 self.rawFile = self.rawFileList[idx]
##                 self.productName = self.productNameList[idx]

        # setup input output directories and files
##         if(self.outFile==None): self.outFile=self.rawFile+'_out'
##         self.outFile=self.antFile[self.iterAnt]+'_out'
##         if not ( self.iterAnt in self.outFiles.keys() ):
##             self.outFiles[self.iterAnt] = self.outFile
##         if self.continuumOutput == True:
##             self.contFile = self.outFile+'_cont'
##             if not ( self.iterAnt in self.contFiles.keys() ):
##                 self.contFiles[self.iterAnt] = self.contFile
##             self.contData = self.workDir+self.contFile
        self.outFile = {}
        self.contFile = {}
        self.contData = {}
        for key in self.antKeys:
            self.outFile[key] = self.antFile[key]+'_out'
        if self.continuumOutput == True:
            for key in self.antKeys:
                self.contFile[key] = self.outFile[key]+'_cont'
                self.contData[key] = self.workDir+self.contFile[key]
                
        # 2011/03/14 TN
        # data product naming convention
        #self.HtmlDir = self.outDir+self.antFile[self.iterAnt]+'_html'
        #self.LogDir = self.outDir+self.antFile[self.iterAnt]+'_html'+'/'+BF_LOG_DIR

        # for HTML front page
        # 2011/05/20 TN
        self.HtmlDir = {}
        self.LogDir = {}
        self.productName = {}
        for key in self.antKeys:
            self.productName[key] = self.productNameList[self.rawFileIdx[key]]
            self.HtmlDir[key] = self.HtmlTopDir + '/' + self.productName[key].getLocalHtmlName( ant=self.antName[key], relative=True )
            self.frontPage.addAntenna(name=self.antName[key],id=key,data=self.rawFileList[self.rawFileIdx[key]])
            self.LogDir[key] = self.HtmlDir[key]+'/'+BF_LOG_DIR
        if len(NP.unique(self.antName.values())) > 1:
            self.CombineDir = self.HtmlTopDir + '/' + self.frontPage.combined
        else:
            self.CombineDir = None

        # clean-up work files
        for key in self.antKeys:
            if os.access(self.outDir+self.outFile[key], os.F_OK): os.system('\\rm -r '+self.outDir+self.outFile[key]+'*')

        # 2010/06/29 TN
        # for multiple antenna data 
        #if self.engine.listall != None:
        # 2011/10/25 TN
        # following block would never be needed
##         if len(self.engine.listall) != 0:
##             listall = self.engine.listall[0]
##             if listall[1] == self.ROW:
##                 self.ROW = 'all'
##             if listall[0] == self.ROWbase:
##                 self.ROWbase = 'all'
##             if listall[2] == self.SCAN:
##                 self.SCAN = 'all'
##             if listall[2] == self.SCANbase:
##                 self.SCANbase = 'all'
##             if listall[3] == self.IF:
##                 self.IF = 'all'
##             if listall[4] == self.POL:
##                 self.POL = 'all'
        
##         del self.DataTable
##         del self.listall
        del self.Abcissa
        del self.engine
##         self.DataTable=None
##         self.listall=None
        self.Abcissa=None
        self.engine=SDEngine(casaversion=self.casaversion,casarevision=self.casarevision)
        self.engine.setLogLevel( self.LogLevel, self.ConsoleLevel )
        if not self.isConsoleOutputEnabled():
            self.engine.disableConsoleOutput()
            
        # clean-up existing html directory
        # for clustering analysis
        # all data are combined when finding line
        self.HtmlCluster = 'Clustering'
        if os.access(self.HtmlTopDir+'/'+self.HtmlCluster, os.F_OK):
            for root,dirs,files in os.walk(self.HtmlTopDir+'/'+self.HtmlCluster,topdown=False):
                for name in files:
                    os.remove(os.path.join(root,name))
                for name in dirs:
                    os.rmdir(os.path.join(root,name))
        else: os.mkdir(self.HtmlTopDir+'/'+self.HtmlCluster)
        os.mkdir(self.HtmlTopDir+'/'+self.HtmlCluster+'/'+BF_CLUSTER_PDIR)

        for key in self.antKeys:
            if os.access(self.HtmlDir[key], os.F_OK):
                for root, dirs, files in os.walk(self.HtmlDir[key], topdown=False):
                    for name in files:
                        os.remove(os.path.join(root, name))
                    for name in dirs:
                        os.rmdir(os.path.join(root, name))
            else: os.mkdir(self.HtmlDir[key])
            os.mkdir(self.HtmlDir[key]+'/'+BF_LOG_DIR)
            os.mkdir(self.HtmlDir[key]+'/'+BF_DATA_DIR)
            #os.mkdir(self.HtmlDir[key]+'/'+BF_CLUSTER_PDIR)
            os.mkdir(self.HtmlDir[key]+'/'+BF_FIT_PDIR)
            os.mkdir(self.HtmlDir[key]+'/'+BF_STAT_PDIR)
            os.mkdir(self.HtmlDir[key]+'/'+BF_GRID_PDIR)
            os.mkdir(self.HtmlDir[key]+'/'+BF_CHANNELMAP_PDIR)
            os.mkdir(self.HtmlDir[key]+'/'+BF_SPARSESP_PDIR)

        if self.CombineDir is not None:
            if os.access(self.CombineDir, os.F_OK):
                for root, dirs, files in os.walk(self.CombineDir, topdown=False):
                    for name in files:
                        os.remove(os.path.join(root, name))
                    for name in dirs:
                        os.rmdir(os.path.join(root, name))
            else: os.mkdir(self.CombineDir)
            os.mkdir(self.CombineDir+'/'+BF_LOG_DIR)
            os.mkdir(self.CombineDir+'/'+BF_DATA_DIR)
            #os.mkdir(self.CombineDir+'/'+BF_CLUSTER_PDIR)
            os.mkdir(self.CombineDir+'/'+BF_FIT_PDIR)
            os.mkdir(self.CombineDir+'/'+BF_STAT_PDIR)
            os.mkdir(self.CombineDir+'/'+BF_GRID_PDIR)
            os.mkdir(self.CombineDir+'/'+BF_CHANNELMAP_PDIR)
            os.mkdir(self.CombineDir+'/'+BF_SPARSESP_PDIR)

        # set up logger
        for key in self.antKeys:
            if self.calLogFile.has_key(key) and os.path.exists( self.calLogFile[key] ):
                os.system( '\\mv %s %s'%(self.calLogFile[key], self.LogDir[key]+'/'+self.CALIBRATION_LOG) )
        #self.engine.stagesLogFile = self.LogDir+'/'+self.SD_PIPELINE_LOG

        # 2011/03/08 TN
        # Moved to calibrate()
##         # detect WVR scan, create WVR data if necessary
##         self.detectWVRScan()
##         if self.WVRScan is not None:
##             self.saveWVRData()
##        
##         # prepare working data, calibrate if necessary
##         needCal = self.checkCal( self.workDir+self.antFile[self.iterAnt] )
##         if doCal == 'fold':
##             needCal[0] = True 
##         if needCal[0]:
##             # do calibration
##             ProcStartTime = time.time()
##             self.LogMessage('INFO', Origin=origin, Msg='Calibration Start:')
##             self.calibrate( self.workDir+self.antFile[self.iterAnt], doCal )
##             os.system( 'cp -r '+self.calDir+self.calFile+' '+self.workDir+self.workFile )
##             ProcEndTime = time.time()
##             self.LogMessage('INFO', Origin=origin, Msg='Calibration End: ElapseTime=%.1f sec' % (ProcEndTime-ProcStartTime)) 
##         else:
##             self.LogMessage('INFO', Origin=origin, Msg='Skip Calibration')
##             self.createWorkFileWithoutWVRScan()

        # create working data if calibration stage is skipped
        for key in self.antKeys:
            if not os.path.exists( self.workDir+self.workFile[key] ):
                self.createWorkFileWithoutWVRScan( key )
            
        # copy 'exp_rms_factors' file to log directory
        for key in self.antKeys:
            idx = self.rawFileIdx[key]
            rawFile = self.rawFileList[idx]
            if os.path.exists( self.rawDir+rawFile+'.exp_rms_factors' ):
                os.system('cp -r '+self.rawDir+rawFile+'.exp_rms_factors '+self.LogDir[key]+'/exp_rms_factors.txt')
        for key in self.antKeys:
            SDH.HtmlInit(self.HtmlDir[key])
        if self.CombineDir is not None:
            SDH.HtmlInitCombine(self.CombineDir)

        #ProcStartTime = time.time()
        #self.LogMessage('INFO', Origin=origin, Msg='readData Start: %s' % time.ctime(ProcStartTime))

        #self.LogMessage('INFO',Origin=origin,Msg='Processing %s in %s'%(self.antFile[self.iterAnt],self.rawFile))

        #TableOutFile=self.LogDir+'/' + self.BF_DATA_LOG
##         (DataTable, listall, Abcissa) = DP.ReadData(self.workDir+self.workFile, self.outDir+self.outFile, self.rawDir+self.rawFile, TableOutFile, self.HtmlDir+'/'+BF_DATA_DIR+'/index.html', self.recipeName)
        #listall = self.engine.ReadData(self.workDir+self.workFile, self.outDir+self.outFile, self.rawDir+self.rawFile, self.contData, TableOutFile, self.HtmlDir+'/'+BF_DATA_DIR+'/index.html', LogFile=self.LogDir+'/'+self.SD_PIPELINE_LOG, recipe=self.recipeName)
        #listall = self.engine.ReadData(self.workDir+self.workFile, self.outDir+self.outFile, self.rawDir+self.rawFile, self.contData, TableOutFile, self.HtmlDir+'/'+BF_DATA_DIR+'/index.html', LogFile=self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG, recipe=self.recipeName)
        workData = {}
        outData = {}
        rawData = {}
        htmlData = {}
        TableOutFile = {}
        productTable = {}
        for key in self.antKeys:
            workData[key] = self.workDir+self.workFile[key]
            outData[key] = self.outDir+self.outFile[key]
            rawData[key] = self.rawDir+self.rawFileList[self.rawFileIdx[key]]
            htmlData[key] = self.HtmlDir[key]+'/'+BF_DATA_DIR+'/index.html'
            TableOutFile[key] = self.LogDir[key]+'/'+self.BF_DATA_LOG
            productTable[key] = self.productName[key].getProductName(ant=self.antName[key])
        listall = self.engine.ReadData(workData, outData, rawData, self.contData, TableOutFile, htmlData, LogFile=self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG, recipe=self.recipeName, productTable=productTable)
        #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )

        # set polarization type
##         self._tb.open(self.workDir+self.workFile)
##         self.poltype=self._tb.getkeyword('POLTYPE').lower()
##         self._tb.close()
        # temporary
        #print 'self.dataSummary[0]=%s'%(self.dataSummary[0])
        self.poltype = self.dataSummary[0]['pol'][0]['type']

        # Setup lists
##         if ROW == 'all': self.ROW = listall[1]
##         else:
##             if type(ROW)==int: self.ROW = [ROW]
##             else: self.ROW = ROW
##         if ROWbase == 'all': self.ROWbase = listall[0]
##         else:
##             if type(ROWbase)==int: self.ROWbase = [ROWbase]
##             else: self.ROWbase = ROWbase
##         if SCAN == 'all': self.SCAN = listall[2]
##         else:
##             if type(SCAN)==int: self.SCAN = [SCAN]
##             else: self.SCAN = SCAN
##         if SCANbase == 'all': self.SCANbase = listall[2]
##         else: 
##             if type(SCANbase)==int: self.SCANbase = [SCANbase]
##             else: self.SCANbase = SCANbase
##         if IF == 'all': self.IF = listall[3]
##         else:
##             if type(IF)==int: self.IF = [IF]
##             else: self.IF = IF
##         if POL == 'all': self.POL = listall[4]
##         else:
##             if type(POL)==int: self.POL = [POL]
##             else: self.POL = POL

        if self.ROW == 'all':
            self.ROW = {}
            for key in self.antKeys:
                self.ROW[key] = listall[key][1] 
        else:
            #if type(self.ROW)==int:
            d = {}
            for key in self.antKeys:
                d[key] = self.ROW
            self.ROW = d
        if self.ROWbase == 'all':
            self.ROWbase = {}
            for key in self.antKeys:
                #self.ROWbase[key] = listall[key][0] 
                self.ROWbase[key] = listall[key][1]
        else:
            #if type(self.ROWbase)==int:
            d = {}
            for key in self.antKeys:
                d[key] = self.ROWbase
            self.ROWbase = d
        if self.SCAN == 'all':
            self.SCAN = {}
            for key in self.antKeys:
                self.SCAN[key] = listall[key][2] 
        else:
            #if type(self.SCAN)==int:
            d = {}
            for key in self.antKeys:
                d[key] = self.SCAN
            self.SCAN = d
        if self.SCANbase == 'all':
            self.SCANbase = {}
            for key in self.antKeys:
                self.SCANbase[key] = listall[key][2] 
        else: 
            #if type(self.SCANbase)==int:
            d = {}
            for key in self.antKeys:
                d[key] = self.SCANbase
            self.SCANbase = d
        if self.IF == 'all':
            self.IF = {}
            for key in self.antKeys:
                self.IF[key] = listall[key][3] 
        else:
            #if type(self.IF)==int:
            d = {}
            for key in self.antKeys:
                d[key] = self.IF
            self.IF = d
        if self.POL == 'all':
            self.POL = {}
            for key in self.antKeys:
                self.POL[key] = listall[key][4] 
        else:
            #if type(self.POL)==int:
            d = {}
            for key in self.antKeys:
                d[key] = self.POL
            self.POL = d
        #self.LogMessage('INFO',Origin=origin,Msg='self.ROW=%s\nself.ROWbase=%s\nself.SCAN=%s\nself.SCANbase=%s\nself.IF=%s\nself.POL=%s'%(self.ROW,self.ROWbase,self.SCAN,self.SCANbase,self.IF,self.POL))


##         self.DataTable=DataTable
##         self.DataTable=self.engine.DataTable
##         self.listall=listall
##         self.Abcissa=Abcissa
        self.Abcissa=self.engine.Abcissa        
        
        #self.iteration = 0
        #self.initial = 0

        #self.processedIFPOL = []

        #self.LinesSPW = {}
        self.Lines = []
        
        # Prepare dummy scantable
        #scan=scantable(self.rawDir+self.rawFile, average=True)
        #sc=DP.RenumASAPData(self.workDir+self.workFile)
        #self.scan=DP.RenumASAPData(self.workDir+self.workFile)
        #DP.RenumASAPData(self.workDir+self.workFile)
        for key in self.antKeys:
            self.engine.RenumASAPData(self.workDir+self.workFile[key])
        #self.scan=sc.get_scan(0)
        #del sc
        #self.GridTable = []
        
        #self.setFileOut(self.LogDir+'/' + self.SD_PIPELINE_LOG )
        #self.setFileOut(self.GlobalLogDir+'/' + self.SD_PIPELINE_LOG )
        #ProcEndTime = time.time()
        #self.LogMessage('INFO', Origin=origin, Msg='readData End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), (ProcEndTime-ProcStartTime)))



    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def createPositionOverviewPlots(self):

        """
        this method creates RADEC and AzEl overview plots to show the observing pattern
        and time/position gaps in the processed observation set.
        """
        origin = 'createPositionOverviwePlots()'

        for key in self.antKeys:

            # 2010/06/29 TN
            # skip if same antenna (tentative)
            if self.BPlotRADEC != False and os.path.exists(self.HtmlDir[key]+'/'+BF_DATA_DIR+'/RADEC.png'):
                return

            ROWbase = self.ROWbase[key]
            ROW = self.ROW[key]

            # to set appropriate radius from self.beamsize
            self.__setRadiusFromBeamSize( key )
            
            # Gap tables are temporally created. Final gaps will be determined in the later stage
            self.LogMessage('INFO', Origin=origin, Msg='Create temporary gap tables')
            (tmpdict, PosGap) = self.engine.GroupByPosition(ROWbase, key, self.radius * 1.0, self.radius / 10.0)
            #self.LogMessage('INFO', Origin=origin, Msg='tmpdict=%s' % tmpdict)
            #self.LogMessage('INFO', Origin=origin, Msg='PosGap=%s' % PosGap)
            (tmptable, TimeGap) = self.engine.GroupByTime(ROWbase, ROWbase, key)
            #self.LogMessage('INFO', Origin=origin, Msg='tmptable=%s' % tmptable)
            #self.LogMessage('INFO', Origin=origin, Msg='TimeGap=%s' % TimeGap)
            (tmptable, TimeGap) = self.engine.MergeGapTables(TimeGap, tmptable, PosGap)
            #self.LogMessage('INFO', Origin=origin, Msg='tmptable=%s' % tmptable)
            #self.LogMessage('INFO', Origin=origin, Msg='TimeGap=%s' % TimeGap)
            #self.LogMessage('INFO', Origin=origin, Msg='len(ROW)=%s' % len(ROW))
            tmpPattern = self.engine.ObsPatternAnalysis(tmpdict, ROW, key, LogFile=self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG, LogLevel=self.LogLevel)
            self.LogMessage('INFO', Origin=origin, Msg='ObsPattern = %s' % tmpPattern)
            del tmpdict, tmptable

            # RA,Dec plot versus MJD
            if self.BPlotRADEC != False: figFile = self.HtmlDir[key]+'/'+BF_DATA_DIR+'/RADEC.png'
            else: figFile = False
            #self.engine.DrawRADEC(ROWbase, connect=True, circle=[self.radius], ObsPattern=tmpPattern, ShowPlot=self.TPlotRADEC, FigFile=figFile)
            self.engine.DrawRADEC(ROW, key, connect=True, circle=[self.radius], ObsPattern=tmpPattern, ShowPlot=self.TPlotRADEC, FigFile=figFile)

            # Az,El plot versus MJD
            if self.BPlotAzEl != False: figFile = self.HtmlDir[key]+'/'+BF_DATA_DIR+'/AzEl.png'
            else: FigFile = False
            self.engine.DrawAzEl(TimeGap, ROWbase, key, ShowPlot=self.TPlotAzEl, FigFile=figFile)

            # Weather and Tsys plot versus MJD
            if self.BPlotWeather != False: figFile = self.HtmlDir[key]+'/'+BF_DATA_DIR+'/Weather.png'
            else: FigFile = False
            WeatherDic = self.getWeather(key,['TEMPERATURE', 'PRESSURE', 'REL_HUMIDITY', 'WIND_SPEED'])
            self.engine.DrawWeather( WeatherDic, ROWbase, key, ShowPlot=self.TPlotWeather, FigFile=figFile)
            del WeatherDic

            # WVR plot versus MJD if it contains WVR data
            WVRdata = self.getWVR( key )
            WVRFreq = self.getWVRFreq( key )
            if type(WVRdata) != bool:
                if self.BPlotWVR != False: figFile = self.HtmlDir[key]+'/'+BF_DATA_DIR+'/WVR.png'
                else: figFile = False
                self.engine.DrawWVR( WVRdata, WVRFreq, ShowPlot=self.TPlotWVR, FigFile=figFile)
                del WVRdata, WVRFreq
                # delete self.WVRFile if exists
                #if self.WVRFile is not None:
                if len(self.WVRFile) != 0:
                    for key in self.WVRFile.keys():
                        self.LogMessage('INFO',Origin=origin,Msg='Deleting WVR file %s ...'%(self.WVRFile[key]))
                        os.system('\\rm -rf %s'%(self.workDir+self.WVRFile[key]))
                    self.WVRFile = {}

    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def writeMSDataCube(self, outFile='default', moments=[0]):

        """
        writeMSData: Write baselined data to disk.
        Additional Parameters
        outFile='default'
        it writes the data cube to the default file name taken from the raw file name or a
        user defined one given as a string here
        """
        origin = 'writeMSDataCube()'
        #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )

        # WriteMSDataCube: Write baselined data to Disk
        # DataTable is not used yet. Flags, RMS, and etc should be added as sub-table.
        # register parameter to paramlist
        self.addparam(moments=moments)

        # check if single antenna or not
        singleAnt = len(NP.unique(self.antName.values())) == 1
        self.LogMessage('DEBUG',Origin=origin,Msg='singleAnt=%s'%(singleAnt))

        # 2010/06/11 TN
        # Loop on antenna
        self.LogMessage('DEBUG',Origin=origin, Msg='outFiles=%s'%self.outFiles)
        self.LogMessage('DEBUG',Origin=origin, Msg='LinesSPW=%s'%self.LinesSPW)
        self.LogMessage('DEBUG',Origin=origin, Msg='gridInfo=%s'%self.gridInfo)
        for iant in self.antKeys:
            if not self.gridInfo.has_key( iant ):
                continue
            if(outFile!='default'):
                outName=outFile.rstrip('/')+'.ant%s'%iant
            else:
                outName = self.outFile[iant]

            # 2009/11/4 write as MS
            # 2010/06/25 modify TELESCOPE_NAME if needed
            # 2011/03/14 TN
            # data product naming convention
            #fileBaselined = self.outDir+self.outFile+'.baseline.MS'
            #fileBaselined = self.productName.getDataName( self.iterAnt, baseline=True )
            fileBaselined = self.productName[iant].getDataName( self.antName[iant], baseline=True )
            s = scantable(self.outDir+outName, average=False)
            antName = s.get_antennaname()
            s.save(fileBaselined, format='MS2', overwrite=True)
            del s
            # IRAM_30m
            if antName.find('30M-')==0:
                self._tb.open(fileBaselined+'/OBSERVATION',nomodify=False)
                self._tb.putcell('TELESCOPE_NAME',0,'IRAM_30m')
                self._tb.flush()
                self._tb.close()
            if antName.find('OSF-')==0:
                self._tb.open(fileBaselined+'/OBSERVATION',nomodify=False)
                self._tb.putcell('TELESCOPE_NAME',0,'OSF')
                self._tb.flush()
                self._tb.close()
            nchan = self.NCHAN

            # if single antenna, FITS name should not contain antenna name
            if singleAnt:
                antName = 'COMBINE'
            else:
                antName = self.antName[iant]
            self.LogMessage('DEBUG',Origin=origin,Msg='antName=%s'%(antName))

            #if ImageCube:
            if self.imageCube and self.Pattern.upper() == 'RASTER':
##                 # CASA Imager output
##                 #radius; OK
##                 #iteration; OK
##                 #spacing; OK
##                 #Lines; OK
##                 #print 'Lines=', Lines
##                 # singleimage =True to make a single data cube for all the lines
##                 # will use max channel width and ignores NChannelMap

##                 #singleimage=False
##                 ##singleimage=True
##                 #(mapCenter, ngridx, ngridy, cellsize, startchans, nchans, chansteps)=SDI.getImParams(fileBaselined, self.Lines, self.radius, self.spacing, singleimage)
##                 #cellx=cellsize
##                 #celly=cellsize
##                 #outImagename=self.outDir+self.outFile+'.bs'
##                 #field=0
##                 #convsupport=3
##                 #spw=0
##                 #moments=[0]
##                 #SDI.MakeImages(fileBaselined, outImagename, ngridx, ngridy, cellx, celly, mapCenter, nchans, startchans, chansteps,field, convsupport, spw, moments, showImage=False)
##                 #SDI.MakeImage(fileBaselined, outImagename+'.image', ngridx, ngridy, cellx, celly, mapCenter, (nchan-self.edge[0]-self.edge[1]), self.edge[0], 1, field, convsupport, spw, moments, showImage=False)

##     ##             (spwids,spwchans)=SDI.getSpectralWindows(fileBaselined)
##                 import SDImaging as SDI
##                 logdir = self.outDir+self.antFile[iant]+'_html'+'/'+BF_LOG_DIR
##                 sdImager = SDI.SDImaging( fileBaselined, self.edge, LogLevel=self.LogLevel, LogFile=self.LogDir+'/'+self.SD_PIPELINE_LOG )
##                 (spwids,spwchans)=sdImager.getSpectralWindows( self.IF )
##                 self.LogMessage('DEBUG', Origin=origin, Msg='spwids = %s, spwchans = %s'%(spwids,spwchans))
##                 for ispw in range(len(spwids)):
##                     if not spwids[ispw] in self.IF:
##                         continue
##                     if spwchans[ispw]==1:
##                         singleimage=True
##                     else:
##                         singleimage=False
##     ##                 (mapCenter, ngridx, ngridy, cellsize, startchans, nchans, chansteps)=SDI.getImParams(fileBaselined, self.LinesSPW[spwids[ispw]], self.radius, self.spacing, singleimage)
##                     sdImager.getImParams(spwids[ispw], self.LinesSPW[iant][spwids[ispw]], self.radius, self.spacing, singleimage)
##     ##                 cellx=cellsize
##     ##                 celly=cellsize
##                     spwname='.spw%s'%spwids[ispw]
##                     outImagename=self.outDir+self.outFile+spwname+'.bs'
##                     field=0
##                     #convsupport=3
##                     convsupport=-1
##                     spw=spwids[ispw]
##                     if singleimage==False:
##                         #moments=[0]
##     ##                     SDI.MakeImages(fileBaselined, outImagename, ngridx, ngridy, cellx, celly, mapCenter, nchans, startchans, chansteps,field, convsupport, spw, self.moments, False, self.LogLevel, BFlog0)
##     ##                     SDI.MakeImage(fileBaselined, outImagename+'.image', ngridx, ngridy, cellx, celly, mapCenter, (spwchans[ispw]-self.edge[0]-self.edge[1]), self.edge[0], 1, field, convsupport, spw, self.moments, False, self.LogLevel, BFlog0)
##                         sdImager.MakeImages(outImagename, field, convsupport, spw, self.moments, False, self.LogLevel, logdir+'/'+self.SD_PIPELINE_LOG)
##                         sdImager.MakeImage(outImagename+'.image', -1, field, convsupport, spw, self.moments, False, self.LogLevel, logdir+'/'+self.SD_PIPELINE_LOG)
##                     else:
##                         moments=[]
##     ##                     SDI.MakeImage(fileBaselined, outImagename+'.image', ngridx, ngridy, cellx, celly, mapCenter, 1, 0, 1, field, convsupport, spw, moments, False, self.LogLevel, BFlog0)
##                         sdImager.MakeImage(outImagename+'.image', 0, field, convsupport, spw, moments, False, self.LogLevel, logdir+'/'+self.SD_PIPELINE_LOG)
##                 del sdImager
                ###
                # Create CASA image from regridded spectra using SDImaging2
                #
                # SDImaging2 doesn't create images from baselined MS.
                # Instead, it creates images from regridded spectra
                # using ia tool.
                #
                # 2010/11/5 TN
                ###
                import SDImaging as SDI
                outImagename=self.outDir+outName
                for iIF in self.gridInfo[iant].keys():
                    files={}
                    cfiles={}
                    for iPOL in self.gridInfo[iant][iIF]:
                        if type(iPOL)!=str:
                            files[iPOL]=self.gridInfo[iant][iIF][iPOL]
                        if type(iPOL)==str and iPOL.find('cont') != -1:
                            key=int(iPOL[4:])
                            cfiles[key]=self.gridInfo[iant][iIF][iPOL]
                    if len(cfiles) == 0:
                        cfiles = None
                    self.LogMessage('DEBUG',Origin=origin,Msg='spfiles=%s'%files)
                    self.LogMessage('DEBUG',Origin=origin,Msg='contfiles=%s'%cfiles)
                    #sdimager=SDI.SDImaging2(spfiles=files,contfiles=cfiles,edge=self.edge,LogLevel=self.LogLevel, LogFile=self.LogDir+'/'+self.SD_PIPELINE_LOG)
                    sdimager=SDI.SDImaging2(spfiles=files,contfiles=cfiles,edge=self.edge,LogLevel=self.LogLevel,ConsoleLevel=self.ConsoleLevel,LogFile=self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG)
                    if not self.isConsoleOutputEnabled():
                        sdimager.disableConsoleOutput()
                        
                    # 2011/03/15 TN
                    # data product naming convention
                    sdimager.setProductName( self.productName[iant] )
                    #sdimager.setAntennaId( iant )
                    #sdimager.setAntennaName( self.antName[iant] )
                    sdimager.setAntennaName( antName )
                    sdimager.setPolType( self.poltype )
                    sdimager.setSpectralWindowId( iIF )
                    linesspw = self.LinesSPW[iIF]
                    self.LogMessage('DEBUG',Origin=origin,Msg='linesspw=%s'%linesspw)
                    sdimager.setimparams( nx=self.gridInfo[iant][iIF]['grid'][0], ny=self.gridInfo[iant][iIF]['grid'][1], cell=self.gridInfo[iant][iIF]['cell'], center=self.gridInfo[iant][iIF]['center'],lines=linesspw )
                    sdimager.fromfile( outprefix=outImagename )
                    #sdimager.moments( moments=self.moments )
                    sdimager.moments( moments=moments )
                    #sdimager.createfits( dirname=self.outDir, prefix=self.outFile )
                    del sdimager
        #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )
        #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )

        # 2012/03/07 TN
        # combined data
        if self.GridTableComb is None:
            self.LogMessage('INFO',Origin=origin,Msg='Skip %s since no combined data available'%(origin))
            return

        self.LogMessage('DEBUG',Origin=origin,Msg='self.outGridComb=%s'%(self.outGridComb))
        self.LogMessage('DEBUG',Origin=origin,Msg='self.contGridComb=%s'%(self.contGridComb))

        for ifno in self.outGridComb.keys():
            spfiles = self.outGridComb[ifno]
            if self.contGridComb.has_key(ifno):
                contfiles = self.contGridComb[ifno]
            else:
                contfiles = None
            sdimager=SDI.SDImaging2(spfiles=spfiles,contfiles=contfiles,edge=self.edge,LogLevel=self.LogLevel,ConsoleLevel=self.ConsoleLevel,LogFile=self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG)
            if not self.isConsoleOutputEnabled():
                sdimager.disableConsoleOutput()
            sdimager.setProductName( self.productName[0] )
            #sdimager.setAntennaId( iant )
            sdimager.setAntennaName( 'COMBINE' )
            sdimager.setPolType( self.poltype )
            sdimager.setSpectralWindowId( ifno )
            linesspw = self.LinesSPW[ifno]
            sdimager.setimparams( nx=self.gridInfo[0][ifno]['grid'][0], ny=self.gridInfo[0][ifno]['grid'][1], cell=self.gridInfo[0][ifno]['cell'], center=self.gridInfo[0][iIF]['center'],lines=linesspw )
            sdimager.fromfile()
            #sdimager.moments( moments=self.moments )
            sdimager.moments( moments=moments )
            del sdimager

        # 2009/11/4 Comment out (moved to regridData)
        ## WriteNewMSData: Write final output to Disk
        #FileReGrid = self.outDir+self.outFile+'.grid.MS'
        #
        #DP.WriteNewMSData(self.lastData, FileReGrid, self.FinalSP, self.FinalTable, outform='ASAP', LogLevel=self.LogLevel, LogFile=BFlog0)
        #DP.WriteNewMSData(self.lastData, FileReGrid, self.FinalSP, self.FinalTable, outform='MS2', LogLevel=self.LogLevel, LogFile=BFlog0)

        #casalog.post('WriteNewMSData End: %s (ElapseTime=%.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime),origin='SDPipeline/Engines' )
        #if ImageCube:
        #if self.imageCube and self.Pattern.upper() == 'RASTER':
        #    # CASA Imager output
        #    #radius; OK
        #    #iteration; OK
        #    #spacing; OK
        #    #Lines; OK
        #    #print 'Lines=', Lines
        #    # singleimage =True to make a single data cube for all the lines
        #    # willl use max channel width and ignores NChannelMap
        #    singleimage=False
        #    #singleimage=True
        #    (mapCenter, ngridx, ngridy, cellsize, startchans, nchans, chansteps)=SDI.getImParams(FileReGrid, self.Lines, self.radius, self.spacing, singleimage)
        #    cellx=cellsize
        #    celly=cellsize
        #    outImagename=self.outDir+self.outFile+'.gd'
        #    field=0
        #    convsupport=-1
        #    spw=0
        #    moments=[0]
        #    SDI.MakeImages(FileReGrid, outImagename, ngridx, ngridy, cellx, celly, mapCenter, nchans, startchans, chansteps,field, convsupport, spw, moments, showImage=False)
        #    SDI.MakeImage(FileReGrid, outImagename+'.image', ngridx, ngridy, cellx, celly, mapCenter, (nchan-self.edge[0]-self.edge[1]), self.edge[0], 1, field, convsupport, spw, moments, showImage=False)

        
    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def groupData(self,SWconfig):
    #def groupData(self,vIF,vPOL):

        """
        groupData : groups the data by position. It works for a given IF and Polarization.
        parameter : SWconfig = [ [vAnt, vIF, [vPOLs]], [vAnt, vIF, [vPOLs]], ..., [vAnt, vIF, [vPOLs]] ]
        self.rowsSel: [ [ [sel0Pol0], [sel0Pol1],.., [sel0Poln] ], [ [sel1Pol0],...,[sel1Poln] ],...[[],[]] ]
        """
        origin = 'groupData()'

        self.LogMessage('DEBUG',Origin=origin,Msg='SWconfig=%s'%(SWconfig))

        # 2011/10/23 GK initialization
        self.AntIndex = {}
        self.vIF = []
        self.vPOL = []
        self.rowsSel = []
        self.rowsidSel = []
        self.basedataSel = []
        self.basedataidSel = []
        self.PosDictAll = []
        self.PosDict = []
        self.TimeTable = []
        self.TimeGap = []
        idx = 0

        # 2011/10/26 TN
        # it's enough by checking whether first antenna is TP or not
        tmpAntIdx = self.rawFileIdx[SWconfig[0][0]]
        tmpIF = SWconfig[0][1]
        self.isTP = self.dataSummary[tmpAntIdx]['spw'][tmpIF]['type'] is 'TP'
        
        # 2011/10/23 GK needs consideration of the following working files
        for key in self.antKeys:
            #self.dtctData[key] = self.workDir+self.workFile[key]
            self.lastData[key] = self.workDir+self.workFile[key]
        # 2011/10/23 GK add dual roop structure for the change of getLoopElements()

        # 2012/03/01 TN
        # set appropriate beam size
        self.__setRadiusFromBeamSize( tmpAntIdx, tmpIF )
        
        print 'SWconfig', SWconfig
        for (vAnt, vIF, vPOLs) in SWconfig:
            print 'vAnt, vIF, vPOLs', vAnt, vIF, vPOLs
            self.AntIndex[vAnt] = idx
            self.vIF.append(vIF)
            self.vPOL.append(vPOLs)
            rowsSelTmp = []
            rowsidSelTmp = []
            basedataSelTmp = []
            basedataidSelTmp = []
            TimeTableTmp = []
            TimeGapTmp = []
            PosDictAllTmp = []
            PosDictTmp = []
            TimeTableTmp = []
            TimeGapTmp = []
            LogDir = self.LogDir[vAnt]
            WorkDir = self.workDir+self.workFile[vAnt]
            for vPOL in vPOLs:
                print 'vPOL', vPOL
                # 2011/10/23 GK commented out 2 lines below
                #self.vIF = vIF
                #self.vPOL = vPOL
                self.iteration = 0
                # 2011/10/23 GK commented out 6 lines below
                #self.rowsSel = []
                #self.rowsidSel = []
                #self.basedataSel = []
                #self.basedataidSel = []
                #self.dtctData = self.workDir+self.workFile
                #self.lastData = self.workDir+self.workFile
                # add History
                self.processedStages.append((self.vIF, self.vPOL, 'groupData', self.iteration))
                ROWbase = NP.array(self.ROWbase[vAnt])
                ROW = NP.array(self.ROW[vAnt])
                SCANbase = NP.array(self.SCANbase[vAnt])
                SCAN = NP.array(self.SCAN[vAnt])
                #(self.basedataSel, self.basedataidSel, self.rowsSel, self.rowsidSel) = self.engine.selectGroupData(vAnt, vIF, vPOL, ROWbase, ROW, SCANbase, SCAN )
                (basedataSel, basedataidSel, rowsSel, rowsidSel) = self.engine.selectGroupData(vAnt, vIF, vPOL, ROWbase, ROW, SCANbase, SCAN )
               
                self.LogMessage('DEBUG',Origin=origin,Msg= 'len(rowsSel)=%s'%(len(rowsSel)))
                if len(rowsSel) != 0: self.tmp = True
                else:
                    self.tmp = False
                    return
                basedataSelTmp.append(basedataSel)
                basedataidSelTmp.append(basedataidSel)
                rowsSelTmp.append(rowsSel)
                rowsidSelTmp.append(rowsidSel)

                # 2009/10/19 Check if it is a TP data
##                 if self.DataTable[self.rowsSel[0]][13] == 1:
##                     self.isTP = True
##                 else:
##                     self.isTP = False
                #self.isTP = self.engine.isTP( self.rowsSel[0] )
                #self.isTP = self.dataSummary[idx]['spw'][vIF]['type'] is 'TP'
        
                # GroupByPosition: grouping the data by their positions.
                #  PosTable: output table consists of information i.e.,
                #   ID, IDs associated to the ID (within the radius)
                #(self.PosDictAll, PosGap) = self.engine.GroupByPosition(self.basedataidSel, self.radius * 1.0, self.radius / 10.0, LogFile=self.LogDir+'/'+self.BF_GROUP_LOG)
                #(PosDictAll, PosGap) = self.engine.GroupByPosition(basedataidSel, self.radius * 1.0, self.radius / 10.0, LogFile=LogDir+'/'+self.BF_GROUP_LOG,LogLevel=self.LogLevel)
                (PosDictAll, PosGap) = self.engine.GroupByPosition(basedataSel, vAnt, self.radius * 1.0, self.radius / 10.0, LogFile=LogDir+'/'+self.BF_GROUP_LOG,LogLevel=self.LogLevel)
                PosDictAllTmp.append(PosDictAll)

                #(self.PosDict, PosGapTmp) = self.engine.GroupByPosition(self.rowsidSel, self.radius * 1.0, self.radius / 10.0, LogFile=self.LogDir+'/'+self.BF_GROUP_LOG)
                #(PosDict, PosGapTmp) = self.engine.GroupByPosition(rowsidSel, self.radius * 1.0, self.radius / 10.0, LogFile=LogDir+'/'+self.BF_GROUP_LOG)
                (PosDict, PosGapTmp) = self.engine.GroupByPosition(rowsSel, vAnt, self.radius * 1.0, self.radius / 10.0, LogFile=LogDir+'/'+self.BF_GROUP_LOG)
                PosDictTmp.append(PosDict)

                #self.Pattern = self.engine.ObsPatternAnalysis(self.PosDictAll, self.basedataSel, LogFile=self.LogDir+'/'+self.BF_GROUP_LOG)
                if idx==0: self.Pattern = self.engine.ObsPatternAnalysis(PosDictAll, basedataSel, vAnt, LogFile=LogDir+'/'+self.BF_GROUP_LOG, LogLevel=self.LogLevel)
                
                # GroupByTime: grouping the data by time lag to the next integration
                #(TimeTable, TimeGap) = self.engine.GroupByTime(self.basedataidSel, self.rowsidSel, LogFile=self.LogDir+'/'+self.BF_GROUP_LOG)
                #(TimeTable, TimeGap) = self.engine.GroupByTime(basedataidSel, rowsidSel, LogFile=LogDir+'/'+self.BF_GROUP_LOG)
                (TimeTable, TimeGap) = self.engine.GroupByTime(basedataSel, rowsSel, vAnt, LogFile=LogDir+'/'+self.BF_GROUP_LOG)

                # Merge Gap Tables
                #(self.TimeTable, self.TimeGap) = self.engine.MergeGapTables(TimeGap, TimeTable, PosGap, LogFile=self.LogDir+'/'+self.BF_GROUP_LOG)
                (sTimeTable, sTimeGap) = self.engine.MergeGapTables(TimeGap, TimeTable, PosGap, LogFile=LogDir+'/'+self.BF_GROUP_LOG)
                TimeTableTmp.append(sTimeTable)
                TimeGapTmp.append(sTimeGap)
                #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )
                #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )

                # 2009/10/29 Create dummy scantable
                # Bugs in scantable: sometimes 'RuntimeError' occurs
                #scan = scantable(self.getRawDirectory()+self.getRawFile(), average=False)
                #scan = scantable( self.workDir+self.workFile, average=False )
                scan = scantable( WorkDir, average=False )
                #self.NCHAN = len(scan._getspectrum(self.rowsSel[0]))
                if idx==0: self.NCHAN = len(scan._getspectrum(rowsSel[0]))
                del scan
            self.basedataSel.append(basedataSelTmp)
            self.basedataidSel.append(basedataidSelTmp)
            self.rowsSel.append(rowsSelTmp)
            self.rowsidSel.append(rowsidSelTmp)
            self.PosDictAll.append(PosDictAllTmp)
            self.PosDict.append(PosDictTmp)
            self.TimeTable.append(TimeTableTmp)
            self.TimeGap.append(TimeGapTmp)
            del basedataSelTmp, basedataidSelTmp, rowsSelTmp, rowsidSelTmp, basedataSel, basedataidSel, rowsSel, rowsidSel
            del PosDictAllTmp, PosDictAll, PosDictTmp, PosDict, TimeTableTmp, TimeTable, TimeGapTmp, TimeGap
            idx += 1
                       

    #@dec_stages_interface_logfile
    #@dec_stages_interface_logging
    def subtractBaselineAtRasterEdgeRunThrough(self, SWconfig=None):
        """
        """
        origin = 'subtractBaselineAtRasterEdgeRunThrough'

        (vAnt,vIF,vPOLs) = self.__configureListFromSWConfig(SWconfig)

        # iterative call of subtractBaselineAtRasterEdge
        nIter = len(vAnt)
        self.LogMessage('INFO',Origin=origin,Msg='Start iterative call: nIter=%s'%(nIter))
        for i in xrange(nIter):
            nPol = len(vPOLs[i])
            self.LogMessage('INFO',Origin=origin,Msg='nPol=%s'%(nPol))
            for j in xrange(nPol):
                self.subtractBaselineAtRasterEdge( vAnt[i], vIF[i], vPOLs[i][j])
        self.LogMessage('INFO',Origin=origin,Msg='End iterative call: nIter=%s'%(nIter))

    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def subtractBaselineAtRasterEdge(self, vAnt=None, vIF=None, vPOL=None):

        """
        Subtract baseline which is calculated from the edge of the raster map
        It works for a given IF and Polarization.
        parameter : vIF, vPOL
        """
        origin = 'subtractBaselineAtRasterEdge()'
        
        ProcStartTime = time.time()

        (vAnt,vIF,vPOL,antIdx,polIdx) = self.__handleIndex(vAnt,vIF,vPOL)

        # 2009/10/19 for TP data: return: => 2009/11/1 anyway do it for TP data
        #if self.isTP:
        #    SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='subtractBaselineAtRasterEdge was skipped because it is a TP data')
        #    BFlog0.close()
        #    return

        # set up something here...
        LogDir = self.LogDir[vAnt]
        TimeTable = self.TimeTable[antIdx][polIdx]
        TimeGap = self.TimeGap[antIdx][polIdx]
        initData = self.initData[vAnt]
        workFile = self.workFile[vAnt]
        basedataidSel = self.basedataidSel[antIdx][polIdx]
        self.LogMessage('DEBUG',Origin=origin, Msg='initData=%s\nworkFile=%s'%(initData,workFile))

        self.LogMessage('INFO', Origin=origin, Msg='Subtract baseline which is calculated from the edge of the raster map')
##         BF.BaselineSubAtRasterEdge(self.initData, self.workDir+self.workFile, self.DataTable, self.TimeTable, self.basedataidSel, self.FitOrderRuleDictionary, self.SwitchingRuleDictionary, LogLevel=self.LogLevel, LogFile=BFlog)
        self.engine.BaselineSubAtRasterEdge(initData, self.workDir+workFile, TimeTable, basedataidSel, self.FitOrderRuleDictionary, self.SwitchingRuleDictionary, LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_GROUP_LOG)
        #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )
        #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )
        
        # 2009/10/15 WeightRMS should be False
        self.GridRuleDictionary['WeightRMS'] = False
        self.initData[vAnt] = self.workDir+workFile
        self.lastData[vAnt] = self.workDir+workFile

        # add History
        self.processedStages.append((self.vIF, self.vPOL, 'subtractBaselineAtRasterEdge', self.iteration))
        # Save Meta Data
        self.saveMetaData(self.workDir+workFile+'.meta')

                 
    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def spectralLineDetect(self, vIF=None, vPOL=None):

        """
        spectralLineDetect: detects all possible line features in the selected spec 
        It works for a given IF and Polarization.

        parameter: vIF, vPOL
        the method utilizes Process3, which detects all possible line features

        """
        origin = 'spectralLineDetect()'
        LogDir = self.LogDir[self.LogDir.keys()[0]]

        # for multiple antenna input
        if vIF == None: vIF = self.vIF[0]
        if vPOL == None: vPOL = self.vPOL[0]
        #if vIF == None: vIF = self.vIF
        #if vPOL == None: vPOL = self.vPOL
        
        # Process3: detects all possible line features
        #  scan: dummy scantable
        #  Threshold: detection threshold
        #  DetectSignal = {ID1,[RA,DEC,[[LineStrChan1, LineEndChan1],
        #                               [LineStrChan2, LineEndChan2],
        #                               [LineStrChanN, LineEndChanN]]],
        #                  IDn,[RA,DEC,[[LineStrChan1, LineEndChan1],
        #                               [LineStrChanN, LineEndChanN]]]}

        # 2011/10/26 TN
        # the following line must be put in simpleGridding()
        #self.iteration += 1

        # 2011/10/25 GK comment out
        # for multiple iteration
        #if ( [self.iterAnt,vIF,vPOL] not in self.processedIFPOL ):
        #    self.ResultTable = None
        #    self.processedIFPOL.append( [self.iterAnt,vIF,vPOL] )
        #self.processedIFPOL.append( [self.iterAnt,vIF,vPOL] )
        self.processedIFPOL.append( [vIF,vPOL] )
    
        # 2009/10/19 for TP data
        if self.isTP:
            self.LogMessage('INFO', Origin=origin, Msg='spectralLineDetect was skipped because it is a TP data')
            return

        # for Pre-Defined Spectrum Window
        # SpectrumWindow format: [[CenterFreq(GHz), minVel(Km/s), maxVel(Km/s)],[CF,,],,,]
        # SpWin format: [CenterChannel, FullWidthChannel]
        # SpWinRange format: [channel0, channel1]
        # 2010/6/9 add ChannelWindow (pre-defined window by channel)
        if len(self.ChannelWindow) != 0:
##             self.SpectrumWindow = SDT.Channel2FreqWindow(self.Abcissa[vIF], self.ChannelWindow)
            self.SpectrumWindow = self.engine.Channel2FreqWindow(vIF, self.ChannelWindow)
            #print 'SpectrumWindow is Calculated to be:', self.SpectrumWindow
        # 2009/10/29 Predefined window
##         (SpWin, SpWinRange) = SDT.FreqWindow2Channel(self.Abcissa[vIF], self.SpectrumWindow)
        (SpWin, SpWinRange) = self.engine.FreqWindow2Channel(vIF, self.SpectrumWindow)
        # Comment the following 5 lines if one want to add detected lines to the pre-defined window
        #if len(SpWinRange) != 0:
        #    SDT.LogMessage('INFO', self.LogLevel, BFlog0, Msg='spectralLineDetect stage was skipped because Spectrum Window was manually set')
        #    BFlog0.close()
        #    return

        # 2011/03/24 TN
        # to support multi-iteration for predefined line window
        #print '\nself.ResultTable', self.ResultTable
        if self.ResultTable == None:
            self.ResultTable = []
            rows = self.rowsSel
        elif len(SpWinRange) != 0:
            #rows = self.rowsSel
            rows = range(len(self.ResultTable))
        else:
            rows = range(len(self.ResultTable))

        self.LogMessage('INFO', Origin=origin, Msg='Line detection')
        self.LogMessage('DEBUG', Origin=origin, Msg='SpWinRange=%s'%(SpWinRange))
##         self.DetectSignal = BF.Process3(self.dtctData, self.DataTable, self.ResultTable, self.PosDict, rows, self.NCHAN, self.LineFinderRuleDictionary, SpWinRange, Threshold=3, edge=self.edge, BroadComponent=self.broadComponent, LogLevel=self.LogLevel, LogFile=BFlog)
        self.DetectSignal = self.engine.Process3(self.dtctData, self.ResultTable, self.PosDict, rows, self.NCHAN, self.LineFinderRuleDictionary, SpWinRange, edge=self.edge, BroadComponent=self.broadComponent, LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_DETECT_LOG)
        #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )
        #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )

        # add History
        self.processedStages.append((vIF, vPOL, 'spectralLineDetect', self.iteration))
        #self.processedStages.append((self.vIF, self.vPOL, 'spectralLineDetect', self.iteration))
        # Save Meta Data
        self.saveMetaData(self.dtctData+'.meta')

        self.LogMessage('DEBUG', Origin=origin, Msg='DetectSignal = %s' % self.DetectSignal)
            

    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def spatialLineDetect(self, vIF=None, vPOL=None, createResultPlots=None):

        """
        spatialLineDetect: evaluate lines detected in the previous process by
        the clustering-analysis.
        It works for a given IF and Polarization.

        parameter: vIF, vPOL
        optional parameter: createResultPlots=True/False/None(default)

        the method utilizes Process4

        """
        origin = 'spatialLineDetect()'

        LogDir = self.LogDir[self.AntIndex.keys()[0]]
        #HtmlDir = self.HtmlDir[self.AntIndex.keys()[0]]
        HtmlDir = self.HtmlTopDir+'/'+self.HtmlCluster
        self.addparam( plotcluster=createResultPlots )

        if vIF == None: vIF = self.vIF[0]
        if vPOL == None: vPOL = self.vPOL[0]

        # serialize self.rowsidSel
        idxList = []
        #print 'self.rowsidSel=%s'%(self.rowsidSel)
        for i in xrange(len(self.rowsidSel)):
            for j in xrange(len(self.rowsidSel[i])):
                idxList += self.rowsidSel[i][j]
        
        # 2009/10/19 for TP data
        if self.isTP:
            self.LogMessage('INFO', Origin=origin, Msg='spatialLineDetect was skipped because it is a TP data')
            if not (vIF in self.LinesSPW.keys()):
                self.LinesSPW[vIF] = []
            return
        
        # for Pre-Defined Spectrum Window
        # SpectrumWindow format: [[CenterFreq(GHz), minVel(Km/s), maxVel(Km/s)],[CF,,],,,]
        # SpWin format: [CenterChannel, FullWidthChannel]
        # SpWinRange format: [channel0, channel1]
        # 2009/10/29 Predefined window
        #if len(self.SpectrumWindow) > 0: return
##         (SpWin, SpWinRange) = SDT.FreqWindow2Channel(self.Abcissa[vIF], self.SpectrumWindow)
        (SpWin, SpWinRange) = self.engine.FreqWindow2Channel(vIF, self.SpectrumWindow)

        self.LogMessage('INFO', Origin=origin, Msg='Cluster analysis')
             
        if createResultPlots == True or (createResultPlots == None and self.BPlotCluster != False): FigFileDir = HtmlDir+'/'+BF_CLUSTER_PDIR+'/'
        else: FigFileDir = False

##         self.Lines = BF.Process4(self.DetectSignal, self.DataTable, self.ResultTable, self.Abcissa[vIF], self.rowsSel, SpWin, self.Pattern, self.clusterGridSize, self.clusterGridSize,1, Nsigma=4.0, Xorder=-1, Yorder=-1, BroadComponent=self.broadComponent, ClusterRule=self.ClusterRuleDictionary, ShowPlot=self.TPlotCluster, FigFileDir=FigFileDir, FigFileRoot='Clstr_%s_%s_%s' % (vIF, vPOL,self.iteration), LogLevel=self.LogLevel, LogFile=BFlog)
        self.Lines = self.engine.Process4(self.ResultTable, vIF, idxList, SpWin, self.Pattern, self.clusterGridSize, self.clusterGridSize,1, Nsigma=3.0, Xorder=-1, Yorder=-1, BroadComponent=self.broadComponent, ClusterRule=self.ClusterRuleDictionary, LineFinderRule=self.LineFinderRuleDictionary, ShowPlot=self.TPlotCluster, FigFileDir=FigFileDir, FigFileRoot=('Clstr_%s_%s_%s' % (vIF, vPOL,self.iteration)).replace(' ',''), LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_CLUSTER_LOG)
        #self.Lines = self.engine.Process4(self.ResultTable, vIF, self.rowsSel[0], SpWin, self.Pattern, self.clusterGridSize, self.clusterGridSize,1, Nsigma=3.0, Xorder=-1, Yorder=-1, BroadComponent=self.broadComponent, ClusterRule=self.ClusterRuleDictionary, LineFinderRule=self.LineFinderRuleDictionary, ShowPlot=self.TPlotCluster, FigFileDir=FigFileDir, FigFileRoot='Clstr_%s_%s_%s' % (vIF, vPOL,self.iteration), LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_CLUSTER_LOG)
        dest = self.HtmlDir.values()
        if self.CombineDir is not None:
            dest.append(self.CombineDir)
        SDH.HtmlClustering(HtmlDir,DestList=dest)
        #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )
        #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )

        if not (vIF in self.LinesSPW.keys()):
            self.LogMessage('DEBUG',Origin=origin,Msg='Setting self.LinesSPW: IF=%d, Itr=%d'%(vIF,self.iteration))
            self.LinesSPW[vIF] = self.Lines
        else:
            self.LogMessage('DEBUG',Origin=origin,Msg='Updating self.LinesSPW: IF=%d, Itr=%d'%(vIF,self.iteration))
            self.LinesSPW[vIF] = self.Lines
            
                
            
        # add History
        self.processedStages.append((self.vIF, self.vPOL, 'spatialLineDetect', self.iteration))
        # Save Meta Data
        self.saveMetaData(self.dtctData+'.meta')

    #@dec_stages_interface_logfile
    #@dec_stages_interface_logging
    def removeBaselineRunThrough(self, SWconfig=None, createResultPlots=None, fitOrder='automatic', fitFunc='polynomial'):
        """
        """
        origin = 'removeBaselineRunThrough'

        # 2011/11/15 TN
        # Skip if TP data here instead of each iterative call
        # Also skip if predefined window is set and second iteration or later 
        if self.isTP:
            self.LogMessage('INFO', Origin=origin, Msg='removeBaseline was skipped because it is a TP data')
            return
        if len(self.SpectrumWindow) != 0 and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='removeBaseline was skipped because spectrum window is set and iteration > 1')
            return

        (vAnt,vIF,vPOLs) = self.__configureListFromSWConfig(SWconfig)

        # iterative call of removeBaseline 
        nIter = len(vAnt)
        self.LogMessage('INFO',Origin=origin,Msg='Start iterative call: nIter=%s'%(nIter))
        for i in xrange(nIter):
            nPol = len(vPOLs[i])
            self.LogMessage('INFO',Origin=origin,Msg='nPol=%s'%(nPol))
            for j in xrange(nPol):
                self.removeBaseline( vAnt[i], vIF[i], vPOLs[i][j], createResultPlots, fitOrder, fitFunc )
        self.LogMessage('INFO',Origin=origin,Msg='End iterative call: nIter=%s'%(nIter))

    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def removeBaseline(self, vAnt=None, vIF=None, vPOL=None, createResultPlots=None, fitOrder='automatic', fitFunc='polynomial'):

        """
        removeBaseline: determin fitting order for polynomial baseline fit from
        the dominated frequency determined by FFT and remove a baseline using this fit order.
        Optional one can set the fit order using an optional parameter.
        
        evaluate lines detected in the previous process by
        the clustering-analysis.
        It works for a given IF and Polarization.

        parameter: vAnt, vIF, vPOL
        optional parameter: createResultPlots=False/True/None(default)
                            fitOrder='automatic' (otherwise it expects an integer)
                            fitFunc='polynomial' (fitting function: 'polynomial' or 'spline')

        the method utilizes Process5 and Process6

        """
        origin = 'removeBaseline()'

        self.addparam( fitorder=fitOrder,
                       fitfunc=fitFunc )

        (vAnt,vIF,vPOL,antIdx,polIdx) = self.__handleIndex(vAnt,vIF,vPOL)
 
        # 2009/10/19 for TP data
        if self.isTP:
            self.LogMessage('INFO', Origin=origin, Msg='removeBaseline was skipped because it is a TP data')
            return

        # 2009/10/29 Predefined window
        if len(self.SpectrumWindow) != 0 and self.iteration > 1: return

        self.LogMessage('INFO', Origin=origin, Msg='Fitting Function: %s' % fitFunc)
        self.LogMessage('INFO', Origin=origin, Msg='Fit order determination and baseline subtraction')

        # set up something here...
        LogDir = self.LogDir[vAnt]
        TimeTable = self.TimeTable[antIdx][polIdx]
        TimeGap = self.TimeGap[antIdx][polIdx]
        initData = self.initData[vAnt]
        #rowsSel = self.rowsSel[antIdx][polIdx]
        rowsSel = self.rowsidSel[antIdx][polIdx]
        if self.continuumOutput == True: contData = self.contData[vAnt]
        else: contData = None
        outFile = self.outFile[vAnt]
        productTable = self.productName[vAnt].getProductName(ant=self.antName[vAnt])
        self.LogMessage('DEBUG',Origin=origin,Msg='rowsSel=%s'%(rowsSel))

        if(fitOrder!='automatic'):
            self.FitOrderRuleDictionary['MaxPolynomialOrder'] = fitOrder
            self.FitOrderRuleDictionary['PolynomialOrder'] = fitOrder
            self.LogMessage('INFO', Origin=origin, Msg='fit order preset to %i' %(fitOrder))
##         fitOrder = BF.Process5(self.initData, self.DataTable, self.TimeTable, self.rowsSel, self.NCHAN, edge=self.edge, FitOrderRule=self.FitOrderRuleDictionary, LogLevel=self.LogLevel, LogFile=BFlog)
        fitOrder = self.engine.Process5(initData, TimeTable, rowsSel, self.NCHAN, edge=self.edge, FitOrderRule=self.FitOrderRuleDictionary, LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_FITORDER_LOG)
        #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )
        #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )
        #print 'fitOrder calculated ',fitOrder,type(fitOrder)
        #ProcEndTime = time.time()
        
        FitTableOut = self.outDir+outFile+'.fit.tbl'
        #ProcStartTime = time.time()

        self.LogMessage('DEBUG',Origin=origin,Msg='fitOrder=%s'%(fitOrder))
            
	# 2010/6/12 Plot for the baseline subtraction was separated
        #if createResultPlots == True or (createResultPlots == None and self.BPlotFit != False): FigFileDir = self.HtmlDir+'/'+BF_FIT_PDIR+'/'
        #if createResultPlots == True or (createResultPlots == None and self.BPlotFit != False): FigFileDir = self.HtmlDir+'/'+BF_FIT_PDIR+'/'
        #else: FigFileDir = False
##         BF.Process6(self.initData, self.outDir+self.outFile, self.DataTable, self.rowsSel, self.NCHAN, fitOrder, self.Abcissa[vIF], self.TimeGap, showevery=100, savefile=FitTableOut, edge=self.edge, FitOrderRule=self.FitOrderRuleDictionary, ShowRMS=self.TPlotIntermediate, ShowPlot=self.TPlotFit, DebugPlot=self.TPlotDebug, FigFileDir=FigFileDir, FigFileRoot='Fit_%s_%s_%s' % (vIF, vPOL,self.iteration), LogLevel=self.LogLevel, LogFile=BFlog)
        #self.engine.Process6(self.initData, self.outDir+self.outFile, self.rowsSel, self.NCHAN, fitOrder, vIF, self.TimeGap, showevery=100, savefile=FitTableOut, edge=self.edge, FitOrderRule=self.FitOrderRuleDictionary, ShowRMS=self.TPlotIntermediate, ShowPlot=self.TPlotFit, DebugPlot=self.TPlotDebug, FigFileDir=FigFileDir, FigFileRoot='Fit_%s_%s_%s' % (vIF, vPOL,self.iteration), LogLevel=self.LogLevel, LogFile=self.LogDir+'/'+self.BF_FIT_LOG)
        self.engine.Process6(initData, self.outDir+outFile, contData, rowsSel, self.NCHAN, fitOrder, fitFunc, vIF, TimeGap, edge=self.edge, FitOrderRule=self.FitOrderRuleDictionary, LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_FIT_LOG,productTable=productTable)
        #if createResultPlots == True or (createResultPlots == None and self.BPlotFit != False):
        #    FigFileDir = self.HtmlDir+'/'+BF_FIT_PDIR+'/'
        #    self.engine.DrawFitSpectrum(self.initData, self.outDir+self.outFile, self.rowsSel, self.NCHAN, vIF, edge=self.edge, HandleFlag=False, LogLevel=self.LogLevel, LogFile=self.LogDir+'/'+self.BF_FIT_LOG, FigFileDir=FigFileDir, FigFileRoot='Fit_%s_%s_%s' % (vIF, vPOL,self.iteration))
        #SDH.HtmlBaselineFit(self.HtmlDir)
        #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )
        #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )

        self.lastData[vAnt] = self.outDir+outFile

        # add History
        self.processedStages.append((self.vIF, self.vPOL, 'removeBaseline', self.iteration))
        # Save Meta Data
        self.saveMetaData(self.lastData[vAnt]+'.meta')


    #@dec_stages_interface_logfile
    #@dec_stages_interface_logging
    def flagDataRunThrough(self, SWconfig=None, UserFlag=[], createResultPlots=None, FlagRule=[]):
        """
        """
        origin = 'flagDataRunThrough'

        # 2011/11/15 TN
        # Skip if TP data here instead of each iterative call
        # Also skip if predefined window is set and second iteration or later 
        if self.isTP:
            self.LogMessage('INFO', Origin=origin, Msg='flagData was skipped because it is a TP data')
            return
        if len(self.SpectrumWindow) != 0 and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='flagData was skipped because spectrum window is set and iteration > 1')
            return

        (vAnt,vIF,vPOLs) = self.__configureListFromSWConfig(SWconfig)

        # iterative call of flagData 
        nIter = len(vAnt)
        self.LogMessage('INFO',Origin=origin,Msg='Start iterative call: nIter=%s'%(nIter))
        for i in xrange(nIter):
            nPol = len(vPOLs[i])
            self.LogMessage('INFO',Origin=origin,Msg='nPol=%s'%(nPol))
            for j in xrange(nPol):
                self.flagData( vAnt[i], vIF[i], vPOLs[i][j], UserFlag, createResultPlots, FlagRule )
        self.LogMessage('INFO',Origin=origin,Msg='End iterative call: nIter=%s'%(nIter))

        
    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def flagData(self, vAnt=None, vIF=None, vPOL=None, UserFlag=[], createResultPlots=None, FlagRule=[]):

        """
        flagData: flagging by RMS (pre/post baseline, difference from running mean)
        Optional flagg parameters can be given as a list.
        It works for a given IF and Polarization.

        parameter: vIF, vPOL
        optional parameter: createResultPlots=True/False/None(default)
                            UserFlag=[] list of optional flag parameters

        the method utilizes Process7

        """
        origin = 'flagData()'

        self.addparam( plotflag=createResultPlots )

        (vAnt,vIF,vPOL,antIdx,polIdx) = self.__handleIndex(vAnt,vIF,vPOL)
        
        # Process7: flagging by RMS (pre/post baseline, difference from running mean)

        # 2009/10/19 for TP data
        if self.isTP:
            self.LogMessage('INFO', Origin=origin, Msg='flagData was skipped because it is a TP data')
            return

        # 2009/10/29 Predefined window
        if len(self.SpectrumWindow) != 0 and self.iteration > 1: return

        self.LogMessage('INFO', Origin=origin, Msg='Flagging')

        # set up something here...
        HtmlDir = self.HtmlDir[vAnt]
        LogDir = self.LogDir[vAnt]
        initData = self.initData[vAnt]
        lastData = self.lastData[vAnt]
        #rowsSel = self.rowsidSel[antIdx][polIdx]
        rowsSel = self.rowsSel[antIdx][polIdx]
        TimeGap = self.TimeGap[antIdx][polIdx]
        TimeTable = self.TimeTable[antIdx][polIdx]
        productTable = self.productName[vAnt].getProductName(ant=self.antName[vAnt])
        
        if createResultPlots == True or (createResultPlots == None and self.BPlotFlag != False): FigFileDir = HtmlDir+'/'+BF_STAT_PDIR+'/'
        else: FigFileDir = False

        if len(FlagRule) == 0:
            if len(self.FlagRuleDictionary) == 0:
                self.setFlagRules()
        else:
            for item in self.showFlagRules():
                self.FlagRuleDictionary[item]['isActive'] = False
                for Flag in FlagRule:
                    if Flag.upper() in item.upper():
                        self.FlagRuleDictionary[item]['isActive'] = True

##         BF.Process7(self.initData, self.lastData, self.DataTable, self.Abcissa[vIF], self.rowsSel, self.NCHAN, self.TimeGap, self.TimeTable, Iteration=self.iteration, edge=self.edge, UserFlag=UserFlag, FlagRule=self.FlagRuleDictionary, ShowPlot=self.TPlotFlag, FigFileDir=FigFileDir, FigFileRoot='Stat_%s_%s_%s' % (vIF, vPOL,self.iteration), LogLevel=self.LogLevel, LogFile=BFlog)
        self.engine.Process7(initData, lastData, vIF, rowsSel, vAnt, self.NCHAN, TimeGap, TimeTable, Iteration=self.iteration, edge=self.edge, UserFlag=UserFlag, FlagRule=self.FlagRuleDictionary, ShowPlot=self.TPlotFlag, FigFileDir=FigFileDir, FigFileRoot=('Stat_%s_%s_%s' % (vIF, vPOL,self.iteration)).replace(' ',''), LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_STAT_LOG,rawFileIdx=self.rawFileIdx[vAnt],productTable=productTable)
        SDH.HtmlFitStatistics(HtmlDir)
        #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )
        #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )

        # add History
        #self.processedStages.append((self.vIF, self.vPOL, 'flagData', self.iteration))
        self.processedStages.append((vIF, vPOL, 'flagData', self.iteration))
        # Save Meta Data
        self.saveMetaData(lastData+'.meta')
        

    #@dec_stages_interface_logfile
    #@dec_stages_interface_logging
    def regridDataRunThrough(self, SWconfig=None, gridsize='automatic'):
        """
        """
        origin = 'regridDataRunThrough'

        # 2011/11/15 TN
        # Skip if TP data here instead of each iterative call
        # Also skip if predefined window is set and second iteration or later 
        if self.isTP and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='regridData was skipped because it is TP data and iteration > 1')
            return
        if len(self.SpectrumWindow) != 0 and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='regridData was skipped because spectrum window is set and iteration > 1')
            return

        (vAnt,vIF,vPOLs) = self.__configureListFromSWConfig(SWconfig)

        # iterative call of regridData
        # 2011/11/12 TN
        # process same antenna data at once
##         nIter = len(vAnt)
##         self.LogMessage('INFO',Origin=origin,Msg='Start iterative call: nIter=%s'%(nIter))
##         for i in xrange(nIter):
##             nPol = len(vPOLs[i])
##             self.LogMessage('INFO',Origin=origin,Msg='nPol=%s'%(nPol))
##             for j in xrange(nPol):
##                 self.regridData( vAnt[i], vIF[i], vPOLs[i][j] )
##         self.LogMessage('INFO',Origin=origin,Msg='End iterative call: nIter=%s'%(nIter))
        # group by antenna name
        antList = {}
        ifList = {}
        polList = {}
        for i in xrange(len(vAnt)):
            antName = self.antName[vAnt[i]]
            pols = []
            if antList.has_key(antName):
                antList[antName].append( vAnt[i] )
                ifList[antName].append( vIF[i] )
                polList[antName].append( vPOLs[i] )
            else:
                antList[antName] = [vAnt[i]]
                ifList[antName] = [vIF[i]]
                polList[antName] = [vPOLs[i]]
        self.LogMessage('INFO',Origin=origin,Msg='antList=%s'%(antList))
        # iterative call
        nIter = len(antList)
        self.LogMessage('INFO',Origin=origin,Msg='Start iterative call: nIter=%s'%(nIter))
        for key in antList.keys():
            # assume that IFNO and POLNO are the same for all datasets
            # (in some cases, it may not be correct...)
            nPol = len(polList[key][0])
            self.LogMessage('INFO',Origin=origin,Msg='nPol=%s'%(nPol))
            for j in xrange(nPol):
                pols = []
                for k in xrange(len(antList[key])):
                    pols.append( polList[key][k][j] )
                self.regridData( antList[key], ifList[key], pols, gridsize )
        self.LogMessage('INFO',Origin=origin,Msg='End iterative call: nIter=%s'%(nIter))


    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def regridData(self, vAnt=None, vIF=None, vPOL=None, gridsize='automatic'):

        """
        regridData: combinea spectra for new grid
        It works for a given IF and Polarization.

        parameter: vIF, vPOL
        the method utilizes Process8
        """
        origin = 'regridData()'

        # vAnt, vIF, vPOL are now given as list
        antIdxList = [-1 for i in xrange(len(vAnt))]
        polIdxList = [-1 for i in xrange(len(vAnt))]
        for i in xrange(len(vAnt)):
            (vAnt[i],vIF[i],vPOL[i],antIdxList[i],polIdxList[i]) = self.__handleIndex(vAnt[i],vIF[i],vPOL[i])
##         (vAnt,vIF,vPOL,antIdx,polIdx) = self.__handleIndex(vAnt,vIF,vPOL)
       
        if self.isTP and self.iteration != 1:
            self.LogMessage('INFO', Origin=origin, Msg='regridData was skipped for TP data with iteration of %s' % self.iteration)
            return

        # 2009/10/29 Predefined window
        if len(self.SpectrumWindow) != 0 and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='regridData was skipped because spectrum window is set and the iteration is %s' % self.iteration)
            return

        # set appropriate beam size
        if gridsize == 'automatic':
            self.__setRadiusFromBeamSize( vAnt[0], vIF[0] )
        else:
            self.setBeamRadius( gridsize / 7200.0 )

        # set up something here...
        # 2011/11/12 TN temporary...
##         LogDir = self.LogDir[vAnt]
##         outFile = self.outFile[vAnt]
##         lastData = self.lastData[vAnt]
##         if self.continuumOutput == True: contData = self.contData[vAnt]
##         else: contData = None
##         rowsSel = self.rowsidSel[antIdx][polIdx]
        LogDir = self.LogDir[vAnt[0]]
        outFile = self.outFile[vAnt[0]]
        lastData = [self.lastData[i] for i in vAnt]
        if self.continuumOutput == True: contData = [self.contData[i] for i in vAnt]
        else: contData = None
        rowsSel = []
        for i in xrange(len(vAnt)):
            rowsSel.append( self.rowsidSel[antIdxList[i]][polIdxList[i]] )
        self.LogMessage('INFO',Origin=origin,Msg='DataIn=%s'%(lastData))
        self.LogMessage('DEBUG',Origin=origin,Msg='rowsSel=%s'%(rowsSel))

        # GroupForGrid: re-gridding
        # 2008/09/24 extend the radius for the convolution to 2*hwhm
##         self.GridTable = BF.GroupForGrid(self.DataTable, self.rowsSel, vIF, vPOL, self.radius*2.0, self.radius/10.0, self.spacing, self.Pattern, LogLevel=self.LogLevel, LogFile=BFlog)
##         self.GridTable = self.engine.GroupForGrid(rowsSel, vIF, vPOL, self.radius*2.0, self.radius/10.0, self.spacing, self.Pattern, LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_GRID_LOG)
        self.GridTable = self.engine.GroupForGrid(rowsSel, vIF[0], vPOL[0], self.radius*2.0, self.radius/10.0, self.spacing, self.Pattern, LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_GRID_LOG)
        #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )
        #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )
        
        #ProcEndTime = time.time()
                        
        # Process8: Combine spectra for new grid
        NewTable = self.outDir+outFile+'.new.tbl'
        if self.Pattern.upper() == 'SINGLE-POINT' or \
               self.Pattern.upper() == 'MULTI-POINT':
            weight = 'CONST'
        else:
            weight = 'GAUSS'
        #ProcStartTime = time.time()
        # 2011/03/14 TN
        # Data product naming convention
        #self.dtctData = self.outDir+self.outFile+'_%d_%d.grid' % (vIF, vPOL)
        #self.dtctData = self.productName.getDataName( self.iterAnt, baseline=False, spwid=vIF, poltype=self.poltype, polid=vPOL )
        #self.dtctData = self.productName[vAnt].getDataName( self.antName[vAnt], baseline=False, spwid=vIF, poltype=self.poltype, polid=vPOL )
##         self.gridData[vAnt][vPOL] = self.productName[vAnt].getDataName( self.antName[vAnt], baseline=False, spwid=vIF, poltype=self.poltype, polid=vPOL )
        self.gridData[vAnt[0]][vPOL[0]] = self.productName[vAnt[0]].getDataName( self.antName[vAnt[0]], baseline=False, spwid=vIF[0], poltype=self.poltype, polid=vPOL[0] )
        self.LogMessage('INFO',Origin=origin,Msg='self.gridData.keys()=%s'%(self.gridData.keys()))
##         for i in xrange(1,len(vAnt)):
##             self.gridData[vAnt[i]][vPOL[i]] = self.gridData[vAnt[0]][vPOL[0]]
        if self.continuumOutput == True:
            # 2011/03/14 TN
            # Data product naming convention
            self.gridCont[vAnt[0]][vPOL[0]] = self.productName[vAnt[0]].getDataName( self.antName[vAnt[0]], baseline=False, spwid=vIF[0], poltype=self.poltype, polid=vPOL[0], cont=True )


##         (NewSP, self.ResultTable) = BF.Process8(self.lastData, self.dtctData, self.DataTable, self.rowsSel, self.NCHAN, self.GridTable, self.radius, self.GridRuleDictionary, LogLevel=self.LogLevel, LogFile=BFlog)
        #(NewSP, self.ResultTable) = self.engine.Process8(lastData, self.dtctData, rowsSel, self.NCHAN, self.GridTable, self.radius, self.GridRuleDictionary, LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_GRID_LOG)
##         (NewSP, self.GridResult[vAnt][vPOL]) = self.engine.Process8(lastData, self.dtctData, rowsSel, self.NCHAN, self.GridTable, self.radius, self.GridRuleDictionary, LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_GRID_LOG)
        (NewSP, self.GridResult[vAnt[0]][vPOL[0]]) = self.engine.Process8(lastData, self.dtctData, rowsSel, self.NCHAN, self.GridTable, self.radius, self.GridRuleDictionary, LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_GRID_LOG)
##         for i in xrange(1,len(vAnt)):
##             self.GridResult[vAnt[i]][vPOL[i]] = self.GridResult[vAnt[0]][vPOL[0]]
        #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )
        #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )

        #self.LogMessage('INFO', Origin=origin, Msg='create ASAP file: %s ' % self.dtctData)
        self.LogMessage('INFO', Origin=origin, Msg='create ASAP file: %s ' % self.gridData[vAnt[0]][vPOL[0]])
##         DP.WriteNewMSData(self.lastData, self.dtctData, NewSP, self.ResultTable, 'ASAP', LogLevel=self.LogLevel, LogFile=BFlog)
        #self.engine.WriteNewMSData(lastData, self.dtctData, NewSP, self.ResultTable, 'ASAP', LogLevel=self.LogLevel, LogFile=LogDir+'/' + self.BF_GRID_LOG )
        #self.engine.WriteNewMSData(lastData, self.gridData[vAnt], NewSP, self.ResultTable, 'ASAP', LogLevel=self.LogLevel, LogFile=LogDir+'/' + self.BF_GRID_LOG )
##         self.engine.WriteNewMSData(lastData, self.gridData[vAnt][vPOL], NewSP, self.GridResult[vAnt][vPOL], 'ASAP', LogLevel=self.LogLevel, LogFile=LogDir+'/' + self.BF_GRID_LOG )
        self.engine.WriteNewMSData(lastData[0], self.gridData[vAnt[0]][vPOL[0]], NewSP, self.GridResult[vAnt[0]][vPOL[0]], 'ASAP', LogLevel=self.LogLevel, LogFile=LogDir+'/' + self.BF_GRID_LOG )
        #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )
        #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )
        del NewSP

        # for ia.fromarray imaging 2010/11/3 TN
        if not (vAnt[0] in self.gridInfo.keys()):
            self.gridInfo[vAnt[0]]={}
        if not (vIF[0] in self.gridInfo[vAnt[0]].keys()):
            self.gridInfo[vAnt[0]][vIF[0]]={}
        gridRA=[]
        gridDEC=[]
        for i in xrange(len(self.GridTable)):
            gridRA.append(self.GridTable[i][4])
            gridDEC.append(self.GridTable[i][5])
        centerRA=0.5*(NP.max(gridRA)+NP.min(gridRA))
        centerDEC=0.5*(NP.max(gridDEC)+NP.min(gridDEC))
        tablelen = len(self.GridTable)
        #self.gridInfo[self.iterAnt][vIF][vPOL]=self.dtctData
        self.gridInfo[vAnt[0]][vIF[0]][vPOL[0]]=self.gridData[vAnt[0]][vPOL[0]]
        self.gridInfo[vAnt[0]][vIF[0]]['grid']=self.GridTable[tablelen-1][2:4]
        self.gridInfo[vAnt[0]][vIF[0]]['grid'][0]+=1
        self.gridInfo[vAnt[0]][vIF[0]]['grid'][1]+=1
        ngridx=self.gridInfo[vAnt[0]][vIF[0]]['grid'][0]
        decCorr=1.0 / NP.cos(centerDEC*NP.pi/180.0)
        self.gridInfo[vAnt[0]][vIF[0]]['cell']=[(self.GridTable[tablelen-1][4]-self.GridTable[tablelen-2][4])/decCorr,self.GridTable[tablelen-1][5]-self.GridTable[tablelen-ngridx-1][5]]
        #self.gridInfo[self.iterAnt][vIF]['cell']=[self.GridTable[tablelen-1][5]-self.GridTable[tablelen-ngridx-1][5],self.GridTable[tablelen-1][5]-self.GridTable[tablelen-ngridx-1][5]]
        self.gridInfo[vAnt[0]][vIF[0]]['center']=[centerRA,centerDEC]

        # Continuum Output 2010/10/25 GK
        if self.continuumOutput == True:
            (ContSP, ContResultTable) = self.engine.Process8(contData, self.dtctData, rowsSel, self.NCHAN, self.GridTable, self.radius, self.GridRuleDictionary, LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_GRID_LOG)
            #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )
            #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )
            self.LogMessage('INFO', Origin=origin, Msg='create ASAP file: %s' % self.gridCont[vAnt[0]][vPOL[0]])
            self.engine.WriteNewMSData(contData[0], self.gridCont[vAnt[0]][vPOL[0]], ContSP, ContResultTable, 'ASAP', LogLevel=self.LogLevel, LogFile=LogDir+'/' + self.BF_GRID_LOG )
            #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )
            #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )
            del ContSP, ContResultTable

            key='cont%s'%vPOL[0]
            self.gridInfo[vAnt[0]][vIF[0]][key]=self.gridCont[vAnt[0]][vPOL[0]]

        # 2009/11/4 make image here (moved from writeMSDataCube)
        #if not self.isTP and self.imageCube and self.Pattern.upper() == 'RASTER':
        #    FileReGrid = self.outDir+self.outFile+'_%d_%d.grid.MS' % (vIF, vPOL)
        #    s = scantable(self.dtctData, average=False)
        #    s.save(FileReGrid, format='MS2', overwrite=True)
        #    del s
        #    #if self.imageCube and self.Pattern.upper() == 'RASTER':
        #    # CASA Imager output
        #    #radius; OK
        #    #iteration; OK
        #    #spacing; OK
        #    #Lines; OK
        #    #print 'Lines=', Lines
        #    # singleimage =True to make a single data cube for all the lines
        #    # willl use max channel width and ignores NChannelMap
        #    singleimage=False
        #    #singleimage=True
        #    (mapCenter, ngridx, ngridy, cellsize, startchans, nchans, chansteps)=SDI.getImParams(FileReGrid, self.Lines, self.radius, self.spacing, singleimage)
        #    cellx=cellsize
        #    celly=cellsize
        #    outImagename=self.outDir+self.outFile+'_%d_%d.gd' % (vIF, vPOL)
        #    field=0
        #    convsupport=-1
        #    spw=0
        #    moments=[0]
        #    SDI.MakeImages(FileReGrid, outImagename, ngridx, ngridy, cellx, celly, mapCenter, nchans, startchans, chansteps,field, convsupport, spw, moments, showImage=False)
        #    SDI.MakeImage(FileReGrid, outImagename+'.image', ngridx, ngridy, cellx, celly, mapCenter, (self.NCHAN-self.edge[0]-self.edge[1]), self.edge[0], 1, field, convsupport, spw, moments, showImage=False)

        #self.lastData[vAnt] = self.dtctData

        # add History
        self.processedStages.append((self.vIF[0], self.vPOL[0], 'regridData', self.iteration))
        # Save Meta Data
        #self.saveMetaData(self.dtctData+'.meta')
        self.saveMetaData(self.gridData[vAnt[0]][vPOL[0]]+'.meta')

    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def combineGriddedData(self, SWconfig=None):
        """
        Combine gridded data per antenna.
        
        Combination will be done based on data classification that
        must be done prior to this method call.
        """

        origin = 'combineGriddedData'

        # skip if only one antenna
        if self.CombineDir is None:
            self.LogMessage('INFO',Origin=origin,Msg='Skip %s since only one antenna'%(origin))
            return

        # Skip if TP data here instead of each iterative call
        # Also skip if predefined window is set and second iteration or later 
        if len(self.SpectrumWindow) != 0 and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='%s was skipped because spectrum window is set'%(origin))
            return

        (vAnt,vIF,vPOLs) = self.__configureListFromSWConfig(SWconfig)

        # at the moment, all the data are combined
## ##         baselinedFiles = []
## ##         for v in self.outFile.values():
## ##             baselinedFiles.append(self.outDir+'/'+v)
##         def applyRowFlag( files, DataTable ):
##             for (key,row) in DataTable.items():
##                 if type(key) == str:
##                     continue
##                 self._tb.open(files[row[-1]],nomodify=False)
##                 irow = int(row[1])
##                 rflag = 0 if row[19] == 1 else 1
##                 self._tb.putcell('FLAGROW', irow, rflag)
##                 self._tb.close()
##         baselinedFiles = []
##         for ant in vAnt:
##             baselinedFiles.append(self.outDir+'/'+self.outFile[ant])
##         continuumFiles = []
##         if self.continuumOutput == True:
##             for ant in vAnt:
##                 continuumFiles.append(self.contData[ant])
##         applyRowFlag( baselinedFiles, self.engine.DataTable )
##         self.LogMessage('DEBUG',Origin=origin,Msg='baselinedFiles=%s'%(baselinedFiles))

##         # DEC correction
##         self.LogMessage('INFO',Origin=origin,Msg='Opening %s...'%(baselinedFiles[0]))
##         self._tb.open(baselinedFiles[0])
##         d=self._tb.getcol('DIRECTION')
##         self._tb.close()
##         centerDEC = 0.5*(d[1].max()+d[1].min())
##         extentRA = d[0].max() - d[0].min()
##         extentDEC = d[1].max() - d[1].min()
##         decCorr=1.0 / NP.cos(centerDEC)
##         self.LogMessage('INFO',Origin=origin,Msg='DEC correction factor: %s'%(decCorr))

##         # gridding
##         from sdgrid_cli import sdgrid_cli as sdgrid
##         import math
##         #cell = ['%sarcsec'%(0.5*self.beamsize[0][vIF[0]]*decCorr),
##         #        '%sarcsec'%(0.5*self.beamsize[0][vIF[0]])]
##         #cell = ['%sarcsec'%(self.beamsize[0][vIF[0]]*decCorr),
##         #        '%sarcsec'%(self.beamsize[0][vIF[0]])]
##         #npix = [int(math.ceil(extentRA/(self.beamsize[0][vIF[0]]*decCorr/7200.0*NP.pi/180.0))+3),
##         #        int(math.ceil(extentDEC/(self.beamsize[0][vIF[0]]/7200.0*NP.pi/180.0))+3)]
##         nline = 0
##         self.LogMessage('INFO',Origin=origin,Msg='len(self.GridTable) = %s'%(len(self.GridTable)))
##         while (self.GridTable[nline][2] == 0):
##             nline += 1
##         npix = [self.GridTable[-1][2]+1,self.GridTable[-1][3]+1]
##         # in the future casapy release, dec correction will be
##         # considered in the task
##         #cell = ['%sarcsec'%((self.GridTable[nline][4]-self.GridTable[0][4])*3600.0/decCorr),
##         cell = ['%sarcsec'%((self.GridTable[nline][4]-self.GridTable[0][4])*3600.0),
##                 '%sarcsec'%((self.GridTable[nline*npix[0]][5]-self.GridTable[0][5])*3600.0)]
##         self.LogMessage('INFO',Origin=origin,Msg='cell = %s'%(cell))
##         self.LogMessage('INFO',Origin=origin,Msg='npix = %s'%(npix))
##         outfile = []
##         for pol in vPOLs[0]:
##             outfile.append('%s/test.spw%s.pol%s.asap'%(self.outDir,vIF[0],pol))
##             sdgrid(infiles=baselinedFiles,gridfunction='SF',weight='TSYS',ifno=vIF[0],pollist=pol,cell=cell,clipminmax=True,npix=npix,outfile=outfile[pol],overwrite=True)
##         outcont = []
##         if self.continuumOutput == True:
##             for pol in vPOLs[0]:
##                 outcont.append('%s/test.spw%s.pol%s.cont.asap'%(self.outDir,vIF[0],pol))
##                 sdgrid(infiles=continuumFiles,gridfunction='SF',weight='TSYS',ifno=vIF[0],pollist=pol,cell=cell,clipminmax=True,npix=npix,outfile=outcont[pol],overwrite=True)

        # using engine.combineGriddedData
        infiles = [[] for pol in vPOLs[0]]
        self.outGridComb[vIF[0]] = {}
        for pol in vPOLs[0]:
            self.outGridComb[vIF[0]][pol] = self.productName[vAnt[0]].getDataName( 'COMBINE', baseline=False, spwid=vIF[0], poltype=self.poltype, polid=pol )
        self.GridTableComb = [[] for pol in vPOLs[0]]
        RefTable = self.GridResult[0][0]
        for ant in vAnt:
            for pol in vPOLs[ant]:
                if self.gridData[ant].has_key(pol):
                    infiles[pol].append(self.gridData[ant][pol])
        self.LogMessage('DEBUG',Origin=origin,Msg='infiles=%s'%(infiles))
        for pol in vPOLs[0]:
            #(StorageOut,self.GridTableComb[pol]) = self.engine.combineGriddedData(infiles[pol])
            ret = self.engine.combineGriddedData(infiles[pol])
            if ret is False:
                self.GridTableComb = None
                StorageOut = None
                self.LogMessage('WARN',Origin=origin,Msg='Combining was not done')
                break
            else:
                (StorageOut,self.GridTableComb[pol]) = ret 
            for irow in xrange(len(self.GridTableComb[pol])):
                row = self.GridTableComb[pol][irow]
                ref = RefTable[irow]
                for idx in xrange(2,7):
                    row[idx] = ref[idx]
                row.append(ref[8])
            #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )
            self.LogMessage('INFO', Origin=origin, Msg='create ASAP file: %s' % (self.outGridComb[vIF[0]][pol]))
            self.engine.WriteNewMSData(self.workDir + self.workFile[self.antKeys[0]], self.outGridComb[vIF[0]][pol], StorageOut, self.GridTableComb[pol], 'ASAP', LogLevel=self.LogLevel, LogFile=self.CombineDir+'/' + self.BF_GRID_LOG )
            #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )
        self.LogMessage('DEBUG',Origin=origin,Msg='Table = %s'%(self.GridTableComb))
        outcont = []
        contfiles = [[] for pol in vPOLs[0]]
        self.LogMessage('INFO',Origin=origin,Msg='self.gridCont=%s'%(self.gridCont))
        if self.continuumOutput == True:
            for ant in vAnt:
                for pol in vPOLs[0]:
                    if self.gridCont[ant].has_key(pol):
                        contfiles[pol].append(self.gridCont[ant][pol])
            self.contGridComb[vIF[0]] = {}
            for pol in vPOLs[0]:
                self.contGridComb[vIF[0]][pol] = self.productName[vAnt[0]].getDataName( 'COMBINE', baseline=False, spwid=vIF[0], poltype=self.poltype, polid=pol, cont=True )
            for pol in vPOLs[0]:
                #(StorageOut,Table) = self.engine.combineGriddedData(contfiles[pol])
                ret = self.engine.combineGriddedData(contfiles[pol])
                if ret is False:
                    StorageOut = None
                    Table = None
                    self.LogMessage('WARN',Origin=origin,Msg='Combining continuum data was not done')
                    break
                else:
                    (StorageOut,Table) = ret
                #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )
                self.LogMessage('INFO', Origin=origin, Msg='create ASAP file: %s' % (self.contGridComb[vIF[0]][pol]))
                self.engine.WriteNewMSData(self.workDir+self.workFile[self.antKeys[0]], self.contGridComb[vIF[0]][pol], StorageOut, self.GridTableComb[pol], 'ASAP', LogLevel=self.LogLevel, LogFile=self.CombineDir+'/'+self.BF_GRID_LOG )
                #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )

    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def plotResultSpectraForCombine(self, SWconfig=None):
        """
        Plot ResultSpectra for combined data.
        """

        origin = 'plotResultSpectraForCombine'
                
        # skip if only one antenna
        if self.CombineDir is None:
            self.LogMessage('INFO',Origin=origin,Msg='Skip %s since only one antenna'%(origin))
            return

        # skip if combineGriddedData is not called
        if self.GridTableComb is None:
            self.LogMessage('INFO',Origin=origin,Msg='Skip %s since no combined data available'%(origin))
            return

        # Skip if TP data here instead of each iterative call
        # Also skip if predefined window is set and second iteration or later 
        if self.isTP:
            self.LogMessage('INFO', Origin=origin, Msg='%s was skipped because it is a TP data'%(origin))
            return
        if len(self.SpectrumWindow) != 0 and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='%s was skipped because spectrum window is set'%(origin))
            return

        (vAnt,vIF,vPOLs) = self.__configureListFromSWConfig(SWconfig)

        # plot result
        rowsSelForNavi = [[] for pol in vPOLs[0]]
        for ant in vAnt:
            for pol in vPOLs[ant]:
                (atmp,itmp,ptmp,aidx,pidx) = self.__handleIndex(ant,vIF[0],pol)
                rowsSelForNavi[pol] += self.rowsSel[aidx][pidx]

        for pol in vPOLs[0]:
            lastData = self.outGridComb[vIF[0]][pol]
##             ResultTable = []
##             self._tb.open(lastData)
##             ifnos = self._tb.getcol('IFNO')
##             polnos = self._tb.getcol('POLNO')
##             dir = self._tb.getcol('DIRECTION')*180.0/NP.pi
##             cen = [0.5*(dir[0].max()+dir[0].min()),
##                    0.5*(dir[1].max()+dir[1].min())]
##             for i in xrange(self._tb.nrows()):
##                 ResultTable.append( [ifnos[i],polnos[i],i%npix[0],i/npix[0],dir[0][i],dir[1][i],1,0,0.0] )
##             self._tb.close()
            ResultTable = self.GridTableComb[pol]
            rowsSel = range(len(ResultTable))
            FigFileDir = self.CombineDir

            self.engine.DrawMultiSpectraNavigator(vIF[0], pol, self.iteration, self.GridResult[vAnt[0]][pol], rowsSelForNavi[pol], self.spacing, self.Pattern, FigFileDir=FigFileDir+'/'+BF_GRID_PDIR+'/', FigFileRoot='Result_%s_%s_%s'%(vIF[0],pol,self.iteration))

            SDP.DrawMultiSpectra(lastData, ResultTable, self.Abcissa[vIF[0]], rowsSel, self.Pattern, chan0=self.edge[0], chan1=(self.NCHAN - self.edge[1] - 1), ShowPlot=self.TPlotMultiSP, FigFileDir=FigFileDir+'/'+BF_GRID_PDIR+'/', FigFileRoot='Result_%s_%s_%s'%(vIF[0],pol,self.iteration))
            SDH.HtmlMultiSpectra(self.CombineDir)

    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def plotSparseSpectraMapForCombine(self, SWconfig=None):
        """
        Plot SparseSpectraMap for combined data.
        """

        origin = 'plotSparseSpectraMapForCombine'
                
        # skip if only one antenna
        if self.CombineDir is None:
            self.LogMessage('INFO',Origin=origin,Msg='Skip %s since only one antenna'%(origin))
            return

        # skip if combineGriddedData is not called
        self.LogMessage('INFO',Origin=origin,Msg='self.GridTableComb=%s'%(self.GridTableComb))
        if self.GridTableComb is None:
            self.LogMessage('INFO',Origin=origin,Msg='Skip %s since no combined data available'%(origin))
            return

        # Skip if TP data here instead of each iterative call
        # Also skip if predefined window is set and second iteration or later 
        if self.isTP:
            self.LogMessage('INFO', Origin=origin, Msg='%s was skipped because it is a TP data'%(origin))
            return
        if len(self.SpectrumWindow) != 0 and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='%s was skipped because spectrum window is set'%(origin))
            return

        (vAnt,vIF,vPOLs) = self.__configureListFromSWConfig(SWconfig)

        # plot result
        for pol in vPOLs[0]:
            lastData = self.outGridComb[vIF[0]][pol]
##             ResultTable = []
##             self._tb.open(lastData)
##             ifnos = self._tb.getcol('IFNO')
##             polnos = self._tb.getcol('POLNO')
##             dir = self._tb.getcol('DIRECTION')*180.0/NP.pi
##             cen = [0.5*(dir[0].max()+dir[0].min()),
##                    0.5*(dir[1].max()+dir[1].min())]
##             for i in xrange(self._tb.nrows()):
##                 ResultTable.append( [ifnos[i],polnos[i],i%npix[0],i/npix[0],dir[0][i],dir[1][i],1,0,0.0] )
##             self._tb.close()
            ResultTable = self.GridTableComb[pol]
            rowsSel = range(len(ResultTable))
            FigFileDir = self.CombineDir
            SDP.DrawSparseSpectra(lastData, ResultTable, self.Abcissa[vIF[0]], MaxPanels4SparseSP, chan0=self.edge[0], chan1=(self.NCHAN - self.edge[1] - 1), ShowPlot=self.TPlotSparseSP, FigFileDir=FigFileDir+'/'+BF_SPARSESP_PDIR+'/', FigFileRoot='SparseSpMap_%s_%s_%s' % (vIF[0], pol,self.iteration))
            SDH.HtmlSparseSpectraMap(self.CombineDir)

    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def plotChannelMapForCombine(self, SWconfig=None):
        """
        Plot ChannelMap for combined data.
        """

        origin = 'plotChannelMapForCombine'
                
        # skip if only one antenna
        if self.CombineDir is None:
            self.LogMessage('INFO',Origin=origin,Msg='Skip %s since only one antenna'%(origin))
            return

        # skip if combineGriddedData is not called
        if self.GridTableComb is None:
            self.LogMessage('INFO',Origin=origin,Msg='Skip %s since no combined data available'%(origin))
            return

        # Skip if TP data here instead of each iterative call
        # Also skip if predefined window is set and second iteration or later 
        if len(self.SpectrumWindow) != 0 and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='%s was skipped because spectrum window is set'%(origin))
            return

        (vAnt,vIF,vPOLs) = self.__configureListFromSWConfig(SWconfig)

        # plot result
        for pol in vPOLs[0]:
            lastData = self.outGridComb[vIF[0]][pol]
##             ResultTable = []
##             self._tb.open(lastData)
##             ifnos = self._tb.getcol('IFNO')
##             polnos = self._tb.getcol('POLNO')
##             dir = self._tb.getcol('DIRECTION')*180.0/NP.pi
##             cen = [0.5*(dir[0].max()+dir[0].min()),
##                    0.5*(dir[1].max()+dir[1].min())]
##             for i in xrange(self._tb.nrows()):
##                 ResultTable.append( [ifnos[i],polnos[i],i%npix[0],i/npix[0],dir[0][i],dir[1][i],1,0,0.0] )
##             self._tb.close()
            ResultTable = self.GridTableComb[pol]
            rowsSel = range(len(ResultTable))
            FigFileDir = self.CombineDir
            if self.isTP:
                SDP.DrawTPImage(lastData, ResultTable, self.radius, self.spacing, scale_max=False, scale_min=False, colormap=self.colormap, ShowPlot=self.TPlotChannelMap, FigFileDir=FigFileDir+'/'+BF_CHANNELMAP_PDIR+'/', FigFileRoot='ChannelMap_%s_%s_%s' % (vIF[0], pol,self.iteration))
            else:
                SDP.DrawImage(lastData, ResultTable, self.Abcissa[vIF[0]], self.Lines, self.radius, self.spacing, scale_max=False, scale_min=False, colormap=self.colormap, ShowPlot=self.TPlotChannelMap, FigFileDir=FigFileDir+'/'+BF_CHANNELMAP_PDIR+'/', FigFileRoot='ChannelMap_%s_%s_%s' % (vIF[0], pol,self.iteration))
                
                if self.continuumOutput == True:
                    SDP.DrawContImage(self.contGridComb[vIF[0]][pol], ResultTable, self.Abcissa[vIF[0]], self.radius, self.spacing, scale_max=False, scale_min=False, colormap=self.colormap, ShowPlot=self.TPlotChannelMap, FigFileDir=FigFileDir+'/'+BF_CHANNELMAP_PDIR+'/', FigFileRoot='ChannelMap_%s_%s_%s' % (vIF[0], pol,self.iteration))
            SDH.HtmlGridding(self.CombineDir)
 

    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def simpleGridding(self):
        """
        Simple gridding for line detection and clustering analysis.
        """

        origin = 'simpleGridding()'

        # serialize self.rowsidSel
        idxList = []
        #print 'self.rowsidSel=%s'%(self.rowsidSel)
        for i in xrange(len(self.rowsidSel)):
            for j in xrange(len(self.rowsidSel[i])):
                idxList += self.rowsidSel[i][j]

        LogDir = self.LogDir[self.LogDir.keys()[0]]

        # antenna list is self.AntIndex dictionary
        # corresponding IF and POL is self.vIF and self.vPOL list
        antID = self.AntIndex.keys()[0]
        ifno = self.vIF[self.AntIndex[antID]]

        # 2012/03/01 TN
        # set appropriate beam radius
        self.__setRadiusFromBeamSize( antID, ifno )
            
        # initialization for SimpleGridding
        # grid spacing is beam diameter
        gridRA = self.radius * 2.0
        gridDec = self.radius * 2.0
        gridTable = self.engine.makeSimpleGridTable( idxList, gridRA, gridDec, LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_GRID_LOG )
        #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )

        # 2011/10/26 TN
        # moved from spectralLineDetect()
        self.iteration += 1

        # DataIn: {AntID: inputFile} dictionary
        DataIn = {}
        if self.iteration > 1:
            # use baselined data when second or later itelation
            for key in self.antKeys:
                #DataIn[key] = self.lastData[key]
                DataIn[key] = self.outDir+self.outFile[key]
        else:
            # first iteration
            for key in self.antKeys:
                DataIn[key] = self.workDir + self.workFile[key]
        self.LogMessage('INFO',Origin=origin,Msg='DataIn=%s'%(DataIn))
        # DataOut: not used 
        DataOut = {}
        # nchan: number of channel (must be same over idxList)
        summaryID = self.rawFileIdx[antID]
        nchan = self.dataSummary[summaryID]['spw'][ifno]['nchan']
        (SpStorage,self.ResultTable) = self.engine.SimpleGridding( DataIn, DataOut, nchan, gridTable, LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_GRID_LOG )
        #print '\nlen(self.ResultTable), self.ResultTable', len(self.ResultTable), '\n', self.ResultTable, '\n'
        #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )

        # write out gridded data
        #print '\nself.dtctData', self.dtctData, '\n'
        #self.engine.WriteNewMSData(self.workFile[self.antKeys[0]], self.dtctData, SpStorage, self.ResultTable, 'ASAP', LogLevel=self.LogLevel, LogFile=LogDir+'/' + self.BF_GRID_LOG )
        self.engine.WriteNewMSData(self.workDir + self.workFile[self.antKeys[0]], self.dtctData, SpStorage, self.ResultTable, 'ASAP', LogLevel=self.LogLevel, LogFile=LogDir+'/' + self.BF_GRID_LOG )
        #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )
        #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )

        
    #@dec_stages_interface_logfile
    #@dec_stages_interface_logging
    def plotFitSpectraWithFlagResultRunThrough(self, SWconfig=None):
        """
        """
        origin = 'plotFitSpectraWithFlagResultRunThrough'

        # 2011/11/15 TN
        # Skip if TP data here instead of each iterative call
        # Also skip if predefined window is set and second iteration or later 
        if self.isTP:
            self.LogMessage('INFO', Origin=origin, Msg='plotFitSpectraWithFlagResult was skipped because it is a TP data')
            return
        if len(self.SpectrumWindow) != 0 and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='plotFitSpectraWithFlagResult was skipped because spectrum window is set')
            return

        (vAnt,vIF,vPOLs) = self.__configureListFromSWConfig(SWconfig)

        # iterative call of plotSpectraWithFlagResult 
        nIter = len(vAnt)
        self.LogMessage('INFO',Origin=origin,Msg='Start iterative call: nIter=%s'%(nIter))
        for i in xrange(nIter):
            nPol = len(vPOLs[i])
            self.LogMessage('INFO',Origin=origin,Msg='nPol=%s'%(nPol))
            for j in xrange(nPol):
                self.plotFitSpectraWithFlagResult( vAnt[i], vIF[i], vPOLs[i][j] )
        self.LogMessage('INFO',Origin=origin,Msg='End iterative call: nIter=%s'%(nIter))


    @dec_stages_interface_logfile
    @dec_stages_interface_logging
    def plotFitSpectraWithFlagResult(self, vAnt=None, vIF=None, vPOL=None):
        """
        plotFitSpectraWithFlagResult: plot both raw spectra and reduced spectra. Background color is red if the spectrum is flagged.
        """
        origin = 'plotFitSpectraWithFlagResult()'

        (vAnt,vIF,vPOL,antIdx,polIdx) = self.__handleIndex(vAnt,vIF,vPOL)

        if self.isTP:
            self.LogMessage('INFO', Origin=origin, Msg='plotFitSpectraWithFlagResult was skipped because it is a TP data')
            return

        # set up something here...
        HtmlDir = self.HtmlDir[vAnt]
        LogDir = self.LogDir[vAnt]
        initData = self.initData[vAnt]
        outFile = self.outFile[vAnt]
        rowsSel = self.rowsidSel[antIdx][polIdx]

        if self.BPlotFit:
            FigFileDir = HtmlDir+'/'+BF_FIT_PDIR+'/'
        else:
            FigFileDir = False

        #self.engine.DrawFitSpectrumThumbnail(self.rowsSel, vIF, vPOL, self.iteration,  LogLevel=self.LogLevel, LogFile=self.LogDir+'/'+self.BF_FIT_LOG, FigFileDir=FigFileDir, FigFileRoot='Fit_%s_%s_%s' % (vIF, vPOL,self.iteration))

        self.engine.DrawFitSpectrum(initData, self.outDir+outFile, rowsSel, self.NCHAN, vIF, edge=self.edge, HandleFlag=True, LogLevel=self.LogLevel, LogFile=LogDir+'/'+self.BF_FIT_LOG, FigFileDir=FigFileDir, FigFileRoot='Fit_%s_%s_%s' % (vIF, vPOL,self.iteration))
        #SDH.HtmlBaselineFit(self.HtmlDir)
        SDH.HtmlBaselineFit2(HtmlDir)
        #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )
        #self.setFileOut( self.GlobalLogDir+'/'+self.SD_PIPELINE_LOG )


    #@dec_stages_interface_logging
    def plotResultSpectraRunThrough(self, SWconfig=None):
        """
        """
        origin = 'plotResultSpectraRunThrough'

        # 2011/11/15 TN
        # Skip if TP data here instead of each iterative call
        # Also skip if predefined window is set and second iteration or later 
        if self.isTP:
            self.LogMessage('INFO', Origin=origin, Msg='plotResultSpectra was skipped because it is a TP data')
            return
        if len(self.SpectrumWindow) != 0 and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='plotResultSpectra was skipped because spectrum window is set')
            return

        (vAnt,vIF,vPOLs) = self.__configureListFromSWConfig(SWconfig)

        # iterative call of plotResultSpectra 
        nIter = len(vAnt)
        self.LogMessage('INFO',Origin=origin,Msg='Start iterative call: nIter=%s'%(nIter))
        for i in xrange(nIter):
            nPol = len(vPOLs[i])
            self.LogMessage('INFO',Origin=origin,Msg='nPol=%s'%(nPol))
            for j in xrange(nPol):
                self.plotResultSpectra( vAnt[i], vIF[i], vPOLs[i][j] )
        self.LogMessage('INFO',Origin=origin,Msg='End iterative call: nIter=%s'%(nIter))

    @dec_stages_interface_logging
    def plotResultSpectra(self, vAnt=None, vIF=None, vPOL=None):

        """
        plotResultSpectra: method creates plots which show the resulting Spectra after the conducted steps of reduction
        It works for a given IF and Polarization.

        parameter: vIF, vPOL
        """
        origin = 'plotResultSpectra()'
        
        (vAnt,vIF,vPOL,antIdx,polIdx) = self.__handleIndex(vAnt,vIF,vPOL)

        # 2009/10/19 for TP data
        if self.isTP:
            self.LogMessage('INFO', Origin=origin, Msg='plotResultSpectra was skipped because it is a TP data')
            return

        # 2009/10/29 Predefined window
        if len(self.SpectrumWindow) != 0 and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='plotResultSpectra was skipped because spectrum window is set')
            return

        # skip if self.gridData doesn't have keys
        if not self.gridData.has_key(vAnt) or not self.gridData[vAnt].has_key(vPOL):
            return

        # set up something here...
        HtmlDir = self.HtmlDir[vAnt]
        #lastData = self.lastData[vAnt]
        lastData = self.gridData[vAnt][vPOL]
        rowsSel = self.rowsidSel[antIdx][polIdx]
        ResultTable = self.GridResult[vAnt][vPOL]

        # Show Result Spectra
        if self.BPlotMultiSP:
            FigFileDir = HtmlDir+'/'+BF_GRID_PDIR+'/'
        else:
            FigFileDir = False

        #self.engine.DrawMultiSpectraNavigator(vIF, vPOL, self.iteration, self.ResultTable, rowsSel, self.spacing, self.Pattern, FigFileDir=FigFileDir, FigFileRoot='Result_%s_%s_%s' % (vIF, vPOL,self.iteration))
        self.engine.DrawMultiSpectraNavigator(vIF, vPOL, self.iteration, ResultTable, rowsSel, self.spacing, self.Pattern, FigFileDir=FigFileDir, FigFileRoot='Result_%s_%s_%s' % (vIF, vPOL,self.iteration))

        #SDP.DrawMultiSpectra(lastData, self.ResultTable, self.Abcissa[vIF], range(len(self.ResultTable)), self.Pattern, chan0=self.edge[0], chan1=(self.NCHAN - self.edge[1] - 1), ShowPlot=self.TPlotMultiSP, FigFileDir=FigFileDir, FigFileRoot='Result_%s_%s_%s' % (vIF, vPOL,self.iteration))
        SDP.DrawMultiSpectra(lastData, ResultTable, self.Abcissa[vIF], range(len(ResultTable)), self.Pattern, chan0=self.edge[0], chan1=(self.NCHAN - self.edge[1] - 1), ShowPlot=self.TPlotMultiSP, FigFileDir=FigFileDir, FigFileRoot='Result_%s_%s_%s' % (vIF, vPOL,self.iteration))
        SDH.HtmlMultiSpectra(HtmlDir)


    #@dec_stages_interface_logging
    def plotChannelMapsRunThrough(self, SWconfig=None):
        """
        """
        origin = 'plotChannelMapsRunThrough'

        # 2011/11/15 TN
        # Skip if TP data here instead of each iterative call
        # Also skip if predefined window is set and second iteration or later 
        if self.isTP and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='plotChannelMaps was skipped because it is a TP data and iteration > 1')
            return
        if len(self.SpectrumWindow) != 0 and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='plotChannelMaps was skipped because spectrum window is set and iteration > 1')
            return

        (vAnt,vIF,vPOLs) = self.__configureListFromSWConfig(SWconfig)

        # iterative call of plotChannelMaps
        nIter = len(vAnt)
        self.LogMessage('INFO',Origin=origin,Msg='Start iterative call: nIter=%s'%(nIter))
        for i in xrange(nIter):
            nPol = len(vPOLs[i])
            self.LogMessage('INFO',Origin=origin,Msg='nPol=%s'%(nPol))
            for j in xrange(nPol):
                self.plotChannelMaps( vAnt[i], vIF[i], vPOLs[i][j] )
        self.LogMessage('INFO',Origin=origin,Msg='End iterative call: nIter=%s'%(nIter))

    @dec_stages_interface_logging
    def plotChannelMaps(self, vAnt=None, vIF=None, vPOL=None):

        """
        plotChannelMaps: method creates Channel Maps for each Cluster detected in prvious steps of the reduction
        It works for a given IF and Polarization.

        parameter: vIF, vPOL
        """
        origin = 'plotChannelMaps()'
        
        (vAnt,vIF,vPOL,antIdx,polIdx) = self.__handleIndex(vAnt,vIF,vPOL)

        # 2009/10/29 Predefined window
        if len(self.SpectrumWindow) != 0 and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='plotChannelMaps was skipped because spectrum window is set')
            return

        # 2009/10/19 for TP data: Plot only once for TP mode
        if self.isTP and self.iteration != 1:
            self.LogMessage('INFO', Origin=origin, Msg='plotChannelMaps was skipped because it is a TP data and the iteration is %s' % self.iteration)
            return

        # skip if self.gridData doesn't have keys
        if not self.gridData.has_key(vAnt) or not self.gridData[vAnt].has_key(vPOL):
            return

        # set appropriate beam size
        # 2012/03/01 reuse beam radius set by regridData
        #self.__setRadiusFromBeamSize( vAnt, vIF )

        # set up something here...
        HtmlDir = self.HtmlDir[vAnt]
        lastData = self.gridData[vAnt][vPOL]
        if self.continuumOutput == True:
            lastCont = self.gridCont[vAnt][vPOL]
        else:
            lastCont = ''
        ResultTable = self.GridResult[vAnt][vPOL]

        # Draw Channel Maps for each Cluster
        if self.Pattern.upper() == 'RASTER':
            if self.BPlotChannelMap:
                FigFileDir = HtmlDir+'/'+BF_CHANNELMAP_PDIR+'/'
            else:
                FigFileDir = False
            if not self.isTP:
                #SDP.DrawImage(lastData, self.ResultTable, self.Abcissa[vIF], self.Lines, self.radius, self.spacing, scale_max=False, scale_min=False, colormap=self.colormap, ShowPlot=self.TPlotChannelMap, FigFileDir=FigFileDir, FigFileRoot='ChannelMap_%s_%s_%s' % (vIF, vPOL,self.iteration))
                SDP.DrawImage(lastData, ResultTable, self.Abcissa[vIF], self.Lines, self.radius, self.spacing, scale_max=False, scale_min=False, colormap=self.colormap, ShowPlot=self.TPlotChannelMap, FigFileDir=FigFileDir, FigFileRoot='ChannelMap_%s_%s_%s' % (vIF, vPOL,self.iteration))
                if self.continuumOutput == True:
                    #SDP.DrawContImage(lastCont, self.ResultTable, self.Abcissa[vIF], self.radius, self.spacing, scale_max=False, scale_min=False, colormap=self.colormap, ShowPlot=self.TPlotChannelMap, FigFileDir=FigFileDir, FigFileRoot='ChannelMap_%s_%s_%s' % (vIF, vPOL,self.iteration))
                    SDP.DrawContImage(lastCont, ResultTable, self.Abcissa[vIF], self.radius, self.spacing, scale_max=False, scale_min=False, colormap=self.colormap, ShowPlot=self.TPlotChannelMap, FigFileDir=FigFileDir, FigFileRoot='ChannelMap_%s_%s_%s' % (vIF, vPOL,self.iteration))
            else:
                #SDP.DrawTPImage(lastData, self.ResultTable, self.radius, self.spacing, scale_max=False, scale_min=False, colormap=self.colormap, ShowPlot=self.TPlotChannelMap, FigFileDir=FigFileDir, FigFileRoot='ChannelMap_%s_%s_%s' % (vIF, vPOL,self.iteration))
                SDP.DrawTPImage(lastData, ResultTable, self.radius, self.spacing, scale_max=False, scale_min=False, colormap=self.colormap, ShowPlot=self.TPlotChannelMap, FigFileDir=FigFileDir, FigFileRoot='ChannelMap_%s_%s_%s' % (vIF, vPOL,self.iteration))
            SDH.HtmlGridding(HtmlDir)


    #@dec_stages_interface_logging
    def plotSparseSpectraMapRunThrough(self, SWconfig=None):
        """
        """
        origin = 'plotSparseSpectraMapRunThrough'

        # 2011/11/15 TN
        # Skip if TP data here instead of each iterative call
        # Also skip if predefined window is set and second iteration or later 
        if self.isTP:
            self.LogMessage('INFO', Origin=origin, Msg='plotSparseSpectraMap was skipped because it is a TP data')
            return
        if len(self.SpectrumWindow) != 0 and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='plotSparseSpectraMap was skipped because spectrum window is set and iteration > 1')
            return

        (vAnt,vIF,vPOLs) = self.__configureListFromSWConfig(SWconfig)

        # iterative call of plotSparseSpectraMap 
        nIter = len(vAnt)
        self.LogMessage('INFO',Origin=origin,Msg='Start iterative call: nIter=%s'%(nIter))
        for i in xrange(nIter):
            nPol = len(vPOLs[i])
            self.LogMessage('INFO',Origin=origin,Msg='nPol=%s'%(nPol))
            for j in xrange(nPol):
                self.plotSparseSpectraMap( vAnt[i], vIF[i], vPOLs[i][j] )
        self.LogMessage('INFO',Origin=origin,Msg='End iterative call: nIter=%s'%(nIter))


    @dec_stages_interface_logging
    def plotSparseSpectraMap(self, vAnt=None, vIF=None, vPOL=None):

        """
        plotSparseSpectraMap: method creates Sparse Spectra Maps
        It works for a given IF and Polarization.

        parameter: vIF, vPOL
        """
        origin = 'plotSparseSpectraMap()'

        (vAnt,vIF,vPOL,antIdx,polIdx) = self.__handleIndex(vAnt,vIF,vPOL)

        # 2009/10/19 for TP data
        if self.isTP:
            self.LogMessage('INFO', Origin=origin, Msg='plotSparseSpectraMap was skipped because it is a TP data')
            return

        # 2009/10/29 Predefined window
        if len(self.SpectrumWindow) != 0 and self.iteration > 1:
            self.LogMessage('INFO', Origin=origin, Msg='plotSparseSpectraMap was skipped because spectrum window is set and the iteration is %s' % self.iteration)
            return

        # skip if self.gridData doesn't have keys
        if not self.gridData.has_key(vAnt) or not self.gridData[vAnt].has_key(vPOL):
            return

        # set up something here...
        HtmlDir = self.HtmlDir[vAnt]
        lastData = self.gridData[vAnt][vPOL]
        ResultTable = self.GridResult[vAnt][vPOL]

        if self.BPlotSparseSP:
            FigFileDir = HtmlDir+'/'+BF_SPARSESP_PDIR+'/'
        else:
            FigFileDir = False

        SDP.DrawSparseSpectra(lastData, ResultTable, self.Abcissa[vIF], MaxPanels4SparseSP, chan0=self.edge[0], chan1=(self.NCHAN - self.edge[1] - 1), ShowPlot=self.TPlotSparseSP, FigFileDir=FigFileDir, FigFileRoot='SparseSpMap_%s_%s_%s' % (vIF, vPOL,self.iteration))
        SDH.HtmlSparseSpectraMap(HtmlDir)


    @dec_stages_interface_logging
    def createHTML(self): 

        """
        createHTML: compiles the retrieved results as HTML pages for further inspection
        """
        origin = 'createHTML()'

        SDJ.SDjava2html(self.outDir+self.outFile)


    def getIFPolList(self):
        """
        Get a list of (IF,pol) pair that have corresponding spectral data
        in the main table of the scantable.
        """
        origin = 'getIFPolList()'
        #self.setFileOut( self.LogDir+'/' + self.SD_PIPELINE_LOG )
        self.setFileOut( self.GlobalLogDir+'/' + self.SD_PIPELINE_LOG )
        
        scan=scantable(self.workDir+self.workFile,False)
        ifnos = scan.getifnos()
        polnos = scan.getpolnos()

        ifpollist = []
        for ii in ifnos:
            if not ( ii in self.IF ):
                continue
            for ip in polnos:
                if not ( ip in self.POL ):
                    continue
                sel = selector()
                sel.set_ifs(ii)
                sel.set_polarizations(ip)
                try:
                    scan.set_selection(sel)
                except RuntimeError, e:
                    scan.set_selection()
                    continue
                scan.set_selection()
                ifpollist.append( (ii,ip) )

        self.LogMessage('DEBUG', Origin=origin, Msg='IFPolList=%s'%(ifpollist))
        return ifpollist

    def getLoopElementsPerSpw(self):
        """
        Return a list of (Ant,IF,(Pols)) tuples that can be used to make
        a loop to process whole data.

        Returned list will be classified based on spectral window grouping.

        Return value:
            [ [(Ant_00,IF_00,(Pol_000...)),...(Ant_0N,IF_0N,(Pol_0N0...))]
              [(Ant_10,IF_10,(Pol_100...)),...(Ant_1N,IF_1N,(Pol_1N0...))]
              ...
              [(Ant_M0,IF_M0,(Pol_M00...)),...(Ant_MN,IF_MN,(Pol_MN0...))] ]
        """
        # assume that self.selectAntenna() is already called
        
        # return Ant, IF, Pol filtering result
        # { Ant_0: { IF_0: [<valid Pol list>] or False,
        #            IF_1: [<valid Pol list>] or False,
        #            ...
        #            IF_N: [<valid Pol list>] or False },
        #   ...
        #   Ant_X: { IF_0: [<valid Pol list>] or False,
        #            IF_1: [<valid Pol list>] or False,
        #            ...
        #            IF_M: [<valid Pol list>] or False } }       
        filter = self.__filterElements()
        print 'filter=%s'%(filter)

        # return spw map
        # [ [(Ant_0,IF_X),(Ant_1,IF_Y),...],
        #   [(Ant_0,IF_A),(Ant_1,IF_B),...],
        #   ...
        #   [(Ant_0,IF_K),(Ant_1,IF_L),...] ]
        spwMap = self.__spwMap( filter )
        print 'spwMap=%s'%(spwMap)

        # configure return value
        ret = []
        for spw in spwMap:
            row = []
            for val in spw:
                ant = val[0]
                ifno = val[1]
                if filter[ant].has_key(ifno) and filter[ant][ifno] is not False:
                    row.append( (ant,ifno,filter[ant][ifno]) )
            if len(row) > 0:
                ret.append( row )

        return ret

    def __spwMap( self, filter=None ):
        """
        return spw map

        filter   filter for (Ant,IF) pair

        { Ant_0: { IF_0: [<valid Pol list>] or False,
                   IF_1: [<valid Pol list>] or False,
                   ...
                   IF_N: [<valid Pol list>] or False },
          ...
          Ant_X: { IF_0: [<valid Pol list>] or False,
                   IF_1: [<valid Pol list>] or False,
                   ...
                   IF_M: [<valid Pol list>] or False } } 

        Return value:
        [ [(Ant_0,IF_X),(Ant_1,IF_Y),...],
          [(Ant_0,IF_A),(Ant_1,IF_B),...],
          ...
          [(Ant_0,IF_K),(Ant_1,IF_L),...] ]
        """
        # assume that self.selectAntenna() is already executed.

        origin = '__spwMap()'

        ret = []
        coverageList = None
        # single data input is special case
        if len(self.dataSummary) == 1:
            # no spw mapping is needed
            spwids = self.dataSummary[0]['spw'].keys()
            for spw in spwids:
                l = []
                for iant in xrange(self.numAnt):
                    ant = self.antKeys[iant]
                    if filter is not None \
                           and filter[ant].has_key(spw) \
                           and filter[ant][spw] is not False:
                        l.append( (self.antKeys[iant],spw) )
                if len(l) > 0:
                    ret.append( l )
        else:
            numRawFile = len(self.rawFileList)
            spwgrp = []
            coverageList = []
            import numpy
            dataFormat = numpy.array( [self.dataSummary[i]['type'] for i in xrange(numRawFile)] )
            if all( dataFormat == 'ASAP' ):
                # all inputs are Scantable, so it's relatively easy
                # frame is assumed to be 'LSRK'
                for iant in xrange(self.numAnt):
                    # Scantable includes only one antenna
                    ant = self.antKeys[iant]
                    summary = self.dataSummary[iant]
                    spwSummary = summary['spw']
                    for spw in spwSummary.keys():
                        if filter is not None \
                               and filter[ant].has_key(spw) \
                               and filter[ant][spw] is not False:
                            freqRange = spwSummary[spw]['freq']
                            nchan = spwSummary[spw]['nchan']
                            self.__updateSpwMap( ret,
                                                 spwgrp,
                                                 ant,
                                                 spw,
                                                 freqRange,
                                                 nchan,
                                                 coverageList )
            else:
                # split Scantables have to be examined
                # frame is assumed to be 'LSRK'
                for key in self.antKeys:
                    filename = self.workDir + self.antFile[key]
                    d = getSummary(filename)
                    spwSummary = d['spw']
                    for spw in spwSummary.keys():
                        if filter is not None \
                               and filter[key].has_key(spw) \
                               and filter[key][spw] is not False:
                            freqRange = spwSummary[spw]['freq']
                            nchan = spwSummary[spw]['nchan']
                            self.__updateSpwMap( ret,
                                                 spwgrp,
                                                 key,
                                                 spw,
                                                 freqRange,
                                                 nchan,
                                                 coverageList )
        #self.LogMessage('INFO',Origin=origin,Msg='coverageList=%s'%(coverageList))
        self.__frequencyResampling( ret, coverageList )
                
        return ret

    def __frequencyResampling( self, grp, clist=None ):
        origin = '__frequencyResampling()'
        if clist is None:
            clist = self.__makeCoverageList( grp )
        #self.LogMessage('INFO',Origin=origin,Msg='clist=%s'%(clist))

        for i in xrange(len(clist)):
            cgrp = clist[i]
            if len(cgrp) == 1:
                continue
            else:
                refAnt = grp[i][0][0]
                refIF = grp[i][0][1]
                (refPix,refVal,incr) = self.__getFrequencySpec(refAnt,refIF)
                for j in xrange(1,len(cgrp)):
                    targetAnt = grp[i][j][0]
                    targetIF = grp[i][j][1]
                    if cgrp[j] > 0.999:
                        # just shift axis
                        self.__putFrequencySpec(targetAnt,targetIF,refPix,refVal,incr)
                    else:
                        self.LogMessage('ERROR',Origin=origin,Msg='Not implemented yet.')

    def __putFrequencySpec( self, ant, ifno, rp, rv, ic ):
        origin='__putFrequencySpec()'
        #self.LogMessage('INFO',Origin=origin,Msg='ant=%s,ifno=%s'%(ant,ifno))
        targetData = self.workDir+self.workFile[ant]
        self._tb.open(targetData)
        #freqTab = self._tb.getkeyword('FREQUENCIES').split()[-1]
        freqTab = self._tb.getkeyword('FREQUENCIES').lstrip('Table: ')
        #self.LogMessage('INFO',Origin=origin,Msg='freqTab=%s'%(freqTab))
        tsel = self._tb.query('IFNO==%s'%(ifno))
        #self.LogMessage('INFO',Origin=origin,Msg='tsel.nrow()=%s'%(tsel.nrows()))
        freqId = tsel.getcell('FREQ_ID',0)
        tsel.close()
        self._tb.close()
        self._tb.open(freqTab,nomodify=False)
        tsel = self._tb.query('ID==%s'%(freqId))
        tsel.putcell('REFPIX',0,rp)
        tsel.putcell('REFVAL',0,rv)
        tsel.putcell('INCREMENT',0,ic)
        tsel.flush()
        tsel.close()
        self._tb.flush()
        self._tb.close()
        #self.LogMessage('INFO',Origin=origin,Msg='rp=%s,rv=%s,ic=%s'%(rp,rv,ic))
    def __getFrequencySpec( self, ant, ifno ):
        origin='__getFrequencySpec()'
        #self.LogMessage('INFO',Origin=origin,Msg='ant=%s,ifno=%s'%(ant,ifno))
        refData = self.workDir+self.workFile[ant]
        self._tb.open(refData)
        #freqTab = self._tb.getkeyword('FREQUENCIES').split()[-1]
        freqTab = self._tb.getkeyword('FREQUENCIES').lstrip('Table: ')
        #self.LogMessage('INFO',Origin=origin,Msg='freqTab=%s'%(freqTab))
        tsel = self._tb.query('IFNO==%s'%(ifno))
        #self.LogMessage('INFO',Origin=origin,Msg='tsel.nrow()=%s'%(tsel.nrows()))
        freqId = tsel.getcell('FREQ_ID',0)
        tsel.close()
        self._tb.close()
        self._tb.open(freqTab)
        tsel = self._tb.query('ID==%s'%(freqId))
        rp = tsel.getcell('REFPIX',0)
        rv = tsel.getcell('REFVAL',0)
        ic = tsel.getcell('INCREMENT',0)
        tsel.close()
        self._tb.close()
        #self.LogMessage('INFO',Origin=origin,Msg='rp=%s,rv=%s,ic=%s'%(rp,rv,ic))
        return (rp,rv,ic)


    def __makeCoverageList( self, grp ):
        origin = '__makeCoverageList()'
        clist = []
        summaryList = {}
        #self.LogMessage('INFO',Origin=origin,Msg='grp=%s'%(grp))
        for key in self.antKeys:
            summaryList[key] = getSummary(self.workDir+self.antFile[key])
        for i in xrange(len(grp)):
            elems = grp[i]
            clist.append([-1.0])
            refAnt = elems[0][0]
            refIF = elems[0][1]
            refSummary = summaryList[refAnt]
            refFreq = refSummary['spw'][refIF]['freq']
            for j in xrange(1,len(elems)):
                ant = elems[j][0]
                ifno = elems[j][1]
                freq = summaryList[ant]['spw'][ifno]['freq']
                coverage = self.__freqCoverage(refFreq,freq)
                clist[i].append(coverage)
        return clist

    def __updateSpwMap( self, grp, grpF, ant, spw, freq, nchan, clist ):
        numGrp = len(grp)
        idx = -1
        allowance = 0.99
        for i in xrange(numGrp):
            gf = grpF[i]
            if gf[0] == nchan:
                coverage = self.__freqCoverage(gf[1:],freq)
                if self.__checkFreqRange(coverage,allowance):
                    idx = i
                    grp[i].append( (ant,spw) )
                    clist[i].append(coverage)
                    break
        if idx == -1:
            grp.append( [ (ant,spw) ] )
            grpF.append( [ nchan, freq[0], freq[1] ] )
            clist.append( [-1.0] )
        

    def __freqCoverage( self, grpF, freq ):
        overlap = max( 0.0, min(grpF[1],freq[1]) - max(grpF[0],freq[0]))
        width = max(grpF[1],freq[1]) - min(grpF[0],freq[0])
        coverage = overlap/width
        return coverage

    def __checkFreqRange( self, coverage, allowance=1.0 ):
        if coverage >= allowance: 
            return True
        else:
            return False

    def __filterElements( self ):
        """
        return Ant, IF, Pol filtering result
        { Ant_0: { IF_0: [<valid Pol list>] or False,
                   IF_1: [<valid Pol list>] or False,
                   ...
                   IF_N: [<valid Pol list>] or False },
          ...
          Ant_X: { IF_0: [<valid Pol list>] or False,
                   IF_1: [<valid Pol list>] or False,
                   ...
                   IF_M: [<valid Pol list>] or False } }       
        """
        # assume self.selectAntenna() is already executed

        origin = '__filterElements()'

        # if self.IF and self.POL is 'all', just exclude WVR and CAL
        checkIF = True
        checkPOL = True
        if type(self.IF) == str and self.IF == 'all':
            checkIF = False
        checkPOL = True
        if type(self.POL) == str and self.POL == 'all':
            checkPOL = False

        # main process
        ret = {}
        for iant in xrange(self.numAnt):
            key=self.antKeys[iant]
            idx=self.rawFileIdx[key]
            spws = self.dataSummary[idx]['spw']
            d = {}
            for ii in spws.keys():
                spw=spws[ii]
                # skip spw for calibration 
                if spw['intent'].find('CALIB') == 0:
                    self.LogMessage('INFO',Origin=origin,Msg='Ant%s: Skip IF%s since specific window for calibration'%(iant,ii))
                    continue
                # skip WVR scan
                elif spw['type'] == 'WVR':
                    self.LogMessage('INFO',Origin=origin,Msg='Ant%s: Skip IF%s since WVR data'%(iant,ii))
                    continue
                elif checkIF and not ( ii in self.IF[key] ):
                    continue
                else:
                    pol = self.dataSummary[idx]['pol']
                    polnos = pol[spw['polid'][0]]['polno']
                    pollist = []
                    for ip in polnos:
                        if checkPOL and not ( ip in self.POL[key] ):
                            continue
                        pollist.append( ip )
                    if len(pollist) > 0:
                        d[ii] = pollist
                    else:
                        d[ii] = False
            ret[key] = d

        return ret

    def getLoopElements(self):
        """
        Get a list of (Ant,IF,Pol) pair that can be used to make a loop
        to process whole data.
        """
        origin = 'getLoopElements()'
        # normally, self.LogDir is not set yet here
        #self.setFileOut( self.LogDir+'/' + self.SD_PIPELINE_LOG )

        loopElements = []
        filter = self.__filterElements()
        for ant in filter.keys():
            for ifno in filter[ant].keys():
                if filter[ant][ifno] is not False:
                    for pol in filter[ant][ifno]:
                        loopElements.append((ant,ifno,pol))

        self.LogMessage('DEBUG',Origin=origin,Msg='LoopElements=%s'%(loopElements))
        return loopElements
            
    def checkCal( self, files ):
        """
        Check if calibration is needed or not.
        Data specified by files string (list) should be Scantable format.
        """
        origin = 'checkCal()'
        #self.setFileOut( self.LogDir+'/'+self.SD_PIPELINE_LOG )

        doCal=[]
        import numpy
        if type(files) == str:
            files = [files]
        for ifile in xrange(len(files)):
            self._tb.open(files[ifile])
            srctype=self._tb.getcol('SRCTYPE')
            self._tb.close()
            if len(numpy.unique(srctype)) == 1:
                self.LogMessage('INFO', Origin=origin, Msg='Calibrated data' )
                doCal.append( False )
            else:
                self.LogMessage('INFO', Origin=origin, Msg='Raw data' )
                doCal.append( True )
        
        return doCal

    def createWorkFileWithoutWVRScan(self, antID):
        """
        create work file that doesn't contain WVR scan.
        current criteria for WVR scan is nchan(IF) == 4
        """
        origin = 'createWorkFileWithoutWVRScan()'
        
        #filename = self.workDir + self.antFile[self.iterAnt]
        filename = self.workDir + self.antFile[antID]
        idx = self.rawFileIdx[antID]

        if self.WVRScan is None or len(self.WVRScan) == 0:
            self.detectWVRScan()
        CALScan = self.detectCALScan(idx)
        excludeScan = CALScan + self.WVRScan[idx]

        s=scantable(filename,False)
        if len(excludeScan) == 0:
            # just copy
            #s.save(self.workDir+self.workFile,'ASAP',True)
            s.save(self.workDir+self.workFile[antID],'ASAP',True)
            del s
        else:
            # select data
            iflist = []
            for iIF in s.getifnos():
                if not ( iIF in excludeScan ): 
                    iflist.append(iIF)
            #self.LogMessage('INFO',Origin='',Msg='iflist=%s'%(iflist))
            sel=selector()
            sel.set_ifs(iflist)
            s.set_selection(sel)
            #s.save(self.workDir+self.workFile,'ASAP',True)
            s.save(self.workDir+self.workFile[antID],'ASAP',True)
            del s
            del sel
        return
    
##     def nextAntenna(self):
##         """
##         Go to next antenna.
##         """
##         self.iterAnt = self.iterAnt + 1

##     def endAntennaLoop(self):
##         """
##         Return True if all antennas are processed.
##         """
##         endProc = None
##         if self.iterAnt < self.numAnt:
##             endProc = False
##         else:
##             endProc = True
##         return endProc

    def getWeather( self, antID, weather ):
        """
        Return weather information for current antenna.
        Types of weather information should be specified as a string list.

        Example:
            self.getWeather( ['TEMPERATURE','PRESSURE'] )

        Possible weather type:
            TEMPERATURE
            PRESSURE
            REL_HUMIDITY
            WIND_DIRECTION
            WIND_SPEED

        Return:
            Dictionary that contains TIME and weather informations.
            List elements given as argument will be keys for weather info.
            {'TIME': [...],
             '<weather[0]>': [...],
             '<weather[1]>': [...],
             ...}
        """
        origin = 'getWeather()'

        tname = ''
        idx = self.rawFileIdx[antID]
        rawFile = self.rawFileList[idx]
        dataFormat = self.dataFormat[idx]
        if dataFormat == 'ASDM':
            tname = self.workDir + rawFile + '.ms'
        elif dataFormat == 'MS2' or dataFormat == 'ASAP':
            tname = self.rawDir + rawFile
        else:
            tname = self.workDir + self.workFile[antID]
        self._tb.open(tname)
        #wtable = self._tb.getkeyword('WEATHER').split()[-1]
        wtable = self._tb.getkeyword('WEATHER').lstrip('Table: ')
            
        colnames = []
        winfo = {}
        if dataFormat == 'ASDM' or dataFormat == 'MS2':
            # read MS
            #atable = self._tb.getkeyword('ANTENNA').split()[-1]
            atable = self._tb.getkeyword('ANTENNA').lstrip('Table: ')
            antpos = []
            for col in weather:
                colnames.append( col.upper() )
            self._tb.open( atable )
            antennaId = self._tb.getcol('NAME').tolist().index(self.antName[antID])
            antpos = self._tb.getcell( 'POSITION', antennaId )
            self._tb.close()
            
            # get weather
            self._tb.open( wtable )
            antid = self._tb.getcol( 'ANTENNA_ID' )
            if antennaId in antid:
                # get rows with ANTENNA_ID == self.iterAnt
                tbsel = self._tb.query( 'ANTENNA_ID == %s'%antennaId )
                for i in xrange(len(colnames)):
                    winfo[weather[i]] = tbsel.getcol( colnames[i] )
                tbsel.close()
                del tbsel
            else:
                # get rows for nearest weather station
                wxids = NP.unique( self._tb.getcol( 'NS_WX_STATION_ID' ) )
                wxpos = []
                for id in wxids:
                    tbsel = self._tb.query( 'NS_WX_STATION_ID == %s'%id )
                    wxpos.append( tbsel.getcell( 'NS_WX_STATION_POSITION', 0 ) )
                    tbsel.close()
                    del tbsel
                sep = None
                nearest = -1
                for id in xrange(len(wxids)):
                    dist = 0.0
                    dpos = NP.array(antpos) - NP.array(wxpos[id])
                    dist = NP.sqrt((NP.square(dpos)).sum())
                    #self.LogMessage( 'DEBUG', Origin=origin, Msg='wxid[%s]=%s: dist = %s'%(id,wxids[id],dist) )
                    if sep == None or dist < sep:
                        sep = dist
                        nearest = wxids[id]
                #self.LogMessage( 'DEBUG', Origin=origin, Msg='nearest = %s'%(nearest) )
                tbsel = self._tb.query( 'NS_WX_STATION_ID == %s'%nearest )
                winfo['TIME'] = tbsel.getcol( 'TIME' )
                for iwed in xrange(len(weather)):
                    winfo[weather[iwed]] = tbsel.getcol( colnames[iwed] )
                tbsel.close()
                del tbsel
            self._tb.close()
        else:
            # read Scantable
            for col in weather:
                if col.upper() == 'REL_HUMIDITY':
                    colnames.append( 'HUMIDITY' )
                elif col.upper() == 'WIND_SPEED':
                    colnames.append( 'WINDSPEED' )
                elif col.upper() == 'WIND_DIRECTION':
                    colnames.append( 'WINDAZ' )
                else:
                    colnames.append( col.upper() )
            self._tb.open( wtable )
            tmpwed = {}
            wkeys = self._tb.getcol( 'ID' )
            for col in colnames:
                tmpwed[col] = {}
                wed = self._tb.getcol( col )
                for k in xrange(len(wkeys)):
                    tmpwed[col][wkeys[k]] = wed[k]
            self._tb.close()
            self._tb.open( tname )
            # should be converted to 'sec' since TIME is stored as 'day' 
            winfo['TIME'] = self._tb.getcol( 'TIME' ) * 86400.0
            wid = self._tb.getcol( 'WEATHER_ID' )
            for icol in xrange(len(weather)):
                wlist = []
                for jcol in xrange(self._tb.nrows()):
                    wlist.append( tmpwed[colnames[icol]][wid[jcol]] )
                winfo[weather[icol]] = NP.array( wlist )
            self._tb.close()

        return winfo

    def detectWVRScan(self):
        """
        """
        origin = 'detectWVRScan()'
        #self.setFileOut( self.LogDir+'/' + self.SD_PIPELINE_LOG )

        WVRScan = [[] for i in xrange(len(self.rawFileList))]
        for i in xrange(len(self.rawFileList)):
            WVRScan[i] = []
            summary = self.dataSummary[i]
            spwDic = summary['spw']
            for key in spwDic:
                if spwDic[key]['type'] == 'WVR':
                    self.LogMessage('INFO', Origin=origin, Msg='%s: Detected WVR Scan:  IF=%s'%(self.rawFileList[i],key))
                    WVRScan[i].append( key )
        self.WVRScan = WVRScan
        #self.LogMessage('INFO',Origin=origin,Msg='self.WVRScan=%s'%(self.WVRScan))

    def detectCALScan(self,fileID):
        calScan = []
        summary = self.dataSummary[fileID]
        spwDic = summary['spw']
        for key in spwDic:
            if spwDic[key]['intent'].find('CALIB') != -1:
                calScan.append( key )
        return calScan
        

    def saveWVRData(self,antID):
        """
        """
        origin = 'saveWVRData()'
        #self.setFileOut( self.LogDir+'/' + self.SD_PIPELINE_LOG )

        import asap as sd

        filename = self.workDir + self.antFile[antID]

        idx = self.rawFileIdx[antID]
        if self.WVRScan is None or len(self.WVRScan) == 0:
        #if len(self.WVRScan) == 0:
            self.detectWVRScan()

        WVRScan = self.WVRScan[idx]
        if len(WVRScan) == 0:
            self.LogMessage('INFO', Origin=origin, Msg='No WVR data' )
            return

        #self.WVRFile = self.rawFile + '_WVR'
        self.WVRFile[antID] = self.workFile[antID] + '_WVR'
        s = sd.scantable(filename,False)
        sel = sd.selector()
        sel.set_ifs( WVRScan )
        s.set_selection( sel )
        s.save( self.workDir + self.WVRFile[antID], 'ASAP', True )
        del s
        del sel

    def getWVR( self,antID ):
        """
        Return list of WVR data

        Returns:
            Followeing list of WVR data as numpy array
            
            [ [TIME_0,   TIME_1,   ..., TIME_N],
              [WVR_0[0], WVR_1[0], ..., WVR_N[0]],
              [WVR_0[1], WVR_1[1], ..., WVR_N[1]],
              [WVR_0[2], WVR_1[2], ..., WVR_N[2]],
              [WVR_0[3], WVR_1[3], ..., WVR_N[3]] ]

            False if no WVR data
        """
        origin = 'getWVR()'

        import numpy

        if self.WVRScan is None or len(self.WVRScan) == 0:
        #if len(self.WVRScan) == 0:
            self.detectWVRScan()

        idx = self.rawFileIdx[antID]
        if len(self.WVRScan[idx]) == 0:
            return False
        
        WVRScan = self.WVRScan[idx]
        if len(WVRScan) == 0:
            self.LogMessage('INFO', Origin=origin, Msg='No WVR data' )
            return False
            
        if not self.WVRFile.has_key(antID):
            self.saveWVRData(antID)
        
        self._tb.open( self.workDir + self.WVRFile[antID] )

        timecol = self._tb.getcol( 'TIME' ) * 86400.0 # Day -> Sec
        timecol = timecol.reshape( 1,timecol.shape[0] )

        wvrdata = self._tb.getcol( 'SPECTRA' )

        val = numpy.concatenate( [timecol,wvrdata] )
        
        self._tb.close()

        return val
            
    def getWVRFreq( self,antID ):
        """
        Return list of WVR frequencies.
        """
        origin = 'getWVRFreq()'

        import numpy

        if self.WVRScan is None or len(self.WVRScan) == 0:
        #if len(self.WVRScan) == 0:
            self.detectWVRScan()

        idx = self.rawFileIdx[antID]
        if len(self.WVRScan[idx]) == 0:
            return False
        
        WVRScan = self.WVRScan[idx]
        if len(WVRScan) == 0:
            self.LogMessage('INFO', Origin=origin, Msg='No WVR data' )
            return False
            
        #if self.WVRFile is None:
        if not self.WVRFile.has_key(antID):
            self.saveWVRData(antID)

        freqval = []

        rawFile = self.rawFileList[idx]
        dataFormat = self.dataFormat[idx]
        if dataFormat == 'ASDM':
            tname = self.workDir + rawFile + '.ms'
        elif dataFormat == 'MS2':
            tname = self.rawDir + rawFile
        else:
            tname = self.workDir + self.WVRFile[antID]

        if dataFormat == 'ASDM' or dataFormat == 'MS2':
            self._tb.open(tname)
            #spwtab=self._tb.getkeyword('SPECTRAL_WINDOW').split()[-1]
            spwtab=self._tb.getkeyword('SPECTRAL_WINDOW').lstrip('Table: ')
            self._tb.close()
            self._tb.open(spwtab)
            numchan=self._tb.getcol('NUM_CHAN')
            for i in xrange(len(numchan)):
                if numchan[i] == 4: break
            freqval = self._tb.getcell( 'CHAN_FREQ', i ) 
            self._tb.close()
        else:
            self._tb.open(tname)
            #ftab=self._tb.getkeyword('FREQUENCIES').split()[-1]
            ftab=self._tb.getkeyword('FREQUENCIES').lstrip('Table: ')
            freqid = self._tb.getcell( 'FREQ_ID', 0 )
            nchan = len( self._tb.getcell( 'SPECTRA', 0 ) )
            self._tb.close()

            self._tb.open( ftab )
            refpix = self._tb.getcell( 'REFPIX', freqid ) - 1
            refval = self._tb.getcell( 'REFVAL', freqid )
            increm = self._tb.getcell( 'INCREMENT', freqid )
            self._tb.close()

            for ich in xrange(nchan):
                if ich == refpix:
                    freqval.append( refval )
                else:
                    val = refval + ( refpix - float(ich) ) * increm
                    freqval.append( val )
            freqval = numpy.array(freqval) 
        
        return freqval 

    @dec_stages_interface_logfile
    def cleanup( self ):
        """
        Clean up
           - delete engine instance
           - delete temporal files
        """
        origin = 'cleanup()'
        self.LogMessage('INFO',Origin=origin,Msg='Deleting temporary files...' )

        # delete temporal files
        deletefiles=[[self.workDir,self.workFile],
                     [self.outDir,self.outFiles],
                     [self.workDir,self.WVRFile],
                     [self.workDir,self.contFiles],
                     [self.workDir,self.antFile]]
        extensions=[]
        for ddir,dfile in deletefiles:
            if (ddir is None) or (dfile is None):
                continue
            if type(dfile) is dict:
                for key in dfile.keys():
                    filename=ddir+dfile[key]
                    self._deletefiles( filename, extensions, origin )
            elif type(dfile) is list:
                for fname in dfile:
                    filename=ddir+fname
                    self._deletefiles( filename, extensions, origin )
            else:
                filename=ddir+dfile
                self._deletefiles( filename, extensions, origin )

        if os.path.exists( self.tmpLogFile ):
            self.LogMessage('INFO',Origin=origin,Msg='Deleting %s ...'%self.tmpLogFile)
            os.remove( self.tmpLogFile )
        for key in self.calLogFile.keys():
            if os.path.exists( self.calLogFile[key] ):
                self.LogMessage('INFO',Origin=origin,Msg='Deleting %s ...'%self.calLogFile[key])
                os.remove( self.calLogFile[key] )

        for i in xrange(len(self.rawFileList)):
            if self.dataFormat[i] == 'ASDM':
                msname = self.outDir + self.rawFileList[i].rstrip('/') + '.ms'
                flagname = msname + '.flagversions'
                if os.path.exists( msname ):
                    self.LogMessage('INFO',Origin=origin,Msg='Deleting %s ...'%msname)
                    os.system( '\\rm -rf '+msname )
                if os.path.exists( flagname ):
                    self.LogMessage('INFO',Origin=origin,Msg='Deleting %s ...'%flagname)
                    os.system( '\\rm -rf '+flagname )

        # delete engine class
        if self.engine is not None:
            self.engine = None

        # post status
        message = self.status[0]
        color = self.status[1]
        self.frontPage.postStatus( message,color )
        self.frontPage.finishPage()

        # create profile
        if self.profilePage is not None:
            self.profilePage.makeTimeProfileTable()
            self.profilePage.makeTimeProfilePlot()

        # log endtime
        if self.frontPage is not None:
            endtime=time.gmtime()
            timestr=time.asctime(endtime)+' UTC'
            elapsed=time.mktime(endtime)-time.mktime(self.startTime)
            self.frontPage.postEndTime(endt=timestr,elapsedt=elapsed)

        # remove empty rows from product tables
        self._removeEmptyRowsFromProducts()

    def _removeEmptyRowsFromProducts(self):
        for key in self.antKeys:
            tbl = self.productName[key].getProductName(ant=self.antName[key])
            if os.path.exists( tbl ):
                self._tb.open(tbl,nomodify=False)
                rows = self._tb.query('!ISDEFINED(Sections)').rownumbers({},0)
                self._tb.removerows( rows )
                self._tb.flush()
                self._tb.close()

    def _deletefiles( self, filename, extensions, origin ):
        import os
        if os.path.exists( filename ):
            self.LogMessage('INFO',Origin=origin,Msg='Deleting %s ...'%filename)
            if os.path.isdir( filename ):
                #os.rmdir( filename )
                os.system( '\\rm -rf '+filename )
            else:
                os.remove( dfile )
        for ex in extensions:
            tmpname = filename.rstrip('/')+'.'+ex
            if os.path.exists( tmpname ):
                self.LogMessage('INFO',Origin=origin,Msg='Deleting %s ...'%tmpname)
                os.remove( tmpname )

    def selectAntenna( self ):
        """
        """
        origin='selectAntenna()'

        self.LogMessage('DEBUG',Origin=origin,Msg='self.ANTENNA=%s'%(self.ANTENNA))
        self.antName = {}
        if self.ANTENNA[0] == 'all':
            for key in self.antFile.keys():
                self._tb.open(self.workDir+'/'+self.antFile[key])
                antname=self._tb.getkeyword('AntennaName')
                pos=antname.find('//')
                if pos != -1:
                    antname=antname[pos+2:]
                pos=antname.find('@')
                if pos != -1:
                    antname=antname[:pos]
                self.antName[key]=antname
            return
        else:
            newlist = {}
            newidx = {}
            nant = len(self.antFile)
            antnames = {}
            for key in self.antFile.keys():
                name=self.antFile[key]
                self.LogMessage('DEBUG',origin,Msg='name=%s'%(name))
                self._tb.open(self.workDir+'/'+name)
                antname=self._tb.getkeyword('AntennaName')
                pos=antname.find('//')
                if pos != -1:
                    antname=antname[pos+2:]
                pos=antname.find('@')
                if pos != -1:
                    antname=antname[:pos]
                antnames[key] = antname
                self._tb.close()
            for ant in self.ANTENNA:
                if type(ant) == str:
                    for key in antnames.keys():
                        if antnames[key].find(ant) != -1:
                            newlist[key] = self.antFile[key]
                            newidx[key] = self.rawFileIdx[key]
                            self.antName[key] = antnames[key]
                else:
                    indexes = [0 for i in xrange(len(self.rawFileList))]
                    for key in self.antFile.keys():
                        fileIdx = self.rawFileIdx[key]
                        if indexes[fileIdx] == ant:
                            newlist[key] = self.antFile[key]
                            newidx[key] = self.rawFileIdx[key]
                            self.antName[key] = antnames[key]
                        indexes[fileIdx] += 1
            if len(newlist) == 0:
                self.LogMessage('ERROR',Origin=origin,Msg='Antenna selection excludes all antennas. Abort.')
                raise Exception('Antenna selection excludes all antennas.')
            self.LogMessage('DEBUG',Origin=origin,Msg='newlist=%s'%(newlist))
            # delete excluded files
            for name in self.antFile.values():
                if name not in newlist.values():
                    self.LogMessage('DEBUG',Origin=origin,Msg='Deleting %s...'%(name))
                    os.system('\\rm -rf %s'%(self.workDir+name))
            self.antFile=newlist
            self.rawFileIdx=newidx
            return 
                
    def addparam( self, **kwargs ):
        for k,v in kwargs.items():
            self.paramdic[k]=v
        self.writeparam()

    def writeparam( self ):
        if self.frontPage is not None and self.HtmlTopDir+'/'+self.frontPage.logDir is not None:
            f=open(self.HtmlTopDir+'/'+self.frontPage.logDir+'/params.txt','w')
            #for k,v in self.paramdic.items():
            #    f.write('%s = %s\n'%(k,v))

            dirpar=['rawdir',
                    'outdir',
                    'rawfile']
            self._writeparamcategory( f, 'Directory and Data', dirpar )
            procpar=['beamsize',
                     'edge',
                     'moments',
                     'linewindow',
                     'broadline',
                     'continuum',
                     'interactive',
                     'loglevel']
            self._writeparamcategory( f, 'Processing Parameters', procpar )
            flagpar=['tsys',
                     'tsys_thresh',
                     'weather',
                     'wea_thresh',
                     'prefitrmsexp',
                     'prrmse_thresh',
                     'postfitrmsexp',
                     'pormse_thresh',
                     'prefitrms',
                     'prrms_thresh',
                     'postfitrms',
                     'porms_thresh',
                     'prefitrunmean',
                     'prrme_thresh',
                     'prrme_nmean',
                     'postfitrunmean',
                     'porme_thresh',
                     'porme_nmean',
                     'userflag',
                     'uf_thresh']
            self._writeparamflag( f, 'Flagging Rule', flagpar )
            selpar=['antlist',
                    'iflist',
                    'pollist',
                    'rowlist',
                    'rowbase',
                    'scanlist',
                    'scanbase']
            self._writeparamcategory( f, 'Data Selection', selpar )
            calpar=['calmode']
            self._writeparamcategory( f, 'Data Calibration', calpar )
            blpar=['fitorder',
                   'fitfunc']
            self._writeparamcategory( f, 'Baseline Fit', blpar )
            plotpar=['plotcluster',
                     'plotflag']
            self._writeparamcategory( f, 'Data Plotting', plotpar )
            pardone=dirpar+procpar+flagpar+selpar+calpar+blpar+plotpar
            keys=self.paramdic.keys()
            for k in pardone:
                if k in keys:
                    keys.pop(keys.index(k))
            if len(keys) != 0:
                self._writeparamcategory( f, 'Other', keys )
            f.close()

    def _writeparamcategory( self, file, title, paramlist ):
        indent=''
        unknown='Not processed yet'
        print >> file, '#'
        print >> file, '# %s'%(title)
        print >> file, '#'
        for p in paramlist:
            if self.paramdic.has_key(p):
                print >> file, '%s%s = %s'%(indent,p,self.paramdic[p])
            else:
                print >> file, '%s%s = %s'%(indent,p,unknown)
        print >> file, ''

    def _writeparamflag( self, file, title, paramlist ):
        print >> file, '#'
        print >> file, '# %s'%(title)
        print >> file, '#'
        self._writeparamflagcategory( file, 'tsys', 'tsys_thresh' )
        self._writeparamflagcategory( file, 'weather', 'wea_thresh' )
        self._writeparamflagcategory( file, 'prefitrmsexp', 'prrmse_thresh' )
        self._writeparamflagcategory( file, 'postfitrmsexp', 'pormse_thresh' )
        self._writeparamflagcategory( file, 'prefitrms', 'prrms_thresh' )
        self._writeparamflagcategory( file, 'postfitrms', 'porms_thresh' )
        self._writeparamflagcategory( file, 'prefitrunmean', 'prrme_thresh', 'prrme_nmean' )
        self._writeparamflagcategory( file, 'postfitrunmean', 'porme_thresh', 'porme_nmean' )
        self._writeparamflagcategory( file, 'userflag', 'uf_thresh' )
        print >> file, ''

    def _writeparamflagcategory( self, file, rule, thresh, num=None ):
        indent=''
        indent2=indent+'   '
        unknown='unknown'
        print >> file, '%s%s = %s'%(indent,rule,self.paramdic[rule])
        if self.paramdic[rule]:
            print >> file, '%s%s = %s'%(indent2,thresh,self.paramdic[thresh])
            if num is not None:
                print >> file, '%s%s = %s'%(indent2,num,self.paramdic[num])
                
    def addflagparam( self ):
        frd=self.FlagRuleDictionary
        self.addparam( tsys=frd['TsysFlag']['isActive'],
                       tsys_thresh=frd['TsysFlag']['Threshold'],
                       weather=frd['WeatherFlag']['isActive'],
                       wea_thresh=frd['WeatherFlag']['Threshold'],
                       prefitrmsexp=frd['RmsExpectedPreFitFlag']['isActive'],
                       prrmse_thresh=frd['RmsExpectedPreFitFlag']['Threshold'],
                       postfitrmsexp=frd['RmsExpectedPostFitFlag']['isActive'],
                       pormse_thresh=frd['RmsExpectedPostFitFlag']['Threshold'],
                       prefitrms=frd['RmsPreFitFlag']['isActive'],
                       prrms_thresh=frd['RmsPreFitFlag']['Threshold'],
                       postfitrms=frd['RmsPostFitFlag']['isActive'],
                       porms_thresh=frd['RmsPostFitFlag']['Threshold'],
                       prefitrunmean=frd['RunMeanPreFitFlag']['isActive'],
                       prrme_thresh=frd['RunMeanPreFitFlag']['Threshold'],
                       prrme_nmean=frd['RunMeanPreFitFlag']['Nmean'],
                       postfitrunmean=frd['RunMeanPostFitFlag']['isActive'],
                       porme_thresh=frd['RunMeanPostFitFlag']['Threshold'],
                       porme_nmean=frd['RunMeanPostFitFlag']['Nmean'],
                       userflag=frd['UserFlag']['isActive'],
                       uf_thresh=frd['UserFlag']['Threshold'] )

    def __configureListFromSWConfig(self, SWconfig):
        origin='__configureListFromSWConfig'
        if SWconfig is None:
            vAnt = []
            for key in self.AntIndex.keys():
                vAnt.append(key)
            vIF = self.vIF
            vPOLs = self.vPOL
        else:
            vAnt = []
            vIF = []
            vPOLs = []
            for i in xrange(len(SWconfig)):
                vAnt.append(SWconfig[i][0])
                vIF.append(SWconfig[i][1])
                vPOLs.append(SWconfig[i][2])
        self.LogMessage('INFO',Origin=origin,Msg='vAnt=%s,vIF=%s,vPOLs=%s'%(vAnt,vIF,vPOLs))
        return (vAnt,vIF,vPOLs)

    def __handleIndex(self, vAnt, vIF, vPOL):
        origin = '__handleIndex()'
        if vAnt == None:
            vAnt = self.AntIndex.keys()[0]
        # antenna index
        antIdx = -1
        if vAnt not in self.antKeys:
            self.LogMessage('ERROR',Origin=origin,Msg='Antenna key %s is not in the list'%(vAnt))
            return
        antIdx = self.AntIndex[vAnt]

        if vIF == None: vIF = self.vIF[antIdx]
        if vPOL == None: vPOL = self.vPOL[antIdx][0]

        polIdx = self.vPOL[antIdx].index(vPOL)

        self.LogMessage('INFO',Origin=origin,Msg='Processing vAnt=%s (antIdx=%s), vIF=%s, vPOL=%s'%(vAnt,antIdx,vIF,vPOL))

        return (vAnt,vIF,vPOL,antIdx,polIdx)

    def __setRadiusFromBeamSize(self, akey, skey=-1):
        origin = '__setRadiusFromBeamSize'
        import re
        beamsizePerSpw = self.beamsize[akey]
        fileIdx = self.rawFileIdx[akey]
        spwList = self.dataSummary[fileIdx]['spw']
        for i in xrange(len(beamsizePerSpw)):
            if re.match('TARGET', spwList[i]['intent']) is not None \
                   and re.match('WVR', spwList[i]['type']) is None:
                self.setBeamRadius( beamsizePerSpw[i]/7200.0 )
                self.LogMessage('INFO',Origin=origin,Msg='set beam size as %s'%(beamsizePerSpw[i]))
                break
        
