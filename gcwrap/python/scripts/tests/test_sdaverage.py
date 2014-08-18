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

try:
    import selection_syntax
except:
    import tests.selection_syntax as selection_syntax

import asap as sd
from sdaverage import sdaverage
from sdstatold import sdstatold

from sdutil import tbmanager

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
    sddatapath = os.environ.get('CASAPATH').split()[0] + \
                 '/data/regression/unittest/singledish/'
    taskname = "sdaverage"

    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertTrue(isthere,
                         msg='output file %s was not created because of the task failure'%(name))

    def _get_array_relative_diff(self, data, ref, precision=1.e-6):
        """
        Return an array of relative difference of elements in two arrays
        """
        precision = abs(precision)
        data_arr = numpy.array(data)
        ref_arr = numpy.array(ref)
        ref_denomi = numpy.array(ref)
        # a threshold to assume the value to be zero.
        almostzero = min(precision, max(abs(ref_arr))*precision)
        # set rdiff=0 for elements both data and reference are close to zero
        idx_ref = numpy.where(abs(ref_arr) < almostzero)
        idx_data = numpy.where(abs(data_arr) < almostzero)
        if len(idx_ref[0])>0 and len(idx_data[0])>0:
            idx = numpy.intersect1d(idx_data[0], idx_ref[0], assume_unique=True)
            ref_arr[idx] = almostzero
            data_arr[idx] = almostzero
        # prevent zero division
        ref_denomi[idx_ref] = almostzero
        return (data_arr-ref_arr)/ref_denomi

