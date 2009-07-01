"""Pipeline Heuristics Reduction Script for Single Field Interferometry"""

# History:
#  7-Jan-2008 jfl Moved flagging of TARGET to just before imaging, added
#                 flagging of TARGET edge channels.
#  9-Jan-2008 jfl Made calibrator maps mfs, TARGET maps cube.
# 20-Mar-2008 jfl BookKeeper release.
# 10-Apr-2008 jfl F2F release.

recipe = [ 
         {'comment':'''Direct flagging; e.g. autocorrelations.'''},

         {'name':'basic flagging',
          'stage':"""view=baseData.BaseData(),
                   operator=taqlFlagger.TaqlFlagger(rules=
                       [{'rule':'autocorrelation', 'colour':'crimson'}]),
                   display=noDisplay.NoDisplay()"""
         },

         {'comment':'Sanity check of raw calibrater data.'},

         {'name':'BANDPASS baseline amplitude MAD flagging',
          'stage':"""
                  view=buildImage.BuildImage(yAxis='ANTENNA1', 
                       view=medianAndMAD.MAD(
                       collapseAxis='TIME', view=
                       deviations.RawAmplitudeDeviationPerBaseline(
                       sourceType='BANDPASS'))),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'outlier', 'axis':'ANTENNA2', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'},
                       {'rule':'too many flags', 'axis':'ANTENNA2', 'limit':0.6,
                       'colour':'crimson'}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'BANDPASS baseline phase MAD flagging',
          'stage':"""
                  view=buildImage.BuildImage(yAxis='ANTENNA1', 
                       view=medianAndMAD.MAD(
                       collapseAxis='TIME', view=
                       deviations.RawPhaseDeviationPerBaseline(
                       sourceType='BANDPASS'))),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'outlier', 'axis':'ANTENNA2', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'},
                       {'rule':'too many flags', 'axis':'ANTENNA2', 'limit':0.6,
                       'colour':'crimson'}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'GAIN baseline phase MAD flagging',
          'stage':"""
                  view=buildImage.BuildImage(yAxis='ANTENNA1', 
                       view=medianAndMAD.MAD(
                       collapseAxis='TIME', view=
                       deviations.RawPhaseDeviationPerBaseline(
                       sourceType='GAIN'))),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'outlier', 'axis':'ANTENNA2', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'},
                       {'rule':'too many flags', 'axis':'ANTENNA2', 'limit':0.7,
                       'colour':'crimson'}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'FLUX baseline phase MAD flagging',
          'stage':"""
                  view=buildImage.BuildImage(yAxis='ANTENNA1', 
                       view=medianAndMAD.MAD(
                       collapseAxis='TIME', view=
                       deviations.RawPhaseDeviationPerBaseline(
                       sourceType='FLUX'))),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'outlier', 'axis':'ANTENNA2', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'},
                       {'rule':'too many flags', 'axis':'ANTENNA2', 'limit':0.7,
                       'colour':'crimson'}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'comment':'Flag bad BANDPASS, GAIN, and FLUX data.'},

         {'name':'BANDPASS antenna amplitude flagging',
          'stage':"""
                  view=gainCalibration.GainCalibrationAmplitudePerTimestamp(
                       bandpassCal=[bandpassCalibration.BandpassCalibration],
                       sourceType='BANDPASS'),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'too many flags', 'axis':'TIME', 'limit':0.7,
                       'colour':'crimson'},
                       {'rule':'too many flags', 'axis':'ANTENNA', 'limit':0.7,
                       'colour':'deeppink'},
                       {'rule':'outlier', 'axis':'TIME', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'GAIN antenna amplitude flagging',
          'stage':"""
                  view=gainCalibration.GainCalibrationAmplitudePerTimestamp(
                       bandpassCal=[bandpassCalibration.BandpassCalibration],
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

         {'name':'FLUX antenna amplitude flagging',
          'stage':"""
                  view=gainCalibration.GainCalibrationAmplitudePerTimestamp(
                       bandpassCal=[bandpassCalibration.BandpassCalibration],
                       sourceType='FLUX'),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'too many flags', 'axis':'TIME', 'limit':0.7, 
                       'colour':'crimson'},
                       {'rule':'too many flags', 'axis':'ANTENNA', 'limit':0.7,
                       'colour':'deeppink'},
                       {'rule':'outlier', 'axis':'TIME', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'comment':'Find bandpass edges.'},
 
         {'name':'flag BANDPASS edge',
          'stage':"""
                  view=medianAndMAD.MedianAndMAD(
                       collapseAxis='ANTENNA',
                       view=bandpassCalibration.BandpassCalibrationAmplitude()),
                  operator = bandpassEdgeFlagger.BandpassEdgeFlagger(rules=[
                       {'rule':'VLA edge template', 'colour':'crimson'}]),
                  display=sliceDisplay.SliceX()"""
         },

         {'name':'Remove edge flagging from BANDPASS',
          'stage':"""
                  view=medianAndMAD.MedianAndMAD(
                       collapseAxis='ANTENNA',
                       view=bandpassCalibration.BandpassCalibrationAmplitude()),
                  operator=baseFlagger.BaseFlagger(rules=[  
                       {'rule':'remove flags', 
                       'stageName':'flag BANDPASS edge'}]),
                  display=sliceDisplay.SliceX()"""
         },


         {'comment':'Find the best bandpass calibration.'},

         {'name':'heuristic bandpass calibration selection',
          'stage':"""
                  view=bestMethodSelector.BestMethodSelector(
                       view=rangeTrial.RangeTrial(parameterRange=
                       {'G_INTEGRATION_TIME':[0.0],
                       'METHOD':[{'mode':'CHANNEL'}],
                       'SOURCE_TYPE':['BANDPASS'],
                       'BANDPASS_FLAGGING_STAGE':['flag BANDPASS edge']},
                       viewClassList=[bandpassMerit.BandpassMerit,
                       bandpassCalibration.BandpassSelfCalibratedAmplitude]
                       )),
                  operator=noFlagger.NoFlagger(),
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
                  operator=noFlagger.NoFlagger(),
                  display=sliceDisplay.SliceY()"""
         },

         {'name':'F calibration phase display',
          'stage':"""
                  view=fluxCalibration.FluxCalibrationPhase(
                       bandpassCal=[bestMethod.BestMethod,
		       bandpassCalibration.BandpassCalibration]),
                  operator=noFlagger.NoFlagger(),
                  display=sliceDisplay.SliceY()"""
         },

         {'comment':'Flag bad TARGET data.'},

         {'name':'Flag bandpass edges of TARGET, BANDPASS, GAIN, FLUX',
          'stage':"""view=baseData.BaseData(),
                   operator=taqlFlagger.TaqlFlagger(rules=[
                       {'rule':'flag channels',
                       'stageName':'flag BANDPASS edge',
                       'colour':'crimson'}],
                       flag_targets=['SOURCE', 'GAIN', 'BANDPASS', 'FLUX']),
                   display=noDisplay.NoDisplay()"""
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

         {'comment':'Cleaned images of each source type.'},

         {'name':'BANDPASS calibrater mfs clean map',
          'stage':"""
                  view=cleanImage.CleanImage(
                     sourceType='BANDPASS',
                     mode='mfs',
                     algorithm='hogbom',
                     bandpassCal=[bestMethod.BestMethod,
		     bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psfImage.PsfImage,
                     dirtyImage=dirtyImage.DirtyImage),
                  operator=noFlagger.NoFlagger(),
                  display=skyDisplay.SkyDisplay()"""
         },

         {'name':'GAIN calibrater mfs clean map',
          'stage':"""
                  view=cleanImage.CleanImage(
                     sourceType='GAIN',
                     mode='mfs',
                     algorithm='hogbom',
                     bandpassCal=[bestMethod.BestMethod,
		     bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psfImage.PsfImage,
                     dirtyImage=dirtyImage.DirtyImage),
                  operator=noFlagger.NoFlagger(),
                  display=skyDisplay.SkyDisplay()"""
         },

         {'name':'TARGET clean mfs map',
          'stage':"""
                  view=cleanImage.CleanImage(
                     sourceType='SOURCE',
                     mode='mfs',
                     algorithm = 'hogbom',
                     bandpassCal=[bestMethod.BestMethod,
		     bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psfImage.PsfImage,
                     dirtyImage=dirtyImage.DirtyImage),
                  operator=noFlagger.NoFlagger(),
                  display=skyDisplay.SkyDisplay()"""
         },

         ]
