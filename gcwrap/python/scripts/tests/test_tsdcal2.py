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
from tsdcal2 import tsdcal2
import asap as sd

#
# Unit test of sdcal task.
# 

###
# Base class for sdcal unit test
###
class tsdcal2_unittest_base:
    """
    Base class for sdcal unit test
    """
    taskname='tsdcal2'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdcal2/'
    tolerance=1.0e-15

    def _checkfile(self, name):
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
class tsdcal2_caltest_base(tsdcal2_unittest_base):
    """
    Base class for calibration test
    """
    def _comparecal(self, out, ref, col='SPECTRA'):
        self._checkfile(out)
        tout = tbtool()
        tref = tbtool()
        try:
            tout.open(out)
            tref.open(ref)
            self.assertEqual(tout.nrows(), tref.nrows(),
                             msg='number of rows differ.')
            # check meta data
            meta = ['SCANNO','IFNO','POLNO','TIME','ELEVATION']
            for name in meta:
                vout = tout.getcol(name)
                vref = tref.getcol(name)
                diff = numpy.abs(vout - vref) / vref
                #print 'max difference: ',diff.max()
                #self.assertTrue(numpy.all(vout==vref),
                #                msg='column %s differ'%(name))
                self.assertTrue(numpy.all(diff < 1.0e-15),
                                          msg='column %s differ'%(name))

            # check calibration data
            for irow in xrange(tout.nrows()):
                sp = tout.getcell(col, irow)
                spref = tref.getcell(col, irow)
                diff=self._diff(sp,spref)
                self.assertTrue(numpy.all(diff < 0.01),
                                msg='calibrated result is wrong (irow=%s): maxdiff=%s'%(irow,diff.max()) )
        except Exception, e:
            raise e
        finally:
            tout.close()
            tref.close()

    def _compare(self, out, ref, tsys=False):
        self._checkfile(out)
        tout = tbtool()
        tref = tbtool()
        try:
            tout.open(out)
            tref.open(ref)
            self.assertEqual(tout.nrows(), tref.nrows(),
                             msg='number of rows differ.')
            # check SPECTRA
            col = 'SPECTRA'
            for irow in xrange(tout.nrows()):
                sp = tout.getcell(col, irow)
                spref = tref.getcell(col, irow)
                diff=self._diff(sp,spref)
                self.assertTrue(numpy.all(diff < 0.01),
                                msg='calibrated result is wrong (irow=%s): maxdiff=%s'%(irow,diff.max()) )

            # check Tsys if necessary
            if tsys:
                col = 'TSYS'
                for irow in xrange(tout.nrows()):
                    sp = tout.getcell(col, irow)
                    spref = tref.getcell(col, irow)
                    self.assertEqual(len(sp), len(spref),
                                     msg='Tsys is wrong (irow=%s): shape mismatch'%(irow))
                    diff=self._diff(sp,spref)
                    self.assertTrue(numpy.all(diff < 0.01),
                                    msg='Tsys is wrong (irow=%s): maxdiff=%s'%(irow,diff.max()) )
                
        except Exception, e:
            raise e
        finally:
            tout.close()
            tref.close()
        

