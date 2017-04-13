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

# Paths for data
datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/"

# Read the data sets from another directory
if os.environ.has_key('TEST_DATADIR'):  
    DATADIR = str(os.environ.get('TEST_DATADIR'))
    if os.path.isdir(DATADIR):
        datapath = DATADIR
    else:
        print 'WARN: directory '+DATADIR+' does not exist'

print 'plotcal tests will use data from '+datapath

class plotcal_test_base(unittest.TestCase):

    Btabledir = '/bandpass/'
    Btable  = 'ngc5921.ref1a.bcal'
    Bms  = 'ngc5921.ms'
    Gtabledir = '/gaincal/'
    Gtable  = "ngc5921.ref1a.gcal"
    Ttabledir = "/wvrgcal/input/"
    Ttable  = "wvrgcalctest.W"
    TSYStabledir = "/flagdata/"
    TSYStable  = "X7ef.tsys"
    fig = "/tmp/testplotcal.png"

    def setUp(self):
        self.removePlotfile()
        default(plotcal)

    def setUpBtable(self):
        if (os.path.exists(self.Btable)):
            shutil.rmtree(self.Btable)
        shutil.copytree(datapath+self.Btabledir+self.Btable, self.Btable)

    def setUpGtable(self):
        if (os.path.exists(self.Gtable)):
            shutil.rmtree(self.Gtable)
        shutil.copytree(datapath+self.Gtabledir+self.Gtable, self.Gtable)

    def setUpTtable(self):
        if (os.path.exists(self.Ttable)):
            shutil.rmtree(self.Ttable)
        shutil.copytree(datapath+self.Ttabledir+self.Ttable, self.Ttable)

    def setUpTSYStable(self):
        if (os.path.exists(self.TSYStable)):
            shutil.rmtree(self.TSYStable)
        shutil.copytree(datapath+self.TSYStabledir+self.TSYStable, self.TSYStable)

    def setUpBms(self):
        if (os.path.exists(self.Bms)):
            shutil.rmtree(self.Bms)
        shutil.copytree(datapath+self.Btabledir+self.Bms, self.Bms)

    def tearDown(self):
        self.removePlotfile()
        if (os.path.exists(self.Btable)):
            shutil.rmtree(self.Btable)
        if (os.path.exists(self.Gtable)):
            shutil.rmtree(self.Gtable)
        if (os.path.exists(self.Ttable)):
            shutil.rmtree(self.Ttable)
        if (os.path.exists(self.TSYStable)):
            shutil.rmtree(self.TSYStable)
        if (os.path.exists(self.Bms)):
            shutil.rmtree(self.Bms)

    def checkPlotfile(self, plotfile, minSize, maxSize=None):
        self.assertTrue(os.path.isfile(plotfile), "Plot was not created")
        plotSize = os.path.getsize(plotfile)
        print plotfile, 'file size is', plotSize
        self.assertGreater(plotSize, minSize)
        if maxSize:
            self.assertLess(plotSize, maxSize)

    def checkNoPlotfile(self, plotfile):
        self.assertFalse(os.path.isfile(plotfile))

    def removePlotfile(self, plotfile=None):
        try:
            if not plotfile:
                plotfile = self.fig
            os.remove(plotfile)
        except OSError:  # "No such file or directory"
            pass
            
# ------------------------------------------------------------------------
 
class test_basic(plotcal_test_base):
    ''' Test basic calibration plots '''

    def test_btable_plot(self):
        '''test_btable_plot: plot B table'''
        self.setUpBtable()
        plotcal(caltable=self.Btable, showgui=False, figfile=self.fig) 
        self.checkPlotfile(self.fig, 75000)
        
    def test_gtable_plot(self):
        '''test_gtable_plot: plot G table'''
        self.setUpGtable()
        plotcal(caltable=self.Gtable, showgui=False, figfile=self.fig) 
        self.checkPlotfile(self.fig, 20000)
        
    def test_ttable_plot(self):
        '''test_ttable_plot: plot T table'''
        self.setUpTtable()
        plotcal(caltable=self.Ttable, showgui=False, figfile=self.fig) 
        self.checkPlotfile(self.fig, 15000)
        
    def test_tsys_plot(self):
        '''test_tsys_plot: plot TSYS table'''
        self.setUpTSYStable()
        plotcal(caltable=self.TSYStable, showgui=False, figfile=self.fig) 
        self.checkPlotfile(self.fig, 35000)
        
# ------------------------------------------------------------------------

