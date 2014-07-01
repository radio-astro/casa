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

from sdutil import tbmanager
    
# to rethrow exception 
import inspect
g = sys._getframe(len(inspect.stack())-1).f_globals
g['__rethrow_casa_exceptions'] = True
from sdcal2 import sdcal2
import asap as sd

#
# Unit test of sdcal task.
# 

###
# Base class for sdcal unit test
###
class sdcal2_unittest_base:
    """
    Base class for sdcal unit test
    """
    taskname='sdcal2'
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
class sdcal2_caltest_base(sdcal2_unittest_base):
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
            #meta = ['SCANNO','IFNO','POLNO','TIME','ELEVATION']
            meta = ['SCANNO','IFNO','POLNO','TIME']
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
                for i in range(len(diff)):
                    if (diff[i] > 0.02):
                        simple_diff = abs(sp[i] - spref[i])
                        if (simple_diff < 0.02):
                            diff[i] = simple_diff
                self.assertTrue(numpy.all(diff < 0.02),
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
class sdcal2_exceptions(sdcal2_unittest_base,unittest.TestCase):
    """
    Test on bad parameter setting
    """
    # Input and output names
    rawfile='sdcal2Test.ps.asap'
    #prefix=sdcal2_unittest_base.taskname+'Test.ps'
    prefix='sdcal2Test.ps'
    outfile=prefix+'.asap.out'
    skytable=prefix+'.sky'
    tsystable=prefix+'.tsys'
    spwmap={1:[5,6]}

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.skytable)):
            shutil.copytree(self.datapath+self.skytable, self.skytable)
        if (not os.path.exists(self.tsystable)):
            shutil.copytree(self.datapath+self.tsystable, self.tsystable)

        default(sdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_exception00(self):
        """test_exception00: Default parameters"""
        # argument verification error
        self.res=sdcal2()
        self.assertFalse(self.res)
        """
        try:
            self.res=sdcal2()
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
            self.res=sdcal2(infile=self.rawfile,calmode='apply')
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
            self.res=sdcal2(infile=self.rawfile,calmode='apply',applytable=[self.skytable,self.tsystable],interp=interp,spwmap=self.spwmap)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Interpolation type \'%s\' is invalid or not supported yet.'%(interp))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test_exception03(self):
        """test_exception03: Invalid calibration mode"""
        # argument verification error
        self.res=sdcal2(infile=self.rawfile,calmode='invalid',outfile=self.outfile)
        self.assertFalse(self.res)
        """
        try:
            self.res=sdcal2(infile=self.rawfile,calmode='invalid',outfile=self.outfile)
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
            self.res=sdcal2(infile=self.rawfile,calmode='ps',outfile=self.outfile,overwrite=False)
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
            self.res=sdcal2(infile=self.rawfile,calmode='apply',spwmap=self.spwmap,applytable=[dummytable,self.tsystable],outfile=self.outfile)
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
            self.res=sdcal2(infile=self.rawfile,calmode='apply',spwmap=self.spwmap,applytable=[self.skytable,dummytable],outfile=self.outfile)
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
            self.res=sdcal2(infile=self.rawfile,calmode='apply',applytable=[self.skytable,self.tsystable],interp=interp,spwmap=self.spwmap)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Invalid format of the parameter interp: \'%s\''%(interp))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test_exception09(self):
        """test_exception09: Update infile without setting overwrite to False"""
        try:
            self.res=sdcal2(infile=self.rawfile,calmode='apply',applytable=[self.skytable,self.tsystable],spwmap=self.spwmap)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('You should set overwrite to True if you want to update infile.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

#    def test_exception10(self):
#        """test_exception10: Empty iflist for Tsys calibration"""
#        try:
#            self.res=sdcal2(infile=self.rawfile,calmode='tsys',tsysspw=[],outfile=self.outfile)
#            self.assertTrue(False,
#                            msg='The task must throw exception')
#        except Exception, e:
#            pos=str(e).find('You must specify iflist as a list of IFNOs for Tsys calibration.')
#            self.assertNotEqual(pos,-1,
#                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test_exception11(self):
        """test_exception11: non-scantable input"""
        try:
            self.res=sdcal2(infile=self.skytable,calmode='ps',outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('infile must be in scantable format.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

###
# Test sky calibration (calmode='ps')
###
class sdcal2_skycal_ps(sdcal2_caltest_base,unittest.TestCase):
    """
    Test sky calibration (calmode='ps')
    """
    # Input and output names
    rawfile='sdcal2Test.ps.asap'
    #prefix=sdcal2_unittest_base.taskname+'Test.ps'
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

        default(sdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_skycal_ps00(self):
        """test_skycal_ps00: Sky calibration for calmode='ps' (ALMA)"""
        sdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.outfile)

        self._comparecal(self.outfile, self.skytable)
        
    def test_skycal_ps01(self):
        """test_skycal_ps01: Sky calibration for calmode='ps' (ALMA), overwrite existing table"""
        if (not os.path.exists(self.skytable)):
            shutil.copytree(self.rawfile, self.outfile)
        sdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.outfile,overwrite=True)

        self._comparecal(self.outfile, self.skytable)

    def test_skycal_ps02(self):
        """test_skycal_ps02: Default outfile name for sky calibration."""
        sdcal2(infile=self.rawfile,calmode=self.calmode,outfile='',overwrite=True)
        defaultname = self.rawfile.rstrip('/')+'_sky'
        self.assertTrue(os.path.exists(defaultname),
                        msg='Failed to generate default outfile name.')
        self._comparecal(defaultname, self.skytable)
        
###
# Test sky calibration (calmode='otf')
###
class sdcal2_skycal_otf(sdcal2_caltest_base,unittest.TestCase):
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

        default(sdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_skycal_otf00(self):
        """test_skycal_otf00: Sky calibration for calmode='otf' (ALMA)"""
        sdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.skytable)

        self._comparecal(self.skytable, self.skytable_ref)
        
    def test_skycal_otf01(self):
        """test_skycal_otf01: Sky calibration for calmode='otf' (ALMA), overwrite existing table"""
        if (not os.path.exists(self.skytable_ref)):
            shutil.copytree(self.rawfile, self.skytable)
        sdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.skytable,overwrite=True)

        self._comparecal(self.skytable, self.skytable_ref)

    def test_skycal_otf02(self):
        """test_skycal_otf02: Sky calibration for calmode='otf' (ALMA), apply existing sky table"""
        self.calmode='apply'
        sdcal2(infile=self.rawfile,calmode=self.calmode,applytable=self.skytable_ref,outfile=self.outfile)

        self._compare(self.outfile, self.outfile_ref, False)

    def test_skycal_otf03(self):
        """test_skycal_otf03: Sky calibration with calmode='otf' (ALMA), apply on-the-fly"""
        self.calmode='otf,apply'
        sdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.outfile)

        self._compare(self.outfile, self.outfile_ref, False)
       
