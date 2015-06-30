import os
import sys
import shutil
import re
import numpy
import math

from __main__ import default
from tasks import *
from taskinit import *
import unittest
#
import listing
import sdutil

from tsdsmooth import tsdsmooth

try:
    from testutils import copytree_ignore_subversion
except:
    from tests.testutils import copytree_ignore_subversion

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
                
class tsdsmooth_test_fail(tsdsmooth_test_base):   
    """
    Unit test for task tsdsmooth.

    The list of tests:
    test_tsdsmooth_fail00 --- default parameters (raises an error)
    test_tsdsmooth_fail01 --- invalid kernel type
    test_tsdsmooth_fail02 --- invalid selection (empty selection result)
    test_tsdsmooth_fail03 --- outfile exists (overwrite=False)
    test_tsdsmooth_fail04 --- empty outfile
    """
    invalid_argument_case = tsdsmooth_test_base.invalid_argument_case
    exception_case = tsdsmooth_test_base.exception_case
    
    infile = tsdsmooth_test_base.infile_data
    
    @property
    def outfile(self):
        return self.infile.rstrip('/') + '_out'

    def setUp(self):
        self._setUp([self.infile], tsdsmooth)

    def tearDown(self):
        self._tearDown([self.infile, self.outfile])
        
    @invalid_argument_case
    def test_tsdsmooth_fail00(self):
        """test_tsdsmooth_fail00 --- default parameters (raises an error)"""
        self.result = tsdsmooth()
        
    @invalid_argument_case
    def test_tsdsmooth_fail01(self):
        """test_tsdsmooth_fail01 --- invalid kernel type"""
        self.result = tsdsmooth(infile=self.infile, kernel='normal', outfile=self.outfile)
        
    @exception_case(RuntimeError, 'No Spw ID\(s\) matched specifications')
    def test_tsdsmooth_fail02(self):
        """test_tsdsmooth_fail02 --- invalid selection (empty selection result)"""
        self.result = tsdsmooth(infile=self.infile, kernel='gaussian', outfile=self.outfile, spw='3')
        
    @exception_case(Exception, 'tsdsmooth_test\.ms_out exists\.')
    def test_tsdsmooth_fail03(self):
        """test_tsdsmooth_fail03 --- outfile exists (overwrite=False)"""
        shutil.copytree(self.infile, self.outfile)
        self.result = tsdsmooth(infile=self.infile, kernel='gaussian', outfile=self.outfile, overwrite=False)

    @exception_case(Exception, 'outfile is empty\.')
    def test_tsdsmooth_fail04(self):
        """test_tsdsmooth_fail04 --- empty outfile"""
        self.result = tsdsmooth(infile=self.infile, kernel='gaussian', outfile='')

    def normal_case(**kwargs):
        """
        Decorator for the test case that is intended to verify
        normal execution result.

        selection --- data selection parameter as dictionary

        Here, expected result is as follows:
            - total number of rows is 12
            - number of antennas is 2
            - number of spectral windows is 2
            - each (antenna,spw) pair has 3 rows
            - expected sky data is a certain fixed value except completely
              flagged channels
              ANT, SPW, SKY
              0     9   [1.0, 2.0, 3.0]
              1     9   [7.0, 8.0, 9.0]
              0    11   [4.0, 5.0, 6.0]
              1    11   [10.0, 11.0, 12.0]
            - channels 0~10 are flagged, each integration has sprious
              ANT, SPW, SKY
              0     9   [(511,512), (127,128), (383,384)]
              1     9   [(511,512), (127,128), (383,384)]
              0    11   [(511,512), (127,128), (383,384)]
              1    11   [(511,512), (127,128), (383,384)]
        """
        def wrapper(func):
            import functools
            @functools.wraps(func)
            def _wrapper(self):
                func(self)

                # sanity check
                self.assertIsNone(self.result, msg='The task must complete without error')
                self.assertTrue(os.path.exists(self.outfile), msg='Output file is not properly created.')

                # verifying nrow
                if len(kwargs) == 0:
                    expected_nrow = 12
                    antenna1_selection = None
                    spw_selection = None
                else:
                    myms = gentools(['ms'])[0]
                    myargs = kwargs.copy()
                    if not myargs.has_key('baseline'):
                        with sdutil.tbmanager(self.infile) as tb:
                            antenna1 = numpy.unique(tb.getcol('ANTENNA1'))
                            myargs['baseline'] = '%s&&&'%(','.join(map(str,antenna1)))
                    a = myms.msseltoindex(self.infile, **myargs)
                    antenna1_selection = a['antenna1']
                    spw_selection = a['spw']
                    expected_nrow = 3 * len(spw_selection) * len(antenna1_selection)
                with sdutil.tbmanager(self.outfile) as tb:
                    self.assertEqual(tb.nrows(), expected_nrow, msg='Number of rows mismatch (expected %s actual %s)'%(expected_nrow, tb.nrows()))

                # verifying resulting sky spectra
                expected_value = {0: {9: [1., 2., 3.],
                                      11: [4., 5., 6.]},
                                  1: {9: [7., 8., 9.],
                                      11: [10., 11., 12.]}}
                eps = 1.0e-6
                for (ant,d) in expected_value.items():
                    if antenna1_selection is not None and ant not in antenna1_selection:
                        continue
                    for (spw,val) in d.items():
                        if spw_selection is not None and spw not in spw_selection:
                            continue
                        #print ant, spw, val
                        construct = lambda x: '%s == %s'%(x)
                        taql = ' && '.join(map(construct,[('ANTENNA1',ant), ('SPECTRAL_WINDOW_ID',spw)]))
                        with sdutil.table_selector(self.outfile, taql) as tb:
                            nrow = tb.nrows()
                            self.assertEqual(nrow, 3, msg='Number of rows mismatch')
                            for irow in xrange(tb.nrows()):
                                expected = val[irow]
                                self.assertGreater(expected, 0.0, msg='Internal Error')
                                fparam = tb.getcell('FPARAM', irow)
                                flag = tb.getcell('FLAG', irow)
                                message_template = lambda x,y: 'Unexpected %s for antenna %s spw %s row %s (expected %s)'%(x,ant,spw,irow,y)
                                self.assertTrue(all(flag[:,:10].flatten() == True), msg=message_template('flag status', True))
                                self.assertTrue(all(flag[:,10:].flatten() == False), msg=message_template('flag status', False))
                                fparam_valid = fparam[flag == False]
                                error = abs((fparam_valid - expected) / expected) 
                                self.assertTrue(all(error < eps), msg=message_template('sky data', expected))
            return _wrapper
        return wrapper
            
    
#     @invalid_argument_case
#     def test_ps00(self):
#         """
#         test_ps00 --- default parameters (raises an error)
#         """
#         self.result = tsdsmooth()



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
    infile = tsdsmooth_test_base.infile_data
 
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
    infile = tsdsmooth_test_base.infile_float
    
    
def suite():
    return [tsdsmooth_test_fail, tsdsmooth_test_complex,
            tsdsmooth_test_float]


