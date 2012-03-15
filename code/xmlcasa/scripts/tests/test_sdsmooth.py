import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from asap_init import * 
import unittest
#
#import listing
import numpy

asap_init()
import asap as sd
from sdsmooth import sdsmooth
#from sdstat import sdstat

#
# Unit test of sdsmooth task.
#

class sdsmooth_unittest_base:
    """
    Base class for sdsmooth unit test
    """
    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdsmooth/'
    taskname = "sdsmooth"

    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertTrue(isthere,
                         msg='output file %s was not created because of the task failure'%(name))

    ### helper function to calculate necessary stat
    def _getStats(self,filename, linechan=None, basechan=None):
        if not linechan: linechan = self.linechan
        if not basechan: basechan = self.basechan
        self._checkfile(filename)
        scan = sd.scantable(filename,average=False)
        # only the first row is calculated
        linmsk = scan.create_mask(linechan)
        blmsk = scan.create_mask(basechan)
        linmax = scan.stats('max',mask=linmsk)[0]
        linmaxpos = scan.stats('max_abc',mask=linmsk)[0]
        lineqw = scan.stats('sum',mask=linmsk)[0]/linmax
        blrms = scan.stats('rms',mask=blmsk)[0]
        del scan, linmsk, blmsk
        retdic = {'linemax': linmax, 'linemaxpos': linmaxpos,
                  'lineeqw': lineqw, 'baserms': blrms}
        del linmax, linmaxpos, lineqw, blrms
        return retdic

    def _compareDictVal(self,testdict, refdict,places=4):
        for stat, refval in refdict.iteritems():
            self.assertTrue(testdict.has_key(stat),
                            msg = "'%s' is not defined in the current run" % stat)
            print "Comparing '%s': %f (current run), %f (reference)" % \
                  (stat,testdict[stat],refval)
            self.assertAlmostEqual(testdict[stat],refval,places=places)


class sdsmooth_badinputs(sdsmooth_unittest_base,unittest.TestCase):
    """
    Tests on bad input parameter setting
    """
    inname = 'OrionS_rawACSmod_if2_calTPave_bl.asap'
    outname = sdsmooth_unittest_base.taskname+'_bad.asap'

    badname = "bad"
    badquant = "5bad"
    
    def setUp(self):
        if os.path.exists(self.inname):
            shutil.rmtree(self.inname)
        if os.path.exists(self.outname):
            shutil.rmtree(self.outname)
        shutil.copytree(self.datapath+self.inname, self.inname)

        default(sdsmooth)

    def tearDown(self):
        if (os.path.exists(self.inname)):
            shutil.rmtree(self.inname)

    # Tests on invalid parameter sets
    def test_default(self):
        """Test Default parameters (raises an errror)"""
        res = sdsmooth()
        self.assertFalse(res)

    def testBad_kernel(self):
        """Test bad kernel name"""
        kernel = self.badname

        res = sdsmooth(infile=self.inname,outfile=self.outname,
                       kernel=kernel)
        self.assertFalse(res)

    def testBad_chanwidth(self):
        """Test bad chanwidth"""
        kernel = 'regrid'
        chanwidth = self.badquant

        res = sdsmooth(infile=self.inname,outfile=self.outname,
                       kernel=kernel,chanwidth=chanwidth)
        self.assertFalse(res)


