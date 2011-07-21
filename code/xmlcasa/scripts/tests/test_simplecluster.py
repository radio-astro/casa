import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
import time
from simple_cluster import *
import glob

class simplecluster_test(unittest.TestCase):
    # Input and output names
    
    projectname="test_simplecluster"
    resultfile="test_simplecluster.result"
    clusterfile="test_simplecluster"
    
    cluster=simple_cluster()

    def _cleanUp(self):
        if os.path.exists(self.resultfile):
            os.remove(self.resultfile)
        logfiles=glob.glob("engine-*.log")
        for i in logfiles:
            os.remove(i)
        if os.path.exists(self.clusterfile):
            os.remove(self.clusterfile)

    def setUp(self):
        self._cleanUp()

    def tearDown(self):
        #print 'tearDown.....get called when each test finish'
        self.cluster.cold_start()
        self._cleanUp()

    def test000(self):
        '''Test 0: create a default cluster'''
        host=os.uname()[1]
        cwd=os.getcwd()
        import multiprocessing
        ncpu=multiprocessing.cpu_count()
        msg=host+', '+str(ncpu)+', '+cwd
        f=open(self.clusterfile, 'w')
        f.write(msg)
        f.close()
        self._waitForFile(self.clusterfile, 10)

        self.cluster.init_cluster(self.clusterfile, self.projectname)

        print self.cluster._cluster.hello()
        print "engines:", self.cluster.use_engines()
        print "hosts:",  self.cluster.get_hosts()

    def _checkResultFile(self):
        self.assertTrue(os.path.isfile(self.resultfile))
            
    def _waitForFile(self, file, seconds):
        for i in range(0,seconds):
            if (os.path.isfile(file)):
                return
            time.sleep(1)

class testJobData(unittest.TestCase):
    '''
    This class tests the JobData class in the simple_cluster.
    '''

    def testSimpleJob(self):
        jd = JobData('myJob')
        self.assertEqual('returnVar0 = myJob()', jd.getCommandLine())
        self.assertEqual(['returnVar0'], jd.getReturnVariableList())

    def testJobWithOneArg(self):
        jd = JobData('myJob',{'arg1':1})
        self.assertEqual('returnVar0 = myJob(arg1 = 1)', jd.getCommandLine())
        self.assertEqual(['returnVar0'], jd.getReturnVariableList())

    def testJobWithMultipleArg(self):
        jd = JobData('myJob',{'arg1':1,'arg2':2,'arg3':'three'})
        self.assertEqual\
             ("returnVar0 = myJob(arg1 = 1, arg2 = 2, arg3 = 'three')",
              jd.getCommandLine())
        self.assertEqual(['returnVar0'], jd.getReturnVariableList())

    def testJobWithMultipleCommands(self):
        jd = JobData('myJob1')
        jd.addCommand('myJob2',{'arg1':1})
        self.assertEqual('returnVar0 = myJob1(); '+\
                         'returnVar1 = myJob2(arg1 = 1)',
                         jd.getCommandLine())
        self.assertEqual(['returnVar0', 'returnVar1'],
                         jd.getReturnVariableList())

    def testGetCommandArguments(self):
        # Test with a single command
        jd = JobData('myCommand1',{'arg1':1})
        resp = jd.getCommandArguments()
        self.assertTrue(isinstance(resp,dict))
        self.assertEqual(len(resp),1)
        self.assertEqual(resp['arg1'],1)

        # Now handle the cases of multiple commands
        jd.addCommand('myCommand2',{'arg2':2})
        resp = jd.getCommandArguments()
        self.assertTrue(isinstance(resp,dict))
        self.assertEqual(len(resp),2)
        self.assertTrue(isinstance(resp['myCommand1'],dict))
        self.assertTrue(isinstance(resp['myCommand2'],dict))
        self.assertEqual(resp['myCommand1']['arg1'],1)
        self.assertEqual(resp['myCommand2']['arg2'],2)

        # Now get just a single commands response
        resp = jd.getCommandArguments('myCommand1')
        self.assertTrue(isinstance(resp,dict))
        self.assertEqual(len(resp),1)
        self.assertEqual(resp['arg1'],1)