###
# Test if the task raises exception properly
###
class tsdcal2_exceptions(tsdcal2_unittest_base,unittest.TestCase):
    """
    Test on bad parameter setting
    """
    # Input and output names
    rawfile='sdcal2Test.ps.asap'
    #prefix=tsdcal2_unittest_base.taskname+'Test.ps'
    prefix='sdcal2Test.ps'
    outfile=prefix+'.asap.out'
    skytable=prefix+'.sky'
    tsystable=prefix+'.tsys'
    ifmap={1:[5,6]}

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.skytable)):
            shutil.copytree(self.datapath+self.skytable, self.skytable)
        if (not os.path.exists(self.tsystable)):
            shutil.copytree(self.datapath+self.tsystable, self.tsystable)

        default(tsdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_exception00(self):
        """test_exception00: Default parameters"""
        # argument verification error
        self.res=tsdcal2()
        self.assertFalse(self.res)
        """
        try:
            self.res=tsdcal2()
            #self.assertFalse(self.res)
            self.fail("The task must throw exception.")
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        """
        
    def test_exception01(self):
        """test_exception01: apply calibration without skytable"""
        try:
            self.res=tsdcal2(infile=self.rawfile,calmode='apply')
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Name of the apply table must be given.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test_exception02(self):
        """test_exception02: invalid interp string"""
        interp='invalid_interpolation'
        try:
            self.res=tsdcal2(infile=self.rawfile,calmode='apply',applytable=[self.skytable,self.tsystable],interp=interp,ifmap=self.ifmap)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Interpolation type \'%s\' is invalid or not supported yet.'%(interp))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test_exception03(self):
        """test_exception03: Invalid calibration mode"""
        # argument verification error
        self.res=tsdcal2(infile=self.rawfile,calmode='invalid',outfile=self.outfile)
        self.assertFalse(self.res)
        """
        try:
            self.res=tsdcal2(infile=self.rawfile,calmode='invalid',outfile=self.outfile)
            #self.assertFalse(self.res)
            self.fail("The task must throw exception.")
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        """

    def test_exception04(self):
        """test_exception04: Existing outfile with overwrite=False"""
        if (not os.path.exists(self.outfile)):
            shutil.copytree(self.rawfile, self.outfile)
        try:
            self.res=tsdcal2(infile=self.rawfile,calmode='ps',outfile=self.outfile,overwrite=False)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Output file \'%s\' exists.'%(self.outfile))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test_exception06(self):
        """test_exception06: Non-existing sky table"""
        dummytable='dummy.sky'
        try:
            self.res=tsdcal2(infile=self.rawfile,calmode='apply',ifmap=self.ifmap,applytable=[dummytable,self.tsystable],outfile=self.outfile)
            #self.assertFalse(self.res)
            self.fail("The task must throw exception.")
        except Exception, e:
            pos=str(e).find("Apply table \'%s\' does not exist."%(dummytable))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test_exception07(self):
        """test_exception07: Non-existing tsys table"""
        dummytable='dummy.tsys'
        try:
            self.res=tsdcal2(infile=self.rawfile,calmode='apply',ifmap=self.ifmap,applytable=[self.skytable,dummytable],outfile=self.outfile)
            #self.assertFalse(self.res)
            self.fail("The task must throw exception.")
        except Exception, e:
            pos=str(e).find("Apply table \'%s\' does not exist."%(dummytable))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test_exception08(self):
        """test_exception08: Invalid interp string format (more than one comma)"""
        interp='linear,linear,linear'
        try:
            self.res=tsdcal2(infile=self.rawfile,calmode='apply',applytable=[self.skytable,self.tsystable],interp=interp,ifmap=self.ifmap)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Invalid format of the parameter interp: \'%s\''%(interp))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test_exception09(self):
        """test_exception09: Update infile without setting overwrite to False"""
        try:
            self.res=tsdcal2(infile=self.rawfile,calmode='apply',applytable=[self.skytable,self.tsystable],ifmap=self.ifmap)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('You should set overwrite to True if you want to update infile.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

#    def test_exception10(self):
#        """test_exception10: Empty iflist for Tsys calibration"""
#        try:
#            self.res=tsdcal2(infile=self.rawfile,calmode='tsys',tsysspw=[],outfile=self.outfile)
#            self.assertTrue(False,
#                            msg='The task must throw exception')
#        except Exception, e:
#            pos=str(e).find('You must specify iflist as a list of IFNOs for Tsys calibration.')
#            self.assertNotEqual(pos,-1,
#                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test_exception11(self):
        """test_exception11: non-scantable input"""
        try:
            self.res=tsdcal2(infile=self.skytable,calmode='ps',outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('infile must be in scantable format.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

###
# Test sky calibration (calmode='ps')
###
class tsdcal2_skycal_ps(tsdcal2_caltest_base,unittest.TestCase):
    """
    Test sky calibration (calmode='ps')
    """
    # Input and output names
    rawfile='sdcal2Test.ps.asap'
    #prefix=tsdcal2_unittest_base.taskname+'Test.ps'
    prefix='sdcal2Test.ps'
    outfile=prefix+'.sky.out'
    skytable=prefix+'.sky'
    calmode='ps'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.skytable)):
            shutil.copytree(self.datapath+self.skytable, self.skytable)

        default(tsdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_skycal_ps00(self):
        """test_skycal_ps00: Sky calibration for calmode='ps' (ALMA)"""
        tsdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.outfile)

        self._comparecal(self.outfile, self.skytable)
        
    def test_skycal_ps01(self):
        """test_skycal_ps01: Sky calibration for calmode='ps' (ALMA), overwrite existing table"""
        if (not os.path.exists(self.skytable)):
            shutil.copytree(self.rawfile, self.outfile)
        tsdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.outfile,overwrite=True)

        self._comparecal(self.outfile, self.skytable)

    def test_skycal_ps02(self):
        """test_skycal_ps02: Default outfile name for sky calibration."""
        tsdcal2(infile=self.rawfile,calmode=self.calmode,outfile='',overwrite=True)
        defaultname = self.rawfile.rstrip('/')+'_sky'
        self.assertTrue(os.path.exists(defaultname),
                        msg='Failed to generate default outfile name.')
        self._comparecal(defaultname, self.skytable)
        
