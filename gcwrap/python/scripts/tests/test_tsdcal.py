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

from tsdcal import tsdcal

try:
    from testutils import copytree_ignore_subversion
except:
    from tests.testutils import copytree_ignore_subversion

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
    infile1 = 'uid___A002_X6218fb_X264.ms.sel'
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

        if (tsys1 == tsys2).all():
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
        """Test01: weight = 1/(SIGMA**2) X 1/(FPARAM_ave**2) dictionary version"""
        #focus on antenna1=0, data_disk_id=1
        #spwmap_dict={1:[1],3:[3],5:[5],7:[7]}

        
        tid = "01"
        infile = self.infile1
        tsdcal(infile=infile, calmode='tsys', outfile='tsys.cal')
        initweights(vis=infile, wtmode='nyq', dowtsp=True)        
        #spwmap_list=[0,1,2,3,4,5,6,7,8,1,10,3,12,5,14,7,16]
        #spwmap_dict={1:[9],3:[11],5:[13],7:[15]}
        
        spwmap_dict={1:[1],3:[3],5:[5],7:[7]}        
        tsdcal(infile=infile, calmode='apply', spwmap=spwmap_dict, applytable='tsys.cal', outfile='')
        
                
        tb.open(infile)
        sigma00=tb.getcol('SIGMA')[0][0]
        sigma10=tb.getcol('SIGMA')[1][0]
        weight00=tb.getcol('WEIGHT')[0][0]
        weight10=tb.getcol('WEIGHT')[1][0]
        tb.close()
        
        tb.open('tsys.cal')
        sum_fparam0=0
        sum_fparam1=0
        for i in range(128):
            sum_fparam0 += tb.getvarcol('FPARAM')['r1'][0][i][0]
            sum_fparam1 += tb.getvarcol('FPARAM')['r1'][1][i][0]
        fparam0_ave=sum_fparam0/128.0
        fparam1_ave=sum_fparam1/128.0
        print 'fparam_average_r1_0', fparam0_ave
        print 'fparam_average_r1_1', fparam1_ave
        print 'SIGMA00 ', sigma00
        print 'SIGMA10 ', sigma10
        print 'WEIGHT00 ', weight00
        print 'WEIGHT10 ', weight10
        answer0 = 1/(sigma00**2)*1/(fparam0_ave**2) 
        answer1 = 1/(sigma10**2)*1/(fparam1_ave**2) 
        print 'pol0: 1/SIGMA**2 X 1/(FPARAM_ave)**2', answer0
        print 'pol1: 1/SIGMA**2 X 1/(FPARAM_ave)**2', answer1
        diff0_percent=(weight00-answer0)/weight00*100
        diff1_percent=(weight10-answer1)/weight10*100
        print 'difference between fparam_r1_0 and weight00', diff0_percent, '%' 
        print 'difference between fparam_r1_1 and weight10', diff1_percent, '%'
        tb.close()
            
        
    def test02(self):
        """Test02: weight = 1/(SIGMA**2) X 1/(FPARAM_ave**2) list version"""
        #focus on antenna1=0, data_disk_id=1
        #spwmap_list=[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]

        
        tid = "02"
        infile = self.infile1
        tsdcal(infile=infile, calmode='tsys', outfile='tsys2.cal')
        initweights(vis=infile, wtmode='nyq', dowtsp=True)        
        spwmap_list=[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]
        #spwmap_dict={1:[9],3:[11],5:[13],7:[15]}
        
        #spwmap_dict={1:[1],3:[3],5:[5],7:[7]}        
        tsdcal(infile=infile, calmode='apply', spwmap=spwmap_list, applytable='tsys2.cal', outfile='')
        
                
        tb.open(infile)
        sigma00=tb.getcol('SIGMA')[0][0]
        sigma10=tb.getcol('SIGMA')[1][0]
        weight00=tb.getcol('WEIGHT')[0][0]
        weight10=tb.getcol('WEIGHT')[1][0]
        tb.close()
        
        tb.open('tsys2.cal')
        sum_fparam0=0
        sum_fparam1=0
        for i in range(128):
            sum_fparam0 += tb.getvarcol('FPARAM')['r1'][0][i][0]
            sum_fparam1 += tb.getvarcol('FPARAM')['r1'][1][i][0]
        fparam0_ave=sum_fparam0/128.0
        fparam1_ave=sum_fparam1/128.0
        print 'fparam_average_r1_0', fparam0_ave
        print 'fparam_average_r1_1', fparam1_ave
        print 'SIGMA00 ', sigma00
        print 'SIGMA10 ', sigma10
        print 'WEIGHT00 ', weight00
        print 'WEIGHT10 ', weight10
        answer0 = 1/(sigma00**2)*1/(fparam0_ave**2) 
        answer1 = 1/(sigma10**2)*1/(fparam1_ave**2) 
        print 'pol0: 1/SIGMA**2 X 1/(FPARAM_ave)**2', answer0
        print 'pol1: 1/SIGMA**2 X 1/(FPARAM_ave)**2', answer1
        diff0_percent=(weight00-answer0)/weight00*100
        diff1_percent=(weight10-answer1)/weight10*100
        print 'difference between fparam_r1_0 and weight00', diff0_percent, '%' 
        print 'difference between fparam_r1_1 and weight10', diff1_percent, '%'
        tb.close()
        

        #print type(fparam_dict)
        #print 'shape of fparam'
        #print 'shape of fparam_dict['r29']', fparam_dict['r29'].shape
        #print fparam_dict['r29'][0]
        #print fparam_dict['r29'][1]
        #tb.close()

        #tb.open(infile)

        #data_dict=tb.getvarcol('DATA')

        #subt=tb.query('', sortlist='ANTENNA1, TIME, SPECTRAL_WINDOW_ID', columns='FPARAM, DATA') 
        #data=subt2.getcol('DATA')
        #fparam=subt2.getcol('FPARAM')
        #print data[0]
        #print data[1]
        #print fparam[0]
        #print fparam[1]

        #subt_dict=tb.query('', sortlist='ANTENNA1, TIME', columns='WEIGHT, CORRECTED_DATA')
        #weight_dict = subt_dict.getcol('WEIGHT')
        #weight_dict=tb.getvarcol('WEIGHT')
        #print type(weight_dict)
        #print weight_dict['r69']
        #print weight_dict['r69'][0]
        #print weight_dict['r69'][1]
        #print weight_dict
        
        #corrected_data_dict = subt_dict.getcol('CORRECTED_DATA')
        #tb.close()
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


    def test03(self):
        """Test03: Validation of CORRECTED_DATA = DATA X FPARAM (spwmap={1:[1], 3:[3], 5:[5], 7:[7]})""" 

        tid ="03"
        infile=self.infile1
        tsysfile='tsys3.cal'
        
        
        #tsys table is produced 
        tsdcal(infile=infile, calmode='tsys', outfile=tsysfile)
        #spwmap=[0,1,2,3,4,5,6,7,8,1,10,3,12,5,14,7,16]
        spwmap={1:[1],3:[3],5:[5],7:[7]}
        initweights(vis=infile, wtmode='nyq', dowtsp=True)
        #tsdcal(infile=infile, calmode='apply', spwmap=spwmap, applytable=tsysfile, outfile='')
    
       
        tsdcal(infile=infile, calmode='apply', spwmap=spwmap, applytable=tsysfile)
       
                
        tb.open(infile)
        corrected_data=tb.getvarcol('CORRECTED_DATA')['r1'][0][0][0]
        data=tb.getvarcol('DATA')['r1'][0][0][0]
        tb.close()

         
        tb.open(tsysfile)
        fparam= tb.getvarcol('FPARAM')['r1'][0][0][0]
        tb.close()
                
        print "CORRECTED_DATA", corrected_data
        print "DATA", data
        print "FPARAM", fparam
        diff = corrected_data.real - (data.real*fparam)
        diff_per = (diff/corrected_data.real)*100 
        print "difference between CORRECTED_DATA and DATA X FPARAM", diff_per, "%" 
    
       
    def test04(self):
        """Test04: Validation of CORRECTED_DATA = DATA X FPARAM 
        (spwmap={1:[9], 3:[11], 5:[13], 7:[15]})
        antanna1=0, DATA_DISC_ID=9, FPARAM_average
        """
        
        
        tid ="04"
        infile=self.infile1
        tsysfile='tsys4.cal'
    
        #tsys table is produced 
        tsdcal(infile=infile, calmode='tsys', outfile=tsysfile)
        #spwmap=[0,1,2,3,4,5,6,7,8,1,10,3,12,5,14,7,16]
        spwmap={1:[9],3:[11],5:[13],7:[15]}
        initweights(vis=infile, wtmode='nyq', dowtsp=True)
        #tsdcal(infile=infile, calmode='apply', spwmap=spwmap, applytable=tsysfile, outfile='')       
        tsdcal(infile=infile, calmode='apply', spwmap=spwmap, applytable=tsysfile)       
             
        tb.open(infile)
        corrected_data=tb.getvarcol('CORRECTED_DATA')['r2'][0][0][0]
        data=tb.getvarcol('DATA')['r2'][0][0][0]
        tb.close()
         
        tb.open(tsysfile)
        sum_fparam=0
        for i in range(128):
            fparam= tb.getvarcol('FPARAM')['r1'][0][i][0]
            sum_fparam += fparam
        fparam_ave=sum_fparam/128.0    
        tb.close()
                
        print "CORRECTED_DATA", corrected_data
        print "DATA", data
        print "FPARAM average(128ch)", fparam_ave
        diff = corrected_data.real - (data.real*fparam_ave)
        diff_per = (diff/corrected_data.real)*100 
        print "difference between CORRECTED_DATA and DATA X FPARAM_average(128)", diff_per, "%" 
    


    def test05(self):
        """Test05: Validation of CORRECTED_DATA = DATA X FPARAM 
        (spwmap={1:[9], 3:[11], 5:[13], 7:[15]})
        antanna1=0, DATA_DISC_ID=9, FPARAM_average
        """
        print '' 
        
        tid ="05"
        infile=self.infile1
        tsysfile='tsys5.cal'
    
        #tsys table is produced 
        tsdcal(infile=infile, calmode='tsys', outfile=tsysfile)
        #spwmap=[0,1,2,3,4,5,6,7,8,1,10,3,12,5,14,7,16]
        spwmap={1:[9],3:[11],5:[13],7:[15]}
        initweights(vis=infile, wtmode='nyq', dowtsp=True)
        #tsdcal(infile=infile, calmode='apply', spwmap=spwmap, applytable=tsysfile, outfile='')       
        tsdcal(infile=infile, calmode='apply', spwmap=spwmap, applytable=tsysfile)       
             
        tb.open(infile)
        corrected_data=tb.getvarcol('CORRECTED_DATA')['r2'][0][0][0]
        data=tb.getvarcol('DATA')['r2'][0][0][0]
        tb.close()
         
        tb.open(tsysfile)
        fparam= tb.getvarcol('FPARAM')['r1'][0][0][0]
        tb.close()
                
        print "CORRECTED_DATA", corrected_data
        print "DATA", data
        print "FPARAM", fparam
        diff = corrected_data.real - (data.real*fparam)
        diff_per = (diff/corrected_data.real)*100 
        print "difference between CORRECTED_DATA and DATA X FPARAM", diff_per, "%" 
    


    def test06(self):
        """Test06: weight_spectrum = 1/(SIGMA**2) X 1/(FPARAMx**2) dictionary version"""
        #focus on antenna1=0, data_disk_id=1
        #spwmap_dict={1:[1],3:[3],5:[5],7:[7]}

        
        tid = "06"
        infile = self.infile1
        tsdcal(infile=infile, calmode='tsys', outfile='tsys6.cal')
        initweights(vis=infile, wtmode='nyq', dowtsp=True)        
        #spwmap_list=[0,1,2,3,4,5,6,7,8,1,10,3,12,5,14,7,16]
        #spwmap_dict={1:[9],3:[11],5:[13],7:[15]}
        
        spwmap_dict={1:[1],3:[3],5:[5],7:[7]}        
        tsdcal(infile=infile, calmode='apply', spwmap=spwmap_dict, applytable='tsys6.cal', interp='nearest', outfile='')
        
        row=0
        eps = 1.0e-1

        tb.open(infile)
        sigma=tb.getcell('SIGMA', row)
        weight_spectrum=tb.getcell('WEIGHT_SPECTRUM', row)
        total_ch=tb.getcell('WEIGHT_SPECTRUM',row).shape[1]
        tb.close()
        
        tb.open('tsys.cal')
        fparam=tb.getcell('FPARAM', row)
        for ch in range(total_ch):
            #print 'SIGMA00 ', sigma[0]
            #print 'SIGMA10 ', sigma[1]
            #print 'WEIGHT_SPECTRUM00 ', weight_spectrum[0][ch]
            #print 'WEIGHT_SPECTRUM10 ', weight_spectrum[1][ch]
            answer0 = 1/(sigma[0]**2)*1/(fparam[0][ch]**2) 
            answer1 = 1/(sigma[1]**2)*1/(fparam[1][ch]**2) 
            #print 'pol0: 1/SIGMA**2 X 1/(FPARAM)**2', answer0
            #print 'pol1: 1/SIGMA**2 X 1/(FPARAM)**2', answer1i
            diff0=weight_spectrum[0][ch]-answer0
            diff1=weight_spectrum[1][ch]-answer1
            diff0_percent= diff0/weight_spectrum[0][ch]*100
            diff1_percent= diff1/weight_spectrum[1][ch]*100

            #diff0_percent=(weight_spectrum[0][ch]-answer0)/weight_spectrum[0][ch]*100
            #diff1_percent=(weight_spectrum[1][ch]-answer1)/weight_spectrum[1][ch]*100
            print ''
            print 'pol0 & pol1 ch '+ str(ch)+ ': diff between 1/SIGMA**2 X 1/(FPARAM['+str(ch)+'])**2 and WEIGHT_SPECTRUM['+ str(ch)+']' , diff0, diff1
            print diff0_percent, '%', diff1_percent, '%'
            #self.assertTrue(diff0 < eps, msg='The error is small enough')
        tb.close()
            




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
                
