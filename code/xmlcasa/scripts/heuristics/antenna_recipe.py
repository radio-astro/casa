recipe = [ 
         {'comment':'Flag bad BANDPASS data.'},

         {'name':'BANDPASS antenna amplitude flagging',
          'colour':'slateblue',
          'stage':"""view=coaddedAntennaAmplitude.\
                       CoaddedAntennaRawScalarAmplitude(sourceType='BANDPASS'),
                  operator=sequenceFlagger.SequenceFlagger(rules=[
                       {'rule':'too many flags', 'axis':'TIME', 'limit':0.7,
                       'colour':'crimson'},
                       {'rule':'too many flags', 'axis':'ANTENNA', 'limit':0.7,
                       'colour':'deeppink'},
                       {'rule':'outlier', 'axis':'TIME', 'limit':10.0,
                       'min_N':10, 'colour':'tomato'}]),
                  display=imageDisplay.ImageDisplay()"""
         }
         ]
