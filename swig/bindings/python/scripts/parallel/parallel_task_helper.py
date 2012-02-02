#!/usr/bin/env python
from taskinit import *
import simple_cluster
import os
import copy


class ParallelTaskHelper:
    '''
    This is the extension of the TaskHelper to allow for parallel
    operation.  For simple tasks all that should be required to make
    a task parallel is to use this rather than the TaskHelper method
    above
    '''
    def __init__(self, task_name, args = {}):
        self._arg = args
        self._taskName = task_name
        self._executionList = []
        self._jobQueue = None
        # Cache the initial inputs
        self.__originalParams = args

    def initialize(self):
        '''
        This is the setup portion.
        Currently it:
           * Finds the full path for the input vis.
        '''
        self._arg['vis'] = os.path.abspath(self._arg['vis'])

    def generateJobs(self):
        '''
        This is the method which generates all of the actual jobs to be
        done.  The default is to asume the input vis is a reference ms and
        build one job for each referenced ms.
        '''
        try:
            msTool = mstool.create()
            if not msTool.open(self._arg['vis']):
                raise ValueError, "Unable to open MS %s," % self._arg['vis']
            if not msTool.ismultims():
                raise ValueError, \
                      "MS is not a MultiMS, simple parallelization failed"

            for subMS in msTool.getreferencedtables():
                localArgs = copy.copy(self._arg)
                localArgs['vis'] = subMS
                self._executionList.append(
                    simple_cluster.JobData(self._taskName,localArgs))
        finally:
            msTool.close()


    def executeJobs(self):
        cluster = simple_cluster.simple_cluster.getCluster()
        self._jobQueue = simple_cluster.JobQueueManager(cluster)
        self._jobQueue.addJob(self._executionList)
        self._jobQueue.executeQueue()

    def postExecution(self):
        pass
            
    def go(self):
        self.initialize()
        self.generateJobs()
        self.executeJobs()
        self.postExecution()

    @staticmethod
    def getReferencedMSs(vis):
        msTool = mstool.create()
        if not msTool.open(vis):
            raise ValueError, "Unable to open MS %s." % vis

        if not msTool.ismultims():
            raise ValueError, "MS %s is not a refernce MS." % vis

        rtnValue = msTool.getreferencedtables()
        if not isinstance(rtnValue, list):
            rtnValue = [rtnValue]
      
        msTool.close()
        return rtnValue

    @staticmethod
    def isParallelMS(vis):
        '''
        This task really will let us know if we can do the simple form
        of parallelization by invoking on many refernced mss.
        '''
        msTool = mstool.create()
        if not msTool.open(vis):
            raise ValueError, "Unable to open MS %s," % vis
        rtnVal = msTool.ismultims() and \
                 isinstance(msTool.getreferencedtables(), list)

        msTool.close()
        return rtnVal
    
    @staticmethod
    def findAbsPath(input):
        if isinstance(input,str):
            return os.path.abspath(input)

        if isinstance(input, list):
            rtnValue = []
            for file in input:
                rtnValue.append(os.path.abspath(file))
            return rtnValue

        # Your on your own, don't know what to do
        return input

    @staticmethod
    def listToCasaString(inputList):
        '''
        This Method will take a list of integers and try to express them as a 
        compact set using the CASA notation.
        '''
        if inputList is None or len(inputList) == 0:
            return ''
        
        def selectionString(rangeStart, rangeEnd):
            if rangeStart == rangeEnd:
                return str(rangeStart)
            return "%d~%d" % (rangeStart, rangeEnd)
    
        inputList.sort()
        compactStrings = []
        rangeStart = inputList[0]
        lastValue = inputList[0]
        for val in inputList[1:]:
            if val > lastValue + 1:
                compactStrings.append(selectionString(rangeStart,lastValue))
                rangeStart = val
            lastValue = val
        compactStrings.append(selectionString(rangeStart,lastValue))

        return ','.join([a for a in compactStrings])
