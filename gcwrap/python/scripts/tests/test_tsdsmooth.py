import os
import sys
import shutil
import re
import numpy
import math
from scipy import signal

from __main__ import default
from tasks import *
from taskinit import *
import unittest
#
import listing
import sdutil

from tsdsmooth import tsdsmooth
#from test.test_funcattrs import StaticMethodAttrsTest

try:
    from testutils import copytree_ignore_subversion
except:
    from tests.testutils import copytree_ignore_subversion

def gaussian_kernel(nchan, kwidth):
    sigma = kwidth / (2.0 * math.sqrt(2.0 * math.log(2.0)))
    g = signal.gaussian(nchan, sigma, False)
    g /= g.sum()
    g0 = g[0]
    g[:-1] = g[1:]
    g[-1] = g0
    return g

class tsdsmooth_test_base(unittest.TestCase):
    """
    Base class for tsdsmooth unit test.
    The following attributes/functions are defined here.

        datapath
        decorators (invalid_argument_case, exception_case)
    """
    # Data path of input
    datapath=os.environ.get('CASAPATH').split()[0]+ '/data/regression/unittest/tsdsmooth/'

    # Input
    infile_data = 'tsdsmooth_test.ms'
    infile_float = 'tsdsmooth_test_float.ms'
    
    # task execution result
    result = None
    
    @property
    def outfile(self):
        return self.infile.rstrip('/') + '_out'

    # decorators
    @staticmethod
    def invalid_argument_case(func):
        """
        Decorator for the test case that is intended to fail
        due to invalid argument.
        """
        import functools
        @functools.wraps(func)
        def wrapper(self):
            func(self)
            self.assertFalse(self.result, msg='The task must return False')
        return wrapper

    @staticmethod
    def exception_case(exception_type, exception_pattern):
        """
        Decorator for the test case that is intended to throw
        exception.

            exception_type: type of exception
            exception_pattern: regex for inspecting exception message 
                               using re.search
        """
        def wrapper(func):
            import functools
            @functools.wraps(func)
            def _wrapper(self):
                self.assertTrue(len(exception_pattern) > 0, msg='Internal Error')
                with self.assertRaises(exception_type) as ctx:
                    func(self)
                    self.fail(msg='The task must throw exception')
                the_exception = ctx.exception
                message = the_exception.message
                self.assertIsNotNone(re.search(exception_pattern, message), msg='error message \'%s\' is not expected.'%(message))
            return _wrapper
        return wrapper
    
    @staticmethod    
    def weight_case(func):
        import functools
        @functools.wraps(func)
        def wrapper(self):
            with sdutil.tbmanager(self.infile) as tb:
                for irow in xrange(tb.nrows()):
                    self.assertTrue(tb.iscelldefined('WEIGHT_SPECTRUM', irow))
            
            # weight mode flag
            self.weight_propagation = True
            
            func(self)
            
        return wrapper
    
    def run_test(self, *args, **kwargs):
        datacol_name = self.datacolumn.upper()
        weight_mode = hasattr(self, 'weight_propagation') and getattr(self, 'weight_propagation') is True
        
        if kwargs.has_key('kwidth'):
            kwidth = kwargs['kwidth']
        else:
            kwidth = 5
        
        self.result = tsdsmooth(infile=self.infile, outfile=self.outfile, kernel='gaussian', datacolumn=self.datacolumn, **kwargs)

        # sanity check
        self.assertIsNone(self.result, msg='The task must complete without error')
        self.assertTrue(os.path.exists(self.outfile), msg='Output file is not properly created.')

        if kwargs.has_key('spw'):
            spw = kwargs['spw']
        else:
            spw = ''
        dd_selection = None
        if len(spw) == 0:
            expected_nrow = 2
            with sdutil.tbmanager(self.infile) as tb:
                data_in = tb.getvarcol(datacol_name)
                flag_in = tb.getvarcol('FLAG')
                if weight_mode is True:
                    weight_in = tb.getvarcol('WEIGHT_SPECTRUM')
        else:
            myms = gentools(['ms'])[0]
            a = myms.msseltoindex(self.infile, spw=spw)
            spw_selection = a['spw']
            dd_selection = a['dd']
            expected_nrow = len(spw_selection)
            with sdutil.tbmanager(self.infile) as tb:
                try:
                    tsel = tb.query('DATA_DESC_ID IN %s'%(dd_selection.tolist()))
                    data_in = tsel.getvarcol(datacol_name)
                    flag_in = tsel.getvarcol('FLAG')
                    if weight_mode is True:
                        weight_in = tsel.getvarcol('WEIGHT_SPECTRUM')
                finally:
                    tsel.close()

        with sdutil.tbmanager(self.outfile) as tb:
            nrow = tb.nrows()
            data_out = tb.getvarcol(datacol_name)
            flag_out = tb.getvarcol('FLAG')
            if weight_mode is True:
                weight_out = tb.getvarcol('WEIGHT_SPECTRUM')
            
        # verify nrow
        self.assertEqual(nrow, expected_nrow, msg='Number of rows mismatch (expected %s actual %s)'%(expected_nrow, nrow))

        # verify data
        eps = 1.0e-6
        for key in data_out.keys():
            row_in = data_in[key]
            flg_in = flag_in[key]
            row_in[numpy.where(flg_in == True)] = 0.0
            row_out = data_out[key]
            self.assertEqual(row_in.shape, row_out.shape, msg='Shape mismatch in row %s'%(key))
            
            npol, nchan, _ = row_out.shape
            kernel_array = gaussian_kernel(nchan, kwidth)
            expected = numpy.convolve(row_in[0,:,0], kernel_array, mode='same')
            output = row_out[0,:,0]
            zero_index = numpy.where(numpy.abs(expected) <= eps)
            self.assertTrue(all(numpy.abs(output[zero_index]) < eps), msg='Failed to verify zero values: row %s'%(key))
            nonzero_index= numpy.where(numpy.abs(expected) > eps)
            diff = numpy.abs((output[nonzero_index] - expected[nonzero_index]) / expected[nonzero_index].max())
            #print diff
            #print output[nonzero_index]
            #print expected[nonzero_index]
            self.assertTrue(all(diff < eps), msg='Failed to verify nonzero values: row %s'%(key))
            #print 'row_in', row_in[0,:,0].tolist()
            #print 'gaussian', kernel_array.tolist()
            #print 'expected', expected.tolist() 
            #print 'result', row_out[0,:,0].tolist()
            
            # weight check if this is weight test
            if weight_mode is True:
                #print 'Weight propagation test'
                wgt_in = weight_in[key]
                wgt_out = weight_out[key]
                wkwidth = int(kwidth + 0.5)
                wkwidth += (1 if wkwidth % 2 == 0 else 0)
                half_width = wkwidth / 2
                peak_chan = kernel_array.argmax()
                start_chan = peak_chan - half_width
                wkernel = kernel_array[start_chan:start_chan+wkwidth].copy()
                wkernel /= sum(wkernel)
                weight_expected = wgt_in.copy()
                for ichan in xrange(half_width, nchan-half_width):
                    s = numpy.zeros(npol, dtype=float)
                    for jchan in xrange(wkwidth):
                        s += wkernel[jchan] * wkernel[jchan] / wgt_in[:,ichan-half_width+jchan,0]
                    weight_expected[:,ichan,0] = 1.0 / s
                #print weight_expected[:,:10]
                diff = numpy.abs((wgt_out - weight_expected) / weight_expected)
                self.assertTrue(all(diff.flatten() < eps), msg='Failed to verify spectral weight: row %s'%(key))
        
    def _setUp(self, files, task):
        for f in files:
            if os.path.exists(f):
                shutil.rmtree(f)
            copytree_ignore_subversion(self.datapath, f)

        default(task)

    def _tearDown(self, files):
        for f in files:
            if os.path.exists(f):
                shutil.rmtree(f)
                
    def setUp(self):
        self._setUp([self.infile], tsdsmooth)

    def tearDown(self):
        self._tearDown([self.infile, self.outfile])