class sdaverage_smoothtest_base(sdaverage_unittest_base):
    """
    Base class for smoothing test
    """
    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdsmooth/'
    postfix='.sm.asap'
    
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

        default(sdaverage)

    def tearDown(self):
        if (os.path.exists(self.inname)):
            shutil.rmtree(self.inname)

    # Tests on invalid parameter sets
    def test_default(self):
        """Test Default parameters (raises an errror)"""
        # argument verification error
        try:
            res = sdaverage()
            self.fail("The task must throw exception.")
        except Exception, e:
            pos=str(e).find("Neither averaging nor smoothing parameter is set.")

    def testBad_kernel(self):
        """Test bad kernel name"""
        kernel = self.badname

        # argument verification error
        res = sdaverage(infile=self.inname,outfile=self.outname,
                       kernel=kernel)
        self.assertFalse(res)

    def testBad_chanwidth(self):
        """Test bad chanwidth"""
        kernel = 'regrid'
        chanwidth = self.badquant

        try:
            res = sdaverage(infile=self.inname,outfile=self.outname,
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
            self.res=sdaverage(infile=outfile,timeaverage=True,
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
      sdstatold(infile=outfile,masklist=linechan)
      sdstatold(infile=outfile,masklist=basechan)
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

        default(sdaverage)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    def test01(self):
        """Test 1: kernel = 'hanning'"""
        tid="01"
        outfile = self.outroot+tid+'.asap'

        result = sdaverage(infile=self.infile,kernel='hanning',outfile=outfile)
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

        result =sdaverage(infile=self.infile,outfile=outfile,kernel=kernel)
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

        result =sdaverage(infile=self.infile,outfile=outfile,
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

        result =sdaverage(infile=self.infile,outfile=outfile,kernel=kernel)
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

        result =sdaverage(infile=self.infile,outfile=outfile,
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
        result1 =sdaverage(infile=self.infile,kernel='hanning',outfile=outfile)
        self.assertEqual(result1,None,
                         msg="The task returned '"+str(result1)+"' instead of None for the 1st run")
        self.assertTrue(os.path.exists(outfile),
                        msg="Output file doesn't exist after the 1st run.")

        # overwrite 'outfile'
        # the second run with kernel = 'gaussian'
        kernel = 'gaussian'
        print "The 2nd run of sdaverage (OVERWRITE with Gaussian smooth)"
        result2 =sdaverage(infile=self.infile,outfile=outfile,
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

        result =sdaverage(infile=self.infile,outfile=outfile,
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

        result =sdaverage(infile=self.infile,outfile=outfile,
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

        result =sdaverage(infile=self.infile,outfile=outfile,
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

        result =sdaverage(infile=self.infile,outfile=outfile,
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

        result =sdaverage(infile=self.infile,outfile=outfile,
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

        result =sdaverage(infile=self.infile,outfile=outfile,
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

        result =sdaverage(infile=self.infile,outfile=outfile,
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
        sdflagold(infile=outfile, maskflag=flag_line, flagmode='unflag')

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

        default(sdaverage)

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
        result =sdaverage(infile=self.infile,outfile=outfile,
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
        result =sdaverage(infile=self.infile,outfile=outfile,
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
        result =sdaverage(infile=self.infile,outfile=outfile,
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
        result =sdaverage(infile=self.infile,outfile=outfile,
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
        result =sdaverage(infile=self.infile,outfile=outfile,
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
        result =sdaverage(infile=self.infile,outfile=outfile,
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
        result =sdaverage(infile=self.infile,outfile=outfile,
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
        result =sdaverage(infile=self.infile,outfile=outfile,
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

        default(sdaverage)

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
        self.res=sdaverage(infile=self.rawfile,timeaverage=True,scanaverage=True,tweight='var',polaverage=True,pweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during polarization averaging')
        self._compare(outname,self.reffiles[0])
        
    def test601(self):
        """Test 601: test polarization average with pweight='var' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdaverage(infile=self.rawfile,timeaverage=False,polaverage=True,pweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during polarization averaging')
        self._compare(outname,self.reffiles[1])

    def test602(self):
        """Test 602: test polarization average with pweight='tsys' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdaverage(infile=self.rawfile,timeaverage=True,scanaverage=True,tweight='tsys',polaverage=True,pweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during polarization averaging')
        self._compare(outname,self.reffiles[2])
        
    def test603(self):
        """Test 603: test polarization average with pweight='tsys' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdaverage(infile=self.rawfile,timeaverage=False,polaverage=True,pweight='tsys',outfile=outname,outform='ASAP')
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

        default(sdaverage)

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
        self.res=sdaverage(infile=self.rawfile,timeaverage=True,scanaverage=True,tweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[0])
        
    def test701(self):
        """Test 701: test time average with tweight='var' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdaverage(infile=self.rawfile,timeaverage=True,scanaverage=False,tweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[1])
        
    def test702(self):
        """Test 702: test time average with tweight='tsys' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdaverage(infile=self.rawfile,timeaverage=True,scanaverage=True,tweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[2])
        
    def test703(self):
        """Test 703: test time average with tweight='tsys' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdaverage(infile=self.rawfile,timeaverage=True,scanaverage=False,tweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[3])
        
    def test704(self):
        """Test 704: test time average with tweight='tint' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdaverage(infile=self.rawfile,timeaverage=True,scanaverage=True,tweight='tint',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[4])
        
    def test705(self):
        """Test 705: test time average with tweight='tint' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdaverage(infile=self.rawfile,timeaverage=True,scanaverage=False,tweight='tint',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[5])
        
    def test706(self):
        """Test 706: test time average with tweight='tintsys' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdaverage(infile=self.rawfile,timeaverage=True,scanaverage=True,tweight='tintsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[6])
        
    def test707(self):
        """Test 707: test time average with tweight='tintsys' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdaverage(infile=self.rawfile,timeaverage=True,scanaverage=False,tweight='tintsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[7])
        
    def test708(self):
        """Test 708: test time average with tweight='median' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdaverage(infile=self.rawfile,timeaverage=True,scanaverage=True,tweight='median',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[8])
        
    def test709(self):
        """Test 709: test time average with tweight='median' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdaverage(infile=self.rawfile,timeaverage=True,scanaverage=False,tweight='median',outfile=outname,outform='ASAP')
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

        default(sdaverage)

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
        self.res=sdaverage(infile=self.calfile,timeaverage=True,scanaverage=False,tweight='tintsys',polaverage=True,pweight='tsys',outfile=outname,outform='ASAP')
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

        default(sdaverage)

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
        self.res=sdaverage(infile=self.rawfile,timeaverage=True,scanaverage=False,tweight='tintsys',averageall=True,polaverage=False,outfile=outname,outform='ASAP')
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
        default(sdaverage)

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
            data = sdstatold(infile=data)
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
        self.res=sdaverage(infile=inname,timeaverage=True,scanaverage=False,
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
        self.res=sdaverage(infile=inname,timeaverage=True,scanaverage=False,
                       tweight='tint',outfile=outname,outform='ASAP')
        # verification
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        refedge = {'max': 1.5, 'min': 1.5, 'sum': 9286.5}
        refinner = {'max': 2.0, 'min': 2.0, 'sum': 4002.0}
        outstat = sdstatold(outname, masklist=[3000, 5000])
        self._comp_stat(outstat, refinner)
        outstat = sdstatold(outname, masklist=[[0, 2999], [5001, 8192]])        
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
        self.res=sdaverage(infile=inname,timeaverage=False,scanaverage=False,
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
        self.res=sdaverage(infile=inname,timeaverage=False,scanaverage=False,
                       polaverage=True,pweight='tsys',
                       outfile=outname,outform='ASAP')
        # verification
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        refedge = {'max': 1.5, 'min': 1.5, 'sum': 9286.5}
        refinner = {'max': 2.0, 'min': 2.0, 'sum': 4002.0}
        outstat = sdstatold(outname, masklist=[3000, 5000])
        self._comp_stat(outstat, refinner)
        outstat = sdstatold(outname, masklist=[[0, 2999], [5001, 8192]])        
        self._comp_stat(outstat, refedge)


class sdaverage_avetest_selection(selection_syntax.SelectionSyntaxTest,
                                  sdaverage_avetest_base,unittest.TestCase):
    """
    Test selection syntax in averaging.
    Selection parameters to test are:
    field, spw (no channel selection), scan, pol
    """
    rawfile = 'sd_analytic_type2-1.asap'
    prefix = sdaverage_unittest_base.taskname+'TestSel'
    ref_save = ( (5.5,), (1.5,), (3.5,), (10,0.1), (20,-0.1), (20,0.1), (30,-0.1) )
    ref_tave = ( (5.5,), (2.5,), (15,0.1), (25,-0.1) )
    ref_pave = ( (5.5,), (2.5,), (20.,) )
    
    @property
    def task(self):
        return sdaverage
    
    @property
    def spw_channel_selection(self):
        return False

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.sddatapath+self.rawfile, self.rawfile)

        default(sdaverage)
        self.refdata = None
        self.scanaverage = False
        self.timeaverage = False
        self.polaverage = False
        self.outname = self.prefix+self.postfix

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    ####################
    # Additional tests
    ####################
    #N/A

    ####################
    # scan
    ####################
    def test_scan_id_default(self):
        """test scan selection (scan='', timeaverage)"""
        scan = ''
        mode = 'timeaverage'
        ref_idx = []
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile, scan=scan, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_scan_id_exact(self):
        """test scan selection (scan='19', scanaverage)"""
        scan = '19'
        mode = 'scanaverage'
        ref_idx = [0]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile, scan=scan, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_scan_id_lt(self):
        """test scan selection (scan='<17', scanaverage)"""
        scan = '<17'
        mode = 'scanaverage'
        ref_idx = [1,2]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile, scan=scan, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_scan_id_gt(self):
        """test scan selection (scan='>18', scanaverage)"""
        scan = '>18'
        mode = 'scanaverage'
        ref_idx = [0]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile, scan=scan, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_scan_id_range(self):
        """test scan selection (scan='17~18', polaverage)"""
        scan = '17~18'
        mode = 'polaverag'
        ref_idx = [2]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile, scan=scan, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_scan_id_list(self):
        """test scan selection (scan='15,16', timeaverage)"""
        scan = '15,16'
        mode = 'timeaverage'
        ref_idx = [1]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile, scan=scan, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_scan_id_exprlist(self):
        """test scan selection (scan='19,<17', scanaverage)"""
        scan = '19,<17'
        mode = 'scanaverage'
        ref_idx = [0,1,2]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile, scan=scan, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    ####################
    # pol
    ####################
    def test_pol_id_default(self):
        """test pol selection (pol='', timeaverage)"""
        pol = ''
        mode = 'timeaverage'
        ref_idx = []
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile, pol=pol, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_pol_id_exact(self):
        """test pol selection (pol='0', timeaverage)"""
        pol = '0'
        mode = 'timeaverage'
        ref_idx = [1,2]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile, pol=pol, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_pol_id_lt(self):
        """test pol selection (pol='<1', timeaverage)"""
        pol = '<1'
        mode = 'timeaverage'
        ref_idx = [1,2]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile, pol=pol, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_pol_id_gt(self):
        """test pol selection (pol='>0', timeaverage)"""
        pol = '>0'
        mode = 'timeaverage'
        ref_idx = [0,3]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile, pol=pol, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_pol_id_range(self):
        """test pol selection (pol='0~1', timeaverage)"""
        pol = '0~1'
        mode = 'timeaverage'
        ref_idx = []
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile, pol=pol, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_pol_id_list(self):
        """test pol selection (pol='0,1', timeaverage)"""
        pol = '0,1'
        mode = 'timeaverage'
        ref_idx = []
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile, pol=pol, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_pol_id_exprlist(self):
        """test pol selection (pol='0,>0', timeaverage)"""
        pol = '0,>0'
        mode = 'timeaverage'
        ref_idx = []
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile, pol=pol, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    ####################
    # field
    ####################
    def test_field_value_default(self):
        """test field selection (field='', timeaverage)"""
        field = ''
        mode = 'timeaverage'
        ref_idx = []
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_field_id_exact(self):
        """test field selection (field='9', scanaverage)"""
        field = '9'
        mode = 'scanaverage'
        ref_idx = [0]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_field_id_lt(self):
        """test field selection (field='<7', scanaverage)"""
        field = '<7'
        mode = 'scanaverage'
        ref_idx = [1,2]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_field_id_gt(self):
        """test field selection (field='>8', scanaverage)"""
        field = '>8'
        mode = 'scanaverage'
        ref_idx = [0]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_field_id_range(self):
        """test field selection (field='7~8', polaverage)"""
        field = '7~8'
        mode = 'polaverage'
        ref_idx = [2]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_field_id_list(self):
        """test field selection (field='5,6', scanaverage)"""
        field = '5,6'
        mode = 'scanaverage'
        ref_idx = [1,2]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_field_id_exprlist(self):
        """test field selection (field='<7,9', timeaverage)"""
        field = '<7,9'
        mode = 'timeaverage'
        ref_idx = [0,1]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_field_value_exact(self):
        """test field selection (field='3C273', polaverage)"""
        field = '3C273'
        mode = 'polaverage'
        ref_idx = [2]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_field_value_pattern(self):
        """test field selection (field='M*', timeaverage)"""
        field = 'M*'
        mode = 'timeaverage'
        ref_idx = [0,1]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_field_value_list(self):
        """test field selection (field='M100,M30', timeaverage)"""
        field = 'M100,M30'
        mode = 'timeaverage'
        ref_idx = [0,1]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_field_mix_exprlist(self):
        """test field selection (field='M100,>8', timeaverage)"""
        field = 'M100,>8'
        mode = 'timeaverage'
        ref_idx = [0,1]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    ####################
    # spw 
    ####################
    def test_spw_id_default(self):
        """test spw selection (spw='', timeaverage)"""
        spw = ''
        mode = 'timeaverage'
        ref_idx = []
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_spw_id_exact(self):
        """test spw selection (spw='23', timeaverage)"""
        spw = '23'
        mode = 'timeaverage'
        ref_idx = [1]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_spw_id_lt(self):
        """test spw selection (spw='<22', scanaverage)"""
        spw = '<22'
        mode = 'scanaverage'
        ref_idx = [0]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_spw_id_gt(self):
        """test spw selection (spw='>24', polaverage)"""
        spw = '>24'
        mode = 'polaverage'
        ref_idx = [2]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_spw_id_range(self):
        """test spw selection (spw='21~23', scanaverage)"""
        spw = '21~23'
        mode = 'scanaverage'
        ref_idx = [0,1,2]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_spw_id_list(self):
        """test spw selection (spw='23,21', timeaverage)"""
        spw = '23,21'
        mode = 'timeaverage'
        ref_idx = [0,1]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_spw_id_exprlist(self):
        """test spw selection (spw='>24,23', polaverage)"""
        spw = '>24,23'
        mode = 'polaverage'
        ref_idx = [1,2]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_spw_id_pattern(self):
        """test spw selection (spw='*', timeaverage)"""
        spw = '*'
        mode = 'timeaverage'
        ref_idx = []
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_spw_value_frequency(self):
        """test spw selection (spw='300.03~300.50GHz', polaverage)"""
        spw = '300.03~300.50GHz' # IFNO=25 will be selected
        mode = 'polaverage'
        ref_idx = [2]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_spw_value_velocity(self):
        """test spw selection (spw='-510~-30km/s', polaverage)"""
        spw = '-510~-30km/s'  # IFNO=25 will be selected
        mode = 'polaverage'
        ref_idx = [2]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    def test_spw_mix_exprlist(self):
        """test spw selection (spw='23,500~501km/s', timeaverage)"""
        spw = '23,500~501km/s'   #IFNO=23,25 will be selected
        mode = 'timeaverage'
        ref_idx = [0,1]
        self._set_average_mode(mode)
        self.res=self.run_task(infile=self.rawfile,spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,outfile=self.outname,outform='ASAP')
        self._compare_with_polynomial(self.outname, self.refdata, ref_idx)

    ####################
    # Helper functions
    ####################
    def _set_average_mode(self, avemode):
        shortmode = avemode[0].upper()
        if shortmode not in ['S', 'T', 'P']:
            self.fail("Internal Error: Invalid average mode.")
        self.scanaverage = (shortmode == 'S')
        self.timeaverage = (shortmode != 'P')
        self.polaverage = (shortmode == 'P')
        self.kernel = ''
        if shortmode == 'S': self.refdata = self.ref_save
        elif shortmode == 'T': self.refdata = self.ref_tave
        else: self.refdata = self.ref_pave

    def _compare_with_polynomial(self, name, ref_data, ref_idx=[], precision = 1.e-6):
        self._checkfile( name )
        sout = sd.scantable(name,average=False)
        nrow = sout.nrow()
        if len(ref_idx) == 0:
            ref_idx = range(nrow)

        self.assertEqual(nrow,len(ref_idx),"The rows in output table differs from the expected value.")
        for irow in range(nrow):
            y = sout._getspectrum(irow)
            x = numpy.array( range(len(y)) )
            # analytic solution
            coeff = ref_data[ ref_idx[irow] ]
            yana = self._create_ploynomial_array(coeff, x)
            # compare
            rdiff = self._get_array_relative_diff(y,yana)
            rdiff_max = max(abs(rdiff))
            self.assertTrue(rdiff_max < precision, "Maximum relative difference %f > %f" % (rdiff_max, precision))
                
            
    def _create_ploynomial_array(self, coeff, x):
        """ Create an array from a list of polynomial coefficients and x-array"""
        xarr = numpy.array(x)
        yarr = numpy.zeros(len(xarr))
        for idim in range(len(coeff)):
            ai = coeff[idim]
            yarr += ai*xarr**idim
        return yarr
        

class sdaverage_smoothtest_selection(selection_syntax.SelectionSyntaxTest,
                                     sdaverage_smoothtest_base,
                                     unittest.TestCase):
    """
    Test selection syntax in smoothing.
    Selection parameters to test are:
    field, spw (no channel selection), scan, pol
    """
    rawfile = 'sd_analytic_type1-3.bl.asap'
    prefix = sdaverage_unittest_base.taskname+'TestSel'
    kernel = 'boxcar'
    kwidth = 5
    refval = ({'value': 1.0, 'channel': (17,21)},
              {'value': 2.0, 'channel': (37,41)},
              {'value': 4.0, 'channel': (57,61)},
              {'value': 6.0, 'channel': (77,81)},)
    
    @property
    def task(self):
        return sdaverage
    
    @property
    def spw_channel_selection(self):
        return False

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.sddatapath+self.rawfile, self.rawfile)
        
        default(sdaverage)
        self.scanaverage = False
        self.timeaverage = False
        self.polaverage = False
        self.outname = self.prefix+self.postfix

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    ####################
    # Additional tests
    ####################
    #N/A

    ####################
    # scan
    ####################
    def test_scan_id_default(self):
        """test scan selection (scan='', boxcar)"""
        scan = ''
        ref_idx = []
        self.res=self.run_task(infile=self.rawfile, scan=scan, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_scan_id_exact(self):
        """test scan selection (scan='16', boxcar)"""
        scan = '16'
        ref_idx = [1,2]
        self.res=self.run_task(infile=self.rawfile, scan=scan, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_scan_id_lt(self):
        """test scan selection (scan='<16', boxcar)"""
        scan = '<16'
        ref_idx = [0]
        self.res=self.run_task(infile=self.rawfile, scan=scan, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_scan_id_gt(self):
        """test scan selection (scan='>16', boxcar)"""
        scan = '>16'
        ref_idx = [3]
        self.res=self.run_task(infile=self.rawfile, scan=scan, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_scan_id_range(self):
        """test scan selection (scan='16~17', boxcar)"""
        scan = '16~17'
        ref_idx = [1,2,3]
        self.res=self.run_task(infile=self.rawfile, scan=scan, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_scan_id_list(self):
        """test scan selection (scan='15,17', boxcar)"""
        scan = '15,17'
        ref_idx = [0,3]
        self.res=self.run_task(infile=self.rawfile, scan=scan, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_scan_id_exprlist(self):
        """test scan selection (scan='15,>16', boxcar)"""
        scan = '15,>16'
        ref_idx = [0,3]
        self.res=self.run_task(infile=self.rawfile, scan=scan, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    ####################
    # pol
    ####################
    def test_pol_id_default(self):
        """test pol selection (pol='', boxcar)"""
        pol = ''
        ref_idx = []
        self.res=self.run_task(infile=self.rawfile, pol=pol, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_pol_id_exact(self):
        """test pol selection (pol='1', boxcar)"""
        pol = '1'
        ref_idx = [1,3]
        self.res=self.run_task(infile=self.rawfile, pol=pol, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_pol_id_lt(self):
        """test pol selection (pol='<1', boxcar)"""
        pol = '<1'
        ref_idx = [0,2]
        self.res=self.run_task(infile=self.rawfile, pol=pol, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_pol_id_gt(self):
        """test pol selection (pol='>0', boxcar)"""
        pol = '>0'
        ref_idx = [1,3]
        self.res=self.run_task(infile=self.rawfile, pol=pol, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_pol_id_range(self):
        """test pol selection (pol='0~1', boxcar)"""
        pol = '0~1'
        ref_idx = []
        self.res=self.run_task(infile=self.rawfile, pol=pol, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_pol_id_list(self):
        """test pol selection (pol='0,1', boxcar)"""
        pol = '0,1'
        ref_idx = []
        self.res=self.run_task(infile=self.rawfile, pol=pol, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_pol_id_exprlist(self):
        """test pol selection (pol='1,<1', boxcar)"""
        pol = '1,<1'
        ref_idx = []
        self.res=self.run_task(infile=self.rawfile, pol=pol, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    ####################
    # field
    ####################
    def test_field_value_default(self):
        """test field selection (field='', boxcar)"""
        field = ''
        ref_idx = []
        self.res=self.run_task(infile=self.rawfile, field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_field_id_exact(self):
        """test field selection (field='6', boxcar)"""
        field = '6'
        ref_idx = [1]
        self.res=self.run_task(infile=self.rawfile, field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_field_id_lt(self):
        """test field selection (field='<6', boxcar)"""
        field = '<6'
        ref_idx = [0]
        self.res=self.run_task(infile=self.rawfile, field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_field_id_gt(self):
        """test field selection (field='>7', boxcar)"""
        field = '>7'
        ref_idx = [3]
        self.res=self.run_task(infile=self.rawfile, field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_field_id_range(self):
        """test field selection (field='6~8', boxcar)"""
        field = '6~8'
        ref_idx = [1,2,3]
        self.res=self.run_task(infile=self.rawfile, field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_field_id_list(self):
        """test field selection (field='5,8', boxcar)"""
        field = '5,8'
        ref_idx = [0,3]
        self.res=self.run_task(infile=self.rawfile, field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_field_id_exprlist(self):
        """test field selection (field='5,>6', boxcar)"""
        field = '5,>6'
        ref_idx = [0,2,3]
        self.res=self.run_task(infile=self.rawfile, field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_field_value_exact(self):
        """test field selection (field='M30', boxcar)"""
        field = 'M30'
        ref_idx = [2]
        self.res=self.run_task(infile=self.rawfile, field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_field_value_pattern(self):
        """test field selection (field='M*', boxcar)"""
        field = 'M*'
        ref_idx = [0,1,2]
        self.res=self.run_task(infile=self.rawfile, field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_field_value_list(self):
        """test field selection (field='3C273,M30', boxcar)"""
        field = '3C273,M30'
        ref_idx = [2,3]
        self.res=self.run_task(infile=self.rawfile, field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_field_mix_exprlist(self):
        """test field selection (field='<6,3*', boxcar)"""
        field = '<6,3*'
        ref_idx = [0,3]
        self.res=self.run_task(infile=self.rawfile, field=field, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    ####################
    # spw 
    ####################
    def test_spw_id_default(self):
        """test spw selection (spw='', boxcar)"""
        spw = ''
        ref_idx = []
        self.res=self.run_task(infile=self.rawfile, spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_spw_id_exact(self):
        """test spw selection (spw='23', boxcar)"""
        spw = '23'
        ref_idx = [0,3]
        self.res=self.run_task(infile=self.rawfile, spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_spw_id_lt(self):
        """test spw selection (spw='<23', boxcar)"""
        spw = '<23'
        ref_idx = [2]
        self.res=self.run_task(infile=self.rawfile, spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_spw_id_gt(self):
        """test spw selection (spw='>23', boxcar)"""
        spw = '>23'
        ref_idx = [1]
        self.res=self.run_task(infile=self.rawfile, spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_spw_id_range(self):
        """test spw selection (spw='22~25', boxcar)"""
        spw = '22~25'
        ref_idx = [0,1,3]
        self.res=self.run_task(infile=self.rawfile, spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_spw_id_list(self):
        """test spw selection (spw='21,25', boxcar)"""
        spw = '21,25'
        ref_idx = [1,2]
        self.res=self.run_task(infile=self.rawfile, spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_spw_id_exprlist(self):
        """test spw selection (spw='23,>24', boxcar)"""
        spw = '23,>24'
        ref_idx = [0,1,3]
        self.res=self.run_task(infile=self.rawfile, spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_spw_id_pattern(self):
        """test spw selection (spw='*', boxcar)"""
        spw = '*'
        ref_idx = []
        self.res=self.run_task(infile=self.rawfile, spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_spw_value_frequency(self):
        """test spw selection (spw='299.52~300.47GHz', boxcar)"""
        spw = '299.52~300.47GHz' # IFNO=23 will be selected
        ref_idx = [0,3]
        self.res=self.run_task(infile=self.rawfile, spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_spw_value_velocity(self):
        """test spw selection (spw='-510.~470.km/s', boxcar)"""
        spw = '-510.~470km/s' # IFNO=23,25 will be selected
        ref_idx = [0,1,3]
        self.res=self.run_task(infile=self.rawfile, spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    def test_spw_mix_exprlist(self):
        """test spw selection (spw='25,0~501km/s', boxcar)"""
        spw = '25,0~501km/s' # all IFs will be selected
        ref_idx = []
        self.res=self.run_task(infile=self.rawfile, spw=spw, timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.refval, ref_idx)

    ####################
    # Helper functions
    ####################
    def _compare_with_tophat(self, name, ref_data, ref_idx=[], precision = 1.e-6):
        self._checkfile( name )
        sout = sd.scantable(name,average=False)
        nrow = sout.nrow()
        if len(ref_idx) == 0:
            ref_idx = range(nrow)

        self.assertEqual(nrow,len(ref_idx),"The rows in output table differs from the expected value.")
        for irow in range(nrow):
            y = sout._getspectrum(irow)
            nchan = len(y)
            # analytic solution
            curr_ref = ref_data[ ref_idx[irow] ]
            chanlist = curr_ref['channel']
            valuelist = curr_ref['value']
            yana = self._create_tophat_array(nchan, chanlist, valuelist)
            # compare
            rdiff = self._get_array_relative_diff(y, yana, precision)
            rdiff_max = max(abs(rdiff))
            self.assertTrue(rdiff_max < precision, "Maximum relative difference %f > %f" % (rdiff_max, precision))
    
    def _create_tophat_array(self, nchan, chanlist, valuelist):
        array_types = (tuple, list, numpy.ndarray)
        # check for inputs
        if nchan < 1:
            self.fail("Internal error. Number of channels should be > 0")
        if type(chanlist) not in array_types:
            self.fail("Internal error. Channel range list for reference data is not an array type")
        if type(chanlist[0]) not in array_types:
            chanlist = [ chanlist ]
        if type(valuelist) not in array_types:
            valuelist = [ valuelist ]
        nval = len(valuelist)
        nrange = len(chanlist)
        # generate reference data
        ref_data = numpy.zeros(nchan)
        for irange in range(nrange):
            curr_range = chanlist[irange]
            if type(curr_range) not in array_types or len(curr_range) < 2:
                self.fail("Internal error. Channel range list  for reference data should be a list of 2 elements arrays.")
            schan = curr_range[0]
            echan = curr_range[1]
            ref_data[schan:echan+1] = valuelist[irange % nval]
        return ref_data

###
# Test weighting by integration time
###
class sdaverage_test_weighting_tint(unittest.TestCase):
    """
    Test TINT weighting.

    Data is testaverage.asap
    """
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdaverage/'
    rawfile = 'testaverage.asap'
    prefix = 'sdaverage_test_weighting_tint'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdaverage)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        
        os.system( 'rm -rf '+self.prefix+'*' )

    def verify(self, outfile, weight_tsys):
        with tbmanager(outfile) as tb:
            sp = tb.getcol('SPECTRA')

        with tbmanager(self.rawfile) as tb:
            raw_sp = tb.getcol('SPECTRA')
            exposure = tb.getcol('INTERVAL')
            tsys = tb.getcol('TSYS')

        weight = numpy.ones(raw_sp.shape, dtype=float) * exposure.reshape((1,len(exposure)))
        if weight_tsys:
            weight /= (tsys * tsys)

        expected = numpy.sum(raw_sp * weight, axis=1) / numpy.sum(weight, axis=1)
        diff = abs((sp.flatten() - expected.flatten()) / expected.flatten())
        self.assertTrue(all(diff < 1.0e-6),
                        msg='Averaging failed to verify: actual %s expected %s'%(sp.flatten()[0], expected.flatten()[0]))

    def test_weighting_tint(self):
        """test_weighing_tint: test averaging by tint"""
        outfile = self.prefix + '_tint.asap'
        res = sdaverage(infile=self.rawfile, outfile=outfile, timeaverage=True, tweight='tint')
        self.verify(outfile, weight_tsys=False)

    def test_weighting_tintsys(self):
        """test_weighting_tintsys: test averaging by tintsys"""
        outfile = self.prefix + '_tintsys.asap'
        res = sdaverage(infile=self.rawfile, outfile=outfile, timeaverage=True, tweight='tint')
        self.verify(outfile, weight_tsys=True)
        
###
# Test flag information handling
###
class sdaverage_test_average_flag(unittest.TestCase):
    """
    Test flag information handling.

    Data is sdaverage_testflag.asap

    Summary of the data:
    ROW | FLAGROW    | FLAGTRA          | SPECTRA 
     0  | 0          | ch 10~11 flagged | spurious at ch 10~11
     1  | 1 (flagged)| all 0            | spurious at ch 10
     2  | 0          | ch 10,40 flagged | spurious at ch 10,40

    Test list
        test_average_flag: test if average handles flag information properly
        test_average_novaliddata: test if the task handles fully flagged data
                                  properly
        test_avearge_novaliddata_scan: test if the task handles the data that
                                       has several scans and one scan is
                                       fully flagged
        test_smooth_hanning: test if hanning smoothing (direct convolution)
                             handles flag information correctly
        test_smooth_gaussian: test if gaussian smoothing (FFT convolution)
                              handles flag information correctly
        test_smooth_regrid: test if regridding (binning) handles flag
                            information correctly
    """
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdaverage/'
    rawfile = 'sdaverage_testflag.asap'
    prefix = 'sdaverage_test_average_flag'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdaverage)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        
        os.system( 'rm -rf '+self.prefix+'*' )

    def _get_data(self, filename, optional=[]):
        cols = ['FLAGROW', 'FLAGTRA', 'SPECTRA']
        cols.extend(optional)
        with tbmanager(filename) as tb:
            vals = [tb.getcol(col) for col in cols]
        return vals

    def _message_for_assert(self, val, ref, col, isshape, row, channel, refistol=False):
        word = 'tolerance' if refistol else 'expected'
        msg = '%s differ (%s %s result %s)'%(col,word,ref,val)
        if isshape:
            msg = 'shape of ' + msg
        head = ''
        if row is not None:
            head = 'Row %s'%(row)
        if channel is not None:
            head = head + ' Channel %s'%(channel)
        if len(head) > 0:
            msg = head + ': ' + msg
        #print msg
        return msg
    
    def _assert_equal(self, val, ref, col, isshape=False, row=None, channel=None):
        msg = self._message_for_assert(val, ref, col, isshape, row, channel)
        self.assertEqual(val, ref, msg=msg)

    def _assert_less(self, val, tol, col, isshape=False, row=None, channel=None):
        msg = self._message_for_assert(val, tol, col, isshape, row, channel, refistol=True)
        self.assertLess(val, tol, msg=msg)
        
    
    def _verify_shape(self, expected_nrow, expected_nchan, flagrow, flagtra, spectra):
        expected_shape = (expected_nchan,expected_nrow,)
        self._assert_equal(len(flagrow), expected_nrow, 'FLAGROW', isshape=True)
        self._assert_equal(list(flagtra.shape), list(expected_shape), 'FLAGTRA', isshape=True)
        self._assert_equal(list(spectra.shape), list(expected_shape), 'SPECTRA', isshape=True)
       
    def _verify_average(self, outfile):
        # get data before averaging
        flagrow_org, flagtra_org, spectra_org, interval_org = self._get_data(self.rawfile, ['INTERVAL'])

        # get data after averaging
        flagrow, flagtra, spectra = self._get_data(outfile)

        # basic check
        # all data should be averaged into one row
        nrow = 1
        nchan = spectra_org.shape[0]
        self._verify_shape(1, nchan, flagrow, flagtra, spectra)

        # verify
        flagrow = flagrow[0]
        flagtra = flagtra[:,0]
        spectra = spectra[:,0]

        # verify FLAGROW
        flagrow_expected = 0 if any(flagrow_org == 0) else 1
        self._assert_equal(flagrow, flagrow_expected, 'FLAGROW')

        # verify FLAGTRA
        def gen_averaged_channelflag(rflag, chflag):
            nchan, nrow = chflag.shape
            for ichan in xrange(nchan):
                yield 0 if any(rflag + chflag[ichan] == 0) else 128
        flagtra_expected = numpy.array(list(gen_averaged_channelflag(flagrow_org, flagtra_org)))
        for ichan in xrange(nchan):
            self._assert_equal(flagtra_expected[ichan], flagtra[ichan], 'FLAGTRA', row=0, channel=ichan)
        
        # verify SPECTRA
        def gen_averaged_spectra(rflag, chflag, data, weight):
            nchan, nrow = data.shape
            for ichan in xrange(nchan):
                dsum = 0.0
                wsum = 0.0
                for irow in xrange(nrow):
                    if rflag[irow] == 0 and chflag[ichan,irow] == 0:
                        dsum += weight[irow] * data[ichan,irow]
                        wsum += weight[irow]
                if wsum == 0:
                    yield None
                else:
                    yield dsum / wsum
        spectra_expected = numpy.array(list(gen_averaged_spectra(flagrow_org,
                                                                 flagtra_org,
                                                                 spectra_org,
                                                                 interval_org)))
        tol = 1.0e-7
        for ichan in xrange(nchan):
            if flagtra[ichan] == 0:
                sp = spectra[ichan]
                ex = spectra_expected[ichan]
                diff = abs((sp - ex) / ex)
                #print ichan, diff
                self._assert_less(diff, tol, 'SPECTRA', row=0, channel=ichan)
            else:
                print 'Skip channel %s since it is flagged'%(ichan)

    def _verify_smooth(self, outfile):
        flagrow_org, flagtra_org, spectra_org = self._get_data(self.rawfile)
        flagrow, flagtra, spectra = self._get_data(outfile)

        # basic check
        nchan_expected,nrow_expected = flagtra_org.shape
        self._verify_shape(nrow_expected, nchan_expected, flagrow, flagtra, spectra)

        nchan, nrow = spectra.shape

        for irow in xrange(nrow):
            # FLAGROW should not be modified
            rflag_ref = flagrow_org[irow]
            rflag = flagrow[irow]
            self._assert_equal(rflag_ref, rflag, 'FLAGROW', row=irow)

            # FLAGTRA should not be modified
            flag_ref = flagtra[:,irow]
            flag = flagtra[:,irow]
            for ichan in xrange(nchan):
                self._assert_equal(flag_ref[ichan], flag[ichan], 'FLAGTRA', row=irow, channel=ichan)

            # verify SPECTRA
            sp_ref = spectra_org[:,irow]
            sp = spectra[:,irow]
            if rflag != 0:
                # flagged row should not be processed
                for ichan in xrange(nchan):
                    self._assert_equal(sp_ref[ichan], sp[ichan], 'SPECTRA', row=irow, channel=ichan)
            else:
                # Here, what should be tested is spurious data (at flagged
                # channels) are replaced with the value interpolated from
                # neighbors. Tolerance is set to loose value to avoid false
                # failue due to unexpected behavior of FFT based smoothing.
                tol = 1.0e1
                
                sp_expected = sp_ref.copy()
                sp_expected[:] = sp_ref[0]
                diff = abs((sp - sp_expected) / sp_expected)
                self._assert_less(max(diff), tol, 'SPECTRA', row=irow)
        
    def _verify_regrid(self, outfile, chanwidth):
        flagrow_org, flagtra_org, spectra_org = self._get_data(self.rawfile)
        flagrow, flagtra, spectra = self._get_data(outfile)

        # basic check
        nchan_org, nrow = spectra_org.shape
        nchan_expected = int(numpy.ceil(float(nchan_org) / float(chanwidth)))
        self._verify_shape(nrow, nchan_expected, flagrow, flagtra, spectra)

        nchan, nrow = spectra.shape

        # generator for FLAGTRA and SPECTRA
        def gen_flagtra(flag, w):
            nchan = len(flag)
            width = int(w)
            for i in xrange(0, nchan, width):
                yield 0 if any(flag[i:i+w] == 0) else 128
                
        def gen_spectra(sp, flag, w):
            nchan = len(sp)
            width = int(w)
            for i in xrange(0, nchan, width):
                s = sp[i:i+w]
                f = [1.0 if _f == 0 else 0.0 for _f in flag[i:i+w]]
                sumf = sum(f)
                if sumf == 0.0:
                    yield 0.0
                else:
                    yield sum(s * f) / sum(f) 

        # verify
        for irow in xrange(nrow):
            # FLAGROW should not be modified
            rflag_ref = flagrow_org[irow]
            rflag = flagrow[irow]
            self._assert_equal(rflag_ref, rflag, 'FLAGROW', row=irow)

            # FLAGTRA is combined with OR operation
            flag_ref = numpy.array(list(gen_flagtra(flagtra_org[:,irow], chanwidth)))
            flag = flagtra[:,irow]
            #print irow, flag_ref, flag
            for ichan in xrange(nchan):
                self._assert_equal(flag_ref[ichan], flag[ichan], 'FLAGTRA', row=irow, channel=ichan)

            # verify SPECTRA
            sp_ref = numpy.array(list(gen_spectra(spectra_org[:,irow],
                                                  flagtra_org[:,irow],
                                                  chanwidth)))
            sp = spectra[:,irow]
            
            # ignore FLAG_ROW, all rows are processed
            tol = 1.0e-6

            for ichan in xrange(nchan):
                val = sp[ichan]
                ref = sp_ref[ichan]
                if ref == 0.0:
                    self._assert_equal(ref, val, 'SPECTRA', row=irow, channel=ichan)
                else:
                    diff = abs((val - ref) / ref)
                    #print irow, ichan,  diff
                    self._assert_less(diff, tol, 'SPECTRA', row=irow, channel=ichan)

    def test_average_flag(self):
        """test_average_flag: test if average handles flag information properly"""
        outfile = self.prefix + '.asap'
        res = sdaverage(infile=self.rawfile, outfile=outfile, timeaverage=True, tweight='tint')

        self._verify_average(outfile)

    def test_average_novaliddata(self):
        """test_average_novaliddata: test if the task throws exception if no valid data exists"""
        # flag all rows
        with tbmanager(self.rawfile, nomodify=False) as tb:
            flagrow = tb.getcol('FLAGROW')
            flagrow[:] = 1
            tb.putcol('FLAGROW', flagrow)

        # double check
        with tbmanager(self.rawfile, nomodify=False) as tb:
            flagrow = tb.getcol('FLAGROW')
        self.assertTrue(all(flagrow == 1), msg='Failed to preparing data')

        outfile = self.prefix + '.asap'

        sdaverage(infile=self.rawfile, outfile=outfile, timeaverage=True, tweight='tint')
        
        flagrow_in, flagtra_in, spectra_in, interval_in = self._get_data(self.rawfile, ['INTERVAL'])
        flagrow_out, flagtra_out, spectra_out, interval_out = self._get_data(outfile, ['INTERVAL'])

        # all data are averaged to one spectrum
        nrow_expected = 1
        shape_expected = (flagtra_in.shape[0],nrow_expected,)
        self.assertEqual(len(flagrow_out), nrow_expected, msg='FLAGROW: shape differ')
        self.assertEqual(len(interval_out), nrow_expected, msg='INTERVAL: shape differ')
        self.assertEqual(flagtra_out.shape, shape_expected, msg='FLAGTRA: shape differ')
        self.assertEqual(spectra_out.shape, shape_expected, msg='SPECTRA: shape differ')

        # FLAGROW must be 1
        self.assertEqual(flagrow_out[0], 1, msg='FLAGROW: value differ')

        # FLAGTRA must be all 128
        self.assertTrue(all(flagtra_out.flatten() == 128), msg='FLAGTRA: value differ')

        # SPECTRA must be nominal averaged data
        spectra_expected = (spectra_in * interval_in).sum(axis=1) / interval_in.sum()
        diff = abs((spectra_out.flatten() - spectra_expected) / spectra_expected)
        tol = 1.0e-6
        self.assertTrue(all(diff < tol), msg='SPECTRA: value differ')

        # INTERVAL must be a sum of all rows
        self.assertEqual(interval_out, interval_in.sum(), msg='INTERVAL: value differ')
        

    def test_average_novaliddata_scan(self):
        """test_avearge_novaliddata_scan: test if the task handles the data that has several scans and one scan is fully flagged"""
        outfile = self.prefix + '.asap'
        res = sdaverage(infile=self.rawfile, outfile=outfile, timeaverage=True, tweight='tint', scanaverage=True)

        # one row per scan
        # there are three rows that has proper scan number (0, 1, 2)
        # scan 1 is row flagged so that resulting data is just a copy
        # of rows 0 and 2 of input data
        flagrow_in, flagtra_in, spectra_in = self._get_data(self.rawfile)
        flagrow_out, flagtra_out, spectra_out = self._get_data(outfile)

        #valid_rows = numpy.where(flagrow_in == 0)[0]
        #flagtra_expected = flagtra_in.take(valid_rows, axis=1)
        #spectra_expected = spectra_in.take(valid_rows, axis=1)
        # if all data are flagged, FLAGTRA will be all 128
        flagtra_expected = flagtra_in.copy()
        for irow in xrange(len(flagrow_in)):
            if flagrow_in[irow] != 0:
                flagtra_expected[:,irow] = 128
        spectra_expected = spectra_in
        self.assertEqual(flagrow_out.shape, flagrow_in.shape, msg='FLAGROW: shape differ')
        self.assertEqual(flagtra_out.shape, flagtra_expected.shape, msg='FLAGTRA: shape differ')
        self.assertEqual(spectra_out.shape, spectra_expected.shape, msg='SPECTRA: shape differ')
        self.assertTrue(all(flagrow_out == flagrow_in), msg='FLAGROW: value differ')
        self.assertTrue(all(flagtra_out.flatten() == flagtra_expected.flatten()), msg='FLAGTRA: value differ')
        self.assertTrue(all(spectra_out.flatten() == spectra_expected.flatten()), msg='SPECTRA: value differ')

    def test_smooth_hanning(self):
        """test_smooth_hanning: test if hanning smoothing (direct convolution) handles flag information correctly"""
        outfile = self.prefix + '.asap'
        res = sdaverage(infile=self.rawfile, outfile=outfile, kernel='hanning')

        self._verify_smooth(outfile)

    def test_smooth_gaussian(self):
        """test_smooth_gaussian: test if gaussian smoothing (FFT convolution) handles flag information correctly"""
        outfile = self.prefix + '.asap'
        res = sdaverage(infile=self.rawfile, outfile=outfile, kernel='gaussian')

        self._verify_smooth(outfile)


    def test_smooth_regrid(self):
        """test_smooth_regrid: test if regridding (binning) handles flag information correctly"""
        outfile = self.prefix + '.asap'
        chanwidth = 2
        res = sdaverage(infile=self.rawfile, outfile=outfile, kernel='regrid', chanwidth=str(chanwidth))

        self._verify_regrid(outfile, chanwidth)

    

def suite():
    return [sdaverage_badinputs, sdaverage_smoothTest, sdaverage_storageTest,
            sdaverage_test6, sdaverage_test7, sdaverage_test8, sdaverage_test9,
            sdaverage_test_flag, sdaverage_avetest_selection, sdaverage_smoothtest_selection,
            sdaverage_test_weighting_tint, sdaverage_test_average_flag]
