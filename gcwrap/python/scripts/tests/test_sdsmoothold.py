import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
#
#import listing
import numpy

import asap as sd
from sdsmoothold import sdsmoothold
#from sdstatold import sdstatold

#
# Unit test of sdsmoothold task.
#

class sdsmoothold_unittest_base:
    """
    Base class for sdsmoothold unit test
    """
    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdsmooth/'
    taskname = "sdsmoothold"

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


class sdsmoothold_badinputs(sdsmoothold_unittest_base,unittest.TestCase):
    """
    Tests on bad input parameter setting
    """
    inname = 'OrionS_rawACSmod_if2_calTPave_bl.asap'
    outname = sdsmoothold_unittest_base.taskname+'_bad.asap'

    badname = "bad"
    badquant = "5bad"
    
    def setUp(self):
        if os.path.exists(self.inname):
            shutil.rmtree(self.inname)
        if os.path.exists(self.outname):
            shutil.rmtree(self.outname)
        shutil.copytree(self.datapath+self.inname, self.inname)

        default(sdsmoothold)

    def tearDown(self):
        if (os.path.exists(self.inname)):
            shutil.rmtree(self.inname)

    # Tests on invalid parameter sets
    def test_default(self):
        """Test Default parameters (raises an errror)"""
        # argument verification error
        res = sdsmoothold()
        self.assertFalse(res)

    def testBad_kernel(self):
        """Test bad kernel name"""
        kernel = self.badname

        # argument verification error
        res = sdsmoothold(infile=self.inname,outfile=self.outname,
                       kernel=kernel)
        self.assertFalse(res)

    def testBad_chanwidth(self):
        """Test bad chanwidth"""
        kernel = 'regrid'
        chanwidth = self.badquant

        try:
            res = sdsmoothold(infile=self.inname,outfile=self.outname,
                           kernel=kernel,chanwidth=chanwidth)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Invalid quantity chanwidth 5bad')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))