class sdsmooth_test(sdsmooth_unittest_base,unittest.TestCase):
    """
    Basic unit tests for task sdsmooth. No interactive testing.

    The list of tests:
    test00    --- default parameters (raises an errror)
    test01    --- default + valid input filename (hanning smooth)
    test02-03 --- kernel = 'boxcar' w/ and w/o kwidth param
    test04-05 --- kernel = 'gaussian' w/ and w/o kwidth param
    test06    --- overwrite = True
    test07    --- kernel = 'regrid' with chwidth in channel unit
    test08    --- kernel = 'regrid' with chwidth in frequency unit
    test09    --- kernel = 'regrid' with chwidth in velocity unit

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdaverage)
      sdaverage(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile='temp.asap')
      default(sdaverage)
      sdaverage(infile='temp.asap',timeaverage=True,tweight='tintsys',
                polaverage=True,pweight='tsys',outfile='temp2.asap')
      default(sdbaseline)
      sdbaseline(infile='temp2.asap',iflist=[2],masklist=blchan2,
                 maskmode='list',outfile=self.infile)

      The reference stat values can be obtained by
      sdstat(infile=outfile,masklist=linechan)
      sdstat(infile=outfile,masklist=basechan)
    """
    ### TODO:
    ### - need checking for flag application
    ### - comparison with simple and known spectral shape

    # Input and output names
    infile = 'OrionS_rawACSmod_if2_calTPave_bl.asap'
    outroot = sdsmooth_unittest_base.taskname+'_test'
    linechan = [[2951,3088]]
    basechan = [[500,2950],[3089,7692]]
    regridw = 4
    reglinech = [[737,772]]
    regbasech = [[125,736],[773,1923]]

    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(sdsmooth)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    def test01(self):
        """Test 1: Default parameters + valid input filename (hanning smooth)"""
        tid="01"
        outfile = self.outroot+tid+'.asap'

        result = sdsmooth(infile=self.infile,outfile=outfile)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 1.9679156541824341,
                  'linemaxpos': 3049.0,
                  'lineeqw': 61.521829228644677,
                  'baserms': 0.17469978332519531}
        testval = self._getStats(outfile)
        self._compareDictVal(refdic, testval)

    def test02(self):
        """Test 2: kernel = 'boxcar'"""
        tid="02"
        outfile = self.outroot+tid+'.asap'
        kernel = 'boxcar'

        result =sdsmooth(infile=self.infile,outfile=outfile,kernel=kernel)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 1.9409093856811523,
                  'linemaxpos': 3048.0,
                  'lineeqw': 62.508279566880944,
                  'baserms': 0.15886218845844269}
        testval = self._getStats(outfile)
        self._compareDictVal(refdic, testval)

    def test03(self):
        """Test 3: kernel = 'boxcar' + kwidth = 16"""
        tid="03"
        outfile = self.outroot+tid+'.asap'
        kernel = 'boxcar'
        kwidth = 16

        result =sdsmooth(infile=self.infile,outfile=outfile,
                         kernel=kernel,kwidth=kwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 1.8286358118057251,
                  'linemaxpos': 3047.0,
                  'lineeqw': 65.946254391136108,
                  'baserms': 0.096009217202663422}
        testval = self._getStats(outfile)
        self._compareDictVal(refdic, testval)


    def test04(self):
        """Test 4: kernel = 'gaussian'"""
        tid="04"
        outfile = self.outroot+tid+'.asap'
        kernel = 'gaussian'

        result =sdsmooth(infile=self.infile,outfile=outfile,kernel=kernel)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 1.9107955694198608,
                  'linemaxpos': 3049.0,
                  'lineeqw': 63.315141667417912,
                  'baserms': 0.14576216042041779}
        testval = self._getStats(outfile)
        self._compareDictVal(refdic, testval)

    def test05(self):
        """Test 5: kernel = 'gaussian' + kwidth = 16"""
        tid="05"
        outfile = self.outroot+tid+'.asap'
        kernel = 'gaussian'
        kwidth = 16

        result =sdsmooth(infile=self.infile,outfile=outfile,
                         kernel=kernel,kwidth=kwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 1.762944221496582,
                  'linemaxpos': 3046.0,
                  'lineeqw': 68.064315277502047,
                  'baserms': 0.074664078652858734}
        testval = self._getStats(outfile)
        self._compareDictVal(refdic, testval)

    def test06(self):
        """Test 6: Test overwrite=True"""
        tid="06"
        outfile = self.outroot+tid+'.asap'

        # the fist run with hanning smooth
        print "The 1st run of sdsmooth (hanning smooth)"
        result1 =sdsmooth(infile=self.infile,outfile=outfile)
        self.assertEqual(result1,None,
                         msg="The task returned '"+str(result1)+"' instead of None for the 1st run")
        self.assertTrue(os.path.exists(outfile),
                        msg="Output file doesn't exist after the 1st run.")

        # overwrite 'outfile'
        # the second run with kernel = 'gaussian'
        kernel = 'gaussian'
        print "The 2nd run of sdsmooth (OVERWRITE with Gaussian smooth)"
        result2 =sdsmooth(infile=self.infile,outfile=outfile,
                          kernel=kernel,overwrite=True)
        self.assertEqual(result2,None,
                         msg="The task returned '"+str(result2)+"' instead of None for the 2nd run")
        self.assertTrue(os.path.exists(outfile),
                        msg="Output file doesn't exist after the 2nd run.")
        # Should be the same value as test04
        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 1.9107955694198608,
                  'linemaxpos': 3049.0,
                  'lineeqw': 63.315141667417912,
                  'baserms': 0.14576216042041779}
        testval = self._getStats(outfile)
        self._compareDictVal(refdic, testval)


    def test07(self):
        """Test 7: kernel = 'regrid' + chanwidth in channel unit"""
        tid="07"
        unit='channel'
        scan = sd.scantable(self.infile, average=False)
        scan.set_unit(unit)
        nch_old = scan.nchan(scan.getif(0))
        del scan
        
        outfile = self.outroot+tid+'.asap'
        kernel = 'regrid'
        chanwidth = str(self.regridw)

        result =sdsmooth(infile=self.infile,outfile=outfile,
                         kernel=kernel,chanwidth=chanwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 1.9440968036651611,
                  'linemaxpos': 762.0,
                  'lineeqw': 15.466022935853511,
                  'baserms': 0.16496795415878296}
        testval = self._getStats(outfile,self.reglinech,self.regbasech)
        self._compareDictVal(refdic, testval)
        # channel number check
        scan = sd.scantable(outfile, average=False)
        scan.set_unit(unit)
        nch_new = scan.nchan(scan.getif(0))
        del scan
        self.assertEqual(nch_new,numpy.ceil(nch_old/self.regridw))
        # TODO: add tests of FREQUENCY subtable

    def test08(self):
        """Test 8: kernel = 'regrid' + chanwidth in frequency unit"""
        tid="08"
        unit="Hz"
        # get channel number and width in input data
        scan = sd.scantable(self.infile, average=False)
        scan.set_unit(unit)
        nch_old = scan.nchan(scan.getif(0))
        fx = scan._getabcissa(0)
        chw_old = (fx[nch_old-1]-fx[0])/float(nch_old-1)
        del scan
        chw_new = chw_old*self.regridw
        
        outfile = self.outroot+tid+'.asap'
        kernel = 'regrid'
        chanwidth = qa.tos(qa.quantity(chw_new,unit))

        result =sdsmooth(infile=self.infile,outfile=outfile,
                         kernel=kernel,chanwidth=chanwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 1.9440968036651611,
                  'linemaxpos': 762.0,
                  'lineeqw': 15.466022935853511,
                  'baserms': 0.16496795415878296}
        testval = self._getStats(outfile,self.reglinech,self.regbasech)
        self._compareDictVal(refdic, testval)
        # channel number check
        scan = sd.scantable(outfile, average=False)
        scan.set_unit(unit)
        nch_new = scan.nchan(scan.getif(0))
        del scan
        self.assertEqual(nch_new,numpy.ceil(nch_old/self.regridw))
        # TODO: add tests of FREQUENCY subtable

    def test09(self):
        """Test 9: kernel = 'regrid' + chanwidth in velocity unit"""
        tid="09"
        unit="km/s"
        # get channel number and width in input data
        scan = sd.scantable(self.infile, average=False)
        scan.set_unit(unit)
        nch_old = scan.nchan(scan.getif(0))
        fx = scan._getabcissa(0)
        chw_old = (fx[nch_old-1]-fx[0])/float(nch_old-1)
        del scan
        chw_new = chw_old*self.regridw

        outfile = self.outroot+tid+'.asap'
        kernel = 'regrid'
        chanwidth = qa.tos(qa.quantity(chw_new,unit))

        result =sdsmooth(infile=self.infile,outfile=outfile,
                         kernel=kernel,chanwidth=chanwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 1.9440968036651611,
                  'linemaxpos': 762.0,
                  'lineeqw': 15.466022935853511,
                  'baserms': 0.16496795415878296}
        testval = self._getStats(outfile,self.reglinech,self.regbasech)
        self._compareDictVal(refdic, testval)
        # channel number check
        scan = sd.scantable(outfile, average=False)
        scan.set_unit(unit)
        nch_new = scan.nchan(scan.getif(0))
        del scan
        self.assertEqual(nch_new,numpy.ceil(nch_old/self.regridw))
        # TODO: add tests of FREQUENCY subtable


def suite():
    return [sdsmooth_badinputs,sdsmooth_test]
