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
    sddatapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/singledish/'
    tolerance=1.0e-15

    def _comparecal_with_selection( self, name, reffile, tbsel={} ):
        self._checkfile(name)
        self._checkfile(reffile)
        sp=self._getspectra(name)
        spref=self._getspectra_selected(reffile, tbsel)
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
            self.res=sdcal2(infile=self.rawfile,calmode='apply',outfile=self.outfile)
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
            self.res=sdcal2(infile=self.rawfile,calmode='apply',applytable=[self.skytable,self.tsystable],interp=interp,spwmap=self.spwmap,outfile=self.outfile)
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
            self.res=sdcal2(infile=self.rawfile,calmode='apply',applytable=[self.skytable,self.tsystable],interp=interp,spwmap=self.spwmap,outfile=self.outfile)
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
            pos=str(e).find("Output file '%s' exists." % self.rawfile)
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
    default_params = {'infile': rawfile, 'calmode': calmode,
                      'interp': interp, 'spwmap': spwmap,
                      'applytable': applytable, 'outfile': prefix+postfix}

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
        scan = ''
        tbsel = {}
        self._run_cal2_test(tbsel, scan=scan)

    def test_scan_id_exact(self):
        """ test scan selection (scan='15')"""
        scan = '15'
        tbsel = {'SCANNO': [15]}
        self._run_cal2_test(tbsel, scan=scan)

    def test_scan_id_lt(self):
        """ test scan selection (scan='<17')"""
        scan = '<17'
        tbsel = {'SCANNO': [15,16]}
        self._run_cal2_test(tbsel, scan=scan)

    def test_scan_id_gt(self):
        """ test scan selection (scan='>15')"""
        scan = '>15'
        tbsel = {'SCANNO': [16,17]}
        self._run_cal2_test(tbsel, scan=scan)

    def test_scan_id_range(self):
        """ test scan selection (scan='15~16')"""
        scan = '15~16'
        tbsel = {'SCANNO': [15,16]}
        self._run_cal2_test(tbsel, scan=scan)

    def test_scan_id_list(self):
        """ test scan selection (scan='15,17')"""
        scan = '15,17'
        tbsel = {'SCANNO': [15,17]}
        self._run_cal2_test(tbsel, scan=scan)

    def test_scan_id_exprlist(self):
        """ test scan selection (scan='<16, 17')"""
        scan = '<16, 17'
        tbsel = {'SCANNO': [15,17]}
        self._run_cal2_test(tbsel, scan=scan)

    ####################
    # pol
    ####################
    def test_pol_id_default(self):
        #test pol selection (pol='')
        pol = ''
        tbsel = {}
        self._run_cal2_test(tbsel, pol=pol)

    def test_pol_id_exact(self):
        #test pol selection (pol='1')
        pol = '1'
        tbsel = {'POLNO': [1]}
        self._run_cal2_test(tbsel, pol=pol)

    def test_pol_id_lt(self):
        #test pol selection (pol='<1')
        pol = '<1'
        tbsel = {'POLNO': [0]}
        self._run_cal2_test(tbsel, pol=pol)

    def test_pol_id_gt(self):
        #test pol selection (pol='>0')
        pol = '>0'
        tbsel = {'POLNO': [1]}
        self._run_cal2_test(tbsel, pol=pol)

    def test_pol_id_range(self):
        #test pol selection (pol='0~1')
        pol = '0~1'
        tbsel = {}
        self._run_cal2_test(tbsel, pol=pol)

    def test_pol_id_list(self):
        #test pol selection (pol='0,1')
        pol = '0,1'
        tbsel = {}
        self._run_cal2_test(tbsel, pol=pol)

    def test_pol_id_exprlist(self):
        #test pol selection (pol='1,<1')
        pol='1,<1'
        tbsel = {}
        self._run_cal2_test(tbsel, pol=pol)

    ####################
    # field
    ####################
    def test_field_value_default(self):
        #test field selection (field='')
        field=''
        tbsel = {}
        self._run_cal2_test(tbsel, field=field)

    def test_field_id_exact(self):
        #test field selection (field='6')
        field = '6'
        tbsel = {'FIELDNAME': ['M100__6']}
        self._run_cal2_test(tbsel, field=field)

    def test_field_id_lt(self):
        #test field selection (field='<6')
        field = '<6'
        tbsel = {'FIELDNAME': ['M100__5']}
        self._run_cal2_test(tbsel, field=field)

    def test_field_id_gt(self):
        #test field selection (field='>7')
        field = '>7'
        tbsel = {'FIELDNAME': ['3C273__8']}
        self._run_cal2_test(tbsel, field=field)

    def test_field_id_range(self):
        #test field selection (field='5~7')
        field = '5~7'
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', 'M30__7']}
        self._run_cal2_test(tbsel, field=field)

    def test_field_id_list(self):
        #test field selection (field='5,7')
        field = '5,7'
        tbsel = {'FIELDNAME': ['M100__5', 'M30__7']}
        self._run_cal2_test(tbsel, field=field)

    def test_field_id_exprlist(self):
        #test field selection (field='<7,8')
        field = '<7,8'
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', '3C273__8']}
        self._run_cal2_test(tbsel, field=field)

    def test_field_value_exact(self):
        #test field selection (field='M100')
        field = 'M100'
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6']}
        self._run_cal2_test(tbsel, field=field)

    def test_field_value_pattern(self):
        #test field selection (field='M*')
        field = 'M*'
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', 'M30__7']}
        self._run_cal2_test(tbsel, field=field)

    def test_field_value_list(self):
        #test field selection (field='M30,3C273')
        field = 'M30,3C273'
        tbsel = {'FIELDNAME': ['M30__7', '3C273__8']}
        self._run_cal2_test(tbsel, field=field)

    def test_field_mix_exprlist(self):
        #test field selection (field='<7,3C273')
        field = '<7,3C273'
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', '3C273__8']}
        self._run_cal2_test(tbsel, field=field)

    ####################
    # spw 
    ####################
    def test_spw_id_default(self):
        #test spw selection (spw='')
        spw=''
        tbsel = {}
        self._run_cal2_test(tbsel, spw=spw)

    def test_spw_id_exact(self):
        #test spw selection (spw='21')
        spw = '21'
        tbsel = {'IFNO': [21]}
        self._run_cal2_test(tbsel, spw=spw)

    def test_spw_id_lt(self):
        #test spw selection (spw='<25')
        spw = '<25'
        tbsel = {'IFNO': [21,23]}
        self._run_cal2_test(tbsel, spw=spw)


    def test_spw_id_gt(self):
        #test spw selection (spw='>21')
        spw = '>21'
        tbsel = {'IFNO': [23,25]}
        self._run_cal2_test(tbsel, spw=spw)

    def test_spw_id_range(self):
        #test spw selection (spw='21~24')
        spw = '21~24'
        tbsel = {'IFNO': [21,23]}
        self._run_cal2_test(tbsel, spw=spw)

    def test_spw_id_list(self):
        #test spw selection (spw='21,22,23,25')
        spw = '21,22,23,25'
        tbsel = {'IFNO': [21,23,25]}
        self._run_cal2_test(tbsel, spw=spw)

    def test_spw_id_exprlist(self):
        #test spw selection (spw='<22,>24')
        spw = '<22,>24'
        tbsel = {'IFNO': [21,25]}
        self._run_cal2_test(tbsel, spw=spw)

    def test_spw_id_pattern(self):
        #test spw selection (spw='*')
        spw='*'
        tbsel={}
        self._run_cal2_test(tbsel, spw=spw)

    def test_spw_value_frequency(self):
        #test spw selection (spw='299.5~310GHz')
        spw = '299.5~310GHz'
        tbsel = {'IFNO': [23,25]}
        self._run_cal2_test(tbsel, spw=spw)

    def test_spw_value_velocity(self):
        #test spw selection (spw='-50~50km/s')
        spw = '-50~50km/s'
        tbsel = {'IFNO': [23]}
        self._run_cal2_test(tbsel, spw=spw)

    def test_spw_mix_exprlist(self):
        #test spw selection (spw='150~500km/s,>23')
        spw = '150~550km/s,>23'
        tbsel = {'IFNO': [21,25]}
        self._run_cal2_test(tbsel, spw=spw)

    ####################
    # Helper functions
    ####################
    def _run_cal2_test(self, tbsel, **kwargs):
        """
        Run sdcal2 task with parameters given as key word arguments,
        and test the results by running _comparecal_with_selection().
        """
        params = self.default_params.copy()
        for k,v in kwargs.items():
            params[k] = v
        self.res=self.run_task(**params)
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        outname = params['outfile'] if len(params['outfile']) > 0 else params['infile']
        self._comparecal_with_selection(outname, self.reffile, tbsel)


