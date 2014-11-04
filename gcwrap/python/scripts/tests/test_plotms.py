import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
import fnmatch
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
        
    def _getFileCount(self, dirName, namePattern ):
        nameTarget = namePattern + '*'
        count = 0
        for  file in os.listdir( dirName ):
            if fnmatch.fnmatch( file, nameTarget):
                count = count + 1
        return count
    
    def _removeFiles(self, dirName, namePattern ):
        nameTarget = namePattern + '*'
        for  file in os.listdir( dirName ):
            if fnmatch.fnmatch( file, nameTarget):
                os.remove( dirName + "/" + file )
            
    
        
class plotms_test1(test_base):

    def setUp(self):
        self.setUpdata()
        
    def tearDown(self):
       self.tearDowndata()
        
            
    def stest001(self):
        '''Plotms 1: Write a jpg file using the plotms task'''
        self.plotfile_jpg = self.outputDir + "testPlot001.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat="jpg", 
                          overwrite=True, showgui=False, gridrows=1, gridcols=1)   
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile_jpg)
        self._checkPlotFile(60000, self.plotfile_jpg)
        print
        
    def stest002(self):
        '''Plotms 2: Check overwrite=False functionality works'''
        self.plotfile_jpg = self.outputDir + "testPlot002.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        #First overwrite is turned on in case the plot file already exists.
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, gridrows=1, 
                          gridcols=1)
        self.assertTrue(self.res)
        self.assertTrue( os.path.exists(self.plotfile_jpg), 'Plot was not created')
        self._checkPlotFile(60000, self.plotfile_jpg)
        #Next, overwrite is turned off so the save should fail.
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          showgui=False)
        self.assertFalse(self.res)
        print

    def stest003(self):
        '''Plotms 3: Plot using data selection'''
        self.plotfile_jpg = self.outputDir + "testPlot003.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):    
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms,  plotfile=self.plotfile_jpg, expformat='jpg', 
                          showgui=False,  overwrite=True, 
                          selectdata=True, scan='2,4',
                          gridrows=1, gridcols=1)
        self.assertTrue(self.res)
        self.assertTrue(os.path.isfile(self.plotfile_jpg))
        plotSize = os.path.getsize(self.plotfile_jpg)
        self.assertTrue(plotSize > 85000)
        self.assertTrue(plotSize < 95000)
    
    def stest004(self):
        print
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
                          symbolcolor='00ff00', symbolfill='mesh3',
                          gridrows=1, gridcols=1)   
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile_jpg)
        self._checkPlotFile(60000, self.plotfile_jpg)
        print

    def stest005(self):
        '''Plotms 5: Check overwrite=True functionality works by saving the plot twice.'''
        self.plotfile_jpg = self.outputDir + "testPlot005.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False,
                          gridrows=1, gridcols=1)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)

        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)
        print
        
    def stest006(self):
        '''Plotms 6: Export an iteration plot consisting of two pages.'''
        self.plotfile_jpg = self.outputDir + "testPlot006.jpg"
        self.plotfile1_jpg = self.outputDir + "testPlot006_Scan1,2,3,4.jpg"
        self.plotfile2_jpg = self.outputDir + "testPlot006_Scan5,6,7_2.jpg"
        print 'Writing to ', self.plotfile1_jpg, " and ", self.plotfile2_jpg
        if os.path.exists( self.plotfile1_jpg):
            os.remove( self.plotfile1_jpg)
        if os.path.exists( self.plotfile2_jpg):
            os.remove( self.plotfile2_jpg) 
              
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        '''Make 2 pages of 2x2 iteration plots over scan sharing common axes & scales'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg,
                          gridrows=2, gridcols=2, 
                          overwrite=True, showgui=False, expformat='jpg', 
                          iteraxis='scan',
                           exprange='all',xselfscale=True, yselfscale=True, 
                          xsharedaxis=True, ysharedaxis=True
                          )   
        self.assertTrue(self.res)
        
        '''Check the first page got saved'''
        self.assertTrue(os.path.exists(self.plotfile1_jpg), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile1_jpg)
        self._checkPlotFile(91000, self.plotfile1_jpg)
        
        '''Check the second page got saved'''
        self.assertTrue(os.path.exists(self.plotfile2_jpg), 'Plot2 was not created')
        print 'Plot2 file size is ', os.path.getsize(self.plotfile2_jpg)
        self._checkPlotFile(66000, self.plotfile2_jpg)
        print    

    def stest007(self):
        '''Plotms 7: Check that setting an invalid selection returns false and allows a subsequenty plotms command in casapy'''
        self.plotfile_jpg = self.outputDir + "testPlot007.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Should not succeed because this is an invalid spw'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, spw='500',
                          gridrows=1, gridcols=1)
        self.assertFalse(self.res)

        '''Now we are setting a valid spw so it should work'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          showgui=False, spw='')
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)

    def stest008(self):
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
                          overwrite=True, showgui=False, gridrows=2, gridcols=3, xaxis="time",
                          rowindex=1, colindex=1)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)
        
    def stest009(self):
        '''Plotms 9: Check that the display can be set to multiple row/col and that each grid can be filled with a plot'''
        self.plotfile_jpg = self.outputDir + "testPlot009.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        '''Use a grid with 2 rows and 2 columns. Fill all the plots in the grid'''
        self.res = plotms(vis=self.ms, plotindex=0, title='Plot A',
                          showgui=False, gridrows=2, gridcols=2,
                          rowindex=0, colindex=0)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=1, title='Plot B',
                          showgui=False, clearplots=False, rowindex=0, colindex=1)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=2, title='Plot C',
                          showgui=False, clearplots=False, rowindex=1, colindex=0)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=3, title='Plot D',
                          plotfile=self.plotfile_jpg, expformat='jpg',  
                          overwrite=True, showgui=False, clearplots=False, rowindex=1, colindex=1)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)

    def stest010(self):
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
                          showgui=False,  gridrows=2, gridcols=3,
                          rowindex=0, colindex=0)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=1, title='Plot B',
                          rowindex=0, showgui=False, clearplots = False, colindex=1)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=2, title='Plot C',
                          rowindex=0, showgui=False, clearplots=False, colindex=2)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=3, title='Plot D',
                          rowindex=1, showgui=False, clearplots=False, colindex=0)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=4, title='Plot E',
                          rowindex=1, showgui=False, clearplots=False, colindex=1)
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotindex=5, title='Plot F',
                          plotfile=self.plotfile_jpg, expformat='jpg',
                          overwrite=True, showgui=False, clearplots=False,
                          rowindex=1, colindex=2)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)
        
        '''Now use a grid with 1 row and 2 cols.  Fill in all the plots in the grid.'''
        self.res = plotms(vis=self.ms, plotindex=0, title='Plot A',
                          gridrows=1, gridcols=2,showgui=False,
                          rowindex=0, colindex=0)
        self.res = plotms(vis=self.ms, plotindex=1, title='Plot B',
                          plotfile=self.plotfile2_jpg, expformat='jpg',
                          overwrite=True,  clearplots=False,showgui=False,
                          rowindex=0, colindex=1)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile2_jpg)
        
    def stest011(self):
        '''Plotms 11: Check that a legend can be placed on a plot.'''
        self.plotfile_jpg = self.outputDir + "testPlot011.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Place a legend in the upper right corner of the plot'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False,
                          gridrows=1, gridcols=1, 
                          showlegend=True, legendposition='upperRight')
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)   
        
    def stest012(self):
        '''Plotms 12: Test that we can colorize by time on an elevation x amp plot.'''
        self.plotfile_jpg = self.outputDir + "testPlot012.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Colorize by time.'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False,
                          xaxis='elevation', yaxis='amp',
                          coloraxis='time',
                          gridrows=1, gridcols=1)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)        
       
    def stest012a(self):
        '''Plotms 12a: Test that we can colorize by synonym see CAS-6921.'''
        self.plotfile_jpg = self.outputDir + "testPlot012a.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Colorize by chan.'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False,
                          xaxis='elevation', yaxis='amp',
                          coloraxis='chan',
                          gridrows=1, gridcols=1)
        self.assertTrue(self.res)
        # Note that if coloraxis arg reverts to default the plot will be ~180000
        self._checkPlotFile(200000, self.plotfile_jpg)
 
    def stest013(self):
        '''Plotms 13: Test that we can colorize by averaged time on an elevation x amp plot.'''
        self.plotfile_jpg = self.outputDir + "testPlot013.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Colorize by averaged time.'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, xaxis='elevation', yaxis='amp',
                          coloraxis='time', averagedata=True, avgtime='3600',
                          gridrows=1, gridcols=1)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)
        
    def stest014(self):
        '''Plotms 14: Test that we iterate over time on an elevation x amp plot.'''
        self.plotfile_jpg = self.outputDir + "testPlot014.jpg"
        self.writefile_jpg = self.outputDir + "testPlot014_Time09:18:59.9998,09:19:30.0002,09:20:00.000572205,09:20:30.001.jpg"
        print 'Writing to ', self.writefile_jpg
        if os.path.exists( self.writefile_jpg):
            os.remove( self.writefile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Iterate by time.'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, xaxis='elevation', yaxis='amp',
                          iteraxis='time', gridrows=2, gridcols=2)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.writefile_jpg) 
        
        
    def stest015(self):
        '''Plotms 15: Test that we iterate over averaged time on an elevation x amp plot.'''
        self.plotfile_jpg = self.outputDir + "testPlot015.jpg"
        self.writefile_jpg = self.outputDir + "testPlot015_Time09:18:59.9998,09:19:30.0002.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Iterate by averaged time.'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, xaxis='elevation', yaxis='amp',
                          iteraxis='time', gridrows=1, gridcols=2)
        self.assertTrue(self.res)
        self._checkPlotFile(90000, self.writefile_jpg)  
        
    
    def stest016(self):
        '''Plotms 16: Test if we can overplot scan and field on the left y-axis with time on the x-axis.'''
        self.plotfile_jpg = self.outputDir + "testPlot016.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create a (scan & field)/time plot'''
        self.res = plotms(vis=self.ms, overwrite=True, showgui=False, xaxis='time', yaxis=['scan','field'],
                          plotfile=self.plotfile_jpg, expformat='jpg',
                          showlegend=True, legendposition='lowerRight',
                          customsymbol=[True,True], symbolshape=['diamond','circle'], symbolsize=[5,5],
                          symbolcolor=['ff0000','00ff00'], symbolfill=['mesh3','mesh3'],
                          gridrows=1, gridcols=1)
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg) 
        
    def stest017(self):               
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
        
    def stest018(self):
        '''Plotms 18: Test if we can overplot (scan and field) vs time with one data set using the left axis and one data set using the right y-axis.'''
        self.plotfile_jpg = self.outputDir + "testPlot018.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create a (scan & field)/time plot'''
        self.res = plotms(vis=self.ms, overwrite=True, showgui=False, xaxis='time', yaxis=['scan','field'], yaxislocation=['left','right'],
                          plotfile=self.plotfile_jpg, expformat='jpg',
                          showlegend=True, legendposition='exteriorTop',
                          customsymbol=[True,True], symbolshape=['diamond','circle'], symbolsize=[5,5],
                          symbolcolor=['ff0000','00ff00'], symbolfill=['mesh3','mesh3'],
                          gridrows=1, gridcols=1)
        self.assertTrue(self.res)
        self._checkPlotFile(70000, self.plotfile_jpg) 
        
    def stest019(self):
        '''Plotms 19: Test if we can overplot (scan and field) vs time and iterate over antenna.'''
        self.plotfile_jpg = self.outputDir + "testPlot019.jpg"
        self.writefile_jpg = self.outputDir + "testPlot019_Antenna1@VLA:N7,2@VLA:W1,3@VLA:W2,4@VLA:E1.jpg"
        print 'Writing to ', self.writefile_jpg
        if os.path.exists( self.writefile_jpg):
            os.remove( self.writefile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create a (scan & field)/time plot with iteration over antenna'''
        self.res = plotms(vis=self.ms, overwrite=True, showgui=False,  xaxis='time', yaxis=['scan','field'], yaxislocation=['left','right'],
                          plotfile=self.plotfile_jpg, expformat='jpg',
                          gridrows=2, gridcols=2, iteraxis="antenna",
                          showlegend=True, legendposition='lowerLeft',
                          customsymbol=[True,True], symbolshape=['diamond','circle'], symbolsize=[5,5],
                          symbolcolor=['ff0000','00ff00'], symbolfill=['mesh3','mesh3'])
        self.assertTrue(self.res)
        self._checkPlotFile(213500, self.writefile_jpg) 
        
    def stest020(self):
        '''Plotms 20: Export an iteration plot with one plot per page (pipeline).'''
        self.plotfile = self.outputDir + "testPlot020.jpg"
        self.plotFiles = [self.outputDir + "testPlot020_Scan1.jpg",
                          self.outputDir + "testPlot020_Scan2_2.jpg",
                          self.outputDir + "testPlot020_Scan3_3.jpg",
                          self.outputDir + "testPlot020_Scan4_4.jpg",
                          self.outputDir + "testPlot020_Scan5_5.jpg",
                          self.outputDir + "testPlot020_Scan6_6.jpg",
                          self.outputDir + "testPlot020_Scan7_7.jpg"]
        
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
        self.res = plotms(vis=self.ms, plotfile=self.plotfile,
                          overwrite=True, showgui=False, expformat='jpg',  exprange='all',
                          iteraxis='scan', gridrows=1, gridcols=1)   
        self.assertTrue(self.res)
        
        '''Check each page got saved'''
        for  i in range(0, len(self.plotFiles)):
            self.assertTrue(os.path.exists(self.plotFiles[i]), 'Plot was not created')
            print 'Plot file size ', i, ' is ', os.path.getsize(self.plotFiles[i])
            self._checkPlotFile(60000, self.plotFiles[i]) 
        print  
        
    def stest021(self):
        '''Plotms 21: Test that model/data works.'''
        self.plotfile_jpg = self.outputDir + "testPlot021.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Set up the y-axis to use data/model'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, yaxis='amp', 
                          ydatacolumn='data/model', gridrows=1, gridcols=1)
        self.assertTrue(self.res)
        self._checkPlotFile(230000, self.plotfile_jpg)   
   
    def stest022(self):
        '''Plotms 22: Test that wt*amp works for x-and y-axis choices.'''
        self.plotfile_jpg = self.outputDir + "testPlot022.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Use a 2x1 grid and plot weight*am vs time on x-axis with one graph and y-axis on other'''
        self.res = plotms(vis=self.ms, gridrows=2, gridcols=1, 
                          showgui=False, yaxis='wt*amp', xaxis='time')
        
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          plotindex=1, rowindex=1, colindex=0, clearplots=False,
                          overwrite=True, showgui=False, xaxis='wt*amp', yaxis='time')
        self.assertTrue( self.res )
        self._checkPlotFile(220000, self.plotfile_jpg) 
        
    def stest023(self):
        '''Plotms 23: Test that corrected/model works for x-and y-amp/data choices.'''
        self.plotfile_jpg = self.outputDir + "testPlot023.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Use a 2x1 grid and plot amp vs time and vice versa.  Set to corrected/model on amp axis'''
        self.res = plotms(vis=self.ms, gridrows=2, gridcols=1,
                          showgui=False, yaxis='amp', ydatacolumn='corrected/model', xaxis='time')
        
        self.assertTrue(self.res)
        self.res = plotms(vis=self.ms, gridrows=2, gridcols=1, plotfile=self.plotfile_jpg, expformat='jpg', 
                          plotindex=1, rowindex=1, colindex=0, 
                          showgui=False, clearplots=False,
                          xaxis='amp', xdatacolumn='corrected/model', yaxis='time')
        self.assertTrue( self.res )
        self._checkPlotFile(249000, self.plotfile_jpg)    
        
    def stest024(self):
        '''Plotms 24: Test an invalid antenna selection does not crash plotms.'''
        self.plotfile_jpg = self.outputDir + "testPlot024.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        '''Verify error with antenna'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          gridrows=1, gridcols=1,
                          overwrite=True, showgui=False,
                          selectdata=True, antenna='ea22&&*' )  
        self.assertFalse( self.res)
        
        '''Use a 1x1 grid and plot amp vs time and vice versa.  Verify success without antenna'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          gridrows=1, gridcols=1, 
                          overwrite=True, showgui=False)
        self.assertTrue( self.res )
        self._checkPlotFile(175000, self.plotfile_jpg)   
        
       
        
        
    def stest025(self):
        '''Plotms 25: Test that we can overplot plots with two data sets.'''
        self.plotfile_jpg = self.outputDir + "testPlot025.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Use a 1x1 grid and plot scan vs time.'''
        self.res = plotms(vis=self.ms, gridrows=1, gridcols=1, 
                          overwrite=True, showgui=False, yaxis='scan',
                          rowindex=0, colindex=0, plotindex=0,
                          showlegend=True, legendposition='lowerRight',
                          customsymbol=[True], symbolshape=['diamond'], symbolsize=[3],
                          symbolcolor=['ff0000'], symbolfill=['mesh3'])
        self.assertTrue( self.res )
        
        '''Now add in field vs time and export it'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False, yaxis='field',
                          rowindex=0, colindex=0, plotindex=1, clearplots=False,
                          showlegend=True, legendposition='lowerRight',
                          customsymbol=[True], symbolshape=['circle'], symbolsize=[3],
                          symbolcolor=['00FF00'], symbolfill=['mesh3'])   
        self.assertTrue( self.res) 
        self._checkPlotFile(58000, self.plotfile_jpg)    
        
        
    def stest026(self):
        '''Plotms 26: Export an iteration plot consisting of two pages. Duplicate of test 6 except we use a right axis and a non-square grid.'''
        self.plotfile_jpg = self.outputDir + "testPlot026.jpg"
        self.plotfile1_jpg = self.outputDir + "testPlot026_Scan1,2,3,4,5,6.jpg"
        self.plotfile2_jpg = self.outputDir + "testPlot026_Scan7_2.jpg"
        print 'Writing to ', self.plotfile1_jpg, " and ", self.plotfile2_jpg
        if os.path.exists( self.plotfile1_jpg):
            os.remove( self.plotfile1_jpg)
        if os.path.exists( self.plotfile2_jpg):
            os.remove( self.plotfile2_jpg) 
              
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        '''Make 2 pages of 3x2 iteration plots over scan sharing common axes & scales'''
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg,
                          gridrows=3, gridcols=2, 
                          overwrite=True, showgui=False, expformat='jpg', 
                          exprange='all', iteraxis='scan'
                          ,xselfscale=True, yselfscale=True, 
                          xsharedaxis=True, ysharedaxis=True, 
                          yaxis=['amp'], yaxislocation=['right']
                          )   
        self.assertTrue(self.res)
        
        '''Check the first page got saved'''
        self.assertTrue(os.path.exists(self.plotfile1_jpg), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile1_jpg)
        self._checkPlotFile(100000, self.plotfile1_jpg)
        
        '''Check the second page got saved'''
        self.assertTrue(os.path.exists(self.plotfile2_jpg), 'Plot2 was not created')
        print 'Plot2 file size is ', os.path.getsize(self.plotfile2_jpg)
        self._checkPlotFile(64000, self.plotfile2_jpg)
        print    
        
    def stest027(self):
        '''Plotms 27: Test that we can do a 2x2 multiplot display. Consisting of single plots and overplots'''
        self.plotFile = self.outputDir + "testPlot027.jpg"
        self.plotFiles = [self.outputDir + "testPlot027_Scan1,2.jpg",
                          self.outputDir + "testPlot027_Scan3,4,5,6_2.jpg",
                          self.outputDir + "testPlot027_Scan7_3.jpg"]
        
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
        self.res = plotms(vis=self.ms, gridrows=2, gridcols=2,  
                          showgui=False, yaxis='scan',
                          rowindex=0, colindex=0, plotindex=0,
                          customsymbol=[True], symbolshape=['diamond'], symbolsize=[3],
                          symbolcolor=['ff0000'], symbolfill=['mesh3'])
        self.assertTrue( self.res )
        print 'Added overplot 1'
        
        '''Overplot field vs time.'''
        self.res = plotms(vis=self.ms,  
                          showgui=False, yaxis='field',
                          gridrows=2, gridcols=2,
                          rowindex=0, colindex=0, plotindex=1, clearplots=False,
                          showlegend=True, legendposition='lowerRight',
                          customsymbol=[True], symbolshape=['circle'], symbolsize=[3],
                          symbolcolor=['00FF00'], symbolfill=['mesh3'])   
        self.assertTrue( self.res)
        print 'Added overplot 2'  
        
        
        '''Now add a single basic plot in the first row, second column'''
        print
        print 'Adding single plot'
        self.res = plotms(vis=self.ms, gridrows=2, gridcols=2,
                          rowindex=0, colindex=1, plotindex=2, clearplots=False, 
                          showgui=False)   
        print 'Added single plot'
        
        '''Now add in an iteration plot and export it.'''
        self.res = plotms(vis=self.ms, 
                          overwrite=True, showgui=False, clearplots=False, expformat='jpg', 
                          iteraxis='scan',
                          gridrows=2, gridcols=2,
                          rowindex=1, colindex=0, plotindex=3,
                          plotfile=self.plotFile,exprange='all')   
        print 'Added iteration plot, now exporting'
        '''Check each page got saved'''
        for  i in range(0, len(self.plotFiles)):
            self.assertTrue(os.path.exists(self.plotFiles[i]), 'Plot was not created')
            print 'Plot file size ', i, ' is ', os.path.getsize(self.plotFiles[i])
            self._checkPlotFile(54000, self.plotFiles[i]) 
        print    
        
    def stest028(self):
        '''Plotms 28: Test generation of a single plot with two y-axes.'''
        self.plotfile_jpg = self.outputDir + "testPlot028.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Plot amp vs time and scan vs time. '''
        self.res = plotms(vis=self.ms, gridrows=1, gridcols=1, 
                          showgui=False, yaxis=['amp','scan'], 
                          ydatacolumn=['corrected/model'], 
                          yaxislocation=['left','right'],xaxis='time',
                          plotfile=self.plotfile_jpg, expformat='jpg')
        
        self.assertTrue(self.res)
        self._checkPlotFile(249000, self.plotfile_jpg)      
        
    def stest029(self):
        '''Plotms 29: Test that generation of a single plot with two y-axes using identical data returns false.'''
        self.plotfile_jpg = self.outputDir + "testPlot029.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Plot amp vs time and amp vs time. '''
        self.res = plotms(vis=self.ms, gridrows=1, gridcols=1,
                          showgui=False, yaxis=['amp','amp'], 
                          yaxislocation=['left','right'],xaxis='time',
                          plotfile=self.plotfile_jpg, expformat='jpg')
        
        self.assertFalse(self.res)  
        
    def stest030(self):
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
                          self.outputDir + "testPlot0310.jpg"]
        
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
                          showgui=False,            
                          plotfile=self.plotFiles[i])
            self.assertTrue(self.res)
            self._checkPlotFile(57000, self.plotFiles[i])
            
                  
    def stest031(self):
        print
        '''Plotms 31: Set a custom flagged plotting symbol'''
        self.plotfile_jpg = self.outputDir + "testPlot031.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):    
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          overwrite=True, showgui=False,
                          customflaggedsymbol=True, flaggedsymbolshape='diamond', flaggedsymbolsize=5,
                          flaggedsymbolcolor='00ff00', flaggedsymbolfill='mesh3',
                          gridrows=1, gridcols=1)   
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile_jpg)
        self._checkPlotFile(51000, self.plotfile_jpg)
        print 
        
    def stest032(self):
        '''Plotms 32: Pipeline no plot scenario.  Test was developed in response to CAS-6662.'''      
        self.plotFile = '/tmp/testPlot032.png'
        self.plotFile2 = '/tmp/testPlot0322.png'
       
        
        '''for  i in range(0, len(self.plotFiles)):'''
        if os.path.exists( self.plotFile):
            os.remove( self.plotFile) 
        if os.path.exists( self.plotFile2):
            os.remove( self.plotFile2)          
        
        
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
    
        self.res = plotms(vis='/home/uniblab/casa/trunk/test/Plotms/ftp.cv.nrao.edu/NRAO-staff/bkent/pipeline/13A-537.sb24066356.eb24324502.56514.05971091435.ms', 
                          xaxis='uvdist', yaxis='amp',ydatacolumn='model', gridrows=1, gridcols=1,
                          spw='0', scan='2,4,6,8', 
                          plotrange=[0,0,0,0], avgscan=False, avgbaseline=False, coloraxis='spw',
                          showgui=False,             
                          plotfile=self.plotFile)
        print 'Result from plotms=', self.res
        self.assertTrue(self.res)
        self._checkPlotFile(13000, self.plotFile)   
        
        self.res =plotms(vis='/home/uniblab/casa/trunk/test/Plotms/ftp.cv.nrao.edu/NRAO-staff/bkent/pipeline/13A-537.sb24066356.eb24324502.56514.05971091435.ms',xaxis="freq",xdatacolumn="",yaxis="amp",
ydatacolumn="corrected",selectdata=True,field="1",spw="",timerange="",
uvrange="",antenna="",scan="2,3",correlation="LL,RR",array="",
observation="",msselect="",averagedata=True,avgchannel="",avgtime="1e8s",
avgscan=True,avgfield=False,avgbaseline=False,avgantenna=False,avgspw=False,
scalar=False,transform=False,freqframe="",restfreq="",veldef="RADIO",
shift=[0.0, 0.0],extendflag=False,extcorr=False,extchannel=False,iteraxis="",
xselfscale=False,yselfscale=False,customsymbol=True,symbolshape="autoscaling",symbolsize=2,
symbolcolor="0000ff",symbolfill="fill",symboloutline=False,coloraxis="antenna2",customflaggedsymbol=False,
flaggedsymbolshape="nosymbol",flaggedsymbolsize=2,flaggedsymbolcolor="ff0000",flaggedsymbolfill="fill",flaggedsymboloutline=False,
plotrange=[],title="",xlabel="",ylabel="",showmajorgrid=False,
majorwidth=1,majorstyle="",majorcolor="B0B0B0",showminorgrid=False,minorwidth=1,
minorstyle="",minorcolor="D0D0D0",plotfile=self.plotFile2,expformat="", highres=False,overwrite=True, showgui=False)
        print 'Result from plotms2=', self.res
        self.assertTrue(self.res)
        self._checkPlotFile(15000, self.plotFile2)        
        '''self.res = plotms(vis='/home/uniblab/casa/trunk/test/Plotms/uid___A002_X5f231a_X179b.ms', 
                          xaxis='uvdist', yaxis='amp',ydatacolumn='model',
                          spw='0', scan='2,3,4,8',
                          plotrange=[0,0,0,0], avgscan=False, avgbaseline=False, coloraxis='spw',
                          showgui=False, clearplots=True,            
                          plotfile=self.plotFiles[1])
        self.assertTrue(self.res)
        self._checkPlotFile(50000, self.plotFiles[1]) '''     

    def stest033(self):
        print
        '''Plotms 33: CAS-6813, Iteration problem with two spws in a row'''
        plotFile = '/home/uniblab/casa/trunk/test/Plotms/Maw/maw.ms'
        self.plotfile1_jpg = "/tmp/testPlot033.jpg"
        self.plotfile2_jpg = "/tmp/testPlot0332.jpg"
    
        print 'Writing to ', self.plotfile1_jpg, ' ', self.plotfile2_jpg
        if os.path.exists( self.plotfile1_jpg):    
            os.remove( self.plotfile1_jpg)
        if os.path.exists( self.plotfile2_jpg):    
            os.remove( self.plotfile2_jpg)    
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        self.res = plotms(vis=plotFile,
                          spw='0', showgui=False,plotfile=self.plotfile1_jpg)  
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.plotfile1_jpg), 'Plot  1 was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile1_jpg)
        self._checkPlotFile(48000, self.plotfile1_jpg)
        
        self.res = plotms(vis=plotFile,yaxis='amp',
                          xaxis='freq',spw='2',avgtime='1e8',avgscan=True,
                          showgui=False,clearplots=True, plotfile=self.plotfile2_jpg)  
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.plotfile2_jpg), 'Plot  2 was not created')
        print 'Plot file 2 size is ', os.path.getsize(self.plotfile2_jpg)
        self._checkPlotFile(37000, self.plotfile2_jpg)
        
        print 

    def stest034(self):
        print
        '''Plotms 34: Tests whether an iteration plot can be placed in the first slot of a 2x2 grid, when specifying a plot index out of range.'''
        self.plotfile_jpg = self.outputDir + "testPlot034.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):    
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        self.res = plotms(vis=self.ms, plotindex=1,
                          rowindex=0, colindex=1, 
                          gridrows=2, gridcols=2, 
                          iteraxis='baseline',
                          showgui=False, plotfile=self.plotfile_jpg)  
        self.assertFalse(self.res)
        self.assertFalse(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        print 
        
    def stest035(self):
        print
        '''Plotms 34: See CAS-6844. Huge and fuzzy plot for large plot index'''
        self.plotfile_jpg = self.outputDir + "testPlot035.jpg"
        
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):    
            os.remove( self.plotfile_jpg)  
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        self.res = plotms(vis=self.ms, plotindex=0, 
                          gridrows=3, gridcols=3,
                          showgui=False, plotfile=self.plotfile_jpg)  
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile_jpg)
        self._checkPlotFile(52000, self.plotfile_jpg)
        
        '''Now increase the plot index to something huge- it should return false'''
        self.res = plotms(vis=self.ms, plotindex=400, 
                          gridrows=3, gridcols=3,
                          showgui=False)
        
        self.assertFalse(self.res)
        print  
        
    def stest036(self):
        print
        '''Plotms 36: See CAS-6857 Pixel symbol shape not selectable'''
        self.plotfile_jpg = self.outputDir + "testPlot036.jpg"
        
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):    
            os.remove( self.plotfile_jpg)
        
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        self.res = plotms(vis=self.ms, plotfile=self.plotfile_jpg, expformat='jpg', 
                          showgui=False,
                          customsymbol=True, symbolshape='pixel',
                          gridrows=1, gridcols=1)
        
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotfile_jpg)
        self._checkPlotFile(94000, self.plotfile_jpg)
       
        print            
 
    def stest037(self):
        print
        '''Plotms 37: Juergan's cookbook example'''
        self.plotfile_jpg = self.outputDir + "testPlot037.jpg"
        self.plotfile2_jpg = self.outputDir + "testPlot0372.jpg"
        self.plotfile3_jpg = self.outputDir + "testPlot0373.jpg"
        self.plotfile4_jpg = self.outputDir + "testPlot0374.jpg"
        self.plotfile5_jpg = self.outputDir + "testPlot0375.jpg"
        
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):    
            os.remove( self.plotfile_jpg)
        if os.path.exists( self.plotfile2_jpg):    
            os.remove( self.plotfile2_jpg)
        if os.path.exists( self.plotfile3_jpg):    
            os.remove( self.plotfile3_jpg)    
        if os.path.exists( self.plotfile4_jpg):    
            os.remove( self.plotfile4_jpg)     
        if os.path.exists( self.plotfile5_jpg):    
            os.remove( self.plotfile5_jpg)                
        
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        
        #self.res = plotms(vis=self.ms, gridrows=2, gridcols=2, showgui=False, plotfile=self.plotfile_jpg)
        #self.assertTrue(self.res)
        #self.assertTrue(os.path.exists(self.plotfile_jpg), 'Plot was not created')
        
        #Plot in the second column, first row, and define this plot as plotindex=0
        self.res = plotms(vis=self.ms, gridrows=2, gridcols=2,
                          colindex=1, rowindex=0,
                          showgui=False, plotfile=self.plotfile2_jpg,
                          customsymbol=True, symbolshape='diamond', symbolsize=5,
                          symbolcolor='ff0000')
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.plotfile2_jpg), 'Plot 2 was not created')
        
        #Overplot in the same panel.
        print ''
        print 'Test plot 3'
        self.res = plotms(vis='/home/uniblab/casa/trunk/test/Plotms/Maw/maw.ms', 
                          clearplots=False, 
                          plotindex=1, rowindex=0, colindex=1,
                          gridrows=2, gridcols=2,
                          yaxislocation='right', 
                          showgui=False, plotfile=self.plotfile3_jpg,
                          customsymbol=True, symbolshape='circle', symbolsize=5,
                          symbolcolor='00ff00')
        self.assertTrue(os.path.exists(self.plotfile3_jpg), 'Plot 3 was not created')
        
        #Define a second plot and give it a label plotindex=2, in the lower right corner
        print ''
        print 'Test plot 4'
        self.res = plotms(vis=self.ms, clearplots=False, 
                          plotindex=2, rowindex=1,colindex=1, 
                          gridrows=2, gridcols=2,
                          showgui=False, plotfile=self.plotfile4_jpg,
                          customsymbol=False, yaxislocation='')
        self.assertTrue(os.path.exists(self.plotfile4_jpg), 'Plot was not created')
        
        #Move the plot with the overplot one panel to the left. This requires clearing
        #the plots and rerunning the script specifications.
        self.res = plotms(vis=self.ms, gridrows=2, gridcols=2,
                          colindex=0, rowindex=0,
                          showgui=False, 
                          customsymbol=True, symbolshape='diamond', symbolsize=5,
                          symbolcolor='ff0000')
        self.res = plotms(vis='/home/uniblab/casa/trunk/test/Plotms/Maw/maw.ms', 
                          clearplots=False, 
                          plotindex=1, rowindex=0, colindex=0,
                          gridrows=2, gridcols=2,
                          yaxislocation='right', 
                          showgui=False,
                          customsymbol=True, symbolshape='circle', symbolsize=5,
                          symbolcolor='00ff00')
        self.res = plotms(vis=self.ms, clearplots=False, 
                          plotindex=2, rowindex=1,colindex=1, 
                          gridrows=2, gridcols=2,
                          showgui=False, plotfile=self.plotfile5_jpg,
                          customsymbol=False, yaxislocation='')
        self.assertTrue(os.path.exists(self.plotfile5_jpg), 'Plot was not created')
        
        #print 'Plot file size is ', os.path.getsize(self.plotfile_jpg)
        #self._checkPlotFile(94000, self.plotfile_jpg)
     
    def stest038(self):
        '''Plotms 38: Test for CAS-6975 overplotting problem.'''
        self.plotfile_jpg = self.outputDir + "testPlot038a.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create the first plot'''
        self.res = plotms(vis='/home/uniblab/tmp/uid___A002_X8666c7_X1fa.ms.split.cal',
                          spw="1",xaxis="freq",yaxis="phase",antenna="0&1",avgchannel="1e6",
                          field="0",correlation="XX", showgui=False)
        self.assertTrue( self.res )
        '''Do an overplot with a different file'''
        self.res = plotms(vis='/home/uniblab/tmp/uid___A002_X8666c7_X1fa.ms.split.cal.mapall',
                          spw="1",xaxis="freq",yaxis="phase",antenna="0&1",avgchannel="1e6",
                          field="0",correlation="XX", showgui=False, plotindex=1, clearplots=False, 
                          plotfile=self.plotfile_jpg )
        
        self.assertTrue(self.res)
        self._checkPlotFile(48000, self.plotfile_jpg)   
        print
        
    def stest039( self ):
        '''Plotms 39:'Making a plot with a 2x1 grid.'''
        self.plotfile_jpg = self.outputDir + "testPlot039.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create the first plot'''
        self.res = plotms(vis='/home/uniblab/tmp/uid___A002_X915f1c_X8be.ms.split.spw0chanavg',
                          xaxis="time",yaxis="amp",antenna="DV01",avgchannel="1000",
                          showgui=False, gridrows=2, rowindex=0, colindex=0)
        '''plotms(vis='/home/uniblab/tmp/uid___A002_X915f1c_X8be.ms.split.spw0chanavg',
                          xaxis="time",yaxis="amp",antenna="DV01",
                          gridrows=2, rowindex=0, colindex=0)'''
        self.assertTrue( self.res )
        '''Do an overplot with a different file'''
        self.res = plotms(vis='/home/uniblab/tmp/uid___A002_X915f1c_X8be.ms.split.spw0chanavg',
                          xaxis="time",yaxis="amp",antenna="!DV01",avgchannel="1000",
                          showgui=False, plotindex=1, clearplots=False, gridrows=2, rowindex=1, colindex = 0, 
                          plotfile=self.plotfile_jpg )
        '''plotms(vis='/home/uniblab/tmp/uid___A002_X915f1c_X8be.ms.split.spw0chanavg',
                          xaxis="time",yaxis="amp",antenna="!DV01",
                          gridrows=2, rowindex=1, colindex=0,plotindex=1,clearplots=False)'''
        
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)   
        print
        
    def stest040( self ):
        '''Plotms 40:  CAS-7043:  Cannot create two adjacent plots using plotms with plotindex'''
        self.plotfile_jpg = self.outputDir + "testPlot040.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create the first plot'''
        self.res = plotms(vis='/home/uniblab/tmp/uid___A002_X915f1c_X8be.ms.split.spw0chanavg',
                          xaxis="time",yaxis="amp",antenna="DV01",avgchannel="1000",
                          showgui=False,customsymbol=True,symbolshape='diamond',symbolsize=5,symbolcolor='00ff00')
        '''plotms(vis='/home/uniblab/tmp/uid___A002_X915f1c_X8be.ms.split.spw0chanavg',
                          xaxis="time",yaxis="amp",antenna="DV01",
                          customsymbol=True,symbolshape='diamond',
                          symbolsize=5,symbolcolor='00ff00')'''
        self.assertTrue( self.res )
        '''Do an overplot with a different file'''
        self.res = plotms(vis='/home/uniblab/tmp/uid___A002_X915f1c_X8be.ms.split.spw0chanavg',
                          xaxis="time",yaxis="amp",antenna="!DV01",avgchannel="1000",
                          showgui=False, plotindex=1, clearplots=False, customsymbol=True,symbolshape='diamond',symbolsize=1,symbolcolor='0000ff', 
                          plotfile=self.plotfile_jpg )
        '''plotms(vis='/home/uniblab/tmp/uid___A002_X915f1c_X8be.ms.split.spw0chanavg',
                          xaxis="time",yaxis="amp",antenna="!DV01",
                          plotindex=1, clearplots=False, 
                          customsymbol=True,symbolshape='diamond',symbolsize=1,symbolcolor='0000ff' )'''
        self.assertTrue(self.res)
        self._checkPlotFile(60000, self.plotfile_jpg)   
        print
        
    def stest041( self ):
        '''Plotms 41:  CAS-7046:  With custom flagged symbol=False, should not be able to change shape of points.'''
        self.plotfile_jpg = self.outputDir + "testPlot041.jpg"
        self.plotfile_jpg2 = self.outputDir + "testPlot0412.jpg"
        print 'Writing to ', self.plotfile_jpg
        if os.path.exists( self.plotfile_jpg):
            os.remove( self.plotfile_jpg)
        if ( os.path.exists( self.plotfile_jpg2 ) ):
             os.remove( self.plotfile_jpg2 )    
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create the first plot check that a custom flagged symbol has been set'''
        self.res = plotms(vis='/home/uniblab/casa/trunk/test/Plotms/titan.ms',
                          xaxis="time",yaxis="amp",avgtime="1000",
                          showgui=False,
                          customflaggedsymbol=True,flaggedsymbolshape='diamond',flaggedsymbolsize=5,flaggedsymbolcolor='00ff00',
                          plotfile = self.plotfile_jpg)
        self.assertTrue( self.res )
        self._checkPlotFile( 50000, self.plotfile_jpg )
        plotms(vis='/home/uniblab/casa/trunk/test/Plotms/titan.ms',
                          xaxis="time",yaxis="amp",avgtime="1000",
                          showgui = False,
                          customflaggedsymbol=False,flaggedsymbolshape='diamond',flaggedsymbolsize=5,flaggedsymbolcolor='00ff00',
                          plotfile = self.plotfile_jpg2)
        self.assertTrue( self.res )
        self._checkPlotFile( 50000, self.plotfile_jpg2 ) 
        print   
        
    def stest042( self ):
        '''Plotms 42:  CAS-7050:  (Pipeline) With iteration plots and bad selections you should be able to tell what plots are skipped.'''
        '''The expectation is the plot with the bad spw, 100, will be skipped'''
        plotfile="/home/uniblab/casa/trunk/test/Plotms/Maw/maw.ms"
        self.plotfile_jpg = self.outputDir + "testPlot042.jpg"
        self.plotfile_jpg1 = self.outputDir + "testPlot042_Spw10.jpg"
        self.plotfile_jpg2 = self.outputDir + "testPlot042_Spw11_2.jpg"
        self.plotfile_jpg3 = self.outputDir + "testPlot042_Spw12_3.jpg"
        self.plotfile_jpg4 = self.outputDir + "testPlot042_Spw13_4.jpg"
        print 'Writing to ', self.plotfile_jpg1, ', ', self.plotfile_jpg2, ', ',self.plotfile_jpg3, ', ',self.plotfile_jpg4
        if os.path.exists( self.plotfile_jpg1):
            os.remove( self.plotfile_jpg1)
        if ( os.path.exists( self.plotfile_jpg2 ) ):
             os.remove( self.plotfile_jpg2 ) 
        if ( os.path.exists( self.plotfile_jpg3 ) ):
             os.remove( self.plotfile_jpg3 )
        if ( os.path.exists( self.plotfile_jpg4 ) ):
             os.remove( self.plotfile_jpg4 )                  
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create the first plot check that a custom flagged symbol has been set'''
        self.res = plotms(vis=plotfile,
                          xaxis="time",yaxis="amp",spw="10,11,100,12,13",
                          showgui=False,iteraxis='spw',clearplots=True,
                          plotfile = self.plotfile_jpg, exprange='all')
        self.assertTrue( self.res )
        self._checkPlotFile( 95000, self.plotfile_jpg1 )
        self._checkPlotFile( 95000, self.plotfile_jpg2 )
        self._checkPlotFile( 94000, self.plotfile_jpg3 )
        self._checkPlotFile( 95000, self.plotfile_jpg4 )
        print
                
        
    def stest043(self):
        '''Plotms 43: Test that legend works with overplots'''
        '''Note when testing this, don't just check the file size, but look at the
           plot and make sure there is a legend there.'''
        self.plotFile = self.outputDir + "testPlot043.jpg"
        
        printMsg = 'Writing to ', self.plotFile
        
        if os.path.exists( self.plotFile):
            os.remove( self.plotFile )        
        
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        
        '''First put the multiplot in, scan,field vs time'''
        '''Plot scan vs time.'''
        self.res = plotms(vis=self.ms,  
                          showgui=False, yaxis='scan',
                          plotindex=0,
                          showlegend=True, legendposition='lowerRight',
                          customsymbol=[True], symbolshape=['diamond'], symbolsize=[3],
                          symbolcolor=['ff0000'], symbolfill=['mesh3'])
        self.assertTrue( self.res )
        print 'Added overplot 1'
        
        '''Overplot field vs time.'''
        self.res = plotms(vis=self.ms,  
                          showgui=False, yaxis='field',
                          plotindex=1, clearplots=False,
                          showlegend=True, legendposition='lowerRight',
                          customsymbol=[True], symbolshape=['circle'], symbolsize=[3],
                          symbolcolor=['00FF00'], symbolfill=['mesh3'],
                          plotfile=self.plotFile)   
        self.assertTrue( self.res)
        print 'Added overplot 2'  
      
        self.assertTrue(os.path.exists(self.plotFile), 'Plot was not created')
        print 'Plot file size is ', os.path.getsize(self.plotFile)
        self._checkPlotFile(55000, self.plotFile) 
        print
        
        
    def stest044( self ):
        '''Plotms 44:  CAS-7050:  (Pipeline) Check that if you specify the first 3 antenna and iterate over
           antenna then you only get 3 iteration plots'''
        plotfile="/home/uniblab/casa/trunk/test/Plotms/uid___A002_X5f231a_X179b.ms"
        self.plotfile_jpg = self.outputDir + "testPlot044.jpg"
        self.plotfile_jpg1 = self.outputDir + "testPlot044_AntennaDA43@A075.jpg"
        self.plotfile_jpg2 = self.outputDir + "testPlot044_AntennaDA45@A070_2.jpg"
        self.plotfile_jpg3 = self.outputDir + "testPlot044_AntennaDA46@A067_3.jpg"
        print 'Writing to ', self.plotfile_jpg1, ', ', self.plotfile_jpg2, ', ',self.plotfile_jpg3
        print 'But not writing any more!'
        if os.path.exists( self.plotfile_jpg1):
            os.remove( self.plotfile_jpg1)
        if ( os.path.exists( self.plotfile_jpg2 ) ):
             os.remove( self.plotfile_jpg2 ) 
        if ( os.path.exists( self.plotfile_jpg3 ) ):
             os.remove( self.plotfile_jpg3 )
                       
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create the plot and check that there are only 3 iterations'''
        self.res = plotms(vis=plotfile,
                          xaxis="time",yaxis="amp",antenna='0~2',
                          showgui=False,iteraxis='antenna',clearplots=True,avgantenna=True,
                          plotfile = self.plotfile_jpg, exprange='all')
        self.assertTrue( self.res )
        self._checkPlotFile( 130000, self.plotfile_jpg1 )
        self._checkPlotFile( 130000, self.plotfile_jpg2 )
        self._checkPlotFile( 130000, self.plotfile_jpg3 )
        fileCount = self._getFileCount( self.outputDir, "testPlot044_" )
        self.assertTrue( fileCount == 3 )
        print 
        
    def stest045( self ):
        '''Plotms 45:  CAS-7050:  (Pipeline) Check antenna with a range that includes only one valid antenna and iterate over
           antenna then you only get 1 iteration plot'''
        plotfile="/home/uniblab/casa/trunk/test/Plotms/uid___A002_X5f231a_X179b.ms"
        self.plotfile_jpg = self.outputDir + "testPlot045.jpg"
        self.plotfile_jpg1 = self.outputDir + "testPlot045_AntennaDV24@A088.jpg"
       
        print 'Writing to ', self.plotfile_jpg1
        print 'But hopefully not writing any more of them '
        if os.path.exists( self.plotfile_jpg1):
            os.remove( self.plotfile_jpg1)
         
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create the plot and check that there are only 3 iterations'''
        self.res = plotms(vis=plotfile,
                          xaxis="time",yaxis="amp",antenna='21~30',
                          showgui=False,iteraxis='antenna',clearplots=True,avgantenna=True,
                          plotfile = self.plotfile_jpg, exprange='all')
        self.assertTrue( self.res )
        self._checkPlotFile( 70000, self.plotfile_jpg1 )
        fileCount = self._getFileCount( self.outputDir, "testPlot045_" )
        self.assertTrue( fileCount == 1 )
        print 
        
    def stest046( self ):
        '''Plotms 46:  CAS-7050:  (Pipeline) Iterate on antenna, but don't select.  
             Make sure there are 22 iteration plots'''
        plotfile="/home/uniblab/casa/trunk/test/Plotms/uid___A002_X5f231a_X179b.ms"
        self.plotfile_jpg = self.outputDir + "testPlot046.jpg"
        #self.plotfile_jpg1 = self.outputDir + "testPlot045_AntennaDV24@A08.jpg"
       
        print 'Writing to ', self.plotfile_jpg
 
        #if os.path.exists( self.plotfile_jpg1):
        #    os.remove( self.plotfile_jpg1)
         
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create the plot and check that there are only 3 iterations'''
        self.res = plotms(vis=plotfile,
                          xaxis="time",yaxis="amp",
                          showgui=False,iteraxis='antenna',clearplots=True,avgantenna=True,
                          plotfile = self.plotfile_jpg, exprange='all')
        self.assertTrue( self.res )
        fileCount = self._getFileCount( self.outputDir, "testPlot046_" )
        self.assertTrue( fileCount == 22 )
        print         
        
    def stest047( self ):
        '''Plotms 47:  CAS-7050:  (Pipeline) Iterate on antenna with an empty selection.  
             Make sure there are NO iteration plots'''
        plotfile="/home/uniblab/casa/trunk/test/Plotms/uid___A002_X5f231a_X179b.ms"
        self.plotfile_jpg = self.outputDir + "testPlot047.jpg"
       
        print 'Writing NO plots '
         
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create the plot with empty antenna selection and check the result is false'''
        self.res = plotms(vis=plotfile,
                          xaxis="time",yaxis="amp", antenna="100,101,102",
                          showgui=False,iteraxis='antenna',clearplots=True,avgantenna=True,
                          plotfile = self.plotfile_jpg, exprange='all')
        self.assertFalse( self.res )
        print  
        
    def test048( self ):
        '''Plotms 48:  CAS-7074:  xsharedaxis needs to be a subparameter of global'''
        plotfile="/home/uniblab/casa/trunk/test/Plotms/Maw/maw.ms"
        self.plotfile_jpg = self.outputDir + "testPlot048.jpg"
       
         
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create the plot with empty antenna selection and check the result is false'''
        self.res = plotms(vis=plotfile,
                          iteraxis='antenna',clearplots=True, showgui=False, xsharedaxis=True,
                          plotfile = self.plotfile_jpg, exprange='all')
        self.assertFalse( self.res )
        print 
        
    def test049( self ):
        '''Plotms 49:  CAS-7074:  (Pipeline) Iterate on baseline with a 2x2 grid, check that shared axis works'''
        plotfile="/home/uniblab/casa/trunk/test/Plotms/uid___A002_X5f231a_X179b.ms"
        self.plotfile_jpg = self.outputDir + "testPlot049.jpg"
       
        print 'Writing to ', self.plotfile_jpg
        self._removeFiles( self.outputDir, "testPlot049_" )
         
        self.assertTrue(self.display.startswith(':'),'DISPLAY not set, cannot run test')
        time.sleep(5)
        '''Create the plot and check that there are only 3 iterations'''
        self.res = plotms(vis=plotfile,
                          xaxis="time",yaxis="amp", gridrows=2, gridcols=2,antenna='1~10',
                          showgui=False,iteraxis='baseline',clearplots=True,
                          plotfile = self.plotfile_jpg, exprange='all', scan='9',
                          xselfscale=True, yselfscale=True, xsharedaxis=True, ysharedaxis=True)
        self.assertTrue( self.res )
        fileCount = self._getFileCount( self.outputDir, "testPlot049_" )
        self.assertTrue( fileCount == 42 )
        print  
                                            
 
def suite():
    print 'Tests may fail due to DBUS timeout if the version of Qt is not at least 4.8.5'
    return [plotms_test1]
  
 
