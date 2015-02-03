import os
import sys
import shutil
import re
import numpy

from __main__ import default
from tasks import *
from taskinit import *
import unittest
#
import listing

from tsdcal import tsdcal 

class tsdcal_test(unittest.TestCase):
    
    """
    Unit test for task tsdcal.

    The list of tests:
    test00   --- default parameters (raises an error)
    """

    # Data path of input
    datapath=os.environ.get('CASAPATH').split()[0]+ '/data/regression/unittest/tsdcal/'

    # Input 
    infile1 = 'uid___A002_X6218fb_X264.ms'
    infiles = [infile1]

    def setUp(self):
        for infile in self.infiles:
            if os.path.exists(infile):
                shutil.rmtree(infile)
            shutil.copytree(self.datapath+infile, infile)

        default(tsdcal)


    def tearDown(self):
        for infile in self.infiles:
            if (os.path.exists(infile)):
                shutil.rmtree(infile)

    def _compareOutFile(self,out,reference):
        self.assertTrue(os.path.exists(out))
        self.assertTrue(os.path.exists(reference),
                        msg="Reference file doesn't exist: "+reference)
        self.assertTrue(listing.compare(out,reference),
                        'New and reference files are different. %s != %s. '
                        %(out,reference))


    def test00(self):
	"""Test00:Check the identification of TSYS_SPECTRuM and FPARAM"""
	
	tid = "00"
	infile = self.infile1
        tsdcal(infile=infile, calmode='tsys', outfile='out.cal')
        compfile1=infile+'/SYSCAL'
        compfile2='out.cal'
	
	tb.open(compfile1)
	subt1=tb.query('', sortlist='ANTENNA_ID, TIME, SPECTRAL_WINDOW_ID', columns='TSYS_SPECTRUM')
	tsys1=subt1.getcol('TSYS_SPECTRUM')
	tb.close()
	subt1.close()

	tb.open(compfile2)
	subt2=tb.query('', sortlist='ANTENNA1, TIME, SPECTRAL_WINDOW_ID', columns='FPARAM, FLAG')
	tsys2=subt2.getcol('FPARAM')
	flag=subt2.getcol('FLAG')
	
	tb.close()
	subt2.close()

        if tsys1.all() == tsys2.all():
		print ''
		print 'The shape of the MS/SYSCAL/TSYS_SPECTRUM', tsys1.shape
		print 'The shape of the FPARAM extracted with tsdcal', tsys2.shape  
		print 'Both tables are identical.'
	else:
		print ''
		print 'The shape of the MS/SYSCAL/TSYS_SPECTRUM', tsys1.shape
        	print 'The shape of the FPARAM of the extraction with tsdcal', tsys2.shape
        	print 'Both tables are not identical.'

       
        if flag.all()==0:
		print 'ALL FLAGs are set to zero.'


