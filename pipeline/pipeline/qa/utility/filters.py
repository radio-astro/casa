#!/usr/bin/env python

'''Common filters.'''

import numpy as np

def outlierFilter(data, sigma):
    dataMedian = np.median(data)
    dataMAD = np.median(abs(data-dataMedian)) / 0.6745
    filteredData = data[np.where(abs(data-dataMedian) < sigma * dataMAD)]
    return filteredData
