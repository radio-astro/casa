import os
import sys
import shutil
import numpy
from __main__ import default
from tasks import *
from taskinit import *
import unittest
#
#import listing
#from numpy import array

try:
    import selection_syntax
except:
    import tests.selection_syntax as selection_syntax

import asap as sd
from sdreduce import sdreduce
#from sdstat import sdstat

class sdreduce_unittest_base:
    """
    Base class for sdreduce unit test
    """
    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdreduce/'
    sddatapath = os.environ.get('CASAPATH').split()[0] + \
                 '/data/regression/unittest/singledish/'
    taskname = "sdreduce"
    postfix='.red.asap'

    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertTrue(isthere,
                         msg='output file %s was not created because of the task failure'%(name))
    def _compare_with_tophat(self, name, ref_data, ref_idx=[], precision = 1.e-6):
        self._checkfile( name )
        sout = sd.scantable(name,average=False)
        nrow = sout.nrow()
        if len(ref_idx) == 0:
            ref_idx = range(nrow)

        self.assertEqual(nrow,len(ref_idx),"The rows in output table differs from the expected value.")
        for irow in range(nrow):
            y = sout._getspectrum(irow)
            nchan = len(y)
            # analytic solution
            curr_ref = ref_data[ ref_idx[irow] ]
            chanlist = curr_ref['channel']
            valuelist = curr_ref['value']
            yana = self._create_tophat_array(nchan, chanlist, valuelist)
            # compare
            rdiff = self._get_array_relative_diff(y, yana, precision)
            rdiff_max = max(abs(rdiff))
            self.assertTrue(rdiff_max < precision, "Maximum relative difference %f > %f" % (rdiff_max, precision))
    
    def _create_tophat_array(self, nchan, chanlist, valuelist):
        array_types = (tuple, list, numpy.ndarray)
        # check for inputs
        if nchan < 1:
            self.fail("Internal error. Number of channels should be > 0")
        if type(chanlist) not in array_types:
            self.fail("Internal error. Channel range list for reference data is not an array type")
        if type(chanlist[0]) not in array_types:
            chanlist = [ chanlist ]
        if type(valuelist) not in array_types:
            valuelist = [ valuelist ]
        nval = len(valuelist)
        nrange = len(chanlist)
        # generate reference data
        ref_data = numpy.zeros(nchan)
        for irange in range(nrange):
            curr_range = chanlist[irange]
            if type(curr_range) not in array_types or len(curr_range) < 2:
                self.fail("Internal error. Channel range list  for reference data should be a list of 2 elements arrays.")
            schan = curr_range[0]
            echan = curr_range[1]
            ref_data[schan:echan+1] = valuelist[irange % nval]
        return ref_data

    def _get_array_relative_diff(self, data, ref, precision=1.e-6):
        """
        Return an array of relative difference of elements in two arrays
        """
        precision = abs(precision)
        data_arr = numpy.array(data)
        ref_arr = numpy.array(ref)
        ref_denomi = numpy.array(ref)
        # a threshold to assume the value to be zero.
        almostzero = min(precision, max(abs(ref_arr))*precision)
        # set rdiff=0 for elements both data and reference are close to zero
        idx_ref = numpy.where(abs(ref_arr) < almostzero)
        idx_data = numpy.where(abs(data_arr) < almostzero)
        if len(idx_ref[0])>0 and len(idx_data[0])>0:
            idx = numpy.intersect1d(idx_data[0], idx_ref[0], assume_unique=True)
            ref_arr[idx] = almostzero
            data_arr[idx] = almostzero
        # prevent zero division
        ref_denomi[idx_ref] = almostzero
        return (data_arr-ref_arr)/ref_denomi

