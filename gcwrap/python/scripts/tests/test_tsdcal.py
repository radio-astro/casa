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
    
    Data is taken from OrionS_rawACSmod and created by the following
    script:
    scan = sd.scantable('OrionS_rawACSmod', average=False)
    sel=sd.selector(cycles=[0, 4])
    scan.set_selection(sel)
    scan.save('selection.mod.asap')
    tb.open('selection.mod.asap', nomodify=False)
    # modify SCANNO --> [30, 31, 32, 33]
    scanmap = {21: 30, 22: 30, 23: 31, 24: 31, 25: 32, 26: 32, 27: 33, 28: 33}
    scancol = tb.getcol('SCANNO')
    for irow in range(len(scancol)): scancol[irow] = scanmap[ scancol[irow] ]
    tb.putcol('SCANNO', scancol)
    # modify FIELD and SOURCE NAME
    for sno in [30, 31, 32, 33]:
        subtb = tb.query('SCANNO==%d' % sno)
        newname = 'ptg%d' % (sno % 10)
        srccol = subtb.getcol('SRCNAME')
        newsrc = [ name+newname for name in srccol ]
        subtb.putcol('SRCNAME', newsrc)
        fldcol = subtb.getcol('FIELDNAME')
        newfld = [ name.replace('0', '1').replace('2','3') for name in fldcol ]
        subtb.putcol('FIELDNAME', newfld)
        subtb.flush()
        subtb.close()
    tb.flush()
    tb.close()
    sdcal(infile='selection.mod.asap', calmode='ps',outfile='selection.mod.cal.asap')
    """
    # Input and output names
    rawfile='selection.mod.asap'
    reffile='selection.mod.cal.asap'
    prefix=sdcal_unittest_base.taskname+'TestSel'
    calmode='ps'
    field_prefix = 'OrionS__'

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
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(tsdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    ####################
    # Additional tests
    ####################
    def test_all(self):
        """ test default selection (use whole data)"""
        outname=self.prefix+self.postfix
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

    def test_mixed_selection(self):
        """ test field, spw, scan and pol selection (field='3', spw='14,15', scan='33', pol='1')"""
        outname=self.prefix+self.postfix
        field = '3'
        spw = '14,15'
        scan='33'
        pol='1'
        self.res=tsdcal(infile=self.rawfile,field=field,spw=spw,scan=scan,pol=pol,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'FIELDNAME': [self.field_prefix+str(3)],
                 'IFNO': [14,15], 'SCANNO': [33], 'POLNO': [1]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    ####################
    # scan
    ####################
    def test_scan_id_default(self):
        """test scan selection (scan='')"""
        outname=self.prefix+self.postfix
        scan=''
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,scan=scan,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

    def test_scan_id_exact(self):
        """ test scan selection (scan='31')"""
        outname=self.prefix+self.postfix
        scan = '31'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'SCANNO': [31]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_lt(self):
        """ test scan selection (scan='<31')"""
        outname=self.prefix+self.postfix
        scan = '<31'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'SCANNO': [30]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_gt(self):
        """ test scan selection (scan='>31')"""
        outname=self.prefix+self.postfix
        scan = '>31'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'SCANNO': [32,33]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_range(self):
        """ test scan selection (scan='31~32')"""
        outname=self.prefix+self.postfix
        scan = '31~32'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'SCANNO': [31,32]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_list(self):
        """ test scan selection (scan='30,32')"""
        outname=self.prefix+self.postfix
        scan = '30,32'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'SCANNO': [30,32]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_exprlist(self):
        """ test scan selection (scan='<32, 33')"""
        outname=self.prefix+self.postfix
        scan = '<32, 33'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'SCANNO': [30,31,33]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    ####################
    # pol
    ####################
    def test_pol_id_default(self):
        """test pol selection (pol='')"""
        outname=self.prefix+self.postfix
        pol=''
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,pol=pol,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

    def test_pol_id_exact(self):
        """ test pol selection (pol='1')"""
        outname=self.prefix+self.postfix
        pol = '1'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'POLNO': [1]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_pol_id_lt(self):
        """ test pol selection (pol='<1')"""
        outname=self.prefix+self.postfix
        pol = '<1'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'POLNO': [0]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_pol_id_gt(self):
        """ test pol selection (pol='>0')"""
        outname=self.prefix+self.postfix
        pol = '>0'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'POLNO': [1]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_pol_id_range(self):
        """ test pol selection (pol='0~1')"""
        outname=self.prefix+self.postfix
        pol = '0~1'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_pol_id_list(self):
        """ test pol selection (pol='0,1')"""
        outname=self.prefix+self.postfix
        pol = '0,1'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_pol_id_exprlist(self):
        """test pol selection (pol='0,>0')"""
        outname=self.prefix+self.postfix
        pol = '0,>0'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    ####################
    # field
    ####################
    def test_field_value_default(self):
        """test field selection (field='')"""
        outname=self.prefix+self.postfix
        field=''
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,field=field,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

    def test_field_id_exact(self):
        """ test field selection (field='1')"""
        outname=self.prefix+self.postfix
        field = '1'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'FIELDNAME': [self.field_prefix+str(1)]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_id_lt(self):
        """ test field selection (field='<2')"""
        outname=self.prefix+self.postfix
        field = '<2'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'FIELDNAME': [self.field_prefix+str(1)]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_id_gt(self):
        """ test field selection (field='>1')"""
        outname=self.prefix+self.postfix
        field = '>1'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'FIELDNAME': [self.field_prefix+str(3)]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_id_range(self):
        """ test field selection (field='1~3')"""
        outname=self.prefix+self.postfix
        field = '1~3'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'FIELDNAME': [self.field_prefix+str(1),self.field_prefix+str(3)]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_id_list(self):
        """ test field selection (field='1,3')"""
        outname=self.prefix+self.postfix
        field = '1,3'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
        fieldid = [1,3]
        tbsel = {'FIELDNAME': [self.field_prefix+str(idx) for idx in fieldid]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_id_exprlist(self):
        """ test field selection (field='<2,3')"""
        outname=self.prefix+self.postfix
        field = '<2,3'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
        fieldid = [1,3]
        tbsel = {'FIELDNAME': [self.field_prefix+str(idx) for idx in fieldid]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_value_exact(self):
        """ test field selection (field='OrionSptg1')"""
        outname=self.prefix+self.postfix
        field = 'OrionSptg1'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'SRCNAME': ['OrionSptg1']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_value_pattern(self):
        """ test field selection (field='Ori*')"""
        outname=self.prefix+self.postfix
        field = 'Ori*'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_value_list(self):
        """ test field selection (field='OrionSptg1,OrionSptg3')"""
        outname=self.prefix+self.postfix
        field = 'OrionSptg1,OrionSptg3'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'SRCNAME': ['OrionSptg1', 'OrionSptg3']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_mix_exprlist(self):
        """ test field selection (field='OrionSptg0,>2')"""
        outname=self.prefix+self.postfix
        field = 'OrionSptg0,>2'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'SCANNO': [30,32,33]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    ####################
    # spw 
    ####################
    def test_spw_id_default(self):
        """test spw selection (spw='')"""
        outname=self.prefix+self.postfix
        spw=''
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,spw=spw,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

    def test_spw_id_exact(self):
        """ test spw selection (spw='1')"""
        outname=self.prefix+self.postfix
        spw = '1'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'IFNO': [1]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)


    def test_spw_id_lt(self):
        """ test spw selection (spw='<3')"""
        outname=self.prefix+self.postfix
        spw = '<3'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'IFNO': [0,1,2]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)


    def test_spw_id_gt(self):
        """ test spw selection (spw='>13')"""
        outname=self.prefix+self.postfix
        spw = '>13'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'IFNO': [14,15]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_id_range(self):
        """ test spw selection (spw='2~13')"""
        outname=self.prefix+self.postfix
        spw = '2~13'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'IFNO': [2,3,12,13]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_id_list(self):
        """ test spw selection (spw='1,3')"""
        outname=self.prefix+self.postfix
        spw = '1,3'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'IFNO': [1,3]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)


    def test_spw_id_exprlist(self):
        """ test spw selection (spw='<2 , 3~12 , 13, >14')"""
        outname=self.prefix+self.postfix
        spw = '<2 , 3~12 , 13, >14'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'IFNO': [0,1,3,12,13,15]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)


    def test_spw_id_pattern(self):
        """test spw selection (spw='*')"""
        outname=self.prefix+self.postfix
        spw='*'
        self.res=tsdcal(infile=self.rawfile,calmode=self.calmode,spw=spw,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

    def test_spw_value_frequency(self):
        """test spw selection (spw='44.0~44.2GHz')"""
        outname=self.prefix+self.postfix
        spw = '44.0~44.2GHz'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'IFNO': [2,3]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_value_velocity(self):
        """test spw selection (spw='-10~10km/s')"""
        outname=self.prefix+self.postfix
        spw = '-10~10km/s'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'IFNO': [0,12]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_mix_exprlist(self):
        """test spw selection (spw='0,1300~3700km/s')"""
        outname=self.prefix+self.postfix
        spw = '0,1300~3700km/s'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname,outform='ASAP')
        tbsel = {'IFNO': [0,1,15]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    ####################
    # Helper functions
    ####################
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