class tsdcal_test_ps(tsdcal_test_base):   
    """
    Unit test for task tsdcal (position switchsky calibration).

    The list of tests:
    test_ps00 --- default parameters (raises an error)
    test_ps01 --- invalid calibration type
    test_ps02 --- invalid selection (empty selection result)
    test_ps03 --- outfile exists (overwrite=False)
    test_ps04 --- empty outfile
    test_ps05 --- position switch calibration ('ps')
    test_ps06 --- position switch calibration ('ps') with data selection
    test_ps07 --- outfile exists (overwrite=True)
    test_ps08 --- inappropriate calmode ('otfraster')
    """
    invalid_argument_case = tsdcal_test_base.invalid_argument_case
    exception_case = tsdcal_test_base.exception_case
    
    @property
    def outfile(self):
        return self.applytable

    def setUp(self):
        self._setUp([self.infile], tsdcal)

    def tearDown(self):
        self._tearDown([self.infile, self.outfile])

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
            
    
    @invalid_argument_case
    def test_ps00(self):
        """
        test_ps00 --- default parameters (raises an error)
        """
        self.result = tsdcal()

    @invalid_argument_case
    def test_ps01(self):
        """
        test_ps01 --- invalid calibration type
        """
        self.result = tsdcal(infile=self.infile, calmode='invalid_type', outfile=self.outfile)

    @exception_case(RuntimeError, 'No Spw ID\(s\) matched specifications')
    def test_ps02(self):
        """
        test_ps02 --- invalid selection (invalid spw selection)
        """
        self.result = tsdcal(infile=self.infile, calmode='ps', spw='99', outfile=self.outfile)

    @exception_case(RuntimeError, '^Output file \'.+\' exists\.$')
    def test_ps03(self):
        """
        test_ps03 --- outfile exists (overwrite=False)
        """
        # copy input to output
        shutil.copytree(self.infile, self.outfile)
        self.result = tsdcal(infile=self.infile, calmode='ps', outfile=self.outfile, overwrite=False)

    @exception_case(RuntimeError, 'Output file name must be specified\.')
    def test_ps04(self):
        """
        test_ps04 --- empty outfile 
        """
        self.result = tsdcal(infile=self.infile, calmode='ps', outfile='', overwrite=False)

    @normal_case()
    def test_ps05(self):
        """
        test_ps05 --- position switch calibration ('ps')
        """
        self.result = tsdcal(infile=self.infile, calmode='ps', outfile=self.outfile)

    @normal_case(spw='9')
    def test_ps06(self):
        """
        test_ps06 --- position switch calibration ('ps') with data selection
        """
        self.result = tsdcal(infile=self.infile, calmode='ps', spw='9', outfile=self.outfile)

    @normal_case()
    def test_ps07(self):
        """
        test_ps07 --- outfile exists (overwrite=True)
        """
        # copy input to output
        shutil.copytree(self.infile, self.outfile)
        self.result = tsdcal(infile=self.infile, calmode='ps', outfile=self.outfile, overwrite=True)

    @exception_case(RuntimeError, "Error in Calibrater::solve")
    def test_ps08(self):
        """
        test_ps08 --- inappropriate calmode ('otfraster')
        """
        # the data doesn't an OTF raster scan so that unexpected behavior may happen
        # if calmode is 'otfraster'
        # in this case, gap detection detects the row having only one integration
        # due to irregular time stamp distribution and causes the "Too many edge
        # points" error
        self.result = tsdcal(infile=self.infile, outfile=self.outfile, calmode='otfraster')


