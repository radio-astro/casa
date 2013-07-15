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
import testhelper
from parallel.parallel_task_helper import ParallelTaskHelper

class test_simplecluster(unittest.TestCase):

    projectname="test_simplecluster"
    clusterfile="test_simplecluster_config.txt"
    monitorFile="monitoring.log"
    cluster=None

    # Get local host configuration parameters
    host=os.uname()[1]
    cwd=os.getcwd()
    ncpu=multiprocessing.cpu_count()
    
    # jagonzal (CAS-4287): Add a cluster-less mode to by-pass parallel processing for MMSs as requested 
    if os.environ.has_key('BYPASS_SEQUENTIAL_PROCESSING'):
        ParallelTaskHelper.bypassParallelProcessing(1)
        bypassParallelProcessing = True
    else:
        bypassParallelProcessing = False
        
    vis = ""
    ref = ""
    aux = ""

    def stopCluster(self):
        # Stop thread services and cluster
        self.cluster.stop_cluster()
        # Remove log files, cluster files, and monitoring files
        self.cleanUp()

    def cleanUp(self):
        logfiles=glob.glob("engine-*.log")
        for i in logfiles:
            os.remove(i)
        if os.path.exists(self.clusterfile):
            os.remove(self.clusterfile)
        if os.path.exists(self.monitorFile):
            os.remove(self.monitorFile)

    def initCluster(self,userMonitorFile="",max_engines=2,max_memory=0.,memory_per_engine=512.):
        # First of all clean up files from previous sessions
        self.cleanUp()
        # Create cluster object
        if (len(userMonitorFile) > 0):
            self.cluster = simple_cluster(userMonitorFile)
            self.monitorFile = userMonitorFile
        else:
            self.cluster = simple_cluster()
            self.monitorFile = "monitoring.log"
        # Create cluster configuration file
        self.createClusterFile(max_engines,max_memory,memory_per_engine)
        # Initialize cluster object
        if (self.cluster.init_cluster(self.clusterfile, self.projectname)):
            self.cluster.check_resource()
            # Wait unit cluster is producing monitoring info
            if (len(userMonitorFile) > 0):
                self.waitForFile(userMonitorFile, 20)
            else:
                self.waitForFile('monitoring.log', 20)

    def createClusterFile(self,max_engines=0.,max_memory=0.,memory_per_engine=512.):
            
        if (max_engines>1):
            max_memory = 512*max_engines
            msg=self.host + ', ' + str(max_engines) + ', ' + self.cwd + ', ' + str(max_memory)
        else:
            msg=self.host + ', ' + str(max_engines) + ', ' + self.cwd + ', ' + str(max_memory) + ', ' + str(memory_per_engine) 
        f=open(self.clusterfile, 'w')
        f.write(msg)
        f.close()
        self.waitForFile(self.clusterfile, 10)

    def waitForFile(self, file, seconds):
        for i in range(0,seconds):
            if (os.path.isfile(file)):
                return
            time.sleep(1)
            
    def setUpFile(self,file,type_file):
        
        if type(file) is list:
            for file_i in file:
                self.setUpFileCore(file_i,type_file)
        else:
            self.setUpFileCore(file,type_file)
                
        if type_file=='vis':
            self.vis = file
        elif type_file =='ref':
            self.ref = file
        elif type_file=='aux':
            self.aux = file
                
    def setUpFileCore(self,file,type_file):
        
        if os.path.exists(file):
             print "%s file %s is already in the working area, deleting ..." % (type_file,file)
             os.system('rm -rf ' + file)
        print "Copy %s file %s into the working area..." % (type_file,file)
        os.system('cp -R ' + os.environ.get('CASAPATH').split()[0] +
                  '/data/regression/unittest/simplecluster/' + file + ' ' + file)
    
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
        self.waitForFile(filename, 10)
    
        return

    def test1_defaultCluster(self):
        """Test 1: Create a default cluster"""

        # Create cluster file
        self.initCluster(max_engines=0.)

        cluster_list = self.cluster.get_hosts()
        self.assertTrue(cluster_list[0][0]==self.host)
        self.assertTrue(cluster_list[0][2]==self.cwd)

        self.stopCluster()
        
    def test2_availableResourcesCluster(self):
        """Test 2: Create a custom cluster to use all the available resources"""

        # Create cluster file
        max_memory_local = self.ncpu*1024
        self.initCluster(max_engines=1.,max_memory=max_memory_local,memory_per_engine=1024.)

        cluster_list = self.cluster.get_hosts()
        self.assertTrue(cluster_list[0][0]==self.host)
        self.assertTrue(cluster_list[0][1]==self.ncpu)
        self.assertTrue(cluster_list[0][2]==self.cwd)

        self.stopCluster()        
        
    def test3_halfCPUCluster(self):
        """Test 3: Create a custom cluster to use half of available CPU capacity"""

        # Create cluster file
        max_memory_local = self.ncpu*512
        self.initCluster(max_engines=0.5,max_memory=max_memory_local,memory_per_engine=512.)

        cluster_list = self.cluster.get_hosts()
        self.assertTrue(cluster_list[0][0]==self.host)
        self.assertTrue(cluster_list[0][2]==self.cwd)

        self.stopCluster()    
        
    def test3_halfMemoryCluster(self):
        """Test 3: Create a custom cluster to use half of available RAM memory"""

        # Create cluster file
        self.initCluster(max_engines=self.ncpu,max_memory=0.5,memory_per_engine=128.)

        cluster_list = self.cluster.get_hosts()
        self.assertTrue(cluster_list[0][0]==self.host)
        self.assertTrue(cluster_list[0][2]==self.cwd)

        self.stopCluster()                

    def test4_monitoringDefault(self):
        """Test 4: Check default monitoring file exists"""
        
        # Create cluster file
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

    def test5_monitoringUser(self):
        """Test 5: Check custom monitoring file exists"""
        
        # Create cluster file
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

    def test6_monitoringStandAlone(self):
        """Test 6: Check the dict structure of the stand-alone method """
        
        # Create cluster file
        self.initCluster('userMonitorFile.log')
                
        state = self.cluster.show_resource(True)
        cluster_list = self.cluster.get_hosts()
        for engine in range(cluster_list[0][1]):
            self.assertTrue(state[self.host][engine].has_key('Status'))
            self.assertTrue(state[self.host][engine].has_key('Sub-MS'))
            self.assertTrue(state[self.host][engine].has_key('Read'))
            self.assertTrue(state[self.host][engine].has_key('Write'))
            self.assertTrue(state[self.host][engine].has_key('Job'))
            self.assertTrue(state[self.host][engine].has_key('Memory'))
            self.assertTrue(state[self.host][engine].has_key('ReadRate'))
            self.assertTrue(state[self.host][engine].has_key('WriteRate'))

        self.stopCluster()
        
    def test7_bypassParallelProcessing(self):
        """Test 7: Bypass Parallel Processing mode """        
               
        simple_cluster.setDefaults(default_mem_per_engine=33554432)
        
        # Prepare MMS
        self.setUpFile("Four_ants_3C286.mms",'vis')
        
        # Create list file
        text = "mode='unflag'\n"\
               "mode='clip' clipminmax=[0,0.1]"
        filename = 'list_flagdata.txt'
        self.create_input(text, filename)

        # step 1: Do unflag+clip
        flagdata(vis=self.vis, mode='list', inpfile=filename)

        # step 2: Now do summary
        ret_dict = flagdata(vis=self.vis, mode='summary')

        # Check summary
        self.assertTrue(ret_dict['name']=='Summary')
        self.assertTrue(ret_dict['spw']['15']['flagged'] == 96284.0)
        self.assertTrue(ret_dict['spw']['0']['flagged'] == 129711.0)
        self.assertTrue(ret_dict['spw']['1']['flagged'] == 128551.0)
        self.assertTrue(ret_dict['spw']['2']['flagged'] == 125686.0)
        self.assertTrue(ret_dict['spw']['3']['flagged'] == 122862.0)
        self.assertTrue(ret_dict['spw']['4']['flagged'] == 109317.0)
        self.assertTrue(ret_dict['spw']['5']['flagged'] == 24481.0)
        self.assertTrue(ret_dict['spw']['6']['flagged'] == 0)
        self.assertTrue(ret_dict['spw']['7']['flagged'] == 0)
        self.assertTrue(ret_dict['spw']['8']['flagged'] == 0)
        self.assertTrue(ret_dict['spw']['9']['flagged'] == 27422.0)
        self.assertTrue(ret_dict['spw']['10']['flagged'] == 124638.0)
        self.assertTrue(ret_dict['spw']['11']['flagged'] == 137813.0)
        self.assertTrue(ret_dict['spw']['12']['flagged'] == 131896.0)
        self.assertTrue(ret_dict['spw']['13']['flagged'] == 125074.0)
        self.assertTrue(ret_dict['spw']['14']['flagged'] == 118039.0)    
        
        # Remove MMS
        os.system('rm -rf ' + self.vis)        
        
        # Restore default values
        simple_cluster.setDefaults(default_mem_per_engine=512)
        if not self.bypassParallelProcessing:
            ParallelTaskHelper.bypassParallelProcessing(0)
            
    def test8_IgnoreNullSelectionError(self):
        """Test 8: Check that NullSelection errors happening for some sub-MSs are ignored  """
        """Note: In this test we also check simple_cluster initialization via ParallelTaskHelper  """
        
        # Prepare MMS
        self.setUpFile("Four_ants_3C286.mms",'vis')          
        
        # Unflag entire MMS
        flagdata(vis=self.vis, mode='unflag')
        
        # Manually flag scan 30
        flagdata(vis=self.vis, mode='manual', scan='30')

        # step 2: Now do summary
        ret_dict = flagdata(vis=self.vis, mode='summary')

        # Check summary
        self.assertTrue(ret_dict['scan']['30']['flagged'] == 2187264.0)
        self.assertTrue(ret_dict['scan']['31']['flagged'] == 0)  
        
        # Stop cluster if it was started
        self.cluster = simple_cluster.getCluster()
        if (self.cluster != None):
            self.stopCluster()
            
        # Remove MMS
        os.system('rm -rf ' + self.vis)
        


