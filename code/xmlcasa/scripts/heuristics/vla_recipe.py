"""Pipeline Heuristics Reduction Script for Single Field Interferometry"""

# History:
#  7-Jan-2008 jfl Moved flagging of TARGET to just before imaging, added
#                 flagging of TARGET edge channels.
#  9-Jan-2008 jfl Made calibrator maps mfs, TARGET maps cube.
# 20-Mar-2008 jfl BookKeeper release.
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
#  2-Jun-2008 jfl 2-jun release.
# 25-Jun-2008 jfl regression release.
# 10-Sep-2008 jfl msCalibrater release.
# 26-Sep-2008 jfl mosaic release.
# 10-Oct-2008 jfl complex display release.
#  3-Nov-2008 jfl amalgamated stage release.
# 14-Nov-2008 jfl document upgrade release.
# 12-Dec-2008 jfl 12-dec release.
# 21-Jan-2009 jfl ut4b release.
#  2-Jun-2009 jfl line and continuum release.
# 15-Jun-2009 jfl clean displays calibration internally.
# 31-Jul-2009 jfl no maxPixels release.

recipe = [ 
         {'comment':'''Direct flagging; e.g. autocorrelations.'''},

         {'name':'Basic flagging',
          'description':"""This stage flags data that are already known to be
                  bad or are inappropriate for use in further reduction.""",
          'stage':"""view=baseData.BaseData(),
                   operator=taqlFlagger.TaqlFlagger(rules=
                       [{'rule':'autocorrelation', 'colour':'crimson'}])"""
         },

         {'comment':'Results from direct calibration of data.'},

         {'name':'Initial B calibration coefficients display',
          'description':"""The aim of this stage is to show the
                  bandpass calibration coefficients that
                  are derived in a straightforward way from the
                  MeasurementSet before it has undergone any significant
                  flagging.""",
          'stage':"""
                  view=bandpassCalibration.BandpassCalibration(),
                  display=complexSliceDisplay.SliceX()"""

         },

         {'name':'Initial F calibration coefficients display',
          'description':"""The aim of this stage is to show the 
                  flux calibration coefficients that are derived in a
                  straightforward way from the MeasurementSet before
                  it has undergone any significant flagging.""",
          'stage':"""
                  view=fluxCalibration.FluxCalibration(
                       bandpassCal=[bandpassCalibration.BandpassCalibration]),
                  display=complexSliceDisplay.SliceY()"""
         },

#          {'name':'Initial GAIN and TARGET clean integrated maps',
#           'description':"""The aim of this stage is to show 
#                   cleaned images of the GAIN calibrator and target sources
#                   using a straightforward calibration method 
#                   before the data has undergone any significant flagging.
#                   The maps show the emission integrated over the
#                   spectral window""",
#           'stage':"""
#                   view=cleanImageV2.CleanImageV2(
#                      sourceType=['*GAIN*', '*SOURCE*', '*FLUX*'],
#                      mode='mfs',
#                      algorithm='hogbom',
#                      maxPixels = 1000,
#                      bandpassCal=[bandpassCalibration.BandpassCalibration],
#                      bandpassCalDisplay=complexSliceDisplay.SliceX,
#                      gainCal=fluxCalibration.FluxCalibration,
#                      gainCalDisplay=complexSliceDisplay.SliceY,
#                      psf=psf.Psf,
#                      dirtyImage=dirtyImageV2.DirtyImageV2),
#                   display=skyDisplay.SkyDisplay()"""
#          },

         {'comment':'Flag bad antennas in raw calibrater data.'},

         {'name':'Flag calibrator baselines with noisy amplitudes',
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
                       {'rule':'high outlier', 'colour':'crimson', 'limit':10.0,
                       'min_N':10}]), 
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'Flag calibrator baselines with noisy phases',
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
                       {'rule':'high outlier', 'colour':'crimson', 'limit':10.0,
                       'min_N':10},
                       {'rule':'too many flags', 'axis':'ANTENNA2', 'limit':0.6,
                       'colour':'deeppink'}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'comment':'Flag antenna timestamps with bad calibrator data.'},

         {'name':'''Flag antenna timestamps with bad amplitudes in calibrator data''',
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
                       bandpassCal=None,
                       sourceType=['*BANDPASS*', '*GAIN*', '*FLUX*']),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'too many flags', 'axis':'TIME', 'limit':0.7,
                       'colour':'crimson'},
                       {'rule':'too many flags', 'axis':'ANTENNA', 'limit':0.7,
                       'colour':'deeppink'},
                       {'rule':'outlier in chunk', 'axis':'TIME', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'Display calibration phases for each antenna/timestamp in calibrator data',
          'description':"""The aim of this stage is to display the 
                  phase behaviour of the calibrator data for each antenna as 
                  a function of time.""",
          'stage':"""
                  view=gainCalibration.GainCalibrationPhasePerTimestamp(
                       bandpassCal=None,
                       sourceType=['*BANDPASS*', '*GAIN*', '*FLUX*']),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'comment':'Flag baseline timestamps with bad calibrator data.'},

         {'name':'Flag baseline timestamps with outlying amplitudes in calibrator data',
          'description':"""The aim of this stage is to look at the
                  calibrater data for each baseline as a function of time
                  and to flag timestamps whose amplitudes are statistical
                  outliers.""",
          'stage':"""
                  view=deviations.RawAmplitudeDeviationPerBaseline(
                       sourceType=['*BANDPASS*', '*GAIN*', '*FLUX*']),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'axis':'TIME', 'rule':'outlier in chunk',
                       'colour':'crimson',
                       'min_N':10, 'limit':10.0}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'comment':'Detect bandpass edges.'},

         {'name':'Detect BANDPASS edge',
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
                       {'rule':'VLA edge template', 'colour':'crimson'}]),
                  display=sliceDisplay.SliceX()"""
         },

         {'comment':"""Find the best bandpass calibration method."""},

         {'name':'Find best bandpass calibration',
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
                       {'G_t':[20.0,80.0,160.0,320.0,3600.0],
#                       {'G_t':[60.0],
                       'method':[{'mode':'CHANNEL'}],
                       'source_type':['*BANDPASS*'],
                       'bandpass_flagging_stage':['Detect BANDPASS edge'],
                       'test_source_type':['*FLUX*']},
                       viewClassList=[bandpassMerit.BandpassMerit,
                       bandpassCalibration.BandpassTestCalibratedAmplitude]),
                       description='''The data view is a list of 2-d arrays,
                       one for each spectral window, with axes 
                       ANTENNA1 and ANTENNA2. Each
                       pixel shows the 'merit' of the bandpass calibration
                       test for that baseline. Numbers of order 1 or less
                       are good.'''), 
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':"Display 'quality' of best bandpass calibration",
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
         },

# all types are flagged by the next stage. This is because bad bandpass
# baselines are being flagged. Doing this just for BANDPASS would not 
# prevent calibrations being calculated and applied to those baselines in
# the SOURCE.

         {'name':'Flag best bandpass calibration',
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
                       {'rule':'outlier', 'colour':'crimson', 'limit':10.0,
                       'min_N':10},
#                       {'rule':'max abs', 'colour':'deeppink', 'limit':5.0},
                       {'rule':'too many flags', 'axis':'ANTENNA2', 'limit':0.7,
                       'colour':'brown'}],
                       flag_targets=['*BANDPASS*','*GAIN*','*FLUX*','*SOURCE*']),   
                  display=imageDisplay.ImageDisplay()"""
         },

         {'comment':'Flag on calibration results.'},

         {'name':'Flag antenna G calibrations with bad SNR',
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
                       {'rule':'min abs', 'limit':10.0, 'colour':'brown'},
                       {'rule':'low outlier', 'limit':10.0, 'min_N':10,
                       'colour':'tomato'}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'Flag antennas with high G calibration median phase jump',
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
                       {'axis':'ANTENNA', 'rule':'high outlier', 'colour':'crimson',
                       'limit':10.0, 'min_N':10}],
                       flag_targets=['*BANDPASS*', '*GAIN*', '*FLUX*', '*SOURCE*']),
                  display=sliceDisplay.SliceY()"""
         },

         {'comment':'Flag bad TARGET data'},

         {'name':'Flag antenna timestamps with bad amplitudes in TARGET data',
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
         },

         {'comment':'Flag data with large closure errors.'},

         {'name':'Flag GAIN closure magnitude',
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
                      mode='mfs',
                      algorithm='hogbom',
                      maxPixels=1000,
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
         },

         {'name':'Flag TARGET closure magnitude',
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
                      Each pixel is the median over time of the 
                      closure error magnitude [distance from 1 + 0i]
                      of the ratio between source model and data''',
                      view=closureError.ClosureErrorMagnitude(
                      view=cleanImageV2.CleanImageV2(
                      sourceType='*SOURCE*',
                      mode='mfs',
                      algorithm='hogbom',
                      maxPixels = 1000,
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
         },

         {'comment':'Display final flux calibrated gain coefficients.'},

         {'name':'Final B calibration coefficients display',
          'description':"""The aim of this stage is to show the
                  bandpass calibration coefficients that
                  are derived in a straightforward way from the
                  MeasurementSet before it has undergone any significant
                  flagging.""",
          'stage':"""
                  view=bestMethod.BestMethod(viewClassList=[
                  bandpassCalibration.BandpassCalibration]),
                  display=complexSliceDisplay.SliceX()"""
         },

         {'name':'F calibration coefficient display',
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
         },

         {'comment':'Display of final flagged and calibrated complex data'},

         {'name':'calibrated GAIN complex data',
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
         },

         {'name':'calibrated GAIN and TARGET amplitude display',
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


#          {'comment':'Cleaned calibrator images and spectra.'},

#          {'name':'Calibrator clean cubes',
#           'description':"""The aim of this stage is to show 
#                   the cleaned cubes of the calibrator sources, calculated
#                   from the flagged data using the 'best' calibration
#                   methods.""",
#           'stage':"""
#                   view=cleanImageV2.CleanImageV2(
#                      sourceType=['*GAIN*','*FLUX*','*BANDPASS*'],
#                      mode='channel',
#                      algorithm='hogbom',
#                      maxPixels = 1000,
#                      bandpassCal=[bestMethod.BestMethod,
#                      bandpassCalibration.BandpassCalibration],
#                      bandpassCalDisplay=complexSliceDisplay.SliceX,
#                      gainCal=fluxCalibration.FluxCalibration,
#                      gainCalDisplay=complexSliceDisplay.SliceY,
#                      psf=psf.Psf,
#                      dirtyImage=dirtyImageV2.DirtyImageV2,
#                      bandpassFlaggingStage='Detect BANDPASS edge'),
#                   display=skyDisplay.SkyDisplay()"""
#          },

