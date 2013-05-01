import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
import sha
import time

# Path for data
datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/plotms/"

# Pick up alternative data directory to run tests on MMSs
testmms = False
if os.environ.has_key('TEST_DATADIR'):   
    DATADIR = str(os.environ.get('TEST_DATADIR'))+'/plotms/'
    if os.path.isdir(DATADIR):
        testmms = True
        datapath = DATADIR

print 'plotms tests will use data from '+datapath         

class test_base(unittest.TestCase):

    ms = "pm_ngc5921.ms"
    if testmms:
        ms = "pm_ngc5921.mms"
    plotfile_jpg = "myplot.jpg"
    display = os.environ.get("DISPLAY")
    plotfile_hash = ""
    plotfile_minsize = 120000

    def _cleanUp(self):
        if os.path.exists(self.ms):
            shutil.rmtree(self.ms)
        if os.path.exists(self.plotfile_jpg):
            os.remove(self.plotfile_jpg)
    
    def setUpdata(self):
        if not self.display.startswith(':'):
            self.res = None
            default(plotms)
            self._cleanUp()
            shutil.copytree(datapath+self.ms, self.ms, symlinks=True)            

    def tearDowndata(self):
        if not self.display.startswith(':'):
            self._cleanUp()
            pm.setPlotMSFilename("")

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
        
class plotms_test1(test_base):

    def setUp(self):
        self.setUpdata()
        
    def tearDown(self):
        self.tearDowndata()
        
    # This test does not work. It seems that the pm tool 
    # does not draw anything on the GUI inside this script.
    # It works manually inside casapy. ??????????
#    def test000(self):
#        '''Plotms 0: Write a jpg file using the pm tool'''
#        self.assertFalse(self.display.startswith(':'),'DISPLAY not set, cannot run test')
#        pm.setPlotMSFilename(self.ms)
#        time.sleep(5)
#        pm.show()
#        time.sleep(5)
#        while (pm.isDrawing()):
#            time.sleep(0.5)
#        pm.save(self.plotfile_jpg, format='jpg')
#        self._waitForFile(self.plotfile_jpg, 10)
#        self._checkPlotFile()
        
    def test001(self):
        '''Plotms 1: Write a jpg file using the plotms task'''
        self.assertFalse(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, format='jpg')
        self.assertTrue(self.res)
        self._waitForFile(self.plotfile_jpg, 10)
        self.assertTrue(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        self._checkPlotFile()
        
    def test002(self):
        '''Plotms 2: Check overwrite=False functionality works'''
        self.assertFalse(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, format='jpg')
        self.assertTrue(self.res)
        self._waitForFile(self.plotfile_jpg, 10)
        self._checkPlotFile()

        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, format='jpg')
        self.assertFalse(self.res)


    def test003(self):
        '''Plotms 3: Plot using data selection'''
        self.assertFalse(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms, scan='2,4', plotfile=self.plotfile_jpg, format='jpg')
        self.assertTrue(self.res)
        self._waitForFile(self.plotfile_jpg, 5)
        self.assertTrue(os.path.isfile(self.plotfile_jpg))
        self.assertTrue(os.path.getsize(self.plotfile_jpg) > 50000)
        
        
# This test has been separated into another class so that
# it runs as the last test. If it runs before the other tests,
# an exception from plotms such as:
# terminate called after throwing an instance of 'casa::AipsError'
#  what():  CountedPtr: null dereference error
# will kill the GUI and will prevent the other files from working. 
# To be fixed in plotms. ?????
class plotms_test2(test_base):

    def setUp(self):
        self.setUpdata()
        
    def tearDown(self):
        self.tearDowndata()

    def test004(self):
        '''Plotms 4: Check overwrite=True functionality works'''
        self.assertFalse(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, format='jpg')
        self.assertTrue(self.res)
        self._waitForFile(self.plotfile_jpg, 10)
        self._checkPlotFile()

        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, format='jpg', overwrite=True)
        self.assertTrue(self.res)
        self._waitForFile(self.plotfile_jpg, 10)
        self._checkPlotFile()


def suite():
    return [plotms_test1, plotms_test2]
 