###
# Test sky calibration (calmode='otf')
###
class tsdcal2_skycal_otf(tsdcal2_caltest_base,unittest.TestCase):
    """
    Test sky calibration (calmode='otf')
    """
    # Input and output names
    rawfile='lissajous.asap'
    prefix='lissajous'
    skytable=prefix+'.sky.out'
    outfile=prefix+'.asap.out'
    skytable_ref=prefix+'.sky'
    outfile_ref=prefix+'.asap_cal2'
    calmode='otf'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.outfile)):
            shutil.copytree(self.datapath+self.outfile_ref, self.outfile_ref)
        if (not os.path.exists(self.skytable_ref)):
            shutil.copytree(self.datapath+self.skytable_ref, self.skytable_ref)

        default(tsdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_skycal_otf00(self):
        """test_skycal_otf00: Sky calibration for calmode='otf' (ALMA)"""
        tsdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.skytable)

        self._comparecal(self.skytable, self.skytable_ref)
        
    def test_skycal_otf01(self):
        """test_skycal_otf01: Sky calibration for calmode='otf' (ALMA), overwrite existing table"""
        if (not os.path.exists(self.skytable_ref)):
            shutil.copytree(self.rawfile, self.skytable)
        tsdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.skytable,overwrite=True)

        self._comparecal(self.skytable, self.skytable_ref)

    def test_skycal_otf02(self):
        """test_skycal_otf02: Sky calibration for calmode='otf' (ALMA), apply existing sky table"""
        self.calmode='apply'
        tsdcal2(infile=self.rawfile,calmode=self.calmode,applytable=self.skytable_ref,outfile=self.outfile)

        self._compare(self.outfile, self.outfile_ref, False)

    def test_skycal_otf03(self):
        """test_skycal_otf03: Sky calibration with calmode='otf' (ALMA), apply on-the-fly"""
        self.calmode='otf,apply'
        tsdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.outfile)

        self._compare(self.outfile, self.outfile_ref, False)
       
