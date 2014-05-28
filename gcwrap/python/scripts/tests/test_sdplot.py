import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
import time
import filecmp
from matplotlib import pylab as pl

try:
    import selection_syntax
except:
    import tests.selection_syntax as selection_syntax

#to rethrow exception
import inspect
g = sys._getframe(len(inspect.stack())-1).f_globals
g['__rethrow_casa_exceptions'] = True
from sdplot import sdplot
import asap as sd
from asap.scantable import is_scantable

class sdplot_unittest_base:
    """
    Base class for sdplot unit test
    """
    taskname = 'sdplot'
    # Data path of input/output
    #datapath = os.environ.get('CASAPATH').split()[0] + \
    #           '/data/regression/unittest/' + taskname + '/'
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdplot/'
    # directories to save figure
    prevdir = datapath+'prev_sdplot'
    currdir = os.path.abspath('curr_sdplot')
    # Minimum data size of figure allowed
    minsize = 20000
    # GUI settings
    oldgui = sd.rcParams['plotter.gui']  # store previous GUI setting
    usegui = False   # need to set GUI status constant to compare
    # Do you want to test pixel to pixel comparison of figures?
    compare = False #(not usegui)
    # save figure for reference?
    saveref = compare

    # initialize plotter
    def _switchPlotterGUI(self, usegui):
        # explicitly delete scantable set to plotter
        if sd.plotter._data:
            del sd.plotter._data
            sd.plotter._data = None
        # restore GUI setting
        sd.rcParams['plotter.gui'] = usegui
        sd.plotter.__init__()

    # compare two figures
    def _checkOutFile( self, filename, compare=False ):
        self.assertTrue(os.path.exists(filename),"'%s' does not exists." % filename)
        self.assertTrue(os.path.isfile(filename),\
                        "Not a regular file. (A directory?): %s" % filename)
        self.assertTrue(os.path.getsize(filename) > self.minsize,\
                        "Data to small (less than %d KB): %s" % \
                        (int(self.minsize/1000), filename))
        prevfig = self.prevdir+'/'+filename
        currfig = self.currdir+'/'+filename
        if compare and os.path.exists(prevfig):
            # The unit test framework doesn't allow saving previous results
            # This test is not run.
            msg = "Compareing with previous plot:\n"
            msg += " (prev) %s\n (new) %s" % (prevfig,os.path.abspath(filename))
            casalog.post(msg,'INFO')
            self.assertTrue(filecmp.cmp(prevfig,os.path.abspath(filename),shallow=False))
        shutil.move(filename,currfig)

        # Save the figure for future comparison if possible.
        if os.path.exists(self.prevdir) and self.saveref:
            try:
                casalog.post("copying %s to %s" % (currfig,prevfig),'INFO')
                shutil.copyfile(currfig, prevfig)
            except IOError:
                msg = "Unable to copy Figure '"+filename+"' to "+self.prevdir+".\n"
                msg += "The figure remains in "+self.prevdir
                casalog.post(msg,'WARN')

    # get plot information
    def _get_plot_info( self ):
        retdic = {}
        ax0 = sd.plotter._plotter.subplots[0]['axes']
        retdic['npanel'] = len(sd.plotter._plotter.subplots)
        retdic['nstack'] = len(sd.plotter._plotter.subplots[0]['lines'])
        retdic['rows'] = sd.plotter._rows
        retdic['cols'] = sd.plotter._cols
        retdic['xlabel'] = ax0.get_xlabel()
        retdic['xlim'] = ax0.get_xlim()
        retdic['ylabel'] = ax0.get_ylabel()
        retdic['ylim'] = ax0.get_ylim()
        retdic['title0'] = ax0.get_title()
        retdic['label0'] = ax0.get_lines()[0].get_label()
        return retdic

    def _mergeDict( self, base, add ):
        self.assertTrue(isinstance(base,dict) and \
                        isinstance(add, dict),\
                        "Need to specify two dictionaries to merge")
        retdic = base.copy()
        for key, val in add.iteritems():
            retdic[key] = val
        return retdic
        

    # compare two dictionaries
    def _compareDictVal( self, testdict, refdict, reltol=1.0e-5, complist=None ):
        self.assertTrue(isinstance(testdict,dict) and \
                        isinstance(refdict, dict),\
                        "Need to specify two dictionaries to compare")
        if complist:
            keylist = complist
        else:
            keylist = refdict.keys()
        
        for key in keylist:
            self.assertTrue(testdict.has_key(key),\
                            msg="%s is not defined in the current results."\
                            % key)
            self.assertTrue(refdict.has_key(key),\
                            msg="%s is not defined in the reference data."\
                            % key)
            testval = self._to_list(testdict[key])
            refval = self._to_list(refdict[key])
            self.assertTrue(len(testval)==len(refval),"Number of elemnets differs.")
            for i in range(len(testval)):
                if isinstance(refval[i],str):
                    self.assertTrue(testval[i]==refval[i],\
                                    msg="%s[%d] differs: %s (expected: %s) " % \
                                    (key, i, str(testval[i]), str(refval[i])))
                else:
                    self.assertTrue(self._isInAllowedRange(testval[i],refval[i],reltol),\
                                    msg="%s[%d] differs: %s (expected: %s) " % \
                                    (key, i, str(testval[i]), str(refval[i])))
            del testval, refval
            
    def _isInAllowedRange( self, testval, refval, reltol=1.0e-5 ):
        """
        Check if a test value is within permissive relative difference from refval.
        Returns a boolean.
        testval & refval : two numerical values to compare
        reltol           : allowed relative difference to consider the two
                           values to be equal. (default 0.01)
        """
        denom = refval
        if refval == 0:
            if testval == 0:
                return True
            else:
                denom = testval
        rdiff = (testval-refval)/denom
        del denom,testval,refval
        return (abs(rdiff) <= reltol)

    def _to_list( self, input ):
        """
        Convert input to a list
        If input is None, this method simply returns None.
        """
        import numpy
        listtypes = (list, tuple, numpy.ndarray)
        if input == None:
            return None
        elif type(input) in listtypes:
            return list(input)
        else:
            return [input]
    