class tsdcal_test_skycal(unittest.TestCase):
    
    """
    Unit test for task tsdcal (sky calibration).

    The list of tests:
    test_skycal00 --- default parameters (raises an error)
    test_skycal01 --- invalid calibration type
    test_skycal02 --- invalid selection (empty selection result)
    test_skycal03 --- outfile exists (overwrite=False)
    test_skycal04 --- empty outfile
    test_skycal05 --- position switch calibration ('ps')
    test_skycal06 --- position switch calibration ('ps') with data selection
    test_skycal07 --- outfile exists (overwrite=True)
    """

    # Data path of input
    datapath=os.environ.get('CASAPATH').split()[0]+ '/data/regression/unittest/tsdcal/'

    # Input 
    infile = 'uid___A002_X6218fb_X264.ms.sel'
    infiles = [infile]

    # Output
    @property
    def outfile(self):
        return self.infile + '.sky'

    def setUp(self):
        for infile in self.infiles:
            if os.path.exists(infile):
                shutil.rmtree(infile)
            shutil.copytree(self.datapath+infile, infile)

        default(tsdcal)


    def tearDown(self):
        for infile in self.infiles:
            if (os.path.exists(infile)):
                shutil.rmtree(infile)
        if os.path.exists(self.outfile):
            shutil.rmtree(self.outfile)

    def invalid_argument_case(func):
        """
        Decorator for the test case that is intended to fail
        due to invalid argument.
        """
        import functools
        @functools.wraps(func)
        def wrapper(self):
            result = func(self)
            self.assertFalse(result, msg='The task must return False')
        return wrapper

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
                with self.assertRaises(exception_type) as ctx:
                    result = func(self)
                    self.fail(msg='The task must throw exception')
                the_exception = ctx.exception
                message = the_exception.message
                self.assertIsNotNone(re.search(exception_pattern, message), msg='error message \'%s\' is not expected.'%(message))
            return _wrapper
        return wrapper

    def normal_case(calmode='ps', **kwargs):
        """
        Decorator for the test case that is intended to verify
        normal execution result.

        calmode --- calibration mode
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
                result = func(self)

                # sanity check
                self.assertIsNone(result, msg='The task must complete without error')
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
                        import sdutil
                        with sdutil.tbmanager(self.infile) as tb:
                            antenna1 = numpy.unique(tb.getcol('ANTENNA1'))
                            myargs['baseline'] = '%s&&&'%(','.join(map(str,antenna1)))
                    print myargs
                    a = myms.msseltoindex(self.infile, **myargs)
                    print a
                    antenna1_selection = a['antenna1']
                    spw_selection = a['spw']
                    expected_nrow = 3 * len(spw_selection) * len(antenna1_selection)
                #self.assertTrue(isinstance(selection, dict), msg='Internal Error')
                #for (k,v) in selection.items():
                #    self.assertTrue(isinstance(k,int), msg='Internal Error')
                #    self.assertTrue(isinstance(v,list), msg='Internal Error')
                #    expected_nrow += 3 * len(v)
                #    print k, v, expected_nrow
                #if expected_nrow == 0:
                #    expected_nrow = 12
                import sdutil
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
            
    
    @invalid_argument_case
    def test_skycal00(self):
        """
        test_skycal00 --- default parameters (raises an error)
        """
        result = tsdcal()
        return result

    @invalid_argument_case
    def test_skycal01(self):
        """
        test_skycal01 --- invalid calibration type
        """
        result = tsdcal(infile=self.infile, calmode='invalid_type', outfile=self.outfile)
        return result

    @exception_case(RuntimeError, 'No Spw ID\(s\) matched specifications')
    def test_skycal02(self):
        """
        test_skycal02 --- invalid selection (invalid spw selection)
        """
        result = tsdcal(infile=self.infile, calmode='ps', spw='99', outfile=self.outfile)
        return result

    @exception_case(RuntimeError, '^Output file \'.+\' exists\.$')
    def test_skycal03(self):
        """
        test_skycal03 --- outfile exists (overwrite=False)
        """
        # copy input to output
        shutil.copytree(self.infile, self.outfile)
        result = tsdcal(infile=self.infile, calmode='ps', outfile=self.outfile, overwrite=False)
        return result

    @exception_case(RuntimeError, 'Output file name must be specified\.')
    def test_skycal04(self):
        """
        test_skycal04 --- empty outfile 
        """
        result = tsdcal(infile=self.infile, calmode='ps', outfile='', overwrite=False)
        return result

    @normal_case()
    def test_skycal05(self):
        """
        test_skycal05 --- position switch calibration ('ps')
        """
        result = tsdcal(infile=self.infile, calmode='ps', outfile=self.outfile)
        return result

    @normal_case(spw='9')
    def test_skycal06(self):
        """
        test_skycal06 --- position switch calibration ('ps') with data selection
        """
        result = tsdcal(infile=self.infile, calmode='ps', spw='9', outfile=self.outfile)
        return result

    @normal_case()
    def test_skycal07(self):
        """
        test_skycal07 --- outfile exists (overwrite=True)
        """
        # copy input to output
        shutil.copytree(self.infile, self.outfile)
        result = tsdcal(infile=self.infile, calmode='ps', outfile=self.outfile, overwrite=True)
        return result

def suite():
    return [tsdcal_test, tsdcal_test_skycal]


