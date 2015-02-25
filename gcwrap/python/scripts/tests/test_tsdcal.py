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
import sdutil

from tsdcal import tsdcal 

class tsdcal_test(unittest.TestCase):

    """
    Unit test for task tsdcal.

    The list of tests:
    test00	--- default parameters (raises an error)
    test01	--- spwmap comprising list
    test02	--- spwmap comprising dictionary
    test03	--- spwmap comprising others
    test04	--- there is no infile
    test05
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
        self.assertTrue(os.path.exists(reference),msg="Reference file doesn't exist: "+reference)
        self.assertTrue(listing.compare(out,reference),'New and reference files are different. %s != %s. '%(out,reference))

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


    def test01(self):
        """Test01: Validation when spwmap comprising list"""
        tid = "01"
        infile = self.infile1
        tsdcal(infile=infile, calmode='tsys', outfile='tsys.cal')
        spwmap_list=[0,1,2,3,4,5,6,7,8,1,10,3,12,5,14,7,16]
        spwmap_dict={1:[9],3:[11],5:[13],7:[15]}
        tsdcal(infile=infile, calmode='apply', spwmap=spwmap_dict, applytable='tsys.cal', outfile='')
        tb.open(infile)
        subt_dict=tb.query('', sortlist='ANTENNA1, TIME', columns='WEIGHT, CORRECTED_DATA')
        #weight_dict = subt_dict.getcol('WEIGHT')
        #corrected_data_dict = subt_dict.getcol('CORRECTED_DATA')
        tb.close()
        #subt_dict.close()

        #tsdcal(infile=infile, calmode='apply', spwmap=spwmap_dict, applytable='tsys.cal', outfile='')
        #tb.open(infile)
        #subt_list=tb.query('', sortlist='ANTENNA1, TIME, SPECTRAL_WINDOW_ID', columns='WEIGHT, CORRECTED_DATA')
        #weight_list = subt_list.getcol('WEIGHT')
        #corrected_data_list = subt_list.getcol('CORRECTED_DATA')
        #tb.close()
        #subt_list.close()

        #tsdcal(infile=infile, calmode='apply', spwmap=spwmap_list, applytable='tsys.cal', outfile='')


        #print 'dict:', spwmap
        #print 'list:', spwmap
        #if spwmap.all()==spwmap_dict.all():
        #    Spwmap is able to cope with dictionary and list.
        #print spwmap.all()==spwmap_dict.all()


    def test02(self):
        """Test02: Validation when spwmap comprising dictionary

        tid ="02"
        infile=self.infile1
        tsdcal(infile=infile, calmode='tsys', outfile='tsys_dic.cal')
        #spwmap=[0,1,2,3,4,5,6,7,8,1,10,3,12,5,14,7,16]
        spwmap={1:[9],3:[11],5:[13],7:[15]}
        tsdcal(infile=infile, calmode='apply', spwmap=spwmap, applytable='tsys_dic.cal', outfile='')
        """
        

class tsdcal_test_base(unittest.TestCase):
    """
    Base class for tsdcal unit test.
    The following attributes/functions are defined here.

        datapath
        decorators (invalid_argument_case, exception_case)
    """
    # Data path of input
    datapath=os.environ.get('CASAPATH').split()[0]+ '/data/regression/unittest/tsdcal/'

    # Input
    infile = 'uid___A002_X6218fb_X264.ms.sel'
    applytable = infile + '.sky'
    
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
                with self.assertRaises(exception_type) as ctx:
                    func(self)
                    self.fail(msg='The task must throw exception')
                the_exception = ctx.exception
                message = the_exception.message
                self.assertIsNotNone(re.search(exception_pattern, message), msg='error message \'%s\' is not expected.'%(message))
            return _wrapper
        return wrapper

    
class tsdcal_test_skycal(tsdcal_test_base):
    
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
    @property
    def outfile(self):
        return self.applytable

    def setUp(self):  
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(tsdcal)


    def tearDown(self):
        for filename in [self.infile, self.outfile]:
            if os.path.exists(filename):
                shutil.rmtree(filename)

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
                    print myargs
                    a = myms.msseltoindex(self.infile, **myargs)
                    print a
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
            
    
    @tsdcal_test_base.invalid_argument_case
    def test_skycal00(self):
        """
        test_skycal00 --- default parameters (raises an error)
        """
        self.result = tsdcal()

    @tsdcal_test_base.invalid_argument_case
    def test_skycal01(self):
        """
        test_skycal01 --- invalid calibration type
        """
        self.result = tsdcal(infile=self.infile, calmode='invalid_type', outfile=self.outfile)

    @tsdcal_test_base.exception_case(RuntimeError, 'No Spw ID\(s\) matched specifications')
    def test_skycal02(self):
        """
        test_skycal02 --- invalid selection (invalid spw selection)
        """
        self.result = tsdcal(infile=self.infile, calmode='ps', spw='99', outfile=self.outfile)

    @tsdcal_test_base.exception_case(RuntimeError, '^Output file \'.+\' exists\.$')
    def test_skycal03(self):
        """
        test_skycal03 --- outfile exists (overwrite=False)
        """
        # copy input to output
        shutil.copytree(self.infile, self.outfile)
        self.result = tsdcal(infile=self.infile, calmode='ps', outfile=self.outfile, overwrite=False)

    @tsdcal_test_base.exception_case(RuntimeError, 'Output file name must be specified\.')
    def test_skycal04(self):
        """
        test_skycal04 --- empty outfile 
        """
        self.result = tsdcal(infile=self.infile, calmode='ps', outfile='', overwrite=False)

    @normal_case()
    def test_skycal05(self):
        """
        test_skycal05 --- position switch calibration ('ps')
        """
        self.result = tsdcal(infile=self.infile, calmode='ps', outfile=self.outfile)

    @normal_case(spw='9')
    def test_skycal06(self):
        """
        test_skycal06 --- position switch calibration ('ps') with data selection
        """
        self.result = tsdcal(infile=self.infile, calmode='ps', spw='9', outfile=self.outfile)

    @normal_case()
    def test_skycal07(self):
        """
        test_skycal07 --- outfile exists (overwrite=True)
        """
        # copy input to output
        shutil.copytree(self.infile, self.outfile)
        self.result = tsdcal(infile=self.infile, calmode='ps', outfile=self.outfile, overwrite=True)

# interpolator utility for testing
class Interpolator(object):
    def __init__(self, table):
        self.table = table
        self.taql = ''
        self.time = None
        self.data = None

    def select(self, antenna, spw):
        self.taql = 'ANTENNA1 == %s && ANTENNA2 == %s && SPECTRAL_WINDOW_ID == %s'%(antenna, antenna, spw)
        with sdutil.table_selector(self.table, self.taql) as tb:
            self.time = tb.getcol('TIME')
            self.data = tb.getcol('FPARAM')

    def interpolate(self, t):
        raise Exception('Not implemented')

class LinearInterpolator(Interpolator):
    def __init__(self, table):
        super(LinearInterpolator, self).__init__(table)

    def interpolate(self, t):
        dt = self.time - t
        index = abs(dt).argmin()
        if dt[index] > 0.0:
            index -= 1
        if index < 0:
            ref = self.data[:,:,0].copy()
        elif index >= len(self.time) - 1:
            ref = self.data[:,:,-1].copy()
        else:
            t0 = self.time[index]
            t1 = self.time[index+1]
            d0 = self.data[:,:,index]
            d1 = self.data[:,:,index+1]
            ref = ((t1 - t) * d0 + (t - t0) * d1) / (t1 - t0)
        return ref

class NearestInterpolator(Interpolator):
    def __init__(self, table):
        super(NearestInterpolator, self).__init__(table)

    def interpolate(self, t):
        dt = self.time - t
        index = abs(dt).argmin()
        return self.data[:,:,index].copy()
            
class tsdcal_test_apply(tsdcal_test_base):
    
    """
    Unit test for task tsdcal (apply tables).

    The list of tests:
    test_apply_sky00 --- empty applytable
    test_apply_sky01 --- empty applytable (list ver.)
    test_apply_sky02 --- empty applytable list 
    test_apply_sky03 --- unexisting applytable
    test_apply_sky04 --- unexisting applytable (list ver.)
    test_apply_sky05 --- invalid selection (empty selection result)
    test_apply_sky06 --- invalid interp value
    test_apply_sky07 --- invalid applytable (not caltable)
    test_apply_sky08 --- apply data (linear) 
    test_apply_sky09 --- apply selected data
    test_apply_sky10 --- apply data (nearest)
    """

    @property
    def nrow_per_chunk(self):
        # number of rows per antenna per spw is 18
        return 18

    @property
    def eps(self):
        # required accuracy is 2.0e-4
        return 3.0e-4
    
    def setUp(self):
        for f in [self.infile, self.applytable]:
            if os.path.exists(f):
                shutil.rmtree(f)
            shutil.copytree(self.datapath+f, f)

        default(tsdcal)


    def tearDown(self):
        for f in [self.infile, self.applytable]:
            if os.path.exists(f):
                shutil.rmtree(f)

    def normal_case(interp='linear', **kwargs):
        """
        Decorator for the test case that is intended to verify
        normal execution result.

        interp --- interpolation option ('linear' or 'nearest')
        selection --- data selection parameter as dictionary
        """
        def wrapper(func):
            import functools
            @functools.wraps(func)
            def _wrapper(self):
                func(self)

                # sanity check
                self.assertIsNone(self.result, msg='The task must complete without error')
                # verify if CORRECTED_DATA exists
                with sdutil.tbmanager(self.infile) as tb:
                    self.assertTrue('CORRECTED_DATA' in tb.colnames(), msg='CORRECTED_DATA column must be created after task execution!')

                # data selection 
                myms = gentools(['ms'])[0]
                myargs = kwargs.copy()
                if not myargs.has_key('baseline'):
                    with sdutil.tbmanager(self.infile) as tb:
                        antenna1 = numpy.unique(tb.getcol('ANTENNA1'))
                        myargs['baseline'] = '%s&&&'%(','.join(map(str,antenna1)))
                a = myms.msseltoindex(self.infile, **myargs)
                with sdutil.tbmanager(self.applytable) as tb:
                    spwlist = numpy.unique(tb.getcol('SPECTRAL_WINDOW_ID'))
                with sdutil.tbmanager(os.path.join(self.infile, 'DATA_DESCRIPTION')) as tb:
                    spwidcol = tb.getcol('SPECTRAL_WINDOW_ID').tolist()
                    spwddlist = map(spwidcol.index, spwlist)
                if len(a['spw']) > 0:
                    spwlist = list(set(spwlist) & set(a['spw']))
                    spwddlist = map(spwidcol.index, spwlist)
                
                # result depends on interp
                self.assertTrue(interp in ['linear', 'nearest'], msg='Internal Error')
                if interp == 'linear':
                    interpolator = LinearInterpolator(self.applytable)
                else:
                    interpolator = NearestInterpolator(self.applytable)
                for antenna in a['antenna1']:
                    for (spw,spwdd) in zip(spwlist,spwddlist):
                        interpolator.select(antenna, spw)
                        taql = 'ANTENNA1 == %s && ANTENNA2 == %s && DATA_DESC_ID == %s'%(antenna, antenna, spwdd)
                        with sdutil.table_selector(self.infile, taql) as tb:
                            self.assertEqual(tb.nrows(), self.nrow_per_chunk, msg='Number of rows mismatch in antenna %s spw %s'%(antenna, spw))
                            for irow in xrange(tb.nrows()):
                                t = tb.getcell('TIME', irow)
                                data = tb.getcell('DATA', irow)
                                corrected = tb.getcell('CORRECTED_DATA', irow)
                                ref = interpolator.interpolate(t)
                                expected = (data - ref) / ref
                                self.assertEqual(corrected.shape, expected.shape, msg='Shape mismatch in antenna %s spw %s row %s (expeted %s actual %s)'%(antenna,spw,irow,list(expected.shape),list(corrected.shape)))
                                npol, nchan = corrected.shape
                                for ipol in xrange(npol):
                                    for ichan in xrange(nchan):
                                        _expected = expected[ipol,ichan]
                                        _corrected = corrected[ipol,ichan]
                                        if _expected == 0.0:
                                            diff = abs(_corrected - _expected)
                                        else:
                                            diff = abs((_corrected - _expected) / _expected)
                                        self.assertLess(diff, self.eps, msg='Calibrated result differ in antenna %s spw %s row %s pol %s chan %s (expected %s actual %s diff %s)'%(antenna,spw,irow,ipol,ichan,_expected,_corrected,diff))
                                
                    
            return _wrapper
        return wrapper

    @tsdcal_test_base.exception_case(Exception, 'Applytable name must be specified.')
    def test_apply_sky00(self):
        """
        test_apply_sky00 --- empty applytable
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable='')

    @tsdcal_test_base.exception_case(Exception, 'Applytable name must be specified.')
    def test_apply_sky01(self):
        """
        test_apply_sky01 --- empty applytable (list ver.)
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=[''])

    @tsdcal_test_base.exception_case(Exception, 'Applytable name must be specified.')
    def test_apply_sky02(self):
        """
        test_apply_sky02 --- empty applytable list
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=[])

    @tsdcal_test_base.exception_case(Exception, '^Table \".+\" doesn\'t exist\.$')
    def test_apply_sky03(self):
        """
        test_apply_sky03 --- unexisting applytable
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable='notexist.sky')

    @tsdcal_test_base.exception_case(Exception, '^Table \".+\" doesn\'t exist\.$')
    def test_apply_sky04(self):
        """
        test_apply_sky04 --- unexisting applytable (list ver.)
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=['notexist.sky'])

    @tsdcal_test_base.exception_case(RuntimeError, 'No Spw ID\(s\) matched specifications')
    def test_apply_sky05(self):
        """
        test_apply_sky05 --- invalid selection (empty selection result)
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', spw='99', applytable=[self.applytable])
    
    #@tsdcal_test_base.exception_case(RuntimeError, '^Unknown interptype: \'.+\'!! Check inputs and try again\.$')
    @tsdcal_test_base.exception_case(RuntimeError, 'Error in Calibrater::setapply.')
    def test_apply_sky06(self):
        """
        test_apply_sky06 --- invalid interp value
        """
        # 'cubic' interpolation along time axis is not supported yet
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=[self.applytable], interp='cubic')
    
    @tsdcal_test_base.exception_case(RuntimeError, '^Applytable \'.+\' is not a caltable format$')
    def test_apply_sky07(self):
        """
        test_apply_sky07 --- invalid applytable (not caltable)
        """
        # 'cubic' interpolation along time axis is not supported yet
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=[self.infile], interp='linear')

    @normal_case()
    def test_apply_sky08(self):
        """
        test_apply_sky08 --- apply data (linear)
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=[self.applytable], interp='linear')

    @normal_case(spw='9')
    def test_apply_sky09(self):
        """
        test_apply_sky09 --- apply selected data
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=[self.applytable], spw='9', interp='linear')

    @normal_case(interp='nearest')
    def test_apply_sky10(self):
        """
        test_apply_sky10 --- apply data (nearest)
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=[self.applytable], interp='nearest')
        
def suite():
    return [tsdcal_test, tsdcal_test_skycal,
            tsdcal_test_apply]


