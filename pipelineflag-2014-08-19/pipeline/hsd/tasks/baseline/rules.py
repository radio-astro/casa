# Rule for Line Detection
LineFinderRule = {'MaxFWHM': 900, 
                  'MinFWHM': 5, 
                  #'MinFWHM': 3, 
                  'Threshold': 3}
                  #'Threshold': 2.5}
                  #'Threshold': 2}

# Rule for Line Validation
ClusterRule = {'ThresholdValid': 0.5, 
               'ThresholdMarginal': 0.35, 
               'ThresholdQuestionable': 0.2, 
               'MaxCluster': 100, 
               'BlurRatio': 0.3}
