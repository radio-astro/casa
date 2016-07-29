import os
import sys
import shutil
import re
import numpy
import math

from __main__ import default
from taskinit import gentools
import unittest

from sdgaincal import sdgaincal

try:
    from testutils import copytree_ignore_subversion
except:
    from tests.testutils import copytree_ignore_subversion


class sdgaincal_test_base(unittest.TestCase):
    """
    Base class for sdgainal unit tests.
    
    This class defines attributes and methods common to test cases 
    """
    datapath=os.environ.get('CASAPATH').split()[0]+ '/data/regression/unittest/sdgaincal/'
    
    def __copy_from_datapath(self, filename):
        if os.path.exists(filename):
            shutil.rmtree(filename)
        copytree_ignore_subversion(self.datapath, filename)        
        
    def setUp(self):
        self.__copy_from_datapath(self.infile)
        
        if hasattr(self, 'reffile'):
            self.__copy_from_datapath(self.reffile)

        default(sdgaincal)

    def tearDown(self):
        to_be_removed = [self.infile, self.outfile]
        for f in to_be_removed:
            if os.path.exists(f):
                shutil.rmtree(f)
                
    def generate_params(self, **params):
        default_params = {'infile': self.infile,
                          'outfile': self.outfile,
                          'overwrite': False,
                          'calmode': 'doublecircle',
                          'radius': '',
                          'smooth': True,
                          'antenna': '',
                          'spw': '',
                          'field': '',
                          'spw': '',
                          'scan': '',
                          'applytable': ''}
        retval = {}
        for (k,v) in default_params.items():
            if params.has_key(k):
                retval[k] = params[k]
            else:
                retval[k] = v
        return retval
    
    def run_task(self, **params):
        result = sdgaincal(**params)
        return result
    
    def _verify_caltable(self, custom_check_func, **params):
        caltable = params['outfile']
        
        # basic check
        self.inspect_caltable(caltable)
        
        custom_check_func(**params)        
    
    def inspect_caltable(self, caltable):
        # caltable must exist
        self.assertTrue(os.path.exists(caltable))
        
        # caltable must be a directory
        self.assertTrue(os.path.isdir(caltable))
        
        # caltable must be opened by table tool
        (tb,) = gentools(['tb'])
        is_open_successful = tb.open(caltable)
        self.assertTrue(is_open_successful)
        
        try:
            # caltype must be "G Jones"
            caltype = tb.getkeyword('VisCal')
            self.assertEqual(caltype, "G Jones")
            
            # paramter must be Float (non-Complex)
            self.assertIn('FPARAM', tb.colnames())
        finally:
            tb.close()
            
    def _generic_verify(self, **params):
        (tb,ms,) = gentools(['tb','ms'])
        
        nrow_per_spw = 102
        
        spwsel = params['spw']
        if spwsel == '':
            nspw = 2
        else:
            infile = params['infile']
            ms.open(infile)
            try:
                ms.msselect({'spw': spwsel})
                mssel = ms.msselectedindices()
                nspw = len(mssel['spw'])
            finally:
                ms.close()
        
        caltable = params['outfile']
        tb.open(caltable)
        try:
            nrow = tb.nrows()
            self.assertEqual(nrow, nspw * nrow_per_spw)
            
            spwids = tb.getcol('SPECTRAL_WINDOW_ID')
            spwid_list = set(spwids)
            for spwid in spwid_list:
                self.assertEqual(len(spwids[spwids == spwid]), nrow_per_spw)
                
                
            # by definition, mean of gain factor becomes almost 1.0
            for spwid in spwid_list:
                t = tb.query('SPECTRAL_WINDOW_ID == %s'%(spwid))
                try:
                    fparam = t.getcol('FPARAM')
                    flag = t.getcol('FLAG')
                finally:
                    t.close()
                ma = numpy.ma.masked_array(fparam, flag)
                mean_gain = ma.mean(axis=2)
                print mean_gain
                self.assertTrue(numpy.all((numpy.abs(mean_gain) - 1.0) < 0.01))
                
            
            self._verify_fparam_and_flag(tb)
            
        finally:
            tb.close()
        pass
    
    def _verify_fparam_and_flag(self, table):
        self.assertFail('_verify_fparam_and_flag not implemented')