class tsdsmooth_test_fail(tsdsmooth_test_base):   
    """
    Unit test for task tsdsmooth.

    The list of tests:
    test_tsdsmooth_fail01 --- default parameters (raises an error)
    test_tsdsmooth_fail02 --- invalid kernel type
    test_tsdsmooth_fail03 --- invalid selection (empty selection result)
    test_tsdsmooth_fail04 --- outfile exists (overwrite=False)
    test_tsdsmooth_fail05 --- empty outfile
    test_tsdsmooth_fail06 --- invalid data column name
    """
    invalid_argument_case = tsdsmooth_test_base.invalid_argument_case
    exception_case = tsdsmooth_test_base.exception_case
    
    infile = tsdsmooth_test_base.infile_data
    
    @invalid_argument_case
    def test_tsdsmooth_fail01(self):
        """test_tsdsmooth_fail01 --- default parameters (raises an error)"""
        self.result = tsdsmooth()
        
    @invalid_argument_case
    def test_tsdsmooth_fail02(self):
        """test_tsdsmooth_fail02 --- invalid kernel type"""
        self.result = tsdsmooth(infile=self.infile, kernel='normal', outfile=self.outfile)
        
    @exception_case(RuntimeError, 'Spw Expression: No match found for 3')
    def test_tsdsmooth_fail03(self):
        """test_tsdsmooth_fail03 --- invalid selection (empty selection result)"""
        self.result = tsdsmooth(infile=self.infile, kernel='gaussian', outfile=self.outfile, spw='3')
        
    @exception_case(Exception, 'tsdsmooth_test\.ms_out exists\.')
    def test_tsdsmooth_fail04(self):
        """test_tsdsmooth_fail04 --- outfile exists (overwrite=False)"""
        shutil.copytree(self.infile, self.outfile)
        self.result = tsdsmooth(infile=self.infile, kernel='gaussian', outfile=self.outfile, overwrite=False)

    @exception_case(Exception, 'outfile is empty\.')
    def test_tsdsmooth_fail05(self):
        """test_tsdsmooth_fail05 --- empty outfile"""
        self.result = tsdsmooth(infile=self.infile, kernel='gaussian', outfile='')
        
    @invalid_argument_case
    def test_tsdsmooth_fail06(self):
        """test_tsdsmooth_fail06 --- invalid data column name"""
        self.result = tsdsmooth(infile=self.infile, outfile=self.outfile, kernel='gaussian', datacolumn='spectra')


