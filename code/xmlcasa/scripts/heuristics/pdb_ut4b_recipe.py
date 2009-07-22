"""Pipeline Heuristics Reduction Script for Single Field Interferometry"""

# History:
# 10-Apr-2008 jfl F2F release.
#  2-Jun-2008 jfl 2-jun release.
# 25-Jun-2008 jfl regression release.
# 10-Sep-2008 jfl msCalibrater release.
# 26-Sep-2008 jfl mosaic release.
# 10-Oct-2008 jfl complex display release.
#  3-Nov-2008 jfl amalgamated stage release.
# 12-Dec-2008 jfl 12-dec release.
# 23-Dec-2008 jfl maxPixels reduced to 300 to compensate for halving of cell
#                 size.
#  7-Apr-2009 jfl mosaic release.
#  2-Jun-2009 jfl line and continuum release.

recipe = [ 
         {'comment':'''Direct flagging; e.g. autocorrelations.'''},

         {'name':'basic flagging',
          'description':"""This stage flagged data that were already known to be
                  bad or were inappropriate to be used in further reduction.""",
          'stage':"""view=baseData.BaseData(),
                   operator=taqlFlagger.TaqlFlagger(rules=[
                       {'rule':'autocorrelation', 'colour':'crimson'},
                       {'rule':'PdB Gibbs channels', 'colour':'tomato'}])"""
         },

         {'comment':'Results from direct reduction of data.'},

         {'name':'Initial F calibration amplitude display',
          'description':"""The aim of this stage was to show the
                  flux calibration amplitudes that were derived in a
                  straightforward way from the MeasurementSet before
                  it had undergone any significant flagging.""",
          'stage':"""
                  view=fluxCalibration.FluxCalibrationAmplitude(
                       bandpassCal=[bandpassCalibration.BandpassCalibration]),
                  display=sliceDisplay.SliceY()"""
         },

         {'name':'Initial F calibration phase display',
          'description':"""The aim of this stage was to show the
                  flux calibration phases that were derived in a
                  straightforward way from the MeasurementSet before
                  it had undergone any significant flagging.""",
          'stage':"""
                  view=fluxCalibration.FluxCalibrationPhase(
                       bandpassCal=[bandpassCalibration.BandpassCalibration]),
                  display=sliceDisplay.SliceY()"""
         },

         {'name':'Initial GAIN and TARGET clean maps',
          'description':"""The aim of this stage was to show
                  cleaned images of the GAIN calibrator and target sources
                  using a straightforward calibration method
                  before the data had undergone any significant flagging.""",
          'stage':"""
                  view=cleanImageV2.CleanImageV2(
                     sourceType=['*GAIN*', '*SOURCE*'],
                     mode='mfs',
                     algorithm='hogbom',
                     maxPixels=300,
                     bandpassCal=[bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2),
                  display=skyDisplay.SkyDisplay()"""
         },

         {'comment':'Flag bad antennas in raw calibrater data.'},

         {'name':'Flag calibrator baselines with noisy amplitudes',
          'description':"""The aim of this stage was to flag baselines in
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
          'description':"""The aim of this stage was to flag baselines in
                  calibrater data that have a lot of phase noise. In addition,
                  if a large fraction of the baselines from a given antenna
                  had been flagged then all data for that antenna were
                  flagged.""",
          'stage':"""
                  view=buildImage.BuildImage(yAxis='ANTENNA1',
                       view=medianAndMAD.MAD(
                       collapseAxis='TIME', view=
                       deviations.RawPhaseDeviationPerBaseline(
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

         {'comment':'Antenna based flagging of bad calibrator timestamps.'},

         {'name':'''Flag antenna timestamps with bad amplitudes in calibrator data''',
          'description':"""The aim of this stage was to look at the
                  calibrater data for each antenna as a function of time
                  and to flag timestamps whose coadded amplitudes were
                  statistical outliers.
                  In addition, if a large fraction of the time series from a
                  given antenna had been flagged then all data for that
                  antenna were flagged. If a large fraction of the data from
                  all antennas at a given timestamp had been flagged then
                  all data for that timestamp were flagged.""",
          'stage':"""
                  view=coaddedAntennaAmplitude.\
                       CoaddedAntennaRawScalarAmplitude(
                       sourceType=['*BANDPASS*', '*GAIN*', '*FLUX*']),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'too many flags', 'axis':'TIME', 'limit':0.7,
                       'colour':'crimson'},
                       {'rule':'too many flags', 'axis':'ANTENNA', 'limit':0.7,
                       'colour':'deeppink'},
                       {'rule':'outlier', 'axis':'TIME', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'}]),
                  display=imageDisplay.ImageDisplay()"""
         },
                       
         {'comment':'Baseline based flagging of bad calibrator timestamps.'},
    
         {'name':'Flag baseline timestamps with deviant amplitudes in calibrator data',
          'description':"""The aim of this stage was to look at the
                  calibrater data for each baseline as a function of time
                  and to flag timestamps whose amplitudes were statistical
                  outliers.""",
          'stage':"""
                  view=deviations.RawAmplitudeDeviationPerBaseline(
                       sourceType=['*BANDPASS*', '*GAIN*', '*FLUX*']),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'axis':'TIME', 'rule':'outlier', 'colour':'crimson',
                       'min_N':10, 'limit':10.0}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'comment':'Detect and flag bandpass edges.'},
 
         {'name':'Detect BANDPASS edge',
          'description':"""The aim of this stage was to detect the edges
                  of the bandpass in each spectral window. The identity
                  of the edge channels was stored so that they could
                  be flagged as needed in later stages.""",
          'stage':"""
                  view=medianAndMAD.MedianAndMAD(
                       collapseAxis='ANTENNA',
                       view=bandpassCalibration.BandpassCalibrationAmplitude(
                       bandpassFlaggingStage='Detect BANDPASS edge'),
                       description='''This data view for each spectral window
                       is a 1-d array (degenerate 2-d, with 1 axis having a
                       single point) with x-axis:CHANNEL. The value in each
                       channel is the median of the bandpass calibration
                       amplitudes from all antennas.'''),
                  operator = bandpassEdgeFlagger.BandpassEdgeFlagger(rules=[
                       {'rule':'PdB edge template', 'colour':'crimson'}]),
                  display=sliceDisplay.SliceX()"""
         },

         {'comment':"""Find the best bandpass calibration method (doesn't do
           much for the regression test)."""},

         {'name':'Find best bandpass calibration',
          'description':"""The aim of this stage was to find the best method
                  for calculating the bandpass calibration in this data set.
                  This was achieved by using a range of methods to calculate
                  the bandpass calibration, then calibrating another source by
                  each result, lastly measuring the 'flatness' of that
                  spectrum. The best method was that giving the flattest
                  spectrum.""",
          'stage':"""
                  view=bestMethodSelector.BestMethodSelector(
                       view=rangeTrial.RangeTrial(parameterRange=
                       {'G_t':[60.0],
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

# all types are flagged by the next stage. This is because bad bandpass
# baselines are being flagged. Doing this just for BANDPASS would not
# prevent calibrations being calculated and applied to those baselines in
# the SOURCE.

         {'name':'Flag best bandpass calibration',
          'description':"""The aim of this stage was to flag baselines
                  for which the 'best' bandpass calibration method
                  did not produce very good results.
                  This was achieved by using the 'best' method to calculate
                  the bandpass calibration, then calibrating another source
                  with it, lastly measuring the 'flatness' of that
                  spectrum to produce a 'figure of merit' for each baseline.
                  Baselines with anomalous, high figures of merit were
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
                       'min_N':10}],
                       flag_targets=['*BANDPASS*', '*GAIN*', '*FLUX*', '*SOURCE*']),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'comment':'Flag on calibration results.'},

         {'name':'Antenna flagging on G calibration SNR',
          'description':"""The aim of this stage was to flag G
                  calibrations with unusually low signal to noise ratio.
                  This stage is still under development - the flagging
                  will flag G calibrations with unusually high signal
                  to noise as well.""",
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
                       {'rule':'min abs', 'limit':5.0, 'colour':'brown'},
                       {'rule':'low outlier', 'limit':10.0, 'min_N':10,
                       'colour':'tomato'}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'Antenna flagging on G calibration median phase jump',
          'description':"""The aim of this stage was to flag antennas
                  for which the median absolute phase jump between G 
                  calibrations was unusually high.""",
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
                       {'axis':'ANTENNA', 'rule':'outlier', 'colour':'crimson',
                       'limit':10.0, 'min_N':10}],
                       flag_targets=['*BANDPASS*', '*GAIN*', '*FLUX*', '*SOURCE*']),
                  display=sliceDisplay.SliceY()"""
         },
          
         {'comment':'Display flux calibrated antenna gains.'},
                  
         {'name':'F calibration amplitude display',
          'description':"""The aim of this stage was to show the
                  amplitudes of the flux calibration as calculated
                  after flagging of the calibrators has been completed.
                  This is the calibration that will be used in the
                  calculation of the final images.""",
          'stage':"""
                  view=fluxCalibration.FluxCalibrationAmplitude(
                       bandpassCal=[bestMethod.BestMethod,
                       bandpassCalibration.BandpassCalibration],
                       bandpassFlaggingStage='Detect BANDPASS edge'),
                  display=sliceDisplay.SliceY()"""
         },

         {'name':'F calibration phase display',
          'description':"""The aim of this stage was to show the
                  phases of the flux calibration as calculated
                  after flagging of the calibrators has been completed.
                  This is the calibration that will be used in the
                  calculation of the final images.""",
          'stage':"""
                  view=fluxCalibration.FluxCalibrationPhase(
                       bandpassCal=[bestMethod.BestMethod,
                       bandpassCalibration.BandpassCalibration],
                       bandpassFlaggingStage='Detect BANDPASS edge'),
                  display=sliceDisplay.SliceY()"""
         },

         {'comment':'Flag bad TARGET data.'},

         {'name':'Antenna flagging of bad timestamps in calibrated TARGET amplitudes',
          'description':"""The aim of this stage was to look at the
                  calibrated target data for each antenna as a function of time
                  and to flag timestamps whose amplitudes were statistical
                  outliers.
                  In addition, if a large fraction of the time series from a
                  given antenna had been flagged then all data for that
                  antenna were flagged. If a large fraction of the data from
                  all antennas at a given timestamp had been flagged then
                  all data for that timestamp were flagged.""",
 
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
         },

         {'comment':'Display of calibrated complex data'},

         {'name':'calibrated GAIN and TARGET complex data',
          'description':"""The aim of this stage was to display the calibrated
                  visibilities of the GAIN calibrater and target source as
                  points on the complex plane. For multi-channel spectral
                  windows the visibilities were averaged over the channels
                  before display.""",
          'stage':"""
                  view=baselineData.BaselineCorrectedComplexData(
                   sourceType=['*GAIN*', '*SOURCE*'],
                   bandpassCal=[bestMethod.BestMethod,
                   bandpassCalibration.BandpassCalibration],
                   gainCal=fluxCalibration.FluxCalibration,
                   bandpassFlaggingStage='Detect BANDPASS edge'),
                  display=complexDisplay.ComplexDisplay()"""
         },

         {'name':'calibrated GAIN and TARGET amplitude display',
          'description':"""The aim of this stage was to display the
                  calibrated visibility amplitudes of the GAIN calibrater
                  and target source in such a way that deviant data
                  can be identified as coming from a particular time
                  range or antenna. For multi-channel spectral
                  windows the visibilities were averaged over the channels
                  before display.""",
          'stage':"""
                  view=baselineData.BaselineCorrectedAmplitude(
                   sourceType=['*GAIN*', '*SOURCE*'],
                   bandpassCal=[bestMethod.BestMethod,
                   bandpassCalibration.BandpassCalibration],
                   gainCal=fluxCalibration.FluxCalibration,
                   bandpassFlaggingStage='Detect BANDPASS edge'),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'calibrated GAIN and TARGET phase display',
          'description':"""The aim of this stage was to display the
                  calibrated visibility phases of the GAIN calibrater
                  and target source in such a way that deviant data
                  can be identified as coming from a particular time
                  range or antenna. For multi-channel spectral   
                  windows the visibilities were averaged over the channels
                  before display.""",
          'stage':"""
                  view=baselineData.BaselineCorrectedPhase(
                   sourceType=['*GAIN*', '*SOURCE*'],
                   bandpassCal=[bestMethod.BestMethod,
                   bandpassCalibration.BandpassCalibration],
                   gainCal=fluxCalibration.FluxCalibration,
                   bandpassFlaggingStage='Detect BANDPASS edge'),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'comment':'Cleaned GAIN and TARGET images and spectra.'},

         {'name':'GAIN calibrater clean cube',
          'description':"""The aim of this stage was to show
                  the cleaned map of the GAIN calibrator, calculated
                  from the flagged data using the 'best' calibration
                  methods.""",
          'stage':"""
                  view=cleanImageV2.CleanImageV2(
                     sourceType='*GAIN*',
                     mode='channel',
                     algorithm='hogbom',
                     maxPixels=300,
                     bandpassCal=[bestMethod.BestMethod,
                     bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2,
                     bandpassFlaggingStage='Detect BANDPASS edge'),
                  display=skyDisplay.SkyDisplay()"""
         },

         {'name':'Display GAIN closure phase.',
          'description':"""The aim of this stage was to flag
                  data with unusually high closure phase errors.
                  This stage is still under development - the results
                  are displayed but no flagging is done.""",
           'stage':"""
                  view=chunkMedian.ChunkMedian(
                       view=closurePhase.ClosurePhase(
                       view=cleanImageV2.CleanImageV2(
                       sourceType='*GAIN*',
                       mode='channel',
                       algorithm='hogbom',
                       maxPixels=300,
                       bandpassCal=[bestMethod.BestMethod,
                       bandpassCalibration.BandpassCalibration],
                       gainCal=fluxCalibration.FluxCalibration,
                       psf=psf.Psf,
                       dirtyImage=dirtyImageV2.DirtyImageV2,
                       bandpassFlaggingStage='Detect BANDPASS edge'))),
#
#                  operator=imageFlagger.ImageFlagger(rules=[
#                       {'rule':'outlier', 'colour':'crimson', 'limit':10.0,
#                       'min_N':10},
#                       {'rule':'max abs', 'colour':'deeppink', 'limit':0.5}
#                       ]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'spectra from GAIN calibrater',
          'description':"""The aim of this stage was to show
                  the spectra from sources detected during the cleaning
                  of the GAIN calibrater. The GAIN calibrater is
                  normally a continuum source so the spectrum
                  of it should be flat.""",
          'stage':"""
                  view=sourceSpectra.SourceSpectra(
                     view=cleanImageV2.CleanImageV2(
                     sourceType='*GAIN*',
                     mode='channel',
                     algorithm='hogbom',
                     maxPixels=300,
                     bandpassCal=[bestMethod.BestMethod,
                     bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2,
                     bandpassFlaggingStage='Detect BANDPASS edge')),
                  display=sliceDisplay.SliceX()"""
         },

         {'name':'TARGET clean map cube',
          'description':"""The aim of this stage was to show
                  the cleaned map of the target source, calculated
                  from the flagged data using the 'best' calibration
                  methods.""",
          'stage':"""
                  view=cleanImageV2.CleanImageV2(
                     sourceType='*SOURCE*',
                     mode='channel',
                     algorithm='hogbom',
                     maxPixels=300,
                     bandpassCal=[bestMethod.BestMethod,
                     bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2,
                     bandpassFlaggingStage='Detect BANDPASS edge'),
                  display=skyDisplay.SkyDisplay()"""
         },

         {'name':'Display TARGET closure phase', 
          'description':"""The aim of this stage was to flag
                  data with unusually high closure phase errors.
                  This stage is still under development - the results
                  are displayed but no flagging is done.""",
          'stage':"""
                  view=chunkMedian.ChunkMedian(
                       view=closurePhase.ClosurePhase(
                       view=cleanImageV2.CleanImageV2(
                       sourceType='*SOURCE*',
                       mode='channel',
                       algorithm='hogbom',
                       maxPixels=300,
                       bandpassCal=[bestMethod.BestMethod,
                       bandpassCalibration.BandpassCalibration],
                       gainCal=fluxCalibration.FluxCalibration,
                       psf=psf.Psf,
                       dirtyImage=dirtyImageV2.DirtyImageV2,
                       bandpassFlaggingStage='Detect BANDPASS edge'))),
#                  operator=imageFlagger.ImageFlagger(rules=[
#                       {'rule':'outlier', 'colour':'crimson', 'limit':10.0,
#                       'min_N':10}]),
                  display=imageDisplay.ImageDisplay()"""
         },

         {'name':'spectra from TARGET sources',
          'description':"""The aim of this stage was to show
                  the spectra from sources detected during the cleaning
                  of the target field.""",
          'stage':"""
                  view=sourceSpectra.SourceSpectra(
                     view=cleanImageV2.CleanImageV2(
                     sourceType='*SOURCE*',
                     mode='channel',
                     algorithm='hogbom',
                     maxPixels=300,
                     bandpassCal=[bestMethod.BestMethod,
                     bandpassCalibration.BandpassCalibration],
                     gainCal=fluxCalibration.FluxCalibration,
                     psf=psf.Psf,
                     dirtyImage=dirtyImageV2.DirtyImageV2,
                     bandpassFlaggingStage='Detect BANDPASS edge')),
                  display=sliceDisplay.SliceX()"""
         }

         ]
