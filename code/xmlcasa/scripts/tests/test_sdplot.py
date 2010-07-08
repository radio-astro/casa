import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from asap_init import * 
import unittest
import sha
import time

asap_init()
from sdplot import sdplot

class sdplot_test_plot(unittest.TestCase):
    """
    Test plot parameters only. Least data filterings and no averaging.
    
    The list of tests:
      test00    --- default parameters (raises an error)
      test01-03 --- possible plot types
      test04-07 --- possible axes (spectral plotting)
      test08-12 --- panelling and stacking (spectral plotting)
      test13-15 --- plot range control (spectral plotting)
      test16-19 --- plot style control (spectral plotting)
      test20-21 --- header control (spectral plotting)
      test22-23 --- plot layout control (spectral plotting)

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdaverage)
      sdaverage(sdfile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile=self.sdfile)
    """
    # Input and output names
    sdfile='OrionS_rawACSmod_cal2123.asap'
    fig='sdplottest.png'
    fig_minsize = 20000
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdplot/'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.sdfile)):
            shutil.copytree(self.datapath+self.sdfile, self.sdfile)

        default(sdplot)

    def tearDown(self):
        if (os.path.exists(self.sdfile)):
            os.system('rm -rf ' + self.sdfile)
        os.system('rm -rf ' + self.fig)

    def _checkPlotFile(self):
        self.assertTrue(os.path.exists(self.fig))
        self.assertTrue(os.path.isfile(self.fig))
        #print "The size of figure = ",os.path.getsize(self.fig)
        self.assertTrue(os.path.getsize(self.fig) > self.fig_minsize)


    def testplot00(self):
        """Test 0: Default parameters"""
        self.res=sdplot()
        self.assertFalse(self.res)        

    def testplot01(self):
        """Test 1: test plot type --- az/el"""
        #pl.ion()
        sdfile=self.sdfile
        plottype='azel'
        header=False
        self.res=sdplot(sdfile=self.sdfile,plottype=plottype,header=header,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()
        

    def testplot02(self):
        """Test 2: test plot type --- pointing"""
        #pl.ion()
        sdfile=self.sdfile
        plottype='pointing'
        header=False
        self.res=sdplot(sdfile=self.sdfile,plottype=plottype,header=header,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot03(self):
        """
        Test 3: test plot type --- total power (row # vs total power)
        """
        #pl.ion()
        sdfile=self.sdfile
        plottype='totalpower'
        header=False
        self.res=sdplot(sdfile=sdfile,plottype=plottype,header=header,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot04(self):
        """
        Test 4: test possible axes for spectral plotting --- specunit='channel'
        """
        sdfile=self.sdfile
        specunit='channel'
        fluxunit='K'
        stack='p'
        panel='i'
        iflist=[0,2]
        self.res=sdplot(sdfile=sdfile,specunit=specunit,fluxunit=fluxunit,
                        stack=stack,panel=panel,iflist=iflist,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot05(self):
        """
        Test 5: test possible axes for spectral plotting --- specunit='GHz'
        """
        sdfile=self.sdfile
        specunit='GHz'
        fluxunit='K'
        stack='p'
        panel='i'
        iflist=[0,2]
        self.res=sdplot(sdfile=sdfile,specunit=specunit,fluxunit=fluxunit,
                        stack=stack,panel=panel,iflist=iflist,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot06(self):
        """
        Test 6: test possible axes for spectral plotting --- specunit='km/s'
        """
        sdfile=self.sdfile
        specunit='km/s'
        fluxunit='K'
        stack='p'
        panel='i'
        iflist=[0,2]
        self.res=sdplot(sdfile=sdfile,specunit=specunit,fluxunit=fluxunit,
                        stack=stack,panel=panel,iflist=iflist,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot07(self):
        """
        Test 7: test possible axes for spectral plotting --- fluxunit='Jy'
        """
        sdfile=self.sdfile
        specunit='channel'
        fluxunit='Jy'
        stack='p'
        panel='i'
        iflist=[0,2]
        self.res=sdplot(sdfile=sdfile,specunit=specunit,fluxunit=fluxunit,
                        stack=stack,panel=panel,iflist=iflist,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot08(self):
        """
        Test 8: test panelling and stacking (spectral plotting) --- panel='pol', stack='beam' (2px2s)
        """
        sdfile=self.sdfile
        panel='pol'
        stack='scan'
        self.res=sdplot(sdfile=sdfile,stack=stack,panel=panel,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot09(self):
        """
        Test 9: test panelling and stacking (spectral plotting) --- panel='beam', stack='if' (2px4s)
        """
        sdfile=self.sdfile
        panel='scan'
        stack='if'
        self.res=sdplot(sdfile=sdfile,stack=stack,panel=panel,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot10(self):
        """
        Test 10: test panelling and stacking (spectral plotting) --- panel='if', stack='time' (4px8s)
        """
        sdfile=self.sdfile
        panel='if'
        stack='time'
        self.res=sdplot(sdfile=sdfile,stack=stack,panel=panel,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot11(self):
        """
        Test 11: test panelling and stacking (spectral plotting) --- panel='time', stack='scan' (8px1s)
        """
        sdfile=self.sdfile
        panel='time'
        stack='beam'
        self.res=sdplot(sdfile=sdfile,stack=stack,panel=panel,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot12(self):
        """
        Test 12: test panelling and stacking (spectral plotting) --- panel='scan', stack='pol' (1px2s)
        """
        sdfile=self.sdfile
        panel='beam'
        stack='pol'
        self.res=sdplot(sdfile=sdfile,stack=stack,panel=panel,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot13(self):
        """
        Test 13: test plot range control for spectral plotting --- abscissa
        """
        sdfile=self.sdfile
        iflist=[0]
        sprange=[3500,5000]
        self.res=sdplot(sdfile=sdfile,iflist=iflist,sprange=sprange,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot14(self):
        """
        Test 14: test plot range control for spectral plotting --- ordinate
        """
        sdfile=self.sdfile
        iflist=[0]
        flrange=[0.,6.]
        self.res=sdplot(sdfile=sdfile,iflist=iflist,flrange=flrange,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot15(self):
        """
        Test 15: test plot range control for spectral plotting --- both
        """
        sdfile=self.sdfile
        iflist=[0]
        sprange=[3500,5000]
        flrange=[0.,6.]
        self.res=sdplot(sdfile=sdfile,iflist=iflist,sprange=sprange,
                        flrange=flrange,plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot16(self):
        """
        Test 16: plot style control (spectral plotting) --- histogram
        """
        sdfile=self.sdfile
        iflist=[0]
        sprange=[3900,4300]
        histogram=True
        self.res=sdplot(sdfile=sdfile,iflist=iflist,sprange=sprange,
                        histogram=histogram,plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot17(self):
        """
        Test 17: plot style control (spectral plotting) --- colormap
        """
        sdfile=self.sdfile
        iflist=[0]
        sprange=[3900,4300]
        colormap="pink orange"
        self.res=sdplot(sdfile=sdfile,iflist=iflist,sprange=sprange,
                        colormap=colormap,plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot18(self):
        """
        Test 18: plot style control (spectral plotting) --- linewidth
        """
        sdfile=self.sdfile
        iflist=[0]
        sprange=[3900,4300]
        linewidth=3
        self.res=sdplot(sdfile=sdfile,iflist=iflist,sprange=sprange,
                        linewidth=linewidth,plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot19(self):
        """
        Test 19: plot style control (spectral plotting) --- linestyle
        """
        sdfile=self.sdfile
        iflist=[0]
        sprange=[3900,4300]
        linewidth=2
        linestyles="dotted dashdot"
        self.res=sdplot(sdfile=sdfile,iflist=iflist,sprange=sprange,
                        linewidth=linewidth,linestyles=linestyles,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot20(self):
        """
        Test 20: header control (spectral plotting) --- larger fontsize
        """
        sdfile=self.sdfile
        iflist=[0,2]
        headsize=11
        self.res=sdplot(sdfile=sdfile,iflist=iflist,headsize=headsize,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot21(self):
        """
        Test 21: header control (spectral plotting) --- header off
        """
        sdfile=self.sdfile
        iflist=[0,2]
        header=False
        self.res=sdplot(sdfile=sdfile,iflist=iflist,header=header,
                        plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot22(self):
        """
        Test 22: plot layout control (spectral plotting) --- panel layout
        """
        sdfile=self.sdfile
        iflist=[0,2]
        plotstyle=True
        layout=[0.15,0.3,0.85,0.7,0.25,0.25]
        self.res=sdplot(sdfile=sdfile,iflist=iflist,plotstyle=plotstyle,
                        layout=layout,plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

    def testplot23(self):
        """
        Test 23: plot layout control (spectral plotting) --- legend location (left bottom)
        """
        sdfile=self.sdfile
        iflist=[0,2]
        plotstyle=True
        legendloc=3
        self.res=sdplot(sdfile=sdfile,iflist=iflist,plotstyle=plotstyle,
                        legendloc=legendloc,plotfile=self.fig)
        self.assertEqual(self.res,None)
        self._checkPlotFile()

def suite():
    return [sdplot_test_plot]
