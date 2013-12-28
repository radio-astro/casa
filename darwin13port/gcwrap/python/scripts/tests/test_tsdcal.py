import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
import sha
import time
import numpy
import re
import string

# to rethrow exception 
import inspect
g = sys._getframe(len(inspect.stack())-1).f_globals
g['__rethrow_casa_exceptions'] = True
from tsdcal_cli import tsdcal_cli as tsdcal
#from sdcal import sdcal
import asap as sd

#
# Unit test of sdcal task.
# 

###
# Base class for sdcal unit test
###
class sdcal_unittest_base:
    """
    Base class for sdcal unit test
    """
    taskname='sdcal'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdcal/'
    tolerance=1.0e-15

    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='output file %s was not created because of the task failure'%(name))
        

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
        

###
# Base class for calibration test
###
class sdcal_caltest_base(sdcal_unittest_base):
    """
    Base class for calibration test
    """
    reffile=''
    postfix='.cal.asap'

    def _comparecal( self, name ):
        self._checkfile(name)
        sp=self._getspectra(name)
        spref=self._getspectra(self.reffile)

        self._checkshape( sp, spref )
        
        for irow in xrange(sp.shape[0]):
            diff=self._diff(sp[irow],spref[irow])
            retval=numpy.all(diff<0.01)
            maxdiff=diff.max()
            self.assertEqual( retval, True,
                             msg='calibrated result is wrong (irow=%s): maxdiff=%s'%(irow,diff.max()) )
        del sp, spref

###
# Base class for edgemarker testing
###
class sdcal_edgemarker_base(sdcal_unittest_base):
    """
    Base class for edgemarker testing
    """
    def _readref( self, name ):
        f = open( name, 'r' )
        lines = f.readlines()
        f.close()

        ret = []
        nline = len(lines)
        for line in lines:
            s = line.split()
            ret.append( [float(s[0]),float(s[1])] )
        return numpy.array(ret).transpose()
    
    def _checkmarker( self, name, refdata ):
        # refdata shape is (2,noff)
        noff = refdata.shape[1]
        tb.open(name)
        tsel = tb.query('SRCTYPE==1')
        nrow = tsel.nrows()
        self.assertTrue( nrow > 0,
                         msg='data doesn\'t have OFF spectra' )
        dir = tsel.getcol('DIRECTION')
        tsel.close()
        tb.close()
        
        self.assertEqual( 2*noff, dir.shape[1], # refdata store only POLNO==0
                          msg='number of OFF differ: %s (should be %s)'%(dir.shape[1],2*noff) )

        for irow in xrange(2*noff):
            idx = int(irow) / 2
            diff = self._diff( dir[:,irow], refdata[:,idx] )
            self.assertEqual( numpy.all(diff<0.01), True,
                              msg='direction for OFF differ (irow=%s): [%s,%s] (should be [%s,%s])'%(irow,dir[0,irow],dir[1,irow],refdata[0,idx],refdata[1,idx]) )

###
# Test on bad parameter settings
###
class sdcal_test0(sdcal_unittest_base,unittest.TestCase):
    """
    Test on bad parameter setting
    """
    # Input and output names
    rawfile='calpsGBT.asap'
    prefix=sdcal_unittest_base.taskname+'Test0'
    outfile=prefix+'.asap'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(tsdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test000(self):
        """Test 000: Default parameters"""
        # argument verification error
        try:
            self.res=tsdcal()
            #self.assertFalse(self.res)
            self.fail("The task must throw exception.")
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        
    def test001(self):
        """Test 001: Invalid calibration mode"""
        # argument verification error
        try:
            self.res=tsdcal(infile=self.rawfile,calmode='invalid',outfile=self.outfile)
            #self.assertFalse(self.res)
            self.fail("The task must throw exception.")
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test002(self):
        """Test 002: Specify existing output file name with overwrite=False"""
        outfile='calpsGBT.cal.asap'
        if (not os.path.exists(outfile)):
            shutil.copytree(self.datapath+outfile, outfile)
        try:
            self.res=tsdcal(infile=self.rawfile,outfile=outfile,overwrite=False)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Output file \'%s\' exists.'%(outfile))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        finally:
            os.system( 'rm -rf %s'%outfile )        