class tsdcal_test_otfraster(tsdcal_test_base):   
    """
    Unit test for task tsdcal (OTF raster sky calibration).
    Since basic test case is covered by tsdcal_test_ps, only
    tests specific to otfraster calibration are defined here.

    The list of tests:
    test_otfraster00 --- invalid fraction (non numeric value)
    test_otfraster01 --- too many edge points (fraction 0.5)
    test_otfraster02 --- too many edge points (fraction '50%')
    test_otfraster03 --- too many edge points (noff 100000)
    ###test_otfraster04 --- negative edge points 
    ###test_otfraster05 --- zero edge points 
    test_otfraster06 --- inappropriate calibration mode ('ps')
    test_otfraster07 --- OTF raster calibration ('otfraster') with default setting
    test_otfraster08 --- OTF raster calibration ('otfraster') with string fraction (numeric value)
    test_otfraster09 --- OTF raster calibration ('otfraster') with string fraction (percentage)
    test_otfraster10 --- OTF raster calibration ('otfraster') with numeric fraction 
    test_otfraster11 --- OTF raster calibration ('otfraster') with auto detection
    test_otfraster12 --- OTF raster calibration ('otfraster') with custom noff
    test_otfraster13 --- check if noff takes priority over fraction
    """
    invalid_argument_case = tsdcal_test_base.invalid_argument_case
    exception_case = tsdcal_test_base.exception_case
    infile = 'uid___A002_X6218fb_X264.ms.sel.otfraster'
    
    @staticmethod
    def calculate_expected_value(table, numedge=1):
        expected_value = {}
        with sdutil.tbmanager(table) as tb:
            antenna_list = numpy.unique(tb.getcol('ANTENNA1'))
            ddid_list = numpy.unique(tb.getcol('DATA_DESC_ID'))
        with sdutil.tbmanager(os.path.join(table,'DATA_DESCRIPTION')) as tb:
            dd_spw_map = tb.getcol('SPECTRAL_WINDOW_ID')
        for antenna in antenna_list:
            expected_value[antenna] = {}
            for ddid in ddid_list:
                spw = dd_spw_map[ddid]
                taql = 'ANTENNA1 == %s && ANTENNA2 == %s && DATA_DESC_ID == %s'%(antenna,antenna,ddid)
                with sdutil.tbmanager(table) as tb:
                    try:
                        tsel = tb.query(taql, sortlist='TIME')
                        time_list = tsel.getcol('TIME')
                        data = tsel.getcol('DATA').real
                        flag = tsel.getcol('FLAG')
                    finally:
                        tsel.close()
                #print 'time_list', time_list
                if len(time_list) < 2:
                    continue
                data_list = []
                time_difference = time_list[1:] - time_list[:-1]
                #print 'time_difference', time_difference
                gap_threshold = numpy.median(time_difference) * 5
                #print 'gap_threshold', gap_threshold
                gap_list = numpy.concatenate(([0], numpy.where(time_difference > gap_threshold)[0]+1))
                if gap_list[-1] != len(time_list):
                    gap_list = numpy.concatenate((gap_list, [len(time_list)]))
                #print 'gap_list', gap_list
                for i in xrange(len(gap_list)-1):
                    start = gap_list[i]
                    end = gap_list[i+1]
                    raster_data = data[:,:,start:end]
                    raster_flag = flag[:,:,start:end]
                    raster_row = numpy.ma.masked_array(raster_data, raster_flag)
                    left_edge = raster_row[:,:,:numedge].mean(axis=2)
                    right_edge = raster_row[:,:,-numedge:].mean(axis=2)
                    data_list.extend([left_edge, right_edge])
                expected_value[antenna][spw] = data_list
                #print 'antenna', antenna, 'spw', spw, 'len(data_list)', len(data_list)
                    
        return expected_value

    @property
    def outfile(self):
        return self.applytable

    def setUp(self):
        self._setUp([self.infile], tsdcal)

    def tearDown(self):
        self._tearDown([self.infile, self.outfile])

    def normal_case(numedge=1, **kwargs):
        """
        Decorator for the test case that is intended to verify
        normal execution result.

        numedge --- expected number of edge points
        selection --- data selection parameter as dictionary

        Here, expected result is as follows:
            - total number of rows is 24
            - number of antennas is 2
            - number of spectral windows is 2
            - each (antenna,spw) pair has 6 rows
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
                    expected_nrow = 24
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
                    expected_nrow = 6 * len(spw_selection) * len(antenna1_selection)
                with sdutil.tbmanager(self.outfile) as tb:
                    self.assertEqual(tb.nrows(), expected_nrow, msg='Number of rows mismatch (expected %s actual %s)'%(expected_nrow, tb.nrows()))

                # verifying resulting sky spectra
                eps = 1.0e-6
                expected_value = tsdcal_test_otfraster.calculate_expected_value(self.infile, numedge)
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
                            self.assertEqual(nrow, 6, msg='Number of rows mismatch')
                            for irow in xrange(tb.nrows()):
                                expected = val[irow]
                                fparam = tb.getcell('FPARAM', irow)
                                flag = tb.getcell('FLAG', irow)
                                self.assertEqual(expected.shape, fparam.shape, msg='Shape mismatch for antenna %s spw %s row %s (expected %s actual %s)'%(ant,spw,irow,list(expected.shape),list(fparam.shape)))
                                npol,nchan = expected.shape
                                for ipol in xrange(npol):
                                    for ichan in xrange(nchan):
                                        message_template = lambda x,y,z: 'Unexpected %s for antenna %s spw %s row %s pol %s channel %s (expected %s actual %s)'%(x,ant,spw,irow,ipol,ichan,y,z)
                                        _flag = flag[ipol,ichan]
                                        _mask = expected.mask[ipol,ichan]
                                        _expected = expected.data[ipol,ichan]
                                        _fparam = fparam[ipol,ichan]
                                        self.assertEqual(_mask, _flag, msg=message_template('FLAG',_mask,_flag))
                                        if _mask is True:
                                            self.assertEqual(0.0, _fparam, msg=message_template('FPARAM',0.0,_fparam))
                                        elif abs(_expected) < eps:
                                            self.assertLess(abs(_fparam), eps, msg=message_template('FPARAM',_expected,_fparam))
                                        else:
                                            diff = abs((_fparam - _expected) / _expected)
                                            self.assertLess(diff, eps, msg=message_template('FPARAM',_expected,_fparam))
                                #self.assertTrue(all(flag[:,:10].flatten() == True), msg=message_template('flag status', True))
                                #self.assertTrue(all(flag[:,10:].flatten() == False), msg=message_template('flag status', False))
                                #fparam_valid = fparam[flag == False]
                                #error = abs((fparam_valid - expected) / expected) 
                                #self.assertTrue(all(error < eps), msg=message_template('sky data', expected))
            return _wrapper
        return wrapper

    @exception_case(RuntimeError, '^Invalid fraction value \(.+\)$')
    def test_otfraster00(self):
        self.result = tsdcal(infile=self.infile, outfile=self.outfile,
                             calmode='otfraster', fraction='auto')

    @exception_case(ValueError, '^Too many edge points\. fraction must be < 0.5\.$')
    def test_otfraster01(self):
        """
        test_otfraster01 --- too many edge points (fraction 0.5)
        """
        self.result = tsdcal(infile=self.infile, outfile=self.outfile,
                             calmode='otfraster', fraction=0.5)

    @exception_case(ValueError, '^Too many edge points\. fraction must be < 0.5\.$')
    def test_otfraster02(self):
        """
        test_otfraster02 --- too many edge points (fraction 50%)
        """
        self.result = tsdcal(infile=self.infile, outfile=self.outfile,
                             calmode='otfraster', fraction='50%')

    @exception_case(RuntimeError, 'Error in Calibrater::solve')
    def test_otfraster03(self):
        """
        test_otfraster03 --- too many edge points (noff 100000)
        """
        self.result = tsdcal(infile=self.infile, outfile=self.outfile,
                             calmode='otfraster', noff=10000)

    #@exception_case(RuntimeError, 'Error in Calibrater::solve')
    #def test_otfraster04(self):
    #    """
    #    test_otfraster04 --- negative edge points
    #    """
    #    self.result = tsdcal(infile=self.infile, outfile=self.outfile,
    #                         calmode='otfraster', noff=-3)

    #@exception_case(RuntimeError, 'Error in Calibrater::solve')
    #def test_otfraster05(self):
    #    """
    #    test_otfraster05 --- zero edge points
    #    """
    #    self.result = tsdcal(infile=self.infile, outfile=self.outfile,
    #                         calmode='otfraster', noff=0)

    @exception_case(RuntimeError, 'Error in Calibrater::solve')
    def test_otfraster06(self):
        """
        test_otfraster06 --- inappropriate calibration mode ('ps')
        """
        self.result = tsdcal(infile=self.infile, outfile=self.outfile,
                             calmode='ps')
    @normal_case(numedge=1)
    def test_otfraster07(self):
        """
        test_otfraster07 --- OTF raster calibration ('otfraster') with default setting
        """
        self.result = tsdcal(infile=self.infile, outfile=self.outfile,
                             calmode='otfraster')

    @normal_case(numedge=2)
    def test_otfraster08(self):
        """
        test_otfraster08 --- OTF raster calibration ('otfraster') with string fraction (numeric value)
        """
        self.result = tsdcal(infile=self.infile, outfile=self.outfile,
                             calmode='otfraster', fraction='0.3')

    @normal_case(numedge=2)
    def test_otfraster09(self):
        """
        test_otfraster09 --- OTF raster calibration ('otfraster') with string fraction (percentage)
        """
        self.result = tsdcal(infile=self.infile, outfile=self.outfile,
                             calmode='otfraster', fraction='30%')

    @normal_case(numedge=2)
    def test_otfraster10(self):
        """
        test_otfraster10 --- OTF raster calibration ('otfraster') with numeric fraction
        """
        self.result = tsdcal(infile=self.infile, outfile=self.outfile,
                             calmode='otfraster', fraction=0.3)

    @normal_case(numedge=2)
    def test_otfraster11(self):
        """
        test_otfraster11 --- OTF raster calibration ('otfraster') with auto detection
        """
        self.result = tsdcal(infile=self.infile, outfile=self.outfile,
                             calmode='otfraster', fraction=0, noff=0)

    @normal_case(numedge=3)
    def test_otfraster12(self):
        """
        test_otfraster12 --- OTF raster calibration ('otfraster') with custom noff
        """
        self.result = tsdcal(infile=self.infile, outfile=self.outfile,
                             calmode='otfraster', noff=3)

    @normal_case(numedge=3)
    def test_otfraster13(self):
        """
        test_otfraster13 --- check if noff takes priority over fraction
        """
        self.result = tsdcal(infile=self.infile, outfile=self.outfile,
                             calmode='otfraster', fraction='90%', noff=3)
        