class test_axes(plotcal_test_base):
    ''' Test xaxis and yaxis options '''

    def test_xaxis(self):
        '''test xaxis options with G table'''
        self.setUpGtable()
        xaxes = ['time', 'chan', 'freq', 'antenna', 'antenna1', 'antenna2',
                 'scan', 'amp', 'phase', 'real', 'imag', 'snr', 'delay',
                 'spgain']
        for axis in xaxes:
            print "Testing", axis 
            plotcal(caltable=self.Gtable, xaxis=axis, showgui=False, 
                    figfile=self.fig)
            # invalid axes for G table
            if axis in ['tsys', 'delay', 'spgain']:
                self.checkNoPlotfile(self.fig)
            else:
                self.checkPlotfile(self.fig, 15000)
                self.removePlotfile()

    def test_yaxis(self):
        '''test yaxis options with G table'''
        self.setUpGtable()
        yaxes = ['amp', 'phase', 'real', 'imag', 'snr', 'antenna', 'antenna1',
                 'antenna2', 'scan', 'tsys', 'delay', 'spgain', 'tec']
        for axis in yaxes:
            print "Testing", axis 
            plotcal(caltable=self.Gtable, yaxis=axis, showgui=False, 
                    figfile=self.fig)
            # invalid axes for G table
            if axis in ['tsys', 'delay', 'spgain', 'tec']:
                self.checkNoPlotfile(self.fig)
            else:
                self.checkPlotfile(self.fig, 10000)
                self.removePlotfile()

    def test_tsys_axis(self):
        '''test tsys axis with TSYS and G tables'''
        self.setUpGtable()
        self.setUpTSYStable()
        # tsys table works
        plotcal(caltable=self.TSYStable, yaxis='tsys', xaxis='antenna',
                showgui=False, figfile=self.fig) 
        self.checkPlotfile(self.fig, 20000)
        self.removePlotfile()
        # this should fail! -- no figfile
        plotcal(caltable=self.Gtable, yaxis='tsys', xaxis='antenna',
                showgui=False, figfile=self.fig) 
        self.checkNoPlotfile(self.fig)

    def test_xaxis_yaxis(self):
        '''set both axes for real vs imag plot'''
        self.setUpBtable()
        plotcal(caltable=self.Btable, showgui=False, xaxis='imag',
                yaxis='real', figfile=self.fig)
        self.checkPlotfile(self.fig, 115000)

# ------------------------------------------------------------------------

class test_selection(plotcal_test_base):
    ''' Test selection options '''
    # NOTE selection does not work when no MS

    def test_poln(self):
        '''test poln selection'''
        self.setUpBtable()
        self.setUpBms()
        polns = ['RL', 'R', 'L', 'X', 'Y', '/']
        # with most cal tables you can't tell if circular or linear
        # plotcal works with either (plots 1st or 2nd axis)
        for poln in polns:
            print "Select poln", poln
            plotcal(caltable=self.Btable, poln=poln, showgui=False, 
                    figfile=self.fig)
            self.checkPlotfile(self.fig, 60000)
            self.removePlotfile()

    def test_field(self):
        '''test field selection'''
        self.setUpBtable()
        self.setUpBms()
        fields = ['0', '1', '2']
        for field in fields:
            print "Select field", field
            plotcal(caltable=self.Btable, field=field, showgui=False, 
                    figfile=self.fig)
            if field=='0':
                self.checkPlotfile(self.fig, 20000)
                self.removePlotfile()
            else:
                self.checkNoPlotfile(self.fig)

    def test_antenna(self):
        '''test antenna selection'''
        self.setUpBtable()
        self.setUpBms()
        plotcal(caltable=self.Btable, antenna='VA04,VA08,VA12,VA16', 
                showgui=False, figfile=self.fig)
        self.checkPlotfile(self.fig, 30000)

    def test_spw(self):
        '''test spw selection'''
        self.setUpBtable()
        self.setUpBms()
        plotcal(caltable=self.Btable, spw='0',
                showgui=False, figfile=self.fig)
        self.checkPlotfile(self.fig, 15000)

    def test_timerange(self):
        '''test timerange selection'''
        self.setUpBtable()
        self.setUpBms()
        plotcal(caltable=self.Btable, 
                timerange="1995/04/13/09:15:00~1995/04/13/09:25:00",
                showgui=False, figfile=self.fig)
        self.checkPlotfile(self.fig, 75000)

# ------------------------------------------------------------------------

class test_iter_subplot(plotcal_test_base):
    ''' Test iteration and subplots '''

    def test_iter_ant(self):
        '''test antenna iteration with B table'''
        self.setUpBtable()
        plotcal(caltable=self.Btable, iteration='antenna', 
                showgui=False, figfile=self.fig)
        self.checkPlotfile(self.fig, 15000)
        self.removePlotfile()
        # 3 rows, 2 columns
        plotcal(caltable=self.Btable, iteration='antenna', 
                showgui=False, figfile=self.fig, subplot=321)
        self.checkPlotfile(self.fig, 55000)

    def test_iter_field(self):
        '''test field iteration with G table'''
        self.setUpGtable()
        plotcal(caltable=self.Gtable, iteration='field', 
                showgui=False, figfile=self.fig)
        self.checkPlotfile(self.fig, 20000)
        self.removePlotfile()
        # 3 rows, 1 column
        plotcal(caltable=self.Gtable, iteration='field', 
                showgui=False, figfile=self.fig, subplot=311)
        self.checkPlotfile(self.fig, 50000)

    def test_iter_spw(self):
        '''test spw iteration with G table'''
        self.setUpGtable()
        plotcal(caltable=self.Gtable, iteration='spw',
                showgui=False, figfile=self.fig)
        self.checkPlotfile(self.fig, 30000)
        self.removePlotfile()

