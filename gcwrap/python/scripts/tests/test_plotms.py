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

class test_base(unittest.TestCase):

    ms = "pm_ngc5921.ms"
    if testmms:
        ms = "pm_ngc5921.mms"
    datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/plotms/'
    if not testmms:
        ms = datapath + ms
    outputDir='/tmp/'
    plotfile_jpg = "/tmp/myplot.jpg"
    display = os.environ.get("DISPLAY")
    plotfile_hash = ""

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

    def _checkPlotFile(self, minSize, plotfileName):
        self.assertTrue(os.path.isfile(plotfileName))
        print 'File size is ', os.path.getsize(plotfileName)
        self.assertTrue(os.path.getsize(plotfileName) > minSize)
        #if(self.plotfile_hash):
        #    self.assertEqual(
        #        sha.new(open(plotfileName, 'r').read()).hexdigest(),
        #        self.plotfile_hash
        #    )
        #else:
            # store to check against following test results
        #    self.plotfile_hash = sha.new(open(plotfileName, 'r').read()).hexdigest()
            
    
        
class plotms_test1(test_base):

    def setUp(self):
        self.setUpdata()
        
    def tearDown(self):
       self.tearDowndata()
        
  
        
    def test001(self):
        '''Plotms 1: Write a jpg file using the plotms task'''
        self.plotfile_jpg = self.outputDir + "testPlot001.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat="jpg", 
                          overwrite=True, showgui=False)   
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile_jpg)
        self._checkPlotFile(60000, self.plotfile_jpg)
        
        
    def test002(self):
        '''Plotms 2: Check overwrite=False functionality works'''
        self.plotfile_jpg = self.outputDir + "testPlot002.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        #First overwrite is turned on in case the plot file already exists.
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False)
        self.assertTrue(self.res)
        self.assertTrue( os.path.exists(self.plotfile_jpg), 'Plot was not created')
        self._checkPlotFile(60000, self.plotfile_jpg)
        #Next, overwrite is turned off so the save should fail.
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', showgui=False)
        self.assertFalse(self.res)
        

    def test003(self):
        '''Plotms 3: Plot using data selection'''
        self.plotfile_jpg = self.outputDir + "testPlot003.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):    
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms,  plotfile=self.plotfile_jpg, expformat='jpg', 
                          showgui=False, overwrite=True, selectdata=True, scan='2,4')
        self.assertTrue(self.res)
        self.assertTrue(os.path.isfile(self.plotfile_jpg))
        plotSize = os.path.getsize(self.plotfile_jpg)
        self.assertTrue(plotSize > 60000)
        self.assertTrue(plotSize < 70000)
    
    def test004(self):
        '''Plotms 4: Set a custom plotting symbol'''
        self.plotfile_jpg = self.outputDir + "testPlot004.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):    
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False,
                          customsymbol=True, symbolshape='diamond', symbolsize=5,
                          symbolcolor='00ff00', symbolfill='mesh3')   
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile_jpg)
        self._checkPlotFile(60000, self.plotfile_jpg)
        

    def test005(self):
        '''Plotms 5: Check overwrite=True functionality works'''
        self.plotfile_jpg = self.outputDir + "testPlot005.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)

        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)
        
    def test006(self):
        '''Plotms 6: Export an iteration plot consisting of two pages.'''
        self.plotfile_jpg = self.outputDir + "testPlot006.jpg"
        self.plotfile2_jpg = self.outputDir + "testPlot0062.jpg"
        print 'Writing to ', self.plotfile_jpg, " and ", self.plotfile2_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        if os.path.exists( self.plotfile2_jpg):
            os.remove( self.plotfile2_jpg) 
              
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        '''Make 2 pages of 2x2 iteration plots over scan sharing common axes & scales'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg,
                          overwrite=True, showgui=False, expformat='jpg', 
                          exprange='all', iteraxis='scan',
                          xselfscale=True, yselfscale=True, 
                          xsharedaxis=True, ysharedaxis=True,
                          iternx=2, iterny=2)   
        self.assertTrue(self.res)
        
        '''Check the first page got saved'''
        self.assertTrue(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile_jpg)
        self._checkPlotFile(91000, self.plotfile_jpg)
        
        '''Check the second page got saved'''
        self.assertTrue(os.path.exists(self.plotfile2_jpg), 'Plot2 was not created')
        print 'Plot2 file size is ', os.path.getsize(self.plotfile2_jpg)
        self._checkPlotFile(66000, self.plotfile2_jpg)    


def suite():
    print 'Tests may fail due to DBUS timeout if the version of Qt is not at least 4.8.5'
    return [plotms_test1]
  
 
