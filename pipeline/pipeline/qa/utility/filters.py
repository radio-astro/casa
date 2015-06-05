#!/usr/bin/env python

'''Common filters.'''

import numpy as np

def getOutlierIndices(nData, sigma):
    nDataMedian = np.median(nData)
    nDataMAD = np.median(abs(nData-nDataMedian)) / 0.6745
    return np.where(abs(nData-nDataMedian) < sigma * nDataMAD)
    
def outlierFilter(data, sigma):
    nData = np.array(data)
    return nData[getOutlierIndices(nData, sigma)]

def outlierDictFilter(data, key, sigma):
    nData = np.array([i[key] for i in data])
    return [data[i] for i in getOutlierIndices(nData, sigma)[0]]

def minDict(data, key):
    nData = np.array([i[key] for i in data])
    return data[np.argmin(nData)]

def maxDict(data, key):
    nData = np.array([i[key] for i in data])
    return data[np.argmax(nData)]