#####
# Tests on bad parameters and exceptions
#####
class sdplot_errorTest( sdplot_unittest_base, unittest.TestCase ):
    """
    Test bad input parameters and exceptions
    
    The list of tests:
      test_default    --- default parameters (raises an error)

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdcal)
      sdcal(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile=self.infile)
    """
    # Input and output names
    infile = 'OrionS_rawACSmod_cal2123.asap'
    #outfile = sdplot_unittest_base.taskname + '_testErr.png'
    outfile = 'sdplot_testErr.png'
    badid = '99'
    badstr = "bad"

    def setUp( self ):
        # switch on/off GUI
        self._switchPlotterGUI(self.usegui)
        # Fresh copy of input data
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        default(sdplot)

    def tearDown( self ):
        # restore GUI setting
        self._switchPlotterGUI(self.oldgui)
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    # Actual tests
    def test_default( self ):
        """test_default: Default parameters (should Fail)"""
        result = sdplot()
        self.assertFalse(result)

    def test_overwrite( self ):
        """test_overwrite: Specify existing output file name with overwrite=False (task script raises Exception)"""
        res1 = sdplot(infile=self.infile,outfile=self.outfile)
        self.assertEqual(res1,None)
        try:
            res2 = sdplot(infile=self.infile,
                          outfile=self.outfile,overwrite=False)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, err:
            pos=str(err).find("Output file '%s' exists." % self.outfile)
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(err)))
    def test_badSelection( self ):
        """test_badSelection: Invalid data selection (no data selected) """
        # the AipsError raised in cpp is caught and rethrown in
        # the middle of task script
        spw = self.badid
        try:
            result = sdplot(infile=self.infile,spw=spw)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, err:
            #pos=str(err).find("Selection contains no data. Not applying it.")
            pos=str(err).find("No valid spw.")
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(err)))

    def test_noTweight( self ):
        """test_noTweight: Time averaging without tweight"""
        # this error is handled in task interface in sdplot
        result = sdplot(infile=self.infile,timeaverage=True,tweight='')
        self.assertFalse(result)

    def test_noPweight( self ):
        """test_noPweight: Polarization averaging without pweight"""
        # this error is handled in task interface in sdplot
        result = sdplot(infile=self.infile,polaverage=True,pweight='')
        self.assertFalse(result)

    def test_badLincat( self ):
        """test_badLinecat: Invalid line catalog (cpp throws AipsError)"""
        # The aips error thrown in cpp is caught and rethrown at the end
        # of task script
        type = "spectra"
        linecat = self.badstr
        try:
            result = sdplot(infile=self.infile,plottype=type,linecat=linecat)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, err:
            pos=str(err).find("No match.")
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(err)))

    def test_badStack( self ):
        """test_badStack: Invalid stack mode (python tool raises TypeError)"""
        type = "spectra"
        stack = " "
        try:
            result = sdplot(infile=self.infile,plottype=type,stack=stack)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, err:
            pos=str(err).find("Invalid mode")
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(err)))