###
# Test sky calibration (calmode='otfraster')
###
class tsdcal2_skycal_otfraster(tsdcal2_caltest_base,unittest.TestCase):
    """
    Test sky calibration (calmode='otfraster')
    """
    # Input and output names
    rawfile='raster.asap'
    prefix='raster'
    skytable=prefix+'.sky.out'
    outfile=prefix+'.asap.out'
    skytable_ref=prefix+'.sky'
    outfile_ref=prefix+'.asap_cal2'
    calmode='otfraster'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.outfile)):
            shutil.copytree(self.datapath+self.outfile_ref, self.outfile_ref)
        if (not os.path.exists(self.skytable_ref)):
            shutil.copytree(self.datapath+self.skytable_ref, self.skytable_ref)

        default(tsdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_skycal_otfraster00(self):
        """test_skycal_otfraster00: Sky calibration for calmode='otfraster' (ALMA)"""
        tsdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.outfile)

        self._comparecal(self.outfile, self.skytable_ref)
        
    def test_skycal_otfraster01(self):
        """test_skycal_otfraster01: Sky calibration for calmode='otfraster' (ALMA), overwrite existing table"""
        if (not os.path.exists(self.skytable_ref)):
            shutil.copytree(self.rawfile, self.outfile)
        tsdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.outfile,overwrite=True)

        self._comparecal(self.outfile, self.skytable_ref)
        
    def test_skycal_otfraster02(self):
        """test_skycal_otfraster02: Sky calibration for calmode='otfraster' (ALMA), apply existing sky table"""
        self.calmode='apply'
        tsdcal2(infile=self.rawfile,calmode=self.calmode,applytable=self.skytable_ref,outfile=self.outfile)

        self._compare(self.outfile, self.outfile_ref, False)

    def test_skycal_otfraster03(self):
        """test_skycal_otfraster03: Sky calibration with calmode='otfraster' (ALMA), apply on-the-fly"""
        self.calmode='otfraster,apply'
        tsdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.outfile)

        self._compare(self.outfile, self.outfile_ref, False)