class test_flagdata_mms(test_simplecluster):

    def setUp(self):
        # Prepare MMS
        self.setUpFile("Four_ants_3C286.mms",'vis')
        
        # Startup cluster
        if not self.bypassParallelProcessing:            
            self.initCluster()

    def tearDown(self):
        # Stop cluster
        if not self.bypassParallelProcessing:
            self.stopCluster()
        # Remove MMS
        os.system('rm -rf ' + self.vis)
    
    def test1_flagdata_list_return(self):
        """Test 1: Test support for MMS using flagdata in unflag+clip mode"""

        # Create list file
        text = "mode='unflag'\n"\
               "mode='clip' clipminmax=[0,0.1]"
        filename = 'list_flagdata.txt'
        self.create_input(text, filename)

        # step 1: Do unflag+clip
        flagdata(vis=self.vis, mode='list', inpfile=filename)

        # step 2: Now do summary
        ret_dict = flagdata(vis=self.vis, mode='summary')

        # Check summary
        self.assertTrue(ret_dict['name']=='Summary')
        self.assertTrue(ret_dict['spw']['15']['flagged'] == 96284.0)
        self.assertTrue(ret_dict['spw']['0']['flagged'] == 129711.0)
        self.assertTrue(ret_dict['spw']['1']['flagged'] == 128551.0)
        self.assertTrue(ret_dict['spw']['2']['flagged'] == 125686.0)
        self.assertTrue(ret_dict['spw']['3']['flagged'] == 122862.0)
        self.assertTrue(ret_dict['spw']['4']['flagged'] == 109317.0)
        self.assertTrue(ret_dict['spw']['5']['flagged'] == 24481.0)
        self.assertTrue(ret_dict['spw']['6']['flagged'] == 0)
        self.assertTrue(ret_dict['spw']['7']['flagged'] == 0)
        self.assertTrue(ret_dict['spw']['8']['flagged'] == 0)
        self.assertTrue(ret_dict['spw']['9']['flagged'] == 27422.0)
        self.assertTrue(ret_dict['spw']['10']['flagged'] == 124638.0)
        self.assertTrue(ret_dict['spw']['11']['flagged'] == 137813.0)
        self.assertTrue(ret_dict['spw']['12']['flagged'] == 131896.0)
        self.assertTrue(ret_dict['spw']['13']['flagged'] == 125074.0)
        self.assertTrue(ret_dict['spw']['14']['flagged'] == 118039.0)


