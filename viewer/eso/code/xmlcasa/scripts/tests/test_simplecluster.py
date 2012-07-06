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
import multiprocessing

class simplecluster_test(unittest.TestCase):

    projectname="test_simplecluster"
    resultfile="test_simplecluster.result"
    clusterfile="test_simplecluster_config.txt"
    monitorFile="monitoring.log"
    cluster=None

    # Get local host configuration parameters
    host=os.uname()[1]
    cwd=os.getcwd()
    ncpu=multiprocessing.cpu_count()

    def cleanUp(self):
        if os.path.exists(self.resultfile):
            os.remove(self.resultfile)
        logfiles=glob.glob("engine-*.log")
        for i in logfiles:
            os.remove(i)
        if os.path.exists(self.clusterfile):
            os.remove(self.clusterfile)
        if os.path.exists(self.monitorFile):
            os.remove(self.monitorFile)

    def stopCluster(self):
        # Stop cluster and thread services
        self.cluster.stop_monitor()
        self.cluster.stop_resource()
        # Need to wait until the threads have actually returned
        time.sleep(10)
        # Now we can stop the cluster w/o problems
        self.cluster.cold_start()
        # Remove log files, cluster files, and result files
        self.cleanUp()

    def createClusterFile(self):

        msg=self.host + ', ' + str(self.ncpu) + ', ' + self.cwd
        f=open(self.clusterfile, 'w')
        f.write(msg)
        f.close()
        self._waitForFile(self.clusterfile, 10)

    def create_input(self,str_text, filename):
        """Save the string in a text file"""
    
        inp = filename
        cmd = str_text
    
        # remove file first
        if os.path.exists(inp):
            os.system('rm -f '+ inp)
        
        # save to a file    
        fid = open(inp, 'w')
        fid.write(cmd)
        
        # close file
        fid.close()

        # wait until file is visible for the filesystem
        self._waitForFile(filename, 10)
    
        return

    def _waitForFile(self, file, seconds):
        for i in range(0,seconds):
            if (os.path.isfile(file)):
                return
            time.sleep(1)

    def initCluster(self,userMonitorFile=""):
        # First of all clean up files from previous sessions
        self.cleanUp()
        # Create cluster object
        if (len(userMonitorFile) > 0):
            self.cluster = simple_cluster(userMonitorFile)
        else:
            self.cluster = simple_cluster()
        # Create cluster file
        self.createClusterFile()
        # Initialize cluster object
        self.cluster.init_cluster(self.clusterfile, self.projectname)
        # Wait 10 seconds to stabilize
        time.sleep(10)

    def tearDown(self):
        # Remove log files, cluster files, and result files
        self.stopCluster()

    def setUp_4Ants_partition(self):
        self.vis = "Four_ants_3C286.partition.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                         os.environ.get('CASAPATH').split()[0] +
                        "/data/regression/unittest/simplecluster/" + self.vis + ' ' + self.vis)
            os.system('cp -r ' + \
                         os.environ.get('CASAPATH').split()[0] +
                        "/data/regression/unittest/simplecluster/" + self.vis.replace('.ms','.data') + ' ' + self.vis.replace('.ms','.data'))

    def test1_defaultCluster(self):
        """Test 1: Create a default cluster"""

        self.initCluster()

        # Check parameters vs actual cluster configuration
        cluster_list = self.cluster.get_hosts()
        self.assertTrue(cluster_list[0][0]==self.host)
        self.assertTrue(cluster_list[0][1]==self.ncpu)
        self.assertTrue(cluster_list[0][2]==self.cwd)

        self.stopCluster()

    def test2_monitoringDefault(self):
        """Test 2: Check default monitoring file exists"""

        self.initCluster()

        fid = open('monitoring.log', 'r')
        line = fid.readline()
        self.assertTrue(line.find('Host')>=0)
        self.assertTrue(line.find('Engine')>=0)
        self.assertTrue(line.find('Status')>=0)
        self.assertTrue(line.find('CPU[%]')>=0)
        self.assertTrue(line.find('Memory[%]')>=0)
        self.assertTrue(line.find('Time[s]')>=0)
        self.assertTrue(line.find('Read[MB]')>=0)
        self.assertTrue(line.find('Write[MB]')>=0)
        self.assertTrue(line.find('Read[MB/s]')>=0)
        self.assertTrue(line.find('Write[MB/s]')>=0)
        self.assertTrue(line.find('Job')>=0)
        self.assertTrue(line.find('Sub-MS')>=0)

        self.stopCluster()

    def test3_monitoringUser(self):
        """Test 3: Check custom monitoring file exists"""

        self.initCluster('userMonitorFile.log')

        fid = open('userMonitorFile.log', 'r')
        line = fid.readline()
        self.assertTrue(line.find('Host')>=0)
        self.assertTrue(line.find('Engine')>=0)
        self.assertTrue(line.find('Status')>=0)
        self.assertTrue(line.find('CPU[%]')>=0)
        self.assertTrue(line.find('Memory[%]')>=0)
        self.assertTrue(line.find('Time[s]')>=0)
        self.assertTrue(line.find('Read[MB]')>=0)
        self.assertTrue(line.find('Write[MB]')>=0)
        self.assertTrue(line.find('Read[MB/s]')>=0)
        self.assertTrue(line.find('Write[MB/s]')>=0)
        self.assertTrue(line.find('Job')>=0)
        self.assertTrue(line.find('Sub-MS')>=0)

        self.stopCluster()

    def test4_monitoringStandAlone(self):
        """Test 4: Check the dict structure of the stand-alone method """

        self.initCluster('userMonitorFile.log')

        time.sleep(10)
        state = self.cluster.show_state()
        for engine in range(self.ncpu):
            self.assertTrue(state[self.host][engine].has_key('Status'))
            self.assertTrue(state[self.host][engine].has_key('Sub-MS'))
            self.assertTrue(state[self.host][engine].has_key('Read'))
            self.assertTrue(state[self.host][engine].has_key('Write'))
            self.assertTrue(state[self.host][engine].has_key('Job'))
            self.assertTrue(state[self.host][engine].has_key('Memory'))
            self.assertTrue(state[self.host][engine].has_key('ReadRate'))
            self.assertTrue(state[self.host][engine].has_key('WriteRate'))

        self.stopCluster()

    def test5_tflagdata_list_return(self):
        """Test 5: Test support for MMS using tflagdata in unflag+clip mode"""

        self.setUp_4Ants_partition()
        self.initCluster()

        # Create list file
        text = "mode='unflag'\n"\
               "mode='clip' clipminmax=[0,0.1]"
        filename = 'list_tflagdata.txt'
        self.create_input(text, self.projectname + '/' + filename)

        # step 1: Do unflag+clip
        tflagdata(vis=self.vis, mode='list', inpfile=filename)

        # step 2: Now do summary
        summary = tflagdata(vis=self.vis, mode='summary')

        # Print summary (note: the first 16 jobs correspond to the step 1)
        self.assertTrue(summary[0]['spw']['15']['flagged'] == 96284.0)
        self.assertTrue(summary[1]['spw']['0']['flagged'] == 129711.0)
        self.assertTrue(summary[2]['spw']['1']['flagged'] == 128551.0)
        self.assertTrue(summary[3]['spw']['2']['flagged'] == 125686.0)
        self.assertTrue(summary[4]['spw']['3']['flagged'] == 122862.0)
        self.assertTrue(summary[5]['spw']['4']['flagged'] == 109317.0)
        self.assertTrue(summary[6]['spw']['5']['flagged'] == 24481.0)
        self.assertTrue(summary[7]['spw']['6']['flagged'] == 0)
        self.assertTrue(summary[8]['spw']['7']['flagged'] == 0)
        self.assertTrue(summary[9]['spw']['8']['flagged'] == 0)
        self.assertTrue(summary[10]['spw']['9']['flagged'] == 27422.0)
        self.assertTrue(summary[11]['spw']['10']['flagged'] == 124638.0)
        self.assertTrue(summary[12]['spw']['11']['flagged'] == 137813.0)
        self.assertTrue(summary[13]['spw']['12']['flagged'] == 131896.0)
        self.assertTrue(summary[14]['spw']['13']['flagged'] == 125074.0)
        self.assertTrue(summary[15]['spw']['14']['flagged'] == 118039.0)

        self.stopCluster()


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

