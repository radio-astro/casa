"""Pipeline Heuristics Reduction Script for SFI of MERLIN Data"""

# History:
#  2-Jun-2008 jfl First version.
#  3-Nov-2008 jfl amalgamated stage release.

recipe = [ 
         {'comment':'''Direct flagging; e.g. autocorrelations.'''},

         {'name':'basic flagging',
          'stage':"""view=baseData.BaseData(),
                   operator=taqlFlagger.TaqlFlagger(rules=
                       [{'rule':'autocorrelation', 'colour':'crimson'}])"""
         },

         {'comment':'Results from direct reduction of data.'},
          
         {'name':'initial GAIN and TARGET clean maps',
          'stage':"""
                  view=cleanImageV2.CleanImageV2(
                     sourceType=['GAIN', 'SOURCE'],
                     mode='mfs',
                     algorithm='hogbom',
                     maxPixels=500,
                     bandpassCal=[bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2),
                  display=skyDisplay.SkyDisplay()"""
         },

         {'comment':'Sanity check of raw calibrater data.'},

         {'name':'BANDPASS, GAIN and FLUX baseline amplitude MAD flagging',
          'stage':"""
                  view=buildImage.BuildImage(yAxis='ANTENNA1', 
                       view=medianAndMAD.MAD(
                       collapseAxis='TIME',
                       view=deviations.RawAmplitudeDeviationPerBaseline(
                       sourceType=['BANDPASS', 'GAIN', 'FLUX']))),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'outlier', 'axis':'ANTENNA2', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'},
                       {'rule':'too many flags', 'axis':'ANTENNA2', 'limit':0.6,
                       'colour':'crimson'}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'BANDPASS, GAIN and FLUX baseline phase MAD flagging',
          'stage':"""
                  view=buildImage.BuildImage(yAxis='ANTENNA1', 
                       view=medianAndMAD.MAD(
                       collapseAxis='TIME',
                       view=deviations.RawPhaseDeviationPerBaseline(
                       sourceType=['BANDPASS', 'GAIN', 'FLUX']))),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'outlier', 'axis':'ANTENNA2', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'},
                       {'rule':'too many flags', 'axis':'ANTENNA2', 'limit':0.6,
                       'colour':'crimson'}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'comment':'Antenna based flagging of bad calibrator timestamps.'},

         {'name':'BANDPASS, GAIN and FLUX antenna amplitude flagging',
          'stage':"""
                  view=gainCalibration.GainCalibrationAmplitudePerTimestamp(
                       bandpassCal=[bandpassCalibration.BandpassCalibration],
                       sourceType=['BANDPASS', 'GAIN', 'FLUX']),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'too many flags', 'axis':'TIME', 'limit':0.7,
                       'colour':'crimson'},
                       {'rule':'too many flags', 'axis':'ANTENNA', 'limit':0.7,
                       'colour':'deeppink'},
                       {'rule':'outlier', 'axis':'TIME', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'BANDPASS, GAIN and FLUX  antenna phase display',
          'stage':"""
                  view=gainCalibration.GainCalibrationPhasePerTimestamp(
                       bandpassCal=[bandpassCalibration.BandpassCalibration],
                       sourceType=['BANDPASS', 'GAIN', 'FLUX']),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'comment':'Baseline based flagging of bad calibrator timestamps.'},

         {'name':'BANDPASS, GAIN and FLUX  baseline amplitude deviation flagging',
          'stage':"""
                  view=deviations.RawAmplitudeDeviationPerBaseline(
                       sourceType=['BANDPASS', 'GAIN', 'FLUX']),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'axis':'TIME', 'rule':'outlier', 'colour':'crimson',
                       'min_N':10, 'limit':10.0}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'comment':'Find and flag bandpass edges.'},
 
         {'name':'Detect BANDPASS edge',
          'stage':"""
                  view=medianAndMAD.MedianAndMAD(
                       collapseAxis='ANTENNA',
                       view=bandpassCalibration.BandpassCalibrationAmplitude()),
                  operator = bandpassEdgeFlagger.BandpassEdgeFlagger(rules=[
                       {'rule':'VLA edge template', 'colour':'crimson'}]),
                  display=sliceDisplay.SliceX()"""
         },

         {'comment':'Find the best bandpass calibration.'},

         {'name':'heuristic bandpass calibration selection',
          'stage':"""
                  view=bestMethodSelector.BestMethodSelector(
                       view=rangeTrial.RangeTrial(parameterRange=
#                       {'G_INTEGRATION_TIME':[20, 40, 160, 640, 2560],
#                       'METHOD':[{'mode':'CHANNEL'},
#                       {'mode':'POLYNOMIAL', 'ampdeg':2, 'phasedeg':2},
#                       {'mode':'POLYNOMIAL', 'ampdeg':4, 'phasedeg':4}],
#                       'SOURCE_TYPE':['BANDPASS','FLUX'],
#                       'BANDPASS_FLAGGING_STAGE':['flag BANDPASS edge']},
                       {'G_t':[0.0],
                       'method':[{'mode':'CHANNEL'}],
                       'source_type':['BANDPASS'],
                       'bandpass_flagging_stage':['Detect BANDPASS edge'],
                       'test_source_type':['FLUX']},
                       viewClassList=[bandpassMerit.BandpassMerit,
                       bandpassCalibration.BandpassTestCalibratedAmplitude]
                       )), 
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'Flag best bandpass calibration',
          'stage':"""
                  view=bestMethod.BestMethod(
                       viewClassList=[bandpassMerit.BandpassMerit,
                       bandpassCalibration.BandpassTestCalibratedAmplitude]),
                  operator=sequenceFlagger.SequenceFlagger(rules=[   
                       {'axis':'ANTENNA1', 'rule':'outlier', 'colour':'crimson',
                       'limit':10.0, 'min_N':10}],
                       flag_targets=['BANDPASS', 'GAIN', 'FLUX', 'SOURCE']),   
                  display=imageDisplay.ImageDisplay()"""
         },

         {'comment':'Flag GAIN calibrator on calibration results.'},

         {'name':'GAIN antenna SNR flagging',
          'stage':"""
                  view=gainCalibration.GainCalibrationSNR(
                       bandpassCal=[bestMethod.BestMethod,
                       bandpassCalibration.BandpassCalibration],
                       sourceType='GAIN'),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'too many flags', 'axis':'TIME', 'limit':0.7,
                       'colour':'crimson'},
                       {'rule':'too many flags', 'axis':'ANTENNA', 'limit':0.7,
                       'colour':'deeppink'},
                       {'rule':'outlier', 'axis':'TIME', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'G calibration median phase jump flagging',
          'stage':"""
                  view=medianJump.MedianJump(
                       view=gainCalibration.GainCalibrationPhase(
                       bandpassCal=[bestMethod.BestMethod,
                       bandpassCalibration.BandpassCalibration])),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'axis':'ANTENNA', 'rule':'outlier', 'colour':'crimson',
                       'limit':10.0, 'min_N':10}],
                       flag_targets=['BANDPASS', 'GAIN', 'FLUX', 'SOURCE']),
                  display=sliceDisplay.SliceY()"""
         },

         {'comment':'Display flux calibrated antenna gains.'},

         {'name':'F calibration amplitude display',
          'stage':"""
                  view=fluxCalibration.FluxCalibrationAmplitude(
                       bandpassCal=[bestMethod.BestMethod,
                       bandpassCalibration.BandpassCalibration]),
                  display=sliceDisplay.SliceY()"""
         },

         {'name':'F calibration phase display',
          'stage':"""
                  view=fluxCalibration.FluxCalibrationPhase(
                       bandpassCal=[bestMethod.BestMethod,
                       bandpassCalibration.BandpassCalibration]),
                  display=sliceDisplay.SliceY()"""
         },

         {'comment':'Flag bad TARGET data.'},

         {'name':'Flag bandpass edges of TARGET and calibrators',
          'stage':"""view=baseData.BaseData(),
                   operator=taqlFlagger.TaqlFlagger(rules=[
                       {'rule':'flag channels',
                       'stageName':'flag BANDPASS edge',
                       'colour':'crimson'}],
                       flag_targets=['SOURCE','BANDPASS','GAIN','FLUX'])"""
         },

         {'name':'TARGET antenna amplitude flagging',
          'stage':"""
                  view=gainCalibration.GainCalibrationAmplitudePerTimestamp(
                       bandpassCal=[bestMethod.BestMethod,
                       bandpassCalibration.BandpassCalibration],
                       sourceType='SOURCE'),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'too many flags', 'axis':'TIME', 'limit':0.7,
                       'colour':'crimson'},
                       {'rule':'too many flags', 'axis':'ANTENNA', 'limit':0.7,
                       'colour':'deeppink'},
                       {'rule':'outlier', 'axis':'TIME', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'comment':'Cleaned GAIN and TARGET images and spectra.'},
                   
         {'name':'GAIN calibrater clean cube',
          'stage':"""
                  view=cleanImageV2.CleanImageV2(
                     sourceType='GAIN',
                     mode='channel',
                     algorithm='hogbom',
                     maxPixels = 500,
                     bandpassCal=[bestMethod.BestMethod,
                     bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2),
                  display=skyDisplay.SkyDisplay()"""
         },

         {'name':'Display GAIN closure phase.',
          'stage':"""
                  view=chunkMedian.ChunkMedian(
                       view=closurePhase.ClosurePhase(
                       view=cleanImageV2.CleanImageV2(
                       sourceType='GAIN',
                       mode='channel',
                       algorithm='hogbom',
                       maxPixels=500,
                       bandpassCal=[bestMethod.BestMethod,
                       bandpassCalibration.BandpassCalibration],
                       gainCal=fluxCalibration.FluxCalibration,
                       psf=psf.Psf,
                       dirtyImage=dirtyImageV2.DirtyImageV2))),
#                  operator=imageFlagger.ImageFlagger(rules=[
#                       {'rule':'outlier', 'colour':'crimson', 'limit':10.0,
#                       'min_N':10},
#                       {'rule':'max abs', 'colour':'deeppink', 'limit':0.5}
#                       ]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'Spectra from GAIN calibrater',
          'stage':"""
                  view=sourceSpectra.SourceSpectra(
                     view=cleanImageV2.CleanImageV2(
                     sourceType='GAIN',
                     mode='channel',
                     algorithm='hogbom',
                     maxPixels = 500,
                     bandpassCal=[bestMethod.BestMethod,
                     bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2)),
                  display=sliceDisplay.SliceX()"""
         },

         {'name':'TARGET clean map cube',
          'stage':"""
                  view=cleanImageV2.CleanImageV2(
                     sourceType='SOURCE',
                     mode='channel',
                     algorithm='hogbom',
                     maxPixels = 500,
                     bandpassCal=[bestMethod.BestMethod,
                     bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2),
                  display=skyDisplay.SkyDisplay()"""
         },

         {'name':'Display TARGET closure phase',
          'stage':"""
                  view=chunkMedian.ChunkMedian(
                      view=closurePhase.ClosurePhase(
                      view=cleanImageV2.CleanImageV2(
                      sourceType='SOURCE',
                      mode='channel',
                      algorithm='hogbom',
                      maxPixels = 500,
                      bandpassCal=[bestMethod.BestMethod,
                      bandpassCalibration.BandpassCalibration],
                      gainCal=fluxCalibration.FluxCalibration,
                      psf=psf.Psf,
                      dirtyImage=dirtyImageV2.DirtyImageV2))),
#                  operator=imageFlagger.ImageFlagger(rules=[
#                       {'rule':'outlier', 'colour':'crimson', 'limit':10.0,
#                       'min_N':10}
#                       {'rule':'max abs', 'colour':'crimson', 'limit':35}
#                       ]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'Spectra from TARGET sources',
          'stage':"""
                  view=sourceSpectra.SourceSpectra(
                     view=cleanImageV2.CleanImageV2(
                     sourceType='SOURCE',
                     mode='channel',
                     algorithm='hogbom',
                     maxPixels = 500,
                     bandpassCal=[bestMethod.BestMethod,
                     bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2)),
                 display=sliceDisplay.SliceX()"""
         }

         ]