class sdsmoothold_basicTest(sdsmoothold_unittest_base,unittest.TestCase):
    """
    Basic unit tests for task sdsmoothold. No interactive testing.

    The list of tests:
    test00    --- default parameters (raises an errror)
    test01    --- default + valid input filename (hanning smooth)
    test02-03 --- kernel = 'boxcar' w/ and w/o kwidth param
    test04-05 --- kernel = 'gaussian' w/ and w/o kwidth param
    test06    --- overwrite = True
    test07,10 --- kernel = 'regrid' with chwidth in channel unit
    test08,11 --- kernel = 'regrid' with chwidth in frequency unit
    test09,12 --- kernel = 'regrid' with chwidth in velocity unit

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdcal)
      sdcal(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile='temp.asap')
      default(sdcal)
      sdcal(infile='temp.asap',timeaverage=True,tweight='tintsys',
                polaverage=True,pweight='tsys',outfile='temp2.asap')
      default(sdbaseline)
      sdbaseline(infile='temp2.asap',iflist=[2],masklist=blchan2,
                 maskmode='list',outfile=self.infile)

      The reference stat values can be obtained by
      sdstatold(infile=outfile,masklist=linechan)
      sdstatold(infile=outfile,masklist=basechan)
    """
    ### TODO:
    ### - comparison with simple and known spectral shape

    # Input and output names
    infile = 'OrionS_rawACSmod_if2_calTPave_bl.asap'
    outroot = sdsmoothold_unittest_base.taskname+'_test'
    linechan = [[2951,3088]]
    basechan = [[500,2950],[3089,7692]]
    regridw = 4
    reglinech = [[737,772]]
    regbasech = [[125,736],[773,1923]]

    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(sdsmoothold)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    def test01(self):
        """Test 1: Default parameters + valid input filename (hanning smooth)"""
        tid="01"
        outfile = self.outroot+tid+'.asap'

        result = sdsmoothold(infile=self.infile,outfile=outfile)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 1.9679156541824341,
                  'linemaxpos': 3049.0,
                  'lineeqw': 61.521829228644677,
                  'baserms': 0.17469978332519531}
        testval = self._getStats(outfile)
        self._compareDictVal(testval, refdic)

    def test02(self):
        """Test 2: kernel = 'boxcar'"""
        tid="02"
        outfile = self.outroot+tid+'.asap'
        kernel = 'boxcar'

        result =sdsmoothold(infile=self.infile,outfile=outfile,kernel=kernel)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 1.9409093856811523,
                  'linemaxpos': 3048.0,
                  'lineeqw': 62.508279566880944,
                  'baserms': 0.15886218845844269}
        testval = self._getStats(outfile)
        self._compareDictVal(testval, refdic)

    def test03(self):
        """Test 3: kernel = 'boxcar' + kwidth = 16"""
        tid="03"
        outfile = self.outroot+tid+'.asap'
        kernel = 'boxcar'
        kwidth = 16

        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,kwidth=kwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 1.8286358118057251,
                  'linemaxpos': 3047.0,
                  'lineeqw': 65.946254391136108,
                  'baserms': 0.096009217202663422}
        testval = self._getStats(outfile)
        self._compareDictVal(testval, refdic)


    def test04(self):
        """Test 4: kernel = 'gaussian'"""
        tid="04"
        outfile = self.outroot+tid+'.asap'
        kernel = 'gaussian'

        result =sdsmoothold(infile=self.infile,outfile=outfile,kernel=kernel)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 1.9107955694198608,
                  'linemaxpos': 3049.0,
                  'lineeqw': 63.315141667417912,
                  'baserms': 0.14576216042041779}
        testval = self._getStats(outfile)
        self._compareDictVal(testval, refdic)

    def test05(self):
        """Test 5: kernel = 'gaussian' + kwidth = 16"""
        tid="05"
        outfile = self.outroot+tid+'.asap'
        kernel = 'gaussian'
        kwidth = 16

        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,kwidth=kwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 1.762944221496582,
                  'linemaxpos': 3046.0,
                  'lineeqw': 68.064315277502047,
                  'baserms': 0.074664078652858734}
        testval = self._getStats(outfile)
        self._compareDictVal(testval, refdic)

    def test06(self):
        """Test 6: Test overwrite=True"""
        tid="06"
        outfile = self.outroot+tid+'.asap'

        # the fist run with hanning smooth
        print "The 1st run of sdsmoothold (hanning smooth)"
        result1 =sdsmoothold(infile=self.infile,outfile=outfile)
        self.assertEqual(result1,None,
                         msg="The task returned '"+str(result1)+"' instead of None for the 1st run")
        self.assertTrue(os.path.exists(outfile),
                        msg="Output file doesn't exist after the 1st run.")

        # overwrite 'outfile'
        # the second run with kernel = 'gaussian'
        kernel = 'gaussian'
        print "The 2nd run of sdsmoothold (OVERWRITE with Gaussian smooth)"
        result2 =sdsmoothold(infile=self.infile,outfile=outfile,
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
        self._compareDictVal(testval, refdic)


    def test07(self):
        """Test 7: kernel = 'regrid' + chanwidth in channel unit"""
        tid="07"
        unit='channel'
        scan = sd.scantable(self.infile, average=False)
        oldunit = scan.get_unit()
        scan.set_unit(unit)
        nch_old = scan.nchan(scan.getif(0))
        scan.set_unit(oldunit)
        del scan
        
        outfile = self.outroot+tid+'.asap'
        kernel = 'regrid'
        chanwidth = str(self.regridw)

        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,chanwidth=chanwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2435 + CASA r18782)
        refdic = {'linemax': 1.9440968036651611,
                  'linemaxpos': 762.0,
                  'lineeqw': 15.466022935853511,
                  'baserms': 0.16496795415878296}
        refsc = {"ch0": 44049935561.916985,
                 "incr": 24416.931915037214}
        testval = self._getStats(outfile,self.reglinech,self.regbasech)
        self._compareDictVal(testval, refdic)
        # spectral coordinate check
        scan = sd.scantable(outfile, average=False)
        scan.set_unit("Hz")
        sc_new = scan._getabcissa(0)
        del scan
        nch_new = len(sc_new)
        ch0_new = sc_new[0]
        incr_new = (sc_new[nch_new-1]-sc_new[0])/float(nch_new-1)
        
        self.assertEqual(nch_new,numpy.ceil(nch_old/self.regridw))
        self.assertAlmostEqual((ch0_new-refsc['ch0'])/refsc['ch0'],0.,places=5)
        self.assertAlmostEqual((incr_new-refsc['incr'])/refsc['incr'],0.,places=5)


    def test08(self):
        """Test 8: kernel = 'regrid' + chanwidth in frequency unit"""
        tid="08"
        unit="Hz"
        # get channel number and width in input data
        scan = sd.scantable(self.infile, average=False)
        oldunit = scan.get_unit()
        scan.set_unit(unit)
        nch_old = scan.nchan(scan.getif(0))
        fx = scan._getabcissa(0)
        chw_old = (fx[nch_old-1]-fx[0])/float(nch_old-1)
        scan.set_unit(oldunit)
        del scan
        chw_new = chw_old*self.regridw
        
        outfile = self.outroot+tid+'.asap'
        kernel = 'regrid'
        chanwidth = qa.tos(qa.quantity(chw_new,unit))

        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,chanwidth=chanwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2435 + CASA r18782)
        refdic = {'linemax': 1.9440968036651611,
                  'linemaxpos': 762.0,
                  'lineeqw': 15.466022935853511,
                  'baserms': 0.16496795415878296}
        refsc = {"ch0": 44049935561.916985,
                 "incr": 24416.931914787496}
        testval = self._getStats(outfile,self.reglinech,self.regbasech)
        self._compareDictVal(testval, refdic)
        # spectral coordinate check
        scan = sd.scantable(outfile, average=False)
        scan.set_unit("Hz")
        sc_new = scan._getabcissa(0)
        del scan
        nch_new = len(sc_new)
        ch0_new = sc_new[0]
        incr_new = (sc_new[nch_new-1]-sc_new[0])/float(nch_new-1)
        
        self.assertEqual(nch_new,numpy.ceil(nch_old/self.regridw))
        self.assertAlmostEqual((ch0_new-refsc['ch0'])/refsc['ch0'],0.,places=5)
        self.assertAlmostEqual((incr_new-refsc['incr'])/refsc['incr'],0.,places=5)


    def test09(self):
        """Test 9: kernel = 'regrid' + chanwidth in velocity unit"""
        tid="09"
        unit="km/s"
        # get channel number and width in input data
        scan = sd.scantable(self.infile, average=False)
        oldunit = scan.get_unit()
        scan.set_unit(unit)
        nch_old = scan.nchan(scan.getif(0))
        fx = scan._getabcissa(0)
        chw_old = (fx[nch_old-1]-fx[0])/float(nch_old-1)
        scan.set_unit(oldunit)
        del scan
        chw_new = chw_old*self.regridw

        outfile = self.outroot+tid+'.asap'
        kernel = 'regrid'
        chanwidth = qa.tos(qa.quantity(chw_new,unit))

        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,chanwidth=chanwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2435 + CASA r18782)
        refdic = {'linemax': 1.9440964460372925,
                  'linemaxpos': 762.0,
                  'lineeqw': 15.466027743114028,
                  'baserms': 0.16496789455413818}
        refsc = {"ch0": 44049935561.917,
                 "incr": 24416.931942994266}
        testval = self._getStats(outfile,self.reglinech,self.regbasech)
        self._compareDictVal(testval, refdic)
        # spectral coordinate check
        scan = sd.scantable(outfile, average=False)
        scan.set_unit("Hz")
        sc_new = scan._getabcissa(0)
        del scan
        nch_new = len(sc_new)
        ch0_new = sc_new[0]
        incr_new = (sc_new[nch_new-1]-sc_new[0])/float(nch_new-1)
        
        self.assertEqual(nch_new,numpy.ceil(nch_old/self.regridw))
        self.assertAlmostEqual((ch0_new-refsc['ch0'])/refsc['ch0'],0.,places=5)
        self.assertAlmostEqual((incr_new-refsc['incr'])/refsc['incr'],0.,places=5)

    def test10(self):
        """Test 10: kernel = 'regrid' + negative chanwidth in channel unit"""
        tid="10"
        unit='channel'
        scan = sd.scantable(self.infile, average=False)
        oldunit = scan.get_unit()
        scan.set_unit(unit)
        nch_old = scan.nchan(scan.getif(0))
        scan.set_unit(oldunit)
        del scan
        
        outfile = self.outroot+tid+'.asap'
        kernel = 'regrid'
        chanwidth = str(-self.regridw)

        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,chanwidth=chanwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2435 + CASA r18782)
        refdic = {'linemax': 1.9440968036651611,
                  'linemaxpos': 1285.0,
                  'lineeqw': 15.466022935853511,
                  'baserms': 0.16496795415878296}
        refsc = {"ch0": 44099917021.547066,
                 "incr": -24416.931915037214}
        nnew = int(numpy.ceil(nch_old/float(self.regridw)))
        reglinech = []
        regbasech = []
        for chans in self.reglinech:
            reglinech.append([nnew-1-chans[0],nnew-1-chans[1]])
        for chans in self.regbasech:
            regbasech.append([nnew-1-chans[0],nnew-1-chans[1]])
        testval = self._getStats(outfile,reglinech,regbasech)
        self._compareDictVal(testval, refdic)
        # spectral coordinate check
        scan = sd.scantable(outfile, average=False)
        scan.set_unit("Hz")
        sc_new = scan._getabcissa(0)
        del scan
        nch_new = len(sc_new)
        ch0_new = sc_new[0]
        incr_new = (sc_new[nch_new-1]-sc_new[0])/float(nch_new-1)
        
        self.assertEqual(nch_new,numpy.ceil(nch_old/self.regridw))
        self.assertAlmostEqual((ch0_new-refsc['ch0'])/refsc['ch0'],0.,places=5)
        self.assertAlmostEqual((incr_new-refsc['incr'])/refsc['incr'],0.,places=5)

    def test11(self):
        """Test11: kernel = 'regrid' + negative chanwidth in frequency unit"""
        tid="11"
        unit="Hz"
        # get channel number and width in input data
        scan = sd.scantable(self.infile, average=False)
        oldunit = scan.get_unit()
        scan.set_unit(unit)
        nch_old = scan.nchan(scan.getif(0))
        fx = scan._getabcissa(0)
        chw_old = (fx[nch_old-1]-fx[0])/float(nch_old-1)
        scan.set_unit(oldunit)
        del scan
        chw_new = chw_old*(-self.regridw)
        
        outfile = self.outroot+tid+'.asap'
        kernel = 'regrid'
        chanwidth = qa.tos(qa.quantity(chw_new,unit))

        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,chanwidth=chanwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2435 + CASA r18782)
        refdic = {'linemax': 1.9440968036651611,
                  'linemaxpos': 1285.0,
                  'lineeqw': 15.466022935853511,
                  'baserms': 0.16496795415878296}
        refsc = {"ch0": 44099917021.547066,
                 "incr": -24416.931914787496}
        nnew = int(numpy.ceil(nch_old/float(self.regridw)))
        reglinech = []
        regbasech = []
        for chans in self.reglinech:
            reglinech.append([nnew-1-chans[0],nnew-1-chans[1]])
        for chans in self.regbasech:
            regbasech.append([nnew-1-chans[0],nnew-1-chans[1]])
        testval = self._getStats(outfile,reglinech,regbasech)
        self._compareDictVal(testval, refdic)
        # spectral coordinate check
        scan = sd.scantable(outfile, average=False)
        scan.set_unit("Hz")
        sc_new = scan._getabcissa(0)
        del scan
        nch_new = len(sc_new)
        ch0_new = sc_new[0]
        incr_new = (sc_new[nch_new-1]-sc_new[0])/float(nch_new-1)
        
        self.assertEqual(nch_new,numpy.ceil(nch_old/self.regridw))
        self.assertAlmostEqual((ch0_new-refsc['ch0'])/refsc['ch0'],0.,places=5)
        self.assertAlmostEqual((incr_new-refsc['incr'])/refsc['incr'],0.,places=5)


    def test12(self):
        """Test 12: kernel = 'regrid' + positive chanwidth in velocity unit"""
        tid="12"
        unit="km/s"
        # get channel number and width in input data
        scan = sd.scantable(self.infile, average=False)
        oldunit = scan.get_unit()
        scan.set_unit(unit)
        nch_old = scan.nchan(scan.getif(0))
        fx = scan._getabcissa(0)
        chw_old = (fx[nch_old-1]-fx[0])/float(nch_old-1)
        scan.set_unit(oldunit)
        del scan
        chw_new = chw_old*(-self.regridw)

        outfile = self.outroot+tid+'.asap'
        kernel = 'regrid'
        chanwidth = qa.tos(qa.quantity(chw_new,unit))

        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,chanwidth=chanwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        ### reference values ( ASAP r2435 + CASA r18782)
        refdic = {'linemax': 1.9440964460372925,
                  'linemaxpos': 1285.0,
                  'lineeqw': 15.466027743114028,
                  'baserms': 0.16496789455413818}
        refsc = {"ch0": 44099917021.547066,
                 "incr": -24416.931942994266}
        nnew = int(numpy.ceil(nch_old/float(self.regridw)))
        reglinech = []
        regbasech = []
        for chans in self.reglinech:
            reglinech.append([nnew-1-chans[0],nnew-1-chans[1]])
        for chans in self.regbasech:
            regbasech.append([nnew-1-chans[0],nnew-1-chans[1]])
        testval = self._getStats(outfile,reglinech,regbasech)
        self._compareDictVal(testval, refdic)
        # spectral coordinate check
        scan = sd.scantable(outfile, average=False)
        scan.set_unit("Hz")
        sc_new = scan._getabcissa(0)
        del scan
        nch_new = len(sc_new)
        ch0_new = sc_new[0]
        incr_new = (sc_new[nch_new-1]-sc_new[0])/float(nch_new-1)
        
        self.assertEqual(nch_new,numpy.ceil(nch_old/self.regridw))
        self.assertAlmostEqual((ch0_new-refsc['ch0'])/refsc['ch0'],0.,places=5)
        self.assertAlmostEqual((incr_new-refsc['incr'])/refsc['incr'],0.,places=5)

    def test13(self):
        """Test 13: test flagged data kernel = 'boxcar' + kwidth = 16"""
        tid="13"
        outfile = self.outroot+tid+'.asap'
        kernel = 'boxcar'
        kwidth = 16

        # flag line channels
        from sdflagold import sdflagold
        flag_line = []
        for rg in self.linechan:
            flag_line.append([rg[0]+kwidth, rg[1]-kwidth])
        sdflagold(infile=self.infile, maskflag=flag_line)

        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,kwidth=kwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # check flag in output scantable
        scan = sd.scantable(outfile, average=False)
        for irow in range(scan.nrow()):
            mask0 = scan.get_mask(irow)
            flag0 = [(not mmm) for mmm in mask0]
            res_mask = scan.get_masklist(flag0)
            del mask0, flag0
            self.assertEqual(res_mask, flag_line,
                             "unexpected flag range (row %d)" % irow)
        del scan
        
        
        # unflag line channels
        sdflagold(infile=outfile, maskflag = flag_line,flagmode='unflag')

        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 0,
                  'baserms': 0.096009217202663422}
        line_1 = []
        for rg in flag_line:
            line_1.append([rg[0]+kwidth, rg[1]-kwidth])
        testval = self._getStats(outfile, line_1)
        self._compareDictVal(testval, refdic)


