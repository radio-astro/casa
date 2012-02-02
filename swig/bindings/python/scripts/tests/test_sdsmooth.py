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
#from numpy import array

asap_init()
import asap as sd
from sdsmooth import sdsmooth
#from sdstat import sdstat

class sdsmooth_test(unittest.TestCase):
    """
    Basic unit tests for task sdsmooth. No interactive testing.

    The list of tests:
    test00    --- default parameters (raises an errror)
    test01    --- default + valid input filename (hanning smooth)
    test02-03 --- kernel = 'boxcar' w/ and w/o kwidth param
    test04-05 --- kernel = 'gaussian' w/ and w/o kwidth param
    test06    --- overwrite = True

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
      sdstat(infile=outfile,masklist=linechan2)
      sdstat(infile=outfile,masklist=basechan2)
    """
    ### TODO:
    ### - need checking for flag application
    ### - comparison with simple and known spectral shape

    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdsmooth/'
    # Input and output names
    infile = 'OrionS_rawACSmod_if2_calTPave_bl.asap'
    outroot = 'sdsmooth_test'

    ### helper function to calculate necessary stat
    def _getStats(self,filename):
        linechan2 = [[2951,3088]]
        basechan2 = [[500,2950],[3089,7692]]
        self.assertTrue(os.path.exists(filename),
                        msg=("Output file '%s' doesn't exist" % filename))
        scan = sd.scantable(filename,average=False)
        # only the first row is calculated
        linmsk = scan.create_mask(linechan2)
        blmsk = scan.create_mask(basechan2)
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


    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(sdsmooth)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    def test00(self):
        """Test 0: Default parameters (raises an errror)"""
        result = sdsmooth()
        self.assertFalse(result)

    def test01(self):
        """Test 1: Default parameters + valid input filename (hanning smooth)"""
        self.tid="01"
        infile = self.infile
        outfile = self.outroot+self.tid+'.asap'

        result = sdsmooth(infile=infile,outfile=outfile)
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
        self.tid="02"
        infile = self.infile
        outfile = self.outroot+self.tid+'.asap'
        kernel = 'boxcar'

        result =sdsmooth(infile=infile,outfile=outfile,kernel=kernel)
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
        self.tid="03"
        infile = self.infile
        outfile = self.outroot+self.tid+'.asap'
        kernel = 'boxcar'
        kwidth = 16

        result =sdsmooth(infile=infile,outfile=outfile,
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
        self.tid="04"
        infile = self.infile
        outfile = self.outroot+self.tid+'.asap'
        kernel = 'gaussian'

        result =sdsmooth(infile=infile,outfile=outfile,kernel=kernel)
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
        self.tid="05"
        infile = self.infile
        outfile = self.outroot+self.tid+'.asap'
        kernel = 'gaussian'
        kwidth = 16

        result =sdsmooth(infile=infile,outfile=outfile,
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
        self.tid="06"
        infile = self.infile
        outfile = self.outroot+self.tid+'.asap'

        # the fist run with hanning smooth
        print "The 1st run of sdsmooth (hanning smooth)"
        result1 =sdsmooth(infile=infile,outfile=outfile)
        self.assertEqual(result1,None,
                         msg="The task returned '"+str(result1)+"' instead of None for the 1st run")
        self.assertTrue(os.path.exists(outfile),
                        msg="Output file doesn't exist after the 1st run.")

        # overwrite 'outfile'
        # the second run with kernel = 'gaussian'
        kernel = 'gaussian'
        print "The 2nd run of sdsmooth (OVERWRITE with Gaussian smooth)"
        result2 =sdsmooth(infile=infile,outfile=outfile,
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


def suite():
    return [sdsmooth_test]
