SDFlagRule = {\
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
    'Flagging':\
        {'ApplicableDuration': 'subscan'}, \
}

# ApplicableDuration: 'raster' | 'subscan'