###
# Test Tsys calibration (calmode='tsys')
###
class tsdcal2_tsyscal(tsdcal2_caltest_base,unittest.TestCase):
    """
    Test Tsys calibration
    """
    # Input and output names
    rawfile='sdcal2Test.ps.asap'
    #prefix=tsdcal2_unittest_base.taskname+'Test.ps'
    prefix='sdcal2Test.ps'
    outfile=prefix+'.tsys.out'
    tsystable=prefix+'.tsys'
    calmode='tsys'
    tsysspw='1'#[1]
    
    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.tsystable)):
            shutil.copytree(self.datapath+self.tsystable, self.tsystable)

        default(tsdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_tsyscal00(self):
        """test_tsyscal00: Tsys calibration"""
        tsdcal2(infile=self.rawfile,calmode=self.calmode,tsysspw=self.tsysspw,outfile=self.outfile)

        self._comparecal(self.outfile, self.tsystable, 'TSYS')
        
    def test_tsyscal01(self):
        """test_tsyscal01: Tsys calibration, overwrite existing table"""
        if (not os.path.exists(self.tsystable)):
            shutil.copytree(self.rawfile, self.outfile)
        tsdcal2(infile=self.rawfile,calmode=self.calmode,tsysspw=self.tsysspw,outfile=self.outfile,overwrite=True)

        self._comparecal(self.outfile, self.tsystable, 'TSYS')

    def test_tsyscal02(self):
        """test_tsyscal02: Default outfile name for Tsys calibration."""
        tsdcal2(infile=self.rawfile,calmode=self.calmode,tsysspw=self.tsysspw,outfile='',overwrite=True)
        defaultname = self.rawfile.rstrip('/')+'_tsys'
        self.assertTrue(os.path.exists(defaultname),
                        msg='Failed to generate default outfile name.')
        self._comparecal(defaultname, self.tsystable, 'TSYS')

    def test_tsyscal03(self):
        """test_tsyscal03: Auto-detect spws for atmcal"""
        tsdcal2(infile=self.rawfile,calmode=self.calmode,tsysspw=self.tsysspw,outfile=self.outfile)

        self._comparecal(self.outfile, self.tsystable, 'TSYS')


class tsdcal2_tsyscal_average(tsdcal2_caltest_base,unittest.TestCase):
    """
    Test Tsys calibration
    """
    # Input and output names
    rawfile='sdcal2Test.ps.asap.tsysavg'
    #prefix=tsdcal2_unittest_base.taskname+'Test.ps'
    prefix='sdcal2Test.ps'
    outfile=prefix+'.tsys.out'
    tsystable=prefix+'.tsysavg'
    calmode='tsys'
    tsysspw='1'#[1]
    
    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.tsystable)):
            shutil.copytree(self.datapath+self.tsystable, self.tsystable)

        default(tsdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def _comparecal_average(self, out, ref, col='SPECTRA', channelrange=[]):
        self._checkfile(out)
        tout = tbtool()
        tref = tbtool()
        try:
            tout.open(out)
            tref.open(ref)
            self.assertEqual(tout.nrows(), tref.nrows(),
                             msg='number of rows differ.')
            # check meta data
            meta = ['SCANNO','IFNO','POLNO','TIME','ELEVATION']
            for name in meta:
                vout = tout.getcol(name)
                vref = tref.getcol(name)
                diff = numpy.abs(vout - vref) / vref
                #print 'max difference: ',diff.max()
                #self.assertTrue(numpy.all(vout==vref),
                #                msg='column %s differ'%(name))
                self.assertTrue(numpy.all(diff < 1.0e-15),
                                          msg='column %s differ'%(name))

            # check calibration data
            for irow in xrange(tout.nrows()):
                sp = tout.getcell(col, irow)
                spref = tref.getcell(col, irow)
                #print 'sp =', sp
                #print 'spref =', spref
                averaged_value = 0.0
                nchan_averaged = 0
                if len(channelrange) == 0:
                    averaged_value = spref.mean()
                else:
                    for crange in channelrange:
                        segment = spref[crange[0]:crange[1]+1]
                        averaged_value += segment.sum()
                        nchan_averaged += len(segment)
                    averaged_value /= nchan_averaged
                #print 'averaged_value =', averaged_value
                diff=self._diff(sp,averaged_value)
                self.assertTrue(numpy.all(diff < 0.01),
                                msg='calibrated result is wrong (irow=%s): maxdiff=%s'%(irow,diff.max()) )
        except Exception, e:
            raise e
        finally:
            tout.close()
            tref.close()

    def test_tsyscal04(self):
        """test_tsyscal04: Averaging Tsys for full range"""
        tsdcal2(infile=self.rawfile,calmode=self.calmode,tsysavg=True,tsysspw=self.tsysspw,outfile=self.outfile)

        self._comparecal_average(self.outfile, self.tsystable, 'TSYS')

    def test_tsyscal05(self):
        """test_tsyscal05: Averaging Tsys for specified single range"""
        self.tsysspw = '1:0~128'
        channelrange = [[0,128]]
        tsdcal2(infile=self.rawfile,calmode=self.calmode,tsysavg=True,tsysspw=self.tsysspw,outfile=self.outfile)

        self._comparecal_average(self.outfile, self.tsystable, 'TSYS', channelrange)

    def test_tsyscal06(self):
        """test_tsyscal06: Averaging Tsys for specified multiple ranges"""
        self.tsysspw = '1:0~100;300~400'
        channelrange = [[0,100],[300,400]]
        tsdcal2(infile=self.rawfile,calmode=self.calmode,tsysavg=True,tsysspw=self.tsysspw,outfile=self.outfile)

        self._comparecal_average(self.outfile, self.tsystable, 'TSYS', channelrange)

    def test_tsyscal07(self):
        """test_tsyscal07: Averaging Tsys for specified single channel"""
        self.tsysspw = '1:0'
        channelrange = [[0,0]]
        tsdcal2(infile=self.rawfile,calmode=self.calmode,tsysavg=True,tsysspw=self.tsysspw,outfile=self.outfile)

        self._comparecal_average(self.outfile, self.tsystable, 'TSYS', channelrange)



###
# Test apply calibration
###
class tsdcal2_applycal(tsdcal2_caltest_base,unittest.TestCase):
    """
    Test apply calibration
    """
    # Input and output names
    rawfile='sdcal2Test.ps.asap'
    #prefix=tsdcal2_unittest_base.taskname+'Test.ps'
    prefix='sdcal2Test.ps'
    outfile=prefix+'.asap.out'
    skytable=prefix+'.sky'
    tsystable=prefix+'.tsys'
    reftables=[prefix+'.asap.ref',prefix+'.asap.noTsys.ref',prefix+'.asap.cspline.ref']
    calmode='apply'
    tsysspw='1'#[1]
    ifmap={1:[5,6]}
    
    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.skytable)):
            shutil.copytree(self.datapath+self.skytable, self.skytable)
        if (not os.path.exists(self.tsystable)):
            shutil.copytree(self.datapath+self.tsystable, self.tsystable)
        for ref in self.reftables:
            if (not os.path.exists(ref)):
                shutil.copytree(self.datapath+ref, ref)
                
        default(tsdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_applycal00(self):
        """test_applycal00: apply existing sky table and Tsys table"""
        tsdcal2(infile=self.rawfile,calmode=self.calmode,applytable=[self.skytable,self.tsystable],ifmap=self.ifmap,outfile=self.outfile)

        self._compare(self.outfile, self.reftables[0], True)
        
    def test_applycal01(self):
        """test_applycal01: apply existing skytable (ps)"""
        tsdcal2(infile=self.rawfile,calmode=self.calmode,applytable=self.skytable,ifmap=self.ifmap,outfile=self.outfile)

        self._compare(self.outfile, self.reftables[1], False)

    def test_applycal02(self):
        """test_applycal02: apply existing sky table and Tsys table with cubic spline interpolation along frequency axis"""
        tsdcal2(infile=self.rawfile,calmode=self.calmode,applytable=[self.skytable,self.tsystable],interp='linear,cspline',ifmap=self.ifmap,outfile=self.outfile)

        self._compare(self.outfile, self.reftables[2], True)

    def test_applycal03(self):
        """test_applycal03: test update mode (overwrite infile)"""
        tsdcal2(infile=self.rawfile,calmode=self.calmode,applytable=[self.skytable,self.tsystable],ifmap=self.ifmap,overwrite=True)

        tb.open(self.rawfile)
        tsel=tb.query('IFNO IN [5,6] && SRCTYPE==0')
        tcp = tsel.copy(self.outfile)
        tsel.close()
        tb.close()
        tcp.close()
        self._compare(self.outfile, self.reftables[0], True)

    def test_applycal04(self):
        """test_applycal04: calibrate sky and apply it on-the-fly"""
        self.calmode='ps,apply'
        tsdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.outfile)

        self._compare(self.outfile, self.reftables[1], False)

    def test_applycal05(self):
        """test_applycal05: calibrate sky and apply it on-the-fly with existing Tsys table"""
        self.calmode='ps,apply'
        tsdcal2(infile=self.rawfile,calmode=self.calmode,ifmap=self.ifmap,applytable=self.tsystable,outfile=self.outfile)

        self._compare(self.outfile, self.reftables[0], 'TSYS')

    def test_applycal06(self):
        """test_applycal06: calibrate sky as well as Tsys and apply them on-the-fly"""
        self.calmode='ps,tsys,apply'
        tsdcal2(infile=self.rawfile,calmode=self.calmode,tsysspw=self.tsysspw,ifmap=self.ifmap,outfile=self.outfile)

        self._compare(self.outfile, self.reftables[0], 'TSYS')

    def test_applycal07(self):
        """test_applycal07: overwrite existing scantable"""
        if not os.path.exists(self.outfile):
            shutil.copytree(self.rawfile, self.outfile)
        tsdcal2(infile=self.rawfile,calmode=self.calmode,applytable=[self.skytable,self.tsystable],ifmap=self.ifmap,outfile=self.outfile,overwrite=True)

        self._compare(self.outfile, self.reftables[0], 'TSYS')