class tsdsmooth_test_complex(tsdsmooth_test_base):   
    """
    Unit test for task tsdsmooth. Process MS having DATA column.

    The list of tests:
    test_tsdsmooth_complex_fail01 --- non-existing data column (FLOAT_DATA)
    test_tsdsmooth_complex_gauss01 --- gaussian smoothing (kwidth 5)
    test_tsdsmooth_complex_gauss02 --- gaussian smoothing (kwidth 3)
    test_tsdsmooth_complex_select --- data selection (spw)
    test_tsdsmooth_complex_overwrite --- overwrite existing outfile (overwrite=True)
    """
    exception_case = tsdsmooth_test_base.exception_case
    infile = tsdsmooth_test_base.infile_data
    datacolumn = 'data'

    @exception_case(RuntimeError, 'Desired column \(FLOAT_DATA\) not found in the input MS')
    def test_tsdsmooth_complex_fail01(self):
        """test_tsdsmooth_complex_fail01 --- non-existing data column (FLOAT_DATA)"""
        self.result = tsdsmooth(infile=self.infile, outfile=self.outfile, kernel='gaussian', datacolumn='float_data')
 
    def test_tsdsmooth_complex_gauss01(self):
        """test_tsdsmooth_complex_gauss01 --- gaussian smoothing (kwidth 5)"""
        self.run_test(kwidth=5)
        
    def test_tsdsmooth_complex_gauss02(self):
        """test_tsdsmooth_complex_gauss02 --- gaussian smoothing (kwidth 3)"""
        self.run_test(kwidth=3)

    def test_tsdsmooth_complex_select(self):
        """test_tsdsmooth_complex_select --- data selection (spw)"""
        self.run_test(kwidth=5, spw='1')

    def test_tsdsmooth_complex_overwrite(self):
        """test_tsdsmooth_complex_overwrite --- overwrite existing outfile (overwrite=True)"""
        shutil.copytree(self.infile, self.outfile)
        self.run_test(kwidth=5, overwrite=True)
 
class tsdsmooth_test_float(tsdsmooth_test_base):   
    """
    Unit test for task tsdsmooth. Process MS having FLOAT_DATA column.

    The list of tests:
    test_tsdsmooth_float_fail01 --- non-existing data column (DATA)
    test_tsdsmooth_float_gauss01 --- gaussian smoothing (kwidth 5)
    test_tsdsmooth_float_gauss02 --- gaussian smoothing (kwidth 3)
    test_tsdsmooth_float_select --- data selection (spw)
    test_tsdsmooth_float_overwrite --- overwrite existing outfile (overwrite=True)
    """
    exception_case = tsdsmooth_test_base.exception_case
    infile = tsdsmooth_test_base.infile_float
    datacolumn = 'float_data'
    
    @exception_case(RuntimeError, 'Desired column \(DATA\) not found in the input MS')
    def test_tsdsmooth_float_fail01(self):
        """test_tsdsmooth_complex_fail01 --- non-existing data column (DATA)"""
        self.result = tsdsmooth(infile=self.infile, outfile=self.outfile, kernel='gaussian', datacolumn='data')
    
    def test_tsdsmooth_float_gauss01(self):
        """test_tsdsmooth_float_gauss01 --- gaussian smoothing (kwidth 5)"""
        self.run_test(kwidth=5)
        
    def test_tsdsmooth_float_gauss02(self):
        """test_tsdsmooth_float_gauss02 --- gaussian smoothing (kwidth 3)"""
        self.run_test(kwidth=3)

    def test_tsdsmooth_float_select(self):
        """test_tsdsmooth_float_select --- data selection (spw)"""
        self.run_test(kwidth=5, spw='1')

    def test_tsdsmooth_float_overwrite(self):
        """test_tsdsmooth_float_overwrite --- overwrite existing outfile (overwrite=True)"""
        shutil.copytree(self.infile, self.outfile)
        self.run_test(kwidth=5, overwrite=True)

