'''
Unit tests for task partition.

This is mostly just a check of the python layers,ensuring correct commands are
passed to the JobQueueManager and assuming that the ms.partition function
works correctly.
'''

import os
import unittest
from task_partition import PartitionHelper
from simple_cluster import JobData

datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/'

class PartitionHelperTest(unittest.TestCase,
                          PartitionHelper):

    def __init__(self, methodName='runTest'):
        # Default Arguments
        argList = {}
        argList['vis'] = ''
        argList['outputvis'] = ''
        argList['createmms'] = False
        argList['separationaxis'] = 'scan'
        argList['numsubms'] = 64
        argList['calmsselection'] = 'none'
        argList['calmsname'] = ''
        argList['calfield'] = ''
        argList['calintent'] =''
        argList['calscan'] = ''
        argList['datacolumn'] = 'corrected'
        argList['field'] = ''
        argList['spw'] = ''
        argList['width'] = 1
        argList['antenna'] = ''
        argList['timebin'] = '0s'
        argList['timerange'] = ''
        argList['scan'] = ''
        argList['scanintent'] = ''
        argList['array'] = ''
        argList['uvrange'] = ''
        argList['correlation'] = ''
        argList['combine'] = ''
        argList['keepflags'] = True

        # Internal variables for simulating various MS structures
        self.testSPWList = None
        self.testScanList = None

        PartitionHelper.__init__(self, argList)
        unittest.TestCase.__init__(self, methodName)
        # Populate the arguments with defaults

    def testScanSeparationCommands(self):
        # Set up this test by creating a few variables we need
        self.dataDir='dataDir'
        self.outputBase='outputBase'

        #=======================================================
        # Check the simple case of more scans than subMSs
        self._arg['numsubms'] = 8
        self.setScanList(range(16))
        self._createScanSeparationCommands()

        commandList = []
        for idx in xrange(8):
            outputvis = '%s/%s.%04d.ms' %  (self.dataDir,self.outputBase,idx)
            commandList.append(JobData('partition',{
                'outputvis': outputvis,
                'scan': self.listToCasaString([2*idx,(2*idx)+1])}))
        self.checkCommandList(commandList)

        #=======================================================
        # Check the case of fewer scans than subMSs
        self._arg['numsubms'] = 8
        self.setScanList(range(6))
        self._createScanSeparationCommands()

        commandList = []
        for idx in xrange(6):
            outputvis = '%s/%s.%04d.ms' %  (self.dataDir,self.outputBase,idx)
            commandList.append(JobData('partition',{
                'outputvis': outputvis,
                'scan': str(idx)}))
        self.checkCommandList(commandList)

        # =======================================================
        # Check the case of the cal Scans being defined
        self._arg['numsubms'] = 8
        self.setScanList(range(32))

        # Do A Selection to populat the calScanList
        self._selectMS(True)
        self._calScanList = self._getScanList()

        # Reset the local Scan list
        self.setScanList(range(32))
        self._createScanSeparationCommands()

        commandList = []
        for idx in xrange(8):
            outputvis = '%s/%s.%04d.ms' %  (self.dataDir,self.outputBase,idx)
            commandList.append(JobData('partition',{
                'outputvis': outputvis,
                'scan': self.listToCasaString([4*idx + 1,4*idx +3])}))
        self.checkCommandList(commandList)


    def testSpwSeparationCommands(self):
        # Set up this test by creating a few variables we need
        self.dataDir='dataDir'
        self.outputBase='outputBase'

        # Check the simple case of more spws than subMSs
        self._arg['numsubms'] = 8
        self.setSPWList(range(16))
        self._createSPWSeparationCommands()

        commandList = []
        for idx in xrange(8):
            outputvis = '%s/%s.%04d.ms' %  (self.dataDir,self.outputBase,idx)
            commandList.append(JobData('partition',{
                'outputvis': outputvis,
                'spw': self.listToCasaString([2*idx,(2*idx)+1])}))
        self.checkCommandList(commandList)


        # Check the case of fewer spws than subMSs
        self._arg['numsubms'] = 8
        self.setSPWList(range(6))
        self._createSPWSeparationCommands()

        commandList = []
        for idx in xrange(6):
            outputvis = '%s/%s.%04d.ms' %  (self.dataDir,self.outputBase,idx)
            commandList.append(JobData('partition',{
                'outputvis': outputvis,
                'spw': str(idx)}))
        self.checkCommandList(commandList)

        #=======================================================
        # Check the case of the cal Scans being defined
        self._arg['numsubms'] = 8
        self.setScanList(range(32))
        self.setSPWList(range(16))

        # Do A Selection to populate the calScanList
        self._selectMS(True)
        self._calScanList = self._getScanList()

        # Reset the local Scan list
        self.setScanList(range(32))
        self._createSPWSeparationCommands()

        commandList = []
        for idx in xrange(8):
            outputvis = '%s/%s.%04d.ms' %  (self.dataDir,self.outputBase,idx)
            commandList.append(JobData('partition',{
                'outputvis': outputvis,
                'spw' : self.listToCasaString([2*idx, 2*idx+1]),
                'scan': self.listToCasaString(range(1,32,2))}))
        self.checkCommandList(commandList)


    def testDefaultSeparationCommands(self):
        # Set up this test by creating a few variables we need
        self.dataDir='dataDir'
        self.outputBase='outputBase'
        
        # ========================================================
        # Check the case where we have more spw than subMS
        self._arg['numsubms'] = 8
        self.setSPWList(range(16))
        self.setScanList(range(10))
        self._createDefaultSeparationCommands()

        # Set up the expected output, Note there is no selection for
        # the CalScans so we do not expect a Scan output
        commandList = []
        for idx in xrange(8):
            outputvis = '%s/%s.%04d.ms' %  (self.dataDir,self.outputBase,idx)
            commandList.append(JobData('partition',{
                'outputvis': outputvis,
                'spw': self.listToCasaString([2*idx,(2*idx)+1])}))
        self.checkCommandList(commandList)

        # ========================================================
        # Check the case of not enough spws
        self._arg['numsubms'] = 16
        self.setSPWList(range(4))
        self.setScanList(range(12))
        self._createDefaultSeparationCommands()

        # Set up the expected output
        commandList = []
        for idx in xrange(16):
            outputvis = '%s/%s.%04d.ms' %  (self.dataDir,self.outputBase,idx)
            commandList.append(JobData('partition',{
                'outputvis': outputvis,
                'scan': self.listToCasaString(range((idx%4)*3,((idx%4)+1)*3)),
                'spw': str(idx/4)}))
        self.checkCommandList(commandList)

        # ========================================================
        # Check the case of not being able to make the number of submss
        self._arg['numsubms'] = 48
        self.setSPWList(range(15))
        self.setScanList(range(2))
        self._createDefaultSeparationCommands()

         # Set up the expected output
        commandList = []
        for idx in xrange(30):
            outputvis = '%s/%s.%04d.ms' %  (self.dataDir,self.outputBase,idx)
            commandList.append(JobData('partition',{
                'outputvis': outputvis,
                'scan': str(idx % 2),
                'spw': str(idx/2)}))
        self.checkCommandList(commandList)
        
        # Check the case of not passing in any scans (but not needing them)
        self._arg['numsubms'] = 8
        self.setSPWList(range(16))
        self._createDefaultSeparationCommands()

        # Set up the expected output
        commandList = []
        for idx in xrange(8):
            outputvis = '%s/%s.%04d.ms' %  (self.dataDir,self.outputBase,idx)
            commandList.append(JobData('partition',{
                'outputvis': outputvis,
                'scan': '',
                'spw': self.listToCasaString([2*idx,(2*idx)+1])}))
        self.checkCommandList(commandList)

        # ========================================================
        # Check the case of not passing any scans (but needing them)
        self._arg['numsubms'] = 16
        self.setSPWList(range(4))
        self.setScanList(range(4))
        self._createDefaultSeparationCommands()

        # Set up the expected output
        commandList = []
        for idx in xrange(16):
            outputvis = '%s/%s.%04d.ms' %  (self.dataDir,self.outputBase,idx)
            commandList.append(JobData('partition',{
                'outputvis': outputvis,
                'scan': str(idx%4),
                'spw': str(idx/4)}))
        self.checkCommandList(commandList)

    def testCreateCalibrationCommand(self):
        pass

    def testCreatePrimarySplitCommand(self):
        # Particularly the single MS command
        
        pass

    def testSelectMS(self):
        pass


    def testGetSPWList(self):
        pass


    def testGetScanList(self):
        pass

    def testPostExecution(self):
        pass

    def testPartition(self):
        pass

    # =========== Internal methods used to help testing ============
    def checkCommandList(self, commandList):
        '''
        Probably could be smarter but lets do it the easy way.
        '''
        for expJob in commandList:
            matchFound = False
            for job in self._executionList:

                if expJob.getCommandNames() != job.getCommandNames():
                    continue


                try:
                    for cmdName in expJob.getCommandNames():
                        expArgs = expJob.getCommandArguments(cmdName)
                        jobArgs = job.getCommandArguments(cmdName)
                        if [expArgs[key] for key in expArgs] != \
                               [jobArgs[key] for key in expArgs]:
                            raise KeyError
                except KeyError:
                    # This means that they don't match; missing key
                    continue

                self._executionList.remove(job)
                matchFound = True
                break
            if not matchFound:
                self.fail("Unable to find match for expected Job")

        if len(self._executionList) != 0:
            self.fail("Found %d uexpected job(s)" % len(self._executionList))
            
    def setScanList(self, scanList):
        self.testScanList = scanList


    def setSPWList(self,spwList):
        self.testSPWList = spwList

    # ========== Overridden methods to allow isolated testing ==========
    def _getScanList(self):
        return self.testScanList

    def _getSPWList(self):
        '''
        Override the method of the Helper class for testing purposes
        '''
        return self.testSPWList
    
    def _selectMS(self, doCalibrationSelection = False):
        '''
        Override the method of the helper class for testing purposes
        Assume all even scans are calibration scans.
        '''
        if not doCalibrationSelection and self._calScanList is not None:
            if self._selectionScanList is None:
                self._selectionScanList = self._getScanList()
                
                for scan in self._calScanList:
                    self._selectionScanList.remove(scan)

                self.setScanList(self._selectionScanList)

        if doCalibrationSelection:
            for scan in self.testScanList:
                if scan%2 == 1:
                    self.testScanList.remove(scan)


def suite():
    return [PartitionHelperTest]

    
if __name__ == '__main__':
    testSuite = []
    for testClass in suite():
        testSuite.append(unittest.makeSuite(testClass,'test'))
    allTests = unittest.TestSuite(testSuite)
    unittest.TextTestRunner(verbosity=2).run(allTests)

