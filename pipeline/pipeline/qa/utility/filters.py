#!/usr/bin/env python

'''Common filters.'''

import numpy as np

def outlierFilter(data, sigma):
    nData = np.array(data)
    nDataMedian = np.median(nData)
    nDataMAD = np.median(abs(nData-nDataMedian)) / 0.6745
    return nData[np.where(abs(nData-nDataMedian) < sigma * nDataMAD)]