class tsdcal2_test_selection(selection_syntax.SelectionSyntaxTest,
                           tsdcal2_caltest_base, unittest.TestCase):
    """
    Test selection syntax. Selection parameters to test are:
    field, spw (no channel selection), scan, pol
    
    Data used for this test are sd_analytic_type1-3.asap (raw data)
    and sd_analytic_type1-3.cal.asap (reference data).

    """
    # Input and output names
    rawfile='sd_analytic_type1-3.asap'
    reffile='sdcal2TestSel.pstsysapply.asap.ref'
    prefix=tsdcal2_unittest_base.taskname+'TestSel'
    postfix='.cal.asap'
    calmode='ps,tsys,apply'
    field_prefix = 'M100__'
    
    @property
    def task(self):
        return tsdcal2
    
    @property
    def spw_channel_selection(self):
        return False

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(tsdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    ####################
    # scan
    ####################
    def test_scan_id_default(self):
        """test scan selection (scan='')"""
        outname=self.prefix+self.postfix
        scan=''
        self.res=self.run_task(infile=self.rawfile,calmode=self.calmode,scan=scan,outfile=outname)
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname, self.reffile)

    def test_scan_id_exact(self):
        """ test scan selection (scan='15')"""
        outname=self.prefix+self.postfix
        scan = '15'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname)
        tbsel = {'SCANNO': [15]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_lt(self):
        """ test scan selection (scan='<17')"""
        outname=self.prefix+self.postfix
        scan = '<17'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname)
        tbsel = {'SCANNO': [15,16]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_gt(self):
        """ test scan selection (scan='>15')"""
        outname=self.prefix+self.postfix
        scan = '>15'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname)
        tbsel = {'SCANNO': [16,17]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_range(self):
        """ test scan selection (scan='15~16')"""
        outname=self.prefix+self.postfix
        scan = '15~16'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname)
        tbsel = {'SCANNO': [15,16]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_list(self):
        """ test scan selection (scan='15,17')"""
        outname=self.prefix+self.postfix
        scan = '15,17'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname)
        tbsel = {'SCANNO': [15,17]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_exprlist(self):
        """ test scan selection (scan='<16, 17')"""
        outname=self.prefix+self.postfix
        scan = '<16, 17'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,outfile=outname)
        tbsel = {'SCANNO': [15,17]}
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
        self.res=self.run_task(infile=self.rawfile,calmode=self.calmode,pol=pol,outfile=outname)
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname, self.reffile)

    def test_pol_id_exact(self):
        """ test pol selection (pol='1')"""
        outname=self.prefix+self.postfix
        pol = '1'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname)
        tbsel = {'POLNO': [1]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_pol_id_lt(self):
        """ test pol selection (pol='<1')"""
        outname=self.prefix+self.postfix
        pol = '<1'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname)
        tbsel = {'POLNO': [0]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_pol_id_gt(self):
        """ test pol selection (pol='>0')"""
        outname=self.prefix+self.postfix
        pol = '>0'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname)
        tbsel = {'POLNO': [1]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_pol_id_range(self):
        """ test pol selection (pol='0~1')"""
        outname=self.prefix+self.postfix
        pol = '0~1'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname)
        tbsel = {}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_pol_id_list(self):
        """ test pol selection (pol='0,1')"""
        outname=self.prefix+self.postfix
        pol = '0,1'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,outfile=outname)
        tbsel = {}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    @unittest.expectedFailure
    def test_pol_id_exprlist(self):
        """test pol selection (pol='')"""
        self._default_test()

    ####################
    # field
    ####################
    def test_field_value_default(self):
        """test field selection (field='')"""
        outname=self.prefix+self.postfix
        field=''
        self.res=self.run_task(infile=self.rawfile,calmode=self.calmode,field=field,outfile=outname)
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname, self.reffile)

    def test_field_id_exact(self):
        """ test field selection (field='6')"""
        outname=self.prefix+self.postfix
        field = '6'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__6']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_id_lt(self):
        """ test field selection (field='<6')"""
        outname=self.prefix+self.postfix
        field = '<6'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__5']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_id_gt(self):
        """ test field selection (field='>7')"""
        outname=self.prefix+self.postfix
        field = '>7'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname)
        tbsel = {'FIELDNAME': ['3C273__8']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_id_range(self):
        """ test field selection (field='5~7')"""
        outname=self.prefix+self.postfix
        field = '5~7'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', 'M30__7']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_id_list(self):
        """ test field selection (field='5,7')"""
        outname=self.prefix+self.postfix
        field = '5,7'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname)
        fieldid = [1]
        tbsel = {'FIELDNAME': ['M100__5', 'M30__7']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_id_exprlist(self):
        """ test field selection (field='<7,8')"""
        outname=self.prefix+self.postfix
        field = '<7,8'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname)
        fieldid = [1]
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', '3C273__8']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_value_exact(self):
        """ test field selection (field='M100')"""
        outname=self.prefix+self.postfix
        field = 'M100'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname)
        fieldid = [1]
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_value_pattern(self):
        """ test field selection (field='M*')"""
        outname=self.prefix+self.postfix
        field = 'M*'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', 'M30__7']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_value_list(self):
        """ test field selection (field='M30,3C273')"""
        outname=self.prefix+self.postfix
        field = 'M30,3C273'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname)
        tbsel = {'FIELDNAME': ['M30__7', '3C273__8']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_mix_exprlist(self):
        """ test field selection (field='<7,3C273')"""
        outname=self.prefix+self.postfix
        field = '<7,3C273'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', '3C273__8']}
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
        self.res=self.run_task(infile=self.rawfile,calmode=self.calmode,spw=spw,outfile=outname)
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname, self.reffile)

    @unittest.expectedFailure
    def test_spw_id_exact(self):
        """ test spw selection (spw='21')"""
        self._default_test()