class testJobQueueManager(unittest.TestCase):
    '''
    This class tests the Job Queue Manager.
    '''
    def __init__(self, methodName = 'runTest'):
        self.setUpCluster()
        unittest.TestCase.__init__(self, methodName)

    
    def testJobExecutionSingleReturn(self):
        # Test a single job with a single return
        queue = JobQueueManager()
        queue.addJob(JobData('echoFunction', {'input':'inputVar'}))
        queue.executeQueue()

        jobList = queue.getOutputJobs('done')
        self.assertEqual(len(jobList),1)
        self.assertEqual(jobList[0].getReturnValues(),'inputVar')
        
        queue.clearJobs()
        for idx in xrange(16):
            queue.addJob(JobData('echoFunction',
                                 {'input':'inputVar%d' % idx}))
        queue.executeQueue()

        jobList = queue.getOutputJobs('done')
        self.assertEqual(len(jobList),16)
        for job in jobList:
            self.assertEqual(job.getReturnValues(),
                             job._commandList[0].commandInfo['input'])

    def testJobExectionWithoutReturn(self):
        queue = JobQueueManager()
        queue.addJob(JobData('noReturn'))
        queue.executeQueue()

        jobList = queue.getOutputJobs('done')
        self.assertEqual(len(jobList),1)
        self.assertEqual(jobList[0].getReturnValues(),None)

        # Test a mixed return case
        queue.clearJobs()
        for idx in xrange(16):
            if idx % 2:
                queue.addJob(JobData('echoFunction',
                                     {'input':'inputVar%d' % idx}))
            else:
                queue.addJob(JobData('noReturn'))
        queue.executeQueue()

        jobList = queue.getOutputJobs('done')
        self.assertEqual(len(jobList),16)
        for job in jobList:
            if job._commandList[0].commandName == 'echoFunction':
                self.assertEqual(job.getReturnValues(),
                                 job._commandList[0].commandInfo['input'])
            else:
                self.assertEqual(job.getReturnValues(), None)
  
    def testFailedJobs(self):
        queue = JobQueueManager()
        queue.addJob(JobData('setErrror',{'setError':True,
                                          'returnValue':1}))
        queue.executeQueue()

        jobList = queue.getOutputJobs()
        self.assertEqual(len(jobList),1)
        self.assertEqual(jobList[0].status,'broken')

        # Test a mixed return case
        queue.clearJobs()
        for idx in xrange(16):
            queue.addJob(JobData('setError', {'setError': idx %2,
                                              'returnValue': idx}))
        queue.executeQueue()
        jobList = queue.getOutputJobs('done')
        self.assertEqual(len(jobList),8)

        jobList = queue.getOutputJobs()
        self.assertEqual(len(jobList),16)
        for job in jobList:
            if job._commandList[0].commandInfo['setError']:
                self.assertEqual(job.getReturnValues(), None)
                self.assertEqual(job.status, 'broken')
            else:
                self.assertEqual(job.getReturnValues(),
                         job._commandList[0].commandInfo['returnValue'])
                self.assertEqual(job.status, 'done')

          
    def setUpCluster(self):
        '''
        This method defines three methods on the cluster:
        echoFunction - returns whatever is sent
        noReturn - has no return value
        errorMethod - which will raise an exception if the argument is true
        '''
        cluster = simple_cluster.getCluster()
            
        command = '''
        def echoFunction(input = ''):
            return input
        '''
        for engine in cluster.use_engines():
            cluster.do_and_record(command, engine)


        command = '''
        def noReturn():
            pass
        '''
        
        for engine in cluster.use_engines():
            cluster.do_and_record(command, engine)

        command = '''
        def setError(setError, returnValue):
            if setError:
                raise Exception, "Error Condition"
            return returnValue
        '''

        for engine in cluster.use_engines():
            cluster.do_and_record(command, engine)

        # Wait for all of the engines to return
        counter = 30
        while counter > 0 and (len(cluster.get_status(True)) <
                               len(cluster.use_engines())):
            time.sleep(1)
            counter -= 1
        cluster.remove_record()

        


def suite():
    return [testJobData]
    return [simplecluster_test, testJobData, testJobQueueManager]
     
if __name__ == '__main__':
    testSuite = []
    for testClass in suite():
        testSuite.append(unittest.makeSuite(testClass,'test'))
    allTests = unittest.TestSuite(testSuite)
    unittest.TextTestRunner(verbosity=2).run(allTests)