class sdsmoothold_storageTest( sdsmoothold_unittest_base, unittest.TestCase ):
    """
    Unit tests for task sdsmoothold. Test scantable sotrage and insitu
    parameters

    The list of tests:
    testMTsm  --- storage = 'memory', insitu = True (smooth)
    testMFsm  --- storage = 'memory', insitu = False (smooth)
    testDTsm  --- storage = 'disk', insitu = True (smooth)
    testDFsm  --- storage = 'disk', insitu = False (smooth)
    testMTrg  --- storage = 'memory', insitu = True (regrid)
    testMFrg  --- storage = 'memory', insitu = False (regrid)
    testDTrg  --- storage = 'disk', insitu = True (regrid)
    testDFrg  --- storage = 'disk', insitu = False (regrid)

    Note on handlings of disk storage:
       Returns new table after smooth and regrid.
       Returns new table after scantable.convert_flux.
       Task script restores unit and frame information.
    """
    # Input and output names
    infile = 'OrionS_rawACSmod_if2_calTPave_bl.asap'
    outroot = sdsmoothold_unittest_base.taskname+'_test'
    linechan = [[2951,3088]]
    basechan = [[500,2950],[3089,7692]]
    regridw = 4
    reglinech = [[737,772]]
    regbasech = [[125,736],[773,1923]]

    ### reference values ( ASAP r2435 + CASA r18782)
    smrefdic = {'linemax': 1.8286358118057251,
                'linemaxpos': 3047.0,
                'lineeqw': 65.946254391136108,
                'baserms': 0.096009217202663422}
    rgrefdic = {'linemax': 1.9440968036651611,
                'linemaxpos': 762.0,
                'lineeqw': 15.466022935853511,
                'baserms': 0.16496795415878296}
    rgrefsc = {"ch0": 44049935561.916985,
               "incr": 24416.931914787496}


    def setUp( self ):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(sdsmoothold)

    def tearDown( self ):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    def testMTsm( self ):
        """Storage Test MTsm: kernel = 'boxcar' + kwidth = 16 on storage='memory' and insitu=T"""
        tid="MTsm"
        outfile = self.outroot+tid+'.asap'
        kernel = 'boxcar'
        kwidth = 16

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))

        initval = self._getStats(self.infile)
        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,kwidth=kwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Test input data
        print "Comparing INPUT statistics before/after smoothing"
        newinval = self._getStats(self.infile)
        self._compareDictVal(newinval, initval)
        # Test output data
        print "Testing OUTPUT statistics"
        testval = self._getStats(outfile)
        self._compareDictVal(testval, self.smrefdic)

    def testMFsm( self ):
        """Storage Test MFsm: kernel = 'boxcar' + kwidth = 16 on storage='memory' and insitu=F"""
        tid="MFsm"
        outfile = self.outroot+tid+'.asap'
        kernel = 'boxcar'
        kwidth = 16

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))

        initval = self._getStats(self.infile)
        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,kwidth=kwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Test input data
        print "Comparing INPUT statistics before/after smoothing"
        newinval = self._getStats(self.infile)
        self._compareDictVal(newinval, initval)
        # Test output data
        print "Testing OUTPUT statistics"
        testval = self._getStats(outfile)
        self._compareDictVal(testval, self.smrefdic)

    def testDTsm( self ):
        """Storage Test DTsm: kernel = 'boxcar' + kwidth = 16 on storage='disk' and insitu=T"""
        tid="DTsm"
        outfile = self.outroot+tid+'.asap'
        kernel = 'boxcar'
        kwidth = 16

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))

        initval = self._getStats(self.infile)
        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,kwidth=kwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Test input data
        print "Comparing INPUT statistics before/after smoothing"
        newinval = self._getStats(self.infile)
        self._compareDictVal(newinval, initval)
        # Test output data
        print "Testing OUTPUT statistics"
        testval = self._getStats(outfile)
        self._compareDictVal(testval, self.smrefdic)

    def testDFsm( self ):
        """Storage Test DFsm: kernel = 'boxcar' + kwidth = 16 on storage='disk' and insitu=F"""
        tid="DFsm"
        outfile = self.outroot+tid+'.asap'
        kernel = 'boxcar'
        kwidth = 16

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))

        initval = self._getStats(self.infile)
        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,kwidth=kwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Test input data
        print "Comparing INPUT statistics before/after smoothing"
        newinval = self._getStats(self.infile)
        self._compareDictVal(newinval, initval)
        # Test output data
        print "Testing OUTPUT statistics"
        testval = self._getStats(outfile)
        self._compareDictVal(testval, self.smrefdic)

    def testMTrg( self ):
        """Storage Test MTrg: kernel = 'regrid' on storage='memory' and insitu=T"""
        tid="MTrg"
        unit="Hz"
        # get channel number and width in input data
        sd.rcParams['scantable.storage'] = 'memory'
        scan = sd.scantable(self.infile, average=False)
        oldunit = scan.get_unit()
        scan.set_unit(unit)
        nch_old = scan.nchan(scan.getif(0))
        fx = scan._getabcissa(0)
        chw_old = (fx[nch_old-1]-fx[0])/float(nch_old-1)
        scan.set_unit(oldunit)
        del scan
        chw_new = chw_old*self.regridw
        
        outfile = self.outroot+tid+'.asap'
        kernel = 'regrid'
        chanwidth = qa.tos(qa.quantity(chw_new,unit))

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))

        initval = self._getStats(self.infile,self.linechan,self.basechan)
        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,chanwidth=chanwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Test input data
        print "Comparing INPUT statistics before/after smoothing"
        newinval = self._getStats(self.infile,self.linechan,self.basechan)
        self._compareDictVal(newinval, initval)
        # Test output data
        print "Testing OUTPUT statistics"
        testval = self._getStats(outfile,self.reglinech,self.regbasech)
        self._compareDictVal(testval, self.rgrefdic)
        # spectral coordinate check
        sd.rcParams['scantable.storage'] = 'memory'
        scan = sd.scantable(outfile, average=False)
        scan.set_unit("Hz")
        sc_new = scan._getabcissa(0)
        del scan
        nch_new = len(sc_new)
        ch0_new = sc_new[0]
        incr_new = (sc_new[nch_new-1]-sc_new[0])/float(nch_new-1)
        
        self.assertEqual(nch_new,numpy.ceil(nch_old/self.regridw))
        self.assertAlmostEqual((ch0_new-self.rgrefsc['ch0'])/self.rgrefsc['ch0'],0.,places=5)
        self.assertAlmostEqual((incr_new-self.rgrefsc['incr'])/self.rgrefsc['incr'],0.,places=5)

    def testMFrg( self ):
        """Storage Test MFrg: kernel = 'regrid' on storage='memory' and insitu=F"""
        tid="MFrg"
        unit="Hz"
        # get channel number and width in input data
        sd.rcParams['scantable.storage'] = 'memory'
        scan = sd.scantable(self.infile, average=False)
        oldunit = scan.get_unit()
        scan.set_unit(unit)
        nch_old = scan.nchan(scan.getif(0))
        fx = scan._getabcissa(0)
        chw_old = (fx[nch_old-1]-fx[0])/float(nch_old-1)
        scan.set_unit(oldunit)
        del scan
        chw_new = chw_old*self.regridw
        
        outfile = self.outroot+tid+'.asap'
        kernel = 'regrid'
        chanwidth = qa.tos(qa.quantity(chw_new,unit))

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))

        initval = self._getStats(self.infile,self.linechan,self.basechan)
        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,chanwidth=chanwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Test input data
        print "Comparing INPUT statistics before/after smoothing"
        newinval = self._getStats(self.infile,self.linechan,self.basechan)
        self._compareDictVal(newinval, initval)
        # Test output data
        print "Testing OUTPUT statistics"
        testval = self._getStats(outfile,self.reglinech,self.regbasech)
        self._compareDictVal(testval, self.rgrefdic)
        # spectral coordinate check
        sd.rcParams['scantable.storage'] = 'memory'
        scan = sd.scantable(outfile, average=False)
        scan.set_unit("Hz")
        sc_new = scan._getabcissa(0)
        del scan
        nch_new = len(sc_new)
        ch0_new = sc_new[0]
        incr_new = (sc_new[nch_new-1]-sc_new[0])/float(nch_new-1)
        
        self.assertEqual(nch_new,numpy.ceil(nch_old/self.regridw))
        self.assertAlmostEqual((ch0_new-self.rgrefsc['ch0'])/self.rgrefsc['ch0'],0.,places=5)
        self.assertAlmostEqual((incr_new-self.rgrefsc['incr'])/self.rgrefsc['incr'],0.,places=5)

    def testDTrg( self ):
        """Storage Test DTrg: kernel = 'regrid' on storage='disk' and insitu=T"""
        tid="DTrg"
        unit="Hz"
        # get channel number and width in input data
        sd.rcParams['scantable.storage'] = 'memory'
        scan = sd.scantable(self.infile, average=False)
        oldunit = scan.get_unit()
        scan.set_unit(unit)
        nch_old = scan.nchan(scan.getif(0))
        fx = scan._getabcissa(0)
        chw_old = (fx[nch_old-1]-fx[0])/float(nch_old-1)
        scan.set_unit(oldunit)
        del scan
        chw_new = chw_old*self.regridw
        
        outfile = self.outroot+tid+'.asap'
        kernel = 'regrid'
        chanwidth = qa.tos(qa.quantity(chw_new,unit))

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))

        initval = self._getStats(self.infile,self.linechan,self.basechan)
        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,chanwidth=chanwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Test input data
        print "Comparing INPUT statistics before/after smoothing"
        newinval = self._getStats(self.infile,self.linechan,self.basechan)
        self._compareDictVal(newinval, initval)
        # Test output data
        print "Testing OUTPUT statistics"
        testval = self._getStats(outfile,self.reglinech,self.regbasech)
        self._compareDictVal(testval, self.rgrefdic)
        # spectral coordinate check
        sd.rcParams['scantable.storage'] = 'memory'
        scan = sd.scantable(outfile, average=False)
        scan.set_unit("Hz")
        sc_new = scan._getabcissa(0)
        del scan
        nch_new = len(sc_new)
        ch0_new = sc_new[0]
        incr_new = (sc_new[nch_new-1]-sc_new[0])/float(nch_new-1)
        
        self.assertEqual(nch_new,numpy.ceil(nch_old/self.regridw))
        self.assertAlmostEqual((ch0_new-self.rgrefsc['ch0'])/self.rgrefsc['ch0'],0.,places=5)
        self.assertAlmostEqual((incr_new-self.rgrefsc['incr'])/self.rgrefsc['incr'],0.,places=5)

    def testDFrg( self ):
        """Storage Test DFrg: kernel = 'regrid' on storage='disk' and insitu=F"""
        tid="DFrg"
        unit="Hz"
        # get channel number and width in input data
        sd.rcParams['scantable.storage'] = 'memory'
        scan = sd.scantable(self.infile, average=False)
        oldunit = scan.get_unit()
        scan.set_unit(unit)
        nch_old = scan.nchan(scan.getif(0))
        fx = scan._getabcissa(0)
        chw_old = (fx[nch_old-1]-fx[0])/float(nch_old-1)
        scan.set_unit(oldunit)
        del scan
        chw_new = chw_old*self.regridw
        
        outfile = self.outroot+tid+'.asap'
        kernel = 'regrid'
        chanwidth = qa.tos(qa.quantity(chw_new,unit))

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))

        initval = self._getStats(self.infile,self.linechan,self.basechan)
        result =sdsmoothold(infile=self.infile,outfile=outfile,
                         kernel=kernel,chanwidth=chanwidth)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Test input data
        print "Comparing INPUT statistics before/after smoothing"
        newinval = self._getStats(self.infile,self.linechan,self.basechan)
        self._compareDictVal(newinval, initval)
        # Test output data
        print "Testing OUTPUT statistics"
        testval = self._getStats(outfile,self.reglinech,self.regbasech)
        self._compareDictVal(testval, self.rgrefdic)
        # spectral coordinate check
        sd.rcParams['scantable.storage'] = 'memory'
        scan = sd.scantable(outfile, average=False)
        scan.set_unit("Hz")
        sc_new = scan._getabcissa(0)
        del scan
        nch_new = len(sc_new)
        ch0_new = sc_new[0]
        incr_new = (sc_new[nch_new-1]-sc_new[0])/float(nch_new-1)
        
        self.assertEqual(nch_new,numpy.ceil(nch_old/self.regridw))
        self.assertAlmostEqual((ch0_new-self.rgrefsc['ch0'])/self.rgrefsc['ch0'],0.,places=5)
        self.assertAlmostEqual((incr_new-self.rgrefsc['incr'])/self.rgrefsc['incr'],0.,places=5)


def suite():
    return [sdsmoothold_badinputs, sdsmoothold_basicTest, sdsmoothold_storageTest]
