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
        if (sys.platform=='darwin'):
            pass
        else:
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

def suite():
    return [simplecluster_test]
 
