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

try:
    import test_sdcal
except:
    import tests.test_sdcal as test_sdcal

import asap as sd
from sdreduce import sdreduce
#from sdstat import sdstat

from sdutil import tbmanager

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

        result = sdreduce(infile=infile,outfile=outfile)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': 0.55389463901519775, 'min': 0.26541909575462341,
                   'max_abc': 773.0, 'max': 0.91243284940719604,
                   'sum': 3802.1259765625, 'stddev': 0.16529126465320587,
                   'min_abc': 7356.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

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


        result = sdreduce(infile=infile,outfile=outfile,calmode=calmode,
                       kernel=kernel,blfunc=blfunc)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")

        refstat = {'rms': 0.21985267102718353, 'min': -0.7019438743591309,
                   'max_abc': 4093.0, 'max': 0.9684028029441833,
                   'sum': 5.486020088195801, 'stddev': 0.21986639499664307,
                   'min_abc': 7623.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)
        

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

        result = sdreduce(infile=infile,outfile=outfile,calmode=calmode)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': 2.1299083232879639, 'min': 1.2246102094650269,
                   'max_abc': 4093.0, 'max': 3.1902554035186768,
                   'sum': 15209.119140625, 'stddev': 0.25390961766242981,
                   'min_abc': 7434.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

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

        result = sdreduce(infile=infile,outfile=outfile,
                          average=average,timeaverage=timeaverage,
                          tweight = 'tintsys',scanaverage=scanaverage)

        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': 4.128033638000488, 'min': 3.2304768562316895,
                   'max_abc': 4093.0, 'max': 5.676198482513428,
                   'sum': 29638.3515625, 'stddev': 0.24048194289207458,
                   'min_abc': 2452.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test06(self):
        """ Test 6:  operate only smoothing step """
        self.tid="06"
        infile = self.infile1
        outfile = self.outroot+self.tid+'.asap'
        kernel = 'hanning'

        result = sdreduce(infile=infile,outfile=outfile,kernel=kernel)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': 3.5979659557342529, 'min': 2.3542881011962891,
                   'max_abc': 4093.0, 'max': 5.2421674728393555,
                   'sum': 25737.166015625, 'stddev': 0.37295544147491455,
                   'min_abc': 6472.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test07(self):
        """ Test 7:  operate only baseline step """
        self.tid="07"
        infile = self.infile1
        outfile = self.outroot+self.tid+'.asap'
        blfunc = 'poly'

        result = sdreduce(infile=infile,outfile=outfile,blfunc=blfunc)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': 0.4292968213558197, 'min': -1.4878684282302856,
                   'max_abc': 4093.0, 'max': 1.8000496625900269,
                   'sum': 6.965803146362305, 'stddev': 0.42932620644569397,
                   'min_abc': 7434.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test08(self):
        """ Test 8:  skip calibration and averaging """
        self.tid="08"
        infile = self.infile1
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'none'
        average = False
        kernel = 'hanning'
        blfunc = 'poly'

        result = sdreduce(infile=infile,outfile=outfile,calmode=calmode,
                       average=average,kernel=kernel,blfunc=blfunc)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': 0.37204012274742126, 'min': -1.1878470182418823,
                   'max_abc': 4093.0, 'max': 1.638755202293396,
                   'sum': 9.283474922180176, 'stddev': 0.37206435203552246,
                   'min_abc': 7623.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test09(self):
        """ Test 9:  skip smoothing"""
        self.tid="09"
        infile = self.infile0
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'ps'
        kernel = 'none'
        blfunc = 'poly'

        result = sdreduce(infile=infile,outfile=outfile,calmode=calmode,
                       kernel=kernel,blfunc=blfunc)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")

        refstat = {'rms': 0.2536879777908325, 'min': -0.8792380690574646,
                   'max_abc': 4093.0, 'max': 1.0637180805206299,
                   'sum': 4.116354942321777, 'stddev': 0.2537047564983368,
                   'min_abc': 7434.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test10(self):
        """ Test 10:  skip baseline"""
        self.tid="10"
        infile = self.infile0
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'ps'
        kernel = 'hanning'
        blfunc = 'none'

        result = sdreduce(infile=infile,outfile=outfile,calmode=calmode,
                       kernel=kernel,blfunc=blfunc)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': 2.126171350479126, 'min': 1.3912382125854492,
                   'max_abc': 4093.0, 'max': 3.0977959632873535,
                   'sum': 15209.0869140625, 'stddev': 0.2203933447599411,
                   'min_abc': 6472.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

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
        self.calmode = 'none'
        self.average = False
        self.timeaverage = False
        self.scanaverage = False
        self.polaverage = False
        self.kernel = 'none'
        self.kwidth = 5
        self.maskmode = 'auto'
        self.blfunc = 'none'
        self.order = 3
        self.outname = self.prefix+self.postfix

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
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, scan=scan, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_scan_id_exact(self):
        """test scan selection (scan='16')"""
        scan = '16'
        ref_idx = [1,2]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, scan=scan, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)


    def test_scan_id_lt(self):
        """test scan selection (scan='<16')"""
        scan = '<16'
        ref_idx = [0]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, scan=scan, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_scan_id_gt(self):
        """test scan selection (scan='>16')"""
        scan = '>16'
        ref_idx = [3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, scan=scan, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_scan_id_range(self):
        """test scan selection (scan='16~17')"""
        scan = '16~17'
        ref_idx = [1,2,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, scan=scan, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_scan_id_list(self):
        """test scan selection (scan='15,17')"""
        scan = '15,17'
        ref_idx = [0,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, scan=scan, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_scan_id_exprlist(self):
        """test scan selection (scan='15,>16')"""
        scan = '15,>16'
        ref_idx = [0,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, scan=scan, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    ####################
    # pol
    ####################
    def test_pol_id_default(self):
        """test pol selection (pol='')"""
        pol = ''
        ref_idx = []
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, pol=pol, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_pol_id_exact(self):
        """test pol selection (pol='1')"""
        pol = '1'
        ref_idx = [1,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, pol=pol, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_pol_id_lt(self):
        """test pol selection (pol='<1')"""
        pol = '<1'
        ref_idx = [0,2]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, pol=pol, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_pol_id_gt(self):
        """test pol selection (pol='>0')"""
        pol = '>0'
        ref_idx = [1,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, pol=pol, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_pol_id_range(self):
        """test pol selection (pol='0~1')"""
        pol = '0~1'
        ref_idx = []
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, pol=pol, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_pol_id_list(self):
        """test pol selection (pol='0,1')"""
        pol = '0,1'
        ref_idx = []
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, pol=pol, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_pol_id_exprlist(self):
        """test pol selection (pol='0,>0')"""
        pol = '0,>0'
        ref_idx = []
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, pol=pol, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    ####################
    # field
    ####################
    def test_field_value_default(self):
        """test field selection (field='')"""
        field = ''
        ref_idx = []
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, field=field, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_field_id_exact(self):
        """test field selection (field='6')"""
        field = '6'
        ref_idx = [1]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, field=field, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_field_id_lt(self):
        """test field selection (field='<6')"""
        field = '<6'
        ref_idx = [0]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, field=field, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_field_id_gt(self):
        """test field selection (field='>7')"""
        field = '>7'
        ref_idx = [3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, field=field, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_field_id_range(self):
        """test field selection (field='6~8')"""
        field = '6~8'
        ref_idx = [1,2,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, field=field, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_field_id_list(self):
        """test field selection (field='5,8')"""
        field = '5,8'
        ref_idx = [0,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, field=field, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_field_id_exprlist(self):
        """test field selection (field='5,>7')"""
        field = '5,>7'
        ref_idx = [0,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, field=field, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_field_value_exact(self):
        """test field selection (field='M30')"""
        field = 'M30'
        ref_idx = [2]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, field=field, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_field_value_pattern(self):
        """test field selection (field='M*')"""
        field = 'M*'
        ref_idx = [0,1,2]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, field=field, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_field_value_list(self):
        """test field selection (field='3C273,M30')"""
        field = '3C273,M30'
        ref_idx = [2,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, field=field, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_field_mix_exprlist(self):
        """test field selection (field='<6,3*')"""
        field = '<6,3*'
        ref_idx = [0,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, field=field, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    ####################
    # spw 
    ####################
    def test_spw_id_default(self):
        """test spw selection (spw='', all channels)"""
        spw = ''
        ref_idx = []
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_exact(self):
        """test spw selection (spw='23', all channels)"""
        spw = '23'
        ref_idx = [0,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_lt(self):
        """test spw selection (spw='<23', all channels)"""
        spw = '<23'
        ref_idx = [2]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_gt(self):
        """test spw selection (spw='>23', all channels)"""
        spw = '>23'
        ref_idx = [1]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_range(self):
        """test spw selection (spw='23~25', all channels)"""
        spw = '23~25'
        ref_idx = [0,1,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_list(self):
        """test spw selection (spw='21,25', all channels)"""
        spw = '21,25'
        ref_idx = [1,2]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_exprlist(self):
        """test spw selection (spw='23,>24', all channels)"""
        spw = '23,>24'
        ref_idx = [0,1,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_pattern(self):
        """test spw selection (spw='*', all channels)"""
        spw = '*'
        ref_idx = []
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_value_frequency(self):
        """test spw selection (spw='299.52~300.47GHz', all channels)"""
        spw = '299.52~300.47GHz' # IFNO=23 will be selected
        ref_idx = [0,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_value_velocity(self):
        """test spw selection (spw='-510.~470.km/s', all channels)"""
        spw = '-510.~470km/s' # IFNO=23,25 will be selected
        ref_idx = [0,1,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_mix_exprlist(self):
        """test spw selection (spw='25,0~501km/s', all channels)"""
        spw = '25,0~501km/s' # all IFs will be selected
        ref_idx = []
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=False)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    #########################
    # spw with channel range
    #########################
    def test_spw_id_default_channel(self):
        """test spw selection w/ channel selection (spw=':21~39')"""
        spw = ':21~39'
        ref_idx = []
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx, precision = 2.e-5)

    def test_spw_id_default_frequency(self):
        """test spw selection w/ channel selection (spw=':300.491~300.549GHz')"""
        spw = ':300.491~300.549GHz' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_default_velocity(self):
        """test spw selection w/ channel selection (spw=':-548.7~-490.5km/s')"""
        spw = ':-548.7~-490.5km/s' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)


    def test_spw_id_default_list(self):
        """test spw selection w/ channel selection (spw=':0~19;21~39;41~59;61~79;81~100')"""
        spw = ':0~19;21~39;41~59;61~79;81~100'
        ref_idx = []
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_exact_channel(self):
        """test spw selection w/ channel selection (spw='23:21~79')"""
        spw = '23:21~79'
        ref_idx = [0,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_exact_frequency(self):
        """test spw selection w/ channel selection (spw='25:300.491~300.549GHz')"""
        spw = '25:300.491~300.549GHz' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_exact_velocity(self):
        """test spw selection w/ channel selection (spw='25:-548.7~-490.5km/s')"""
        spw = '25:-548.7~-490.5km/s' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_exact_list(self):
        """test spw selection w/ channel selection (spw='23:0~19;21~79;81~100')"""
        spw = '23:0~19;21~79;81~100'
        ref_idx = [0,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_pattern_channel(self):
        """test spw selection w/ channel selection (spw='*:21~39')"""
        spw = '*:21~39'
        ref_idx = []
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx, precision = 2.e-5)

    def test_spw_id_pattern_frequency(self):
        """test spw selection w/ channel selection (spw='*:300.491~300.549GHz')"""
        spw = '*:300.491~300.549GHz' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_pattern_velocity(self):
        """test spw selection w/ channel selection (spw='*:-548.7~-490.5km/s')"""
        spw = '*:-548.7~-490.5km/s' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_pattern_list(self):
        """test spw selection w/ channel selection (spw='*:0~19;21~39;41~59;61~79;81~100')"""
        spw = '*:0~19;21~39;41~59;61~79;81~100'
        ref_idx = []
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_value_frequency_channel(self):
        """test spw selection w/ channel selection (spw='299.9~300.1GHz:21~79')"""
        spw = '299.9~300.1GHz:21~79' # IFNO=23
        ref_idx = [0,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_value_frequency_frequency(self):
        """test spw selection w/ channel selection (spw='300.4~300.6GHz:300.491~300.549GHz')"""
        spw = '300.4~300.6GHz:300.491~300.549GHz' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_value_frequency_velocity(self):
        """test spw selection w/ channel selection (spw='300.4~300.6GHz:-548.7~-490.5km/s')"""
        spw = '300.4~300.6GHz:-548.7~-490.5km/s' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_value_frequency_list(self):
        """test spw selection w/ channel selection (spw='299.9~300.1GHz:0~19;21~79;81~100')"""
        spw = '299.9~300.1GHz:0~19;21~79;81~100' # IFNO=23
        ref_idx = [0,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_value_velocity_channel(self):
        """test spw selection w/ channel selection (spw='-30~30km/s:21~79')"""
        spw = '-30~30km/s:21~79' # IFNO=23
        ref_idx = [0,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_value_velocity_frequency(self):
        """test spw selection w/ channel selection (spw='-500~-499km/s:-548.7~-490.5km/s')"""
        spw = '-500~-499km/s:-548.7~-490.5km/s' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_value_velocity_velocity(self):
        """test spw selection w/ channel selection (spw='-500~-499km/s:-548.7~-490.5km/s')"""
        spw = '-500~-499km/s:-548.7~-490.5km/s' # IFNO=25 channel=41~99 will be selected
        ref_idx = [1]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_value_velocity_list(self):
        """test spw selection w/ channel selection (spw='-30~30km/s:0~19;21~79;81~100')"""
        spw = '-30~30km/s:0~19;21~79;81~100' # IFNO=23
        ref_idx = [0,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    def test_spw_id_list_channel(self):
        """test spw selection w/ channel selection (spw='21:0~59,23:21~79')"""
        spw = '21:0~59,23:21~79'
        ref_idx = [0,2,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, spw=spw, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')
        self._compare_with_tophat(self.outname, self.ref_data, ref_idx)

    ####################
    # timerange
    ####################
    def test_timerange_value_default(self):
        """test timerange selection (timerange='')"""
        timerange = ''
        ref_idx = []
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, timerange=timerange, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')

    def test_timerange_value_exact(self):
        """test timerange selection (timerange='2011/11/11/02:33:03.47')"""
        timerange = '<2011/11/11/02:33:03.47' # SCANNO=15 will be selected
        ref_idx = [0]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, timerange=timerange, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')

    def test_timerange_value_lt(self):
        """test timerange selection (timerange='<2011/11/11/02:33:03.47')"""
        timerange = '<2011/11/11/02:33:03.47' # SCANNO=15 will be selected
        ref_idx = [0]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, timerange=timerange, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')

    def test_timerange_value_gt(self):
        """test timerange selection (timerange='>2011/11/11/02:33:03.5')"""
        timerange = '>2011/11/11/02:33:03.5' # SCANNO=17 will be selected
        ref_idx = [3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, timerange=timerange, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')

    def test_timerange_value_range(self):
        """test timerange selection (timerange='2011/11/11/02:33:03.47~02:34:03.48')"""
        timerange = '2011/11/11/02:33:03.47~02:34:03.48' # SCANNO=16,17 will be selected
        ref_idx = [1,2,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, timerange=timerange, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')

    def test_timerange_value_interval(self):
        """test timerange selection (timerange='2011/11/11/02:33:03.47+00:01:00.1')"""
        timerange = '2011/11/11/02:33:03.47+00:01:00.1' # SCANNO=16,17 will be selected
        ref_idx = [1,2,3]
        # blflag = T (channel selection: average+baseline)
        #          F (no channel selection: only smoothing)
        self._set_test_mode(blflag=True)
        self.res=self.run_task(infile=self.rawfile, timerange=timerange, calmode=self.calmode,average=self.average,timeaverage=self.timeaverage,scanaverage=self.scanaverage,polaverage=self.polaverage,kernel=self.kernel,kwidth=self.kwidth,maskmode=self.maskmode,blfunc=self.blfunc,order=self.order,outfile=self.outname,outform='ASAP')


    ####################
    # Helper functions
    ####################
    def _set_test_mode(self, blflag):
        # set proper input file
        #self.calmode = 'ps'
        if blflag: # averaging and baseline
            self.rawfile = self.rawfile_cal
            self.average = True
            self.timeaverage = True
            self.polaverage = False
            self.blfunc = 'poly'
            self.maskmode = 'list'
            self.ref_data = self.refval_bl
        else: # smoothing only
            self.rawfile = self.rawfile_bl
            self.kernel = 'boxcar'
            self.kwidth = 5
            self.ref_data = self.refval_sm

    
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

###
# Test flag information handling
###
class sdreduce_test_average_flag(unittest.TestCase):
    """
    ### This is a copy of test_sdavearge.sdaverage_test_average_flag ###
    
    Test flag information handling.

    Data is sdaverage_testflag.asap

    Summary of the data:
    ROW | FLAGROW    | FLAGTRA          | SPECTRA 
     0  | 0          | ch 10~11 flagged | spurious at ch 10~11
     1  | 1 (flagged)| all 0            | spurious at ch 10
     2  | 0          | ch 10,40 flagged | spurious at ch 10,40

    Test list
        test_average_flag: test if average handles flag information properly
        test_average_novaliddata: test if the task throws exception if
                                  no valid data exists
        test_smooth_hanning: test if hanning smoothing (direct convolution)
                             handles flag information correctly
        test_smooth_gaussian: test if gaussian smoothing (FFT convolution)
                              handles flag information correctly
        test_smooth_regrid: test if regridding (binning) handles flag
                            information correctly
    """
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdaverage/'
    rawfile = 'sdaverage_testflag.asap'
    prefix = 'sdreduce_test_average_flag'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdreduce)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        
        os.system( 'rm -rf '+self.prefix+'*' )

    def _get_data(self, filename, optional=[]):
        cols = ['FLAGROW', 'FLAGTRA', 'SPECTRA']
        cols.extend(optional)
        with tbmanager(filename) as tb:
            vals = [tb.getcol(col) for col in cols]
        return vals

    def _message_for_assert(self, val, ref, col, isshape, row, channel, refistol=False):
        word = 'tolerance' if refistol else 'expected'
        msg = '%s differ (%s %s result %s)'%(col,word,ref,val)
        if isshape:
            msg = 'shape of ' + msg
        head = ''
        if row is not None:
            head = 'Row %s'%(row)
        if channel is not None:
            head = head + ' Channel %s'%(channel)
        if len(head) > 0:
            msg = head + ': ' + msg
        #print msg
        return msg
    
    def _assert_equal(self, val, ref, col, isshape=False, row=None, channel=None):
        msg = self._message_for_assert(val, ref, col, isshape, row, channel)
        self.assertEqual(val, ref, msg=msg)

    def _assert_less(self, val, tol, col, isshape=False, row=None, channel=None):
        msg = self._message_for_assert(val, tol, col, isshape, row, channel, refistol=True)
        self.assertLess(val, tol, msg=msg)
        
    
    def _verify_shape(self, expected_nrow, expected_nchan, flagrow, flagtra, spectra):
        expected_shape = (expected_nchan,expected_nrow,)
        self._assert_equal(len(flagrow), expected_nrow, 'FLAGROW', isshape=True)
        self._assert_equal(list(flagtra.shape), list(expected_shape), 'FLAGTRA', isshape=True)
        self._assert_equal(list(spectra.shape), list(expected_shape), 'SPECTRA', isshape=True)
       
    def _verify_average(self, outfile):
        # get data before averaging
        flagrow_org, flagtra_org, spectra_org, interval_org = self._get_data(self.rawfile, ['INTERVAL'])

        # get data after averaging
        flagrow, flagtra, spectra = self._get_data(outfile)

        # basic check
        # all data should be averaged into one row
        nrow = 1
        nchan = spectra_org.shape[0]
        self._verify_shape(1, nchan, flagrow, flagtra, spectra)

        # verify
        flagrow = flagrow[0]
        flagtra = flagtra[:,0]
        spectra = spectra[:,0]

        # verify FLAGROW
        flagrow_expected = 0 if any(flagrow_org == 0) else 1
        self._assert_equal(flagrow, flagrow_expected, 'FLAGROW')

        # verify FLAGTRA
        def gen_averaged_channelflag(rflag, chflag):
            nchan, nrow = chflag.shape
            for ichan in xrange(nchan):
                yield 0 if any(rflag + chflag[ichan] == 0) else 128
        flagtra_expected = numpy.array(list(gen_averaged_channelflag(flagrow_org, flagtra_org)))
        for ichan in xrange(nchan):
            self._assert_equal(flagtra_expected[ichan], flagtra[ichan], 'FLAGTRA', row=0, channel=ichan)
        
        # verify SPECTRA
        def gen_averaged_spectra(rflag, chflag, data, weight):
            nchan, nrow = data.shape
            for ichan in xrange(nchan):
                dsum = 0.0
                wsum = 0.0
                for irow in xrange(nrow):
                    if rflag[irow] == 0 and chflag[ichan,irow] == 0:
                        dsum += weight[irow] * data[ichan,irow]
                        wsum += weight[irow]
                if wsum == 0:
                    yield None
                else:
                    yield dsum / wsum
        spectra_expected = numpy.array(list(gen_averaged_spectra(flagrow_org,
                                                                 flagtra_org,
                                                                 spectra_org,
                                                                 interval_org)))
        tol = 1.0e-7
        for ichan in xrange(nchan):
            if flagtra[ichan] == 0:
                sp = spectra[ichan]
                ex = spectra_expected[ichan]
                diff = abs((sp - ex) / ex)
                #print ichan, diff
                self._assert_less(diff, tol, 'SPECTRA', row=0, channel=ichan)
            else:
                print 'Skip channel %s since it is flagged'%(ichan)

    def _verify_smooth(self, outfile):
        flagrow_org, flagtra_org, spectra_org = self._get_data(self.rawfile)
        flagrow, flagtra, spectra = self._get_data(outfile)

        # basic check
        nchan_expected,nrow_expected = flagtra_org.shape
        self._verify_shape(nrow_expected, nchan_expected, flagrow, flagtra, spectra)

        nchan, nrow = spectra.shape

        for irow in xrange(nrow):
            # FLAGROW should not be modified
            rflag_ref = flagrow_org[irow]
            rflag = flagrow[irow]
            self._assert_equal(rflag_ref, rflag, 'FLAGROW', row=irow)

            # FLAGTRA should not be modified
            flag_ref = flagtra[:,irow]
            flag = flagtra[:,irow]
            for ichan in xrange(nchan):
                self._assert_equal(flag_ref[ichan], flag[ichan], 'FLAGTRA', row=irow, channel=ichan)

            # verify SPECTRA
            sp_ref = spectra_org[:,irow]
            sp = spectra[:,irow]
            if rflag != 0:
                # flagged row should not be processed
                for ichan in xrange(nchan):
                    self._assert_equal(sp_ref[ichan], sp[ichan], 'SPECTRA', row=irow, channel=ichan)
            else:
                # Here, what should be tested is spurious data (at flagged
                # channels) are replaced with the value interpolated from
                # neighbors. Tolerance is set to loose value to avoid false
                # failue due to unexpected behavior of FFT based smoothing.
                tol = 1.0e1
                
                sp_expected = sp_ref.copy()
                sp_expected[:] = sp_ref[0]
                diff = abs((sp - sp_expected) / sp_expected)
                self._assert_less(max(diff), tol, 'SPECTRA', row=irow)
        
    def _verify_regrid(self, outfile, chanwidth):
        flagrow_org, flagtra_org, spectra_org = self._get_data(self.rawfile)
        flagrow, flagtra, spectra = self._get_data(outfile)

        # basic check
        nchan_org, nrow = spectra_org.shape
        nchan_expected = int(numpy.ceil(float(nchan_org) / float(chanwidth)))
        self._verify_shape(nrow, nchan_expected, flagrow, flagtra, spectra)

        nchan, nrow = spectra.shape

        # generator for FLAGTRA and SPECTRA
        def gen_flagtra(flag, w):
            nchan = len(flag)
            width = int(w)
            for i in xrange(0, nchan, width):
                yield 0 if any(flag[i:i+w] == 0) else 128
                
        def gen_spectra(sp, flag, w):
            nchan = len(sp)
            width = int(w)
            for i in xrange(0, nchan, width):
                s = sp[i:i+w]
                f = [1.0 if _f == 0 else 0.0 for _f in flag[i:i+w]]
                sumf = sum(f)
                if sumf == 0.0:
                    yield 0.0
                else:
                    yield sum(s * f) / sum(f) 

        # verify
        for irow in xrange(nrow):
            # FLAGROW should not be modified
            rflag_ref = flagrow_org[irow]
            rflag = flagrow[irow]
            self._assert_equal(rflag_ref, rflag, 'FLAGROW', row=irow)

            # FLAGTRA is combined with OR operation
            flag_ref = numpy.array(list(gen_flagtra(flagtra_org[:,irow], chanwidth)))
            flag = flagtra[:,irow]
            #print irow, flag_ref, flag
            for ichan in xrange(nchan):
                self._assert_equal(flag_ref[ichan], flag[ichan], 'FLAGTRA', row=irow, channel=ichan)

            # verify SPECTRA
            sp_ref = numpy.array(list(gen_spectra(spectra_org[:,irow],
                                                  flagtra_org[:,irow],
                                                  chanwidth)))
            sp = spectra[:,irow]
            
            # ignore FLAG_ROW, all rows are processed
            tol = 1.0e-6

            for ichan in xrange(nchan):
                val = sp[ichan]
                ref = sp_ref[ichan]
                if ref == 0.0:
                    self._assert_equal(ref, val, 'SPECTRA', row=irow, channel=ichan)
                else:
                    diff = abs((val - ref) / ref)
                    #print irow, ichan,  diff
                    self._assert_less(diff, tol, 'SPECTRA', row=irow, channel=ichan)

    def test_average_flag(self):
        """test_average_flag: test if average handles flag information properly"""
        outfile = self.prefix + '.asap'
        res = sdreduce(infile=self.rawfile, outfile=outfile, calmode='none', average=True, timeaverage=True, tweight='tint', kernel='none', blfunc='none')

        self._verify_average(outfile)

    def test_average_novaliddata(self):
        """test_average_novaliddata: test if the task throws exception if no valid data exists"""
        # flag all rows
        with tbmanager(self.rawfile, nomodify=False) as tb:
            flagrow = tb.getcol('FLAGROW')
            flagrow[:] = 1
            tb.putcol('FLAGROW', flagrow)

        # double check
        with tbmanager(self.rawfile, nomodify=False) as tb:
            flagrow = tb.getcol('FLAGROW')
        self.assertTrue(all(flagrow == 1), msg='Failed to preparing data')

        outfile = self.prefix + '.asap'
        # the task must raise RuntimeError with correct message
        with self.assertRaises(RuntimeError) as cm:
            sdreduce(infile=self.rawfile, outfile=outfile, calmode='none', average=True, timeaverage=True, tweight='tint', kernel='none', blfunc='none')
        the_exception = cm.exception
        message = the_exception.message
        expected_message = 'Can\'t average fully flagged data.'
        self.assertEqual(message, expected_message, msg='Exception contains unexpected message: "%s" (expected "%s")'%(message,expected_message))

    def test_smooth_hanning(self):
        """test_smooth_hanning: test if hanning smoothing (direct convolution) handles flag information correctly"""
        outfile = self.prefix + '.asap'
        res = sdreduce(infile=self.rawfile, outfile=outfile, calmode='none', average=False, kernel='hanning', blfunc='none')

        self._verify_smooth(outfile)

    def test_smooth_gaussian(self):
        """test_smooth_gaussian: test if gaussian smoothing (FFT convolution) handles flag information correctly"""
        outfile = self.prefix + '.asap'
        res = sdreduce(infile=self.rawfile, outfile=outfile, calmode='none', average=False, kernel='gaussian', blfunc='none')

        self._verify_smooth(outfile)


    def test_smooth_regrid(self):
        """test_smooth_regrid: test if regridding (binning) handles flag information correctly"""
        outfile = self.prefix + '.asap'
        chanwidth = 2
        res = sdreduce(infile=self.rawfile, outfile=outfile, calmode='none', average=False, kernel='regrid', chanwidth=str(chanwidth), blfunc='none')

        self._verify_regrid(outfile, chanwidth)

class sdreduce_test_baseline_flag( unittest.TestCase ):
    """
    ### This is a copy of test_sdbaseline.sdbaseline_flagTest ###
    
    Unit tests for task sdbaseline. No interactive testing.
    This test is to verify the proper flag handling in sdbaseline that
       (1) for row-flagged spectra, neither fitting nor subtraction should be executed.
       (2) if a channel is flagged, it will not be used for baseline calculation,
           but the baseline subtraction at the channel should be made.
       (3) no flag values themselves should be modified.
           
    The list of tests:
    testFlagPoly01     --- test polynomial fitting with maskmode = 'list'
    testFlagPoly02     --- test polynomial fitting with maskmode = 'auto'
    testFlagCheby01    --- test Chebyshev polynomial fitting with maskmode = 'list'
    testFlagCheby02    --- test Chebyshev polynomial fitting with maskmode = 'auto'
    testFlagCSpline01  --- test cubic spline fitting with maskmode = 'list'
    testFlagCSpline02  --- test cubic spline fitting with maskmode = 'auto'
    testFlagSinusoid01 --- test sinusoidal fitting with maskmode = 'list'
    testFlagSinusoid02 --- test sinusoidal fitting with maskmode = 'auto'

    Note: the rms noise of input data for the tests *02 is 1.0.
    """
    tol01 = 1.0e-6
    tol02 = 1.0 # large value owing to uncertainty in linefinder results and
                # to small channel numbers. enough for this testing.
    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
              '/data/regression/unittest/sdbaseline/'    
    # Input and output names
    infile_01 = 'sdbaseline_flagtest_withoutnoise.asap'
    infile_02 = 'sdbaseline_flagtest_withnoise.asap'
    outroot = 'sdreduce_test'
    tid = None

    def setUp( self ):
        if os.path.exists(self.infile_01):
            shutil.rmtree(self.infile_01)
        shutil.copytree(self.datapath+self.infile_01, self.infile_01)
        if os.path.exists(self.infile_02):
            shutil.rmtree(self.infile_02)
        shutil.copytree(self.datapath+self.infile_02, self.infile_02)

        default(sdreduce)

    def tearDown( self ):
        if os.path.exists(self.infile_01):
            shutil.rmtree(self.infile_01)
        if os.path.exists(self.infile_02):
            shutil.rmtree(self.infile_02)
        os.system('rm -rf '+self.outroot+'*')

    def testFlagPoly01( self ):
        """Test FlagPoly01: Polynomial fitting with maskmode = 'list'"""
        self.tid = "FlagPoly01"
        infile = self.infile_01
        mode = "list"
        outfile = self.outroot+self.tid+".asap"
        
        result = sdreduce(infile=infile,maskmode=mode,outfile=outfile,blfunc='poly',order=0,calmode='none',average=False,kernel='none')
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self._checkResult(infile, outfile, self.tol01)

    def testFlagPoly02( self ):
        """Test FlagPoly02: Polynomial fitting with maskmode = 'auto'"""
        self.tid = "FlagPoly02"
        infile = self.infile_02
        mode = "auto"
        outfile = self.outroot+self.tid+".asap"
        
        result = sdreduce(infile=infile,maskmode=mode,outfile=outfile,blfunc='poly',order=0,calmode='none',average=False,kernel='none')
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self._checkResult(infile, outfile, self.tol02)

    def testFlagCheby01( self ):
        """Test FlagCheby01: Chebyshev Polynomial fitting with maskmode = 'list'"""
        self.tid = "FlagCheby01"
        infile = self.infile_01
        mode = "list"
        outfile = self.outroot+self.tid+".asap"
        
        result = sdreduce(infile=infile,maskmode=mode,outfile=outfile,blfunc='chebyshev',order=0,calmode='none',average=False,kernel='none')
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self._checkResult(infile, outfile, self.tol01)

    def testFlagCheby02( self ):
        """Test FlagCheby02: Chebyshev Polynomial fitting with maskmode = 'auto'"""
        self.tid = "FlagCheby02"
        infile = self.infile_02
        mode = "auto"
        outfile = self.outroot+self.tid+".asap"
        
        result = sdreduce(infile=infile,maskmode=mode,outfile=outfile,blfunc='chebyshev',order=0,calmode='none',average=False,kernel='none')
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self._checkResult(infile, outfile, self.tol02)

    def testFlagCSpline01( self ):
        """Test FlagCSpline01: Cubic spline fitting with maskmode = 'list'"""
        self.tid = "FlagCSpline01"
        infile = self.infile_01
        mode = "list"
        outfile = self.outroot+self.tid+".asap"
        
        result = sdreduce(infile=infile,maskmode=mode,outfile=outfile,blfunc='cspline',npiece=1,calmode='none',average=False,kernel='none')
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self._checkResult(infile, outfile, self.tol01)

    def testFlagCSpline02( self ):
        """Test FlagCSpline02: Cubic spline fitting with maskmode = 'auto'"""
        self.tid = "FlagCSpline02"
        infile = self.infile_02
        mode = "auto"
        outfile = self.outroot+self.tid+".asap"
        
        result = sdreduce(infile=infile,maskmode=mode,outfile=outfile,blfunc='cspline',npiece=1,calmode='none',average=False,kernel='none')
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self._checkResult(infile, outfile, self.tol02)

    def testFlagSinusoid01( self ):
        """Test FlagSinusoid01: Sinusoidal Polynomial fitting with maskmode = 'list'"""
        self.tid = "FlagSinusoid01"
        infile = self.infile_01
        mode = "list"
        outfile = self.outroot+self.tid+".asap"
        
        result = sdreduce(infile=infile,maskmode=mode,outfile=outfile,blfunc='sinusoid',calmode='none',average=False,kernel='none')
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self._checkResult(infile, outfile, self.tol01)

    def testFlagSinusoid02( self ):
        """Test FlagSinusoid02: Sinusoidal Polynomial fitting with maskmode = 'auto'"""
        self.tid = "FlagSinusoid02"
        infile = self.infile_02
        mode = "auto"
        outfile = self.outroot+self.tid+".asap"
        
        result = sdreduce(infile=infile,maskmode=mode,outfile=outfile,blfunc='sinusoid',calmode='none',average=False,kernel='none')
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self._checkResult(infile, outfile, self.tol02)

    def _checkResult(self, infile, outfile, tol):
        tb.open(infile)
        inspec = [tb.getcell('SPECTRA', 0), tb.getcell('SPECTRA', 1), tb.getcell('SPECTRA', 2)]
        inchnf = [tb.getcell('FLAGTRA', 0), tb.getcell('FLAGTRA', 1), tb.getcell('FLAGTRA', 2)]
        inrowf = tb.getcol('FLAGROW')
        tb.close()

        tb.open(outfile)
        outspec = [tb.getcell('SPECTRA', 0), tb.getcell('SPECTRA', 1), tb.getcell('SPECTRA', 2)]
        outchnf = [tb.getcell('FLAGTRA', 0), tb.getcell('FLAGTRA', 1), tb.getcell('FLAGTRA', 2)]
        outrowf = tb.getcol('FLAGROW')
        tb.close()

        #check if the values of row-flagged spectra are not changed
        for i in xrange(2):
            self.assertTrue(all(inspec[i]==outspec[i]))
            
        #check if flagged channels are (1) excluded from fitting, but are
        #(2) the targets of baseline subtraction.
        #  if the difference values between the input and output spectra
        #  (input-output) are almost 1.0 (for tests *01) or distribute around
        #  1.0 (for tests *02), it can be recognised that both of the above
        # requirements are satisfied. actually, the mean of the (input-output)
        # values is examined if it is close enough to 1.0.
        #print '***************'+str(abs((inspec[2]-outspec[2]).mean()-1.0))
        self.assertTrue(abs((inspec[2]-outspec[2]).mean()-1.0) < tol)
        
        #check if flag values are not changed in the output file.
        for i in xrange(len(inchnf)):
            self.assertTrue(all(inchnf[i]==outchnf[i]))
        self.assertTrue(all(inrowf==outrowf))

###
# Test flag handling in ALMA position switch calibration
###
class sdreduce_test_cal_psalma_flag(test_sdcal.sdcal_caltest_base,unittest.TestCase):
    """
    This is a copy from sdcal unit test, test_sdcal.sdcal_testFlagPSALMA.
    """
    # Input and output names
    raw1file='calpsALMA_flagtest.asap'
    raw2file='calpsALMA_flagtest_rowflagged.asap'
    ref1file='calpsALMA_flagtest.cal.asap'
    ref2file='calpsALMA_flagtest_rowflagged.cal.asap'
    prefix=test_sdcal.sdcal_unittest_base.taskname+'TestFlagPSALMA'
    calmode='ps'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.raw1file)):
            shutil.copytree(self.datapath+self.raw1file, self.raw1file)
        if (not os.path.exists(self.raw2file)):
            shutil.copytree(self.datapath+self.raw2file, self.raw2file)
        if (not os.path.exists(self.ref1file)):
            shutil.copytree(self.datapath+self.ref1file, self.ref1file)
        if (not os.path.exists(self.ref2file)):
            shutil.copytree(self.datapath+self.ref2file, self.ref2file)

        default(sdreduce)

    def tearDown(self):
        if (os.path.exists(self.raw1file)):
            shutil.rmtree(self.raw1file)
        if (os.path.exists(self.raw2file)):
            shutil.rmtree(self.raw2file)
        if (os.path.exists(self.ref1file)):
            shutil.rmtree(self.ref1file)
        if (os.path.exists(self.ref2file)):
            shutil.rmtree(self.ref2file)
        os.system( 'rm -rf '+self.prefix+'*' )

    def _comparecal( self, name, reffile ):
        self._checkfile(name)
        sp=self._getspectra(name)
        spref=self._getspectra(reffile)

        self._checkshape( sp, spref )
        
        for irow in xrange(sp.shape[0]):
            diff=self._diff(sp[irow],spref[irow])
            retval=numpy.all(diff<0.01)
            maxdiff=diff.max()
            self.assertEqual( retval, True,
                             msg='calibrated result is wrong (irow=%s): maxdiff=%s'%(irow,diff.max()) )
        del sp, spref

    def testFlagPSALMA01(self):
        """Test FlagPSALMA01: for non-row-flagged ON-data (ALMA position switch)"""
        outname=self.prefix+self.postfix
        self.res=sdreduce(infile=self.raw1file,calmode=self.calmode,outfile=outname,outform='ASAP',maskmode='list',blfunc='none',average=False,kernel='none')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname, self.ref1file)

    def testFlagPSALMA02(self):
        """Test FlagPSALMA02: for row-flagged ON-data (ALMA position switch)"""
        outname=self.prefix+self.postfix
        self.res=sdreduce(infile=self.raw2file,calmode=self.calmode,outfile=outname,outform='ASAP',maskmode='list',blfunc='none',average=False,kernel='none')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname, self.ref2file)


###
# Test flag handling in calibrating ALMA OTF data
###
class sdreduce_test_cal_otf_flag(test_sdcal.sdcal_caltest_base,unittest.TestCase):
    """
    This is a copy of sdcal unit test, test_sdcal.sdcal_testFlagOTF.
    """
    # Input and output names
    rawfile='lissajous_flagtest.asap'
    reffile='lissajous_flagtest.cal.asap'
    prefix=test_sdcal.sdcal_unittest_base.taskname+'TestFlagOTF'
    calmode='otf'
    fraction='10%'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(sdreduce)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def testFlagOTF01(self):
        """Test FlagOTF01: for ALMA OTF data"""
        outname=self.prefix+self.postfix
        self.res=sdreduce(infile=self.rawfile,calmode=self.calmode,fraction=self.fraction,outfile=outname,outform='ASAP',maskmode='list',blfunc='none',average=False,kernel='none')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')

        tb.open(self.rawfile)
        spec_rowflagged_input = tb.getcell('SPECTRA', 100)
        tb.close()
        tb.open(self.reffile)
        spec_rowflagged_output = tb.getcell('SPECTRA', 52)
        spec_output  = tb.getcol('SPECTRA')[0]
        cflag_output = tb.getcol('FLAGTRA')[0]
        rflag_output = tb.getcol('FLAGROW')
        tb.close()

        #check row-flagged ON spectrum is not calibrated
        self.assertTrue(self._diff(spec_rowflagged_input, spec_rowflagged_output) < 1e-5)
        #check other ON spectra is calibrated so have values less than ~0.3
        spec_output[52] = 0.0
        self.assertTrue(all(spec_output < 0.03))
        #check channel flags not modified
        self.assertEqual(cflag_output[53], 128)
        cflag_output[53] = 0
        self.assertTrue(all(cflag_output == 0))
        #check row flags not modified
        self.assertEqual(rflag_output[52], 1)
        rflag_output[52] = 0
        self.assertTrue(all(rflag_output == 0))

###
# Test flag handling in calibrating ALMA OTF raster data
###
class sdreduce_test_cal_otfraster_flag(test_sdcal.sdcal_caltest_base,unittest.TestCase):
    """
    This is a copy from sdcal unit test, test_sdcal.sdcal_testFlagOTFRASTER.
    """
    # Input and output names
    rawfile='raster_flagtest.asap'
    reffile='raster_flagtest.cal.asap'
    prefix=test_sdcal.sdcal_unittest_base.taskname+'TestFlagOTFRASTER'
    calmode='otfraster'
    fraction='10%'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(sdreduce)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def testFlagOTF01(self):
        """Test FlagOTF01: for ALMA OTF raster data"""
        outname=self.prefix+self.postfix
        self.res=sdreduce(infile=self.rawfile,calmode=self.calmode,fraction=self.fraction,outfile=outname,outform='ASAP',maskmode='list',blfunc='none',average=False,kernel='none')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')

        irow_rowflagged_input  = 50
        irow_rowflagged_output = 44
        irow_chanflagged_output = 45

        tb.open(self.rawfile)
        spec_rowflagged_input = tb.getcell('SPECTRA', irow_rowflagged_input)
        tb.close()
        tb.open(self.reffile)
        spec_rowflagged_output = tb.getcell('SPECTRA', irow_rowflagged_output)
        spec_output  = tb.getcol('SPECTRA')[0]
        cflag_output = tb.getcol('FLAGTRA')[0]
        rflag_output = tb.getcol('FLAGROW')
        tb.close()

        #check row-flagged ON spectrum is not calibrated
        self.assertTrue(self._diff(spec_rowflagged_input, spec_rowflagged_output) < 1e-5)
        #check other ON spectra is calibrated so have values less than ~0.3
        spec_output[irow_rowflagged_output] = 0.0
        self.assertTrue(all(spec_output < 0.01))
        #check channel flags not modified
        self.assertEqual(cflag_output[irow_chanflagged_output], 128)
        cflag_output[irow_chanflagged_output] = 0
        self.assertTrue(all(cflag_output == 0))
        #check row flags not modified
        self.assertEqual(rflag_output[irow_rowflagged_output], 1)
        rflag_output[irow_rowflagged_output] = 0
        self.assertTrue(all(rflag_output == 0))

def suite():
    return [sdreduce_test, sdreduce_selection, sdreduce_test_average_flag,
            sdreduce_test_baseline_flag, sdreduce_test_cal_psalma_flag,
            sdreduce_test_cal_otf_flag, sdreduce_test_cal_otfraster_flag]
