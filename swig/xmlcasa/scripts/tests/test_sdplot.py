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
import filecmp

asap_init()
from sdplot import sdplot
import asap as sd

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
      test22-23,28 --- plot layout control (spectral plotting)
      test24-25 --- row panelling or stacking (spectral plotting)
      test26-27 --- flagg application

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdaverage)
      sdaverage(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile=self.infile)
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdplot/'
    # Input and output names
    infile = 'OrionS_rawACSmod_cal2123.asap'
    figroot = 'sdplot_test'
    figpost = '.png'
    fig=None
    # Minimum data size of figure allowed
    fig_minsize = 20000
    # directories to save figure
    prevdir = datapath+'prev_sdplot'
    currdir = os.path.abspath('curr_sdplot')
    # GUI settings
    nogui = True
    oldgui = sd.rcParams['plotter.gui']  # store previous GUI setting
    # save figure for reference?
    saveref = True

    def setUp(self):
        # switch off GUI
        if self.nogui:
            sd.rcParams['plotter.gui'] = False
            sd.plotter.__init__()
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        if not (os.path.exists(self.currdir)):
            os.makedirs(self.currdir)
        # Create a directory to save the figure for future comparison
        if (not os.path.exists(self.prevdir)) and self.saveref:
            try: os.makedirs(self.prevdir)
            except OSError:
                msg = "Unable to create directory, "+self.prevdir+".\n"
                msg += "Plot figures will remain in "+self.currdir
                casalog.post(msg,'WARN')
        
        default(sdplot)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        #if self.fig and (os.path.exists(self.fig)):
        #    os.remove(self.fig)
        if self.oldgui:
            sd.rcParams['plotter.gui'] = True
            sd.plotter.__init__()

    def _checkOutFile(self,compare=True):
        self.assertTrue(os.path.exists(self.fig))
        self.assertTrue(os.path.isfile(self.fig))
        self.assertTrue(os.path.getsize(self.fig) > self.fig_minsize)
        prevfig = self.prevdir+'/'+self.fig
        currfig = self.currdir+'/'+self.fig
        if compare and os.path.exists(prevfig):
            # The unit test framework doesn't allow saving previous results
            # This test is not run.
            msg = "Compareing with previous plot:\n"
            msg += " (prev) %s\n (new) %s" % (prevfig,os.path.abspath(self.fig))
            casalog.post(msg,'INFO')
            self.assertTrue(filecmp.cmp(prevfig,os.path.abspath(self.fig),shallow=False))
        shutil.move(self.fig,currfig)

        # Save the figure for future comparison if possible.
        if os.path.exists(self.prevdir) and self.saveref:
            try:
                casalog.post("copying %s to %s" % (currfig,prevfig),'INFO')
                shutil.copyfile(currfig, prevfig)
            except IOError:
                msg = "Unable to copy Figure '"+self.fig+"' to "+self.prevdir+".\n"
                msg += "The figure remains in "+self.prevdir
                casalog.post(msg,'WARN')


    def testplot00(self):
        """Test 0: Default parameters"""
        result=sdplot()
        self.assertFalse(result)   

    def testplot01(self):
        """Test 1: test plot type --- az/el"""
        tid = "01"
        infile=self.infile
        self.fig = self.figroot+tid+self.figpost
        plottype='azel'
        header=False
        result=sdplot(infile=self.infile,plottype=plottype,header=header,
                        outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot02(self):
        """Test 2: test plot type --- pointing"""
        tid = "02"
        infile=self.infile
        self.fig = self.figroot+tid+self.figpost
        plottype='pointing'
        header=False
        result=sdplot(infile=self.infile,plottype=plottype,header=header,
                        outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot03(self):
        """
        Test 3: test plot type --- total power (row # vs total power)
        """
        #pl.ion()
        tid = "03"        
        infile=self.infile
        self.fig = self.figroot+tid+self.figpost
        plottype='totalpower'
        header=False
        result=sdplot(infile=infile,plottype=plottype,header=header,
                        outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot04(self):
        """
        Test 4: test possible axes for spectral plotting --- specunit='channel'
        """
        tid = "04"
        infile=self.infile
        self.fig = self.figroot+tid+self.figpost
        specunit='channel'
        fluxunit='K'
        stack='p'
        panel='i'
        iflist=[0,2]
        header=False
        result=sdplot(infile=infile,specunit=specunit,fluxunit=fluxunit,
                        stack=stack,panel=panel,iflist=iflist,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot05(self):
        """
        Test 5: test possible axes for spectral plotting --- specunit='GHz'
        """
        tid = "05"
        infile=self.infile
        self.fig = self.figroot+tid+self.figpost
        specunit='GHz'
        fluxunit='K'
        stack='p'
        panel='i'
        iflist=[0,2]
        header=False
        result=sdplot(infile=infile,specunit=specunit,fluxunit=fluxunit,
                        stack=stack,panel=panel,iflist=iflist,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot06(self):
        """
        Test 6: test possible axes for spectral plotting --- specunit='km/s'
        """
        tid = "06"
        infile=self.infile
        self.fig = self.figroot+tid+self.figpost
        specunit='km/s'
        fluxunit='K'
        stack='p'
        panel='i'
        iflist=[0,2]
        header=False
        result=sdplot(infile=infile,specunit=specunit,fluxunit=fluxunit,
                        stack=stack,panel=panel,iflist=iflist,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot07(self):
        """
        Test 7: test possible axes for spectral plotting --- fluxunit='Jy'
        """
        tid = "07"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        specunit='channel'
        fluxunit='Jy'
        stack='p'
        panel='i'
        iflist=[0,2]
        header=False
        result=sdplot(infile=infile,specunit=specunit,fluxunit=fluxunit,
                        stack=stack,panel=panel,iflist=iflist,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot08(self):
        """
        Test 8: test panelling and stacking (spectral plotting) --- panel='pol', stack='beam' (2px2s)
        """
        tid = "08"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        panel='pol'
        stack='scan'
        header=False
        result=sdplot(infile=infile,stack=stack,panel=panel,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot09(self):
        """
        Test 9: test panelling and stacking (spectral plotting) --- panel='beam', stack='if' (2px4s)
        """
        tid = "09"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        panel='scan'
        stack='if'
        header=False
        result=sdplot(infile=infile,stack=stack,panel=panel,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot10(self):
        """
        Test 10: test panelling and stacking (spectral plotting) --- panel='if', stack='time' (4px8s)
        """
        tid = "10"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        panel='if'
        stack='time'
        header=False
        result=sdplot(infile=infile,stack=stack,panel=panel,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot11(self):
        """
        Test 11: test panelling and stacking (spectral plotting) --- panel='time', stack='scan' (8px1s)
        """
        tid = "11"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        panel='time'
        stack='beam'
        header=False
        result=sdplot(infile=infile,stack=stack,panel=panel,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot12(self):
        """
        Test 12: test panelling and stacking (spectral plotting) --- panel='scan', stack='pol' (1px2s)
        """
        tid = "12"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        panel='beam'
        stack='pol'
        header=False
        result=sdplot(infile=infile,stack=stack,panel=panel,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot13(self):
        """
        Test 13: test plot range control for spectral plotting --- abscissa
        """
        tid = "13"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        iflist=[0]
        sprange=[3500,5000]
        header=False
        result=sdplot(infile=infile,iflist=iflist,sprange=sprange,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot14(self):
        """
        Test 14: test plot range control for spectral plotting --- ordinate
        """
        tid = "14"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        iflist=[0]
        flrange=[0.,6.]
        header=False
        result=sdplot(infile=infile,iflist=iflist,flrange=flrange,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot15(self):
        """
        Test 15: test plot range control for spectral plotting --- both
        """
        tid = "15"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        iflist=[0]
        sprange=[3500,5000]
        flrange=[0.,6.]
        header=False
        result=sdplot(infile=infile,iflist=iflist,sprange=sprange,
                        header=header,flrange=flrange,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot16(self):
        """
        Test 16: plot style control (spectral plotting) --- histogram
        """
        tid = "16"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        iflist=[0]
        sprange=[3900,4300]
        histogram=True
        header=False
        result=sdplot(infile=infile,iflist=iflist,sprange=sprange,
                        header=header,histogram=histogram,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot17(self):
        """
        Test 17: plot style control (spectral plotting) --- colormap
        """
        tid = "17"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        iflist=[0]
        sprange=[3900,4300]
        colormap="pink orange"
        header=False
        result=sdplot(infile=infile,iflist=iflist,sprange=sprange,
                        header=header,colormap=colormap,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot18(self):
        """
        Test 18: plot style control (spectral plotting) --- linewidth
        """
        tid = "18"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        iflist=[0]
        sprange=[3900,4300]
        linewidth=3
        header=False
        result=sdplot(infile=infile,iflist=iflist,sprange=sprange,
                        header=header,linewidth=linewidth,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot19(self):
        """
        Test 19: plot style control (spectral plotting) --- linestyle
        """
        tid = "19"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        iflist=[0]
        sprange=[3900,4300]
        linewidth=2
        linestyles="dotted dashdot"
        header=False
        result=sdplot(infile=infile,iflist=iflist,sprange=sprange,
                        linewidth=linewidth,linestyles=linestyles,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot21(self):
        """
        Test 21: header control (spectral plotting) --- header on
        """
        tid = "21"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        iflist=[0,2]
        result=sdplot(infile=infile,iflist=iflist,
                        outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile(compare=False)

    def testplot20(self):
        """
        Test 20: header control (spectral plotting) --- larger fontsize
        """
        tid = "20"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        iflist=[0,2]
        headsize=11
        result=sdplot(infile=infile,iflist=iflist,headsize=headsize,
                        outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile(compare=False)

    def testplot22(self):
        """
        Test 22: plot layout control (spectral plotting) --- panel margin
        """
        tid = "22"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        iflist=[0,2]
        plotstyle=True
        margin=[0.15,0.3,0.85,0.7,0.25,0.25]
        header=False
        result=sdplot(infile=infile,iflist=iflist,plotstyle=plotstyle,
                        margin=margin,header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot23(self):
        """
        Test 23: plot layout control (spectral plotting) --- legend location (left bottom)
        """
        tid = "23"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        iflist=[0,2]
        plotstyle=True
        legendloc=3
        header=False
        result=sdplot(infile=infile,iflist=iflist,plotstyle=plotstyle,
                        legendloc=legendloc,header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot24(self):
        """
        Test 24: test panelling and stacking (spectral plotting) --- panel='row' (16px1s)
        """
        tid = "24"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        panel='row'
        header=False
        result=sdplot(infile=infile,panel=panel,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot25(self):
        """
        Test 25: test panelling and stacking (spectral plotting) --- panel='scan', stack='row' (1px16s)
        """
        tid = "25"
        self.fig = self.figroot+tid+self.figpost
        infile=self.infile
        stack='row'
        header=False
        result=sdplot(infile=infile,stack=stack,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot26(self):
        """
        Test 26: test handling of FLAGROW
        """
        from asap import scantable
        tid = "26"
        self.fig = self.figroot+tid+self.figpost
        ### flag rows=[0,3,6,9,12,15]
        infile = "orion_flagrow.asap"
        scan = scantable(filename=self.infile,average=False)
        scan.flag_row(rows=[0,3,6,9,12,15])
        scan.save(name=infile,format='ASAP')
        #
        panel='row'
        stack='scan'
        header=False
        result=sdplot(infile=infile,panel=panel,stack=stack,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot27(self):
        """
        Test 27: test handling of FLAGTRA
        """
        from asap import scantable
        tid = "27"
        self.fig = self.figroot+tid+self.figpost
        ### flag channels [7168,8191]
        infile = "orion_flagchan7168to8191.asap"
        scan = scantable(filename=self.infile,average=False)
        scan.set_unit('channel')
        msk = scan.create_mask([7168,8191])
        scan.flag(mask=msk)
        scan.save(name=infile,format='ASAP')
        #
        panel='row'
        header=False
        result=sdplot(infile=infile,panel=panel,
                        header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

    def testplot28(self):
        """
        Test 28: plot layout control (spectral plotting) --- panel layout
        """
        tid = "28"
        self.fig = self.figroot+tid+self.figpost
        infile = self.infile
        iflist = [0,2]
        panel = 'r'
        plotstyle = True
        subplot = 24
        header = False
        result = sdplot(infile=infile,iflist=iflist,plotstyle=plotstyle,
                        subplot=subplot,header=header,outfile=self.fig)
        self.assertEqual(result,None)
        self._checkOutFile()

def suite():
    return [sdplot_test_plot]
