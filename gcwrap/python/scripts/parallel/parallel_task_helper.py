#!/usr/bin/env python
from taskinit import *
import simple_cluster
import os
import copy
import shutil
import partitionhelper as ph

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
        # jagonzal: Add reference to cluster object
        self._cluster = None

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
            msTool = mstool()
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
        self._cluster = simple_cluster.simple_cluster.getCluster()
        self._jobQueue = simple_cluster.JobQueueManager(self._cluster)
        self._jobQueue.addJob(self._executionList)
        self._jobQueue.executeQueue()

    def postExecution(self):
        if (self._cluster != None):
            return self._cluster.get_return_list()
        else:
            return None
            
    def go(self):
        self.initialize()
        self.generateJobs()
        self.executeJobs()
        retVar = self.postExecution()
        return retVar

    @staticmethod
    def getReferencedMSs(vis):
        msTool = mstool()
        if not msTool.open(vis):
            raise ValueError, "Unable to open MS %s." % vis

        if not msTool.ismultims():
            raise ValueError, "MS %s is not a reference MS." % vis

        rtnValue = msTool.getreferencedtables()
        if not isinstance(rtnValue, list):
            rtnValue = [rtnValue]
      
        msTool.close()
        return rtnValue


    @staticmethod
    def restoreSubtableAgreement(vis, mastersubms='', subtables=[]):
        '''
        Tidy up the MMS vis by replacing the subtables of all SubMSs
        by the subtables from the SubMS given by "mastersubms".
        If specified, only the subtables in the list "subtables"
        are replaced, otherwise all.
        If "mastersubms" is not given, the first SubMS of the MMS
        will be used as master.
        '''

        msTool = mstool();
        msTool.open(vis)
        theSubMSs = msTool.getreferencedtables()
        msTool.close()

        tbTool = tbtool();
        
        if mastersubms=='':
            tbTool.open(vis)
            myKeyw = tbTool.getkeywords()
            tbTool.close()
            mastersubms=os.path.dirname(myKeyw['ANTENNA'].split(' ')[1]) #assume ANTENNA is present

        mastersubms = os.path.abspath(mastersubms)
            
        theSubTables = ph.getSubtables(mastersubms)

        if subtables==[]:
            subtables=theSubTables
        else:
            for s in subtables:
                if not (s in theSubTables):
                    raise ValueError, s+' is not a subtable of '+ mastersubms

        origpath = os.getcwd()      
        masterbase = os.path.basename(mastersubms)
        
        for r in theSubMSs:
            rbase = os.path.basename(r)
            if not rbase==masterbase:
                for s in subtables:
                    theSubTab = r+'/'+s
                    if os.path.islink(theSubTab): # don't copy over links
                        if(os.path.basename(os.path.dirname(os.path.realpath(theSubTab)))!=masterbase):
                            # the mastersubms has changed: make new link
                            os.chdir(r)
                            shutil.rmtree(s, ignore_errors=True)
                            os.symlink('../'+masterbase+'/'+s, s)
                            os.chdir(origpath)
                    else:    
                        shutil.rmtree(theSubTab, ignore_errors=True)
                        #print "Copying from "+mastersubms+'/'+s+" to "+ theSubTab
                        shutil.copytree(mastersubms+'/'+s, theSubTab)

        return True

    @staticmethod
    def isParallelMS(vis):
        '''
        This method will let us know if we can do the simple form
        of parallelization by invoking on many refernced mss.
        '''
        msTool = mstool()
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