#          {'name':'Calibrator spectra',
#           'description':"""The aim of this stage is to show
#                   the spectra from sources detected during the cleaning
#                   of the calibrators.""",
#           'stage':"""
#                   view=sourceSpectra.SourceSpectra(
#                      view=cleanImageV2.CleanImageV2(
#                      sourceType=['*GAIN*','*FLUX*','*BANDPASS*'],
#                      mode='channel',
#                      algorithm='hogbom',
#                      maxPixels = 1000,
#                      bandpassCal=[bestMethod.BestMethod,
#                      bandpassCalibration.BandpassCalibration],
#                      gainCal=fluxCalibration.FluxCalibration,
#                      psf=psf.Psf,
#                      dirtyImage=dirtyImageV2.DirtyImageV2,
#                      bandpassFlaggingStage='Detect BANDPASS edge')),
#                   display=sliceDisplay.SliceX()"""
#          },

#          {'comment':'Cleaned TARGET images and spectra.'},

#          {'name':'TARGET clean cube',
#           'description':"""The aim of this stage is to show 
#                   the final cleaned cube of the target source, calculated
#                   from the flagged data using the 'best' calibration
#                   methods.""",
#           'stage':"""
#                   view=cleanImageV2.CleanImageV2(
#                      sourceType='*SOURCE*',
#                      mode='channel',
#                      algorithm='hogbom',
#                      maxPixels = 1000,
#                      bandpassCal=[bestMethod.BestMethod,
#                      bandpassCalibration.BandpassCalibration],
#                      bandpassCalDisplay=complexSliceDisplay.SliceX,
#                      gainCal=fluxCalibration.FluxCalibration,
#                      gainCalDisplay=complexSliceDisplay.SliceY,
#                      psf=psf.Psf,
#                      dirtyImage=dirtyImageV2.DirtyImageV2,
#                      bandpassFlaggingStage='Detect BANDPASS edge'),
#                   display=skyDisplay.SkyDisplay()"""
#          },