#####
# Tests on basic task parameters
#####
class sdplot_basicTest( sdplot_unittest_base, unittest.TestCase ):
    """
    Test plot parameters only. Least data filterings and no averaging.
    
    The list of tests:
      testplot01-03 --- possible plot types
      testplot04-07 --- possible axes (spectral plotting)
      testplot08-12 --- panelling and stacking (spectral plotting)
      testplot13-15 --- plot range control (spectral plotting)
      testplot16-19 --- plot style control (spectral plotting)
      testplot20-21 --- header control (spectral plotting)
      testplot22-23,28 --- plot layout control (spectral plotting)
      testplot24-25 --- row panelling or stacking (spectral plotting)
      testplot26-27 --- flag application
      testplot29-30 --- restfreq

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdcal)
      sdcal(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile=self.infile)
    """
    # Input and output names
    infile = 'OrionS_rawACSmod_cal2123.asap'
    #figroot = sdplot_unittest_base.taskname + '_test'
    figroot = 'sdplot_test'
    figsuff = '.png'
    fig=None
    baseinfo = {'npanel': 2, 'nstack': 2,
               'xlabel': 'Channel',
               'ylabel': 'Brightness Temperature (K)'}

    def setUp( self ):
        # switch on/off GUI
        self._switchPlotterGUI(self.usegui)
        # Fresh copy of input data
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        # Generate directory to save figures
        if not os.path.exists(self.currdir):
            os.makedirs(self.currdir)
        # Create a directory to store the figures for future comparison
        if (not os.path.exists(self.prevdir)) and self.saveref:
            try: os.makedirs(self.prevdir)
            except OSError:
                msg = "Unable to create directory, "+self.prevdir+".\n"
                msg += "Plot figures will remain in "+self.currdir
                casalog.post(msg,'WARN')
        
        default(sdplot)

    def tearDown( self ):
        # restore GUI setting
        self._switchPlotterGUI(self.oldgui)
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    # Actual tests
    def testplot01( self ):
        """Test 1: test plot type --- az/el"""
        tid = "01"
        infile = self.infile
        outfile = self.figroot+tid+self.figsuff
        plottype = 'azel'
        header = False
        result = sdplot(infile=self.infile,plottype=plottype,header=header,
                        outfile=outfile)
        # Tests
        self.assertEqual(result,None)
        self.assertEqual(len(pl.gcf().axes),2)
        self.assertEqual(len(pl.gca().get_lines()),1)
        self.assertEqual(pl.gca().get_xlabel(),'Time (UT [hour])')
        self.assertEqual(pl.gca().get_ylabel(),'Az [deg.]')
        self._checkOutFile(outfile,self.compare)

    def testplot02( self ):
        """Test 2: test plot type --- pointing"""
        tid = "02"
        infile = self.infile
        outfile = self.figroot+tid+self.figsuff
        plottype = 'pointing'
        header = False
        result = sdplot(infile=self.infile,plottype=plottype,header=header,
                        outfile=outfile)
        # Tests
        self.assertEqual(result,None)
        self.assertEqual(len(pl.gcf().axes),1)
        self.assertEqual(len(pl.gca().get_lines()),2)
        self.assertEqual(pl.gca().get_xlabel(),'RA [deg.]')
        self.assertEqual(pl.gca().get_ylabel(),'Declination [deg.]')
        self._checkOutFile(outfile,self.compare)

    def testplot03( self ):
        """
        Test 3: test plot type --- total power (row # vs total power)
        """
        tid = "03"
        infile = self.infile
        outfile = self.figroot+tid+self.figsuff
        plottype = 'totalpower'
        header = False
        result = sdplot(infile=infile,plottype=plottype,header=header,
                        outfile=outfile)
        locinfo = {'npanel': 1, 'nstack': 1,
                   'xlabel': 'row number',
                   'ylabel': 'Brightness Temperature (K)'}
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, locinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot04( self ):
        """
        Test 4: test possible axes for spectral plotting --- specunit='channel'
        """
        tid = "04"
        infile = self.infile
        outfile = self.figroot+tid+self.figsuff
        specunit = 'channel'
        fluxunit = 'K'
        stack = 'p'
        panel = 'i'
        spw = '0,2'
        header = False
        result = sdplot(infile=infile,specunit=specunit,fluxunit=fluxunit,
                        stack=stack,panel=panel,spw=spw,
                        header=header,outfile=outfile)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, self.baseinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot05( self ):
        """
        Test 5: test possible axes for spectral plotting --- specunit='GHz'
        """
        tid = "05"
        infile = self.infile
        outfile = self.figroot+tid+self.figsuff
        specunit = 'GHz'
        fluxunit = 'K'
        stack = 'p'
        panel = 'i'
        spw = '0,2'
        header = False
        result = sdplot(infile=infile,specunit=specunit,fluxunit=fluxunit,
                        stack=stack,panel=panel,spw=spw,
                        header=header,outfile=outfile)
        locinfo = {'xlabel': 'LSRK Frequency (%s)' % specunit}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot06( self ):
        """
        Test 6: test possible axes for spectral plotting --- specunit='km/s'
        """
        tid = "06"
        infile = self.infile
        outfile = self.figroot+tid+self.figsuff
        specunit = 'km/s'
        fluxunit = 'K'
        stack = 'p'
        panel = 'i'
        spw = '0,2'
        header = False
        result = sdplot(infile=infile,specunit=specunit,fluxunit=fluxunit,
                        stack=stack,panel=panel,spw=spw,
                        header=header,outfile=outfile)
        locinfo = {'xlabel': 'LSRK RADIO velocity (%s)' % (specunit)}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot07( self ):
        """
        Test 7: test possible axes for spectral plotting --- fluxunit='Jy'
        """
        tid = "07"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        specunit = 'channel'
        fluxunit = 'Jy'
        stack = 'p'
        panel = 'i'
        spw = '0,2'
        header = False
        result = sdplot(infile=infile,specunit=specunit,fluxunit=fluxunit,
                        stack=stack,panel=panel,spw=spw,
                        header=header,outfile=outfile)
        locinfo = {'ylabel': 'Flux density (Jy)'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot08( self ):
        """
        Test 8: test panelling and stacking (spectral plotting) --- panel='pol', stack='scan' (2px2s)
        """
        tid = "08"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        panel = 'pol'
        stack = 'scan'
        header = False
        result = sdplot(infile=infile,stack=stack,panel=panel,
                        header=header,outfile=outfile)
        locinfo = {'title0': 'XX', 'label0': 'Scan 21 (OrionS)'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot09( self ):
        """
        Test 9: test panelling and stacking (spectral plotting) --- panel='scan', stack='if' (2px4s)
        """
        tid = "09"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        panel = 'scan'
        stack = 'if'
        header = False
        result = sdplot(infile=infile,stack=stack,panel=panel,
                        header=header,outfile=outfile)
        locinfo = {'nstack': 4,
                   'title0': 'Scan 21 (OrionS)', 'label0': 'IF0'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot10( self ):
        """
        Test 10: test panelling and stacking (spectral plotting) --- panel='if', stack='time' (4px8s)
        """
        tid = "10"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        panel = 'if'
        stack = 'time'
        header = False
        result = sdplot(infile=infile,stack=stack,panel=panel,
                        header=header,outfile=outfile)
        locinfo = {'npanel': 4, 'nstack': 8,
                   'title0': 'IF0', 'label0': '2006/01/19/01:48:38'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot11( self ):
        """
        Test 11: test panelling and stacking (spectral plotting) --- panel='time', stack='beam' (8px1s)
        """
        tid = "11"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        panel = 'time'
        stack = 'beam'
        header = False
        result = sdplot(infile=infile,stack=stack,panel=panel,
                        header=header,outfile=outfile)
        locinfo = {'npanel': 8,'nstack': 1,
                   'title0': '2006/01/19/01:48:38','label0': 'Beam 0'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot12( self ):
        """
        Test 12: test panelling and stacking (spectral plotting) --- panel='beam', stack='pol' (1px2s)
        """
        tid = "12"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        panel = 'beam'
        stack = 'pol'
        header = False
        result = sdplot(infile=infile,stack=stack,panel=panel,
                        header=header,outfile=outfile)
        locinfo = {'npanel': 1,'nstack': 2,
                   'title0': 'Beam 0', 'label0': 'XX'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot13( self ):
        """
        Test 13: test plot range control for spectral plotting --- abscissa
        """
        tid = "13"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        spw = '0'
        sprange = [3500,5000]
        header = False
        result = sdplot(infile=infile,spw=spw,sprange=sprange,
                        header=header,outfile=outfile)
        locinfo = {'npanel': 1,'nstack': 2,
                   'title0': 'IF0', 'label0': 'XX',
                   'xlim': sprange}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot14( self ):
        """
        Test 14: test plot range control for spectral plotting --- ordinate
        """
        tid = "14"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        spw = '0'
        flrange = [0.,6.]
        header = False
        result = sdplot(infile=infile,spw=spw,flrange=flrange,
                        header=header,outfile=outfile)
        locinfo = {'npanel': 1, 'ylim': flrange}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot15( self ):
        """
        Test 15: test plot range control for spectral plotting --- both
        """
        tid = "15"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        spw = '0'
        sprange = [3500,5000]
        flrange = [0.,6.]
        header = False
        result = sdplot(infile=infile,spw=spw,sprange=sprange,
                        header=header,flrange=flrange,outfile=outfile)
        locinfo = {'npanel': 1, 'xlim': sprange, 'ylim': flrange}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot16( self ):
        """
        Test 16: plot style control (spectral plotting) --- histogram
        """
        tid = "16"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        spw = '0'
        sprange = [3900,4300]
        histogram = True
        header = False
        result = sdplot(infile=infile,spw=spw,sprange=sprange,
                        header=header,histogram=histogram,outfile=outfile)
        locinfo = {'npanel': 1, 'xlim': sprange}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot17( self ):
        """
        Test 17: plot style control (spectral plotting) --- colormap
        """
        tid = "17"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        spw = '0'
        sprange = [3900,4300]
        colormap = "pink orange"
        header = False
        result = sdplot(infile=infile,spw=spw,sprange=sprange,
                        header=header,colormap=colormap,outfile=outfile)
        locinfo = {'npanel': 1, 'xlim': sprange}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot18( self ):
        """
        Test 18: plot style control (spectral plotting) --- linewidth
        """
        tid = "18"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        spw = '0'
        sprange = [3900,4300]
        linewidth = 3
        header = False
        result = sdplot(infile=infile,spw=spw,sprange=sprange,
                        header=header,linewidth=linewidth,outfile=outfile)
        locinfo = {'npanel': 1, 'xlim': sprange}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot19( self ):
        """
        Test 19: plot style control (spectral plotting) --- linestyle
        """
        tid = "19"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        spw = '0'
        sprange = [3900,4300]
        linewidth = 2
        linestyles = "dotted dashdot"
        header = False
        result = sdplot(infile=infile,spw=spw,sprange=sprange,
                        linewidth=linewidth,linestyles=linestyles,
                        header=header,outfile=outfile)
        locinfo = {'npanel': 1, 'xlim': sprange}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot20( self ):
        """
        Test 20: header control (spectral plotting) --- larger fontsize
        """
        tid = "20"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        spw = '0,2'
        headsize = 11
        result = sdplot(infile=infile,spw=spw,headsize=headsize,
                        outfile=outfile)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, self.baseinfo)
        self._checkOutFile(outfile, compare=False)

    def testplot21( self ):
        """
        Test 21: header control (spectral plotting) --- header on
        """
        tid = "21"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        spw = '0,2'
        result = sdplot(infile=infile,spw=spw,
                        outfile=outfile)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, self.baseinfo)
        self._checkOutFile(outfile, compare=False)

    def testplot22( self ):
        """
        Test 22: plot layout control (spectral plotting) --- panel margin
        """
        tid = "22"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        spw = '0,2'
        plotstyle = True
        margin = [0.15,0.3,0.85,0.7,0.25,0.25]
        header = False
        result = sdplot(infile=infile,spw=spw,plotstyle=plotstyle,
                        margin=margin,header=header,outfile=outfile)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, self.baseinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot23( self ):
        """
        Test 23: plot layout control (spectral plotting) --- legend location (left bottom)
        """
        tid = "23"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        spw = '0,2'
        plotstyle = True
        legendloc = 3
        header = False
        result = sdplot(infile=infile,spw=spw,plotstyle=plotstyle,
                        legendloc=legendloc,header=header,outfile=outfile)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, self.baseinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot24( self ):
        """
        Test 24: test panelling and stacking (spectral plotting) --- panel='row' (16px1s)
        """
        tid = "24"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        panel = 'row'
        header = False
        result = sdplot(infile=infile,panel=panel,
                        header=header,outfile=outfile)
        locinfo = {'npanel': 16, 'nstack': 1,
                   'title0': 'row 0', 'label0': 'XX'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot25( self ):
        """
        Test 25: test panelling and stacking (spectral plotting) --- panel='scan', stack='row' (1px16s)
        """
        tid = "25"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        stack = 'row'
        header = False
        result = sdplot(infile=infile,stack=stack,
                        header=header,outfile=outfile)
        locinfo = {'npanel': 1, 'nstack': 16,
                   'title0': '', 'label0': 'row 0'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot26( self ):
        """
        Test 26: test handling of FLAGROW
        """
        from asap import scantable
        tid = "26"
        outfile = self.figroot+tid+self.figsuff
        ### flag rows=[0,3,6,9,12,15]
        infile = "orion_flagrow.asap"
        scan = scantable(filename=self.infile,average=False)
        scan.flag_row(rows=[0,3,6,9,12,15])
        scan.save(name=infile,format='ASAP')
        #
        panel = 'row'
        stack = 'scan'
        header = False
        result = sdplot(infile=infile,panel=panel,stack=stack,
                        header=header,outfile=outfile)
        self.assertEqual(result,None)
        self._checkOutFile(outfile,self.compare)

    def testplot27( self ):
        """
        Test 27: test handling of FLAGTRA
        """
        from asap import scantable
        tid = "27"
        outfile = self.figroot+tid+self.figsuff
        ### flag channels [7168,8191]
        infile = "orion_flagchan7168to8191.asap"
        scan = scantable(filename=self.infile,average=False)
        scan.set_unit('channel')
        msk = scan.create_mask([7168,8191])
        scan.flag(mask=msk)
        scan.save(name=infile,format='ASAP')
        #
        panel = 'row'
        header = False
        result = sdplot(infile=infile,panel=panel,
                        header=header,outfile=outfile)
        self.assertEqual(result,None)
        self._checkOutFile(outfile,self.compare)

    def testplot28( self ):
        """
        Test 28: plot layout control (spectral plotting) --- panel layout
        """
        tid = "28"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        spw = '0,2'
        panel = 'r'
        plotstyle = True
        subplot = 24
        header = False
        result = sdplot(infile=infile,spw=spw,plotstyle=plotstyle,
                        subplot=subplot,header=header,outfile=outfile)
        locinfo = {'rows': 2, 'cols': 4}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot29( self ):
        """
        Test 29: plot with user defined restfreq (a list of num and quantity)
        """
        tid = "29"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        spw = '1,2'
        specunit = 'km/s'
        restfreq = ['45.3GHz', 44.075e9]
        panel = 'i'
        header = False
        result = sdplot(infile=infile,spw=spw,panel = panel,
                        specunit=specunit,restfreq=restfreq,
                        header=header,outfile=outfile)
        locinfo = {'xlim': (-170.62517234590837, 160.27007370505743),
                   'xlabel': 'LSRK RADIO velocity (km/s)',
                   'title0': 'IF1', 'label0': 'XX'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testplot30( self ):
        """
        Test 30: plot with user defined restfreq (a list of dict)
        """
        tid = "30"
        outfile = self.figroot+tid+self.figsuff
        infile = self.infile
        spw = '2'
        specunit = 'km/s'
        restfreq = [{'name': 'ch3oh', 'value': 44.075e9}]
        panel = 'i'
        header = False
        result = sdplot(infile=infile,spw=spw,panel = panel,
                        specunit=specunit,restfreq=restfreq,
                        header=header,outfile=outfile)
        locinfo = {'npanel': 1, 'xlabel': 'LSRK RADIO velocity (km/s)',
                   'xlim': (-169.54464299991017, 170.54735123960856),
                   'title0': 'IF2'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)


#####
# Tests on scantable storage and insitu
#####
class sdplot_storageTest( sdplot_unittest_base, unittest.TestCase ):
    """
    Unit tests of task sdplot. Test scantable sotrage and insitu
    parameters
    
    The list of tests:
    testMT  --- storage = 'memory', insitu = True
    testMF  --- storage = 'memory', insitu = False
    testDT  --- storage = 'disk', insitu = True
    testDF  --- storage = 'disk', insitu = False

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdcal)
      sdcal(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile=self.infile)
    """
    # Input and output names
    infile = 'OrionS_rawACSmod_cal2123.asap'
    #figroot = sdplot_unittest_base.taskname + '_store'
    figroot = 'sdplot_store'
    figsuff = '.png'
    fig=None

    out_uc = {'spunit': 'km/s', 'flunit': 'Jy',\
              'frame': 'LSRD', 'doppler': 'OPTICAL'}
    refaxlabel = {'x': ("%s %s velocity (%s)" % \
                        (out_uc['frame'],out_uc['doppler'],out_uc['spunit'])),
                  'y': "Flux density (%s)" % out_uc['flunit']}

    # common parameter values
    fluxunit = out_uc['flunit']
    specunit = out_uc['spunit']
    frame = out_uc['frame']
    doppler = out_uc['doppler']
    stack = 'p'
    panel = 'i'
    spw = '0,2'
    header = False

    def setUp( self ):
        # switch on/off GUI
        self._switchPlotterGUI(self.usegui)
        # Fresh copy of input data
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        # Generate directory to save figures
        if not os.path.exists(self.currdir):
            os.makedirs(self.currdir)
        # Create a directory to store the figures for future comparison
        if (not os.path.exists(self.prevdir)) and self.saveref:
            try: os.makedirs(self.prevdir)
            except OSError:
                msg = "Unable to create directory, "+self.prevdir+".\n"
                msg += "Plot figures will remain in "+self.currdir
                casalog.post(msg,'WARN')
        
        default(sdplot)

    def tearDown( self ):
        # restore GUI setting
        self._switchPlotterGUI(self.oldgui)
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    # helper functions of tests
    def _get_unit_coord( self, scanname ):
        # Returns a dictionary which stores units and coordinates of a
        # scantable, scanname. Returned dictionary stores spectral
        # unit, flux unit, frequency frame, and doppler of scanname.
        self.assertTrue(os.path.exists(scanname),\
                        "'%s' does not exists." % scanname)
        self.assertTrue(is_scantable(scanname),\
                        "Input table is not a scantable: %s" % scanname)
        scan = sd.scantable(scanname, average=False,parallactify=False)
        retdict = {}
        retdict['spunit'] = scan.get_unit()
        retdict['flunit'] = scan.get_fluxunit()
        coord = scan._getcoordinfo()
        retdict['frame'] = coord[1]
        retdict['doppler'] = coord[2]
        return retdict

    def _get_uclist( self, stlist ):
        # Returns a list of dictionaries of units and coordinates of
        # a list of scantables in stlist. This method internally calls
        # _get_unit_coord(scanname).
        retlist = []
        for scanname in stlist:
            retlist.append(self._get_unit_coord(scanname))
        #print retlist
        return retlist

    def _comp_unit_coord( self, stlist, before):
        ### stlist: a list of scantable names
        if isinstance(stlist,str):
            stlist = [ stlist ]
        ### before: a return value of _get_uclist() before run
        if isinstance(before, dict):
            before = [ before ]
        if len(stlist) != len(before):
            raise Exception("Number of scantables in list is different from reference data.")
        self.assertTrue(isinstance(before[0],dict),\
                        "Reference data should be (a list of) dictionary")

        after = self._get_uclist(stlist)
        for i in range(len(stlist)):
            print "Testing units and coordinates of '%s'" %\
                  stlist[i]
            self._compareDictVal(after[i],before[i])

    def _check_axlabel( self, axlist, refval ):
        ip = 0
        print "Testing axes labels [expected: x = %s, y = %s]" % (refval['x'],refval['y'])
        for ax in axlist:
            xlab = ax.get_xlabel()
            ylab = ax.get_ylabel()
            self.assertTrue(xlab==refval['x'],\
                            "X-label differs (panel%d): %s (expected: %s)" %\
                            (ip,xlab,refval['x']))
            self.assertTrue(ylab==refval['y'],\
                            "Y-label differs (panel%d): %s (expected: %s)" %\
                            (ip,ylab,refval['y']))
            ip += 1

    def _get_axlist( self ):
        axlist = []
        for subp in sd.plotter._plotter.subplots:
            axlist.append(subp['axes'])
        return axlist

    # Actual tests
    def testMT( self ):
        """Storage Test MT: storage='memory' and insitu=T"""
        tid="MT"
        outfile = self.figroot+tid+self.figsuff
        
        # Backup units and coords of input scantable before run.
        initval = self._get_unit_coord(self.infile)

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = sdplot(infile=self.infile,specunit=self.specunit,\
                        fluxunit=self.fluxunit,frame=self.frame,\
                        doppler=self.doppler,stack=self.stack,panel=self.panel,\
                        spw=self.spw,header=self.header,outfile=outfile)
        # Test plot
        self.assertEqual(result,None)
        self._checkOutFile(outfile,self.compare)
        axlist = self._get_axlist()
        self._check_axlabel(axlist,self.refaxlabel)
        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord(self.infile,initval)

    def testMF( self ):
        """Storage Test MF: storage='memory' and insitu=F"""
        tid="MF"
        outfile = self.figroot+tid+self.figsuff
        
        # Backup units and coords of input scantable before run.
        initval = self._get_unit_coord(self.infile)

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = sdplot(infile=self.infile,specunit=self.specunit,\
                        fluxunit=self.fluxunit,frame=self.frame,\
                        doppler=self.doppler,stack=self.stack,panel=self.panel,\
                        spw=self.spw,header=self.header,outfile=outfile)
        # Test plot
        self.assertEqual(result,None)
        self._checkOutFile(outfile,self.compare)
        axlist = self._get_axlist()
        self._check_axlabel(axlist,self.refaxlabel)
        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord(self.infile,initval)

    def testDT( self ):
        """Storage Test DT: storage='disk' and insitu=T"""
        tid="DT"
        outfile = self.figroot+tid+self.figsuff
        
        # Backup units and coords of input scantable before run.
        initval = self._get_unit_coord(self.infile)

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = sdplot(infile=self.infile,specunit=self.specunit,\
                        fluxunit=self.fluxunit,frame=self.frame,\
                        doppler=self.doppler,stack=self.stack,panel=self.panel,\
                        spw=self.spw,header=self.header,outfile=outfile)
        # Test plot
        self.assertEqual(result,None)
        self._checkOutFile(outfile,self.compare)
        axlist = self._get_axlist()
        self._check_axlabel(axlist,self.refaxlabel)
        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord(self.infile,initval)

    def testDF( self ):
        """Storage Test DF: storage='disk' and insitu=F"""
        tid="DF"
        outfile = self.figroot+tid+self.figsuff
        
        # Backup units and coords of input scantable before run.
        initval = self._get_unit_coord(self.infile)

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = sdplot(infile=self.infile,specunit=self.specunit,\
                        fluxunit=self.fluxunit,frame=self.frame,\
                        doppler=self.doppler,stack=self.stack,panel=self.panel,\
                        spw=self.spw,header=self.header,outfile=outfile)
        # Test plot
        self.assertEqual(result,None)
        self._checkOutFile(outfile,self.compare)
        axlist = self._get_axlist()
        self._check_axlabel(axlist,self.refaxlabel)
        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord(self.infile,initval)

#####
# Tests on plottype='grid'
#####
class sdplot_gridTest( sdplot_unittest_base, unittest.TestCase ):
    """
    Unit tests of task sdplot. Test plottype='grid'
    
    The list of tests:
    testgrid01 - 08 --- test gridding
    testgrid09 - 11 --- test plot range
    testgrid12 - 14 --- test color and line

    Note: input data is generated from a intermediate data of
    single dish regression data,
    'FLS3a_calfs', as follows:
    sdsave(infile='FLS3a_calfs',outfile='FLS3a_calfs.asap')
    sdgrid(infiles=['FLS3a_calfs.asap'], ifno=0, npix=[6,6],
           outfile='FLS3a_calfs.6x6.asap')
    """
    # Input and output names
    infile = 'FLS3a_calfs.6x6.asap'
    #infile = 'FLS3a_calfs.asap'
    #figroot = sdplot_unittest_base.taskname + '_grid'
    figroot = 'sdplot_grid'
    figsuff = '.png'
    fig=None

    # common parameter values
    type = 'grid'
    header = False
    pol = '0'
    subplot = 66
    #cell = ["0.033934774957430407rad","0.0080917391193671574rad"]
    cell = ["0.0087400000000000064rad", "0.0094409136746534481rad"]
    #center="J2000 17:17:58.94 +59.30.01.962"
    center = "J2000 17:17:58.94 +059.29.20.020"

    baseinfo = {'npanel': 36, 'nstack': 1,
                'rows': 6, 'cols': 6,
               'xlabel': 'Channel',
               'ylabel': 'Brightness Temperature (K)'}

    def setUp( self ):
        # switch on/off GUI
        self._switchPlotterGUI(self.usegui)
        # Fresh copy of input data
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        # Generate directory to save figures
        if not os.path.exists(self.currdir):
            os.makedirs(self.currdir)
        # Create a directory to store the figures for future comparison
        if (not os.path.exists(self.prevdir)) and self.saveref:
            try: os.makedirs(self.prevdir)
            except OSError:
                msg = "Unable to create directory, "+self.prevdir+".\n"
                msg += "Plot figures will remain in "+self.currdir
                casalog.post(msg,'WARN')
        
        default(sdplot)

    def tearDown( self ):
        # restore GUI setting
        self._switchPlotterGUI(self.oldgui)
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    # helper functions of tests

    # Actual tests
    def testgrid01( self ):
        """testgrid01: default gridding (1x1)"""
        tid="01"
        outfile = self.figroot+tid+self.figsuff
        result = sdplot(infile=self.infile, pol=self.pol,
                        plottype=self.type, header=self.header,
                        outfile=outfile)
        locinfo = {'npanel': 1, 'rows': 1, 'cols': 1,
                   'title0': "J2000 17:17:58.9 +59.30.01.9"}
                   #"J2000 17:17:58.9 +59.29.20.0"}#'J2000 17:17:58.9 +59.30.02.0'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testgrid02( self ):
        """testgrid02: default center"""
        tid="02"
        outfile = self.figroot+tid+self.figsuff
        cell = self.cell
        subplot = self.subplot
        result = sdplot(infile=self.infile, pol=self.pol,
                        plottype=self.type,
                        cell=cell, subplot=subplot,
                        header=self.header, outfile=outfile)
        locinfo = {}#'title0': 'J2000 17:17:58 +59.30.02.0'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testgrid03( self ):
        """testgrid03: default cell"""
        tid="03"
        outfile = self.figroot+tid+self.figsuff
        center = self.center
        subplot = self.subplot
        result = sdplot(infile=self.infile, pol=self.pol,
                        plottype=self.type, center=center,
                        subplot=subplot,
                        header=self.header, outfile=outfile)
        locinfo = {}#'title0': 'J2000 17:17:58 +59.30.02.0'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testgrid04( self ):
        """testgrid04: default subplot  (1x1)"""
        tid="04"
        outfile = self.figroot+tid+self.figsuff
        center = self.center
        cell = self.cell
        result = sdplot(infile=self.infile, pol=self.pol,
                        plottype=self.type, center=center,
                        cell=cell,
                        header=self.header, outfile=outfile)
        locinfo = {'npanel': 1, 'rows': 1, 'cols': 1,
                   'title0': "J2000 17:17:58.9 +59.29.20.0"}#'J2000 17:17:58.9 +59.30.02.0'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testgrid05( self ):
        """testgrid05: test center (1x1)"""
        tid="05"
        outfile = self.figroot+tid+self.figsuff
        center = self.center
        result = sdplot(infile=self.infile, pol=self.pol,
                        plottype=self.type, center=center,
                        header=self.header, outfile=outfile)
        locinfo = {'npanel': 1, 'rows': 1, 'cols': 1,
                   'title0': "J2000 17:17:58.9 +59.29.20.0"}#'J2000 17:17:58.9 +59.30.02.0'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testgrid06( self ):
        """testgrid06: test cell  (1x1)"""
        tid="06"
        outfile = self.figroot+tid+self.figsuff
        cell = self.cell
        result = sdplot(infile=self.infile, pol=self.pol,
                        plottype=self.type, cell=cell,
                        header=self.header, outfile=outfile)
        locinfo = {'npanel': 1, 'rows': 1, 'cols': 1,
                   'title0': "J2000 17:17:58.9 +59.30.01.9"}#'J2000 17:17:58.9 +59.30.02.0'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testgrid07( self ):
        """testgrid07: test subplot"""
        tid="07"
        outfile = self.figroot+tid+self.figsuff
        subplot = self.subplot
        result = sdplot(infile=self.infile, pol=self.pol,
                        plottype=self.type, subplot=subplot,
                        header=self.header, outfile=outfile)
        locinfo = {}#'title0': 'J2000 17:17:58 +59.30.02.0'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testgrid08( self ):
        """testgrid08: test grid"""
        tid="08"
        outfile = self.figroot+tid+self.figsuff
        center = self.center
        cell = self.cell
        subplot = self.subplot

        result = sdplot(infile=self.infile, pol=self.pol,
                        plottype=self.type, center=center,
                        cell=cell, subplot=subplot,
                        header=self.header, outfile=outfile)
        locinfo = {}#'title0': 'J2000 17:17:58 +59.30.02.0'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testgrid09( self ):
        """testgrid09: test sprange"""
        tid="09"
        outfile = self.figroot+tid+self.figsuff
        subplot = self.subplot
        sprange = [200,800]

        result = sdplot(infile=self.infile, pol=self.pol,
                        plottype=self.type, subplot=subplot,
                        sprange=sprange,
                        header=self.header, outfile=outfile)
        locinfo = {#'npanel': 1, 'rows': 1, 'cols': 1,
                   'xlim': sprange}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testgrid10( self ):
        """testgrid10: test flrange"""
        tid="10"
        outfile = self.figroot+tid+self.figsuff
        subplot = self.subplot
        flrange = [-2.,10.]

        result = sdplot(infile=self.infile, pol=self.pol,
                        plottype=self.type, subplot=subplot,
                        flrange=flrange,
                        header=self.header, outfile=outfile)
        locinfo = {#'npanel': 1, 'rows': 1, 'cols': 1,
                   'ylim': flrange}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testgrid11( self ):
        """testgrid11: test sprange and flrange"""
        tid="11"
        outfile = self.figroot+tid+self.figsuff
        subplot = self.subplot
        sprange = [200,800]
        flrange = [-2.,10.]

        result = sdplot(infile=self.infile, pol=self.pol,
                        plottype=self.type, subplot=subplot,
                        sprange=sprange, flrange=flrange,
                        header=self.header, outfile=outfile)
        locinfo = {#'npanel': 1, 'rows': 1, 'cols': 1,
                   'xlim': sprange, 'ylim': flrange}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testgrid12( self ):
        """testgrid12: test colormap"""
        tid="12"
        outfile = self.figroot+tid+self.figsuff
        subplot = self.subplot
        colormap = "orange pink"
        
        result = sdplot(infile=self.infile, pol=self.pol,
                        plottype=self.type, subplot=subplot,
                        colormap=colormap,
                        header=self.header, outfile=outfile)
        locinfo = {}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testgrid13( self ):
        """testgrid13: test linestyles"""
        tid="13"
        outfile = self.figroot+tid+self.figsuff
        subplot = self.subplot
        linestyles = "dashdot"
        
        result = sdplot(infile=self.infile, pol=self.pol,
                        plottype=self.type, subplot=subplot,
                        linestyles=linestyles,
                        header=self.header, outfile=outfile)
        locinfo = {}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testgrid14( self ):
        """testgrid14: test linewidth"""
        tid="14"
        outfile = self.figroot+tid+self.figsuff
        subplot = self.subplot
        linewidth=3
        
        result = sdplot(infile=self.infile, pol=self.pol,
                        plottype=self.type, subplot=subplot,
                        linewidth=linewidth,
                        header=self.header, outfile=outfile)
        locinfo = {}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

#####
# Tests on string data selection
#####
class sdplot_selectTest( sdplot_unittest_base, unittest.TestCase ):
    """
    Test CASA type data selection in string.
    
    The list of tests:
      testsel01 --- scan
      testsel01 --- spw
      testsel01 --- pol
      testsel01 --- beam

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdcal)
      sdcal(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
            calmode='ps',tau=0.09,outfile='tmp.asap')
      sdcal(infile='tmp.asap',scanaverage=True,outfile=self.infile)
    """
    # Input and output names
    infile = 'OrionS_rawACSmod_calSave.asap'
    #figroot = sdplot_unittest_base.taskname + '_sel'
    figroot = 'sdplot_sel'
    figsuff = '.png'
    fig=None
    baseinfo = {'npanel': 2, 'nstack': 2,
               'xlabel': 'Channel',
               'ylabel': 'Brightness Temperature (K)'}

    # common parameter values
    header = False

    
    def setUp( self ):
        # switch on/off GUI
        self._switchPlotterGUI(self.usegui)
        # Fresh copy of input data
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        # Generate directory to save figures
        if not os.path.exists(self.currdir):
            os.makedirs(self.currdir)
        # Create a directory to store the figures for future comparison
        if (not os.path.exists(self.prevdir)) and self.saveref:
            try: os.makedirs(self.prevdir)
            except OSError:
                msg = "Unable to create directory, "+self.prevdir+".\n"
                msg += "Plot figures will remain in "+self.currdir
                casalog.post(msg,'WARN')
        
        default(sdplot)

    def tearDown( self ):
        # restore GUI setting
        self._switchPlotterGUI(self.oldgui)
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    # helper functions of tests

    # Actual tests
    def testsel01( self ):
        """testsel01: scan='23~25'"""
        tid="01"
        outfile = self.figroot+tid+self.figsuff
        scan="23~25"
        panel = 's'
        result = sdplot(infile=self.infile, scan=scan,
                        panel=panel, header=self.header,
                        outfile=outfile)
        locinfo = {'npanel': 2, 'title0': 'Scan 23 (OrionS)'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testsel02( self ):
        """testsel02: spw='1, 3~12,>= 14'"""
        tid="02"
        outfile = self.figroot+tid+self.figsuff
        #spw='1, 3~12,>= 14'
        spw='1, 3~12,> 13'
        panel = 'i'
        result = sdplot(infile=self.infile, spw=spw,
                        panel=panel, header=self.header,
                        outfile=outfile)
        locinfo = {'npanel': 5, 'title0': 'IF1'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testsel03( self ):
        """testsel03: pol='>0'"""
        tid="03"
        outfile = self.figroot+tid+self.figsuff
        pol='>0'
        panel = 'p'
        result = sdplot(infile=self.infile, pol=pol,
                        panel=panel, header=self.header,
                        outfile=outfile)
        locinfo = {'npanel': 1, 'nstack': 1, 'title0': 'LL'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

    def testsel04( self ):
        """testsel04: beam='<1'"""
        tid="04"
        outfile = self.figroot+tid+self.figsuff
        beam='<1'
        panel='b'
        result = sdplot(infile=self.infile, beam=beam,
                        panel=panel, header=self.header,
                        outfile=outfile)
        locinfo = {'npanel': 1, 'title0': 'Beam 0'}
        refinfo = self._mergeDict(self.baseinfo,locinfo)
        # Tests
        self.assertEqual(result,None)
        currinfo = self._get_plot_info()
        self._compareDictVal(currinfo, refinfo)
        self._checkOutFile(outfile,self.compare)

class sdplot_selectionTest(selection_syntax.SelectionSyntaxTest,sdplot_unittest_base,unittest.TestCase):
    """
    Test selection syntax. Selection parameters to test are:
    field, spw (no channel selection), scan, pol, beam
    
    The dataset used for this test is sd_analytic_type1-3.asap.
    """
    # Input and output names
    infile='sd_analytic_type1-3.asap'
    prefix=sdplot_unittest_base.taskname+'TestSel'
    postfix='.plot.png'
    outfile=prefix+postfix

    def _get_selection_plot_info( self, is_field=None ):
        if is_field is None: is_field = False
        
        retdic = {}
        titles = []
        npanel = len(sd.plotter._plotter.subplots)
        for i in xrange(npanel):
            ttmp = sd.plotter._plotter.subplots[i]['axes'].get_title().upper().strip()
            if is_field:
                title = ttmp.split('(')[1].split(')')[0].strip()
            else:
                if len(ttmp.split(' ')) > 1: # scan/beam
                    title = ttmp.split(' ')[1]
                elif ttmp[0:2].upper() == 'IF': # spw
                    title = ttmp.strip()[2:]
                #elif ttmp[0:4].upper() == 'BEAM': # beam
                #    title = ttmp.strip()[4:]
                else: # pol
                    title = ttmp
            if title not in titles:
                titles.append(title)
        titles.sort()
        retdic['title'] = titles

        return retdic
    
    def _set_selection_plot_info( self, fields, which_param=None ):
        retdic = {}
        flist = []
        for field in fields:
            if which_param == 'pol':
                if field == 0:
                    val = 'XX'
                else:
                    val = 'YY'
                flist.append(val)
            elif which_param == 'field':
                try:
                    fid = int(field)
                    if fid == 5:
                        sfield = 'M100'
                    elif fid == 6:
                        sfield = 'M100'
                    elif fid == 7:
                        sfield = 'M30'
                    elif fid == 8:
                        sfield = '3C273'
                    else:
                        raise Exception("bad field ID (" + str(fid) + ")")
                except:
                    sfield = field.upper().strip()
                finally:
                    if sfield not in flist:
                        flist.append(sfield)
            else:
                flist.append(str(field))
                
        flist.sort()
        retdic['title'] = flist
        return retdic

    @property
    def task(self):
        return sdplot
    
    @property
    def spw_channel_selection(self):
        return False

    def setUp(self):
        # switch on/off GUI
        self._switchPlotterGUI(self.usegui)
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copytree(self.datapath+self.infile, self.infile)

        default(sdplot)

    def tearDown(self):
        # restore GUI setting
        self._switchPlotterGUI(self.oldgui)
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        os.system( 'rm -rf '+self.prefix+'*' )

    ####################
    # scan
    ####################
    def test_scan_id_default(self):
        """test scan selection (scan='')"""
        scan=''
        self.res=sdplot(scan=scan,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([15,16,17])
        self._compareDictVal(outinfo, refinfo)
    
    def test_scan_id_exact(self):
        """ test scan selection (scan='15')"""
        scan='15'
        self.res=sdplot(scan=scan,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([15])
        self._compareDictVal(outinfo, refinfo)
        
    def test_scan_id_lt(self):
        """ test scan selection (scan='<17')"""
        scan = '<17'
        self.res=sdplot(scan=scan,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([15,16])
        self._compareDictVal(outinfo, refinfo)
    
    def test_scan_id_gt(self):
        """ test scan selection (scan='>15')"""
        scan = '>15'
        self.res=sdplot(scan=scan,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([16,17])
        self._compareDictVal(outinfo, refinfo)
    
    def test_scan_id_range(self):
        """ test scan selection (scan='15~16')"""
        scan = '15~16'
        self.res=sdplot(scan=scan,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([15,16])
        self._compareDictVal(outinfo, refinfo)
    
    def test_scan_id_list(self):
        """ test scan selection (scan='15,17')"""
        scan = '15,17'
        self.res=sdplot(scan=scan,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([15,17])
        self._compareDictVal(outinfo, refinfo)
    
    def test_scan_id_exprlist(self):
        """ test scan selection (scan='<16, 17')"""
        scan = '<16, 17'
        self.res=sdplot(scan=scan,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([15,17])
        self._compareDictVal(outinfo, refinfo)
    
    ####################
    # pol
    ####################
    def test_pol_id_default(self):
        """test pol selection (pol='')"""
        pol=''
        self.res=sdplot(pol=pol,panel='p',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([0,1], 'pol')
        self._compareDictVal(outinfo, refinfo)
    
    def test_pol_id_exact(self):
        """ test pol selection (pol='1')"""
        pol = '1'
        self.res=sdplot(pol=pol,panel='p',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([1], 'pol')
        self._compareDictVal(outinfo, refinfo)
    
    def test_pol_id_lt(self):
        """ test pol selection (pol='<1')"""
        outname=self.prefix+self.postfix
        pol = '<1'
        self.res=sdplot(pol=pol,panel='p',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([0], 'pol')
        self._compareDictVal(outinfo, refinfo)
    
    def test_pol_id_gt(self):
        """ test pol selection (pol='>0')"""
        pol = '>0'
        self.res=sdplot(pol=pol,panel='p',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([1], 'pol')
        self._compareDictVal(outinfo, refinfo)
    
    def test_pol_id_range(self):
        """ test pol selection (pol='0~1')"""
        pol = '0~1'
        self.res=sdplot(pol=pol,panel='p',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([0,1], 'pol')
        self._compareDictVal(outinfo, refinfo)
    
    def test_pol_id_list(self):
        """ test pol selection (pol='0,1')"""
        pol = '0,1'
        self.res=sdplot(pol=pol,panel='p',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([0,1], 'pol')
        self._compareDictVal(outinfo, refinfo)
    
    def test_pol_id_exprlist(self):
        """test pol selection (pol='<1,1')"""
        pol='<1,1'
        self.res=sdplot(pol=pol,panel='p',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([0,1], 'pol')
        self._compareDictVal(outinfo, refinfo)

    ####################
    # field
    ####################
    def test_field_value_default(self):
        """test field selection (field='')"""
        field=''
        self.res=sdplot(field=field,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info(True)
        refinfo = self._set_selection_plot_info([5,6,7,8], 'field')
        self._compareDictVal(outinfo, refinfo)
    
    def test_field_id_exact(self):
        """ test field selection (field='6')"""
        field = '6'
        self.res=sdplot(field=field,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info(True)
        refinfo = self._set_selection_plot_info([6], 'field')
        self._compareDictVal(outinfo, refinfo)

    def test_field_id_lt(self):
        """ test field selection (field='<6')"""
        field = '<6'
        self.res=sdplot(field=field,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info(True)
        refinfo = self._set_selection_plot_info([5], 'field')
        self._compareDictVal(outinfo, refinfo)

    def test_field_id_gt(self):
        """ test field selection (field='>7')"""
        field = '>7'
        self.res=sdplot(field=field,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info(True)
        refinfo = self._set_selection_plot_info([8], 'field')
        self._compareDictVal(outinfo, refinfo)
    
    def test_field_id_range(self):
        """ test field selection (field='5~7')"""
        field = '5~7'
        self.res=sdplot(field=field,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info(True)
        refinfo = self._set_selection_plot_info([5,6,7], 'field')
        self._compareDictVal(outinfo, refinfo)
    
    def test_field_id_list(self):
        """ test field selection (field='5,7')"""
        field = '5,7'
        self.res=sdplot(field=field,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info(True)
        refinfo = self._set_selection_plot_info([5,7], 'field')
        self._compareDictVal(outinfo, refinfo)

    def test_field_id_exprlist(self):
        """ test field selection (field='<7,8')"""
        field = '<7,8'
        self.res=sdplot(field=field,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info(True)
        refinfo = self._set_selection_plot_info([6,8], 'field')
        self._compareDictVal(outinfo, refinfo)
    
    def test_field_value_exact(self):
        """ test field selection (field='M100')"""
        field = 'M100'
        self.res=sdplot(field=field,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info(True)
        refinfo = self._set_selection_plot_info(['M100'], 'field')
        self._compareDictVal(outinfo, refinfo)
    
    def test_field_value_pattern(self):
        """ test field selection (field='M*')"""
        field = 'M*'
        self.res=sdplot(field=field,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info(True)
        refinfo = self._set_selection_plot_info(['M100','M30'], 'field')
        self._compareDictVal(outinfo, refinfo)
    
    def test_field_value_list(self):
        """ test field selection (field='M30,3C273')"""
        field = 'M30,3C273'
        self.res=sdplot(field=field,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info(True)
        refinfo = self._set_selection_plot_info(['M30','3C273'], 'field')
        self._compareDictVal(outinfo, refinfo)
    
    def test_field_mix_exprlist(self):
        """ test field selection (field='<7,3C273')"""
        field = '<7,3C273'
        self.res=sdplot(field=field,panel='s',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info(True)
        refinfo = self._set_selection_plot_info([6,'3C273'], 'field')
        self._compareDictVal(outinfo, refinfo)
    
    ####################
    # spw 
    ####################
    def test_spw_id_default(self):
        """test spw selection (spw='')"""
        spw=''
        self.res=sdplot(spw=spw,infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([20,21,22,23,24,25])
        self._compareDictVal(outinfo, refinfo)
    
    def test_spw_id_exact(self):
        """ test spw selection (spw='21')"""
        spw = '21'
        self.res=sdplot(spw=spw,infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([21])
        self._compareDictVal(outinfo, refinfo)
    
    def test_spw_id_lt(self):
        """ test spw selection (spw='<25')"""
        spw = '<25'
        self.res=sdplot(spw=spw,infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([20,21,22,23,24])
        self._compareDictVal(outinfo, refinfo)
    
    def test_spw_id_gt(self):
        """ test spw selection (spw='>21')"""
        spw = '>21'
        self.res=sdplot(spw=spw,infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([22,23,24,25])
        self._compareDictVal(outinfo, refinfo)
    
    def test_spw_id_range(self):
        """ test spw selection (spw='21~24')"""
        spw = '21~24'
        self.res=sdplot(spw=spw,infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([21,22,23,24])
        self._compareDictVal(outinfo, refinfo)
    
    def test_spw_id_list(self):
        """ test spw selection (spw='21,22,23,25')"""
        spw = '21,22,23,25'
        self.res=sdplot(spw=spw,infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([21,22,23,25])
        self._compareDictVal(outinfo, refinfo)
    
    def test_spw_id_exprlist(self):
        """ test spw selection (spw='<22,>24')"""
        spw = '<22,>24'
        self.res=sdplot(spw=spw,infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([20,21,25])
        self._compareDictVal(outinfo, refinfo)
    
    def test_spw_id_pattern(self):
        """test spw selection (spw='*')"""
        spw='*'
        self.res=sdplot(spw=spw,infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([20,21,22,23,24,25])
        self._compareDictVal(outinfo, refinfo)
    
    def test_spw_value_frequency(self):
        """test spw selection (spw='299.5~310GHz')"""
        spw = '299.5~310GHz'
        self.res=sdplot(spw=spw,infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([22,23,24,25])
        self._compareDictVal(outinfo, refinfo)
    
    def test_spw_value_velocity(self):
        """test spw selection (spw='-50~50km/s')"""
        spw = '-50~50km/s'
        self.res=sdplot(spw=spw,infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([22,23])
        self._compareDictVal(outinfo, refinfo)
    
    def test_spw_mix_exprlist(self):
        """test spw selection (spw='150~550km/s,>23')"""
        spw = '150~550km/s,>23'
        self.res=sdplot(spw=spw,infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([20,21,24,25])
        self._compareDictVal(outinfo, refinfo)
    
    ####################
    # beam
    ####################
    def test_beam_id_default(self):
        """test beam selection (beam='')"""
        beam=''
        self.res=sdplot(beam=beam,panel='b',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([11,12,13])
        self._compareDictVal(outinfo, refinfo)
    
    def test_beam_id_exact(self):
        """ test beam selection (beam='12')"""
        beam='12'
        self.res=sdplot(beam=beam,panel='b',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([12])
        self._compareDictVal(outinfo, refinfo)
    
    def test_beam_id_lt(self):
        """test beam selection (beam='<13')"""
        beam='<13'
        self.res=sdplot(beam=beam,panel='b',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([11,12])
        self._compareDictVal(outinfo, refinfo)
    
    def test_beam_id_gt(self):
        """test beam selection (beam='>11')"""
        beam='>11'
        self.res=sdplot(beam=beam,panel='b',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([12,13])
        self._compareDictVal(outinfo, refinfo)
    
    def test_beam_id_range(self):
        """test beam selection (beam='12~13')"""
        beam='12~13'
        self.res=sdplot(beam=beam,panel='b',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([12,13])
        self._compareDictVal(outinfo, refinfo)
    
    def test_beam_id_list(self):
        """test beam selection (beam='11,13')"""
        beam='11,13'
        self.res=sdplot(beam=beam,panel='b',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([11,13])
        self._compareDictVal(outinfo, refinfo)
    
    def test_beam_id_exprlist(self):
        """test beam selection (beam='<12,>12')"""
        beam='<12,>12'
        self.res=sdplot(beam=beam,panel='b',infile=self.infile,outfile=self.outfile)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        outinfo = self._get_selection_plot_info()
        refinfo = self._set_selection_plot_info([11,13])
        self._compareDictVal(outinfo, refinfo)

def suite():
    return [#sdplot_basicTest, sdplot_storageTest, sdplot_gridTest,
            ##sdplot_selectTest,
            #sdplot_errorTest,
            sdplot_selectionTest]
