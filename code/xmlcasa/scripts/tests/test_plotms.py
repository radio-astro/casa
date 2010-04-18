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

    def _cleanUp(self):
        os.system('rm -rf ' + self.ms)
        os.system('rm -rf ' + self.plotfile_jpg)

    def setUp(self):
        self.res = None
        default(plotms)
        self._cleanUp()
        shutil.copytree(self.datapath+self.ms, self.ms)

    def tearDown(self):
        self._cleanUp()
        pm.setPlotMSFilename("")

    def test000(self):
        '''Test 0: Write a jpg file using the pm tool'''
        pm.setPlotMSFilename(self.ms)
        pm.show()
        while (pm.isDrawing()):
            time.sleep(0.5)
        pm.save(self.plotfile_jpg)
        self._waitForFile(self.plotfile_jpg, 10)
        self.assertTrue(os.path.isfile(self.plotfile_jpg))
        self._compareSHA(self.plotfile_jpg)

    def test001(self):
        '''Test 1: Write a jpg file using the plotms task'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg)
        self.assertTrue(self.res)
        self._waitForFile(self.plotfile_jpg, 10)
        self.assertTrue(os.path.isfile(self.plotfile_jpg))
        self._compareSHA(self.plotfile_jpg)
        
    def test002(self):
        '''Test 2: Check overwrite functionality works'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg)
        self.assertTrue(self.res)
        self._waitForFile(self.plotfile_jpg, 10)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg)
        self.assertFalse(self.res)
        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, overwrite=True)
        self.assertTrue(self.res)
        time.sleep(5)
        self.assertTrue(os.path.isfile(self.plotfile_jpg))
        self._compareSHA(self.plotfile_jpg)

    def _compareSHA(self, file1, file2=None):
        if (file2==None):
            file2 = self.datapath+"/"+ file1
        self.assertEqual(
            sha.new(open(file1,'r').read()).hexdigest(),
            sha.new(open(file2,'r').read()).hexdigest()
        )
        
    def _waitForFile(self, file, seconds):
        for i in range(0,seconds):
            if (os.path.isfile(file)):
                return
            time.sleep(1)

def suite():
    return [plotms_test]
 