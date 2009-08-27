"""Stages Interface for SFI Heuristics sfiReducer.py"""

# History:
#  12-Aug-2009 Hafok initial release based on vla_recipe.py, pdb_recipe.py

import os
import re
import shutil
import sys
import traceback
import types

import casac

import sfiReducer

class SFIPipelineStagesInterface:
    def __init__(self,rawDir='default',outDir='default'):

        
        self.rawDir      = None
        self.outDir      = None
        self.workingDir  = None
        self.rawFile     = None
        self.workingFile = None
        self.outFile     = None
        self.HtmlDir     = None
        self.LogDir      = None
        self.sfiReducer  = None
        self.maxPixels   = 1000
        self.Observatory = 'ALMA'
        self.mosaic      = False
        

        if (rawDir != 'default'):
            self.rawDir = rawDir
        if (outDir != 'default'):
            self.outDir = outDir
            self.workingDir = outDir

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

    def setMaxPixels(self,maxPixels):
        self.maxPixels=maxPixels
    
    def getMaxPixels(self):
        return self.maxPixels

    def setObservatoryVLA(self):
        self.maxPixels=1000
        self.observatory='VLA'

    def setObservatoryPdB(self):
        self.maxPixels=300
        self.observatory='PdB'

    def setObservatoryALMA(self):
        self.maxPixels=3000
        self.observatory='ALMA'

    def setMosaicMode(self,switch):
        if(switch==True):
            self.mosaic = True
        else:
            self.mosaic = False
            
        
    def getObservatory(self):
        return self.observatory

    def _logSFIReducer(logString):
        self.sfiReducer._html.logHTML('''</ul> %s <ul>''' % logString)       

    def setupSFIReducer(self):

        # setup input output directories and files
        
        if(self.outFile==None):
            self.outFile=self.rawFile

        if(self.rawDir==None):
            self.rawDir='./'
        
        if(self.outDir==None):
            self.outDir='./'

        if(self.rawDir!=self.outDir):
            os.system( 'cp -r '+self.rawDir+self.rawFile+' '+self.outDir+self.outFile)
            
        self.workingFile=self.outFile
        self.workingDir=self.outDir
        
        self.sfiReducer=sfiReducer.SFIReducer(self.workingDir+self.workingFile)
        print self.sfiReducer._msName

        # Find out the version of the logfile to be written (to handle multiple runs).
        # The suffix .html is added so that Mac Safari knows to treat the file as HTML.

        log_version = 0
        while os.path.exists('html/casapy.log.%s.text' % log_version):
            log_version += 1
        self.sfiReducer._log['logName'] = 'casapy.log.%s.html' % log_version

        # write the html tag <pre> to the log file - this stops a browser 
        # reformatting it.

        self.sfiReducer._log['logTool'].postLocally('<pre>')

        # set flagging state of MS to value at start stage - in case a reduction run has
        # already been done on the data

        self.sfiReducer._msFlagger.resetFlagsToStageEntry('beforeHeuristics','beforeHeuristics')

        # OK, ready to go
        # write measurement set summary info

        self.sfiReducer._html.timingStageStart('checkMS')
        checkMS.CheckMS(self.sfiReducer._tools, self.sfiReducer._tasks, self.sfiReducer._html, self.sfiReducer._msName).writeHtmlSummary()
        self.sfiReducer._html.timingStageStop()

    def flagBasicErrors(self):
        '''basic flagging'''

        self._logSFIReducer('Direct flagging; e.g. autocorrelations.')        
        
        stageDic={'name':'Basic flagging',
          'description':"""This stage flags data that are already known to be
                  bad or are inappropriate for use in further reduction.""",
          'stage':"""view=baseData.BaseData(),
                   operator=taqlFlagger.TaqlFlagger(rules=
                       [{'rule':'autocorrelation', 'colour':'crimson'}])"""
         }
        
        self.sfiReducer._doStage(stageDic,True)
        
    def flagBasicErrorsPdB(self):
        '''basic flagging'''

        self._logSFIReducer('Direct flagging; e.g. autocorrelations.')        
        
        stageDic={'name':'Basic flagging',
          'description':"""This stage flags data that are already known to be
                  bad or are inappropriate for use in further reduction.""",
          'stage':"""view=baseData.BaseData(),
                   operator=taqlFlagger.TaqlFlagger(rules=
                       [{'rule':'autocorrelation', 'colour':'crimson'},
                       {'rule':'PdB Gibbs channels', 'colour':'tomato'}])"""
         }
        
        self.sfiReducer._doStage(stageDic,True)

    def flagByRule(self,rule='autocorrelation',color='crimson'):
        '''basic flagging by rule (autocorrelation|PdB Gibbs channels) '''

        self._logSFIReducer('Direct flagging; e.g. autocorrelations.')        
        
        stageDic={'name':'Basic flagging',
          'description':"""This stage flags data that are already known to be
                  bad or are inappropriate for use in further reduction.""",
          'stage':"""view=baseData.BaseData(),
                   operator=taqlFlagger.TaqlFlagger(rules=
                       [{'rule':'%%RULE%%', 'colour':'%%COLOR%%'}])"""
         }
        
        stageDic['stage'].replace('%%RULE%%',rule)
        stageDic['stage'].replace('%%COLOR%%',color)
        
        self.sfiReducer._doStage(stageDic,True)
        
    def plotInitalBandpassCalCoeffs(self):
        '''
        Initial B calibration coefficients display
        '''

       self._logSFIReducer('Results from direct Bandpass calibration of data.')
       
       stageDic={'name':'Initial B calibration coefficients display',
          'description':"""The aim of this stage is to show the
                  bandpass calibration coefficients that
                  are derived in a straightforward way from the
                  MeasurementSet before it has undergone any significant
                  flagging.""",
          'stage':"""
                  view=bandpassCalibration.BandpassCalibration(),
                  display=complexSliceDisplay.SliceX()"""

         }

        self.sfiReducer._doStage(stageDic,True)
          
    def plotInitialFluxCalCoeff(self):
        '''Initial F calibration phase display
        '''

        self._logSFIReducer('Results from direct Flux  calibration of data.')
        
        stageDic= {'name':'Initial F calibration coefficients display',
          'description':"""The aim of this stage is to show the 
                  flux calibration coefficients that are derived in a
                  straightforward way from the MeasurementSet before
                  it has undergone any significant flagging.""",
          'stage':"""
                  view=fluxCalibration.FluxCalibration(
                       bandpassCal=[bandpassCalibration.BandpassCalibration]),
                  display=complexSliceDisplay.SliceY()"""
         }

        self.sfiReducer._doStage(stageDic,True)
    
 
    def plotMSCleanedImages(self,maxPixels=None,mode='mfs',sourceTypeList=['*GAIN*', '*SOURCE*', '*FLUX*']):
        '''Initial GAIN and TARGET clean maps for mosaic sourcelTypeList only *SOURCE*
        '''

        self._logSFIReducer('Results from direct GAIN and TARGET clean integrated maps.')
        
        stageDic= {'name':'Initial TARGET mosaic clean maps',
          'description':"""The aim of this stage is to show
                  cleaned images of the target mosaics
                  using a straightforward calibration method
                  before the data have undergone any significant flagging.
                  The maps show the emission integrated over the
                  spectral window""",
          'stage':"""
                  view=%%IMAGEMODE%%(
                     sourceType=%%SOURCE_LIST%%,
                     mode='%%MODE%%',
                     algorithm='%%ALGORITHM%%',
                     maxPixels = %%MAXPIXELS%%,
                     bandpassCal=[bandpassCalibration.BandpassCalibration],
                     bandpassCalDisplay=complexSliceDisplay.SliceX,
                     gainCal=fluxCalibration.FluxCalibration,
                     gainCalDisplay=complexSliceDisplay.SliceY,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2),
                  display=skyDisplay.SkyDisplay()"""
         }

        stageDic['stage'].replace('%%SOURCE_LIST%%',str(sourceTypeList))
        stageDic['stage'].replace('%%MODE%%',mode)

        if(self.mosaic==True):
            stageDic['stage'].replace('%%IMAGEMODE%%','mosaicCleanImage.MosaicCleanImage')
            stageDic['stage'].replace('%%ALGORITHM%%','mfhogbom')
        else:
            stageDic['stage'].replace('%%MAXPIXELS%%','cleanImageV2.CleanImageV2')
            stageDic['stage'].replace('%%ALGORITHM%%','hogbom')
            

        if(maxPixels==None):
            stageDic['stage'].replace('%%MAXPIXELS%%',str(self.maxPixels))
        else:
           stageDic['stage'].replace('%%MAXPIXELS%%',str(maxPixels))
             
        self.sfiReducer._doStage(stageDic,True)
        
    def flagCalibratorNoisyAmplitudes(self,flagRule='high outlier',limit=10.0,min_N=10,color='crimson'):
        '''Flag calibrator baselines with noisy amplitudes
        '''

        self._logSFIReducer('Flag bad antennas with noisy amplitudes in raw calibrater data.')

        stageDic= {'name':'Flag calibrator baselines with noisy amplitudes',
          'description':"""The aim of this stage is to flag baselines in
                  calibrater data that have noisy amplitudes.""",
          'stage':"""
                  view=buildImage.BuildImage(yAxis='ANTENNA1', 
                        view=medianAndMAD.MAD(
                        collapseAxis='TIME',
                        view=deviations.RawAmplitudeDeviationPerBaseline(
                        sourceType=['*BANDPASS*', '*GAIN*', '*FLUX*'])),
                        description='''This data view is a 2-d array with axes
                        x:ANTENNA2 and y:ANTENNA1. Each pixel is a measure
                        of the noise on the uncalibrated amplitudes for that
                        baseline.'''),
                  operator=imageFlagger.ImageFlagger(rules=[
                       {'rule':'%%FLAG_RULE%%', 'colour':'%%COLOR%%', 'limit':%%LIMIT%%,
                       'min_N':%%MIN_N%%}]), 
                  display=imageDisplay.ImageDisplay()"""
        }

        stageDic['stage'].replace('%%FLAG_RULE%%',flagRule)
        stageDic['stage'].replace('%%COLOR%%',color)        
        stageDic['stage'].replace('%%LIMIT%%',str(limit))
        stageDic['stage'].replace('%%MIN_N%%',str(min_N))
        
       self.sfiReducer._doStage(stageDic,True)
        
            
    def flagCalibratorNoisyPhases(self,flagRule='high outlier',limit=10.0,min_N=10,color='crimson'):
        '''Flag calibrator baselines with noisy phases
        '''

        self._logSFIReducer('Flag bad antennas with noisy phases in raw calibrater data.')

        stageDic= {'name':'Flag calibrator baselines with noisy phases',
          'description':"""The aim of this stage is to flag baselines in
                  calibrater data that have a lot of phase noise. In addition,
                  if a large fraction of the baselines from a given antenna 
                  have been flagged then all data for that antenna are
                  flagged.""",
          'stage':"""
                  view=buildImage.BuildImage(yAxis='ANTENNA1', 
                       view=medianAndMAD.MAD(
                       collapseAxis='TIME',
                       view=deviations.RawPhaseDeviationPerBaseline(
                       sourceType=['*BANDPASS*', '*GAIN*', '*FLUX*'])),
                        description='''This data view is a 2-d array with axes
                        x:ANTENNA2 and y:ANTENNA1. Each pixel is a measure
                        of the noise on the uncalibrated phases for that
                        baseline.'''),
                  operator=imageFlagger.ImageFlagger(rules=[
                       {'rule':'%%FLAG_RULE%%, 'colour':'%%COLOR%%', 'limit':%%LIMIT%%,
                       'min_N':%%MIN_N%%},
                       {'rule':'too many flags', 'axis':'ANTENNA2', 'limit':0.6,
                       'colour':'deeppink'}]),
                  display=imageDisplay.ImageDisplay()"""

        }
        stageDic['stage'].replace('%%FLAG_RULE%%',flagRule)
        stageDic['stage'].replace('%%COLOR%%',color)        
        stageDic['stage'].replace('%%LIMIT%%',str(limit))
        stageDic['stage'].replace('%%MIN_N%%',str(min_N))
        self.sfiReducer._doStage(stageDic,True)
         
    def flagCalibratorAntennaCoaddedAmplitudes(self,limit=10.0,min_N=10,color='tomato'):
        '''Flag antenna timestamps with bad amplitudes in calibrator data
        '''

        self._logSFIReducer('Flag antenna timestamps with bad calibrator data.')

        stageDic={'name':'''Flag antenna timestamps with bad amplitudes in calibrator data''',
          'description':"""The aim of this stage is to look at the
                  calibrater data for each antenna as a function of time
                  and to flag timestamps whose coadded amplitudes are
                  statistical outliers.
                  In addition, if a large fraction of the time series from a
                  given antenna have been flagged then all data for that
                  antenna are flagged. If a large fraction of the data from
                  all antennas at a given timestamp have been flagged then
                  all data for that timestamp are flagged.""",
          'stage':"""
                  view=coaddedAntennaAmplitude.\
                       CoaddedAntennaRawScalarAmplitude(
                       sourceType=['*BANDPASS*', '*GAIN*', '*FLUX*']),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'too many flags', 'axis':'TIME', 'limit':0.7,
                       'colour':'crimson'},
                       {'rule':'too many flags', 'axis':'ANTENNA', 'limit':0.7,
                       'colour':'deeppink'},
                       {'rule':'outlier', 'axis':'TIME', 'limit':%%LIMIT%%,
                       'min_N':%%MIN_N%%, 'colour':'%%COLOR%%'}]),
                  display=imageDisplay.ImageDisplay()"""
         }

        stageDic['stage'].replace('%%COLOR%%',color)        
        stageDic['stage'].replace('%%LIMIT%%',str(limit))
        stageDic['stage'].replace('%%MIN_N%%',str(min_N))

        self.sfiReducer._doStage(stageDic,True)
        
    def flagCalibratorAntennaSingleAmplitudes(self,bandpassCal=False,limit=10.0,min_N=10,color='tomato'):
        '''Flag antenna timestamps with bad amplitudes in calibrator data
        '''

        self._logSFIReducer('Flag antenna timestamps with bad calibrator data.')

        stageDic= {'name':'''Flag antenna timestamps with bad amplitudes in calibrator data''',
          'description':"""The aim of this stage is to look at the
                  calibrater data for each antenna as a function of time
                  and to flag timestamps whose amplitudes are statistical
                  outliers.
                  In addition, if a large fraction of the time series from a 
                  given antenna have been flagged then all data for that
                  antenna are flagged. If a large fraction of the data from 
                  all antennas at a given timestamp have been flagged then 
                  all data for that timestamp are flagged.""",
          'stage':"""
                  view=gainCalibration.GainCalibrationAmplitudePerTimestamp(
                       bandpassCal=%%BANDPASS_CAL%%,
                       sourceType=['*BANDPASS*', '*GAIN*', '*FLUX*']),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'too many flags', 'axis':'TIME', 'limit':0.7,
                       'colour':'crimson'},
                       {'rule':'too many flags', 'axis':'ANTENNA', 'limit':0.7,
                       'colour':'deeppink'},
                       {'rule':'outlier in chunk', 'axis':'TIME', 'limit':%%LIMIT%%,
                       'min_N':%%MIN_N%%, 'colour':'%%COLOR%%'}]),
                  display=imageDisplay.ImageDisplay()"""
         }

        if(bandpassCal==False):
            stageDic['stage'].replace('%%BANDPASS_CAL%%','None')
        else:
            stageDic['stage'].replace('%%BANDPASS_CAL%%','[bandpassCalibration.BandpassCalibration]')

        stageDic['stage'].replace('%%COLOR%%',color)        
        stageDic['stage'].replace('%%LIMIT%%',str(limit))
        stageDic['stage'].replace('%%MIN_N%%',str(min_N))
            
        self.sfiReducer._doStage(stageDic,True)
         
    def plotCalibratorPhaseBehaviour(self,bandpassCal=False):
        '''Display calibration phases for each antenna/timestamp in calibrator data
        '''

        self._logSFIReducer('plot phase behavior')
        
        stageDic= {'name':'Display calibration phases for each antenna/timestamp in calibrator data',
          'description':"""The aim of this stage is to display the 
                  phase behaviour of the calibrator data for each antenna as 
                  a function of time.""",
          'stage':"""
                  view=gainCalibration.GainCalibrationPhasePerTimestamp(
                       bandpassCal=%%BANDPASS_CAL%%,
                       sourceType=['*BANDPASS*', '*GAIN*', '*FLUX*']),
                  display=imageDisplay.ImageDisplay()"""
         }
        if(bandpassCal==False):
            stageDic['stage'].replace('%%BANDPASS_CAL%%','None')
        else:
            stageDic['stage'].replace('%%BANDPASS_CAL%%','[bandpassCalibration.BandpassCalibration]')

        self.sfiReducer._doStage(stageDic,True)
         
    def flagCalibratorBaselineAmplitudes(self,limit=10.0,min_N=10,color='crimson'):
        '''Flag baseline timestamps with deviant amplitudes in calibrator data
        '''

        self._logSFIReducer('Flag baseline timestamps with bad calibrator data.')
       
        stageDic= {'name':'Flag baseline timestamps with outlying amplitudes in calibrator data',
          'description':"""The aim of this stage is to look at the
                  calibrater data for each baseline as a function of time
                  and to flag timestamps whose amplitudes are statistical
                  outliers.""",
          'stage':"""
                  view=deviations.RawAmplitudeDeviationPerBaseline(
                       sourceType=['*BANDPASS*', '*GAIN*', '*FLUX*']),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'axis':'TIME', 'rule':'%%FLAG_RULE%%',
                       'colour':'%%COLOR%%',
                       'min_N':%%MIN_N%%, 'limit':%%LIMIT%%}]),
                  display=imageDisplay.ImageDisplay()"""
         }

        if(self.observatory=='PdB'):
            stageDic['stage'].replace('%%FLAG_RULE%%','outlier')
        else:
            stageDic['stage'].replace('%%FLAG_RULE%%','outlier in chunk')

        stageDic['stage'].replace('%%COLOR%%',color)        
        stageDic['stage'].replace('%%LIMIT%%',str(limit))
        stageDic['stage'].replace('%%MIN_N%%',str(min_N))
            
 
        self.sfiReducer._doStage(stageDic,True)
        
 
    def detectBandpassEdge(self,color='crimson'):
        '''Detect BANDPASS edge
        '''

        self._logSFIReducer('Detect bandpass edges.')
        
        stageDic= {'name':'Detect BANDPASS edge',
          'description':"""The aim of this stage is to detect the edges
                  of the bandpass in each spectral window. The identity
                  of the edge channels is stored so that they can
                  be flagged as needed in later stages but the data
                  are not flagged here.""",
          'stage':"""
                  view=medianAndMAD.MedianAndMAD(
                       collapseAxis='ANTENNA',
                       view=bandpassCalibration.BandpassCalibrationAmplitude(
                       bandpassFlaggingStage='Detect BANDPASS edge'),
                       description='''This data view for each spectral window
                       is a 1-d array [degenerate 2-d, with 1 axis having a 
                       single point] with x-axis:CHANNEL. The value in each
                       channel is the median of the bandpass calibration
                       amplitudes from all antennas.'''),
                  operator = bandpassEdgeFlagger.BandpassEdgeFlagger(rules=[
                       {'rule':'%%RULE%%', 'colour':'%%COLOR%%'}]),
                  display=sliceDisplay.SliceX()"""
         }
        if(self.observatory=='PdB'):
            stageDic['stage'].replace('%%RULE%%','PdB edge template')
        elif(self.observatory=='VLA'):
            stageDic['stage'].replace('%%RULE%%','VLA edge template')
        else:
            stageDic['stage'].replace('%%RULE%%','edge template')

        stageDic['stage'].replace('%%COLOR%%',color)        
            

        self.sfiReducer._doStage(stageDic,True)

    def plotBandpassCalibration(self):
        '''Display best bandpass calibration VLA
        '''
        self._logSFIReducer('Display quality of best bandpass calibration.')

        stageDic= {'name':"Display 'quality' of best bandpass calibration",
          'description':"""The aim of this stage is to show the quality
                  of the 'best' bandpass calibration method.
                  This is achieved by using the 'best' method to calculate
                  the bandpass calibration, then calibrating another source
                  with it. That calibrated result is shown here.""",
           'stage':"""
                  view=bestMethod.BestMethod(
                       viewClassList=[
                       bandpassCalibration.BandpassTestCalibratedAmplitude],
                       description=''' '''), 
                  display=imageDisplay.ImageDisplay()"""
#                  display=sliceDisplay.SliceX()"""
         }


        self.sfiReducer._doStage(stageDic,True)
         
    def findBandpassCalibration(self,testSourceTypeList=['*FLUX*'],G_t_List=[20.0,80.0,160.0,320.0,3600.0]):
        '''Find best bandpass calibration
        '''
        self._logSFIReducer('Find best bandpass calibration.')
        
        stageDic= {'name':'Find best bandpass calibration',
          'description':"""The aim of this stage is to find the best method
                  for calculating the bandpass calibration in this data set.
                  This is achieved by using a range of methods to calculate
                  the bandpass calibration, then calibrating another source by
                  each result, lastly measuring the 'flatness' of that
                  spectrum. The best method is that giving the flattest 
                  spectrum.""",
          'stage':"""
                  view=bestMethodSelector.BestMethodSelector(
                       view=rangeTrial.RangeTrial(parameterRange=
                       {'G_t':%%GT_LIST%%,
                       'method':[{'mode':'CHANNEL'}],
                       'source_type':['*BANDPASS*'],
                       'bandpass_flagging_stage':['Detect BANDPASS edge'],
                       'test_source_type':%%TEST_SOURCE_TYPE_LIST%%},
                       viewClassList=[bandpassMerit.BandpassMerit,
                       bandpassCalibration.BandpassTestCalibratedAmplitude]),
                       description='''The data view is a list of 2-d arrays,
                       one for each spectral window, with axes 
                       ANTENNA1 and ANTENNA2. Each
                       pixel shows the 'merit' of the bandpass calibration
                       test for that baseline. Numbers of order 1 or less
                       are good.'''), 
                  display=imageDisplay.ImageDisplay()"""
         }
        stageDic['stage'].replace('%%GT_LIST%%',str(G_t_List))
        stageDic['stage'].replace('%%TEST_SOURCE_TYPE_LIST%%',str(testSourceTypeList))
        self.sfiReducer._doStage(stageDic,True)


    def flagBandpassCalibrationVLA(self,limit=10.0,min_N=10,color='crimson'):
        '''Flag best bandpass calibration
        '''

        self._logSFIReducer('Flag best bandpass calibration.')
        
        stageDic= {'name':'Flag best bandpass calibration',
          'description':"""The aim of this stage is to flag baselines
                  for which the 'best' bandpass calibration method
                  does not produce very good results. 
                  This is achieved by using the 'best' method to calculate
                  the bandpass calibration, then calibrating another source 
                  with it, lastly measuring the 'flatness' of that
                  spectrum to produce a 'figure of merit' for each baseline.
                  Baselines with anomalous, high figures of merit are
                  flagged.""",
          'stage':"""
                  view=bestMethod.BestMethod(
                       viewClassList=[bandpassMerit.BandpassMerit,
                       bandpassCalibration.BandpassTestCalibratedAmplitude],
                       description='''The data view is a list of 2-d arrays,
                       one for each spectral window, with axes 
                       ANTENNA1 and ANTENNA2. Each
                       pixel shows the 'merit' of the bandpass calibration
                       test for that baseline. Numbers of order 1 or less
                       are good.'''), 
                  operator=imageFlagger.ImageFlagger(rules=[
                       {'rule':'outlier', 'colour':'%%COLOR%%', 'limit':%%LIMIT%%,
                       'min_N':%%MIN_N%%},
#                       {'rule':'max abs', 'colour':'deeppink', 'limit':5.0},
                       {'rule':'too many flags', 'axis':'ANTENNA2', 'limit':0.7,
                       'colour':'brown'}],
                       flag_targets=['*BANDPASS*','*GAIN*','*FLUX*','*SOURCE*']),   
                  display=imageDisplay.ImageDisplay()"""
        }

        stageDic['stage'].replace('%%COLOR%%',color)        
        stageDic['stage'].replace('%%LIMIT%%',str(limit))
        stageDic['stage'].replace('%%MIN_N%%',str(min_N))
 
        self.sfiReducer._doStage(stageDic,True)

    def flagBandpassCalibrationPdB(self,limit=10.0,min_N=10,color='crimson'):
        '''Flag best bandpass calibration
        '''

        self._logSFIReducer('Flag best bandpass calibration.')
        
        stageDic= {'name':'Flag best bandpass calibration',
          'description':"""The aim of this stage is to flag baselines
                  for which the 'best' bandpass calibration method
                  does not produce very good results. 
                  This is achieved by using the 'best' method to calculate
                  the bandpass calibration, then calibrating another source 
                  with it, lastly measuring the 'flatness' of that
                  spectrum to produce a 'figure of merit' for each baseline.
                  Baselines with anomalous, high figures of merit are
                  flagged.""",
          'stage':"""
                  view=bestMethod.BestMethod(
                       viewClassList=[bandpassMerit.BandpassMerit,
                       bandpassCalibration.BandpassTestCalibratedAmplitude],
                       description='''The data view is a list of 2-d arrays,
                       one for each spectral window, with axes 
                       ANTENNA1 and ANTENNA2. Each
                       pixel shows the 'merit' of the bandpass calibration
                       test for that baseline. Numbers of order 1 or less
                       are good.'''), 
                  operator=imageFlagger.ImageFlagger(rules=[
                      {'rule':'outlier', 'colour':'%%COLOR%%', 'limit':%%LIMIT%%,
                       'min_N':%%MIN_N%%},
                       flag_targets=['*BANDPASS*','*GAIN*','*FLUX*','*SOURCE*']),   
                  display=imageDisplay.ImageDisplay()"""
        }

        stageDic['stage'].replace('%%COLOR%%',color)        
        stageDic['stage'].replace('%%LIMIT%%',str(limit))
        stageDic['stage'].replace('%%MIN_N%%',str(min_N))
 
        self.sfiReducer._doStage(stageDic,True)
        
    def flagLowSNRCalibration(self,limitMinAbs=10.0):
        '''Antenna flagging on G calibration SNR
        '''
        
        self._logSFIReducer('Flag antenna G calibrations with bad SNR.')
        
        stageDic= {'name':'Flag antenna G calibrations with bad SNR',
          'description':"""The aim of this stage is to flag G
                  calibrations with unusually low signal to noise ratio,
                  or with SNR below a hard limit.
                  Antennas or timestamps with a high proportion of G calibrations
                  flagged are then completely flagged.""",
          'stage':"""
                  view=gainCalibration.GainCalibrationSNR(
                       bandpassCal=[bestMethod.BestMethod,
                       bandpassCalibration.BandpassCalibration],
                       sourceType='*GAIN*',
                       bandpassFlaggingStage='Detect BANDPASS edge'),
                  operator=imageFlagger.ImageFlagger(rules=[
                       {'rule':'too many flags', 'axis':'TIME', 'limit':0.7,
                       'colour':'crimson'},
                       {'rule':'too many flags', 'axis':'ANTENNA', 'limit':0.7,
                       'colour':'deeppink'},
                       {'rule':'min abs', 'limit':%%LIMIT_MIN_ABS%%, 'colour':'brown'},
                       {'rule':'low outlier', 'limit':10.0, 'min_N':10,
                       'colour':'tomato'}]),
                  display=imageDisplay.ImageDisplay()"""
        }
        stageDic['stage'].replace('%%LIMIT_MIN_ABS%%',str(limitMinAbs))

        self.sfiReducer._doStage(stageDic,True)

         
    def flagPhaseJumpCalibration(self,flagRule='high outlier'):
        '''Antenna flagging on G calibration median phase jump
        '''

        self._logSFIReducer('Flag antenna G calibrations with phase jumps.')
 
        stageDic= {'name':'Flag antennas with high G calibration median phase jump',
          'description':"""The aim of this stage is to flag antennas
                  for which the median absolute phase jump between G 
                  calibrations is unusually high.""",
          'stage':"""
                  view=medianJump.MedianJump(
                       view=gainCalibration.GainCalibrationPhase(
                       bandpassCal=[bestMethod.BestMethod,
                       bandpassCalibration.BandpassCalibration],
                       bandpassFlaggingStage='Detect BANDPASS edge'),
                       description='''The data view is a list of 1-d arrays,
                       one for each spectral window/polarization, with x-axis 
                       ANTENNA. The datum for each antenna is the median
                       absolute jump in phase between G calibrations.'''), 
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'axis':'ANTENNA', 'rule':'%%FLAG_RULE%%', 'colour':'crimson',
                       'limit':10.0, 'min_N':10}],
                       flag_targets=['*BANDPASS*', '*GAIN*', '*FLUX*', '*SOURCE*']),
                  display=sliceDisplay.SliceY()"""
         }

        stageDic['stage'].replace('%%FLAG_RULE%%',flagRule)
        self.sfiReducer._doStage(stageDic,True)
        

    def flagTargetDataAntennaSingleAmplitudes(self):
        '''Antenna flagging of bad timestamps in calibrated TARGET amplitudes
        '''
        self._logSFIReducer('Flag Target data with bad amplitudes.')

        stageDic= {'name':'Flag antenna timestamps with bad amplitudes in TARGET data',
          'description':"""The aim of this stage is to look at the
                  target data for each antenna as a function of time
                  and to flag timestamps whose amplitudes are statistical
                  outliers.
                  In addition, if a large fraction of the time series from a 
                  given antenna have been flagged then all data for that
                  antenna are flagged. If a large fraction of the data from 
                  all antennas at a given timestamp have been flagged then 
                  all data for that timestamp are flagged.""",
          'stage':"""
                  view=gainCalibration.GainCalibrationAmplitudePerTimestamp(
                       bandpassCal=[bestMethod.BestMethod,
                       bandpassCalibration.BandpassCalibration],
                       sourceType='*SOURCE*',
                       bandpassFlaggingStage='Detect BANDPASS edge'),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'too many flags', 'axis':'TIME', 'limit':0.7,
                       'colour':'crimson'},
                       {'rule':'too many flags', 'axis':'ANTENNA', 'limit':0.7,
                       'colour':'deeppink'},
                       {'rule':'outlier in chunk', 'axis':'TIME', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'}]),
                  display=imageDisplay.ImageDisplay()"""
        }

        self.sfiReducer._doStage(stageDic,True)

    def flagTargetDataAntennaCoaddedAmplitudes(self):
        '''Antenna flagging of bad timestamps in calibrated TARGET amplitudes
        '''
        self._logSFIReducer('Flag Target data with bad amplitudes.')

        stageDic=         {'name':'Antenna flagging of bad timestamps in calibrated TARGET amplitudes',
          'description':"""The aim of this stage is to look at the
                  calibrated target data for each antenna as a function of time
                  and to flag timestamps whose amplitudes are statistical
                  outliers.
                  In addition, if a large fraction of the time series from a
                  given antenna have been flagged then all data for that
                  antenna are flagged. If a large fraction of the data from
                  all antennas at a given timestamp have been flagged then
                  all data for that timestamp are flagged.""",
 
          'stage':"""
                  view=coaddedAntennaAmplitude.\
                       CoaddedAntennaCorrectedScalarAmplitude(
                       sourceType='*SOURCE*',
                       bandpassCal=[bestMethod.BestMethod,
                       bandpassCalibration.BandpassCalibration],
                       gainCal=fluxCalibration.FluxCalibration,
                       bandpassFlaggingStage='Detect BANDPASS edge'),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'too many flags', 'axis':'TIME', 'limit':0.7,
                       'colour':'crimson'},
                       {'rule':'too many flags', 'axis':'ANTENNA', 'limit':0.7,
                       'colour':'deeppink'},
                       {'rule':'outlier', 'axis':'TIME', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'}]),
                  display=imageDisplay.ImageDisplay()"""

        }
        
        self.sfiReducer._doStage(stageDic,True)

    def flagLargeGainClosureErrors(self):

        '''Flag GAIN closure magnitude
        '''

        self._logSFIReducer('Flag data with unusually high  GAIN closure errors.')

        stageDic= {'name':'Flag GAIN closure magnitude',
          'description':"""The aim of this stage is to flag 
                  data with unusually high closure errors.""",
          'stage':"""
                  view=buildImage.BuildImage(
                      description='''This data view is a 2-d array. 
                      Each pixel is the median of the closure error
                      of the clean image [closure error = abs{{corrected data / model data}
                      - {1 + 0i}}] for that baseline. For multi-channel spectral
                      windows the visibilities were averaged over the channels 
                      first.''',
                      yAxis='ANTENNA1', 
                      view=medianAndMAD.MedianAndMAD(
                      collapseAxis='TIME',
                      view=chunkMedian.ChunkMedian(
                      description='''This data view is a 2-d array.
                      Each pixel is the median over time of the closure
                      error magnitude [distance from 1 +0i] of the
                      ratio between source model and data''',
                      view=closureError.ClosureErrorMagnitude(
                      view=cleanImageV2.CleanImageV2(
                      sourceType='*GAIN*',
                      mode=%%MODE%%,
                      algorithm='hogbom',
                      maxPixels=%%MAXPIXELS%%,
                      bandpassCal=[bestMethod.BestMethod,
                      bandpassCalibration.BandpassCalibration],
                      gainCal=fluxCalibration.FluxCalibration,
                      psf=psf.Psf,
                      dirtyImage=dirtyImageV2.DirtyImageV2,
                      bandpassFlaggingStage='Detect BANDPASS edge'))))),
                  operator=imageFlagger.ImageFlagger(rules=[
                       {'rule':'high outlier', 'colour':'crimson', 'limit':10.0,
                       'min_N':10}
                       ]),
                  display=imageDisplay.ImageDisplay()"""
        }

        stageDic['stage'].replace('%%MAXPIXELS%%',str(self.maxPixels))
        if(self.observatory=='PdB'):
            stageDic['stage'].replace('%%MODE%%','channel')
        else:
            stageDic['stage'].replace('%%MODE%%','mfs')


        self.sfiReducer._doStage(stageDic,True)


   def flagLargeTargetClosureMagnitude(self):
        '''Flag TARGET closure magnitude
        '''

        self._logSFIReducer('Flag data with unusually high  Target closure errors.')

        stageDic= {'name':'Flag TARGET closure magnitude',
          'description':"""The aim of this stage is to flag
                  TARGET data with unusually high closure errors.""",
          'stage':"""
                  view=buildImage.BuildImage(
                      description='''This data view is a 2-d array.
                      Each pixel is the median of the closure error
                      of the clean image [closure error = abs{{corrected data / $
                      - {1 + 0i}}] for that baseline. For multi-channel spectral
                      windows the visibilities were averaged over the channels
                      first.''',
                      yAxis='ANTENNA1',
                      view=medianAndMAD.MedianAndMAD(
                      collapseAxis='TIME',
                      view=chunkMedian.ChunkMedian(
                      description='''This data view is a 2-d array.
                      Each pixel is the median over time of the closure
                      error magnitude [distance from 1 +0i] of the
                      ratio between source model and data''',
                      view=closureError.ClosureErrorMagnitude(
                      view=%%IMAGEMODE%%(
                      sourceType='*SOURCE*',
                      mode='%%MODE%%',
                      algorithm='%%ALGORITHM%%',
                      maxPixels = %%MAXPIXELS%%,
                      bandpassCal=[bestMethod.BestMethod,
                      bandpassCalibration.BandpassCalibration],
                      bandpassCalDisplay=complexSliceDisplay.SliceX,
                      gainCal=fluxCalibration.FluxCalibration,
                      gainCalDisplay=complexSliceDisplay.SliceY,
                      psf=psf.Psf,
                      dirtyImage=dirtyImageV2.DirtyImageV2,
                      bandpassFlaggingStage='Detect BANDPASS edge'))))),
                  operator=imageFlagger.ImageFlagger(rules=[
                       {'rule':'high outlier', 'colour':'crimson', 'limit':10.0,
                       'min_N':10}
                       ]),
                  display=imageDisplay.ImageDisplay()"""
         }
        
        # only for mosaic ????? bandpassCalDisplay=complexSliceDisplay.SliceX
        
        if(self.mosaic==True):
            stageDic['stage'].replace('%%IMAGEMODE%%','mosaicCleanImage.MosaicCleanImage')
            stageDic['stage'].replace('%%ALGORITHM%%','mfhogbom')
        else:
            stageDic['stage'].replace('%%IMAGEMODE%%','cleanImageV2.CleanImageV2')
            stageDic['stage'].replace('%%ALGORITHM%%','hogbom')

        stageDic['stage'].replace('%%MAXPIXELS%%',str(self.maxPixels))

        if(self.observatory=='PdB'):
            stageDic['stage'].replace('%%MODE%%','channel')
        else:
            stageDic['stage'].replace('%%MODE%%','mfs')

        self.sfiReducer._doStage(stageDic,True)

    def plotBandpassCalibrationCoeffs(self):
        '''show the bandpass calibration coefficients
        '''
        self._logSFIReducer('Display Final Bandpass calibration coefficients.')

        stageDic= {'name':'Final B calibration coefficients display',
          'description':"""The aim of this stage is to show the
                  bandpass calibration coefficients that
                  are derived in a straightforward way from the
                  MeasurementSet before it has undergone any significant
                  flagging.""",
          'stage':"""
                  view=bestMethod.BestMethod(viewClassList=[
                  bandpassCalibration.BandpassCalibration]),
                  display=complexSliceDisplay.SliceX()"""
        }


        self.sfiReducer._doStage(stageDic,True)


    def plotFluxCalibrationCoeffs(self):
        '''show the flux calibration coefficients
        '''

        self._logSFIReducer('Display final flux calibration coefficients.')

        stageDic= {'name':'F calibration coefficient display',
          'description':"""The aim of this stage is to show the
                  flux calibration coefficients as calculated
                  after flagging of the calibrators has been completed.
                  This is the calibration that will be used in the
                  calculation of the final images.""",
          'stage':"""
                  view=fluxCalibration.FluxCalibration(
                       bandpassCal=[bestMethod.BestMethod,
                       bandpassCalibration.BandpassCalibration],
                       bandpassFlaggingStage='Detect BANDPASS edge'),
                  display=complexSliceDisplay.SliceY()"""
        }


        self.sfiReducer._doStage(stageDic,True)

        
    def plotVisibilities(self):
        '''calibrated GAIN and TARGET complex data
        '''

        self._logSFIReducer('Display of final flagged and calibrated complex data')

        stageDic= {'name':'calibrated GAIN complex data',
          'description':"""The aim of this stage is to display the calibrated
                  visibilities of the GAIN calibrator as
                  points on the complex plane. For multi-channel spectral
                  windows the visibilities are averaged over the channels
                  before display.""",
          'stage':"""
                  view=baselineData.BaselineCorrectedComplexData(
                   sourceType=['*GAIN*'],
                   bandpassCal=[bestMethod.BestMethod,
                   bandpassCalibration.BandpassCalibration],
                   gainCal=fluxCalibration.FluxCalibration,
                   bandpassFlaggingStage='Detect BANDPASS edge'),
                  display=complexDisplay.ComplexDisplay()"""
        }

        self.sfiReducer._doStage(stageDic,True)
         
    def plotVisibilityAmplitudes(self):
        '''calibrated GAIN and TARGET amplitude display
        '''

        self._logSFIReducer('Display of final flagged and calibrated complex data')

        stageDic= {'name':'calibrated GAIN and TARGET amplitude display',
          'description':"""The aim of this stage is to display the 
                  calibrated visibility amplitudes of the GAIN calibrater 
                  and target source, coadded for each antenna at each
                  timestamp. This will give some idea of the extent of
                  flagging and of the calibration quality.""",
          'stage':"""
                  view=coaddedAntennaAmplitude.CoaddedAntennaCorrectedVectorAmplitude(
                   sourceType=['*GAIN*', '*SOURCE*'],
                   bandpassCal=[bestMethod.BestMethod,
                   bandpassCalibration.BandpassCalibration],
                   gainCal=fluxCalibration.FluxCalibration,
                   bandpassFlaggingStage='Detect BANDPASS edge'),
                  display=imageDisplay.ImageDisplay()"""
        }

        self.sfiReducer._doStage(stageDic,True)
        

    def plotCalibratorCleanedCube(self):
        '''Calibrator clean cubes
        '''

        self._logSFIReducer('Cleaned calibrator images and spectra.')

        stageDic= {'name':'Calibrator clean cubes',
          'description':"""The aim of this stage is to show 
                  the cleaned cubes of the calibrator sources, calculated
                  from the flagged data using the 'best' calibration
                  methods.""",
          'stage':"""
                  view=cleanImageV2.CleanImageV2(
                     sourceType=['*GAIN*','*FLUX*','*BANDPASS*'],
                     mode='channel',
                     algorithm='hogbom',
                     maxPixels = %%MAXPIXELS%%,
                     bandpassCal=[bestMethod.BestMethod,
                     bandpassCalibration.BandpassCalibration],
                     bandpassCalDisplay=complexSliceDisplay.SliceX,
                     gainCal=fluxCalibration.FluxCalibration,
                     gainCalDisplay=complexSliceDisplay.SliceY,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2,
                     bandpassFlaggingStage='Detect BANDPASS edge'),
                  display=skyDisplay.SkyDisplay()"""
        }

        stageDic['stage'].replace('%%MAXPIXELS%%',str(self.maxPixels))
 
        self.sfiReducer._doStage(stageDic,True)
        
    def plotCalibratorsSpectra(self):
        '''Calibrator spectra
        '''
        self._logSFIReducer('Cleaned calibrator images and spectra.')
        
        stageDic= {'name':'Calibrator spectra',
          'description':"""The aim of this stage is to show
                  the spectra from sources detected during the cleaning
                  of the calibrators.""",
          'stage':"""
                  view=sourceSpectra.SourceSpectra(
                     view=cleanImageV2.CleanImageV2(
                     sourceType=['*GAIN*','*FLUX*','*BANDPASS*'],
                     mode='channel',
                     algorithm='hogbom',
                     maxPixels = %%MAXPIXELS%%,
                     bandpassCal=[bestMethod.BestMethod,
                     bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2,
                     bandpassFlaggingStage='Detect BANDPASS edge')),
                  display=sliceDisplay.SliceX()"""
        }

        stageDic['stage'].replace('%%MAXPIXELS%%',str(self.maxPixels))
 
        self.sfiReducer._doStage(stageDic,True)
        
    def plotTargetCleanedCube(self):
        '''TARGET clean map cube
        '''

        self._logSFIReducer('Cleaned TARGET images and spectra.')

        stageDic= {'name':'TARGET clean cube',
          'description':"""The aim of this stage is to show 
                  the final cleaned cube of the target source, calculated
                  from the flagged data using the 'best' calibration
                  methods.""",
          'stage':"""
                  view=cleanImageV2.CleanImageV2(
                     sourceType='*SOURCE*',
                     mode='channel',
                     algorithm='hogbom',
                     maxPixels = %%MAXPIXELS%%,
                     bandpassCal=[bestMethod.BestMethod,
                     bandpassCalibration.BandpassCalibration],
                     bandpassCalDisplay=complexSliceDisplay.SliceX,
                     gainCal=fluxCalibration.FluxCalibration,
                     gainCalDisplay=complexSliceDisplay.SliceY,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2,
                     bandpassFlaggingStage='Detect BANDPASS edge'),
                  display=skyDisplay.SkyDisplay()"""
        }

        stageDic['stage'].replace('%%MAXPIXELS%%',str(self.maxPixels))
         
        self.sfiReducer._doStage(stageDic,True)


    def plotTargetCleanMap(self):
        '''Display TARGET clean map in line and continuum
        '''

        self._logSFIReducer('Cleaned TARGET images and spectra.')

        stageDic= {'name':'TARGET clean map in line and continuum',
          'description':"""The aim of this stage is to produce separate
                  clean maps of the line emission and the continuum emission
                  from the target source, calculated
                  from the flagged data using the 'best' calbration
                  methods.""",
          'stage':"""
                  view=continuumSubtractedCleanImage.ContinuumSubtractedCleanImage(
                     sourceType='*SOURCE*',
                     mode='channel',
                     algorithm='hogbom',
                     maxPixels = %%MAXPIXELS%%,
                     bandpassCal=[bestMethod.BestMethod,
                     bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2,
                     bandpassFlaggingStage='Detect BANDPASS edge'),
                  display=skyDisplay.SkyDisplay()"""
        }

        stageDic['stage'].replace('%%MAXPIXELS%%',str(self.maxPixels))

        self.sfiReducer._doStage(stageDic,True)


    def plotTargetsSpectra(self):
        '''spectra from TARGET sources
        '''

        self._logSFIReducer('Cleaned TARGET images and spectra.')

        stageDic= {'name':'Spectra from TARGET sources',
          'description':"""The aim of this stage is to show
                  the spectra from sources detected during the cleaning
                  of the target field.""",
          'stage':"""
                  view=sourceSpectra.SourceSpectra(
                     view=%%IMAGEMODE%%(
                     sourceType='*SOURCE*',
                     mode='channel',
                     algorithm='%%ALGORITHM%%',
                     maxPixels = %%MAXPIXELS%%,
                     bandpassCal=[bestMethod.BestMethod,
                     bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2,
                     bandpassFlaggingStage='Detect BANDPASS edge')),
                  display=sliceDisplay.SliceX()"""

        }

        stageDic['stage'].replace('%%MAXPIXELS%%',str(self.maxPixels))

        if(self.mosaic==True):
            stageDic['stage'].replace('%%IMAGEMODE%%','mosaicCleanImage.MosaicCleanImage')
            stageDic['stage'].replace('%%ALGORITHM%%','mfhogbom')
        else:
            stageDic['stage'].replace('%%IMAGEMODE%%','cleanImageV2.CleanImageV2')
            stageDic['stage'].replace('%%ALGORITHM%%','hogbom')

        self.sfiReducer._doStage(stageDic,True)