# interpolator utility for testing
class Interpolator(object):
    @staticmethod
    def __interp_freq_linear(data, flag):
        outdata = data.copy()
        outflag = flag
        npol, nchan = outdata.shape
        for ipol in xrange(npol):
            valid_chans = numpy.where(outflag[ipol,:] == False)[0]
            if len(valid_chans) == 0:
                continue
            for ichan in xrange(nchan):
                if outflag[ipol,ichan] == True:
                    #print '###', ipol, ichan, 'before', data[ipol,ichan]
                    if ichan <= valid_chans[0]:
                        outdata[ipol,ichan] = data[ipol,valid_chans[0]]
                    elif ichan >= valid_chans[-1]:
                        outdata[ipol,ichan] = data[ipol,valid_chans[-1]]
                    else:
                        ii = abs(valid_chans - ichan).argmin()
                        if valid_chans[ii] - ichan > 0:
                            ii -= 1
                        i0 = valid_chans[ii]
                        i1 = valid_chans[ii+1]
                        outdata[ipol,ichan] = ((i1 - ichan) * data[ipol,i0] + (ichan - i0) * data[ipol,i1]) / (i1 - i0)
                    #print '###', ipol, ichan, 'after', data[ipol,ichan]
        return outdata, outflag

    @staticmethod
    def interp_freq_linear(data, flag):
        outflag = flag.copy()
        outflag[:] = False
        outdata, outflag = Interpolator.__interp_freq_linear(data, outflag)
        return outdata, outflag
        
    @staticmethod
    def interp_freq_nearest(data, flag):
        outdata = data.copy()
        outflag = flag
        npol, nchan = outdata.shape
        for ipol in xrange(npol):
            valid_chans = numpy.where(outflag[ipol,:] == False)[0]
            if len(valid_chans) == 0:
                continue
            for ichan in xrange(nchan):
                if outflag[ipol,ichan] == True:
                    #print '###', ipol, ichan, 'before', data[ipol,ichan]
                    if ichan <= valid_chans[0]:
                        outdata[ipol,ichan] = data[ipol,valid_chans[0]]
                    elif ichan >= valid_chans[-1]:
                        outdata[ipol,ichan] = data[ipol,valid_chans[-1]]
                    else:
                        ii = abs(valid_chans - ichan).argmin()
                        outdata[ipol,ichan] = data[ipol,valid_chans[ii]]
                    #print '###', ipol, ichan, 'after', data[ipol,ichan]
        return outdata, outflag

    @staticmethod
    def interp_freq_linearflag(data, flag):
        # NOTE
        # interpolation/extrapolation of flag along frequency axis is
        # also needed for linear interpolation. Due to this, number of
        # flag channels will slightly increase and causes different
        # behavior from existing scantable based single dish task
        # (sdcal2).
        #
        # It appears that effective flag at a certain channel is set to
        # the flag at previous channels (except for channel 0).
        #
        # 2015/02/26 TN
        npol,nchan = flag.shape
        #print '###BEFORE', flag[:,:12]
        outflag = flag.copy()
        for ichan in xrange(nchan-1):
            outflag[:,ichan] = numpy.logical_or(flag[:,ichan], flag[:,ichan+1])
        outflag[:,1:] = outflag[:,:-1]
        outflag[:,-1] = flag[:,-2]
        #print '###AFTER', outflag[:,:12]

        outdata, outflag = Interpolator.__interp_freq_linear(data, outflag)
        return outdata, outflag

    @staticmethod
    def interp_freq_nearestflag(data, flag):
        outdata, outflag = Interpolator.interp_freq_nearest(data, flag)
        return outdata, outflag

    def __init__(self, table, finterp='linear'):
        self.table = table
        self.taql = ''
        self.time = None
        self.data = None
        self.flag = None
        self.exposure = None
        self.finterp = getattr(Interpolator,'interp_freq_%s'%(finterp.lower()))
        print 'self.finterp:', self.finterp.__name__

    def select(self, antenna, spw):
        self.taql = 'ANTENNA1 == %s && ANTENNA2 == %s && SPECTRAL_WINDOW_ID == %s'%(antenna, antenna, spw)
        with sdutil.table_selector(self.table, self.taql) as tb:
            self.time = tb.getcol('TIME')
            self.data = tb.getcol('FPARAM')
            self.flag = tb.getcol('FLAG')
            self.exposure = tb.getcol('INTERVAL')

    def interpolate(self, t):
        raise Exception('Not implemented')

    def weightscale_linear(self, dt_on, dt_off0, dt_off1=None, t_on=None, t_off0=None, t_off1=None):
        if dt_off1 is None:
            return self.weightscale_nearest(dt_on, dt_off0)
        else:
            delta = t_off1 - t_off0
            delta0 = t_on - t_off0
            delta1 = t_off1 - t_on
            sigmasqscale = 1.0 + dt_on / (delta * delta) * (delta1 * delta1 / dt_off0 + delta0 * delta0 / dt_off1) 
            return 1.0 / sigmasqscale 

    def weightscale_nearest(self, dt_on, dt_off):
        return dt_off / (dt_on + dt_off)