class test_setjy_mms(test_simplecluster):

    def setUp(self):
        # Prepare MMS
        self.setUpFile("ngc5921.applycal.mms",'vis')
        
        # Startup cluster
        if not self.bypassParallelProcessing:            
            self.initCluster()

    def tearDown(self):
        # Stop cluster
        if not self.bypassParallelProcessing:
            self.stopCluster()
        # Remove MMS
        os.system('rm -rf ' + self.vis)

    def test1_setjy_scratchless_mode_single_model(self):
        """Test 1: Set vis model header in one single field """

        retval = setjy(vis=self.vis, field='1331+305*',fluxdensity=[1331.,0.,0.,0.], scalebychan=False, usescratch=False)
        self.assertTrue(retval, "setjy run failed")    
        
        mslocal = mstool()
        mslocal.open(self.vis)
        listSubMSs = mslocal.getreferencedtables()
        mslocal.close()
        listSubMSs.append(self.vis)
        for subMS in listSubMSs:
            tblocal = tbtool()
            tblocal.open(subMS + '/SOURCE')
            nrows = tblocal.nrows()
            for row_i in range(0,nrows):
                try:
                    model_i = tblocal.getcell('SOURCE_MODEL',row_i)
                    if (row_i == 0):
                        self.assertEqual(model_i['cl_0']['fields'][0],row_i)
                        self.assertEqual(model_i['cl_0']['container']['component0']['flux']['value'][0],1331.)
                    else:
                        self.assertEqual(len(model_i),0)
                except:
                    print "Problem accesing SOURCE_MODEL col from subMS " + subMS 
            tblocal.close()
            
    def test2_setjy_scratchless_mode_multiple_model(self):
        """Test 2: Set vis model header in one multiple fields """

        retval = setjy(vis=self.vis, field='1331+305*',fluxdensity=[1331.,0.,0.,0.], scalebychan=False, usescratch=False)
        self.assertTrue(retval, "setjy run failed")
        retval = setjy(vis=self.vis, field='1445+099*',fluxdensity=[1445.,0.,0.,0.], scalebychan=False, usescratch=False)
        self.assertTrue(retval, "setjy run failed")
                   
        mslocal = mstool()
        mslocal.open(self.vis)
        listSubMSs = mslocal.getreferencedtables()
        mslocal.close()
        listSubMSs.append(self.vis)
        for subMS in listSubMSs:
            tblocal = tbtool()
            tblocal.open(subMS + '/SOURCE')
            nrows = tblocal.nrows()
            for row_i in range(0,nrows):
                model_i = tblocal.getcell('SOURCE_MODEL',row_i)
                if (row_i == 0):
                    self.assertEqual(model_i['cl_0']['fields'][0],row_i)
                    self.assertEqual(model_i['cl_0']['container']['component0']['flux']['value'][0],1331.)
                elif (row_i == 1):
                    self.assertEqual(model_i['cl_0']['fields'][0],row_i)
                    self.assertEqual(model_i['cl_0']['container']['component0']['flux']['value'][0],1445.)                    
                else:
                    self.assertEqual(len(model_i),0)
            tblocal.close()            
            
    def test3_setjy_scratch_mode_single_model(self):
        """Test 3: Set MODEL_DATA in one single field"""

        retval = setjy(vis=self.vis, field='1331+305*',fluxdensity=[1331.,0.,0.,0.], scalebychan=False,usescratch=True)
        self.assertTrue(retval, "setjy run failed")
        
        mslocal = mstool()
        mslocal.open(self.vis)
        listSubMSs = mslocal.getreferencedtables()
        mslocal.close()
        for subMS in listSubMSs:
            tblocal = tbtool()
            tblocal.open(subMS)
            fieldId = tblocal.getcell('FIELD_ID',1)
            if (fieldId == 0):
                self.assertEqual(tblocal.getcell('MODEL_DATA',1)[0][0].real,1331.0)
            elif (fieldId == 1):
                self.assertEqual(tblocal.getcell('MODEL_DATA',1)[0][0].real,1.0)
            elif (fieldId == 2):
                self.assertEqual(tblocal.getcell('MODEL_DATA',1)[0][0].real,1.0)
            else:
                raise AssertionError, "Unrecognized field [%s] found in Sub-MS [%s]" %(str(fieldId),subMS)
                tblocal.close()
            tblocal.close()

    def test4_setjy_scratch_mode_multiple_model(self):
        """Test 4: Set MODEL_DATA in multiple fields"""

        retval = setjy(vis=self.vis, field='1331+305*',fluxdensity=[1331.,0.,0.,0.], scalebychan=False, usescratch=True)
        self.assertTrue(retval, "setjy run failed")
        retval = setjy(vis=self.vis, field='1445+099*',fluxdensity=[1445.,0.,0.,0.], scalebychan=False, usescratch=True)
        self.assertTrue(retval, "setjy run failed")
        
        mslocal = mstool()
        mslocal.open(self.vis)
        listSubMSs = mslocal.getreferencedtables()
        mslocal.close()
        for subMS in listSubMSs:
            tblocal = tbtool()
            tblocal.open(subMS)
            fieldId = tblocal.getcell('FIELD_ID',1)
            if (fieldId == 0):
                self.assertEqual(tblocal.getcell('MODEL_DATA',1)[0][0].real,1331.0)
            elif (fieldId == 1):
                self.assertEqual(tblocal.getcell('MODEL_DATA',1)[0][0].real,1445.0)
            elif (fieldId == 2):
                self.assertEqual(tblocal.getcell('MODEL_DATA',1)[0][0].real,1.0)
            else:
                raise AssertionError, "Unrecognized field [%s] found in Sub-MS [%s]" %(str(fieldId),subMS)
                tblocal.close()
            tblocal.close()
            