class sdgaincal_fail_test(sdgaincal_test_base):
    """
    Unit tests for task sdgaincal.
    
    The list of tests:
    Test Name       | Reason    
    ==========================================================================
    test_fail01     | infile not exist
    test_fail02     | not overwrite existing outfile
    test_fail03     | wrong calibration mode
    test_fail04     | negative radius 
    test_fail05     | pre-application (not implemented yet)
    """
    infile = 'doublecircletest_const.ms'
    outfile = 'sdgaincal_fail_test.sdgain.caltable'
    
    def _test_fail(self, **params):
        result = self.run_task(**params)
        self.assertEqual(result, False)
        
    def _test_except_regex(self, exception_type, pattern, **params):
        with self.assertRaisesRegexp(exception_type, pattern) as cm:
            self.run_task(**params)
        
    def test_fail01(self):
        """test_fail01: infile not exist"""
        params = self.generate_params(infile=self.infile+'.notexist')
        self._test_fail(**params)

    def test_fail02(self):
        """test_fail02: not overwrite existing outfile"""
        params = self.generate_params()
        
        # outfile exists
        shutil.copytree(params['infile'], params['outfile'])

        self._test_except_regex(RuntimeError, '.* exists\.$', **params)
        
    def test_fail03(self):
        """test_fail03: wrong calibration mode"""
        params = self.generate_params(calmode='otf')
        self._test_fail(**params)
        
    def test_fail04(self):
        """test_fail04: negative radius"""
        params = self.generate_params(radius='-30arcsec')
        self._test_except_regex(RuntimeError, 
                                '^Error in Calibrater::setsolve\.$', 
                                **params)
        
    def test_fail05(self):
        """test_fail05: pre-application (not implemented yet)"""
        params = self.generate_params(applytable=self.outfile)
        self._test_except_regex(RuntimeError,
                                '^Pre-application of calibration solutions is not supported yet\.$',
                                **params)
        
class sdgaincal_const_test(sdgaincal_test_base):
    """
    Unit tests for task sdgaincal.
    Test data contains the data constant over time and direction, which 
    means that gain factor is always 1.0.
    
    The list of tests:
    Test Name        | Radius      | Expectation
    ==========================================================================
    test_const01     | ''          | too narrow central region, empty caltable is created
    test_const02     | '65arcsec'  | valid caltable is created. gain factor is all 1.0.
    test_const03     | ''          | overwrite existing file
    test_const04     | ''          | test if data selection works
    """
    infile = 'doublecircletest_const.ms'
    outfile = 'sdgaincal_const_test.sdgain.caltable'
    
    def _is_empty_caltable(self, **params):
        (tb,) = gentools(['tb'])
        tb.open(params['outfile'])
        try:
            nrow = tb.nrows()
        finally:
            tb.close()
        self.assertEqual(nrow, 0)
        
    def _verify_fparam_and_flag(self, table):
        for irow in xrange(table.nrows()):
            fparam = table.getcell('FPARAM', irow)
            self.assertTrue(numpy.all(fparam == 1.0))
                
            flag = table.getcell('FLAG', irow)
            self.assertTrue(numpy.all(flag == False))
    
    def test_const01(self):
        """test_const01: too narrow central region, empty caltable is created"""
        params = self.generate_params()
        self.run_task(**params)
        
        self._verify_caltable(self._is_empty_caltable, **params)
        
    def test_const02(self):
        """test_const02: too narrow central region, empty caltable is created"""
        params = self.generate_params(radius='65arcsec')
        self.run_task(**params)
        
        self._verify_caltable(self._generic_verify, **params)
       
    def test_const03(self):
        """test_const03: overwrite existing file"""
        params = self.generate_params(overwrite=True)
        
        # outfile exists
        shutil.copytree(params['infile'], params['outfile'])

        self.run_task(**params)
        
        self._verify_caltable(self._is_empty_caltable, **params)
        
class sdgaincal_variable_test(sdgaincal_test_base):
    """
    Unit tests for task sdgaincal.
    Test data contains the data constant over time and direction, which 
    means that gain factor is always 1.0.
    
    The list of tests:
    Test Name           | Radius      | Expectation
    ==========================================================================
    test_variable01     | '65arcsec'  | valid caltable is created
    """
    infile = 'doublecircletest_autoscale.ms'
    outfile = 'sdgaincal_variable_test.sdgain.caltable'
    reffile = 'doublecircletest_autoscale.sdgain.caltable'
    
    def _verify_fparam_and_flag(self, table):
        (reftable,) = gentools(['tb'])
        reftable.open(self.reffile)
        
        try:
            nrow = table.nrows()
            ref_nrow = reftable.nrows()
            self.assertEqual(nrow, ref_nrow)
            
            for irow in xrange(nrow):
                ref_fparam = reftable.getcell('FPARAM', irow)
                fparam = table.getcell('FPARAM', irow)
                self.assertTrue(numpy.all(ref_fparam == fparam))
                
                ref_flag = reftable.getcell('FLAG', irow)
                flag = table.getcell('FLAG', irow)
                self.assertTrue(numpy.all(ref_flag == flag))
        finally:
            reftable.close()
            
    
    def test_varialbe01(self):
        """test_variable01: valid caltable is created"""
        params = self.generate_params(radius='65arcsec')
        self.run_task(**params)
        
        self._verify_caltable(self._generic_verify, **params)


def suite():
    return [sdgaincal_fail_test,
            sdgaincal_const_test,
            sdgaincal_variable_test]


