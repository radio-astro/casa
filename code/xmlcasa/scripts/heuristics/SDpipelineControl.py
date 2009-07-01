SDParam = {\
    'WeatherFlag':\
        {'isActive': False, \
         'Reversible': False, \
         'Threshold': 'unknown'}, \
    'TsysFlag':\
        {'isActive': True, \
         'Reversible': False, \
         'Threshold': 3.0}, \
    'UserFlag':\
        {'isActive': True, \
         'Reversible': False, \
         'Threshold': 'unknown'}, \
    'RmsPreFitFlag':\
        {'isActive': True, \
         'Reversible': True, \
         'Threshold': 4.5}, \
         #'Threshold': 2.5}, \
    'RmsPostFitFlag':\
        {'isActive': True, \
         'Reversible': True, \
         'Threshold': 4.0}, \
    'RunMeanPreFitFlag':\
        {'isActive': True, \
         'Reversible': True, \
         'Threshold': 5.5, \
         'Nmean': 5}, \
    'RunMeanPostFitFlag':\
        {'isActive': True, \
         'Reversible': True, \
         'Threshold': 5.0, \
         'Nmean': 5}, \
    'RmsExpectedPreFitFlag':\
        {'isActive': True, \
         'Reversible': True, \
         #'Threshold': 2.66666}, \
         'Threshold': 3.0}, \
    'RmsExpectedPostFitFlag':\
        {'isActive': True, \
         'Reversible': True, \
         #'Threshold': 1.5}, \
         'Threshold': 1.33333}, \
    'Cluster':\
        {'ThresholdValid': 0.5, \
         'ThresholdMarginal': 0.4, \
         'ThresholdQuestionable': 0.2, \
         'MaxCluster': 10,
         'BlurRatio': 0.1}, \
    'FittingOrder':\
        {'ApplicableDuration': 'raster', \
         #'MaxPolynomialOrder': 1}, \
         'MaxPolynomialOrder': 'none'}, \
    'Flagging':\
        {'ApplicableDuration': 'subscan'}, \
    'Gridding':\
        {'WeightDistance': 'Gauss', \
         #'Clipping': 'none', \
         'Clipping': 'MinMaxReject', \
         'WeightRMS': True, \
         'WeightTsysExptime': False} \
}

# ApplicableDuration: 'raster' | 'subscan'
# Gridding['Weight']: 'Gauss' | 'Linear' | 'Const'
# Gridding['Clipping']: 'MinMaxReject' | 'none'
# Gridding['WeightRMS']: True | False