class test_applycal_mms(test_simplecluster):

    def setUp(self):
        # Set-up MMS
        self.setUpFile("ngc5921.applycal.mms",'vis')
        # Set-up reference MMS
        self.setUpFile("ngc5921.split.mms",'ref')
        # Set-up auxiliary files
        self.setUpFile(["ngc5921.fluxscale", "ngc5921.gcal", "ngc5921.bcal"],'aux')
        
        # Startup cluster
        if not self.bypassParallelProcessing:            
            self.initCluster()

    def tearDown(self):
        # Stop cluster
        if not self.bypassParallelProcessing:
            self.stopCluster()
        # Remove MMS
        os.system('rm -rf ' + self.vis) 
        # Remove ref MMS
        os.system('rm -rf ' + self.ref) 
        # Remove aux files
        for file in self.aux:
            os.system('rm -rf ' + file)         
        
    def test1_applycal_fluxscale_gcal_bcal(self):
        """Test 1: Apply calibration using fluxscal gcal and bcal tables"""

        # Repository caltables are pre-v4.1, and we
        # must update them _before_ applycal to avoid contention
        print 'Updating pre-v4.1 caltables, FIRST:'
        cblocal = cbtool()
        for oldct in self.aux:
            cblocal.updatecaltable(oldct)
        print 'Now it is safe to run applycal.'

        applycal(vis=self.vis,field='',spw='',selectdata=False,gaintable=self.aux,
                 gainfield=['nearest','nearest','0'],
                 interp=['linear', 'linear','nearest'],spwmap=[])
        
        compare = testhelper.compTables(self.ref,self.vis,['FLAG_CATEGORY'])
        
        self.assertTrue(compare)        
        
        