class LinearInterpolator(Interpolator):
    def __init__(self, table, finterp='linear'):
        super(LinearInterpolator, self).__init__(table, finterp)

    def interpolate(self, t, tau):
        dt = self.time - t
        index = abs(dt).argmin()
        if dt[index] > 0.0:
            index -= 1
        if index < 0:
            ref = self.data[:,:,0].copy()
            weightscale = self.weightscale_linear(tau, self.exposure[0])
        elif index >= len(self.time) - 1:
            ref = self.data[:,:,-1].copy()
            weightscale = self.weightscale_linear(tau, self.exposure[-1])
        else:
            t0 = self.time[index]
            t1 = self.time[index+1]
            d0 = self.data[:,:,index]
            d1 = self.data[:,:,index+1]
            dt0 = t - t0
            dt1 = t1 - t
            dt2 = t1 - t0
            ref = (dt1 * d0 + dt0 * d1) / dt2
            tau0 = self.exposure[index]
            tau1 = self.exposure[index+1]
            weightscale = self.weightscale_linear(tau, tau0, tau1, t, t0, t1)
        flag = self.interpolate_flag(t)
        ref, refflag = self.finterp(ref, flag)
                               
        return ref, refflag, weightscale
    
    def interpolate_flag(self, t):
        dt = self.time - t
        index = abs(dt).argmin()
        if dt[index] > 0.0:
            index -= 1
        if index < 0:
            flag = self.flag[:,:,0].copy()
        elif index >= len(self.time) - 1:
            flag = self.flag[:,:,-1].copy()
        else:
            f0 = self.flag[:,:,index]
            f1 = self.flag[:,:,index+1]
            flag = numpy.logical_or(f0, f1)

        return flag

