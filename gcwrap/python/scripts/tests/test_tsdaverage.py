import os
import sys
import shutil
import re
from __main__ import default
from tasks import *
from taskinit import *
import unittest
#
#import listing
import numpy

import asap as sd
from tsdaverage import tsdaverage
from sdstat import sdstat

#
# Unit test of sdaverage task.
#

class sdaverage_unittest_base:
    """
    Base class for sdaverage unit test
    """
    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdsmooth/'
    taskname = "sdaverage"

    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertTrue(isthere,
                         msg='output file %s was not created because of the task failure'%(name))


class sdaverage_smoothtest_base(sdaverage_unittest_base):
    """
    Base class for smoothing test
    """
    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdsmooth/'
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


###
# Base class for averaging test
###
class sdaverage_avetest_base(sdaverage_unittest_base):
    """
    Base class for averaging test
    """
    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdcal/'

    postfix='ave.asap'

    def _getspectra( self, name ):
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='file %s does not exist'%(name))        
        tb.open(name)
        sp=tb.getcol('SPECTRA').transpose()
        tb.close()
        return sp

    def _checkshape( self, sp, ref ):
        # check array dimension 
        self.assertEqual( sp.ndim, ref.ndim,
                          msg='array dimension differ' )
        # check number of spectra
        self.assertEqual( sp.shape[0], ref.shape[0],
                          msg='number of spectra differ' )
        # check number of channel
        self.assertEqual( sp.shape[1], ref.shape[1],
                          msg='number of channel differ' )

    def _diff(self, sp, ref):
        diff=abs((sp-ref)/ref)
        idx=numpy.argwhere(numpy.isnan(diff))
        #print idx
        if len(idx) > 0:
            diff[idx]=sp[idx]
        return diff
        
    def _split( self, s ):
        pos0=s.find('[')+1
        pos1=s.find(']')
        substr=s[pos0:pos1]
        return substr.split(',')

    def _getrefdata( self, name ):
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='file %s does not exist'%(name))
        f=open(name)
        lines=f.readlines()
        f.close
        # scanno
        scans=[]
        if re.match('^SCANNO:',lines[0]) is not None:
            ss=self._split(lines[0])
            for s in ss:
                scans.append(string.atoi(s))

        # polno
        pols=[]
        if re.match('^POLNO:',lines[1]) is not None:
            ss=self._split(lines[1])
            for s in ss:
                pols.append(string.atoi(s))

        # spectral data
        sp=[]
        for line in lines[2:]:
            tmp=[]
            ss=self._split(line)
            for s in ss:
                tmp.append(string.atof(s))
            sp.append(tmp)

        # python list -> numpy array
        scans=numpy.unique(scans)
        pols=numpy.unique(pols)
        sp=numpy.array(sp)

        return (scans, pols, sp)
    

    def _compare( self, name1, name2 ):
        self._checkfile(name1)
        sp=self._getspectra( name1 )
        (scan0,pol0,sp0)=self._getrefdata( name2 )

        self._checkshape( sp, sp0 )

        s=sd.scantable(name1,False)
        scan=numpy.array(s.getscannos())
        pol=numpy.array(s.getpolnos())
        #print scan0, scan
        #print pol0, pol
        retval=len(pol)==len(pol0) and all(pol==pol0)
        self.assertEqual( retval, True,
                          msg='POLNO is wrong' )
        retval=len(scan)==len(scan0) and all(scan==scan0)
        self.assertEqual( retval, True,
                          msg='SCANNO is wrong' )
        del s
        
        for irow in xrange(sp.shape[0]):
            diff=self._diff(sp[irow],sp0[irow])
            retval=numpy.all(diff<0.01)
            self.assertEqual( retval, True,
                              msg='averaged result is wrong (irow=%s): maxdiff=%s'%(irow,diff.max()) )
        del sp, sp0