class test_uvcont_mms(test_simplecluster):

    def setUp(self):           
        # Set-up MMS
        self.setUpFile("ngc5921.uvcont.mms",'vis')
        # Set-up reference MMS
        self.setUpFile(["ngc5921.mms.cont", "ngc5921.mms.contsub"],'ref')      
        
        # Startup cluster
        if not self.bypassParallelProcessing:            
            self.initCluster()

    def tearDown(self):
        # Stop cluster
        if not self.bypassParallelProcessing:
            self.stopCluster()
        # Remove MMS
        os.system('rm -rf ' + self.vis)
        # Remove ref MMS
        for file in self.ref:
            os.system('rm -rf ' + file) 
        
    def test1_uvcont_single_spw(self):
        """Test 1: Extract continuum from one single SPW using uvcontsub"""

        uvcontsub(vis=self.vis,field = 'N5921*',fitspw='0:4~6;50~59',spw = '0',solint = 'int',fitorder = 0,want_cont = True) 
        
        compare_cont = testhelper.compTables(self.ref[0],self.vis+".cont",['FLAG_CATEGORY'])
        self.assertTrue(compare_cont)
        compare_contsub = testhelper.compTables(self.ref[1],self.vis+".contsub",['FLAG_CATEGORY'])
        self.assertTrue(compare_contsub)             

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
    return [test_simplecluster,test_flagdata_mms,test_setjy_mms,test_applycal_mms,test_uvcont_mms]
     
if __name__ == '__main__':
    testSuite = []
    for testClass in suite():
        testSuite.append(unittest.makeSuite(testClass,'test'))
    allTests = unittest.TestSuite(testSuite)
    unittest.TextTestRunner(verbosity=2).run(allTests)