class testMonitoring(unittest.TestCase):
    """
    This class tests the monitoring functionality
    """

    def setUp(self):
       self.setUp_4Ants_partition()

    def setUp_4Ants_partition(self):
       self.vis = "Four_ants_3C286.partition.ms"

       if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
       else:
            print "Moving data..."
            os.system('cp -r ' + \
                         os.environ.get('CASAPATH').split()[0] +
                        "/data/regression/unittest/simplecluster/" + self.vis + ' ' + self.vis)
            os.system('cp -r ' + \
                         os.environ.get('CASAPATH').split()[0] +
                        "/data/regression/unittest/simplecluster/" + self.vis.replace('.ms','.data') + ' ' + self.vis.replace('.ms','.data'))

       self.unflag_table()

    def unflag_table(self):
       tflagdata(vis=self.vis,mode='unflag')

    def tearDown(self):
       os.system('rm -rf ' + self.vis)
       os.system('rm -rf ' + self.vis.replace('.ms','.data'))
       sc = simple_cluster.getCluster()
       sc.stop_monitor()
       sc.stop_resource()
       sc.cold_start()
       # Need to wait until the threads have actually returned
       time.sleep(10)
    
    def testMonitoringDefault(self):
       tflagdata(vis=self.vis,mode='clip')
       fid = open('monitoring.log', 'r')
       line = fid.readline()
       self.assertTrue(line.find('Host')>=0)
       self.assertTrue(line.find('Engine')>=0)
       self.assertTrue(line.find('Status')>=0)
       self.assertTrue(line.find('CPU[%]')>=0)
       self.assertTrue(line.find('Memory[%]')>=0)
       self.assertTrue(line.find('Time[s]')>=0)
       self.assertTrue(line.find('Read[MB]')>=0)
       self.assertTrue(line.find('Write[MB]')>=0)
       self.assertTrue(line.find('Read[MB/s]')>=0)
       self.assertTrue(line.find('Write[MB/s]')>=0)
       self.assertTrue(line.find('Job')>=0)
       self.assertTrue(line.find('Sub-MS')>=0)


def suite():
    return [simplecluster_test]
    return [simplecluster_test, testJobData, testJobQueueManager]
     
if __name__ == '__main__':
    testSuite = []
    for testClass in suite():
        testSuite.append(unittest.makeSuite(testClass,'test'))
    allTests = unittest.TestSuite(testSuite)
    unittest.TextTestRunner(verbosity=2).run(allTests)