#          {'name':'TARGET clean map in line and continuum',
#           'description':"""The aim of this stage is to produce separate
#                   clean maps of the line emission and the continuum emission
#                   from the target source, calculated
#                   from the flagged data using the 'best' calbration
#                   methods.""",
#           'stage':"""
#                   view=continuumSubtractedCleanImage.ContinuumSubtractedCleanImage(
#                      sourceType='*SOURCE*',
#                      mode='channel',
#                      algorithm='hogbom',
#                      maxPixels = 1000,
#                      bandpassCal=[bestMethod.BestMethod,
#                      bandpassCalibration.BandpassCalibration],
#                      gainCal=fluxCalibration.FluxCalibration,
#                      psf=psf.Psf,
#                      dirtyImage=dirtyImageV2.DirtyImageV2,
#                      bandpassFlaggingStage='Detect BANDPASS edge'),
#                   display=skyDisplay.SkyDisplay()"""
#          },

#          {'name':'Spectra from TARGET sources',
#           'description':"""The aim of this stage is to show
#                   the spectra from sources detected during the cleaning
#                   of the target field.""",
#           'stage':"""
#                   view=sourceSpectra.SourceSpectra(
#                      view=cleanImageV2.CleanImageV2(
#                      sourceType='*SOURCE*',
#                      mode='channel',
#                      algorithm='hogbom',
#                      maxPixels = 1000,
#                      bandpassCal=[bestMethod.BestMethod,
#                      bandpassCalibration.BandpassCalibration],
#                      gainCal=fluxCalibration.FluxCalibration,
#                      psf=psf.Psf,
#                      dirtyImage=dirtyImageV2.DirtyImageV2,
#                      bandpassFlaggingStage='Detect BANDPASS edge')),
#                   display=sliceDisplay.SliceX()"""

#          }

         ]