###
# Test GBT position switch calibration 
###
class sdcal_test1(sdcal_caltest_base,unittest.TestCase):
    """
    Test GBT position switch calibration 
    
    Data is taken from OrionS_rawACSmod and created by the following
    script:

    asap_init()
    s=sd.scantable('OrionS_rawACSmod',average=False)
    sel=sd.selector()
    sel.set_ifs([0])
    sel.set_scans([20,21,22,23])
    s.set_selection(sel)
    s.save('calpsGBT.asap','ASAP')

    In addition, unnecessary TCAL rows were removed.
    """
    # Input and output names
    rawfile='calpsGBT.asap'
    reffile='calpsGBT.cal.asap'
    prefix=sdcal_unittest_base.taskname+'Test1'
    calmode='ps'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(tsdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test100(self):
        """Test 100: test to calibrate data (GBT position switch)"""
        outname=self.prefix+self.postfix
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,tau=0.09,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

###
# Test GBT nodding calibration 
###
class sdcal_test2(sdcal_caltest_base,unittest.TestCase):
    """
    Test GBT nodding calibration 
    
    Data is taken from IRC+10216_rawACSmod and created by the following
    script:

    asap_init()
    s=sd.scantable('IRC+10216_rawACSmod',average=False)
    sel=sd.selector()
    sel.set_scans([229,230])
    sel.set_ifs([3])
    sel.set_cycles([0,1])
    s.set_selection(sel)
    s.save('calnodGBT.asap','ASAP')

    In addition, unnecessary TCAL rows were removed.
    """
    # Input and output names
    rawfile='calnodGBT.asap'
    reffile='calnodGBT.cal.asap'
    prefix=sdcal_unittest_base.taskname+'Test2'
    calmode='nod'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(tsdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test200(self):
        """Test 200: test to calibrate data (GBT nod)"""
        outname=self.prefix+self.postfix
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,tau=0.09,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

###
# Test GBT frequency switch calibration 
###
class sdcal_test3(sdcal_caltest_base,unittest.TestCase):
    """
    Test GBT frequency switch calibration 
    
    Data is taken from FLS3_all_newcal_SP and created by the following
    script:

    asap_init()
    sd.rc('scantable',storage='disk')
    s=sd.scantable('FLS3_all_newcal_SP',average=False)
    sel=sd.selector()
    sel.set_scans([14914])
    sel.set_cycles([0,1])
    s.set_selection(sel)
    s.save('calfsGBT.asap','ASAP')

    In addition, unnecessary TCAL rows were removed.
    """
    # Input and output names
    rawfile='calfsGBT.asap'
    reffile='calfsGBT.cal.asap'
    prefix=sdcal_unittest_base.taskname+'Test3'
    calmode='fs'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(tsdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test300(self):
        """Test 300: test to calibrate data (GBT frequency switch)"""
        outname=self.prefix+self.postfix
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)
        
###
# Test quotient
###
class sdcal_test4(sdcal_caltest_base,unittest.TestCase):
    """
    Test quotient.
    
    Data is taken from MOPS.rpf, which is included in ASAP package
    for testing, and created by the following script:

    asap_init()
    s=sd.scantable('MOPS.rpf',average=False)
    sel=sd.selector()
    sel.set_cycles([0,1])
    s.set_selection(sel)
    s.save('quotient.asap','ASAP')

    """
    # Input and output names
    rawfile='quotient.asap'
    reffile='quotient.cal.asap'
    prefix=sdcal_unittest_base.taskname+'Test4'
    calmode='quotient'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(tsdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test400(self):
        """Test 400: test to calibrate data (quotient)"""
        outname=self.prefix+self.postfix
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

###
# Test ALMA position switch calibration
###
class sdcal_test5(sdcal_caltest_base,unittest.TestCase):
    """
    Test ALMA position switch calibration (OTF raster with OFF scan)
    
    Data is taken from uid___A002_X8ae1b_X1 (DV01) and created by 
    the following script:

    asap_init()
    sd.splitant('uid___A002_X8ae1b_X1') # to split data by antenna
    s=sd.scantable('uid___A002_X8ae1b.DV01.asap',average=False)
    sel=sd.selector()
    sel.set_ifs([2])
    sel.set_cycles([20,118,205])
    s.set_selection(sel)
    s.save('calpsALMA.asap','ASAP')

    """
    # Input and output names
    rawfile='calpsALMA.asap'
    reffile='calpsALMA.cal.asap'
    prefix=sdcal_unittest_base.taskname+'Test5'
    calmode='ps'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(tsdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test500(self):
        """Test 500: test to calibrate data (ALMA position switch)"""
        outname=self.prefix+self.postfix
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)


###
# Test edgemarker
###
class sdcal_test_edgemarker_generic(sdcal_edgemarker_base,unittest.TestCase):
    """
    Test edgemarker function that is available for calmode='otf'. 

    Here, data will not be calibrated and only edge marking process
    will be executed. 
    """
    # Input and output names
    rawfile='lissajous.asap'
    prefix=sdcal_unittest_base.taskname+'TestEdgeMarkerGeneric'
    reffiles = [ 'marker.otf.default.ref',
                 'marker.otf.custom.ref' ]

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        for reffile in self.reffiles:
            if (not os.path.exists(reffile)):
                shutil.copyfile(self.datapath+reffile, reffile)
        default(tsdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        for reffile in self.reffiles:
            if (os.path.exists(reffile)):
                os.remove(reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def testEdgeMarkerGeneric0(self):
        """
        Test default setting for edgemarker
        """
        outname = self.prefix+'.asap'
        self.res = tsdcal(infile=self.rawfile,calmode='otf',markonly=True,outfile=outname,outform='ASAP')
        refdir = self._readref( self.reffiles[0] )
        self._checkfile( outname ) 
        self._checkmarker( outname, refdir )

    def testEdgeMarkerGeneric1(self):
        """
        Test customized edge marking
        """
        outname = self.prefix+'.asap'
        self.res = tsdcal(infile=self.rawfile,calmode='otf',fraction='3%',markonly=True,outfile=outname,outform='ASAP')
        refdir = self._readref( self.reffiles[1] )
        self._checkfile( outname ) 
        self._checkmarker( outname, refdir )


class sdcal_test_edgemarker_raster(sdcal_edgemarker_base,unittest.TestCase):
    """
    Test edgemarker function that is available for calmode='otfraster'. 

    Here, data will not be calibrated and only edge marking process
    will be executed. 
    """
    # Input and output names
    rawfile='raster.asap'
    prefix=sdcal_unittest_base.taskname+'TestEdgeMarkerRaster'
    reffiles=[ 'marker.raster.default.ref',
               'marker.raster.custom.ref' ]

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        for reffile in self.reffiles:
            if (not os.path.exists(reffile)):
                shutil.copyfile(self.datapath+reffile, reffile)
        default(tsdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        for reffile in self.reffiles:
            if (os.path.exists(reffile)):
                os.remove(reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def testEdgeMarkerRaster0(self):
        """
        Test default setting for edgemarker
        """
        outname = self.prefix+'.asap'
        self.res = tsdcal(infile=self.rawfile,calmode='otfraster',markonly=True,outfile=outname,outform='ASAP')
        refdir = self._readref( self.reffiles[0] )
        self._checkfile( outname ) 
        self._checkmarker( outname, refdir )

    def testEdgeMarkerRaster1(self):
        """
        Test default setting for edgemarker
        """
        outname = self.prefix+'.asap'
        self.res = tsdcal(infile=self.rawfile,calmode='otfraster',noff=1,markonly=True,outfile=outname,outform='ASAP')
        refdir = self._readref( self.reffiles[1] )
        self._checkfile( outname ) 
        self._checkmarker( outname, refdir )

class sdcal_test_selection(sdcal_caltest_base,unittest.TestCase):
    """
    
    Data is taken from OrionS_rawACSmod and created by the following
    script:
    scan = sd.scantable('OrionS_rawACSmod', average=False)
    sel.set_cycles([0,4])
    scan.save('selection.asap')
    sdcal(infile='selecttion.asap', calmode='ps',outfile='selection.cal.asap')
    """
    # Input and output names
    rawfile='selection.asap'
    reffile='selection.cal.asap'
    prefix=sdcal_unittest_base.taskname+'TestSel'
    calmode='ps'
    field_prefix = 'OrionS__'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(tsdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def testSel000(self):
        """ test default selection (use whole data)"""
        outname=self.prefix+self.postfix
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

    def testSel101(self):
        """ test spw selection (spw='1')"""
        outname=self.prefix+self.postfix
        spw = '1'
        self.res=tsdcal(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'IFNO': [1]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def testSel102(self):
        """ test spw selection (spw='1,3')"""
        outname=self.prefix+self.postfix
        spw = '1,3'
        self.res=tsdcal(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'IFNO': [1,3]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def testSel103(self):
        """ test spw selection (spw='<3')"""
        outname=self.prefix+self.postfix
        spw = '<3'
        self.res=tsdcal(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'IFNO': [0,1,2]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

#     def testSel104(self):
#         """ test spw selection (spw='<=2')"""
#         outname=self.prefix+self.postfix
#         spw = '<=2'
#         self.res=tsdcal(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
#         tbsel = {'IFNO': [0,1,2]}
#         self.assertEqual(self.res,None,
#                          msg='Any error occurred during calibration')
#         self._comparecal_with_selection(outname, tbsel)

    def testSel105(self):
        """ test spw selection (spw='2~13')"""
        outname=self.prefix+self.postfix
        spw = '2~13'
        self.res=tsdcal(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'IFNO': [2,3,12,13]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def testSel106(self):
        """ test spw selection (spw='>13')"""
        outname=self.prefix+self.postfix
        spw = '>13'
        self.res=tsdcal(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'IFNO': [14,15]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

#     def testSel107(self):
#         """ test spw selection (spw='>=13')"""
#         outname=self.prefix+self.postfix
#         spw = '>=13'
#         self.res=tsdcal(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
#         tbsel = {'IFNO': [13,14,15]}
#         self.assertEqual(self.res,None,
#                          msg='Any error occurred during calibration')
#         self._comparecal_with_selection(outname, tbsel)

    def testSel108(self):
        """ test spw selection (spw='<2 , 3~12 , 13, >14')"""
        outname=self.prefix+self.postfix
        spw = '<2 , 3~12 , 13, >14'
        self.res=tsdcal(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'IFNO': [0,1,3,12,13,15]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def testSel201(self):
        """ test pol selection (pol='1')"""
        outname=self.prefix+self.postfix
        pol = '1'
        self.res=tsdcal(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'POLNO': [1]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def testSel202(self):
        """ test pol selection (pol='0,1')"""
        outname=self.prefix+self.postfix
        pol = '0,1'
        self.res=tsdcal(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def testSel203(self):
        """ test pol selection (pol='<1')"""
        outname=self.prefix+self.postfix
        pol = '<1'
        self.res=tsdcal(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'POLNO': [0]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

#     def testSel204(self):
#         """ test pol selection (pol='<=0')"""
#         outname=self.prefix+self.postfix
#         pol = '<=0'
#         self.res=tsdcal(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname,outform='ASAP')
#         tbsel = {'POLNO': [0]}
#         self.assertEqual(self.res,None,
#                          msg='Any error occurred during calibration')
#         self._comparecal_with_selection(outname, tbsel)

    def testSel205(self):
        """ test pol selection (pol='0~1')"""
        outname=self.prefix+self.postfix
        pol = '0~1'
        self.res=tsdcal(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def testSel206(self):
        """ test pol selection (pol='>0')"""
        outname=self.prefix+self.postfix
        pol = '>0'
        self.res=tsdcal(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'POLNO': [1]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

#     def testSel207(self):
#         """ test pol selection (pol='>=1')"""
#         outname=self.prefix+self.postfix
#         pol = '>=1'
#         self.res=tsdcal(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname,outform='ASAP')
#         tbsel = {'POLNO': [1]}
#         self.assertEqual(self.res,None,
#                          msg='Any error occurred during calibration')
#         self._comparecal_with_selection(outname, tbsel)

#     def testSel301(self):
#         """ test scan selection (scan='1')"""
#         outname=self.prefix+self.postfix
#         scan = '1'
#         self.res=tsdcal(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname,outform='ASAP')
#         tbsel = {'SCANNO': [1]}
#         self.assertEqual(self.res,None,
#                          msg='Any error occurred during calibration')
#         self._comparecal_with_selection(outname, tbsel)

    def testSel302(self):
        """ test scan selection (scan='21,22')"""
        outname=self.prefix+self.postfix
        scan = '21,22'
        self.res=tsdcal(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'SCANNO': [22]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def testSel303(self):
        """ test scan selection (scan='<23')"""
        outname=self.prefix+self.postfix
        scan = '<23'
        self.res=tsdcal(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'SCANNO': [22]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

#     def testSel304(self):
#         """ test scan selection (scan='<=22')"""
#         outname=self.prefix+self.postfix
#         scan = '<=22'
#         self.res=tsdcal(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname,outform='ASAP')
#         tbsel = {'SCANNO': [22]}
#         self.assertEqual(self.res,None,
#                          msg='Any error occurred during calibration')
#         self._comparecal_with_selection(outname, tbsel)

    def testSel305(self):
        """ test scan selection (scan='23~26')"""
        outname=self.prefix+self.postfix
        scan = '23~26'
        self.res=tsdcal(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'SCANNO': [24,26]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def testSel306(self):
        """ test scan selection (scan='>26')"""
        outname=self.prefix+self.postfix
        scan = '>26'
        self.res=tsdcal(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'SCANNO': [28]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

#     def testSel307(self):
#         """ test scan selection (scan='>=27')"""
#         outname=self.prefix+self.postfix
#         scan = '>=27'
#         self.res=tsdcal(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname,outform='ASAP')
#         tbsel = {'SCANNO': [28]}
#         self.assertEqual(self.res,None,
#                          msg='Any error occurred during calibration')
#         self._comparecal_with_selection(outname, tbsel)

    def testSel308(self):
        """ test scan selection (scan='<23, 25~26, 27, 28')"""
        outname=self.prefix+self.postfix
        scan = '<23, 25~26, 27, 28'
        self.res=tsdcal(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'SCANNO': [22, 26, 28]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

#     def testSel401(self):
#         """ test field selection (field='1')"""
#         outname=self.prefix+self.postfix
#         field = '1'
#         self.res=tsdcal(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
#         tbsel = {'IFNO': [1]}
#         self.assertEqual(self.res,None,
#                          msg='Any error occurred during calibration')
#         self._comparecal_with_selection(outname, tbsel)

    def testSel402(self):
        """ test field selection (field='0,1')"""
        outname=self.prefix+self.postfix
        field = '0,1'
        self.res=tsdcal(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
        fieldid = [1]
        tbsel = {'FIELDNAME': [self.field_prefix+str(idx) for idx in fieldid]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def testSel403(self):
        """ test field selection (field='<2')"""
        outname=self.prefix+self.postfix
        field = '<2'
        self.res=tsdcal(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'FIELDNAME': [self.field_prefix+str(1)]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

#     def testSel404(self):
#         """ test field selection (field='<=1')"""
#         outname=self.prefix+self.postfix
#         field = '<=1'
#         self.res=tsdcal(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
#         tbsel = {'FIELDNAME': [self.field_prefix+str(1)]}
#         self.assertEqual(self.res,None,
#                          msg='Any error occurred during calibration')
#         self._comparecal_with_selection(outname, tbsel)

    def testSel405(self):
        """ test field selection (field='2~3')"""
        outname=self.prefix+self.postfix
        field = '2~3'
        self.res=tsdcal(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'FIELDNAME': [self.field_prefix+str(3)]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def testSel406(self):
        """ test field selection (field='>1')"""
        outname=self.prefix+self.postfix
        field = '>1'
        self.res=tsdcal(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'FIELDNAME': [self.field_prefix+str(3)]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

#     def testSel407(self):
#         """ test field selection (field='>=2')"""
#         outname=self.prefix+self.postfix
#         field = '>=2'
#         self.res=tsdcal(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
#         tbsel = {'FIELDNAME': [self.field_prefix+str(3)]}
#         self.assertEqual(self.res,None,
#                          msg='Any error occurred during calibration')
#         self._comparecal_with_selection(outname, tbsel)

    def testSel408(self):
        """ test field selection (field='Ori*')"""
        outname=self.prefix+self.postfix
        field = 'Ori*'
        self.res=tsdcal(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def testSel500(self):
        """ test field, spw, scan and pol selection (field='2,3', spw='14', scan='27,28', pol='1')"""
        outname=self.prefix+self.postfix
        field = '2,3'
        spw = '14'
        scan='27, 28'
        pol='1'
        self.res=tsdcal(infile=self.rawfile,field=field,spw=spw,scan=scan,pol=pol,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'FIELDNAME': [self.field_prefix+str(3)],
                 'IFNO': [14], 'SCANNO': [28], 'POLNO': [1]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def _comparecal_with_selection( self, name, tbsel={} ):
        self._checkfile(name)
        sp=self._getspectra(name)
        spref=self._getspectra_selected(self.reffile, tbsel)

        self._checkshape( sp, spref )
        
        for irow in xrange(sp.shape[0]):
            diff=self._diff(sp[irow],spref[irow])
            retval=numpy.all(diff<0.01)
            maxdiff=diff.max()
            self.assertEqual( retval, True,
                             msg='calibrated result is wrong (irow=%s): maxdiff=%s'%(irow,diff.max()) )
        del sp, spref

    def _getspectra_selected( self, name, tbsel={} ):
        """
        Returns an array of spectra in rows selected in table.
        
        name  : the name of scantable
        tbsel : a dictionary of table selection information.
                The key should be column name and the value should be
                a list of column values to select.
        """
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='file %s does not exist'%(name))        
        tb.open(name)
        if len(tbsel) == 0:
            sp=tb.getcol('SPECTRA').transpose()
        else:
            command = ''
            for key, val in tbsel.items():
                if len(command) > 0:
                    command += ' AND '
                command += ('%s in %s' % (key, str(val)))
            newtb = tb.query(command)
            sp=newtb.getcol('SPECTRA').transpose()
            newtb.close()

        tb.close()
        return sp



def suite():
    return [sdcal_test0, sdcal_test1,
            sdcal_test2, sdcal_test3,
            sdcal_test4, sdcal_test5,
            sdcal_test_edgemarker_generic,
            sdcal_test_edgemarker_raster,
            sdcal_test_selection]
