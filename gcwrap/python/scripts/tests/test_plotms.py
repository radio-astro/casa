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
                          overwrite=True, showgui=False, clearplots=True, gridrows=1, gridcols=1)   
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile_jpg)
        self._checkPlotFile(60000, self.plotfile_jpg)
        print
        
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
                          overwrite=True, showgui=False, clearplots=True, gridrows=1, 
                          gridcols=1)
        self.assertTrue(self.res)
        self.assertTrue( os.path.exists(self.plotfile_jpg), 'Plot was not created')
        self._checkPlotFile(60000, self.plotfile_jpg)
        #Next, overwrite is turned off so the save should fail.
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          showgui=False, clearplots=True)
        self.assertFalse(self.res)
        print

    def test003(self):
        '''Plotms 3: Plot using data selection'''
        self.plotfile_jpg = self.outputDir + "testPlot003.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):    
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms,  plotfile=self.plotfile_jpg, expformat='jpg', 
                          showgui=False, clearplots=True, overwrite=True, 
                          selectdata=True, scan='2,4',
                          gridrows=1, gridcols=1)
        self.assertTrue(self.res)
        self.assertTrue(os.path.isfile(self.plotfile_jpg))
        plotSize = os.path.getsize(self.plotfile_jpg)
        self.assertTrue(plotSize > 85000)
        self.assertTrue(plotSize < 95000)
    
    def test004(self):
        print
        '''Plotms 4: Set a custom plotting symbol'''
        self.plotfile_jpg = self.outputDir + "testPlot004.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):    
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, clearplots=True,
                          customsymbol=True, symbolshape='diamond', symbolsize=5,
                          symbolcolor='00ff00', symbolfill='mesh3',
                          gridrows=1, gridcols=1)   
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile_jpg)
        self._checkPlotFile(60000, self.plotfile_jpg)
        print

    def test005(self):
        '''Plotms 5: Check overwrite=True functionality works by saving the plot twice.'''
        self.plotfile_jpg = self.outputDir + "testPlot005.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, clearplots=True,
                          gridrows=1, gridcols=1)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)

        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, clearplots=True, showgui=False)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)
        print
        
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
                          gridrows=2, gridcols=2, clearplots=True,
                          overwrite=True, showgui=False, expformat='jpg', 
                          exprange='all', iteraxis='scan'
                          ,xselfscale=True, yselfscale=True, 
                          xsharedaxis=True, ysharedaxis=True
                          )   
        self.assertTrue(self.res)
        
        '''Check the first page got saved'''
        self.assertTrue(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile_jpg)
        self._checkPlotFile(91000, self.plotfile_jpg)
        
        '''Check the second page got saved'''
        self.assertTrue(os.path.exists(self.plotfile2_jpg), 'Plot2 was not created')
        print 'Plot2 file size is ', os.path.getsize(self.plotfile2_jpg)
        self._checkPlotFile(66000, self.plotfile2_jpg)
        print    

    def test007(self):
        '''Plotms 7: Check that setting an invalid selection returns false and allows a subsequenty plotms command in casapy'''
        self.plotfile_jpg = self.outputDir + "testPlot007.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Should not succeed because this is an invalid spw'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, clearplots=True, spw='500',
                          gridrows=1, gridcols=1)
        self.assertFalse(self.res)

        '''Now we are setting a valid spw so it should work'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, spw='')
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)

    def test008(self):
        '''Plotms 8: Check that the display can be set to multiple row/col and that a plot can
        be placed in a particular location of the grid.'''
        self.plotfile_jpg = self.outputDir + "testPlot008.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Use a grid with 2 rows and 3 columns. Put the plot in the second row, second col'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, clearplots=True, gridrows=2, gridcols=3,
                          rowindex=1, colindex=1)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)
        
    def test009(self):
        '''Plotms 9: Check that the display can be set to multiple row/col and that each grid can be filled with a plot'''
        self.plotfile_jpg = self.outputDir + "testPlot009.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        '''Use a grid with 2 rows and 2 columns. Fill all the plots in the grid'''
        self.res = plotms(vis=self.ms, plotindex=0, title='Plot A',
                          showgui=False, clearplots=True, gridrows=2, gridcols=2,
                          rowindex=0, colindex=0)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=1, title='Plot B',
                          showgui=False, rowindex=0, colindex=1)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=2, title='Plot C',
                          showgui=False, rowindex=1, colindex=0)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=3, title='Plot D',
                          plotfile=self.plotfile_jpg, expformat='jpg',  exprange='all',
                          overwrite=True, showgui=False, rowindex=1, colindex=1)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)

    def test010(self):
        '''Plotms 10: Check that a multiple plot display can be created, and then a second, smaller multiple plot display can be created.'''
        self.plotfile_jpg = self.outputDir + "testPlot010.jpg"
        self.plotfile2_jpg = self.outputDir + "testPlot0102.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        if os.path.exists( self.plotfile2_jpg):
            os.remove( self.plotfile2_jpg)    
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        '''Use a grid with 2 rows and 3 columns. Fill all the plots in the grid'''
        self.res = plotms(vis=self.ms, plotindex=0, title='Plot A',
                          showgui=False, clearplots=True, gridrows=2, gridcols=3,
                          rowindex=0, colindex=0)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=1, title='Plot B',
                          rowindex=0, colindex=1)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=2, title='Plot C',
                          rowindex=0, colindex=2)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=3, title='Plot D',
                          rowindex=1, colindex=0)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=4, title='Plot E',
                          rowindex=1, colindex=1)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=5, title='Plot F',
                          plotfile=self.plotfile_jpg, expformat='jpg',  exprange='all',
                          overwrite=True, 
                          rowindex=1, colindex=2)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)
        
        '''Now use a grid with 1 row and 2 cols.  Fill in all the plots in the grid.'''
        self.res = plotms(vis=self.ms, plotindex=0, title='Plot A',
                          clearplots=True, gridrows=1, gridcols=2,
                          rowindex=0, colindex=0)
        self.res = plotms(vis=self.ms, plotindex=1, title='Plot B',
                          plotfile=self.plotfile2_jpg, expformat='jpg',  exprange='all',
                          overwrite=True, 
                          rowindex=0, colindex=1)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile2_jpg)
        
    def test011(self):
        '''Plotms 11: Check that a legend can be placed on a plot.'''
        self.plotfile_jpg = self.outputDir + "testPlot011.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Place a legend in the upper right corner of the plot'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, clearplots=True,
                          gridrows=1, gridcols=1, 
                          showlegend=True, legendposition='upperRight')
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)   
        
    def test012(self):
        '''Plotms 12: Test that we can colorize by time on an elevation x amp plot.'''
        self.plotfile_jpg = self.outputDir + "testPlot012.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Colorize by time.'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, clearplots=True, 
                          xaxis='elevation', yaxis='amp',
                          coloraxis='time',
                          gridrows=1, gridcols=1)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)        
        
    def test013(self):
        '''Plotms 13: Test that we can colorize by averaged time on an elevation x amp plot.'''
        self.plotfile_jpg = self.outputDir + "testPlot013.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Colorize by averaged time.'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, clearplots=True, xaxis='elevation', yaxis='amp',
                          coloraxis='time', averagedata=True, avgtime='3600',
                          gridrows=1, gridcols=1)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)
        
    def test014(self):
        '''Plotms 14: Test that we iterate over time on an elevation x amp plot.'''
        self.plotfile_jpg = self.outputDir + "testPlot014.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Iterate by time.'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, clearplots=True, xaxis='elevation', yaxis='amp',
                          iteraxis='time', gridrows=2, gridcols=2)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg) 
        
        
    def test015(self):
        '''Plotms 15: Test that we iterate over averaged time on an elevation x amp plot.'''
        self.plotfile_jpg = self.outputDir + "testPlot015.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Iterate by averaged time.'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, clearplots=True, xaxis='elevation', yaxis='amp',
                          iteraxis='time', gridrows=1, gridcols=2)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)  
        
    
    def test016(self):
        '''Plotms 16: Test if we can overplot scan and field on the left y-axis with time on the x-axis.'''
        self.plotfile_jpg = self.outputDir + "testPlot016.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create a (scan & field)/time plot'''
        self.res = plotms(vis=self.ms, overwrite=True, showgui=False, clearplots=True, xaxis='time', yaxis=['scan','field'],
                          plotfile=self.plotfile_jpg, expformat='jpg',
                          showlegend=True, legendposition='lowerRight',
                          customsymbol=[True,True], symbolshape=['diamond','circle'], symbolsize=[5,5],
                          symbolcolor=['ff0000','00ff00'], symbolfill=['mesh3','mesh3'],
                          gridrows=1, gridcols=1)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg) 
        
    def test017(self):               
        '''Plotms 17: Test that we can generate a blank plot running plotms with no arguments'''
        self.plotfile_jpg = self.outputDir + "testPlot017.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create a blank plot'''
        self.res = plotms( showgui=False, plotfile=self.plotfile_jpg, expformat='jpg')
        self.assertTrue(self.res)
        self._checkPlotFile(23000, self.plotfile_jpg)
        
    def test018(self):
        '''Plotms 18: Test if we can overplot (scan and field) vs time with one data set using the left axis and one data set using the right y-axis.'''
        self.plotfile_jpg = self.outputDir + "testPlot018.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create a (scan & field)/time plot'''
        self.res = plotms(vis=self.ms, overwrite=True, showgui=False, clearplots=True, xaxis='time', yaxis=['scan','field'], yaxislocation=['left','right'],
                          plotfile=self.plotfile_jpg, expformat='jpg',
                          showlegend=True, legendposition='exteriorTop',
                          customsymbol=[True,True], symbolshape=['diamond','circle'], symbolsize=[5,5],
                          symbolcolor=['ff0000','00ff00'], symbolfill=['mesh3','mesh3'],
                          gridrows=1, gridcols=1)
        self.assertTrue(self.res)
        self._checkPlotFile(70000, self.plotfile_jpg) 
        
    def test019(self):
        '''Plotms 19: Test if we can overplot (scan and field) vs time and iterate over antenna.'''
        self.plotfile_jpg = self.outputDir + "testPlot019.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create a (scan & field)/time plot with iteration over antenna'''
        self.res = plotms(vis=self.ms, overwrite=True, showgui=False, clearplots=True, xaxis='time', yaxis=['scan','field'], yaxislocation=['left','right'],
                          plotfile=self.plotfile_jpg, expformat='jpg',
                          gridrows=2, gridcols=2, iteraxis="antenna",
                          showlegend=True, legendposition='lowerLeft',
                          customsymbol=[True,True], symbolshape=['diamond','circle'], symbolsize=[5,5],
                          symbolcolor=['ff0000','00ff00'], symbolfill=['mesh3','mesh3'])
        self.assertTrue(self.res)
        self._checkPlotFile(222500, self.plotfile_jpg) 
        
    def test020(self):
        '''Plotms 20: Export an iteration plot with one plot per page (pipeline).'''
        self.plotFiles = [self.outputDir + "testPlot020.jpg",
                          self.outputDir + "testPlot0202.jpg",
                          self.outputDir + "testPlot0203.jpg",
                          self.outputDir + "testPlot0204.jpg",
                          self.outputDir + "testPlot0205.jpg",
                          self.outputDir + "testPlot0206.jpg",
                          self.outputDir + "testPlot0207.jpg"]
        
        printMsg = 'Writing to '
        for  i in range(0, len(self.plotFiles)):
            printMsg = printMsg + self.plotFiles[i]
            printMsg = printMsg + ', '
        print printMsg
        
        for  i in range(0, len(self.plotFiles)):
            if os.path.exists( self.plotFiles[i]):
                os.remove( self.plotFiles[i])               
              
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        '''Make iteration plots over scan'''
        self.res = plotms(vis=self.ms, plotfile=self.plotFiles[0], clearplots=True,
                          overwrite=True, showgui=False, expformat='jpg', 
                          exprange='all', iteraxis='scan', gridrows=1, gridcols=1)   
        self.assertTrue(self.res)
        
        '''Check each page got saved'''
        for  i in range(0, len(self.plotFiles)):
            self.assertTrue(os.path.exists(self.plotFiles[i]), 'Plot was not created')
            print 'Plot file size ', i, ' is ', os.path.getsize(self.plotFiles[i])
            self._checkPlotFile(60000, self.plotFiles[i]) 
        print  
        
    def test021(self):
        '''Plotms 21: Test that model/data works.'''
        self.plotfile_jpg = self.outputDir + "testPlot021.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Set up the y-axis to use data/model'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', clearplots=True, 
                          overwrite=True, showgui=False, yaxis='amp', 
                          ydatacolumn='data/model', gridrows=1, gridcols=1)
        self.assertTrue(self.res)
        self._checkPlotFile(230000, self.plotfile_jpg)   
   
    def test022(self):
        '''Plotms 22: Test that wt*amp works for x-and y-axis choices.'''
        self.plotfile_jpg = self.outputDir + "testPlot022.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Use a 2x1 grid and plot weight*am vs time on x-axis with one graph and y-axis on other'''
        self.res = plotms(vis=self.ms, gridrows=2, gridcols=1, clearplots=True,
                          showgui=False, yaxis='wt*amp', xaxis='time')
        
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          plotindex=1, rowindex=1, colindex=0, 
                          overwrite=True, showgui=False, xaxis='wt*amp', yaxis='time')
        self.assertTrue( self.res )
        self._checkPlotFile(220000, self.plotfile_jpg) 
        
    def test023(self):
        '''Plotms 23: Test that corrected/model works for x-and y-amp/data choices.'''
        self.plotfile_jpg = self.outputDir + "testPlot023.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Use a 2x1 grid and plot amp vs time and vice versa.  Set to corrected/model on amp axis'''
        self.res = plotms(vis=self.ms, gridrows=2, gridcols=1, clearplots=True,
                          showgui=False, yaxis='amp', ydatacolumn='corrected/model', xaxis='time')
        
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          plotindex=1, rowindex=1, colindex=0, 
                          overwrite=True, showgui=False, xaxis='amp', xdatacolumn='corrected/model', yaxis='time')
        self.assertTrue( self.res )
        self._checkPlotFile(249000, self.plotfile_jpg)    
        
    def test024(self):
        '''Plotms 24: Test an invalid antenna selection does not crash plotms.'''
        self.plotfile_jpg = self.outputDir + "testPlot024.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        '''Verify error with antenna'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          clearplots=True, gridrows=1, gridcols=1,
                          overwrite=True, showgui=False,
                          selectdata=True, antenna='ea22&&*' )  
        self.assertFalse( self.res)
        
        '''Use a 1x1 grid and plot amp vs time and vice versa.  Verify success without antenna'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          gridrows=1, gridcols=1, clearplots=True, 
                          overwrite=True, showgui=False)
        self.assertTrue( self.res )
        self._checkPlotFile(175000, self.plotfile_jpg)   
        
       
        
        
    def test025(self):
        '''Plotms 25: Test that we can overplot plots with two data sets.'''
        self.plotfile_jpg = self.outputDir + "testPlot025.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Use a 1x1 grid and plot scan vs time.'''
        self.res = plotms(vis=self.ms, gridrows=1, gridcols=1, clearplots=True, 
                          overwrite=True, showgui=False, yaxis='scan',
                          rowindex=0, colindex=0, plotindex=0,
                          showlegend=True, legendposition='lowerRight',
                          customsymbol=[True], symbolshape=['diamond'], symbolsize=[3],
                          symbolcolor=['ff0000'], symbolfill=['mesh3'])
        self.assertTrue( self.res )
        
        '''Now add in field vs time and export it'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, yaxis='field',
                          rowindex=0, colindex=0, plotindex=1,
                          showlegend=True, legendposition='lowerRight',
                          customsymbol=[True], symbolshape=['circle'], symbolsize=[3],
                          symbolcolor=['00FF00'], symbolfill=['mesh3'])   
        self.assertTrue( self.res) 
        self._checkPlotFile(58000, self.plotfile_jpg)    
        
        
    def test026(self):
        '''Plotms 26: Export an iteration plot consisting of two pages. Duplicate of test 6 except we use a right axis and a non-square grid.'''
        self.plotfile_jpg = self.outputDir + "testPlot026.jpg"
        self.plotfile2_jpg = self.outputDir + "testPlot0262.jpg"
        print 'Writing to ', self.plotfile_jpg, " and ", self.plotfile2_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        if os.path.exists( self.plotfile2_jpg):
            os.remove( self.plotfile2_jpg) 
              
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        '''Make 2 pages of 3x2 iteration plots over scan sharing common axes & scales'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg,
                          gridrows=3, gridcols=2, clearplots=True,
                          overwrite=True, showgui=False, expformat='jpg', 
                          exprange='all', iteraxis='scan'
                          ,xselfscale=True, yselfscale=True, 
                          xsharedaxis=True, ysharedaxis=True, 
                          yaxis=['amp'], yaxislocation=['right']
                          )   
        self.assertTrue(self.res)
        
        '''Check the first page got saved'''
        self.assertTrue(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile_jpg)
        self._checkPlotFile(10000, self.plotfile_jpg)
        
        '''Check the second page got saved'''
        self.assertTrue(os.path.exists(self.plotfile2_jpg), 'Plot2 was not created')
        print 'Plot2 file size is ', os.path.getsize(self.plotfile2_jpg)
        self._checkPlotFile(65000, self.plotfile2_jpg)
        print    
        
    def test027(self):
        '''Plotms 27: Test that we can do a 2x2 multiplot display. Consisting of single plots and overplots'''
        self.plotFiles = [self.outputDir + "testPlot027.jpg",
                          self.outputDir + "testPlot0272.jpg",
                          self.outputDir + "testPlot0273.jpg"]
        
        printMsg = 'Writing to '
        for  i in range(0, len(self.plotFiles)):
            printMsg = printMsg + self.plotFiles[i]
            printMsg = printMsg + ', '
        print printMsg
        
        for  i in range(0, len(self.plotFiles)):
            if os.path.exists( self.plotFiles[i]):
                os.remove( self.plotFiles[i])        
        
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        '''First put the multiplot in, scan,field vs time'''
        '''Plot scan vs time.'''
        self.res = plotms(vis=self.ms, gridrows=2, gridcols=2, clearplots=True, 
                          showgui=False, yaxis='scan',
                          rowindex=0, colindex=0, plotindex=0,
                          showlegend=True, legendposition='lowerRight',
                          customsymbol=[True], symbolshape=['diamond'], symbolsize=[3],
                          symbolcolor=['ff0000'], symbolfill=['mesh3'])
        self.assertTrue( self.res )
        print 'Added overplot 1'
        
        '''Overplot field vs time.'''
        self.res = plotms(vis=self.ms,  
                          showgui=False, yaxis='field',
                          rowindex=0, colindex=0, plotindex=1,
                          showlegend=True, legendposition='lowerRight',
                          customsymbol=[True], symbolshape=['circle'], symbolsize=[3],
                          symbolcolor=['00FF00'], symbolfill=['mesh3'])   
        self.assertTrue( self.res)
        print 'Added overplot 2' 
        
        
        '''Now add a single basic plot in the first row, second column'''
        print
        print 'Adding single plot'
        self.res = plotms(vis=self.ms, 
                          rowindex=0, colindex=1, plotindex=2, 
                          showgui=False)   
        print 'Added single plot'
        
        '''Now add in an iteration plot and export it.'''
        self.res = plotms(vis=self.ms, plotfile=self.plotFiles[0],
                          overwrite=True, showgui=False, expformat='jpg', 
                          exprange='all', iteraxis='scan',
                          rowindex=1, colindex=0, plotindex=3
                          )   
        print 'Added iteration plot, now exporting'
        '''Check each page got saved'''
        for  i in range(0, len(self.plotFiles)):
            self.assertTrue(os.path.exists(self.plotFiles[i]), 'Plot was not created')
            print 'Plot file size ', i, ' is ', os.path.getsize(self.plotFiles[i])
            self._checkPlotFile(55000, self.plotFiles[i]) 
        print    
        
    def test028(self):
        '''Plotms 28: Test generation of a single plot with two y-axes.'''
        self.plotfile_jpg = self.outputDir + "testPlot028.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Plot amp vs time and scan vs time. '''
        self.res = plotms(vis=self.ms, gridrows=1, gridcols=1, clearplots=True,
                          showgui=False, yaxis=['amp','scan'], 
                          ydatacolumn=['corrected/model'], 
                          yaxislocation=['left','right'],xaxis='time',
                          plotfile=self.plotfile_jpg, expformat='jpg')
        
        self.assertTrue(self.res)
        self._checkPlotFile(249000, self.plotfile_jpg)      
        
    def test029(self):
        '''Plotms 29: Test that generation of a single plot with two y-axes using identical data returns false.'''
        self.plotfile_jpg = self.outputDir + "testPlot029.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Plot amp vs time and amp vs time. '''
        self.res = plotms(vis=self.ms, gridrows=1, gridcols=1, clearplots=True,
                          showgui=False, yaxis=['amp','amp'], 
                          yaxislocation=['left','right'],xaxis='time',
                          plotfile=self.plotfile_jpg, expformat='jpg')
        
        self.assertFalse(self.res)  
        
    def test030(self):
        '''Plotms 30: The data set here was producing an 'artifact' when the plot was exported.  Test was developed in response to CAS-6662.'''      
        self.plotFiles = [self.outputDir + "testPlot030.jpg",
                       self.outputDir + "testPlot0302.jpg",
                          self.outputDir + "testPlot0303.jpg",
                          self.outputDir + "testPlot0304.jpg",
                          self.outputDir + "testPlot0305.jpg",
                          self.outputDir + "testPlot0306.jpg",
                          self.outputDir + "testPlot0307.jpg",
                          self.outputDir + "testPlot0308.jpg",
                          self.outputDir + "testPlot0309.jpg",
                          self.outputDir + "testPlot0310.jpg",
                          self.outputDir + "testPlot0311.jpg",
                       self.outputDir + "testPlot0312.jpg",
                          self.outputDir + "testPlot0313.jpg",
                          self.outputDir + "testPlot0314.jpg",
                          self.outputDir + "testPlot0315.jpg",
                          self.outputDir + "testPlot0316.jpg",
                          self.outputDir + "testPlot0317.jpg",
                          self.outputDir + "testPlot0318.jpg",
                          self.outputDir + "testPlot0319.jpg",
                          self.outputDir + "testPlot0320.jpg",
                          self.outputDir + "testPlot0321.jpg",
                       self.outputDir + "testPlot0322.jpg",
                          self.outputDir + "testPlot0323.jpg",
                          self.outputDir + "testPlot0324.jpg",
                          self.outputDir + "testPlot0325.jpg",
                          self.outputDir + "testPlot0326.jpg",
                          self.outputDir + "testPlot0327.jpg",
                          self.outputDir + "testPlot0328.jpg",
                          self.outputDir + "testPlot0329.jpg",
                          self.outputDir + "testPlot0330.jpg"]
        
        printMsg = 'Writing to '
        for  i in range(0, len(self.plotFiles)):
            printMsg = printMsg + self.plotFiles[i]
            printMsg = printMsg + ', '
        print printMsg
        
        for  i in range(0, len(self.plotFiles)):
            if os.path.exists( self.plotFiles[i]):
                os.remove( self.plotFiles[i])    
        
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        '''Plot amp vs time and amp vs time. '''
        for  i in range(0, len(self.plotFiles)):
            self.res = plotms(vis='/home/uniblab/casa/trunk/test/Plotms/uid___A002_X49990a_X1f.ms', 
                          xaxis='uvdist', yaxis='amp',ydatacolumn='model',
                          spw='9', scan='7',
                          showgui=False, clearplots=True,            
                          plotfile=self.plotFiles[i])
            self.assertTrue(self.res)
            self._checkPlotFile(59000, self.plotFiles[i])
            
                  
    def test031(self):
        print
        '''Plotms 31: Set a custom flagged plotting symbol'''
        self.plotfile_jpg = self.outputDir + "testPlot031.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):    
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, clearplots=True,
                          customflaggedsymbol=True, flaggedsymbolshape='diamond', flaggedsymbolsize=5,
                          flaggedsymbolcolor='00ff00', flaggedsymbolfill='mesh3',
                          gridrows=1, gridcols=1)   
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile_jpg)
        self._checkPlotFile(60000, self.plotfile_jpg)
        print 
        
    def test032(self):
        '''Plotms 32: Pipeline no plot scenario.  Test was developed in response to CAS-6662.'''      
        self.plotFiles = [self.outputDir + "testPlot032.jpg", 
                          self.outputDir + "testPlot0322.jpg"]
        
        printMsg = 'Writing to '
        for  i in range(0, len(self.plotFiles)):
            printMsg = printMsg + self.plotFiles[i]
            printMsg = printMsg + ', '
        print printMsg
        
        for  i in range(0, len(self.plotFiles)):
            if os.path.exists( self.plotFiles[i]):
                os.remove( self.plotFiles[i])    
        
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
    
        self.res = plotms(vis='/home/uniblab/casa/trunk/test/Plotms/uid___A002_X5f231a_X179b.ms', 
                          xaxis='uvdist', yaxis='amp',ydatacolumn='model',
                          spw='17', scan='6', 
                          plotrange=[0,0,0,0], avgscan=False, avgbaseline=False, coloraxis='spw',
                          showgui=False, clearplots=True,            
                          plotfile=self.plotFiles[0])
        self.assertTrue(self.res)
        '''self._checkPlotFile(50000, self.plotFiles[0])'''   
               
        self.res = plotms(vis='/home/uniblab/casa/trunk/test/Plotms/uid___A002_X5f231a_X179b.ms', 
                          xaxis='uvdist', yaxis='amp',ydatacolumn='model',
                          spw='21', scan='6',
                          plotrange=[0,0,0,0], avgscan=False, avgbaseline=False, coloraxis='spw',
                          showgui=False, clearplots=True,            
                          plotfile=self.plotFiles[1])
        self.assertTrue(self.res)
        self._checkPlotFile(50000, self.plotFiles[1])      
 
def suite():
    print 'Tests may fail due to DBUS timeout if the version of Qt is not at least 4.8.5'
    return [plotms_test1]
  
 