###
# Test sky calibration (calmode='otfraster')
###
class sdcal2_skycal_otfraster(sdcal2_caltest_base,unittest.TestCase):
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

        default(sdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_skycal_otfraster00(self):
        """test_skycal_otfraster00: Sky calibration for calmode='otfraster' (ALMA)"""
        sdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.outfile)

        self._comparecal(self.outfile, self.skytable_ref)
        
    def test_skycal_otfraster01(self):
        """test_skycal_otfraster01: Sky calibration for calmode='otfraster' (ALMA), overwrite existing table"""
        if (not os.path.exists(self.skytable_ref)):
            shutil.copytree(self.rawfile, self.outfile)
        sdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.outfile,overwrite=True)

        self._comparecal(self.outfile, self.skytable_ref)
        
    def test_skycal_otfraster02(self):
        """test_skycal_otfraster02: Sky calibration for calmode='otfraster' (ALMA), apply existing sky table"""
        self.calmode='apply'
        sdcal2(infile=self.rawfile,calmode=self.calmode,applytable=self.skytable_ref,outfile=self.outfile)

        self._compare(self.outfile, self.outfile_ref, False)

    def test_skycal_otfraster03(self):
        """test_skycal_otfraster03: Sky calibration with calmode='otfraster' (ALMA), apply on-the-fly"""
        self.calmode='otfraster,apply'
        sdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.outfile)

        self._compare(self.outfile, self.outfile_ref, False)
        