class sdaverage_badinputs(sdaverage_unittest_base,unittest.TestCase):
    """
    Tests on bad input parameter setting
    """
    inname = 'OrionS_rawACSmod_if2_calTPave_bl.asap'
    outname = sdaverage_unittest_base.taskname+'_bad.asap'

    badname = "bad"
    badquant = "5bad"
    
    def setUp(self):
        if os.path.exists(self.inname):
            shutil.rmtree(self.inname)
        if os.path.exists(self.outname):
            shutil.rmtree(self.outname)
        shutil.copytree(self.datapath+self.inname, self.inname)

        default(tsdaverage)

    def tearDown(self):
        if (os.path.exists(self.inname)):
            shutil.rmtree(self.inname)

    # Tests on invalid parameter sets
    def test_default(self):
        """Test Default parameters (raises an errror)"""
        # argument verification error
        try:
            res = tsdaverage()
            self.fail("The task must throw exception.")
        except Exception, e:
            pos=str(e).find("Neither averaging nor smoothing parameter is set.")

    def testBad_kernel(self):
        """Test bad kernel name"""
        kernel = self.badname

        # argument verification error
        res = tsdaverage(infile=self.inname,outfile=self.outname,
                       kernel=kernel)
        self.assertFalse(res)

    def testBad_chanwidth(self):
        """Test bad chanwidth"""
        kernel = 'regrid'
        chanwidth = self.badquant

        try:
            res = tsdaverage(infile=self.inname,outfile=self.outname,
                           kernel=kernel,chanwidth=chanwidth)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Invalid quantity chanwidth 5bad')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
    def testBad_overwrite(self):
        """Specify existing output file name with overwrite=False"""
        outfile=self.inname
        if (not os.path.exists(outfile)):
            shutil.copytree(self.datapath+outfile, outfile)
        try:
            self.res=tsdaverage(infile=outfile,timeaverage=True,
                                outfile=outfile,overwrite=False)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Output file \'%s\' exists.'%(outfile))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        finally:
            os.system( 'rm -rf %s'%outfile )        