class NearestInterpolator(Interpolator):
    def __init__(self, table, finterp='nearest'):
        super(NearestInterpolator, self).__init__(table, finterp)

    def interpolate(self, t, tau):
        dt = self.time - t
        index = abs(dt).argmin()
        weightscale = self.weightscale_nearest(tau, self.exposure[index])
        ref, refflag = self.finterp(self.data[:,:,index].copy(), self.flag[:,:,index].copy())
        return ref, refflag, weightscale

    
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
    test_apply_sky11 --- apply data (linearflag for frequency interpolation)
    test_apply_sky12 --- apply data (nearestflag for frequency interpolation)
    test_apply_sky13 --- apply data (string applytable input)
    test_apply_sky14 --- apply data (interp='')
    test_apply_sky15 --- check if WEIGHT_SPECTRUM is updated properly when it exists
    test_apply_sky16 --- apply both sky table and Tsys table simultaneously
    test_apply_composite00 --- on-the-fly application of sky table ('ps,apply')
    test_apply_composite01 --- on-the-fly application of sky table with existing Tsys table
    test_apply_composite02 --- on-the-fly application of sky and tsys tables ('ps,tsys,apply')
    test_apply_composite03 --- on-the-fly application of sky table ('otfraster,apply')
    """
    invalid_argument_case = tsdcal_test_base.invalid_argument_case
    exception_case = tsdcal_test_base.exception_case
    
    @property
    def nrow_per_chunk(self):
        # number of rows per antenna per spw is 18
        return 18

    @property
    def eps(self):
        # required accuracy is 2.0e-4
        return 3.0e-4
    
    def setUp(self):
        self._setUp([self.infile, self.applytable], tsdcal)


    def tearDown(self):
        self._tearDown([self.infile, self.applytable])

    def check_weight(self, inweight, outweight, scale):
        #print 'inweight', inweight
        #print 'outweight', outweight
        #print 'scale', scale
        # shape check
        self.assertEqual(inweight.shape, outweight.shape, msg='')
        
        # weight should not be zero
        self.assertFalse(any(inweight.flatten() == 0.0), msg='')
        self.assertFalse(any(outweight.flatten() == 0.0), msg='')

        # check difference
        expected_weight = inweight * scale
        diff = abs((outweight - expected_weight) / expected_weight)
        self.assertTrue(all(diff.flatten() < self.eps),
                        msg='')

    
    def normal_case(interp='linear', tsys=1.0, **kwargs):
        """
        Decorator for the test case that is intended to verify
        normal execution result.

        interp --- interpolation option ('linear', 'nearest', '*flag')
                   comma-separated list is allowed and it will be
                   interpreted as '<interp for time>,<intep for freq>'
        tsys --- tsys scaling factor
        selection --- data selection parameter as dictionary
        """
        def wrapper(func):
            import functools
            @functools.wraps(func)
            def _wrapper(self):
                # data selection 
                myms = gentools(['ms'])[0]
                myargs = kwargs.copy()
                if not myargs.has_key('baseline'):
                    with sdutil.tbmanager(self.infile) as tb:
                        antenna1 = numpy.unique(tb.getcol('ANTENNA1'))
                        myargs['baseline'] = '%s&&&'%(','.join(map(str,antenna1)))
                a = myms.msseltoindex(self.infile, **myargs)
                antennalist = a['antenna1']
                with sdutil.tbmanager(self.applytable) as tb:
                    spwlist = numpy.unique(tb.getcol('SPECTRAL_WINDOW_ID'))
                with sdutil.tbmanager(os.path.join(self.infile, 'DATA_DESCRIPTION')) as tb:
                    spwidcol = tb.getcol('SPECTRAL_WINDOW_ID').tolist()
                    spwddlist = map(spwidcol.index, spwlist)
                if len(a['spw']) > 0:
                    spwlist = list(set(spwlist) & set(a['spw']))
                    spwddlist = map(spwidcol.index, spwlist)

                # preserve original flag and weight
                flag_org = {}
                weight_org = {}
                weightsp_org = {}
                for antenna in antennalist:
                    flag_org[antenna] = {}
                    weight_org[antenna] = {}
                    weightsp_org[antenna] = {}
                    for (spw,spwdd) in zip(spwlist,spwddlist):
                        taql = 'ANTENNA1 == %s && ANTENNA2 == %s && DATA_DESC_ID == %s'%(antenna, antenna, spwdd)
                        with sdutil.table_selector(self.infile, taql) as tb:
                            flag_org[antenna][spw] = tb.getcol('FLAG')
                            weight_org[antenna][spw] = tb.getcol('WEIGHT')
                            if 'WEIGHT_SPECTRUM' in tb.colnames() and tb.iscelldefined('WEIGHT_SPECTRUM', 0):
                                #print 'WEIGHT_SPECTRUM is defined for antenna %s spw %s'%(antenna, spw)
                                weightsp_org[antenna][spw] = tb.getcol('WEIGHT_SPECTRUM')
                            #else:
                            #    print 'WEIGHT_SPECTRUM is NOT defined for antenna %s spw %s'%(antenna, spw)
                                
                # execute test
                func(self)

                # sanity check
                self.assertIsNone(self.result, msg='The task must complete without error')
                # verify if CORRECTED_DATA exists
                with sdutil.tbmanager(self.infile) as tb:
                    self.assertTrue('CORRECTED_DATA' in tb.colnames(), msg='CORRECTED_DATA column must be created after task execution!')

                # parse interp
                pos = interp.find(',')
                if pos == -1:
                    tinterp = interp.lower()
                    finterp = 'linearflag'
                else:
                    tinterp = interp[:pos].lower()
                    finterp = interp[pos+1:]
                if len(tinterp) == 0:
                    tinterp = 'linear'
                if len(finterp) == 0:
                    finterp = 'linearflag'
                
                # result depends on interp
                print 'Interpolation option:', tinterp, finterp
                self.assertTrue(tinterp in ['linear', 'nearest'], msg='Internal Error')
                if tinterp == 'linear':
                    interpolator = LinearInterpolator(self.applytable, finterp)
                else:
                    interpolator = NearestInterpolator(self.applytable, finterp)
                for antenna in antennalist:
                    for (spw,spwdd) in zip(spwlist,spwddlist):
                        interpolator.select(antenna, spw)
                        taql = 'ANTENNA1 == %s && ANTENNA2 == %s && DATA_DESC_ID == %s'%(antenna, antenna, spwdd)
                        with sdutil.table_selector(self.infile, taql) as tb:
                            self.assertEqual(tb.nrows(), self.nrow_per_chunk, msg='Number of rows mismatch in antenna %s spw %s'%(antenna, spw))
                            if weightsp_org[antenna].has_key(spw):
                                has_weightsp = True
                                
                            else:
                                has_weightsp = False
                            for irow in xrange(tb.nrows()):
                                t = tb.getcell('TIME', irow)
                                dt = tb.getcell('INTERVAL', irow)
                                data = tb.getcell('DATA', irow)
                                outflag = tb.getcell('FLAG', irow)
                                corrected = tb.getcell('CORRECTED_DATA', irow)
                                ref, calflag, weightscale = interpolator.interpolate(t, dt)
                                inflag = flag_org[antenna][spw][:,:,irow]
                                expected = tsys * (data - ref) / ref
                                expected_flag = numpy.logical_or(inflag, calflag)

                                # weight test
                                self.assertEqual(tb.iscelldefined('WEIGHT_SPECTRUM', irow), has_weightsp,
                                                 msg='')
                                inweight = weight_org[antenna][spw][:,irow]
                                outweight = tb.getcell('WEIGHT', irow)
                                tsyssq = tsys * tsys
                                if has_weightsp:
                                    # Need to check WEIGHT_SPECTRUM
                                    inweightsp = weightsp_org[antenna][spw][:,:,irow]
                                    outweightsp = tb.getcell('WEIGHT_SPECTRUM', irow)

                                    self.check_weight(inweight, outweight, weightscale / tsyssq)
                                    self.check_weight(inweightsp, outweightsp, weightscale / tsyssq)
                                else:
                                    self.check_weight(inweight, outweight, weightscale / tsyssq)
                                
                                #print 'antenna', antenna, 'spw', spw, 'row', irow
                                #print 'inflag', inflag[:,:12], 'calflag', calflag[:,:12], 'expflag', expected_flag[:,:12], 'outflag', outflag[:,:12]
                                #print 'ref', ref[:,126:130], 'data', data[:,126:130], 'expected', expected[:,126:130], 'corrected', corrected[:,126:130]
                                
                                self.assertEqual(corrected.shape, expected.shape, msg='Shape mismatch in antenna %s spw %s row %s (expeted %s actual %s)'%(antenna,spw,irow,list(expected.shape),list(corrected.shape)))
                                npol, nchan = corrected.shape

                                # verify data
                                diff = numpy.ones(expected.shape,dtype=float)
                                small_data = numpy.where(abs(expected) < 1.0e-7)
                                diff[small_data] = abs(corrected[small_data] - expected[small_data])
                                regular_data = numpy.where(abs(expected) >= 1.0e-7)
                                diff[regular_data] = abs((corrected[regular_data] - expected[regular_data]) / expected[regular_data])
                                self.assertTrue(all(diff.flatten() < self.eps), msg='Calibrated result differ in antenna %s spw %s row %s (expected %s actual %s diff %s)'%(antenna,spw,irow,expected,corrected,diff))
                                
                                
                                
                                # verify flag
                                self.assertTrue(all(outflag.flatten() == expected_flag.flatten()), msg='Resulting flag differ in antenna%s spw %s row %s (expected %s actual %s)'%(antenna,spw,irow,expected_flag,outflag))
                    
            return _wrapper
        return wrapper

    @exception_case(Exception, 'Applytable name must be specified.')
    def test_apply_sky00(self):
        """
        test_apply_sky00 --- empty applytable
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable='')

    @exception_case(Exception, 'Applytable name must be specified.')
    def test_apply_sky01(self):
        """
        test_apply_sky01 --- empty applytable (list ver.)
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=[''])

    @exception_case(Exception, 'Applytable name must be specified.')
    def test_apply_sky02(self):
        """
        test_apply_sky02 --- empty applytable list
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=[])

    @exception_case(Exception, '^Table \".+\" doesn\'t exist\.$')
    def test_apply_sky03(self):
        """
        test_apply_sky03 --- unexisting applytable
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable='notexist.sky')

    @exception_case(Exception, '^Table \".+\" doesn\'t exist\.$')
    def test_apply_sky04(self):
        """
        test_apply_sky04 --- unexisting applytable (list ver.)
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=['notexist.sky'])

    @exception_case(RuntimeError, 'No Spw ID\(s\) matched specifications')
    def test_apply_sky05(self):
        """
        test_apply_sky05 --- invalid selection (empty selection result)
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', spw='99', applytable=[self.applytable])
    
    #@exception_case(RuntimeError, '^Unknown interptype: \'.+\'!! Check inputs and try again\.$')
    @exception_case(RuntimeError, 'Error in Calibrater::setapply.')
    def test_apply_sky06(self):
        """
        test_apply_sky06 --- invalid interp value
        """
        # 'cubic' interpolation along time axis is not supported yet
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=[self.applytable], interp='cubic')
    
    @exception_case(RuntimeError, '^Applytable \'.+\' is not a caltable format$')
    def test_apply_sky07(self):
        """
        test_apply_sky07 --- invalid applytable (not caltable)
        """
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

    @normal_case(interp='linear,linearflag')
    def test_apply_sky11(self):
        """
        test_apply_sky11 --- apply data (linearflag for frequency interpolation)
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=[self.applytable], interp='linear,linearflag')
        
    @normal_case(interp='linear,nearestflag')
    def test_apply_sky12(self):
        """
        test_apply_sky12 --- apply data (nearestflag for frequency interpolation)
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=[self.applytable], interp='linear,nearestflag')
        
    @normal_case(interp='linear')
    def test_apply_sky13(self):
        """
        test_apply_sky13 --- apply data (string applytable input)
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=self.applytable, interp='linear')

    @normal_case(interp='')
    def test_apply_sky14(self):
        """
        test_apply_sky14 --- apply data (interp='')
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=self.applytable, interp='')

    def fill_weightspectrum(func):
        import functools
        @functools.wraps(func)
        def wrapper(self):
            with sdutil.tbmanager(self.infile, nomodify=False) as tb:
                self.assertTrue('WEIGHT_SPECTRUM' in tb.colnames(), msg='Internal Error')
                nrow = tb.nrows()
                for irow in xrange(nrow):
                    w = tb.getcell('WEIGHT', irow)
                    wsp = numpy.ones(tb.getcell('DATA', irow).shape, dtype=float)
                    for ipol in xrange(len(w)):
                        wsp[ipol,:] = w[ipol]
                    tb.putcell('WEIGHT_SPECTRUM', irow, wsp)
                    self.assertTrue(tb.iscelldefined('WEIGHT_SPECTRUM', irow), msg='Internal Error')
            func(self)
        return wrapper

    @fill_weightspectrum
    @normal_case(interp='linear')
    def test_apply_sky15(self):
        """
        test_apply_sky15 --- check if WEIGHT_SPECTRUM is updated properly when it exists
        """
        self.result = tsdcal(infile=self.infile, calmode='apply', applytable=self.applytable, interp='linear')

    def modify_tsys(func):
        import functools
        @functools.wraps(func)
        def wrapper(self):
            with sdutil.tbmanager(os.path.join(self.infile,'SYSCAL'), nomodify=False) as tb:
                tsel = tb.query('SPECTRAL_WINDOW_ID IN [1,3]', sortlist='ANTENNA_ID,SPECTRAL_WINDOW_ID,TIME')
                try:
                    nrow = tsel.nrows()
                    tsys = 100.0
                    for irow in xrange(nrow):
                        tsys_spectrum = tsel.getcell('TSYS_SPECTRUM', irow)
                        tsys_spectrum[:] = 100.0
                        tsel.putcell('TSYS_SPECTRUM', irow, tsys_spectrum)
                        #tsys += 100.0
                finally:
                    tsel.close()
            func(self)
        return wrapper

    @normal_case()
    def test_apply_composite00(self):
        """
        test_apply_composite00 --- on-the-fly application of sky table ('ps,apply')
        """
        tsdcal(infile=self.infile, calmode='ps,apply')

    @modify_tsys
    @normal_case(tsys=100.0)
    def test_apply_composite01(self):
        """
        test_apply_composite01 --- on-the-fly application of sky table with existing Tsys table
        """
        # generate Tsys table
        tsystable = self.infile.rstrip('/') + '.tsys'

        try:
            # generate Tsys table
            tsdcal(infile=self.infile, calmode='tsys', outfile=tsystable)
            
            # apply
            tsdcal(infile=self.infile, calmode='ps,apply', applytable=tsystable,
                   spwmap={1:[9], 3:[11]})
        finally:
            if os.path.exists(tsystable):
                shutil.rmtree(tsystable)

    @modify_tsys
    @normal_case(tsys=100.0)
    def test_apply_composite02(self):
        """
        test_apply_composite02 --- on-the-fly application of sky and tsys tables ('ps,tsys,apply')
        """
        tsdcal(infile=self.infile, calmode='ps,tsys,apply',
               spwmap={1:[9], 3:[11]})

    @modify_tsys
    @normal_case(tsys=100.0)
    def test_apply_composite03(self):
        """
        test_apply_composite03 --- on-the-fly application of sky table ('otfraster,apply')
        """
        tsdcal(infile=self.infile, calmode='tsys,apply', applytable=self.applytable,
               spwmap={1:[9], 3:[11]})

def suite():
    return [tsdcal_test, tsdcal_test_ps,
            tsdcal_test_otfraster, tsdcal_test_apply]