class sdcal2_storage_test(sdcal2_caltest_base,unittest.TestCase):
    """
    Test overwrite of the input scantable (outfile=infile) with
    disk and memory storage. (Tests for CAS-6601)

    testDisk --- overwrite input file with disk storage
    testMem --- overwrite input file with memory storage
    """
    # Input and output names
    rawfile='sd_analytic_type1-3.asap'
    reffile='sd_analytic_type1-3.cal.asap'
    sky_table='sd_analytic_type1-3.asap_sky'
    tsys_table='sd_analytic_type1-3.asap_tsys'
    applytable=[tsys_table,sky_table]
    params = {'infile': rawfile, 'outfile': rawfile, 'calmode': 'apply',
              'applytable': applytable, 'interp': 'linear,cspline',
              'spwmap': {'20':[21],'22':[23],'24':[25]}}

    def setUp(self):
        self.res=None
        for name in [self.params['infile'], self.reffile]+self.applytable:
            if os.path.exists(name): shutil.rmtree(name)
            shutil.copytree(self.datapath+name, name)
        default(sdcal2)

    def tearDown(self):
        if (os.path.exists(self.params['outfile'])):
            shutil.rmtree(self.params['outfile'])

    def testDisk(self):
        """Test overwrite of the input scantable (outfile=infile) with DISK storage"""
        self._run_cal2_with_storage('disk')
    
    def testMem(self):
        """Test overwrite of the input scantable (outfile=infile) with MEMORY storage"""
        self._run_cal2_with_storage('memory')

    def _run_cal2_with_storage(self, storage):
        storage_save = sd.rcParams['scantable.storage']
        sd.rcParams['scantable.storage'] = storage
        try:
            print("Invoking sdcal2 with storage = %s" % \
                  sd.rcParams['scantable.storage'])
            res = sdcal2(**self.params)
        except  Exception, e:
            self.fail('Unexpected exception was thrown: %s'%(str(e)))
        finally:
            sd.rcParams['scantable.storage'] = storage_save
        self._comparecal_with_selection(self.params['outfile'], self.reffile, {})


def suite():
    return [sdcal2_exceptions, sdcal2_skycal_ps,
            sdcal2_skycal_otf, sdcal2_skycal_otfraster,
            sdcal2_tsyscal, sdcal2_tsyscal_average,
            sdcal2_applycal,
            sdcal2_test_selection,sdcal2_storage_test]
