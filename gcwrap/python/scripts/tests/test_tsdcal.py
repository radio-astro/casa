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

try:
    import selection_syntax
except:
    import tests.selection_syntax as selection_syntax

# to rethrow exception 
import inspect
g = sys._getframe(len(inspect.stack())-1).f_globals
g['__rethrow_casa_exceptions'] = True
from tsdcal import tsdcal
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
        self.res=tsdcal()
        self.assertFalse(self.res)
        
    def test001(self):
        """Test 001: Invalid calibration mode"""
        # argument verification error
        self.res=tsdcal(infile=self.rawfile,calmode='invalid',outfile=self.outfile)
        self.assertFalse(self.res)

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

class sdcal_test_selection(selection_syntax.SelectionSyntaxTest,
                               sdcal_caltest_base,unittest.TestCase):
    """
    Test selection syntax. Selection parameters to test are:
    field, spw (no channel selection), scan, pol

    Data is taken from sd_analytic_type1-3.asap and TSYS column is filled
    by the following script:
    poly = ( (1.0, ), (0.2, 0.02), (2.44, -0.048, 0.0004),
           (-3.096, 0.1536, -0.00192, 8.0e-6) )
    shutil.copytree(os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/singledish/sd_analytic_type1-3.asap','sd_analytic_type1-3.filltsys.asap')
    tb.open('sd_analytic_type1-3.filltsys.asap', nomodify=False)
    subt = tb.query('SRCTYPE==0')
    for irow in range(subt.nrows()):
        nchan = len(subt.getcell('SPECTRA', irow))
        x = numpy.array(range(nchan))
        tsys = numpy.zeros(nchan)
        coeffs = poly[irow]
        for idim in range(len(coeffs)):
            tsys += coeffs[idim]*x**idim
        subt.putcell('TSYS',irow, tsys)
    subt.flush()
    tb.flush()
    subt.close()
    tb.close()
    """
    # Input and output names
    rawfile='sd_analytic_type1-3.filltsys.asap'
    prefix=sdcal_unittest_base.taskname+'TestSel'
    calmode='ps'
    line = ({'value': 5,  'channel': (20,20)},
            {'value': 10, 'channel': (40,40)},
            {'value': 20, 'channel': (60,60)},
            {'value': 30, 'channel': (80,80)},)
    baseline = ( (1.0, ), (0.2, 0.02), (2.44, -0.048, 0.0004),
                (-3.096, 0.1536, -0.00192, 8.0e-6) )
    
    @property
    def task(self):
        return tsdcal
    
    @property
    def spw_channel_selection(self):
        return False

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

        default(tsdcal)
        self.outname=self.prefix+self.postfix
        
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
        """test scan selection (scan='')"""
        scan = ''
        ref_idx = []
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,scan=scan,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_scan_id_exact(self):
        """ test scan selection (scan='16')"""
        scan = '16'
        ref_idx = [1, 2]
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_scan_id_lt(self):
        """ test scan selection (scan='<16')"""
        scan = '<16'
        ref_idx = [0]
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_scan_id_gt(self):
        """ test scan selection (scan='>16')"""
        scan = '>16'
        ref_idx = [3]
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_scan_id_range(self):
        """ test scan selection (scan='16~17')"""
        scan = '16~17'
        ref_idx = [1,2,3]
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_scan_id_list(self):
        """ test scan selection (scan='15,17')"""
        scan = '15,17'
        ref_idx = [0,3]
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_scan_id_exprlist(self):
        """ test scan selection (scan='15,>16')"""
        scan = '15,>16'
        ref_idx = [0,3]
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    ####################
    # pol
    ####################
    def test_pol_id_default(self):
        """test pol selection (pol='')"""
        pol = ''
        ref_idx = []
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,pol=pol,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_pol_id_exact(self):
        """ test pol selection (pol='1')"""
        pol = '1'
        ref_idx = [1,3]
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_pol_id_lt(self):
        """ test pol selection (pol='<1')"""
        pol = '<1'
        ref_idx = [0,2]
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_pol_id_gt(self):
        """ test pol selection (pol='>0')"""
        pol = '>0'
        ref_idx = [1,3]
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_pol_id_range(self):
        """ test pol selection (pol='0~1')"""
        pol = '0~1'
        ref_idx = []
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_pol_id_list(self):
        """ test pol selection (pol='0,1')"""
        pol = '0,1'
        ref_idx = []
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_pol_id_exprlist(self):
        """test pol selection (pol='0,>0')"""
        pol = '0,>0'
        ref_idx = []
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    ####################
    # field
    ####################
    def test_field_value_default(self):
        """test field selection (field='')"""
        field = ''
        ref_idx = []
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,field=field,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_field_id_exact(self):
        """ test field selection (field='6')"""
        field = '6'
        ref_idx = [1]
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_field_id_lt(self):
        """ test field selection (field='<6')"""
        field = '<6'
        ref_idx = [0]
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_field_id_gt(self):
        """ test field selection (field='>7')"""
        field = '>7'
        ref_idx = [3]
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_field_id_range(self):
        """ test field selection (field='6~8')"""
        field = '6~8'
        ref_idx = [1,2,3]
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_field_id_list(self):
        """ test field selection (field='5,8')"""
        field = '5,8'
        ref_idx = [0,3]
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_field_id_exprlist(self):
        """ test field selection (field='5,>7')"""
        field = '5,>7'
        ref_idx = [0,3]
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_field_value_exact(self):
        """ test field selection (field='M30')"""
        field = 'M30'
        ref_idx = [2]
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_field_value_pattern(self):
        """ test field selection (field='M*')"""
        field = 'M*'
        ref_idx = [0,1,2]
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_field_value_list(self):
        """ test field selection (field='3C273,M30')"""
        field = '3C273,M30'
        ref_idx = [2,3]
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_field_mix_exprlist(self):
        """ test field selection (field='<6,3*')"""
        field = '<6,3*'
        ref_idx = [0,3]
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    ####################
    # spw 
    ####################
    def test_spw_id_default(self):
        """test spw selection (spw='')"""
        spw = ''
        ref_idx = []
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,spw=spw,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_spw_id_exact(self):
        """ test spw selection (spw='23')"""
        spw = '23'
        ref_idx = [0,3]
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)


    def test_spw_id_lt(self):
        """ test spw selection (spw='<23')"""
        spw = '<23'
        ref_idx = [2]
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)


    def test_spw_id_gt(self):
        """ test spw selection (spw='>23')"""
        spw = '>23'
        ref_idx = [1]
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_spw_id_range(self):
        """ test spw selection (spw='23~25')"""
        spw = '23~25'
        ref_idx = [0,1,3]
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_spw_id_list(self):
        """ test spw selection (spw='21,25')"""
        spw = '21,25'
        ref_idx = [1,2]
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)


    def test_spw_id_exprlist(self):
        """ test spw selection (spw='23,>24')"""
        spw = '23,>24'
        ref_idx = [0,1,3]
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)


    def test_spw_id_pattern(self):
        """test spw selection (spw='*')"""
        spw='*'
        ref_idx = []
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,spw=spw,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_spw_value_frequency(self):
        """test spw selection (spw='300.4~300.6GHz')"""
        spw = '300.4~300.6GHz' # IFNO=25 should be selected
        ref_idx = [1]
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_spw_value_velocity(self):
        """test spw selection (spw='-30~30km/s')"""
        spw = '-30~30km/s'  # IFNO=23 should be selected
        ref_idx = [0,3]
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    def test_spw_mix_exprlist(self):
        """test spw selection (spw='25,-30~30km/s')"""
        spw = '25,-30~30km/s' # IFNO=23,25 should be selected
        ref_idx = [0,1,3]
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=self.outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._compare_with_analytic(self.outname, self.line, self.baseline, ref_idx)

    ####################
    # Helper functions
    ####################
    def _compare_with_analytic(self, name, ref_line, ref_bl, ref_idx=[], precision = 1.e-6):
        self._checkfile( name )
        sout = sd.scantable(name,average=False)
        nrow = sout.nrow()
        if len(ref_idx) == 0:
            ref_idx = range(nrow)

        self.assertEqual(nrow,len(ref_idx),"The rows in output table differs from the expected value.")
        for irow in range(nrow):
            y = sout._getspectrum(irow)
            nchan = len(y)
            x = numpy.array( range(nchan) )
            # analytic solution
            coeff = ref_bl[ ref_idx[irow] ]
            yana = self._create_ploynomial_array(coeff, x)
            curr_line = ref_line[ ref_idx[irow] ]
            chanlist = curr_line['channel']
            valuelist = curr_line['value']
            yana += self._create_tophat_array(nchan, chanlist, valuelist)

            # compare
            rdiff = self._get_array_relative_diff(y,yana)
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

    def _create_ploynomial_array(self, coeff, x):
        """ Create an array from a list of polynomial coefficients and x-array"""
        xarr = numpy.array(x)
        yarr = numpy.zeros(len(xarr))
        for idim in range(len(coeff)):
            ai = coeff[idim]
            yarr += ai*xarr**idim
        return yarr

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


def suite():
    return [sdcal_test0, sdcal_test1,
            sdcal_test2, sdcal_test3,
            sdcal_test4, sdcal_test5,
            sdcal_test_edgemarker_generic,
            sdcal_test_edgemarker_raster,
            sdcal_test_selection]