class sdreduce_test(unittest.TestCase):
    """
    Basic unit tests for task sdreduce. No interactive testing.

    The list of tests:
    test00    --- default parameters (raises an errror)
    test01    --- Default parameters + valid input filename
    test02    --- operate all 3 steps (mostly with default parameters)
    test03    --- explicitly specify all parameters
    test04-07 --- do one of calibration, average, baseline, or smooth
    test08-10 --- skip one of of calibration and average, baseline, or smooth
    
    Note: input data (infile0) is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdsave)
      sdsave(infile='OrionS_rawACSmod',outfile=self.infile0,outform='ASAP')
    -> Just converted to scantable to eliminate errors by data conversion.
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdreduce/'
    # Input and output names
    # uncalibrated data
    infile0 = 'OrionS_rawACSmod.asap'
    # uncalibrated data
    infile1 = 'OrionS_rawACSmod_cal2123.asap'
    infiles = [infile0, infile1]
    outroot = 'sdreduce_test'

    def setUp(self):
        for file in self.infiles:
            if os.path.exists(file):
                shutil.rmtree(file)
            shutil.copytree(self.datapath+file, file)
        default(sdreduce)

    def tearDown(self):
        for file in self.infiles:
            if (os.path.exists(file)):
                shutil.rmtree(file)

    def _row0_stats(self,file):
        scan = sd.scantable(file,average=False)
        stats=["max","min","sum","rms","stddev","max_abc","min_abc"]
        edge = 500
        chanrange = [edge, scan.nchan()-edge-1]
        mask = scan.create_mask(chanrange)
        statdict = {}
        for stat in stats:
            statdict[stat] = scan.stats(stat,mask=mask,row=0)[0]
        del scan
        print "\nCurrent run: "+str(statdict)
        return statdict

    def _teststats0(self,teststat,refstat):
        for stat, refval in refstat.iteritems():
            self.assertTrue(teststat.has_key(stat),
                            msg = "'%s' is not defined in the current run" % stat)
            allowdiff = 0.01
            reldiff = (teststat[stat]-refval)/refval
            #print "Comparing '%s': %f (current run), %f (reference)" % \
            #      (stat,teststat[stat],refval)
            self.assertTrue(abs(reldiff) < allowdiff,\
                            msg="'%s' differs: %f (ref) != %f" % \
                            (stat, refval, teststat[stat]))

    def _run_sdreduce_test(self, row0stats, **kwargs):
        """
        Invoke sdreduce with given keyword arguments and
        compare statistics of spectrum in the first row of output
        scantable given as the argument.
        """
        outfile = kwargs['outfile']
        result = sdreduce(**kwargs)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,row0stats)
        

    def test00(self):
        """Test 0: Default parameters (raises an errror)"""
        #print blfunc
        result = sdreduce()
        self.assertFalse(result)

    def test01(self):
        """Test 1: Default parameters + valid input filename (do nothing)"""
        self.tid="01"
        infile = self.infile0
        outfile = self.outroot+self.tid+'.asap'
        refstat = {'rms': 0.55389463901519775, 'min': 0.26541909575462341,
                   'max_abc': 773.0, 'max': 0.91243284940719604,
                   'sum': 3802.1259765625, 'stddev': 0.16529126465320587,
                   'min_abc': 7356.0}
        self._run_sdreduce_test(refstat, infile=infile,outfile=outfile)

    def test02(self):
        """
        Test 2: operate all steps (mostly with default parameters)
        testing if default parameter values are changed
        """
        # Don't average GBT ps data at the same time of calibration.
        self.tid="02"
        infile = self.infile0
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'ps'
        kernel = 'hanning'
        blfunc='poly'
        refstat = {'rms': 0.21985267102718353, 'min': -0.7019438743591309,
                   'max_abc': 4093.0, 'max': 0.9684028029441833,
                   'sum': 5.486020088195801, 'stddev': 0.21986639499664307,
                   'min_abc': 7623.0}
        self._run_sdreduce_test(refstat, infile=infile,outfile=outfile,
                                calmode=calmode,kernel=kernel,blfunc=blfunc)
        

    def test03(self):
        """
        Test 3:  explicitly specify all parameters
        testing if parameter names are changed
        """
        self.tid="03"
        infile = self.infile0
        outfile = self.outroot+self.tid+'.asap'

        result = sdreduce(infile=infile,
                          antenna=0,
                          fluxunit='K',
                          telescopeparam='',
                          #specunit='GHz',
                          field='OrionS*',
                          spw='0',
                          frame='',
                          doppler='',
                          timerange='',
                          scan='20~23',
                          pol='',
                          calmode='ps',
                          fraction='10%',
                          noff=-1,
                          width=0.5,
                          elongated=False,
                          markonly=False,
                          plotpointings=False,
                          tau=0.0,
                          average=False,
                          scanaverage=False,
                          timeaverage=False,
                          tweight='tintsys',
                          averageall=False,
                          polaverage=False,
                          pweight='tsys',
                          kernel='hanning',
                          kwidth=5,
                          chanwidth='5',
                          maskmode='auto',
                          thresh=5.0,
                          avg_limit=4,
                          edge=[0],
                          blfunc='poly',
                          order=5,
                          npiece=2,
                          applyfft=True,
                          fftmethod='fft',
                          fftthresh=3.0,
                          addwn=[0.],
                          rejwn=[],
                          clipthresh=3.0,
                          clipniter=1,
                          verifycal=False,
                          verifysm=False,
                          verifybl=False,
                          verbosebl=True,
                          bloutput=True,
                          blformat='',
                          showprogress=True,
                          minnrow=1000,
                          outfile=outfile,
                          outform='ASAP',
                          overwrite=False,
                          plotlevel=0)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")

    def test04(self):
        """ Test 4:  operate only calibration step """
        self.tid="04"
        infile = self.infile0
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'ps'
        refstat = {'rms': 2.1299083232879639, 'min': 1.2246102094650269,
                   'max_abc': 4093.0, 'max': 3.1902554035186768,
                   'sum': 15209.119140625, 'stddev': 0.25390961766242981,
                   'min_abc': 7434.0}
        self._run_sdreduce_test(refstat, infile=infile,outfile=outfile,
                                calmode=calmode)

    def test05(self):
        """ Test 5:  operate only averaging step """
        self.tid="05"
        infile = self.infile1
        outfile = self.outroot+self.tid+'.asap'
        average = True
        # need to run one of average
        timeaverage = True
        tweight = 'tintsys'
        scanaverage = True
        refstat = {'rms': 4.128033638000488, 'min': 3.2304768562316895,
                   'max_abc': 4093.0, 'max': 5.676198482513428,
                   'sum': 29638.3515625, 'stddev': 0.24048194289207458,
                   'min_abc': 2452.0}
        self._run_sdreduce_test(refstat, infile=infile,outfile=outfile,
                                average=average,timeaverage=timeaverage,
                                tweight = 'tintsys',scanaverage=scanaverage)

    def test06(self):
        """ Test 6:  operate only smoothing step """
        self.tid="06"
        infile = self.infile1
        outfile = self.outroot+self.tid+'.asap'
        kernel = 'hanning'
        refstat = {'rms': 3.5979659557342529, 'min': 2.3542881011962891,
                   'max_abc': 4093.0, 'max': 5.2421674728393555,
                   'sum': 25737.166015625, 'stddev': 0.37295544147491455,
                   'min_abc': 6472.0}
        self._run_sdreduce_test(refstat, infile=infile,outfile=outfile,
                                kernel=kernel)

    def test07(self):
        """ Test 7:  operate only baseline step """
        self.tid="07"
        infile = self.infile1
        outfile = self.outroot+self.tid+'.asap'
        blfunc = 'poly'
        refstat = {'rms': 0.4292968213558197, 'min': -1.4878684282302856,
                   'max_abc': 4093.0, 'max': 1.8000496625900269,
                   'sum': 6.965803146362305, 'stddev': 0.42932620644569397,
                   'min_abc': 7434.0}
        self._run_sdreduce_test(refstat, infile=infile,outfile=outfile,
                                blfunc=blfunc)

    def test08(self):
        """ Test 8:  skip calibration and averaging """
        self.tid="08"
        infile = self.infile1
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'none'
        average = False
        kernel = 'hanning'
        blfunc = 'poly'
        refstat = {'rms': 0.37204012274742126, 'min': -1.1878470182418823,
                   'max_abc': 4093.0, 'max': 1.638755202293396,
                   'sum': 9.283474922180176, 'stddev': 0.37206435203552246,
                   'min_abc': 7623.0}
        self._run_sdreduce_test(refstat, infile=infile,outfile=outfile,
                                calmode=calmode,average=average,
                                kernel=kernel,blfunc=blfunc)

    def test09(self):
        """ Test 9:  skip smoothing"""
        self.tid="09"
        infile = self.infile0
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'ps'
        kernel = 'none'
        blfunc = 'poly'
        refstat = {'rms': 0.2536879777908325, 'min': -0.8792380690574646,
                   'max_abc': 4093.0, 'max': 1.0637180805206299,
                   'sum': 4.116354942321777, 'stddev': 0.2537047564983368,
                   'min_abc': 7434.0}
        self._run_sdreduce_test(refstat, infile=infile,outfile=outfile,
                                calmode=calmode,kernel=kernel,blfunc=blfunc)

    def test10(self):
        """ Test 10:  skip baseline"""
        self.tid="10"
        infile = self.infile0
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'ps'
        kernel = 'hanning'
        blfunc = 'none'
        refstat = {'rms': 2.126171350479126, 'min': 1.3912382125854492,
                   'max_abc': 4093.0, 'max': 3.0977959632873535,
                   'sum': 15209.0869140625, 'stddev': 0.2203933447599411,
                   'min_abc': 6472.0}
        self._run_sdreduce_test(refstat, infile=infile,outfile=outfile,
                                calmode=calmode,kernel=kernel,blfunc=blfunc)

class sdreduce_selection(selection_syntax.SelectionSyntaxTest,
                         sdreduce_unittest_base,unittest.TestCase):
    """
    Test selection syntax in sdreduce.
    Selection parameters to test are:
    field, spw (with channel selection), timerange, scan, pol

    For tests with channel selection, scanaveraging (no effect) + baseline
    is invoked and the results are compared with analytic solution.
    For tests without channel selection, only smoothing is invoked
    and the results are compared with analytic solution.
    In both cases calibration is omitted due to limitation of sdcal.
    """
    rawfile_raw = 'sd_analytic_type1-3.asap'
    rawfile_cal = 'sd_analytic_type1-3.cal.asap'
    #rawfile_2 = 'sd_analytic_type2-1.asap'
    rawfile_bl = 'sd_analytic_type1-3.bl.asap'
    rawfiles = [rawfile_raw, rawfile_cal, rawfile_bl]
    prefix = sdreduce_unittest_base.taskname+'TestSel'
    default_param = {'calmode': 'none',
                     'average': False, 'timeaverage': False,
                     'scanaverage': False, 'polaverage': False,
                     'kernel': 'none', 'kwidth': 5, 
                     'maskmode': 'auto', 'blfunc': 'none','order': 3,
                     'outfile': prefix+sdreduce_unittest_base.postfix,
                     'outform': 'ASAP', 'overwrite': False}

    refval_sm = ({'value': 1.0, 'channel': (17,21)},
                 {'value': 2.0, 'channel': (37,41)},
                 {'value': 4.0, 'channel': (57,61)},
                 {'value': 6.0, 'channel': (77,81)},)
    refval_bl = ({'value': 5,  'channel': (20,20)},
                 {'value': 10, 'channel': (40,40)},
                 {'value': 20, 'channel': (60,60)},
                 {'value': 30, 'channel': (80,80)},)

    @property
    def task(self):
        return sdreduce
    
    @property
    def spw_channel_selection(self):
        return True

    def setUp(self):
        self.res=None
        for name in self.rawfiles:
            if os.path.exists(name): shutil.rmtree(name)
            shutil.copytree(self.sddatapath+name, name)
        os.system( 'rm -rf '+self.prefix+'*' )
        
        default(sdreduce)

    def tearDown(self):
        for name in self.rawfiles:
            if os.path.exists(name): shutil.rmtree(name)

        os.system( 'rm -rf '+self.prefix+'*' )

    ####################
    # Additional tests
    ####################
    # N/A

    ####################
    # scan
    ####################
    def test_scan_id_default(self):
        """test scan selection (scan='')"""
        scan = ''
        ref_idx = []
        self._run_sdreduce_test(ref_idx,chansel=False, scan=scan)

    def test_scan_id_exact(self):
        """test scan selection (scan='16')"""
        scan = '16'
        ref_idx = [1,2]
        self._run_sdreduce_test(ref_idx,chansel=False, scan=scan)

    def test_scan_id_lt(self):
        """test scan selection (scan='<16')"""
        scan = '<16'
        ref_idx = [0]
        self._run_sdreduce_test(ref_idx,chansel=False, scan=scan)

    def test_scan_id_gt(self):
        """test scan selection (scan='>16')"""
        scan = '>16'
        ref_idx = [3]
        self._run_sdreduce_test(ref_idx,chansel=False, scan=scan)

    def test_scan_id_range(self):
        """test scan selection (scan='16~17')"""
        scan = '16~17'
        ref_idx = [1,2,3]
        self._run_sdreduce_test(ref_idx,chansel=False, scan=scan)

    def test_scan_id_list(self):
        """test scan selection (scan='15,17')"""
        scan = '15,17'
        ref_idx = [0,3]
        self._run_sdreduce_test(ref_idx,chansel=False, scan=scan)

    def test_scan_id_exprlist(self):
        """test scan selection (scan='15,>16')"""
        scan = '15,>16'
        ref_idx = [0,3]
        self._run_sdreduce_test(ref_idx,chansel=False, scan=scan)

    ####################
    # pol
    ####################
    def test_pol_id_default(self):
        """test pol selection (pol='')"""
        pol = ''
        ref_idx = []
        self._run_sdreduce_test(ref_idx,chansel=False, pol=pol)

    def test_pol_id_exact(self):
        """test pol selection (pol='1')"""
        pol = '1'
        ref_idx = [1,3]
        self._run_sdreduce_test(ref_idx,chansel=False, pol=pol)

    def test_pol_id_lt(self):
        """test pol selection (pol='<1')"""
        pol = '<1'
        ref_idx = [0,2]
        self._run_sdreduce_test(ref_idx,chansel=False, pol=pol)

    def test_pol_id_gt(self):
        """test pol selection (pol='>0')"""
        pol = '>0'
        ref_idx = [1,3]
        self._run_sdreduce_test(ref_idx,chansel=False, pol=pol)

    def test_pol_id_range(self):
        """test pol selection (pol='0~1')"""
        pol = '0~1'
        ref_idx = []
        self._run_sdreduce_test(ref_idx,chansel=False, pol=pol)

    def test_pol_id_list(self):
        """test pol selection (pol='0,1')"""
        pol = '0,1'
        ref_idx = []
        self._run_sdreduce_test(ref_idx,chansel=False, pol=pol)

    def test_pol_id_exprlist(self):
        """test pol selection (pol='0,>0')"""
        pol = '0,>0'
        ref_idx = []
        self._run_sdreduce_test(ref_idx,chansel=False, pol=pol)

    ####################
    # field
    ####################
    def test_field_value_default(self):
        """test field selection (field='')"""
        field = ''
        ref_idx = []
        self._run_sdreduce_test(ref_idx,chansel=False, field=field)

    def test_field_id_exact(self):
        """test field selection (field='6')"""
        field = '6'
        ref_idx = [1]
        self._run_sdreduce_test(ref_idx,chansel=False, field=field)

    def test_field_id_lt(self):
        """test field selection (field='<6')"""
        field = '<6'
        ref_idx = [0]
        self._run_sdreduce_test(ref_idx,chansel=False, field=field)

    def test_field_id_gt(self):
        """test field selection (field='>7')"""
        field = '>7'
        ref_idx = [3]
        self._run_sdreduce_test(ref_idx,chansel=False, field=field)

    def test_field_id_range(self):
        """test field selection (field='6~8')"""
        field = '6~8'
        ref_idx = [1,2,3]
        self._run_sdreduce_test(ref_idx,chansel=False, field=field)

    def test_field_id_list(self):
        """test field selection (field='5,8')"""
        field = '5,8'
        ref_idx = [0,3]
        self._run_sdreduce_test(ref_idx,chansel=False, field=field)

    def test_field_id_exprlist(self):
        """test field selection (field='5,>7')"""
        field = '5,>7'
        ref_idx = [0,3]
        self._run_sdreduce_test(ref_idx,chansel=False, field=field)

    def test_field_value_exact(self):
        """test field selection (field='M30')"""
        field = 'M30'
        ref_idx = [2]
        self._run_sdreduce_test(ref_idx,chansel=False, field=field)

    def test_field_value_pattern(self):
        """test field selection (field='M*')"""
        field = 'M*'
        ref_idx = [0,1,2]
        self._run_sdreduce_test(ref_idx,chansel=False, field=field)

    def test_field_value_list(self):
        """test field selection (field='3C273,M30')"""
        field = '3C273,M30'
        ref_idx = [2,3]
        self._run_sdreduce_test(ref_idx,chansel=False, field=field)

    def test_field_mix_exprlist(self):
        """test field selection (field='<6,3*')"""
        field = '<6,3*'
        ref_idx = [0,3]
        self._run_sdreduce_test(ref_idx,chansel=False, field=field)

    ####################
    # spw 
    ####################
    def test_spw_id_default(self):
        """test spw selection (spw='', all channels)"""
        spw = ''
        ref_idx = []
        self._run_sdreduce_test(ref_idx,chansel=False, spw=spw)

    def test_spw_id_exact(self):
        """test spw selection (spw='23', all channels)"""
        spw = '23'
        ref_idx = [0,3]
        self._run_sdreduce_test(ref_idx,chansel=False, spw=spw)

    def test_spw_id_lt(self):
        """test spw selection (spw='<23', all channels)"""
        spw = '<23'
        ref_idx = [2]
        self._run_sdreduce_test(ref_idx,chansel=False, spw=spw)

    def test_spw_id_gt(self):
        """test spw selection (spw='>23', all channels)"""
        spw = '>23'
        ref_idx = [1]
        self._run_sdreduce_test(ref_idx,chansel=False, spw=spw)

    def test_spw_id_range(self):
        """test spw selection (spw='23~25', all channels)"""
        spw = '23~25'
        ref_idx = [0,1,3]
        self._run_sdreduce_test(ref_idx,chansel=False, spw=spw)

    def test_spw_id_list(self):
        """test spw selection (spw='21,25', all channels)"""
        spw = '21,25'
        ref_idx = [1,2]
        self._run_sdreduce_test(ref_idx,chansel=False, spw=spw)

    def test_spw_id_exprlist(self):
        """test spw selection (spw='23,>24', all channels)"""
        spw = '23,>24'
        ref_idx = [0,1,3]
        self._run_sdreduce_test(ref_idx,chansel=False, spw=spw)

    def test_spw_id_pattern(self):
        """test spw selection (spw='*', all channels)"""
        spw = '*'
        ref_idx = []
        self._run_sdreduce_test(ref_idx,chansel=False, spw=spw)

    def test_spw_value_frequency(self):
        """test spw selection (spw='299.52~300.47GHz', all channels)"""
        spw = '299.52~300.47GHz' # IFNO=23 will be selected
        ref_idx = [0,3]
        self._run_sdreduce_test(ref_idx,chansel=False, spw=spw)

    def test_spw_value_velocity(self):
        """test spw selection (spw='-510.~470.km/s', all channels)"""
        spw = '-510.~470km/s' # IFNO=23,25 will be selected
        ref_idx = [0,1,3]
        self._run_sdreduce_test(ref_idx,chansel=False, spw=spw)

    def test_spw_mix_exprlist(self):
        """test spw selection (spw='25,0~501km/s', all channels)"""
        spw = '25,0~501km/s' # all IFs will be selected
        ref_idx = []
        self._run_sdreduce_test(ref_idx,chansel=False, spw=spw)

    #########################
    # spw with channel range
    #########################
    def test_spw_id_default_channel(self):
        """test spw selection w/ channel selection (spw=':21~39')"""
        spw = ':21~39'
        ref_idx = []
        self._run_sdreduce_test(ref_idx,precision=2.e-5,chansel=True, spw=spw)

    def test_spw_id_default_frequency(self):
        """test spw selection w/ channel selection (spw=':300.491~300.549GHz')"""
        spw = ':300.491~300.549GHz' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_id_default_velocity(self):
        """test spw selection w/ channel selection (spw=':-548.7~-490.5km/s')"""
        spw = ':-548.7~-490.5km/s' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_id_default_list(self):
        """test spw selection w/ channel selection (spw=':0~19;21~39;41~59;61~79;81~100')"""
        spw = ':0~19;21~39;41~59;61~79;81~100'
        ref_idx = []
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_id_exact_channel(self):
        """test spw selection w/ channel selection (spw='23:21~79')"""
        spw = '23:21~79'
        ref_idx = [0,3]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_id_exact_frequency(self):
        """test spw selection w/ channel selection (spw='25:300.491~300.549GHz')"""
        spw = '25:300.491~300.549GHz' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_id_exact_velocity(self):
        """test spw selection w/ channel selection (spw='25:-548.7~-490.5km/s')"""
        spw = '25:-548.7~-490.5km/s' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_id_exact_list(self):
        """test spw selection w/ channel selection (spw='23:0~19;21~79;81~100')"""
        spw = '23:0~19;21~79;81~100'
        ref_idx = [0,3]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_id_pattern_channel(self):
        """test spw selection w/ channel selection (spw='*:21~39')"""
        spw = '*:21~39'
        ref_idx = []
        self._run_sdreduce_test(ref_idx,precision=2.e-5,chansel=True, spw=spw)

    def test_spw_id_pattern_frequency(self):
        """test spw selection w/ channel selection (spw='*:300.491~300.549GHz')"""
        spw = '*:300.491~300.549GHz' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_id_pattern_velocity(self):
        """test spw selection w/ channel selection (spw='*:-548.7~-490.5km/s')"""
        spw = '*:-548.7~-490.5km/s' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_id_pattern_list(self):
        """test spw selection w/ channel selection (spw='*:0~19;21~39;41~59;61~79;81~100')"""
        spw = '*:0~19;21~39;41~59;61~79;81~100'
        ref_idx = []
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_value_frequency_channel(self):
        """test spw selection w/ channel selection (spw='299.9~300.1GHz:21~79')"""
        spw = '299.9~300.1GHz:21~79' # IFNO=23
        ref_idx = [0,3]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_value_frequency_frequency(self):
        """test spw selection w/ channel selection (spw='300.4~300.6GHz:300.491~300.549GHz')"""
        spw = '300.4~300.6GHz:300.491~300.549GHz' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_value_frequency_velocity(self):
        """test spw selection w/ channel selection (spw='300.4~300.6GHz:-548.7~-490.5km/s')"""
        spw = '300.4~300.6GHz:-548.7~-490.5km/s' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_value_frequency_list(self):
        """test spw selection w/ channel selection (spw='299.9~300.1GHz:0~19;21~79;81~100')"""
        spw = '299.9~300.1GHz:0~19;21~79;81~100' # IFNO=23
        ref_idx = [0,3]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_value_velocity_channel(self):
        """test spw selection w/ channel selection (spw='-30~30km/s:21~79')"""
        spw = '-30~30km/s:21~79' # IFNO=23
        ref_idx = [0,3]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_value_velocity_frequency(self):
        """test spw selection w/ channel selection (spw='-500~-499km/s:-548.7~-490.5km/s')"""
        spw = '-500~-499km/s:-548.7~-490.5km/s' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_value_velocity_velocity(self):
        """test spw selection w/ channel selection (spw='-500~-499km/s:-548.7~-490.5km/s')"""
        spw = '-500~-499km/s:-548.7~-490.5km/s' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_value_velocity_list(self):
        """test spw selection w/ channel selection (spw='-30~30km/s:0~19;21~79;81~100')"""
        spw = '-30~30km/s:0~19;21~79;81~100' # IFNO=23
        ref_idx = [0,3]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    def test_spw_id_list_channel(self):
        """test spw selection w/ channel selection (spw='21:0~59,23:21~79')"""
        spw = '21:0~59,23:21~79'
        ref_idx = [0,2,3]
        self._run_sdreduce_test(ref_idx,chansel=True, spw=spw)

    ####################
    # timerange
    ####################
    def test_timerange_value_default(self):
        """test timerange selection (timerange='')"""
        timerange = ''
        ref_idx = []
        self._run_sdreduce_test(ref_idx,chansel=False, timerange=timerange)

    def test_timerange_value_exact(self):
        """test timerange selection (timerange='2011/11/11/02:33:03.47')"""
        timerange = '<2011/11/11/02:33:03.47' # SCANNO=15 will be selected
        ref_idx = [0]
        self._run_sdreduce_test(ref_idx,chansel=False, timerange=timerange)

    def test_timerange_value_lt(self):
        """test timerange selection (timerange='<2011/11/11/02:33:03.47')"""
        timerange = '<2011/11/11/02:33:03.47' # SCANNO=15 will be selected
        ref_idx = [0]
        self._run_sdreduce_test(ref_idx,chansel=False, timerange=timerange)

    def test_timerange_value_gt(self):
        """test timerange selection (timerange='>2011/11/11/02:33:03.5')"""
        timerange = '>2011/11/11/02:33:03.5' # SCANNO=17 will be selected
        ref_idx = [3]
        self._run_sdreduce_test(ref_idx,chansel=False, timerange=timerange)

    def test_timerange_value_range(self):
        """test timerange selection (timerange='2011/11/11/02:33:03.47~02:34:03.48')"""
        timerange = '2011/11/11/02:33:03.47~02:34:03.48' # SCANNO=16,17 will be selected
        ref_idx = [1,2,3]
        self._run_sdreduce_test(ref_idx,chansel=False, timerange=timerange)

    def test_timerange_value_interval(self):
        """test timerange selection (timerange='2011/11/11/02:33:03.47+00:01:00.1')"""
        timerange = '2011/11/11/02:33:03.47+00:01:00.1' # SCANNO=16,17 will be selected
        ref_idx = [1,2,3]
        self._run_sdreduce_test(ref_idx,chansel=False, timerange=timerange)

    ####################
    # Helper functions
    ####################
    def _run_sdreduce_test(self, ref_idx, precision=1.e-6, chansel=False, **kwargs):
        # chansel = T (channel selection: average+baseline)
        #           F (no channel selection: only smoothing)
        params = self.default_param.copy()
        if chansel: # averaging and baseline
            params['infile'] = self.rawfile_cal
            params['average'] = True
            params['timeaverage'] = True
            params['polaverage'] = False
            params['blfunc'] = 'poly'
            params['maskmode'] = 'list'
            ref_data = self.refval_bl
        else: # smoothing only
            params['infile'] = self.rawfile_bl
            params['kernel'] = 'boxcar'
            params['kwidth'] = 5
            ref_data = self.refval_sm
        for k,v in kwargs.items(): params[k] = v
        # Invoke task
        self.res = self.run_task(**params)
        # Test result
        self._compare_with_tophat(params['outfile'], ref_data, ref_idx, precision=precision)
        
class sdreduce_storage_test(sdreduce_unittest_base,unittest.TestCase):
    """
    Test overwrite of the input scantable (outfile=infile) with
    disk and memory storage. (Tests for CAS-6601)

    testDisk --- overwrite input file with disk storage
    testMem --- overwrite input file with memory storage
    """
    #rawfile_raw = 'sd_analytic_type1-3.asap'
    rawfile = 'sd_analytic_type1-3.bl.asap'
    prefix = sdreduce_unittest_base.taskname+'TestSel'
    params = {'infile': rawfile,'outfile': rawfile,
              'calmode': 'none', 'average': False,
              'blfunc': 'none',
              'kernel': 'boxcar', 'kwidth': 5, 
              'outform': 'ASAP', 'overwrite': True}

    ref_data = ({'value': 1.0, 'channel': (17,21)},
                {'value': 2.0, 'channel': (37,41)},
                {'value': 4.0, 'channel': (57,61)},
                {'value': 6.0, 'channel': (77,81)},)
    ref_idx= []

    def setUp(self):
        self.res=None
        name = self.params['infile']
        if os.path.exists(name): shutil.rmtree(name)
        shutil.copytree(self.sddatapath+name, name)
        default(sdreduce)

    def tearDown(self):
        if os.path.exists(self.params['outfile']):
            shutil.rmtree(self.params['outfile'])

    def testDisk(self):
        """testDisk: test overwrite of the input scantable (outfile=infile) with disk storage"""
        self._run_sdreduce_with_storage('disk')
    
    def testMem(self):
        """testDisk: test overwrite of the input scantable (outfile=infile) with disk storage"""
        self._run_sdreduce_with_storage('memory')

    def _run_sdreduce_with_storage(self, storage):
        storage_save = sd.rcParams['scantable.storage']
        sd.rcParams['scantable.storage'] = storage
        try:
            print("Invoking sdflag with storage = %s" % \
                  sd.rcParams['scantable.storage'])
            # Invoke task
            self.res = sdreduce(**self.params)
        except  Exception, e:
            self.fail('Unexpected exception was thrown: %s'%(str(e)))
        finally:
            sd.rcParams['scantable.storage'] = storage_save
        # Test result
        self._compare_with_tophat(self.params['outfile'], self.ref_data,
                                  self.ref_idx, precision=1.e-6)


def suite():
    return [sdreduce_test, sdreduce_selection, sdreduce_storage_test]