class sdaverage_smoothTest(sdaverage_smoothtest_base,unittest.TestCase):
    """
    Basic unit tests of smoothing by task sdaverage. No interactive testing.

    The list of tests:
    test00    --- default parameters (raises an errror)
    test01    --- kernel = 'hanning'
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
      sdstat(infile=outfile,masklist=linechan)
      sdstat(infile=outfile,masklist=basechan)
    """
    ### TODO:
    ### - comparison with simple and known spectral shape

    # Input and output names
    infile = 'OrionS_rawACSmod_if2_calTPave_bl.asap'
    outroot = sdaverage_unittest_base.taskname+'_test'
    linechan = [[2951,3088]]
    basechan = [[500,2950],[3089,7692]]
    regridw = 4
    reglinech = [[737,772]]
    regbasech = [[125,736],[773,1923]]

    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(tsdaverage)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    def test01(self):
        """Test 1: kernel = 'hanning'"""
        tid="01"
        outfile = self.outroot+tid+'.asap'

        result = tsdaverage(infile=self.infile,kernel='hanning',outfile=outfile)
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

        result =tsdaverage(infile=self.infile,outfile=outfile,kernel=kernel)
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

        result =tsdaverage(infile=self.infile,outfile=outfile,
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

        result =tsdaverage(infile=self.infile,outfile=outfile,kernel=kernel)
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

        result =tsdaverage(infile=self.infile,outfile=outfile,
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
        print "The 1st run of sdaverage (hanning smooth)"
        result1 =tsdaverage(infile=self.infile,kernel='hanning',outfile=outfile)
        self.assertEqual(result1,None,
                         msg="The task returned '"+str(result1)+"' instead of None for the 1st run")
        self.assertTrue(os.path.exists(outfile),
                        msg="Output file doesn't exist after the 1st run.")

        # overwrite 'outfile'
        # the second run with kernel = 'gaussian'
        kernel = 'gaussian'
        print "The 2nd run of sdaverage (OVERWRITE with Gaussian smooth)"
        result2 =tsdaverage(infile=self.infile,outfile=outfile,
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

        result =tsdaverage(infile=self.infile,outfile=outfile,
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

        result =tsdaverage(infile=self.infile,outfile=outfile,
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

        result =tsdaverage(infile=self.infile,outfile=outfile,
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

        result =tsdaverage(infile=self.infile,outfile=outfile,
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

        result =tsdaverage(infile=self.infile,outfile=outfile,
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

        result =tsdaverage(infile=self.infile,outfile=outfile,
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
        from sdflag import sdflag
        flag_line = []
        for rg in self.linechan:
            flag_line.append([rg[0]+kwidth, rg[1]-kwidth])
        sdflag(infile=self.infile, maskflag=flag_line)

        result =tsdaverage(infile=self.infile,outfile=outfile,
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
        sdflag(infile=outfile, maskflag = flag_line,flagmode='unflag')

        ### reference values ( ASAP r2084 + CASA r14498)
        refdic = {'linemax': 0,
                  'baserms': 0.096009217202663422}
        line_1 = []
        for rg in flag_line:
            line_1.append([rg[0]+kwidth, rg[1]-kwidth])
        testval = self._getStats(outfile, line_1)
        self._compareDictVal(testval, refdic)


class sdaverage_storageTest( sdaverage_smoothtest_base, unittest.TestCase ):
    """
    Unit tests for task sdaverage. Test scantable sotrage and insitu
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
    outroot = sdaverage_unittest_base.taskname+'_test'
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

        default(tsdaverage)

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
        result =tsdaverage(infile=self.infile,outfile=outfile,
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
        result =tsdaverage(infile=self.infile,outfile=outfile,
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
        result =tsdaverage(infile=self.infile,outfile=outfile,
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
        result =tsdaverage(infile=self.infile,outfile=outfile,
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
        result =tsdaverage(infile=self.infile,outfile=outfile,
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
        result =tsdaverage(infile=self.infile,outfile=outfile,
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
        result =tsdaverage(infile=self.infile,outfile=outfile,
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
        result =tsdaverage(infile=self.infile,outfile=outfile,
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


###
# Test polarization averaging with/without scan average
###
class sdaverage_test6(sdaverage_avetest_base,unittest.TestCase):
    """
    Test polarization averaging with/without scan average

    Data is calpsGBT.cal.asap.

    ###NOTE###
    Polarization average averages data in both time and polarization.
    """
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdaverage/'

    # Input and output names
    rawfile='calpsGBT.cal.asap'
    prefix=sdaverage_unittest_base.taskname+'Test6'
    reffiles=['polaverage.ref0',
              'polaverage.ref1',
              'polaverage.ref2',
              'polaverage.ref3']
    
    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        for reffile in self.reffiles:
            if (not os.path.exists(reffile)):
                shutil.copyfile(self.datapath+reffile, reffile)

        default(tsdaverage)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        for reffile in self.reffiles:
            if (os.path.exists(reffile)):
                os.remove(reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test600(self):
        """Test 600: test polarization average with pweight='var' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.rawfile,timeaverage=True,scanaverage=True,tweight='var',polaverage=True,pweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during polarization averaging')
        self._compare(outname,self.reffiles[0])
        
    def test601(self):
        """Test 601: test polarization average with pweight='var' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.rawfile,timeaverage=False,polaverage=True,pweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during polarization averaging')
        self._compare(outname,self.reffiles[1])

    def test602(self):
        """Test 602: test polarization average with pweight='tsys' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.rawfile,timeaverage=True,scanaverage=True,tweight='tsys',polaverage=True,pweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during polarization averaging')
        self._compare(outname,self.reffiles[2])
        
    def test603(self):
        """Test 603: test polarization average with pweight='tsys' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.rawfile,timeaverage=False,polaverage=True,pweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during polarization averaging')
        self._compare(outname,self.reffiles[3])


###
# Test time averaging with/without scan average
###
class sdaverage_test7(sdaverage_avetest_base,unittest.TestCase):
    """
    Test time averaging with/without scan average

    Data is calpsGBT.cal.asap.
    """
    # Input and output names
    rawfile='calpsGBT.cal.asap'
    prefix=sdaverage_unittest_base.taskname+'Test7'
    reffiles=['timeaverage.ref0',
              'timeaverage.ref1',
              'timeaverage.ref2',
              'timeaverage.ref3',
              'timeaverage.ref4',
              'timeaverage.ref5',
              'timeaverage.ref6',
              'timeaverage.ref7',
              'timeaverage.ref8',
              'timeaverage.ref9']

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        for reffile in self.reffiles:
            if (not os.path.exists(reffile)):
                shutil.copyfile(self.datapath+reffile, reffile)

        default(tsdaverage)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        for reffile in self.reffiles:
            if (os.path.exists(reffile)):
                os.remove(reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test700(self):
        """Test 700: test time average with tweight='var' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.rawfile,timeaverage=True,scanaverage=True,tweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[0])
        
    def test701(self):
        """Test 701: test time average with tweight='var' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.rawfile,timeaverage=True,scanaverage=False,tweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[1])
        
    def test702(self):
        """Test 702: test time average with tweight='tsys' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.rawfile,timeaverage=True,scanaverage=True,tweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[2])
        
    def test703(self):
        """Test 703: test time average with tweight='tsys' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.rawfile,timeaverage=True,scanaverage=False,tweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[3])
        
    def test704(self):
        """Test 704: test time average with tweight='tint' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.rawfile,timeaverage=True,scanaverage=True,tweight='tint',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[4])
        
    def test705(self):
        """Test 705: test time average with tweight='tint' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.rawfile,timeaverage=True,scanaverage=False,tweight='tint',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[5])
        
    def test706(self):
        """Test 706: test time average with tweight='tintsys' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.rawfile,timeaverage=True,scanaverage=True,tweight='tintsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[6])
        
    def test707(self):
        """Test 707: test time average with tweight='tintsys' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.rawfile,timeaverage=True,scanaverage=False,tweight='tintsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[7])
        
    def test708(self):
        """Test 708: test time average with tweight='median' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.rawfile,timeaverage=True,scanaverage=True,tweight='median',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[8])
        
    def test709(self):
        """Test 709: test time average with tweight='median' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.rawfile,timeaverage=True,scanaverage=False,tweight='median',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[9])
        

###
# Test mixed operation (cal+average,time+pol average,...)
###
class sdaverage_test8(sdaverage_avetest_base,unittest.TestCase):
    """
    Test other things including mixed opearation

       - calibration + time average
       - calibration + polarization average
       - time average + polarzation average
       - channelrange parameter
       
    """
    # Input and output names
    rawfile='calpsGBT.asap'
    calfile='calpsGBT.cal.asap'
    prefix=sdaverage_unittest_base.taskname+'Test8'
    reffiles=['polaverage.ref2',
              'timeaverage.ref6',
              'timepolaverage.ref']

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.calfile)):
            shutil.copytree(self.datapath+self.calfile, self.calfile)
        for reffile in self.reffiles:
            if (not os.path.exists(reffile)):
                shutil.copyfile(self.datapath+reffile, reffile)

        default(tsdaverage)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.calfile)):
            shutil.rmtree(self.calfile)
        for reffile in self.reffiles:
            if (os.path.exists(reffile)):
                os.remove(reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test801(self):
        """Test 801: test polarization average with pweight='tsys' + time average with tweight='tintsys' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.calfile,timeaverage=True,scanaverage=False,tweight='tintsys',polaverage=True,pweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during averaging')
        self._compare(outname,self.reffiles[2])
        
###
# Test averageall parameter
###
class sdaverage_test9(sdaverage_avetest_base,unittest.TestCase):
    """
    Test averageall parameter that forces to average spectra with
    different spectral resolution.

    Here, test to average the following two spectra that have same
    frequency in band center.

       - nchan = 8192, resolution = 6104.23 Hz
       - nchan = 8192, resolution = 12208.5 Hz
       
    """
    # Input and output names
    rawfile='averageall.asap'
    prefix=sdaverage_unittest_base.taskname+'Test9'
    reffiles=['averageall.ref']

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        for reffile in self.reffiles:
            if (not os.path.exists(reffile)):
                shutil.copyfile(self.datapath+reffile, reffile)

        default(tsdaverage)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        for reffile in self.reffiles:
            if (os.path.exists(reffile)):
                os.remove(reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test900(self):
        """Test 900: test averageall parameter"""
        outname=self.prefix+self.postfix
        self.res=tsdaverage(infile=self.rawfile,timeaverage=True,scanaverage=False,tweight='tintsys',averageall=True,polaverage=False,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during averaging')
        self._compare(outname,self.reffiles[0])
        self._checkFrequencies(outname)

    def _checkFrequencies(self,outfile):
        tb.open(self.rawfile.rstrip('/')+'/FREQUENCIES')
        rpin=tb.getcol('REFPIX')
        rvin=tb.getcol('REFVAL')
        icin=tb.getcol('INCREMENT')
        tb.close
        tb.open(outfile.rstrip('/')+'/FREQUENCIES')
        rpout=tb.getcol('REFPIX')
        rvout=tb.getcol('REFVAL')
        icout=tb.getcol('INCREMENT')
        tb.close
        ic=[icin[2:4].max(),icin[0:2].max()]
        ledge=[rvin[2]-ic[0]*(rpin[2]+0.5),
               rvin[0]-ic[1]*(rpin[0]+0.5)]
        redge=[rvin[2]+ic[0]*(8191-rpin[2]+0.5),
               rvin[0]+ic[1]*(8191-rpin[0]+0.5)]
        eps = 1.0e-15
        for i in xrange(2):
            self.assertEqual(ic[i],icout[i],
                             msg='INCREMENT for FREQ_ID=%s differ'%(i))
            fmin=rvout[i]-icout[i]*(rpout[i]+0.5)
            self.assertTrue(abs((fmin-ledge[i])/ledge[i]) < eps,
                             msg='Left frequency edge for FREQ_ID=%s does not match'%(i))
            fmax=rvout[i]+icout[i]*(8191-rpout[i]+0.5)
            self.assertTrue(abs((fmax-redge[i])/redge[i]) < eps,
                             msg='Right frequency edge for FREQ_ID=%s does not match'%(i))

###
# Averaging of flagged data
###
class sdaverage_test_flag(sdaverage_avetest_base,unittest.TestCase):
    """
    Test Averaging of flagged data.

    Data varies with tests
    """
    # Input and output names
    prefix=sdaverage_unittest_base.taskname+'Test7'
    filelist = []

    def setUp(self):
        self.res=None
        self.filelist = []
        default(tsdaverage)

    def tearDown(self):
        for name in self.filelist:
            self._remove(name)

    def _remove(self, name):
        # remove file/directory if exists
        if os.path.isdir(name):
            shutil.rmtree(name)
        elif os.path.exists(name):
            os.remove(name)

    def _mark_to_delete(self, name):
        self.filelist.append(name)

    def _copy(self, source, dest):
        # copy source directory to dest
        self.assertTrue(os.path.exists(source),
                        msg="Could not find a source file '%s'" % source)
        if os.path.exists(dest):
            shutil.rmtree(dest)
        shutil.copytree(source, dest)

    def _copyInputs(self, inputs):
        if type(inputs)==str:
            inputs = [inputs]

        for inname in inputs:
            inname = inname.rstrip('/')
            outname = os.path.basename(inname)
            self._copy(inname, outname)
            self._mark_to_delete(outname)


    def _comp_stat(self, data, refstat):
        if type(data)==str:
            data = sdstat(infile=data)
        elif type(data)!=dict:
            self.fail(msg="Internal Error: invalid data given to calculate statistics.")
        for key, refvals in refstat.items():
            refvals = self._to_list(refvals)
            testvals = self._to_list(data[key])
            for idx in range(len(refvals)):
                rdiff = (testvals[idx] - refvals[idx])/refvals[idx]
                self.assertAlmostEqual(rdiff, 0., 4, msg="Verification of '%s' (idx=%d) failed: %f (expected: %f)" % (key, idx, testvals[idx], refvals[idx]))

    def _to_list(self, indata):
        try: len(indata)
        except TypeError: return [indata]
        return list(indata)


    def testFlag01(self):
        """TestFlag01: test time average of row flagged data set"""
        outname=self.prefix+self.postfix
        self._mark_to_delete(outname)
        # Prepare input
        inname = 'flatspec_rowflag.asap'
        self.rawfile = self.datapath+inname
        self._copyInputs(self.rawfile)
        self._remove(outname)

        # run task
        self.res=tsdaverage(infile=inname,timeaverage=True,scanaverage=False,
                       tweight='tint',outfile=outname,outform='ASAP')
        # verification
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        refstat = {'max': 1.5, 'min': 1.5, 'sum': 12288.0}
        self._comp_stat(outname, refstat)

    def testFlag02(self):
        """TestFlag02: test time average of channel flagged data set"""
        outname=self.prefix+self.postfix
        self._mark_to_delete(outname)
        # Prepare input
        inname = 'flatspec_chanflag0.asap'
        self.rawfile = self.datapath+inname
        self._copyInputs(self.rawfile)
        self._remove(outname)

        # run task
        self.res=tsdaverage(infile=inname,timeaverage=True,scanaverage=False,
                       tweight='tint',outfile=outname,outform='ASAP')
        # verification
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        refedge = {'max': 1.5, 'min': 1.5, 'sum': 9286.5}
        refinner = {'max': 2.0, 'min': 2.0, 'sum': 4002.0}
        outstat = sdstat(outname, masklist=[3000, 5000])
        self._comp_stat(outstat, refinner)
        outstat = sdstat(outname, masklist=[[0, 2999], [5001, 8192]])        
        self._comp_stat(outstat, refedge)

    def testFlag03(self):
        """TestFlag03: test polarization average of row flagged data set"""
        outname=self.prefix+self.postfix
        self._mark_to_delete(outname)
        # Prepare input
        inname = 'flatspec_2pol_rowflag.asap'
        self.rawfile = self.datapath+inname
        self._copyInputs(self.rawfile)
        self._remove(outname)

        # run task
        self.res=tsdaverage(infile=inname,timeaverage=False,scanaverage=False,
                       polaverage=True,pweight='tsys',
                       outfile=outname,outform='ASAP')
        # verification
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        refstat = {'max': 1.5, 'min': 1.5, 'sum': 12288.0}
        self._comp_stat(outname, refstat)

    def testFlag04(self):
        """TestFlag04: test polarization average of channel flagged data set"""
        outname=self.prefix+self.postfix
        self._mark_to_delete(outname)
        # Prepare input
        inname = 'flatspec_2pol_chanflag0.asap'
        self.rawfile = self.datapath+inname
        self._copyInputs(self.rawfile)
        self._remove(outname)

        # run task
        self.res=tsdaverage(infile=inname,timeaverage=False,scanaverage=False,
                       polaverage=True,pweight='tsys',
                       outfile=outname,outform='ASAP')
        # verification
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        refedge = {'max': 1.5, 'min': 1.5, 'sum': 9286.5}
        refinner = {'max': 2.0, 'min': 2.0, 'sum': 4002.0}
        outstat = sdstat(outname, masklist=[3000, 5000])
        self._comp_stat(outstat, refinner)
        outstat = sdstat(outname, masklist=[[0, 2999], [5001, 8192]])        
        self._comp_stat(outstat, refedge)


def suite():
    return [sdaverage_badinputs, sdaverage_smoothTest, sdaverage_storageTest,
            sdaverage_test6, sdaverage_test7, sdaverage_test8, sdaverage_test9,
            sdaverage_test_flag]