# ------------------------------------------------------------------------

class test_plot_options(plotcal_test_base):
    ''' Test plot option parameters '''

    def test_subplot(self):
        '''subplot: plot 2 rows, 1 col'''
        self.setUpBtable()
        # first plot in first row: gamp vs. chan
        plotcal(caltable=self.Btable, showgui=False, subplot=211)
        # second plot in second row: gphase vs. chan
        plotcal(caltable=self.Btable, yaxis='phase',
                showgui=False, subplot=212, figfile=self.fig)
        self.checkPlotfile(self.fig, 80000)

    def test_overplot(self):
        '''overplot snr vs chan over gamp vs chan'''
        self.setUpBtable()
        # gamp vs chan
        plotcal(caltable=self.Btable, showgui=False)
        # snr vs chan
        plotcal(caltable=self.Btable, showgui=False, yaxis='snr',
                overplot=True, figfile=self.fig)
        self.checkPlotfile(self.fig, 65000)

    def test_plotrange(self):
        '''test x and y axes ranges'''
        self.setUpGtable()
        plotcal(caltable=self.Gtable, showgui=False, xaxis='antenna',
                plotrange=[5,15,0,0.6], figfile=self.fig)
        self.checkPlotfile(self.fig, 15000)
        self.removePlotfile()
        # test bad range for data plotted - 0 points, blank plot
        plotcal(caltable=self.Gtable, showgui=False, xaxis='antenna',
                plotrange=[3,2,1,0], figfile=self.fig)
        self.checkPlotfile(self.fig, 15000)
        self.removePlotfile()

    def test_showflags(self):
        '''test showflags=True'''
        self.setUpBtable()
        plotcal(caltable=self.Btable, showgui=False, showflags=True,
                figfile=self.fig)
        self.checkPlotfile(self.fig, 15000)

    def test_symbol_font(self):
        '''test plotsymbol, plotcolor, markersize, and fontsize'''
        self.setUpGtable()
        plotcal(caltable=self.Gtable, showgui=False, plotsymbol='-',
                plotcolor='red', markersize=7.0, fontsize=15.0,
                figfile=self.fig)
        self.checkPlotfile(self.fig, 60000)
        
# ------------------------------------------------------------------------

class test_formats(plotcal_test_base):
    ''' Test export formats '''
    # Supported formats: eps, pdf, png, ps, raw, rgba, svg, svgz

    def test_eps(self):
        '''test eps export'''
        self.setUpGtable()
        format = "/tmp/testplotcal.eps"
        self.removePlotfile(format)
        plotcal(caltable=self.Gtable, showgui=False, figfile=format) 
        self.checkPlotfile(format, 40000)
        self.removePlotfile(format)

    def test_pdf(self):
        '''test pdf export'''
        self.setUpGtable()
        format = "/tmp/testplotcal.pdf"
        self.removePlotfile(format)
        plotcal(caltable=self.Gtable, showgui=False, figfile=format) 
        self.checkPlotfile(format, 15000)
        self.removePlotfile(format)

    def test_png(self):
        '''test png export'''
        self.setUpGtable()
        format = "/tmp/testplotcal.png"
        self.removePlotfile(format)
        plotcal(caltable=self.Gtable, showgui=False, figfile=format) 
        self.checkPlotfile(format, 20000)
        self.removePlotfile(format)

    def test_ps(self):
        '''test ps export'''
        self.setUpGtable()
        format = "/tmp/testplotcal.ps"
        self.removePlotfile(format)
        plotcal(caltable=self.Gtable, showgui=False, figfile=format) 
        self.checkPlotfile(format, 40000)
        self.removePlotfile(format)

    def test_raw(self):
        '''test raw export'''
        self.setUpGtable()
        format = "/tmp/testplotcal.raw"
        self.removePlotfile(format)
        plotcal(caltable=self.Gtable, showgui=False, figfile=format) 
        self.checkPlotfile(format, 1900000)
        self.removePlotfile(format)

    def test_rgba(self):
        '''test rgba export'''
        self.setUpGtable()
        format = "/tmp/testplotcal.rgba"
        self.removePlotfile(format)
        plotcal(caltable=self.Gtable, showgui=False, figfile=format) 
        self.checkPlotfile(format, 1900000)
        self.removePlotfile(format)

    def test_svg(self):
        '''test svg export'''
        self.setUpGtable()
        format = "/tmp/testplotcal.svg"
        self.removePlotfile(format)
        plotcal(caltable=self.Gtable, showgui=False, figfile=format) 
        self.checkPlotfile(format, 75000)
        self.removePlotfile(format)

    def test_svgz(self):
        '''test svgz export'''
        self.setUpGtable()
        format = "/tmp/testplotcal.svgz"
        self.removePlotfile(format)
        plotcal(caltable=self.Gtable, showgui=False, figfile=format) 
        self.checkPlotfile(format, 10000)
        self.removePlotfile(format)

# ------------------------------------------------------------------------

def suite():
    return [test_basic,
            test_axes,
            test_selection,
            test_iter_subplot,
            test_plot_options,
            test_formats
           ]
 
