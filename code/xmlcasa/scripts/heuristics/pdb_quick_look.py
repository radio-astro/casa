"""Reduction Script for PdB Quick Look"""

# History:
# 23-jan-2008 jfl Original version.

recipe = [ 
         {'comment':'''Direct flagging; e.g. autocorrelations.'''},

         {'name':'basic flagging',
          'colour':'deepskyblue',
          'stage':"""view=baseData.BaseData(),
                   operator=taqlFlagger.TaqlFlagger(rules=[
                       {'rule':'autocorrelation', 'colour':'crimson'},
                       {'rule':'PdB Gibbs channels', 'colour':'tomato'}]),
                   display=noDisplay.NoDisplay()"""
         },

         {'comment':'Display flux calibrated antenna gains.'},

         {'name':'F calibration amplitude display',
          'stage':"""view=fluxCalibration.FluxCalibrationAmplitude(),
                  operator=noFlagger.NoFlagger(),
                  display=sliceDisplay.SliceY()"""
         },

         {'name':'F calibration phase display',
          'stage':"""view=fluxCalibration.FluxCalibrationPhase(),
                  operator=noFlagger.NoFlagger(),
                  display=sliceDisplay.SliceY()"""
         },

         {'comment':'Dirty images.'},

         {'name':'TARGET dirty image',
          'stage':"""view=dirtyImage.DirtyImage(sourceType='SOURCE',
                       mode='mfs'),
                  operator=noFlagger.NoFlagger(),
                  display=skyDisplay.SkyDisplay()"""
         }

         ]