###
# Test Tsys calibration (calmode='tsys')
###
class sdcal2_tsyscal(sdcal2_caltest_base,unittest.TestCase):
    """
    Test Tsys calibration
    """
    # Input and output names
    rawfile='sdcal2Test.ps.asap'
    #prefix=sdcal2_unittest_base.taskname+'Test.ps'
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

        default(sdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_tsyscal00(self):
        """test_tsyscal00: Tsys calibration"""
        sdcal2(infile=self.rawfile,calmode=self.calmode,tsysspw=self.tsysspw,outfile=self.outfile)

        self._comparecal(self.outfile, self.tsystable, 'TSYS')
        
    def test_tsyscal01(self):
        """test_tsyscal01: Tsys calibration, overwrite existing table"""
        if (not os.path.exists(self.tsystable)):
            shutil.copytree(self.rawfile, self.outfile)
        sdcal2(infile=self.rawfile,calmode=self.calmode,tsysspw=self.tsysspw,outfile=self.outfile,overwrite=True)

        self._comparecal(self.outfile, self.tsystable, 'TSYS')

    def test_tsyscal02(self):
        """test_tsyscal02: Default outfile name for Tsys calibration."""
        sdcal2(infile=self.rawfile,calmode=self.calmode,tsysspw=self.tsysspw,outfile='',overwrite=True)
        defaultname = self.rawfile.rstrip('/')+'_tsys'
        self.assertTrue(os.path.exists(defaultname),
                        msg='Failed to generate default outfile name.')
        self._comparecal(defaultname, self.tsystable, 'TSYS')

    def test_tsyscal03(self):
        """test_tsyscal03: Auto-detect spws for atmcal"""
        sdcal2(infile=self.rawfile,calmode=self.calmode,tsysspw=self.tsysspw,outfile=self.outfile)

        self._comparecal(self.outfile, self.tsystable, 'TSYS')


class sdcal2_tsyscal_average(sdcal2_caltest_base,unittest.TestCase):
    """
    Test Tsys calibration
    """
    # Input and output names
    rawfile='sdcal2Test.ps.asap.tsysavg'
    #prefix=sdcal2_unittest_base.taskname+'Test.ps'
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

        default(sdcal2)

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
                self.assertTrue(numpy.all(diff < 1e-5),
                                msg='calibrated result is wrong (irow=%s): maxdiff=%s'%(irow,diff.max()) )
        except Exception, e:
            raise e
        finally:
            tout.close()
            tref.close()

    def test_tsyscal04(self):
        """test_tsyscal04: Averaging Tsys for full range"""
        sdcal2(infile=self.rawfile,calmode=self.calmode,tsysavg=True,tsysspw=self.tsysspw,outfile=self.outfile)

        self._comparecal_average(self.outfile, self.tsystable, 'TSYS')

    def test_tsyscal05(self):
        """test_tsyscal05: Averaging Tsys for specified single range"""
        self.tsysspw = '1:0~128'
        channelrange = [[0,128]]
        sdcal2(infile=self.rawfile,calmode=self.calmode,tsysavg=True,tsysspw=self.tsysspw,outfile=self.outfile)

        self._comparecal_average(self.outfile, self.tsystable, 'TSYS', channelrange)

    def test_tsyscal06(self):
        """test_tsyscal06: Averaging Tsys for specified multiple ranges"""
        self.tsysspw = '1:0~100;300~400'
        channelrange = [[0,100],[300,400]]
        sdcal2(infile=self.rawfile,calmode=self.calmode,tsysavg=True,tsysspw=self.tsysspw,outfile=self.outfile)

        self._comparecal_average(self.outfile, self.tsystable, 'TSYS', channelrange)

    def test_tsyscal07(self):
        """test_tsyscal07: Averaging Tsys for specified single channel"""
        self.tsysspw = '1:0'
        channelrange = [[0,0]]
        sdcal2(infile=self.rawfile,calmode=self.calmode,tsysavg=True,tsysspw=self.tsysspw,outfile=self.outfile)

        self._comparecal_average(self.outfile, self.tsystable, 'TSYS', channelrange)



###
# Test apply calibration
###
class sdcal2_applycal(sdcal2_caltest_base,unittest.TestCase):
    """
    Test apply calibration
    """
    # Input and output names
    rawfile='sdcal2Test.ps.asap'
    #prefix=sdcal2_unittest_base.taskname+'Test.ps'
    prefix='sdcal2Test.ps'
    outfile=prefix+'.asap.out'
    skytable=prefix+'.sky'
    tsystable=prefix+'.tsys'
    reftables=[prefix+'.asap.ref',prefix+'.asap.noTsys.ref',prefix+'.asap.cspline.ref']
    calmode='apply'
    tsysspw='1'#[1]
    spwmap={1:[5,6]}
    
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
                
        default(sdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_applycal00(self):
        """test_applycal00: apply existing sky table and Tsys table"""
        sdcal2(infile=self.rawfile,calmode=self.calmode,applytable=[self.skytable,self.tsystable],spwmap=self.spwmap,outfile=self.outfile)

        self._compare(self.outfile, self.reftables[0], True)
        
    def test_applycal01(self):
        """test_applycal01: apply existing skytable (ps)"""
        sdcal2(infile=self.rawfile,calmode=self.calmode,applytable=self.skytable,spwmap=self.spwmap,outfile=self.outfile)

        self._compare(self.outfile, self.reftables[1], False)

    def test_applycal02(self):
        """test_applycal02: apply existing sky table and Tsys table with cubic spline interpolation along frequency axis"""
        sdcal2(infile=self.rawfile,calmode=self.calmode,applytable=[self.skytable,self.tsystable],interp='linear,cspline',spwmap=self.spwmap,outfile=self.outfile)

        self._compare(self.outfile, self.reftables[2], True)

    def test_applycal03(self):
        """test_applycal03: test update mode (overwrite infile)"""
        sdcal2(infile=self.rawfile,calmode=self.calmode,applytable=[self.skytable,self.tsystable],spwmap=self.spwmap,overwrite=True)

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
        sdcal2(infile=self.rawfile,calmode=self.calmode,outfile=self.outfile)

        self._compare(self.outfile, self.reftables[1], False)

    def test_applycal05(self):
        """test_applycal05: calibrate sky and apply it on-the-fly with existing Tsys table"""
        self.calmode='ps,apply'
        sdcal2(infile=self.rawfile,calmode=self.calmode,spwmap=self.spwmap,applytable=self.tsystable,outfile=self.outfile)

        self._compare(self.outfile, self.reftables[0], 'TSYS')

    def test_applycal06(self):
        """test_applycal06: calibrate sky as well as Tsys and apply them on-the-fly"""
        self.calmode='ps,tsys,apply'
        sdcal2(infile=self.rawfile,calmode=self.calmode,tsysspw=self.tsysspw,spwmap=self.spwmap,outfile=self.outfile)

        self._compare(self.outfile, self.reftables[0], 'TSYS')

    def test_applycal07(self):
        """test_applycal07: overwrite existing scantable"""
        if not os.path.exists(self.outfile):
            shutil.copytree(self.rawfile, self.outfile)
        sdcal2(infile=self.rawfile,calmode=self.calmode,applytable=[self.skytable,self.tsystable],spwmap=self.spwmap,outfile=self.outfile,overwrite=True)

        self._compare(self.outfile, self.reftables[0], 'TSYS')

class sdcal2_test_selection(selection_syntax.SelectionSyntaxTest,
                           sdcal2_caltest_base, unittest.TestCase):
    """
    Test selection syntax. Selection parameters to test are:
    field, spw (no channel selection), scan, pol
    
    Data used for this test are sd_analytic_type1-3.asap (raw data)
    and sd_analytic_type1-3_ref.asap (reference data).

    """
    # Input and output names
    rawfile='sd_analytic_type1-3.asap'
    reffile='sd_analytic_type1-3.cal.asap'
    prefix=sdcal2_unittest_base.taskname+'TestSel'
    postfix='.cal.asap'

    calmode='apply'
    interp='linear,cspline'
    sky_table='sd_analytic_type1-3.asap_sky'
    tsys_table='sd_analytic_type1-3.asap_tsys'
    applytable=[tsys_table,sky_table]
    spwmap={'20':[21],'22':[23],'24':[25]}
    field_prefix = 'M100__'
    
    @property
    def task(self):
        return sdcal2
    
    @property
    def spw_channel_selection(self):
        return False

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)
        if (not os.path.exists(self.sky_table)):
            shutil.copytree(self.datapath+self.sky_table, self.sky_table)
        if (not os.path.exists(self.tsys_table)):
            shutil.copytree(self.datapath+self.tsys_table, self.tsys_table)

        default(sdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        if (os.path.exists(self.sky_table)):
            shutil.rmtree(self.sky_table)
        if (os.path.exists(self.tsys_table)):
            shutil.rmtree(self.tsys_table)
        os.system( 'rm -rf '+self.prefix+'*' )

    ####################
    # scan
    ####################
    def test_scan_id_default(self):
        """test scan selection (scan='')"""
        outname=self.prefix+self.postfix
        scan=''
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname, self.reffile)

    def test_scan_id_exact(self):
        """ test scan selection (scan='15')"""
        outname=self.prefix+self.postfix
        scan = '15'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'SCANNO': [15]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_lt(self):
        """ test scan selection (scan='<17')"""
        outname=self.prefix+self.postfix
        scan = '<17'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'SCANNO': [15,16]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_gt(self):
        """ test scan selection (scan='>15')"""
        outname=self.prefix+self.postfix
        scan = '>15'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'SCANNO': [16,17]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_range(self):
        """ test scan selection (scan='15~16')"""
        outname=self.prefix+self.postfix
        scan = '15~16'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'SCANNO': [15,16]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_list(self):
        """ test scan selection (scan='15,17')"""
        outname=self.prefix+self.postfix
        scan = '15,17'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'SCANNO': [15,17]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_exprlist(self):
        """ test scan selection (scan='<16, 17')"""
        outname=self.prefix+self.postfix
        scan = '<16, 17'
        self.res=self.run_task(infile=self.rawfile,scan=scan,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'SCANNO': [15,17]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    ####################
    # pol
    ####################
    def test_pol_id_default(self):
        #test pol selection (pol='')
        outname=self.prefix+self.postfix
        pol=''
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname, self.reffile)

    def test_pol_id_exact(self):
        #test pol selection (pol='1')
        outname=self.prefix+self.postfix
        pol = '1'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'POLNO': [1]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_pol_id_lt(self):
        #test pol selection (pol='<1')
        outname=self.prefix+self.postfix
        pol = '<1'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'POLNO': [0]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_pol_id_gt(self):
        #test pol selection (pol='>0')
        outname=self.prefix+self.postfix
        pol = '>0'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'POLNO': [1]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_pol_id_range(self):
        #test pol selection (pol='0~1')
        outname=self.prefix+self.postfix
        pol = '0~1'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_pol_id_list(self):
        #test pol selection (pol='0,1')
        outname=self.prefix+self.postfix
        pol = '0,1'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_pol_id_exprlist(self):
        #test pol selection (pol='1,<1')
        outname=self.prefix+self.postfix
        pol='1,<1'
        self.res=self.run_task(infile=self.rawfile,pol=pol,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    ####################
    # field
    ####################
    def test_field_value_default(self):
        #test field selection (field='')
        outname=self.prefix+self.postfix
        field=''
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname, self.reffile)

    def test_field_id_exact(self):
        #test field selection (field='6')
        outname=self.prefix+self.postfix
        field = '6'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__6']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_id_lt(self):
        #test field selection (field='<6')
        outname=self.prefix+self.postfix
        field = '<6'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__5']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_id_gt(self):
        #test field selection (field='>7')
        outname=self.prefix+self.postfix
        field = '>7'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'FIELDNAME': ['3C273__8']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_id_range(self):
        #test field selection (field='5~7')
        outname=self.prefix+self.postfix
        field = '5~7'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', 'M30__7']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_id_list(self):
        #test field selection (field='5,7')
        outname=self.prefix+self.postfix
        field = '5,7'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        fieldid = [1]
        tbsel = {'FIELDNAME': ['M100__5', 'M30__7']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_id_exprlist(self):
        #test field selection (field='<7,8')
        outname=self.prefix+self.postfix
        field = '<7,8'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        fieldid = [1]
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', '3C273__8']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_value_exact(self):
        #test field selection (field='M100')
        outname=self.prefix+self.postfix
        field = 'M100'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        fieldid = [1]
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_value_pattern(self):
        #test field selection (field='M*')
        outname=self.prefix+self.postfix
        field = 'M*'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', 'M30__7']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_value_list(self):
        #test field selection (field='M30,3C273')
        outname=self.prefix+self.postfix
        field = 'M30,3C273'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'FIELDNAME': ['M30__7', '3C273__8']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_field_mix_exprlist(self):
        #test field selection (field='<7,3C273')
        outname=self.prefix+self.postfix
        field = '<7,3C273'
        self.res=self.run_task(infile=self.rawfile,field=field,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', '3C273__8']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    ####################
    # spw 
    ####################
    def test_spw_id_default(self):
        #test spw selection (spw='')
        outname=self.prefix+self.postfix
        spw=''
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname, self.reffile)

    def test_spw_id_exact(self):
        #test spw selection (spw='21')
        outname=self.prefix+self.postfix
        spw = '21'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'IFNO': [21]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_id_lt(self):
        #test spw selection (spw='<25')
        outname=self.prefix+self.postfix
        spw = '<25'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'IFNO': [21,23]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)


    def test_spw_id_gt(self):
        #test spw selection (spw='>21')
        outname=self.prefix+self.postfix
        spw = '>21'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'IFNO': [23,25]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_id_range(self):
        #test spw selection (spw='21~24')
        outname=self.prefix+self.postfix
        spw = '21~24'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'IFNO': [21,23]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_id_list(self):
        #test spw selection (spw='21,22,23,25')
        outname=self.prefix+self.postfix
        spw = '21,22,23,25'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'IFNO': [21,23,25]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_id_exprlist(self):
        #test spw selection (spw='<22,>24')
        outname=self.prefix+self.postfix
        spw = '<22,>24'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'IFNO': [21,25]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_id_pattern(self):
        #test spw selection (spw='*')
        outname=self.prefix+self.postfix
        spw='*'
        self.res=self.run_task(infile=self.rawfile,calmode=self.calmode,spw=spw,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname, self.reffile)

    def test_spw_value_frequency(self):
        #test spw selection (spw='299.5~310GHz')
        outname=self.prefix+self.postfix
        spw = '299.5~310GHz'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'IFNO': [23,25]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_value_velocity(self):
        #test spw selection (spw='-50~50km/s')
        outname=self.prefix+self.postfix
        spw = '-50~50km/s'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
        tbsel = {'IFNO': [23]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_spw_mix_exprlist(self):
        #test spw selection (spw='150~500km/s,>23')
        outname=self.prefix+self.postfix
        spw = '150~550km/s,>23'
        self.res=self.run_task(infile=self.rawfile,spw=spw,calmode=self.calmode,interp=self.interp,spwmap=self.spwmap,applytable=self.applytable,outfile=outname)
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
            for i in range(len(diff)):
                if (diff[i] > 0.02):
                    simple_diff = abs(sp[irow][i] - spref[irow][i])
                    if (simple_diff < 0.02):
                        diff[i] = simple_diff
            retval=numpy.all(diff<0.02)
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

###
# Base class for flag test
###
class sdcal2_flag_base(sdcal2_caltest_base, unittest.TestCase):
    """
    Test flag information handling.

    Data is sdcal2_testflag.asap

    Summary of the data:
    ROW | SRCTYPE   | FLAGROW    | FLAGTRA          | SPECTRA/TSYS
     0  | 10 (tsys) | 1 (flagged)| ch 10 flagged    | spurious for all channels
     1  | 10 (tsys) | 0          | ch 10,40 flagged | spurious at ch 10,40
     2  | 10 (tsys) | 0          | ch 10 flagged    | spurious at ch 10
     3  | 1  (off)  | 1 (flagged)| ch 20 flagged    | spurious for all channels
     4  | 1  (off)  | 0          | ch 20,80 flagged | spurious at ch 20,80
     5  | 1  (off)  | 0          | ch 20 flagged    | spurious at ch 20
     6  | 0  (on)   | 1 (flagged)| all 0            | spurious for all channels
     7  | 0  (on)   | 0          | ch 30 flagged    | spurious at ch 30
     8  | 0  (on)   | 0          | ch 10,20 flagged | spurious at ch 10,20
     9  | 1  (off)  | 1 (flagged)| ch 20 flagged    | spurious for all channels
    10  | 1  (off)  | 0          | ch 20,80 flagged | spurious at ch 20,80
    11  | 1  (off)  | 0          | ch 20 flagged    | spurious at ch 20
    12  | 10 (tsys) | 1 (flagged)| ch 10 flagged    | spurious for all channels
    13  | 10 (tsys) | 0          | ch 10,40 flagged | spurious at ch 10,40
    14  | 10 (tsys) | 0          | ch 10 flagged    | spurious at ch 10
    """
    tol = 1.0e-6
    
    def _calculate_row(self, timestamp, generator, average):
        yield timestamp.mean()
        result = list(generator)
        flag = numpy.array([r[0] for r in result])
        data = numpy.array([r[1] for r in result])
        if average:
            w = numpy.array([1.0 if f == 0 else 0.0 for f in flag])
            data[:] = sum(data * w) / sum(w)
            flag[:] = 0
        yield flag
        yield data
    
    def _expected_caltable(self, srctype, colname, average=False):
        #print srctype, colname
        with tbmanager(self.rawfile) as tb:
            tsel = tb.query('SRCTYPE==%s'%(srctype))
            flagrow = tsel.getcol('FLAGROW')
            flagtra = tsel.getcol('FLAGTRA')
            data = tsel.getcol(colname)
            timestamp = tsel.getcol('TIME')
            tsel.close()
        nchan,nrow = flagtra.shape

        def gen_result(fr, fl, sp):
            nchan,nrow = fl.shape
            for ichan in xrange(nchan):
                _fl = fl[ichan,:]
                _sp = sp[ichan,:]
                wsum = 0.0
                dsum = 0.0
                for irow in xrange(nrow):
                    if fr[irow] == 0 and _fl[irow] == 0:
                        dsum += _sp[irow]
                        wsum += 1.0
                if wsum == 0.0:
                    yield 128, sum(_sp) / float(len(_sp))
                else:
                    yield 0, dsum / wsum

        # table should have two rows
        row0 = tuple(self._calculate_row(timestamp[:3], gen_result(flagrow[:3], flagtra[:,:3], data[:,:3]), average))
        row1 = tuple(self._calculate_row(timestamp[3:], gen_result(flagrow[3:], flagtra[:,3:], data[:,3:]), average))
        
        return [row0, row1]

    def _msg(self, row, channel, name, val, ref):
        msg = '%s differ (result %s expected %s)'%(name, val, ref)
        if row is not None:
            header = 'Row %s'%(row)
            if channel is not None:
                header += ' Channel %s'%(channel)
            msg = header + ': ' + msg
        #print msg
        return msg
    
    def _is_equal(self, name, val, ref, row=None, channel=None):
        self.assertEqual(val, ref, msg=self._msg(row, channel, name, val, ref))

    def _absdiff(self, val, ref):
        return abs((val - ref) / ref) if ref != 0.0 else abs(val - ref)
    
    def _is_diff_lt_tol(self, name, val, ref, row=None, channel=None):
        self.assertLess(self._absdiff(val, ref), self.tol, msg=self._msg(row, channel, name, val, ref))
    
    def _verify_caltable(self, outfile, expected):
        # file existence check
        self.assertTrue(os.path.exists(outfile), msg='Output file \'%s\' didn\'t created.'%(outfile))

        with tbmanager(outfile) as tb:
            nrow = tb.nrows()
            colnames = tb.colnames()

        # basic check
        self._is_equal('number of rows', nrow, len(expected))
        self.assertTrue('FLAGTRA' in colnames, msg='Column FLAGTRA doesn\'t exist')
        self.assertTrue('SPECTRA' in colnames or 'TSYS' in colnames, msg='Column SPECTRA or TSYS doesn\'t exist')
        
        if 'SPECTRA' in colnames:
            datacol = 'SPECTRA'
        else:
            datacol = 'TSYS'
            
        with tbmanager(outfile) as tb:
            data = tb.getcol(datacol)
            flag = tb.getcol('FLAGTRA')
            timestamp = tb.getcol('TIME')
        nchan,nrow = flag.shape

        for irow in xrange(nrow):
            result_data = data[:,irow]
            result_flag = flag[:,irow]
            result_time = timestamp[irow]
            expected_data = expected[irow][2]
            expected_flag = expected[irow][1]
            expected_time = expected[irow][0]
            self._is_diff_lt_tol('timestamp', result_time, expected_time, row=irow)
            for ichan in xrange(nchan):
                # check flag
                self._is_equal('flag', result_flag[ichan], expected_flag[ichan], row=irow, channel=ichan)

                # check resulting spectral data
                self._is_diff_lt_tol('data', result_data[ichan], expected_data[ichan], row=irow, channel=ichan)

    def _verify_applycal(self, outfile, expected_sky, expected_tsys):
        # file existence check
        self.assertTrue(os.path.exists(outfile), msg='Output file \'%s\' didn\'t created.'%(outfile))

        # expected number of rows and timestamps
        with tbmanager(self.rawfile) as tb:
            tsel = tb.query('SRCTYPE==0')
            nrow_expected = tsel.nrows()
            time_expected = tsel.getcol('TIME')
        
        # get calibrated data
        with tbmanager(outfile) as tb:
            time_result = tb.getcol('TIME')
            flagtra_result = tb.getcol('FLAGTRA')
            spectra_result = tb.getcol('SPECTRA')
            tsys_result = tb.getcol('TSYS')
        nchan,nrow_result = flagtra_result.shape
        
        # check number of rows
        self._is_equal('number of rows', nrow_result, nrow_expected)

        # check timestamp
        for irow in xrange(nrow_result):
            self._is_equal('timestamp', time_result[irow], time_expected[irow], row=irow)

        # evaluate expected data and flag
        expected_cal = self._expected_calibration(expected_sky, expected_tsys)

        # iterate rows:
        for (irow, cal) in zip(xrange(nrow_result), expected_cal):
            cal_flag = cal[0]
            cal_data = cal[1]
            cal_tsys = cal[2]
            flag = flagtra_result[:,irow]
            data = spectra_result[:,irow]
            tsys = tsys_result[:,irow]
            #print '###', irow
            #print cal_data, data
            #print '---'
            #print cal_tsys, tsys
            for ichan in xrange(nchan):
                self._is_equal('flag', flag[ichan], cal_flag[ichan], row=irow, channel=ichan)
                self._is_diff_lt_tol('Tsys', tsys[ichan], cal_tsys[ichan], row=irow, channel=ichan)
                self._is_diff_lt_tol('spectral data', data[ichan], cal_data[ichan], row=irow, channel=ichan)
        
    def _expected_calibration(self, expected_sky, expected_tsys):
        with tbmanager(self.rawfile) as tb:
            tsel = tb.query('SRCTYPE==0')
            time_result = tsel.getcol('TIME')
            flagtra_result = tsel.getcol('FLAGTRA')
            spectra_result = tsel.getcol('SPECTRA')
            tsel.close()

        nrow = len(time_result)

        def gen_calibration(t, fl, sp, expected_sky, expected_tsys):
            retrieve = lambda i, r: numpy.array([_r[i] for _r in r])
            def interp(t, tref, data, flag):
                nrow, nchan = data.shape
                factor = (t - tref[0]) / (tref[1] - tref[0])
                for ichan in xrange(nchan):
                    if flag[0,ichan] == flag[1,ichan]:
                        yield data[0,ichan] + (data[1,ichan] - data[0,ichan]) * factor
                    elif flag[0,ichan] == 0:
                        yield data[0,ichan]
                    elif flag[1,ichan] == 0:
                        yield data[1,ichan]
                        
            calflag = lambda fl_on, fl_sky: numpy.array([128 if f != 0 else 0 for f in (fl_on + fl_sky)])
            t_sky = retrieve(0, expected_sky)
            fl_sky = retrieve(1, expected_sky)
            sp_sky = retrieve(2, expected_sky)
            t_tsys = retrieve(0, expected_tsys)
            fl_tsys = retrieve(1, expected_tsys)
            sp_tsys = retrieve(2, expected_tsys)
            off = numpy.array(list(interp(t, t_sky, sp_sky, fl_sky)))
            tsys = numpy.array(list(interp(t, t_tsys, sp_tsys, fl_tsys)))
            valid_chans = numpy.where(fl_tsys.sum(axis=0) == 0)[0]
            # Tsys in the test data is flat
            scalar_tsys = tsys[valid_chans[0]]
            #print off
            #print scalar_tsys
            #print sp
            calibrated = scalar_tsys * (sp - off) / off
            flag = calflag(fl, fl_sky.sum(axis=0))
            itsys = numpy.ones(len(calibrated), dtype=float) * scalar_tsys
            return (flag, calibrated, itsys)
            
        for irow in xrange(nrow):
            yield gen_calibration(time_result[irow], flagtra_result[:,irow], spectra_result[:,irow], expected_sky, expected_tsys)
            

        

###
# Test Sky calibration flag handling
###
class sdcal2_skycal_flag(sdcal2_flag_base):
    """
    Test list
        test_skycal_flag_ps: test if sky calibration handles flag
                             information properly
    """
    rawfile = 'sdcal2_testflag.asap'
    prefix = 'sdcal2_skycal_flag'
    
    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
                
        default(sdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_skycal_flag_ps(self):
        """test_skycal_flag_ps: test if sky calibration handles flag information properly"""
        outfile = self.prefix + '_sky'
        sdcal2(infile=self.rawfile, calmode='ps', outfile=outfile)

        self._verify_caltable(outfile, self._expected_caltable(1, 'SPECTRA'))

###
# Test Tsys calibration flag handling
###
class sdcal2_tsyscal_flag(sdcal2_flag_base):
    """
    Test list
        test_tsyscal_flag_noaverage:
            test if tsys calibration handles flag information properly
        test_tsyscal_flag_doaverage:
            test if tsys calibration with spectral averaging
            handles flag information properly
    """
    rawfile='sdcal2_testflag.asap'
    prefix = 'sdcal2_tsyscal_flag'
    
    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
                
        default(sdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_tsyscal_flag_noaverage(self):
        """test_tsyscal_flag_noaverage: test if tsys calibration handles flag information properly"""
        outfile = self.prefix + '_tsys'
        average = False
        sdcal2(infile=self.rawfile, calmode='tsys', tsysspw='22', tsysavg=average, outfile=outfile)

        self._verify_caltable(outfile, self._expected_caltable(10, 'TSYS', average))

    def test_tsyscal_flag_doaverage(self):
        """test_tsyscal_flag_noaverage: test if tsys calibration with spectral averaging handles flag information properly"""
        outfile = self.prefix + '_tsys'
        average = True
        sdcal2(infile=self.rawfile, calmode='tsys', tsysspw='22', tsysavg=average, outfile=outfile)

        self._verify_caltable(outfile, self._expected_caltable(10, 'TSYS', average))
        
###
# Test applycal flag handling
###
class sdcal2_applycal_flag(sdcal2_flag_base):
    """
    Test list
    """
    rawfile='sdcal2_testflag.asap'
    prefix = 'sdcal2_applycal_flag'
    
    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
                
        default(sdcal2)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_applycal_flag(self):
        outfile = self.prefix + '_cal'
        average = False

        sdcal2(infile=self.rawfile, outfile=outfile, calmode='ps,tsys,apply',
               tsysspw='22', tsysavg=average, spwmap={22:[23]}, interp='linear')

        self._verify_applycal(outfile, self._expected_caltable(1, 'SPECTRA'),
                              self._expected_caltable(10, 'TSYS', average))
    

def suite():
    return [sdcal2_exceptions, sdcal2_skycal_ps,
            sdcal2_skycal_otf, sdcal2_skycal_otfraster,
            sdcal2_tsyscal, sdcal2_tsyscal_average,
            sdcal2_applycal,
            sdcal2_test_selection,
            sdcal2_skycal_flag, sdcal2_tsyscal_flag, sdcal2_applycal_flag]