class tsdsmooth_test_weight(tsdsmooth_test_base):   
    """
    Unit test for task tsdsmooth. Verify weight propagation.

    The list of tests:
    test_tsdsmooth_weight_gauss01 --- gaussian smoothing (kwidth 5)
    test_tsdsmooth_weight_gauss02 --- gaussian smoothing (kwidth 3)
    """
    weight_case = tsdsmooth_test_base.weight_case
    infile = tsdsmooth_test_base.infile_data
    datacolumn = 'data'
    
    def setUp(self):
        super(tsdsmooth_test_weight, self).setUp()
        
        # initialize WEIGHT_SPECTRUM
        with sdutil.cbmanager(self.infile) as cb:
            cb.initweights()
        
    @weight_case
    def test_tsdsmooth_weight_gauss01(self):
        """test_tsdsmooth_weight_gauss01 --- gaussian smoothing (kwidth 5)"""
        self.run_test(kwidth=5)
        
    @weight_case
    def test_tsdsmooth_weight_gauss02(self):
        """test_tsdsmooth_weight_gauss02 --- gaussian smoothing (kwidth 3)"""
        self.run_test(kwidth=3)

class tsdsmooth_test_boxcar(tsdsmooth_test_base):
    """
    Unit test for checking boxcar smoothing.

    The input data (tsdsmooth_delta.ms) has data with the following features:
      in row0, pol0: 1 at index 100, 0 elsewhere,
      in row0, pol1: 1 at index 0 and 2047(i.e., at both ends), 0 elsewhere,
      in row1, pol0: 1 at index 10 and 11, 0 elsewhere,
      in row1, pol1: 0 throughout.
    If input spectrum has delta-function-like feature, the
    expected output spectrum will be smoothing kernel itself.
    As for the data at [row0, pol0], the output data will be: 
      kwidth==1 -> spec[100] = 1
      kwidth==2 -> spec[100,101] = 1/2 (=0.5)
      kwidth==3 -> spec[99,100,101] = 1/3 (=0.333...)
      kwidth==4 -> spec[99,100,101,102] = 1/4 (=0.25)
      kwidth==5 -> spec[98,99,100,101,102] = 1/5 (=0.2)
      and so on.
    """
    
    infile = 'tsdsmooth_delta.ms'
    datacolumn = 'float_data'
    centers = {'00': [100], '01': [0,2047], '10': [10,11], '11':[]}

    def _getLeftWidth(self, kwidth):
        assert(0 < kwidth)
        return (2-kwidth)/2

    def _getRightWidth(self, kwidth):
        assert(0 < kwidth)
        return kwidth/2
    
    def _checkResult(self, spec, kwidth, centers, tol=5.0e-06):
        sys.stdout.write('testing kernel_width = '+str(kwidth)+'...')
        for i in range(len(spec)):
            count = 0
            for j in range(len(centers)):
                lidx = centers[j] + self._getLeftWidth(kwidth)
                ridx = centers[j] + self._getRightWidth(kwidth)
                if (lidx <= i) and (i <= ridx): count += 1
            value = count/float(kwidth)
            self.assertTrue(((spec[i] - value) < tol), msg='Failed.')
        sys.stdout.write('OK.\n')
    
    def setUp(self):
        super(tsdsmooth_test_boxcar, self).setUp()
        
    def test000(self):
        # testing kwidth from 1 to 5.
        for kwidth in range(1,6):
            result = tsdsmooth(infile=self.infile, outfile=self.outfile,
                               datacolumn=self.datacolumn, overwrite=True,
                               kernel='boxcar', kwidth = kwidth)
            with sdutil.tbmanager(self.outfile) as tb:
                for irow in range(tb.nrows()):
                    spec = tb.getcell(self.datacolumn.upper(), irow)
                    for ipol in range(len(spec)):
                        center = self.centers[str(irow)+str(ipol)]
                        self._checkResult(spec[ipol], kwidth, center)

        
def suite():
    return [tsdsmooth_test_fail, tsdsmooth_test_complex,
            tsdsmooth_test_float, tsdsmooth_test_weight,
            tsdsmooth_test_boxcar]