#         outname=self.prefix+self.postfix
#         spw = '21'
#         self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname)
#         tbsel = {'IFNO': [21]}
#         self.assertEqual(self.res,None,
#                          msg='Any error occurred during calibration')
#         self._comparecal_with_selection(outname, tbsel)

    def test_spw_id_lt(self):
        """ test spw selection (spw='<25')"""
        outname=self.prefix+self.postfix
        spw = '<25'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname)
        tbsel = {'IFNO': [21,23]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)


    def test_spw_id_gt(self):
        """ test spw selection (spw='>21')"""
        outname=self.prefix+self.postfix
        spw = '>21'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname)
        tbsel = {'IFNO': [23,25]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_id_range(self):
        """ test spw selection (spw='21~24')"""
        outname=self.prefix+self.postfix
        spw = '21~24'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname)
        tbsel = {'IFNO': [21,23]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_id_list(self):
        """ test spw selection (spw='21,22,23,25')"""
        outname=self.prefix+self.postfix
        spw = '21,22,23,25'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname)
        tbsel = {'IFNO': [21,23,25]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_id_exprlist(self):
        """ test spw selection (spw='<22,>24')"""
        outname=self.prefix+self.postfix
        spw = '<22,>24'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname)
        tbsel = {'IFNO': [21,25]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_id_pattern(self):
        """test spw selection (spw='*')"""
        outname=self.prefix+self.postfix
        spw='*'
        self.res=self.run_task(infile=self.rawfile,calmode=self.calmode,spw=spw,outfile=outname)
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname, self.reffile)

    def test_spw_value_frequency(self):
        """test spw selection (spw='300~310GHz')"""
        outname=self.prefix+self.postfix
        spw = '300~310GHz'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname)
        tbsel = {'IFNO': [23,25]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_value_velocity(self):
        """test spw selection (spw='-50~50km/s')"""
        outname=self.prefix+self.postfix
        spw = '-50~50km/s'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname)
        tbsel = {'IFNO': [23]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_mix_exprlist(self):
        """test spw selection (spw='150~500km/s,>23')"""
        outname=self.prefix+self.postfix
        spw = '150~500km/s,>23'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,outfile=outname)
        tbsel = {'IFNO': [21,25]}
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
    return [tsdcal2_exceptions, tsdcal2_skycal_ps,
            tsdcal2_skycal_otf, tsdcal2_skycal_otfraster,
            tsdcal2_tsyscal, tsdcal2_tsyscal_average,
            tsdcal2_applycal,
            tsdcal2_test_selection]
