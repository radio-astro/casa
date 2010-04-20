import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
import sha
import time

class plotms_test(unittest.TestCase):
    # Input and output names
    
    ms = "pm_ngc5921.ms"
    plotfile_jpg = "myplot.jpg"
    datapath = os.environ.get('CASAPATH').split()[0] +'/data/regression/unittest/plotms/'
    display = os.environ.get("DISPLAY")
    plotfile_hash = ""
    plotfile_minsize = 120000

    def _cleanUp(self):
        os.system('rm -rf ' + self.ms)
        os.system('rm -rf ' + self.plotfile_jpg)


    def setUp(self):
        if (self.display):
            self.res = None
            default(plotms)
            self._cleanUp()
            shutil.copytree(self.datapath+self.ms, self.ms)

    def tearDown(self):
        if (self.display):
            self._cleanUp()
            pm.setPlotMSFilename("")

    def test000(self):
        '''Test 0: Write a jpg file using the pm tool'''
        if (not self.display):
            print "DISPLAY not set, cannot run test"
            return
        pm.setPlotMSFilename(self.ms)
        pm.show()
        while (pm.isDrawing()):
            time.sleep(0.5)
        pm.save(self.plotfile_jpg)
        self._waitForFile(self.plotfile_jpg, 10)
        self._checkPlotFile()

    def test001(self):
        '''Test 1: Write a jpg file using the plotms task'''
        if (not self.display):
            print "DISPLAY not set, cannot run test"
            return
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg)
        self.assertTrue(self.res)
        self._waitForFile(self.plotfile_jpg, 10)
        self._checkPlotFile()
        
    def test002(self):
        '''Test 2: Check overwrite functionality works'''
        if (not self.display):
            print "DISPLAY not set, cannot run test"
            return
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg)
        self.assertTrue(self.res)
        self._waitForFile(self.plotfile_jpg, 10)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg)
        self.assertFalse(self.res)
        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, overwrite=True)
        self.assertTrue(self.res)
        time.sleep(5)
        self._checkPlotFile()
        
    def _checkPlotFile(self):
        self.assertTrue(os.path.isfile(self.plotfile_jpg))
        self.assertTrue(os.path.getsize(self.plotfile_jpg) > self.plotfile_minsize)
        if(self.plotfile_hash):
            self.assertEqual(
                sha.new(open(self.plotfile_jpg, 'r').read()).hexdigest(),
                self.plotfile_hash
            )
        else:
            # store to check against following test results
            self.plotfile_hash = sha.new(open(self.plotfile_jpg, 'r').read()).hexdigest()
            
    def _waitForFile(self, file, seconds):
        for i in range(0,seconds):
            if (os.path.isfile(file)):
                return
            time.sleep(1)

def suite():
    return [plotms_test]
 
